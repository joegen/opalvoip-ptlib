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
 * Revision 1.7  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.6  2000/11/09 00:28:38  robertj
 * Changed video capture for step frame grab instead of streamed grabbing.
 *
 * Revision 1.5  2000/07/30 03:41:31  robertj
 * Added more colour formats to video device enum.
 *
 * Revision 1.4  2000/07/26 03:50:50  robertj
 * Added last error variable to video device.
 *
 * Revision 1.3  2000/07/25 13:38:26  robertj
 * Added frame rate parameter to video frame grabber.
 *
 * Revision 1.2  2000/07/25 13:14:07  robertj
 * Got the video capture stuff going!
 *
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>

#define STEP_GRAB_CAPTURE 0


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
    BOOL ApplyFormat(HWND hWnd) { return capSetVideoFormat(hWnd, theArray, GetSize()); }

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
    case PVideoDevice::RGB32 :
      bi->bmiHeader.biCompression = BI_RGB;
      bi->bmiHeader.biBitCount = 32;
      break;
    case PVideoDevice::RGB24 :
      bi->bmiHeader.biCompression = BI_RGB;
      bi->bmiHeader.biBitCount = 24;
      break;
    case PVideoDevice::RGB565 :
      bi->bmiHeader.biCompression = BI_BITFIELDS;
      bi->bmiHeader.biBitCount = 16;
      break;
    case PVideoDevice::RGB555 :
      bi->bmiHeader.biCompression = BI_BITFIELDS;
      bi->bmiHeader.biBitCount = 15;
      break;
    case PVideoDevice::YUV422 :
      bi->bmiHeader.biCompression = mmioFOURCC('Y', 'U', 'Y', '2');
      bi->bmiHeader.biBitCount = 16;
      break;
    case PVideoDevice::MJPEG :
      bi->bmiHeader.biCompression = mmioFOURCC('M','J','P','G');
      bi->bmiHeader.biBitCount = 0;
      break;
    default :
      bi->bmiHeader.biCompression = 0xffffffff; // Indicate invalid colour format
      return;
  }

  bi->bmiHeader.biSizeImage = height*((bi->bmiHeader.biBitCount*width + 31)/32)*4;
}


PVideoDeviceBitmap::PVideoDeviceBitmap(HWND hCaptureWindow)
{
  PINDEX sz = capGetVideoFormatSize(hCaptureWindow);
  SetSize(sz);
  if (capGetVideoFormat(hCaptureWindow, theArray, sz))
    return;

  PTRACE(1, "capGetVideoFormat: failed - " << ::GetLastError());
  SetSize(0);
}


///////////////////////////////////////////////////////////////////////////////

PCapStatus::PCapStatus(HWND hWnd)
{
  memset(this, 0, sizeof(*this));
  if (capGetStatus(hWnd, this, sizeof(*this)))
    return;

  PTRACE(1, "capGetStatus: failed - " << ::GetLastError());
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoInputDevice::PVideoInputDevice(VideoFormat videoFmt,
                                     int channel,
                                     ColourFormat colourFmt)
  : PVideoDevice(videoFmt, channel, colourFmt)
{
  captureThread = NULL;
  hCaptureWindow = NULL;
  lastFramePtr = NULL;
  lastFrameSize = 0;

  conversion = NULL;
}

BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  if( channelNumber < 0 )
    return FALSE;

  Close();

  deviceName = devName;

  captureThread = new PVideoInputThread(*this);
  threadStarted.Wait();
  if (hCaptureWindow == NULL) {
    delete captureThread;
    captureThread = NULL;
    return FALSE;
  }

  if (startImmediate)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice::IsOpen() 
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
#if STEP_GRAB_CAPTURE
  return IsOpen();
#else
  if (capCaptureSequenceNoFile(hCaptureWindow))
    return TRUE;

  lastError = ::GetLastError();
  PTRACE(1, "capCaptureSequenceNoFile: failed - " << lastError);
  return FALSE;
#endif
}


BOOL PVideoInputDevice::Stop()
{
#if STEP_GRAB_CAPTURE
  return IsOpen();
#else
  if (capCaptureStop(hCaptureWindow))
    return TRUE;

  lastError = ::GetLastError();
  PTRACE(1, "capCaptureStop: failed - " << lastError);
  return FALSE;
#endif
}


BOOL PVideoInputDevice::IsCapturing()
{
  PCapStatus status(hCaptureWindow);
  return status.fCapturingNow;
}


BOOL PVideoInputDevice::SetFrameRate(unsigned rate)
{
  if (!PVideoDevice::SetFrameRate(rate))
    return FALSE;

  BOOL running = IsCapturing();
  if (running)
    Stop();

  CAPTUREPARMS parms;
  memset(&parms, 0, sizeof(parms));

  if (!capCaptureGetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    lastError = ::GetLastError();
    PTRACE(1, "capCaptureGetSetup: failed - " << lastError);
    return FALSE;
  }

  parms.dwRequestMicroSecPerFrame = 100000000 / frameRate;
  parms.fMakeUserHitOKToCapture = FALSE;
  parms.wPercentDropForError = 100;
  parms.fCaptureAudio = FALSE;
  parms.fAbortLeftMouse = FALSE;
  parms.fAbortRightMouse = FALSE;
  parms.fLimitEnabled = FALSE;

  if (!capCaptureSetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    lastError = ::GetLastError();
    PTRACE(1, "capCaptureSetSetup: failed - " << lastError);
    return FALSE;
  }

  if (running)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
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
    lastError = ::GetLastError();
    PTRACE(1, "capSetVideoFormat: failed - " << lastError);
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
   if (IsOpen())
     return PVideoDeviceBitmap(hCaptureWindow)->bmiHeader.biSizeImage;

  return 0;
}


BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  lastFrameMutex.Wait();

  if (lastFramePtr != NULL) {
    memcpy(buffer, lastFramePtr, lastFrameSize);
    if (bytesReturned != NULL)
      *bytesReturned = lastFrameSize;
  }

  lastFrameMutex.Signal();

#if STEP_GRAB_CAPTURE
  capGrabFrameNoStop(hCaptureWindow);
#endif
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
    if (lastFrameSize == 0)
      lastFrameSize = vh->dwBufferLength;
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
    lastError = ::GetLastError();
    PTRACE(1, "capCreateCaptureWindow: failed");
    return FALSE;
  }

  capSetCallbackOnError(hCaptureWindow, ErrorHandler);

#if STEP_GRAB_CAPTURE
  if (!capSetCallbackOnFrame(hCaptureWindow, VideoHandler)) {
#else
  if (!capSetCallbackOnVideoStream(hCaptureWindow, VideoHandler)) {
#endif
    lastError = ::GetLastError();
    PTRACE(1, "capSetCallbackOnVideoStream: failed - " << lastError);
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
    lastError = ::GetLastError();
    PTRACE(1, "capDriverConnect: failed - " << lastError);
    return FALSE;
  }

  capSetUserData(hCaptureWindow, this);

  CAPDRIVERCAPS driverCaps;
  memset(&driverCaps, 0, sizeof(driverCaps));
  if (!capDriverGetCaps(hCaptureWindow, &driverCaps, sizeof(driverCaps))) {
    lastError = ::GetLastError();
    PTRACE(1, "capGetDriverCaps: failed - " << lastError);
    return FALSE;
  }

/*
  if (driverCaps.fHasOverlay)
    capOverlay(hCaptureWindow, TRUE);
  else {
    capPreviewRate(hCaptureWindow, 66);
    capPreview(hCaptureWindow, TRUE);
  }
*/
  capPreview(hCaptureWindow, FALSE);

  if (!SetFrameRate(frameRate))
    return FALSE;

#if STEP_GRAB_CAPTURE
  if (!SetColourFormat(colourFormat))
    return FALSE;

  return capGrabFrameNoStop(hCaptureWindow);
#else
  return SetColourFormat(colourFormat);
#endif
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
