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
 * Revision 1.13  2001/11/28 04:37:46  robertj
 * Added "flipped" colour formats, thanks Telefonica Spain.
 * Added support for grabbing at a frame rate (from Linux).
 * Adjusted thread priority causing starvation, thanks Telefonica Spain.
 * Fixed startup problem if initialise gets error, thanks Telefonica Spain.
 *
 * Revision 1.12  2001/03/30 07:20:37  robertj
 * Some drivers (QuickCam) use key frame bit to indicate grab complete.
 *
 * Revision 1.11  2001/03/08 22:57:15  robertj
 * Added new VerifyHardwareFrameSize() function
 *
 * Revision 1.10  2001/03/08 02:18:45  robertj
 * Added improved defaulting of video formats so Open() does not fail.
 * Removed the requirement that the device be open before you can set
 *   formats such as colour, video, channel number etc.
 *
 * Revision 1.9  2001/03/06 23:34:20  robertj
 * Added static function to get input device names.
 * Moved some inline virtuals to non-inline.
 *
 * Revision 1.8  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
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
#include <ptlib/vconvert.h>


#define STEP_GRAB_CAPTURE 1


class PVideoInputThread : public PThread
{
  PCLASSINFO(PVideoInputThread, PThread);
  public:
 
    PVideoInputThread(PVideoInputDevice & dev)
      : PThread(30000, NoAutoDeleteThread, NormalPriority), device(dev) { Resume(); }
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
    PVideoDeviceBitmap(unsigned width, unsigned height, const PString & fmt);
    PVideoDeviceBitmap(HWND hWnd);
    BOOL ApplyFormat(HWND hWnd) { return capSetVideoFormat(hWnd, theArray, GetSize()); }

    BITMAPINFO * operator->() const { return (BITMAPINFO *)theArray; }
};



///////////////////////////////////////////////////////////////////////////////


static struct {
  const char * colourFormat;
  BOOL  flipped;
  WORD  bitCount;
  DWORD compression;
} FormatTable[] = {
  { "Grey",    FALSE, 8,  BI_RGB,       },
  { "Gray",    FALSE, 8,  BI_RGB,       },
  { "GreyF",   TRUE,  8,  BI_RGB,       },
  { "GrayF",   TRUE,  8,  BI_RGB,       },
  { "RGB32",   FALSE, 32, BI_RGB,       },
  { "RGB32F",  TRUE,  32, BI_RGB,       },
  { "RGB24",   FALSE, 24, BI_RGB,       },
  { "RGB24F",  TRUE,  24, BI_RGB,       },
  { "RGB565",  FALSE, 16, BI_BITFIELDS, },
  { "RGB565F", TRUE,  16, BI_BITFIELDS, },
  { "RGB555",  FALSE, 15, BI_BITFIELDS, },
  { "RGB555F", TRUE,  15, BI_BITFIELDS, },

  // http://support.microsoft.com/support/kb/articles/q294/8/80.asp
  { "YUV420P", FALSE, 12, mmioFOURCC('I','Y','U','V') },
  { "IYUV",    FALSE, 12, mmioFOURCC('I','Y','U','V') },
  { "I420",    FALSE, 12, mmioFOURCC('I','4','2','0') }, // Like YVUV
  { "YV12",    FALSE, 12, mmioFOURCC('Y','V','1','2') }, // Like YVUV except planes switched

  { "YUV422",  FALSE, 16, mmioFOURCC('Y','U','Y','2') },
  { "YUY2",    FALSE, 16, mmioFOURCC('Y','U','Y','2') },
  { "UYVY",    FALSE, 16, mmioFOURCC('U','Y','V','Y') }, // Like YUY2 except for ordering
  { "YVYU",    FALSE, 16, mmioFOURCC('Y','V','Y','U') }, // Like YUY2 except for ordering

  { "MJPEG",   FALSE, 0,  mmioFOURCC('M','J','P','G') },
  { NULL },
};


PVideoDeviceBitmap::PVideoDeviceBitmap(unsigned width, unsigned height,
                                       const PString & fmt)
  : PBYTEArray(sizeof(BITMAPINFO))
{
  PINDEX i = 0;
  while (FormatTable[i].colourFormat != NULL && !(fmt *= FormatTable[i].colourFormat))
    i++;

  BITMAPINFO * bi = (BITMAPINFO *)theArray;

  bi->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  bi->bmiHeader.biWidth = width;
  bi->bmiHeader.biHeight = FormatTable[i].flipped ? height : -(int)height;
  bi->bmiHeader.biPlanes = 1;
  bi->bmiHeader.biBitCount = FormatTable[i].bitCount;

  if (FormatTable[i].colourFormat != NULL)
    bi->bmiHeader.biCompression = FormatTable[i].compression;
  else if (fmt.GetLength() == 4)
    bi->bmiHeader.biCompression = mmioFOURCC(fmt[0],fmt[1],fmt[2],fmt[3]);
  else {
    bi->bmiHeader.biCompression = 0xffffffff; // Indicate invalid colour format
    return;
  }

  bi->bmiHeader.biSizeImage = height*((bi->bmiHeader.biBitCount*width + 31)/32)*4;

  if (bi->bmiHeader.biCompression == BI_RGB && bi->bmiHeader.biBitCount == 8) {
    SetSize(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
    for (i = 0; i < 256; i++)
      bi->bmiColors[i].rgbBlue = bi->bmiColors[i].rgbGreen = bi->bmiColors[i].rgbRed = (BYTE)i;
    bi->bmiHeader.biHeight = -bi->bmiHeader.biHeight;
  }
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

PVideoInputDevice::PVideoInputDevice()
{
  captureThread = NULL;
  hCaptureWindow = NULL;
  lastFramePtr = NULL;
  lastFrameSize = 0;
  isCapturingNow = FALSE;
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
  if (IsCapturing())
    return FALSE;
#if STEP_GRAB_CAPTURE
  isCapturingNow = TRUE;
  return capGrabFrameNoStop(hCaptureWindow);
#else
  if (capCaptureSequenceNoFile(hCaptureWindow)) {
    PCapStatus status(hCaptureWindow);
    isCapturingNow = status.fCapturingNow;
    return isCapturingNow;
  }

  lastError = ::GetLastError();
  PTRACE(1, "capCaptureSequenceNoFile: failed - " << lastError);
  return FALSE;
#endif
}


BOOL PVideoInputDevice::Stop()
{
  if (!IsCapturing())
    return FALSE;
  isCapturingNow = FALSE;
#if STEP_GRAB_CAPTURE
  return IsOpen() && frameAvailable.Wait(1000);
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
  return isCapturingNow;
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


BOOL PVideoInputDevice::VerifyHardwareFrameSize(unsigned width, unsigned height)
{
  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(const PString & colourFmt)
{
  BOOL running = IsCapturing();
  if (running)
    Stop();

  PString oldFormat = colourFormat;

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


PStringList PVideoInputDevice::GetInputDeviceNames()
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
  if (!IsOpen())
    return 0;

  PINDEX size = PVideoDeviceBitmap(hCaptureWindow)->bmiHeader.biSizeImage;
  if (converter != NULL && size < converter->GetMaxDstFrameBytes())
    return converter->GetMaxDstFrameBytes();

  return size;
}


BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


BOOL PVideoInputDevice::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  lastFrameMutex.Wait();

  if (lastFramePtr != NULL) {
    if (converter != NULL)
      converter->Convert(lastFramePtr, buffer, bytesReturned);
    else {
      memcpy(buffer, lastFramePtr, lastFrameSize);
      if (bytesReturned != NULL)
        *bytesReturned = lastFrameSize;
    }
  }

  lastFrameMutex.Signal();

#if STEP_GRAB_CAPTURE
  if (isCapturingNow)
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
  if ((vh->dwFlags&(VHDR_DONE|VHDR_KEYFRAME)) != 0) {
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

  return SetColourFormat(colourFormat);
}


void PVideoInputDevice::HandleCapture()
{
  BOOL initSucceeded = InitialiseCapture();

  if (initSucceeded) {
    threadStarted.Signal();

    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
      ::DispatchMessage(&msg);
  }
  
  capDriverDisconnect(hCaptureWindow);
  capSetUserData(hCaptureWindow, NULL);

  capSetCallbackOnError(hCaptureWindow, NULL);
  capSetCallbackOnVideoStream(hCaptureWindow, NULL);

  DestroyWindow(hCaptureWindow);
  hCaptureWindow = NULL;

  // Signal the other thread we have completed, even if have error
  if (!initSucceeded)
    threadStarted.Signal();
}


// End Of File ///////////////////////////////////////////////////////////////
