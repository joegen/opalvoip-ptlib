/*
 * vfw.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: vfw.cxx,v $
 * Revision 1.2  2000/07/25 13:14:07  robertj
 * Got the video capture stuff going!
 *
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>


class PVideoInputThread : public PThread
{
  PCLASSINFO(PVideoInputThread, PThread);
  public:
    PVideoInputThread(PVideoInputDevice & dev)
      : PThread(30000, NoAutoDeleteThread, HighPriority), device(dev) { Resume(); }
    void Main() { device.HandleCapture(); }
  protected:
    PVideoInputDevice & device;
};


///////////////////////////////////////////////////////////////////////////////

class PCapStatus : public CAPSTATUS
{
  public:
    PCapStatus(HWND hWnd);
    BOOL IsOK() { return uiImageWidth != 0; }
};


///////////////////////////////////////////////////////////////////////////////

class PVideoDeviceBitmap : PBYTEArray
{
  public:
    PVideoDeviceBitmap(unsigned width, unsigned height, PVideoDevice::ColourFormat fmt);
    PVideoDeviceBitmap(HWND hWnd);
    BOOL ApplyFormat(HWND hWnd);

    BITMAPINFO * operator->() const { return (BITMAPINFO *)theArray; }
};



///////////////////////////////////////////////////////////////////////////////

PVideoDeviceBitmap::PVideoDeviceBitmap(unsigned width, unsigned height,
                                       PVideoDevice::ColourFormat fmt)
  : PBYTEArray(sizeof(BITMAPINFO))
{
  PINDEX i;
  BITMAPINFO * bi = (BITMAPINFO *)theArray;

  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi->bmiHeader.biWidth = width;
  bi->bmiHeader.biHeight = height;
  bi->bmiHeader.biPlanes = 1;

  switch (fmt) {
    case PVideoDevice::Grey :
      bi->bmiHeader.biCompression = BI_RGB;
      bi->bmiHeader.biBitCount = 8;
      SetSize(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
      for (i = 0; i < 256; i++)
        bi->bmiColors[i].rgbBlue = bi->bmiColors[i].rgbGreen = bi->bmiColors[i].rgbRed = (BYTE)i;
      break;
    case PVideoDevice::RGB24 :
      bi->bmiHeader.biCompression = BI_RGB;
      bi->bmiHeader.biBitCount = 24;
      break;
    case PVideoDevice::RGB32 :
      bi->bmiHeader.biCompression = BI_RGB;
      bi->bmiHeader.biBitCount = 32;
      break;
    case PVideoDevice::YUV422 :
      bi->bmiHeader.biCompression = mmioFOURCC('Y', 'U', 'V', '8');
      bi->bmiHeader.biBitCount = 8;
      break;
    case PVideoDevice::RGB565 :
      bi->bmiHeader.biCompression = BI_BITFIELDS;
      bi->bmiHeader.biBitCount = 16;
      break;
    case PVideoDevice::MJPEG :
      bi->bmiHeader.biCompression = mmioFOURCC('M','J','P','G');
      bi->bmiHeader.biBitCount = 0;
      break;
  }

  bi->bmiHeader.biSizeImage = height*((bi->bmiHeader.biBitCount*width + 31)/32)*4;
}


PVideoDeviceBitmap::PVideoDeviceBitmap(HWND hCaptureWindow)
{
  PINDEX sz = capGetVideoFormatSize(hCaptureWindow);
  SetSize(sz);
  if (capGetVideoFormat(hCaptureWindow, theArray, sz))
    return;

  PTRACE(1, "capGetVideoFormat: failed - " << GetLastError());
  SetSize(0);
}


BOOL PVideoDeviceBitmap::ApplyFormat(HWND hWnd)
{
  if (capSetVideoFormat(hWnd, theArray, GetSize()))
    return TRUE;

  PTRACE(1, "capSetVideoFormat: failed - " << GetLastError());
  return FALSE;
}


///////////////////////////////////////////////////////////////////////////////

PCapStatus::PCapStatus(HWND hWnd)
{
  memset(this, 0, sizeof(*this));
  if (capGetStatus(hWnd, this, sizeof(*this)))
    return;

  PTRACE(1, "capGetStatus: failed - " << GetLastError());
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoInputDevice::PVideoInputDevice(VideoFormat videoFmt,
                                     unsigned channel,
                                     ColourFormat colourFmt)
  : PVideoDevice(videoFmt, channel, colourFmt)
{
  captureThread = NULL;
  hCaptureWindow = NULL;
}


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  Close();

  deviceName = devName;

  captureThread = new PVideoInputThread(*this);
  threadStarted.Wait();
  if (hCaptureWindow == NULL) {
    delete captureThread;
    captureThread = NULL;
  }

  if (startImmediate)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice::IsOpen() const
{
  return hCaptureWindow != NULL;
}


BOOL PVideoInputDevice::Close()
{
  if (!IsOpen())
    return FALSE;

  Stop();

  ::PostThreadMessage(captureThread->GetThreadId(), WM_QUIT, 0, 0L);
  captureThread->WaitForTermination();
  delete captureThread;
  captureThread = NULL;

  return TRUE;
}


BOOL PVideoInputDevice::Start()
{
  if (capCaptureSequenceNoFile(hCaptureWindow))
    return TRUE;

  PTRACE(1, "capCaptureSequenceNoFile: failed - " << GetLastError());
  return FALSE;
}


BOOL PVideoInputDevice::Stop()
{
  if (capCaptureStop(hCaptureWindow))
    return TRUE;

  PTRACE(1, "capCaptureStop: failed - " << GetLastError());
  return FALSE;
}


BOOL PVideoInputDevice::IsCapturing()
{
  PCapStatus status(hCaptureWindow);
  return status.fCapturingNow;
}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  if (!PVideoDevice::SetFrameSize(width, height))
    return FALSE;

  BOOL running = IsCapturing();
  if (running)
    Stop();

  ::SetWindowPos(hCaptureWindow, NULL, 0, 0,
                 width + GetSystemMetrics(SM_CXFIXEDFRAME),
                 height + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME),
                 SWP_NOMOVE|SWP_NOACTIVATE);

  PCapStatus status(hCaptureWindow);
  if (status.IsOK()) {
    frameWidth = status.uiImageWidth;
    frameHeight = status.uiImageHeight;
  }

  if (running)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(ColourFormat colourFmt)
{
  BOOL running = IsCapturing();
  if (running)
    Stop();

  ColourFormat oldFormat = colourFormat;

  if (!PVideoDevice::SetColourFormat(colourFmt))
    return FALSE;

  PVideoDeviceBitmap bitmapInfo(frameWidth, frameHeight, colourFmt);
  if (!bitmapInfo.ApplyFormat(hCaptureWindow)) {
    PVideoDevice::SetColourFormat(oldFormat);
    return FALSE;
  }

  if (running)
    return Start();

  return TRUE;
}


PStringList PVideoInputDevice::GetDeviceNames() const
{
  PStringList list;

  for (WORD devId = 0; devId < 10; devId++) {
    char name[100];
    char version[200];
    if (capGetDriverDescription(devId, name, sizeof(name), version, sizeof(version)))
      list.AppendString(name);
  }

  return list;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return PVideoDeviceBitmap(hCaptureWindow)->bmiHeader.biSizeImage;
}


BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  lastFrameMutex.Wait();
  memcpy(buffer, lastFramePtr, lastFrameSize);
  if (bytesReturned != NULL)
    *bytesReturned = lastFrameSize;
  lastFrameMutex.Signal();

  return TRUE;
}


    
LRESULT CALLBACK PVideoInputDevice::ErrorHandler(HWND hWnd, int id, LPCSTR err)
{
  if (hWnd == NULL)
    return FALSE;

  return ((PVideoInputDevice *)capGetUserData(hWnd))->HandleError(id, err);
}


LRESULT PVideoInputDevice::HandleError(int id, LPCSTR err)
{
  if (id != 0) {
    PTRACE(1, "ErrorHandler: [id="<< id << "] " << err);
  }

  return TRUE;
}


LRESULT CALLBACK PVideoInputDevice::VideoHandler(HWND hWnd, LPVIDEOHDR vh)
{
  if (hWnd == NULL)
    return FALSE;

  return ((PVideoInputDevice *)capGetUserData(hWnd))->HandleVideo(vh);
}


LRESULT PVideoInputDevice::HandleVideo(LPVIDEOHDR vh)
{
  if (vh->dwFlags & VHDR_DONE) {
    lastFrameMutex.Wait();
    lastFramePtr = vh->lpData;
    lastFrameSize = vh->dwBytesUsed;
    lastFrameMutex.Signal();
    frameAvailable.Signal();
  }

  return TRUE;
}


BOOL PVideoInputDevice::InitialiseCapture()
{
  if ((hCaptureWindow = capCreateCaptureWindow("Capture Window",
                                               WS_POPUP | WS_CAPTION,
                                               CW_USEDEFAULT, CW_USEDEFAULT,
                                               frameWidth + GetSystemMetrics(SM_CXFIXEDFRAME),
                                               frameHeight + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME),
                                               (HWND)0,
                                               0)) == NULL) {
    PTRACE(1, "capCreateCaptureWindow: failed");
    return FALSE;
  }

  capSetCallbackOnError(hCaptureWindow, ErrorHandler);

  if (!capSetCallbackOnVideoStream(hCaptureWindow, VideoHandler)) {
    PTRACE(1, "capSetCallbackOnVideoStream: failed - " << GetLastError());
    return FALSE;
  }

  WORD devId;
  if (deviceName.GetLength() == 1 && isdigit(deviceName[0]))
    devId = (WORD)(deviceName[0] - '0');
  else {
    for (devId = 0; devId < 10; devId++) {
      char name[100];
      char version[200];
      if (capGetDriverDescription(devId, name, sizeof(name), version, sizeof(version)) &&
          (deviceName *= name))
        break;
    }
  }

  // Use first driver available.
  if (!capDriverConnect(hCaptureWindow, devId)) {
    PTRACE(1, "capDriverConnect: failed - " << GetLastError());
    return FALSE;
  }

  capSetUserData(hCaptureWindow, this);

  CAPDRIVERCAPS driverCaps;
  memset(&driverCaps, 0, sizeof(driverCaps));
  if (!capDriverGetCaps(hCaptureWindow, &driverCaps, sizeof(driverCaps))) {
    PTRACE(1, "capGetDriverCaps: failed - " << GetLastError());
    return FALSE;
  }

  if (driverCaps.fHasOverlay)
    capOverlay(hCaptureWindow, TRUE);
  else {
    capPreviewRate(hCaptureWindow, 66);
    capPreview(hCaptureWindow, TRUE);
  }

  CAPTUREPARMS parms;
  memset(&parms, 0, sizeof(parms));
  if (!capCaptureGetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    PTRACE(1, "capCaptureGetSetup: failed - " << GetLastError());
    return FALSE;
  }

  parms.dwRequestMicroSecPerFrame = 1000000 / 15;
  parms.fMakeUserHitOKToCapture = FALSE;
  parms.wPercentDropForError = 100;
  parms.fCaptureAudio = FALSE;
  parms.fAbortLeftMouse = FALSE;
  parms.fAbortRightMouse = FALSE;
  parms.fLimitEnabled = FALSE;

  if (!capCaptureSetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    PTRACE(1, "capCaptureSetSetup: failed - " << GetLastError());
    return FALSE;
  }

  if (!SetFrameSize(frameWidth, frameHeight))
    return FALSE;

  return SetColourFormat(colourFormat);
}


void PVideoInputDevice::HandleCapture()
{
  if (InitialiseCapture()) {
    threadStarted.Signal();
    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
      ::DispatchMessage(&msg);
  }

  capDriverDisconnect(hCaptureWindow);
  capSetCallbackOnVideoStream(hCaptureWindow, NULL);

  DestroyWindow(hCaptureWindow);
  hCaptureWindow = NULL;

  threadStarted.Signal();
}


// End Of File ///////////////////////////////////////////////////////////////
