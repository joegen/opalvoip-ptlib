/* vidinput_app.cxx
 *
 *
 * Application Implementation for the PTLib Project.
 *
 * Copyright (c) 2007 ISVO (Asia) Pte Ltd. All Rights Reserved.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 *
 *
 * Contributor(s): Craig Southeren, Post Increment (C) 2008
 */


#include <ptlib.h>

#if P_APPSHARE

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/vconvert.h>
#include <ptclib/delaychan.h>
#include <ptlib/msos/ptlib/vidinput_app.h>



static const PConstCaselessString DesktopWindowName("Desktop");
static const PConstCaselessString TopWindowPrefix("TopWindow:");

static HWND FindTopWindow(const PString & deviceName)
{
  if (DesktopWindowName == deviceName)
    return GetDesktopWindow();

  if (TopWindowPrefix.NumCompare(deviceName, TopWindowPrefix.GetLength()) == PObject::EqualTo)
    return ::FindWindow(NULL, deviceName.Mid(TopWindowPrefix.GetLength()));

  return NULL;
}


/* Convert bitmap colour format to format name 
  colourFormat will contains colour format name on success, an empty string if unsupported format
  return true on succes, false otherwise 
*/
static bool GetBitmapColourFormat(BITMAP bitmap, PString & colourFormat) {
	switch (bitmap.bmBitsPixel) {
	case 32:
		colourFormat = "BGR32";
		break;
	case 24:
		colourFormat = "BGR24";
		break;
	default:
		PTRACE(2, "AppInput\tUnsupported pixel format");
		colourFormat = "";
		return false;
	}

	return true;
}


//////////////////////////////////////////////////////////////////////
// Video Input device

PCREATE_VIDINPUT_PLUGIN_EX(Application,

  virtual const char * GetFriendlyName() const
  {
    return "Application Window Video Grabber";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    return FindTopWindow(deviceName) != NULL;
  }
);


//////////////////////////////////////////////////////////////////////////////////////////////////
// Input device

PVideoInputDevice_Application::PVideoInputDevice_Application()
  : m_hWnd(NULL)
  , m_client(false)
{
  SetColourFormat("BGR32");
  SetFrameRate(5);
}


PVideoInputDevice_Application::~PVideoInputDevice_Application()
{
  Close();
}


PBoolean PVideoInputDevice_Application::Close()
{
  if (!IsOpen())
    return false;

  m_hWnd = NULL;
  return true;
}


PStringArray PVideoInputDevice_Application::GetDeviceNames() const
{ 
  return GetInputDeviceNames(); 
}


static BOOL CALLBACK AddWindowName(HWND hWnd, LPARAM userData)
{
  if (IsWindowVisible(hWnd)) {
    int len = GetWindowTextLengthW(hWnd);
    if (len > 0) {
      PWideString wideName;
      if (GetWindowTextW(hWnd, wideName.GetPointer(len+1), len+1))
        reinterpret_cast<PStringArray *>(userData)->AppendString(TopWindowPrefix + wideName);
    }
  }
  return TRUE;
}


PStringArray PVideoInputDevice_Application::GetInputDeviceNames()
{
  PStringArray names;

  names += DesktopWindowName;

  ::EnumWindows(AddWindowName, (LPARAM)&names);

  return names;
}


PBoolean PVideoInputDevice_Application::GetDeviceCapabilities(const PString & /*deviceName*/, Capabilities * /*caps*/)  
{ 
  return false; 
}


PBoolean PVideoInputDevice_Application::Open(const PString & deviceName, PBoolean /*startImmediate*/)
{
  Close();

  m_client = false;

  if ((m_hWnd = FindTopWindow(deviceName)) == NULL) {
    PTRACE(4,"AppInput/tCannot open specified window");
    return false;
  }

  BITMAP bitmap;
  if (GetWindowBitmap(bitmap)) {
    PString bmpColourFmt;
    if (!GetBitmapColourFormat(bitmap, bmpColourFmt)) 
	  return false;
	return PVideoDevice::SetColourFormat(bmpColourFmt);
  }

  m_hWnd = NULL;
  return false;
}


PBoolean PVideoInputDevice_Application::IsOpen()
{
  return m_hWnd != NULL;
}


PBoolean PVideoInputDevice_Application::Start()
{
  return true;
}


PBoolean PVideoInputDevice_Application::Stop()
{
  return true;
}


PBoolean PVideoInputDevice_Application::IsCapturing()
{
  return IsOpen();
}


PBoolean PVideoInputDevice_Application::SetColourFormat(const PString & colourFormat)
{
  BITMAP bitmap;

  if (GetWindowBitmap(bitmap)) {
	PString bmpColourFmt;
	if (!GetBitmapColourFormat(bitmap, bmpColourFmt)) 
      return false;
    if (!(colourFormat *= bmpColourFmt))
      return false;
  }
  else {
    if (!((colourFormat *= "BGR32") || (colourFormat *= "BGR24")))
      return false;
  }

  return PVideoDevice::SetColourFormat(colourFormat);
}


PINDEX PVideoInputDevice_Application::GetMaxFrameBytes()
{
  PINDEX size;
  BITMAP bitmap;
  if (GetWindowBitmap(bitmap))
    size = bitmap.bmHeight*bitmap.bmWidthBytes;
  else
    size = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);

  return GetMaxFrameBytesConverted(size);
}


PBoolean PVideoInputDevice_Application::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  m_grabDelay.Delay(1000/GetFrameRate());
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


PBoolean PVideoInputDevice_Application::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  PWaitAndSignal mutex(m_lastFrameMutex);

  BITMAP bitmap;
  if (converter == NULL) {
    if (GetWindowBitmap(bitmap, buffer)) {
      if (bytesReturned != NULL)
        *bytesReturned = bitmap.bmHeight*bitmap.bmWidthBytes;
      return true;
    }
  }
  else {
    if (GetWindowBitmap(bitmap, NULL, true)) {
      converter->SetSrcFrameSize(frameWidth, frameHeight);
      if (converter->Convert(m_tempPixelBuffer, buffer, bytesReturned))
        return true;
    }

    PTRACE(2, "AppInput\tConverter failed");
  }

  return false;
}


void PVideoInputDevice_Application::AttachCaptureWindow(HWND hWnd, bool client)
{
  m_hWnd = hWnd;
  m_client = client;
}


struct PWrapHDC
{
  HDC m_handle;
  PWrapHDC(HDC h) : m_handle(h) { }
  ~PWrapHDC() { DeleteDC(m_handle); }
  operator HDC() const { return m_handle; }
};


struct PWrapHGDIOBJ
{
  HGDIOBJ m_handle;
  PWrapHGDIOBJ(HGDIOBJ h) : m_handle(h) { }
  ~PWrapHGDIOBJ() { DeleteObject(m_handle); }
  operator HGDIOBJ() const { return m_handle; }
  operator HBITMAP() const { return (HBITMAP)m_handle; }
};


bool PVideoInputDevice_Application::GetWindowBitmap(BITMAP & bitmap, BYTE * pixels, bool useTemp)
{
  if (m_hWnd == NULL) {
    PTRACE(2, "AppInput\tNo window selected.");
    return false;
  }

  RECT rect;

  // Get the client area of the window
  if (m_client) {

    ::GetClientRect(m_hWnd, &rect);

    POINT pt1;
    pt1.x = rect.left;
    pt1.y = rect.top;
    ::ClientToScreen(m_hWnd, &pt1);

    POINT pt2;
    pt2.x = rect.right;             
    pt2.y = rect.bottom;             
    ::ClientToScreen(m_hWnd, &pt2);

    rect.left   = pt1.x;             
    rect.top    = pt1.y;             
    rect.right  = pt2.x;             
    rect.bottom = pt2.y;  
  }
  else
  {
    ::GetWindowRect(m_hWnd, &rect);
  }

  if (IsRectEmpty(&rect)) {
    PTRACE(2, "AppInput\tZero sized window");
    return false;
  }

  // get width and height of grab region
  unsigned width  = rect.right - rect.left;
  unsigned height = rect.bottom - rect.top;

  // create a DC for the screen and create
  PWrapHDC hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);

  // Get the frame size;
  unsigned framewidth, frameheight;
  GetFrameSize(framewidth, frameheight);

   // create a bitmap compatible with the screen DC
  PWrapHGDIOBJ hBitMap = CreateCompatibleBitmap(hScrDC, framewidth, frameheight);

  // a memory DC compatible to screen DC
  PWrapHDC hMemDC = CreateCompatibleDC(hScrDC);

  // select new bitmap into memory DC
  SelectObject(hMemDC, hBitMap);

  // bitblt or stretchblt screen DC to memory DC depending on frame size
  if (width == framewidth && height == frameheight)
    BitBlt(hMemDC, 0, 0, width, height, hScrDC, rect.left, rect.top, SRCCOPY);
  else {
    // Get a better quality scale
    SetStretchBltMode(hMemDC, HALFTONE);
    SetBrushOrgEx(hMemDC, 0, 0, NULL);
    StretchBlt(hMemDC, 0, 0, framewidth, frameheight, hScrDC, rect.left, rect.top, width, height, SRCCOPY);
  }

  // get the bitmap information
  if (GetObject(hBitMap, sizeof(BITMAP), (LPSTR)&bitmap) == 0) {
    PTRACE(2, "AppInput\tCould not get bitmap information");
    return false;
  }

  if (pixels != NULL && ((unsigned)bitmap.bmWidth > frameWidth || (unsigned)bitmap.bmHeight > frameHeight)) {
    PTRACE(2, "AppInput\tWindow increased in size, buffer may not be large enough.");
    return true;
  }

  SetFrameSize(bitmap.bmWidth, bitmap.bmHeight);

  // create a BITMAPINFO with enough room for the pixel data
  unsigned bitmapInfoSize = sizeof(BITMAPINFOHEADER);

  // check pixel format is supported
  PString bmpColourFmt;
  if (!GetBitmapColourFormat(bitmap, bmpColourFmt)) 
	return false;

  LPBITMAPINFO bitmapInfo = (LPBITMAPINFO)_alloca(bitmapInfoSize);

  BITMAPINFOHEADER & bi = bitmapInfo->bmiHeader;
  memset(&bi, 0, sizeof(bi));
  bi.biSize        = sizeof(BITMAPINFOHEADER);
  bi.biWidth       =  bitmap.bmWidth;
  bi.biHeight      = -bitmap.bmHeight;
  bi.biBitCount    =  bitmap.bmBitsPixel;
  bi.biPlanes      = 1;
  bi.biCompression = BI_RGB;

  if (useTemp)
    pixels = m_tempPixelBuffer.GetPointer(bitmap.bmHeight*bitmap.bmWidthBytes);

  // get the pixel data
  if (GetDIBits(hMemDC,
                hBitMap,
                0, bitmap.bmHeight,
                pixels,
                bitmapInfo,
                DIB_RGB_COLORS) == 0) {
    PTRACE(2, "AppInput\tFailed to get bitmap image from window");
    return false;
  }

  // Clean up DCs
  DeleteDC(hScrDC);
  DeleteDC(hMemDC);

  return true;
}
#endif  // P_APPSHARE
