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
 *                 Walter H Whitlock (twohives@nc.rr.com)
 *
 * $Log: vfw.cxx,v $
 * Revision 1.25  2003/12/14 10:01:03  rjongbloed
 * Resolved issue with name space conflict os static and virtual forms of GetDeviceNames() function.
 *
 * Revision 1.24  2003/11/18 06:46:59  csoutheren
 * Changed to support video input plugins
 *
 * Revision 1.23  2003/11/05 05:58:10  csoutheren
 * Added #pragma to include required libs
 *
 * Revision 1.22  2003/05/14 02:51:42  rjongbloed
 * Protected use of user data in video for windows calls.
 *
 * Revision 1.21  2003/03/17 07:52:15  robertj
 * Fixed return value if starting capture and already have it started.
 *
 * Revision 1.20  2002/10/24 20:01:53  dereks
 * Improve  closure of windows capture device, with fix from Diego Tartara. Thanks!
 *
 * Revision 1.19  2002/09/01 23:54:33  dereks
 * Fix from Diego Tartara to handle (better) disconnection situation.
 *
 * Revision 1.18  2002/02/25 08:01:02  robertj
 * Changed to utilise preferred colour format, thanks Martijn Roest
 *
 * Revision 1.17  2002/01/15 23:52:07  robertj
 * Fixed some incorrect table entries for colout formats, thanks Martijn Roest
 *
 * Revision 1.16  2002/01/10 03:52:41  robertj
 * Fixed 8bpp problem, thanks Walter Whitlock.
 *
 * Revision 1.15  2002/01/08 01:42:06  robertj
 * Tidied up some PTRACE debug output.
 * Changed some code formatting.
 * Fixed SetFrameSize so propagates to ancestor and thus sets the the
 *   converter size as well as the raw grabber.
 *
 * Revision 1.14  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
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

#pragma comment(lib, "vfw32.lib")

#define STEP_GRAB_CAPTURE 1

///////////////////////////////////////////////////////////////////////////////

/**This class defines a video input device.
 */
class PVideoInputDevice_VideoForWindows : public PVideoInputDevice
{
  PCLASSINFO(PVideoInputDevice_VideoForWindows, PVideoInputDevice);

  public:
    /** Create a new video input device.
     */
    PVideoInputDevice_VideoForWindows();

    /**Close the video input device on destruction.
      */
    ~PVideoInputDevice_VideoForWindows() { Close(); }

    /** Is the device a camera, and obtain video
     */
    static PStringList GetInputDeviceNames();

    virtual PStringList GetDeviceNames() const
      { return GetInputDeviceNames(); }

    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine if the device is currently open.
      */
    virtual BOOL IsOpen();

    /**Close the device.
      */
    virtual BOOL Close();

    /**Start the video device I/O.
      */
    virtual BOOL Start();

    /**Stop the video device I/O capture.
      */
    virtual BOOL Stop();

    /**Determine if the video device I/O capture is in progress.
      */
    virtual BOOL IsCapturing();

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();

    /**Grab a frame.
      */
    virtual BOOL GetFrame(
      PBYTEArray & frame
    );

    /**Grab a frame, after a delay as specified by the frame rate.
      */
    virtual BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );

    /**Grab a frame. Do not delay according to the current frame rate parameter.
      */
    virtual BOOL GetFrameDataNoDelay(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );


    /**Try all known video formats & see which ones are accepted by the video driver
     */
    virtual BOOL TestAllFormats();

  protected:

   /**Check the hardware can do the asked for size.

       Note that not all cameras can provide all frame sizes.
     */
    virtual BOOL VerifyHardwareFrameSize(unsigned width, unsigned height);

  public:
    virtual BOOL SetColourFormat(const PString & colourFormat);
    virtual BOOL SetFrameRate(unsigned rate);
    virtual BOOL SetFrameSize(unsigned width, unsigned height);

  protected:
    static LRESULT CALLBACK ErrorHandler(HWND hWnd, int id, LPCSTR err);
    LRESULT HandleError(int id, LPCSTR err);
    static LRESULT CALLBACK VideoHandler(HWND hWnd, LPVIDEOHDR vh);
    LRESULT HandleVideo(LPVIDEOHDR vh);
    BOOL InitialiseCapture();
    void HandleCapture();

    PThread     * captureThread;
    PSyncPoint    threadStarted;

    HWND          hCaptureWindow;

    PSyncPoint    frameAvailable;
    LPBYTE        lastFramePtr;
    unsigned      lastFrameSize;
    PMutex        lastFrameMutex;
    BOOL          isCapturingNow;

  friend class PVideoInputThread;
};


///////////////////////////////////////////////////////////////////////////////

class PVideoInputThread : public PThread
{
    PCLASSINFO(PVideoInputThread, PThread);
  public:
    PVideoInputThread(PVideoInputDevice_VideoForWindows & dev)
      : PThread(30000, NoAutoDeleteThread, NormalPriority, "PVideoInput:%x"),
        device(dev)
      { Resume(); }
    void Main()
      { device.HandleCapture(); }
  protected:
    PVideoInputDevice_VideoForWindows & device;
};


///////////////////////////////////////////////////////////////////////////////

class PCapStatus : public CAPSTATUS
{
  public:
    PCapStatus(HWND hWnd);
    BOOL IsOK()
       { return uiImageWidth != 0; }
};

///////////////////////////////////////////////////////////////////////////////

class PVideoDeviceBitmap : PBYTEArray
{
  public:
    // the following method is replaced by PVideoDeviceBitmap(HWND hWnd, WORD bpp)
    // PVideoDeviceBitmap(unsigned width, unsigned height, const PString & fmt);
    //returns object with gray color pallet if needed for 8 bpp formats
    PVideoDeviceBitmap(HWND hWnd, WORD bpp);
    // does not build color pallet
    PVideoDeviceBitmap(HWND hWnd); 
    // apply video format to capture device
    BOOL ApplyFormat(HWND hWnd);

    BITMAPINFO * operator->() const 
    { return (BITMAPINFO *)theArray; }
};

///////////////////////////////////////////////////////////////////////////////

#define INVALID_COMP 0xffffffff

static struct vfwComp {
// MS standard RGB DIB formats scan from bottom up, 
// however standard YUV formats scan from top down
  DWORD compression; // values for .biCompression field that *might* be accepted
  BOOL  askForVFlip; // is it likely to need vertical flipping during conversion?
} vfwCompTable[] = {
  { BI_RGB,                       TRUE, }, //[0]
  { BI_BITFIELDS,                 TRUE, }, //[1]
// http://support.microsoft.com/support/kb/articles/q294/8/80.asp
  { mmioFOURCC('I','Y','U','V'), FALSE, }, //[2]
  { mmioFOURCC('Y','V','1','2'), FALSE, }, //[3], same as IYUV/I420 except that  U and V planes are switched
  { mmioFOURCC('Y','U','Y','2'), FALSE, }, //[4]
  { mmioFOURCC('U','Y','V','Y'), FALSE, }, //[5], Like YUY2 except for ordering
  { mmioFOURCC('Y','V','Y','U'), FALSE, }, //[6], Like YUY2 except for ordering
  { mmioFOURCC('Y','V','U','9'), FALSE, }, //[7]
  { mmioFOURCC('M','J','P','G'), FALSE, }, //[8]
// the following compression formats are repeats of earlier formats
  { mmioFOURCC('I','4','2','0'), FALSE, }, // [9] same as IYUV
  { INVALID_COMP, }, 
};

static struct { 
  const char * colourFormat; 
  WORD  bitCount;
  BOOL negHeight; // MS documentation suggests that negative height will request
                  // top down scan direction from video driver
                  // HOWEVER, not all drivers honor this request
  vfwComp *vfwComp; 
} FormatTable[] = {
  { "RGB24",   24, TRUE,  &vfwCompTable[0], },
  { "RGB24F",  24, FALSE, &vfwCompTable[0], },
  { "RGB32",   32, TRUE,  &vfwCompTable[0], },
  { "RGB32F",  32, FALSE, &vfwCompTable[0], },
  { "Grey",     8, TRUE,  &vfwCompTable[0], },
  { "Gray",     8, TRUE,  &vfwCompTable[0], },
  { "GreyF",    8, FALSE, &vfwCompTable[0], },
  { "GrayF",    8, FALSE, &vfwCompTable[0], },
  { "RGB565",  16, TRUE,  &vfwCompTable[1], },
  { "RGB565F", 16, FALSE, &vfwCompTable[1], },
  { "RGB555",  15, TRUE,  &vfwCompTable[1], },
  { "RGB555F", 15, FALSE, &vfwCompTable[1], },
  //{  NULL,      0, FALSE, NULL, }, // uncomment to prevent YUV420P from camera which can't be resized
  { "YUV420P", 12, FALSE, &vfwCompTable[2], },
  { "IYUV",    12, FALSE, &vfwCompTable[2], },
  { "I420",    12, FALSE, &vfwCompTable[9], },
  { "YV12",    12, FALSE, &vfwCompTable[3], },
  { "YUV422",  16, FALSE, &vfwCompTable[4], },
  { "YUY2",    16, FALSE, &vfwCompTable[4], },
  { "UYVY",    16, FALSE, &vfwCompTable[5], },
  { "YVYU",    16, FALSE, &vfwCompTable[6], },
  { "MJPEG",   12, FALSE, &vfwCompTable[8], },
  { NULL,},
};


static struct {
    unsigned device_width, device_height;
} winTestResTable[] = {
    { 176, 144 },
    { 352, 288 },
    { 320, 240 },
    { 160, 120 },
    { 640, 480 },
    { 704, 576 },
    {1024, 768 },
};

PVideoDeviceBitmap::PVideoDeviceBitmap(HWND hCaptureWindow)
{
  PINDEX sz = capGetVideoFormatSize(hCaptureWindow);
  SetSize(sz);
  if (!capGetVideoFormat(hCaptureWindow, theArray, sz)) { 
    PTRACE(1, "PVidInp\tcapGetVideoFormat(hCaptureWindow) failed - " << ::GetLastError());
    SetSize(0);
    return;
  }
}

PVideoDeviceBitmap::PVideoDeviceBitmap(HWND hCaptureWindow, WORD bpp) {
  PINDEX sz = capGetVideoFormatSize(hCaptureWindow);
  SetSize(sz);
  if (!capGetVideoFormat(hCaptureWindow, theArray, sz)) { 
    PTRACE(1, "PVidInp\tcapGetVideoFormat(hCaptureWindow) failed - " << ::GetLastError());
    SetSize(0);
    return;
  }

  if (8 == bpp && bpp != ((BITMAPINFO*)theArray)->bmiHeader.biBitCount) {
    SetSize(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
    for (int i = 0; i < 256; i++)
      ((BITMAPINFO*)theArray)->bmiColors[i].rgbBlue  = ((BITMAPINFO*)theArray)->bmiColors[i].rgbGreen = ((BITMAPINFO*)theArray)->bmiColors[i].rgbRed = (BYTE)i;
  }
  ((BITMAPINFO*)theArray)->bmiHeader.biBitCount = bpp;
}

PVideoDeviceBitmap::ApplyFormat(HWND hWnd) {
  // NB it is necessary to set the biSizeImage value appropriate to frame size
  // assume bmiHeader.biBitCount has already been set appropriatly for format
  ((BITMAPINFO*)theArray)->bmiHeader.biPlanes = 1;
  ((BITMAPINFO*)theArray)->bmiHeader.biSizeImage = 
    (((BITMAPINFO*)theArray)->bmiHeader.biHeight<0 ? -((BITMAPINFO*)theArray)->bmiHeader.biHeight : ((BITMAPINFO*)theArray)->bmiHeader.biHeight)
    *4*((((BITMAPINFO*)theArray)->bmiHeader.biBitCount * ((BITMAPINFO*)theArray)->bmiHeader.biWidth + 31)/32);
  return capSetVideoFormat(hWnd, theArray, GetSize());
}

///////////////////////////////////////////////////////////////////////////////

PCapStatus::PCapStatus(HWND hWnd)
{
  memset(this, 0, sizeof(*this));
  if (capGetStatus(hWnd, this, sizeof(*this)))
    return;

  PTRACE(1, "PVidInp\tcapGetStatus: failed - " << ::GetLastError());
}


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PCREATE_VIDINPUT_PLUGIN(VideoForWindows, PVideoInputDevice_VideoForWindows);

PVideoInputDevice_VideoForWindows::PVideoInputDevice_VideoForWindows()
{
  captureThread = NULL;
  hCaptureWindow = NULL;
  lastFramePtr = NULL;
  lastFrameSize = 0;
  isCapturingNow = FALSE;
}

BOOL PVideoInputDevice_VideoForWindows::Open(const PString & devName, BOOL startImmediate)
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


BOOL PVideoInputDevice_VideoForWindows::IsOpen() 
{
  return hCaptureWindow != NULL;
}


BOOL PVideoInputDevice_VideoForWindows::Close()
{
  if (!IsOpen())
    return FALSE;
 
  Stop();

  ::PostThreadMessage(captureThread->GetThreadId(), WM_QUIT, 0, 0L);

  // Some brain dead drivers may hang so we provide a timeout.
  if (!captureThread->WaitForTermination(5000))
  {
      // Two things may happen if we are forced to terminate the capture thread:
      // 1. As the VIDCAP window is associated to that thread the OS itself will 
      //    close the window and release the driver
      // 2. the driver will not be released and we will not have video until we 
      //    terminate the process
      // Any of the two ios better than just hanging
      captureThread->Terminate();
      hCaptureWindow = NULL;
      PTRACE(1, "PVidInp\tCapture thread failed to stop. Terminated");
  }

  delete captureThread;
  captureThread = NULL;

  return TRUE;
}


BOOL PVideoInputDevice_VideoForWindows::Start()
{
  if (IsCapturing())
    return TRUE;

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
  PTRACE(1, "PVidInp\tcapCaptureSequenceNoFile: failed - " << lastError);
  return FALSE;
#endif
}


BOOL PVideoInputDevice_VideoForWindows::Stop()
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
  PTRACE(1, "PVidInp\tcapCaptureStop: failed - " << lastError);
  return FALSE;
#endif
}


BOOL PVideoInputDevice_VideoForWindows::IsCapturing()
{
  return isCapturingNow;
}


BOOL PVideoInputDevice_VideoForWindows::TestAllFormats() {
  BOOL running = IsCapturing();
  if (running)
    Stop();

  for (PINDEX prefFormatIdx = 0; prefFormatIdx < PARRAYSIZE(FormatTable); prefFormatIdx++) {
    PVideoDeviceBitmap bi(hCaptureWindow, FormatTable[prefFormatIdx].bitCount); 
    for (PINDEX prefResizeIdx = 0; prefResizeIdx < PARRAYSIZE(winTestResTable); prefResizeIdx++) {
      if (FormatTable[prefFormatIdx].colourFormat != NULL)
        bi->bmiHeader.biCompression = FormatTable[prefFormatIdx].vfwComp->compression;
      else
        continue;

      bi->bmiHeader.biHeight = winTestResTable[prefResizeIdx].device_height;
      bi->bmiHeader.biWidth = winTestResTable[prefResizeIdx].device_width;

      // set .biHeight according to .negHeight value
      if (FormatTable[prefFormatIdx].negHeight && bi->bmiHeader.biHeight > 0)
        bi->bmiHeader.biHeight = -(int)bi->bmiHeader.biHeight; 
      else if (!FormatTable[prefFormatIdx].negHeight && bi->bmiHeader.biHeight < 0)
        bi->bmiHeader.biHeight = -(int)bi->bmiHeader.biHeight; 

      if (bi.ApplyFormat(hCaptureWindow)) {
        PTRACE(3, "PVidInp\tcapSetVideoFormat succeeded: "
               << FormatTable[prefFormatIdx].colourFormat << ' '
               << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
               << " sz=" << bi->bmiHeader.biSizeImage);
      }
      else {
        PTRACE(1, "PVidInp\tcapSetVideoFormat failed: "
               << FormatTable[prefFormatIdx].colourFormat << ' '
               << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
               << " sz=" << bi->bmiHeader.biSizeImage
               << " - lastError=" << lastError);
      }
    } // for prefResizeIdx
  } // for prefFormatIdx

  if (running)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice_VideoForWindows::SetFrameRate(unsigned rate)
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
    PTRACE(1, "PVidInp\tcapCaptureGetSetup: failed - " << lastError);
    return FALSE;
  }

  // keep current (default) framerate if 0==frameRate   
  if (0 != frameRate)
    parms.dwRequestMicroSecPerFrame = 1000000 / frameRate;
  parms.fMakeUserHitOKToCapture = FALSE;
  parms.wPercentDropForError = 100;
  parms.fCaptureAudio = FALSE;
  parms.fAbortLeftMouse = FALSE;
  parms.fAbortRightMouse = FALSE;
  parms.fLimitEnabled = FALSE;

  if (!capCaptureSetSetup(hCaptureWindow, &parms, sizeof(parms))) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapCaptureSetSetup: failed - " << lastError);
    return FALSE;
  }
    
  if (running)
    return Start();

  return TRUE;
}


BOOL PVideoInputDevice_VideoForWindows::SetFrameSize(unsigned width, unsigned height)
{
  BOOL running = IsCapturing();
  if (running)
    Stop();

  PVideoDeviceBitmap bi(hCaptureWindow); 
  PTRACE(5, "PVidInp\tRead current biHeight from driver as " << bi->bmiHeader.biHeight);

  bi->bmiHeader.biWidth = width;
  bi->bmiHeader.biHeight = height;

  if (!bi.ApplyFormat(hCaptureWindow)) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapSetVideoFormat failed: "
           << colourFormat << ' '
           << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
           << " sz=" << bi->bmiHeader.biSizeImage
           << " - lastError=" << lastError);
    return FALSE;
  }

  PTRACE(3, "PVidInp\tcapSetVideoFormat succeeded: "
         << colourFormat << ' '
         << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
         << " sz=" << bi->bmiHeader.biSizeImage);
  
  // verify that the driver really took the frame size
  if (!VerifyHardwareFrameSize(width, height)) 
    return FALSE; 

  // frameHeight must be positive regardlesss of what the driver says
  if (0 > (int)height) 
    height = (unsigned)-(int)height;

  if (!PVideoDevice::SetFrameSize(width, height))
    return FALSE;

  if (running)
    return Start();

  return TRUE;
}


//return TRUE if absolute value of height reported by driver 
//  is equal to absolute value of current frame height AND
//  width reported by driver is equal to current frame width
BOOL PVideoInputDevice_VideoForWindows::VerifyHardwareFrameSize(unsigned width, unsigned height)
{
  PCapStatus status(hCaptureWindow);

  if (!status.IsOK())
    return FALSE;

  if (width != status.uiImageWidth)
    return FALSE;

  if (0 > (int)height)
    height = (unsigned)-(int)height;

  if (0 > (int)status.uiImageHeight)
    status.uiImageHeight = (unsigned)-(int)status.uiImageHeight;

  return (height == status.uiImageHeight);
}


BOOL PVideoInputDevice_VideoForWindows::SetColourFormat(const PString & colourFmt)
{
  BOOL running = IsCapturing();
  if (running)
    Stop();

  PString oldFormat = colourFormat;

  if (!PVideoDevice::SetColourFormat(colourFmt)) {
    return FALSE;
  }

  PINDEX i = 0;
  while (FormatTable[i].colourFormat != NULL && !(colourFmt *= FormatTable[i].colourFormat))
    i++;

  PVideoDeviceBitmap bi(hCaptureWindow, FormatTable[i].bitCount);

  // set frame width and height
  bi->bmiHeader.biHeight = frameHeight;
  bi->bmiHeader.biWidth = frameWidth;
  // set .biHeight according to .negHeight value
  if ( (FormatTable[i].negHeight) && (0 < (int)bi->bmiHeader.biHeight) )
    bi->bmiHeader.biHeight = -(int)bi->bmiHeader.biHeight; 
  else  if (!(FormatTable[i].negHeight) && !(0 < (int)bi->bmiHeader.biHeight) )
    bi->bmiHeader.biHeight = -(int)bi->bmiHeader.biHeight; 

  if (FormatTable[i].colourFormat != NULL)
    bi->bmiHeader.biCompression = FormatTable[i].vfwComp->compression;
  else if (colourFmt.GetLength() == 4)
    bi->bmiHeader.biCompression = mmioFOURCC(colourFmt[0],colourFmt[1],colourFmt[2],colourFmt[3]);
  else {
    bi->bmiHeader.biCompression = INVALID_COMP; // Indicate invalid colour format
    return FALSE;
  }

  // try to apply new colourFmt using current frame resolution
  if (!bi.ApplyFormat(hCaptureWindow)) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapSetVideoFormat failed: "
           << colourFormat << ' '
           << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
           << " sz=" << bi->bmiHeader.biSizeImage
           << " - lastError=" << lastError);
    PVideoDevice::SetColourFormat(oldFormat);
    return FALSE;
  }

  PTRACE(3, "PVidInp\tcapSetVideoFormat succeeded: "
         << colourFormat << ' '
         << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
         << " sz=" << bi->bmiHeader.biSizeImage);

  if (converter) {
    converter->SetVFlipState(FormatTable[i].vfwComp->askForVFlip); 
    PTRACE(4, "PVidInp\tSetColourFormat(): converter.doVFlip set to " << converter->GetVFlipState());
  }
      
  if (running)
    return Start();

  return TRUE;
}


PStringList PVideoInputDevice_VideoForWindows::GetInputDeviceNames()
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


PINDEX PVideoInputDevice_VideoForWindows::GetMaxFrameBytes()
{
  if (!IsOpen())
    return 0;

  PINDEX size = PVideoDeviceBitmap(hCaptureWindow)->bmiHeader.biSizeImage;
  if (converter != NULL && size < converter->GetMaxDstFrameBytes())
    return converter->GetMaxDstFrameBytes();

  return size;
}

BOOL PVideoInputDevice_VideoForWindows::GetFrame(PBYTEArray & frame)
{
  PINDEX returned;
  if (!GetFrameData(frame.GetPointer(GetMaxFrameBytes()), &returned))
    return FALSE;

  frame.SetSize(returned);
  return TRUE;
}

BOOL PVideoInputDevice_VideoForWindows::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


BOOL PVideoInputDevice_VideoForWindows::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  lastFrameMutex.Wait();

  if (lastFramePtr != NULL) {
    if (NULL != converter)
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


LRESULT CALLBACK PVideoInputDevice_VideoForWindows::ErrorHandler(HWND hWnd, int id, LPCSTR err)
{
  if (hWnd == NULL)
    return FALSE;

  return ((PVideoInputDevice_VideoForWindows *)capGetUserData(hWnd))->HandleError(id, err);
}


LRESULT PVideoInputDevice_VideoForWindows::HandleError(int id, LPCSTR err)
{
  if (id != 0) {
    PTRACE(1, "PVidInp\tErrorHandler: [id="<< id << "] " << err);
  }

  return TRUE;
}


LRESULT CALLBACK PVideoInputDevice_VideoForWindows::VideoHandler(HWND hWnd, LPVIDEOHDR vh)
{
  if (hWnd == NULL || capGetUserData(hWnd) == NULL)
    return FALSE;

  return ((PVideoInputDevice_VideoForWindows *)capGetUserData(hWnd))->HandleVideo(vh);
}


LRESULT PVideoInputDevice_VideoForWindows::HandleVideo(LPVIDEOHDR vh)
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


BOOL PVideoInputDevice_VideoForWindows::InitialiseCapture()
{
  if ((hCaptureWindow = capCreateCaptureWindow("Capture Window",
                                               WS_POPUP | WS_CAPTION,
                                               CW_USEDEFAULT, CW_USEDEFAULT,
                                               frameWidth + GetSystemMetrics(SM_CXFIXEDFRAME),
                                               frameHeight + GetSystemMetrics(SM_CYCAPTION) + GetSystemMetrics(SM_CYFIXEDFRAME),
                                               (HWND)0,
                                               0)) == NULL) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapCreateCaptureWindow failed - " << lastError);
    return FALSE;
  }

  capSetCallbackOnError(hCaptureWindow, ErrorHandler);

#if STEP_GRAB_CAPTURE
  if (!capSetCallbackOnFrame(hCaptureWindow, VideoHandler)) { //} balance braces
#else
  if (!capSetCallbackOnVideoStream(hCaptureWindow, VideoHandler)) {
#endif
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapSetCallbackOnVideoStream failed - " << lastError);
    return FALSE;
  }

  WORD devId;
  if (PTrace::CanTrace(6)) { // list available video capture drivers
    PTRACE(5, "PVidInp\tEnumerating available video capture drivers");
    for (devId = 0; devId < 10; devId++) { 
      char name[100];
      char version[200];
      if (capGetDriverDescription(devId, name, sizeof(name), version, sizeof(version)) ) 
        PTRACE(5, "PVidInp\tVideo device[" << devId << "] = " << name << ", " << version);
    }
  }
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

  capSetUserData(hCaptureWindow, this);

  // Use first driver available.
  if (!capDriverConnect(hCaptureWindow, devId)) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapDriverConnect failed - " << lastError);
    return FALSE;
  }

  CAPDRIVERCAPS driverCaps;
  memset(&driverCaps, 0, sizeof(driverCaps));
  if (!capDriverGetCaps(hCaptureWindow, &driverCaps, sizeof(driverCaps))) {
    lastError = ::GetLastError();
    PTRACE(1, "PVidInp\tcapGetDriverCaps failed - " << lastError);
    return FALSE;
  }

  PTRACE(6, "Enumerating CAPDRIVERCAPS values:\n"
            "  driverCaps.wDeviceIndex           = " << driverCaps.wDeviceIndex        << "\n"
            "  driverCaps.fHasOverlay            = " << driverCaps.fHasOverlay         << "\n"
            "  driverCaps.fHasDlgVideoSource     = " << driverCaps.fHasDlgVideoSource  << "\n"
            "  driverCaps.fHasDlgVideoFormat     = " << driverCaps.fHasDlgVideoFormat  << "\n"
            "  driverCaps.fHasDlgVideoDisplay    = " << driverCaps.fHasDlgVideoDisplay << "\n"
            "  driverCaps.fCaptureInitialized    = " << driverCaps.fCaptureInitialized << "\n"
            "  driverCaps.fDriverSuppliesPalettes= " << driverCaps.fDriverSuppliesPalettes);
  
/*
  if (driverCaps.fHasOverlay)
    capOverlay(hCaptureWindow, TRUE);
  else {
    capPreviewRate(hCaptureWindow, 66);
    capPreview(hCaptureWindow, TRUE);
  }
*/
   
  capPreview(hCaptureWindow, FALSE);

#if PTRACING
  if (PTrace::CanTrace(6))
    TestAllFormats(); // list acceptable formats and frame resolutions for video capture driver
#endif
  
  if (!SetFrameRate(frameRate))
    return FALSE;

  if (preferredColourFormat.IsEmpty())
    return SetColourFormat(colourFormat);

  return SetColourFormat(preferredColourFormat);
}


void PVideoInputDevice_VideoForWindows::HandleCapture()
{
  BOOL initSucceeded = InitialiseCapture();

  if (initSucceeded) {
    threadStarted.Signal();

    MSG msg;
    while (::GetMessage(&msg, NULL, 0, 0))
      ::DispatchMessage(&msg);
  }

  PTRACE(5, "PVidInp\tDisconnecting driver");
  capDriverDisconnect(hCaptureWindow);
  capSetUserData(hCaptureWindow, NULL);

  capSetCallbackOnError(hCaptureWindow, NULL);
  capSetCallbackOnVideoStream(hCaptureWindow, NULL);

  PTRACE(5, "PVidInp\tDestroying VIDCAP window");
  DestroyWindow(hCaptureWindow);
  hCaptureWindow = NULL;

  // Signal the other thread we have completed, even if have error
  if (!initSucceeded)
    threadStarted.Signal();
}


// End Of File ///////////////////////////////////////////////////////////////
