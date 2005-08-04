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
 * Revision 1.27.6.3  2005/08/04 08:10:50  rjongbloed
 * Fixed problem where if SetFrameSize() is called and window is already that size, the window is not brought to the front.
 *
 * Revision 1.27.6.2  2005/07/24 07:23:57  rjongbloed
 * Fixed correct use of ShowWindow() return value in video window device Start()
 *
 * Revision 1.27.6.1  2005/07/17 09:27:08  rjongbloed
 * Major revisions of the PWLib video subsystem including:
 *   removal of F suffix on colour formats for vertical flipping, all done with existing bool
 *   working through use of RGB and BGR formats so now consistent
 *   cleaning up the plug in system to use virtuals instead of pointers to functions.
 *   rewrite of SDL to be a plug in compatible video output device.
 *   extensive enhancement of video test program
 *
 * Revision 1.27  2005/01/04 07:44:04  csoutheren
 * More changes to implement the new configuration methodology, and also to
 * attack the global static problem
 *
 * Revision 1.26  2004/10/23 10:50:52  ykiryanov
 * Added ifdef _WIN32_WCE for PocketPC 2003 SDK port
 *
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

#define P_FORCE_STATIC_PLUGIN

#include <ptlib.h>

#if defined(_WIN32) && !defined(P_FORCE_STATIC_PLUGIN)
#error "vfw.cxx must be compiled without precompiled headers"
#endif

#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#ifndef _WIN32_WCE
#pragma comment(lib, "vfw32.lib")
#endif

#define STEP_GRAB_CAPTURE 1


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

    /**Set the colour format to be used.
       Note that this function does not do any conversion. If it returns TRUE
       then the video device does the colour format in native mode.

       To utilise an internal converter use the SetColourFormatConverter()
       function.

       Default behaviour sets the value of the colourFormat variable and then
       returns TRUE.
    */
    virtual BOOL SetColourFormat(
      const PString & colourFormat // New colour format for device.
    );

    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       returns TRUE.
    */
    virtual BOOL SetFrameRate(
      unsigned rate  /// Frames  per second
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       returns TRUE.
    */
    virtual BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();

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

    PDECLARE_NOTIFIER(PThread, PVideoInputDevice_VideoForWindows, HandleCapture);

    static LRESULT CALLBACK ErrorHandler(HWND hWnd, int id, LPCSTR err);
    LRESULT HandleError(int id, LPCSTR err);
    static LRESULT CALLBACK VideoHandler(HWND hWnd, LPVIDEOHDR vh);
    LRESULT HandleVideo(LPVIDEOHDR vh);
    BOOL InitialiseCapture();

    PThread     * captureThread;
    PSyncPoint    threadStarted;

    HWND          hCaptureWindow;

    PSyncPoint    frameAvailable;
    LPBYTE        lastFramePtr;
    unsigned      lastFrameSize;
    PMutex        lastFrameMutex;
    BOOL          isCapturingNow;
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
    BOOL ApplyFormat(HWND hWnd, const char * coloutFormat);

    BITMAPINFO * operator->() const 
    { return (BITMAPINFO *)theArray; }
};

///////////////////////////////////////////////////////////////////////////////

static struct { 
  const char * colourFormat; 
  WORD  bitCount;
  BOOL  negHeight; // MS documentation suggests that negative height will request
                  // top down scan direction from video driver
                  // HOWEVER, not all drivers honor this request
  DWORD compression; 
} FormatTable[] = {
  { "BGR32",   32, TRUE,  BI_RGB },
  { "BGR24",   24, TRUE,  BI_RGB },
  { "Grey",     8, TRUE,  BI_RGB },
  { "Gray",     8, TRUE,  BI_RGB },

  { "RGB565",  16, TRUE,  BI_BITFIELDS },
  { "RGB555",  15, TRUE,  BI_BITFIELDS },

  // http://support.microsoft.com/support/kb/articles/q294/8/80.asp
  { "YUV420P", 12, FALSE, mmioFOURCC('I','Y','U','V') },
  { "IYUV",    12, FALSE, mmioFOURCC('I','Y','U','V') }, // Synonym for IYUV
  { "I420",    12, FALSE, mmioFOURCC('I','4','2','0') }, // Synonym for IYUV
  { "YV12",    12, FALSE, mmioFOURCC('Y','V','1','2') }, // same as IYUV except that U and V planes are switched
  { "YUV422",  16, FALSE, mmioFOURCC('Y','U','Y','2') },
  { "YUY2",    16, FALSE, mmioFOURCC('Y','U','Y','2') },
  { "UYVY",    16, FALSE, mmioFOURCC('U','Y','V','Y') }, // Like YUY2 except for ordering
  { "YVYU",    16, FALSE, mmioFOURCC('Y','V','Y','U') }, // Like YUY2 except for ordering
  { "YVU9",    16, FALSE, mmioFOURCC('Y','V','U','9') },
  { "MJPEG",   12, FALSE, mmioFOURCC('M','J','P','G') },
  { NULL },
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

PVideoDeviceBitmap::PVideoDeviceBitmap(HWND hCaptureWindow, WORD bpp)
{
  PINDEX sz = capGetVideoFormatSize(hCaptureWindow);
  SetSize(sz);
  if (!capGetVideoFormat(hCaptureWindow, theArray, sz)) { 
    PTRACE(1, "PVidInp\tcapGetVideoFormat(hCaptureWindow) failed - " << ::GetLastError());
    SetSize(0);
    return;
  }

  if (8 == bpp && bpp != ((BITMAPINFO*)theArray)->bmiHeader.biBitCount) {
    SetSize(sizeof(BITMAPINFOHEADER)+sizeof(RGBQUAD)*256);
    RGBQUAD * bmiColors = ((BITMAPINFO*)theArray)->bmiColors;
    for (int i = 0; i < 256; i++)
      bmiColors[i].rgbBlue  = bmiColors[i].rgbGreen = bmiColors[i].rgbRed = (BYTE)i;
  }
  ((BITMAPINFO*)theArray)->bmiHeader.biBitCount = bpp;
}

BOOL PVideoDeviceBitmap::ApplyFormat(HWND hWnd, const char * colourFormat)
{
  // NB it is necessary to set the biSizeImage value appropriate to frame size
  // assume bmiHeader.biBitCount has already been set appropriatly for format
  BITMAPINFO & bmi = *(BITMAPINFO*)theArray;
  bmi.bmiHeader.biPlanes = 1;
  bmi.bmiHeader.biSizeImage = 
    (bmi.bmiHeader.biHeight<0 ? -bmi.bmiHeader.biHeight : bmi.bmiHeader.biHeight)
                  *4*((bmi.bmiHeader.biBitCount * bmi.bmiHeader.biWidth + 31)/32);

  if (capSetVideoFormat(hWnd, theArray, GetSize())) {
    PTRACE(3, "PVidInp\tcapSetVideoFormat succeeded: "
            << colourFormat << ' '
            << bmi.bmiHeader.biWidth << "x" << bmi.bmiHeader.biHeight
            << " sz=" << bmi.bmiHeader.biSizeImage);
    return TRUE;
  }

  PTRACE(1, "PVidInp\tcapSetVideoFormat failed: "
          << colourFormat << ' '
          << bmi.bmiHeader.biWidth << "x" << bmi.bmiHeader.biHeight
          << " sz=" << bmi.bmiHeader.biSizeImage
          << " - lastError=" << ::GetLastError());
  return FALSE;
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
// PVideoInputDevice_VideoForWindows

PCREATE_VIDINPUT_PLUGIN(VideoForWindows);

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

  captureThread = PThread::Create(PCREATE_NOTIFIER(HandleCapture), 0,
                                  PThread::NoAutoDeleteThread, PThread::NormalPriority,
                                  "VidIn:%x");
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

  bool applied = false;

  if (FormatTable[i].colourFormat != NULL)
    bi->bmiHeader.biCompression = FormatTable[i].compression;
  else if (colourFmt.GetLength() == 4)
    bi->bmiHeader.biCompression = mmioFOURCC(colourFmt[0],colourFmt[1],colourFmt[2],colourFmt[3]);
  else {
    bi->bmiHeader.biCompression = 0xffffffff; // Indicate invalid colour format
    return FALSE;
  }

  // set frame width and height
  bi->bmiHeader.biWidth = frameWidth;
  if (FormatTable[i].negHeight) {
    // See if driver will do top down format native
    bi->bmiHeader.biHeight = -(int)frameHeight;
    applied = bi.ApplyFormat(hCaptureWindow, colourFormat);
    if (!applied) {
      // Wont do top down, try bottom up, telling everuthing we are up side down
      nativeVerticalFlip = true;
      bi->bmiHeader.biHeight = frameHeight;
      applied = bi.ApplyFormat(hCaptureWindow, colourFormat);
    }
  }
  else {
    bi->bmiHeader.biHeight = frameHeight;
    applied = bi.ApplyFormat(hCaptureWindow, colourFormat);
  }

  if (!applied) {
    lastError = ::GetLastError();
    PVideoDevice::SetColourFormat(oldFormat);
    return FALSE;
  }

  PTRACE(3, "PVidInp\tcapSetVideoFormat succeeded: "
         << colourFormat << ' '
         << bi->bmiHeader.biWidth << "x" << bi->bmiHeader.biHeight
         << " sz=" << bi->bmiHeader.biSizeImage);

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

  bi->bmiHeader.biWidth = nativeVerticalFlip ? width : -(int)width;
  bi->bmiHeader.biHeight = height;

  if (!bi.ApplyFormat(hCaptureWindow, colourFormat)) {
    lastError = ::GetLastError();
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


BOOL PVideoInputDevice_VideoForWindows::TestAllFormats()
{
  BOOL running = IsCapturing();
  if (running)
    Stop();

  for (PINDEX prefFormatIdx = 0; FormatTable[prefFormatIdx].colourFormat != NULL; prefFormatIdx++) {
    PVideoDeviceBitmap bi(hCaptureWindow, FormatTable[prefFormatIdx].bitCount); 
    bi->bmiHeader.biCompression = FormatTable[prefFormatIdx].compression;
    for (PINDEX prefResizeIdx = 0; prefResizeIdx < PARRAYSIZE(winTestResTable); prefResizeIdx++) {
      bi->bmiHeader.biWidth = winTestResTable[prefResizeIdx].device_width;

      // set .biHeight according to .negHeight value
      if (FormatTable[prefFormatIdx].negHeight) {
        bi->bmiHeader.biHeight = -(int)winTestResTable[prefResizeIdx].device_height; 
        bi.ApplyFormat(hCaptureWindow, FormatTable[prefFormatIdx].colourFormat);
      }

      bi->bmiHeader.biHeight = winTestResTable[prefResizeIdx].device_height;
      bi.ApplyFormat(hCaptureWindow, FormatTable[prefFormatIdx].colourFormat);
    } // for prefResizeIdx
  } // for prefFormatIdx

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

  return GetMaxFrameBytesConverted(PVideoDeviceBitmap(hCaptureWindow)->bmiHeader.biSizeImage);
}


BOOL PVideoInputDevice_VideoForWindows::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


BOOL PVideoInputDevice_VideoForWindows::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  if (!frameAvailable.Wait(1000))
    return FALSE;

  bool retval = false;

  lastFrameMutex.Wait();

  if (lastFramePtr != NULL) {
    if (NULL != converter)
      retval = converter->Convert(lastFramePtr, buffer, bytesReturned);
    else {
      memcpy(buffer, lastFramePtr, lastFrameSize);
      if (bytesReturned != NULL)
        *bytesReturned = lastFrameSize;
      retval = true;
    }
  }

  lastFrameMutex.Signal();

#if STEP_GRAB_CAPTURE
  if (isCapturingNow)
    capGrabFrameNoStop(hCaptureWindow);
#endif

  return retval;
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
      {
        PTRACE(5, "PVidInp\tVideo device[" << devId << "] = " << name << ", " << version);
      }
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


void PVideoInputDevice_VideoForWindows::HandleCapture(PThread &, INT)
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


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice_Window

/**This class defines a video output device for RGB in a frame store.
 */
class PVideoOutputDevice_Window : public PVideoOutputDeviceRGB
{
  PCLASSINFO(PVideoOutputDevice_Window, PVideoOutputDeviceRGB);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice_Window();

    /** Destroy a video output device.
     */
    ~PVideoOutputDevice_Window();

    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name (filename base) to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine if the device is currently open.
      */
    virtual BOOL IsOpen();

    /**Close the device.
      */
    virtual BOOL Close();

    /**Start the video device I/O display.
      */
    virtual BOOL Start();

    /**Stop the video device I/O display.
      */
    virtual BOOL Stop();

    /**Get a list of all of the devices available.
      */
    static PStringList GetOutputDeviceNames();

    /**Get a list of all of the devices available.
      */
    virtual PStringList GetDeviceNames() const
    { return GetOutputDeviceNames(); }

    /**Set the colour format to be used.
       Note that this function does not do any conversion. If it returns TRUE
       then the video device does the colour format in native mode.

       To utilise an internal converter use the SetColourFormatConverter()
       function.

       Default behaviour sets the value of the colourFormat variable and then
       returns TRUE.
    */
    virtual BOOL SetColourFormat(
      const PString & colourFormat // New colour format for device.
    );

    /**Get the video conversion vertical flip state.
       Default action is to return FALSE.
     */
    virtual BOOL GetVFlipState();

    /**Set the video conversion vertical flip state.
       Default action is to return FALSE.
     */
    virtual BOOL SetVFlipState(
      BOOL newVFlipState    /// New vertical flip state
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       returns TRUE.
    */
    virtual BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Set a section of the output frame buffer.
      */
    virtual BOOL FrameComplete();

    LRESULT WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

  protected:
    PDECLARE_NOTIFIER(PThread, PVideoOutputDevice_Window, HandleDisplay);
    void Draw(HDC hDC);

    HWND       m_hWnd;
    PThread  * m_thread;
    PSyncPoint m_started;
    BITMAPINFO m_bitmap;
    bool       m_flipped;
};


static bool ParseWindowDeviceName(const PString & deviceName, DWORD * dwStylePtr = NULL, HWND * hWndParentPtr = NULL)
{
  if (deviceName.Find("MSWIN") != 0)
    return false;

  PINDEX pos = deviceName.Find("STYLE=");
  if (pos == P_MAX_INDEX) {
    PTRACE(2, "VidOut\tDevice name must specify STYLE=.");
    return false;
  }

  DWORD dwStyle = strtoul(((const char *)deviceName)+pos+6, NULL, 0);

  HWND hWndParent = NULL;
  pos = deviceName.Find("PARENT=");
  if (pos != P_MAX_INDEX) {
    hWndParent = (HWND)strtoul(((const char *)deviceName)+pos+5, NULL, 0);
    if (!::IsWindow(hWndParent)) {
      PTRACE(2, "VidOut\tIllegal parent window " << hWndParent << " specified.");
      hWndParent = NULL;
    }
  }

  // Have parsed out style & parent, see if legal combination
  if ((dwStyle&(WS_POPUP|WS_CHILD)) == 0) {
    PTRACE(2, "VidOut\tWindow must be WS_POPUP or WS_CHILD window.");
    return false;
  }

  if (hWndParent == NULL && (dwStyle&WS_POPUP) == 0) {
    PTRACE(2, "VidOut\tWindow must be WS_POPUP if parent window not specified.");
    return false;
  }

  if (dwStylePtr != NULL)
    *dwStylePtr = dwStyle;

  if (hWndParentPtr != NULL)
    *hWndParentPtr = hWndParent;

  return true;
}


class PVideoOutputDevice_Window_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *   CreateInstance(int /*userData*/) const { return new PVideoOutputDevice_Window; }
    virtual PStringList GetDeviceNames(int /*userData*/) const { return PVideoOutputDevice_Window::GetOutputDeviceNames(); }
    virtual bool        ValidateDeviceName(const PString & deviceName, int /*userData*/) const { return ParseWindowDeviceName(deviceName); }
} PVideoOutputDevice_Window_descriptor;

PCREATE_PLUGIN(Window, PVideoOutputDevice, &PVideoOutputDevice_Window_descriptor);


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDeviceRGB

PVideoOutputDevice_Window::PVideoOutputDevice_Window()
{
  m_hWnd = NULL;
  m_thread = NULL;
  m_flipped = FALSE;

  m_bitmap.bmiHeader.biSize = sizeof(m_bitmap.bmiHeader);
  m_bitmap.bmiHeader.biWidth = frameWidth;
  m_bitmap.bmiHeader.biHeight = -(int)frameHeight;
  m_bitmap.bmiHeader.biPlanes = 1;
  m_bitmap.bmiHeader.biBitCount = 32;
  m_bitmap.bmiHeader.biCompression = BI_RGB;
  m_bitmap.bmiHeader.biXPelsPerMeter = 0;
  m_bitmap.bmiHeader.biYPelsPerMeter = 0;
  m_bitmap.bmiHeader.biClrImportant = 0;
  m_bitmap.bmiHeader.biClrUsed = 0;
  m_bitmap.bmiHeader.biSizeImage = frameStore.GetSize();
}


PVideoOutputDevice_Window::~PVideoOutputDevice_Window()
{
  Close();
}


PStringList PVideoOutputDevice_Window::GetOutputDeviceNames()
{
  PStringList deviceList;
  deviceList.AppendString(psprintf("MSWIN STYLE=0x%08X TITLE=\"Video Output\"", WS_POPUP|WS_BORDER|WS_SYSMENU|WS_CAPTION));
  return deviceList;
}


BOOL PVideoOutputDevice_Window::Open(const PString & name, BOOL /*startImmediate*/)
{
  Close();

  deviceName = name;

  m_thread = PThread::Create(PCREATE_NOTIFIER(HandleDisplay), 0,
                             PThread::NoAutoDeleteThread, PThread::NormalPriority,
                             "VidOut:%x");

  m_started.Wait();

  return m_hWnd != NULL;
}


BOOL PVideoOutputDevice_Window::IsOpen()
{
  return m_hWnd != NULL;
}


BOOL PVideoOutputDevice_Window::Close()
{
  HWND hWnd;
  {
    PWaitAndSignal m(mutex);
    if (m_hWnd == NULL)
      return FALSE;

    hWnd = m_hWnd;
    m_hWnd = NULL;
  }

  SendMessage(hWnd, WM_CLOSE, 0, 0);
  m_thread->WaitForTermination(3000);
  delete m_thread;
  return TRUE;
}


BOOL PVideoOutputDevice_Window::Start()
{
  if (m_hWnd == NULL)
    return FALSE;
  
  ShowWindow(m_hWnd, SW_SHOW);
  return TRUE;
}


BOOL PVideoOutputDevice_Window::Stop()
{
  if (m_hWnd != NULL)
    return ShowWindow(m_hWnd, SW_HIDE);

  return FALSE;
}


BOOL PVideoOutputDevice_Window::SetColourFormat(const PString & colourFormat)
{
  PWaitAndSignal m(mutex);

  if (((colourFormat *= "BGR24") || (colourFormat *= "BGR32")) &&
                PVideoOutputDeviceRGB::SetColourFormat(colourFormat)) {
    m_bitmap.bmiHeader.biBitCount = (WORD)(bytesPerPixel*8);
    m_bitmap.bmiHeader.biSizeImage = frameStore.GetSize();
    return TRUE;
  }

  return FALSE;
}


BOOL PVideoOutputDevice_Window::GetVFlipState()
{
  return m_flipped;
}


BOOL PVideoOutputDevice_Window::SetVFlipState(BOOL newVFlip)
{
  m_flipped = newVFlip;
  m_bitmap.bmiHeader.biHeight = m_flipped ? frameHeight : -(int)frameHeight;
  return TRUE;
}


BOOL PVideoOutputDevice_Window::SetFrameSize(unsigned width, unsigned height)
{
  {
    PWaitAndSignal m(mutex);

    if (width == frameWidth && height == frameHeight)
      return TRUE;

    if (!PVideoOutputDeviceRGB::SetFrameSize(width, height))
      return FALSE;

    m_bitmap.bmiHeader.biWidth = frameWidth;
    m_bitmap.bmiHeader.biHeight = m_flipped ? frameHeight : -(int)frameHeight;
    m_bitmap.bmiHeader.biSizeImage = frameStore.GetSize();
  }

  // Must be outside of mutex
  if (m_hWnd != NULL) {
    RECT rect;
    rect.top = 0;
    rect.left = 0;
    rect.bottom = frameHeight;
    rect.right = frameWidth;
    AdjustWindowRect(&rect, GetWindowLong(m_hWnd, GWL_STYLE), FALSE);
    ::SetWindowPos(m_hWnd, NULL, 0, 0, rect.right-rect.left, rect.bottom-rect.top, SWP_NOMOVE|SWP_NOZORDER);
  }

  return TRUE;
}


BOOL PVideoOutputDevice_Window::FrameComplete()
{
  PWaitAndSignal m(mutex);

  if (m_hWnd == NULL)
    return FALSE;

  HDC hDC = GetDC(m_hWnd);
  Draw(hDC);
  ReleaseDC(m_hWnd, hDC);

  return TRUE;
}


void PVideoOutputDevice_Window::Draw(HDC hDC)
{
  RECT rect;
  GetClientRect(m_hWnd, &rect);

  int result;
  if (frameWidth == (unsigned)rect.right && frameHeight == (unsigned)rect.bottom)
    result = SetDIBitsToDevice(hDC,
                               0, 0, frameWidth, frameHeight,
                               0, 0, 0, frameHeight,
                               frameStore.GetPointer(), &m_bitmap, DIB_RGB_COLORS);
  else
    result = StretchDIBits(hDC,
                           0, 0, rect.right, rect.bottom,
                           0, 0, frameWidth, frameHeight,
                           frameStore.GetPointer(), &m_bitmap, DIB_RGB_COLORS, SRCCOPY);

  if (result == 0) {
    lastError = ::GetLastError();
    PTRACE(2, "VidOut\tDrawing image failed, error=" << lastError);
  }
}


static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  if (uMsg == WM_CREATE)
    SetWindowLong(hWnd, 0, (LONG)((LPCREATESTRUCT)lParam)->lpCreateParams);

  PVideoOutputDevice_Window * vodw = (PVideoOutputDevice_Window *)GetWindowLong(hWnd, 0);
  if (vodw != NULL)
    return vodw->WndProc(uMsg, wParam, lParam);

  return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void PVideoOutputDevice_Window::HandleDisplay(PThread &, INT)
{
  static const char wndClassName[] = "PVideoOutputDevice_Window";

  static bool needRegistration = true;
  if (needRegistration) {
    needRegistration = false;

    WNDCLASS wndClass;
    memset(&wndClass, 0, sizeof(wndClass));
    wndClass.style = CS_HREDRAW|CS_VREDRAW|CS_DBLCLKS;
    wndClass.lpszClassName = wndClassName;
    wndClass.lpfnWndProc = ::WndProc;
    wndClass.cbWndExtra = sizeof(this);
    PAssertOS(RegisterClass(&wndClass));
  }

  DWORD dwStyle;
  HWND hParent;
  if (ParseWindowDeviceName(deviceName, &dwStyle, &hParent)) {
    PString title;
    PINDEX pos = deviceName.Find("TITLE=");
    if (pos != P_MAX_INDEX)
      title = PString(PString::Literal, &deviceName[pos+6]);

    int x = CW_USEDEFAULT;
    pos = deviceName.Find("X=");
    if (pos != P_MAX_INDEX)
      x = atoi(&deviceName[pos+2]);

    int y = CW_USEDEFAULT;
    pos = deviceName.Find("Y=");
    if (pos != P_MAX_INDEX)
      y = atoi(&deviceName[pos+2]);

    m_hWnd = CreateWindow(wndClassName, title, dwStyle,
                          x, y, frameWidth, frameHeight,
                          hParent, NULL, GetModuleHandle(NULL), this);
  }

  m_started.Signal();

  if (m_hWnd != NULL) {
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
      DispatchMessage(&msg);
  }
}


LRESULT PVideoOutputDevice_Window::WndProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  PWaitAndSignal m(mutex);

  switch (uMsg) {
    case WM_PAINT :
      {
        PAINTSTRUCT paint;
        HDC hDC = BeginPaint(m_hWnd, &paint);
        Draw(hDC);
        EndPaint(m_hWnd, &paint);
        break;
      }

    case WM_CLOSE :
      DestroyWindow(m_hWnd);
      m_hWnd = NULL;
      break;

    case WM_DESTROY:
      PostThreadMessage(GetCurrentThreadId(), WM_QUIT, 0, 0);
      break;
  }
  return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}


// End Of File ///////////////////////////////////////////////////////////////
