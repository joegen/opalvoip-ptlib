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
 *
 * $Revision$
 * $Author$
 * $Date$
 */


#include <ptlib.h>

#ifdef P_APPSHARE

#include <ptlib/vidinput_app.h>
#include <ptlib/vconvert.h>


//////////////////////////////////////////////////////////////////////////////////////

class PVideoInputDevice_Application_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *    CreateInstance(int /*userData*/) const { return new PVideoInputDevice_Application; }

    virtual PStringArray GetDeviceNames(int /*userData*/) const { return PVideoInputDevice_Application::GetInputDeviceNames(); }

    virtual bool         GetDeviceCapabilities(const PString & deviceName, void * caps) const
      { return PVideoInputDevice_Application::GetDeviceCapabilities(deviceName, (PVideoInputDevice::Capabilities *)caps); }

    virtual bool ValidateDeviceName(const PString & deviceName, int /*userData*/) const
    {
      return (deviceName.Left(10) *= "appwindow:") && (FindWindow(NULL, deviceName.Mid(10)) != NULL);
    }


} PVideoInputDevice_Application_descriptor;

PCREATE_PLUGIN(Application, PVideoInputDevice, &PVideoInputDevice_Application_descriptor);

//////////////////////////////////////////////////////////////////////////////////////////////////
// Input device

PVideoInputDevice_Application::PVideoInputDevice_Application()
{
  m_hWnd                = NULL;
  m_client              = true;

  SetColourFormat("BGR32");
  SetFrameRate(10);
}

PVideoInputDevice_Application::~PVideoInputDevice_Application()
{
  Close();
}

PStringArray PVideoInputDevice_Application::GetDeviceNames() const
{ 
  return GetInputDeviceNames(); 
}

PStringArray PVideoInputDevice_Application::GetInputDeviceNames()
{
  return PString("Application");
}

PBoolean PVideoInputDevice_Application::GetDeviceCapabilities(const PString & /*deviceName*/, Capabilities * /*caps*/)  
{ 
  return false; 
}

PBoolean PVideoInputDevice_Application::Open(const PString & deviceName, PBoolean /*startImmediate*/)
{
  Close();

  m_client = false;

  RECT rect;
  memset(&rect, 0, sizeof(rect));  // needed to avoid compiler warning

  if (m_hWnd == NULL) {
    if (deviceName.Left(10) *= "appwindow:") {
      m_hWnd = FindWindow(NULL, deviceName.Mid(10));
      if (m_hWnd != NULL) {
        ::GetWindowRect(m_hWnd, &rect);
        SetFrameSize(rect.right-rect.left, rect.bottom-rect.top);
      }
    }
  }

  if (m_hWnd == NULL) {
    PTRACE(4,"AppInput/tOpen Fail no Window to capture specified!");
    return false;
  }

  return true;
}

PBoolean PVideoInputDevice_Application::IsOpen()
{
  return m_hWnd != NULL;
}

PBoolean PVideoInputDevice_Application::Close()
{
  if (!IsOpen())
    return false;

  return true;
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

PBoolean PVideoInputDevice_Application::SetVideoFormat(VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat(newFormat);
}

PBoolean PVideoInputDevice_Application::SetColourFormat(const PString & colourFormat)
{
  if ((colourFormat *= "BGR32") || (colourFormat *= "BGR24"))
    return PVideoDevice::SetColourFormat(colourFormat);

  return PFalse;
}

PBoolean PVideoInputDevice_Application::SetFrameRate(unsigned Rate)
{
  return PVideoDevice::SetFrameRate(Rate);
}

PBoolean PVideoInputDevice_Application::SetFrameSize(unsigned width, unsigned height)
{
  return PVideoDevice::SetFrameSize(width, height);
}

PINDEX PVideoInputDevice_Application::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(CalculateFrameBytes(frameWidth, frameHeight, colourFormat));
}

PBoolean PVideoInputDevice_Application::GetFrameData(
      BYTE * buffer,                 
      PINDEX * bytesReturned   
    )
{
    grabDelay.Delay(1000/GetFrameRate());
    return GetFrameDataNoDelay(buffer,bytesReturned);
}

static inline WORD GetNumberOfColours(WORD bitsPerPixel)
{
    // only 1, 4 and 8bpp bitmaps use palettes (well, they could be used with
    // 24bpp ones too but we don't support this as I think it's quite uncommon)
    return (WORD)(bitsPerPixel <= 8 ? 1 << bitsPerPixel : 0);
}

PBoolean PVideoInputDevice_Application::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  PWaitAndSignal m(lastFrameMutex);

  PTRACE(6,"AppInput\tGrabbing Frame");

  RECT _rect;

  // Get the client area of the window
  if (m_client) {

    ::GetClientRect(m_hWnd, &_rect);
    int width  = _rect.right - _rect.left;
    int height = _rect.bottom - _rect.top;
    if ((width != (int)frameWidth) || (height != (signed)frameHeight))
      PVideoDevice::SetFrameSize(width, height);

    POINT pt1;
    pt1.x = _rect.left;
    pt1.y = _rect.top;
    ::ClientToScreen(m_hWnd, &pt1);

    POINT pt2;
    pt2.x = _rect.right;             
    pt2.y = _rect.bottom;             
    ::ClientToScreen(m_hWnd, &pt2);

    _rect.left   = pt1.x;             
    _rect.top    = pt1.y;             
    _rect.right  = pt2.x;             
    _rect.bottom = pt2.y;  
  }
  else
  {
    ::GetWindowRect(m_hWnd, &_rect);
  }

  bool retVal = true;

  if (!IsRectEmpty(&_rect)) {

    // create a DC for the screen and create
    // a memory DC compatible to screen DC
    HDC hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);
    HDC hMemDC = CreateCompatibleDC(hScrDC);

    // get borders of grab area in pixels
    int left   = _rect.left;     
    int top    = _rect.top;     
    int right  = _rect.right;     
    int bottom = _rect.bottom;

    // get size of screen in pixels
    //int xScrn = GetDeviceCaps(hScrDC, HORZRES);
    //int yScrn = GetDeviceCaps(hScrDC, VERTRES);

#if 0
    // make sure bitmap rectangle is visible
    if (left < 0)
      nX = 0;
    if (nY < 0)
      nY = 0;
    if (nX2 > xScrn)
      nX2 = xScrn;
    if (nY2 > yScrn)
      nY2 = yScrn;
#endif

    // get width and height of grab region
    int nWidth  = right - left;
    int nHeight = bottom - top;

    HBITMAP hBitMap;
    {
       // create a bitmap compatible with the screen DC
      HGDIOBJ _hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);
   
      // select new bitmap into memory DC
      HGDIOBJ _hOldBitmap = SelectObject(hMemDC, _hBitmap);
   
      // bitblt screen DC to memory DC     
      BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, left, top, SRCCOPY);
   
      // select old bitmap back into memory DC and get handle to     
      // bitmap of the screen
      hBitMap = (HBITMAP)SelectObject(hMemDC, _hOldBitmap);
    }

    // get the bitmap information
    BITMAP bitmap;
    if (GetObject(hBitMap, sizeof(BITMAP), (LPSTR)&bitmap) == 0) {
      PTRACE(2, "AppInput\tCould not get bitmap information");
      retVal = false;
    }
    else
    {
      // create a BITMAPINFO with enough room for the pixel data
      unsigned pixelOffs = sizeof(BITMAPINFOHEADER) + GetNumberOfColours(bitmap.bmBitsPixel) * sizeof(RGBQUAD);
      LPBITMAPINFO bitmapInfo = (LPBITMAPINFO)bitMapInfoStorage.GetPointer(pixelOffs + (bitmap.bmHeight * bitmap.bmWidthBytes));
      BITMAPINFOHEADER & bi = bitmapInfo->bmiHeader;
      memset(&bi, 0, sizeof(bi));

      bi.biSize        = sizeof(BITMAPINFOHEADER);
      bi.biWidth       = bitmap.bmWidth;
      bi.biHeight      = bitmap.bmHeight;
      bi.biPlanes      = 1;
      bi.biBitCount    = bitmap.bmBitsPixel;
      bi.biCompression = BI_RGB;

      // get the pixel data
      int scanLines = GetDIBits(hMemDC, 
                                (HBITMAP)hBitMap, 
                                0, bitmap.bmHeight, 
                                (char *)bitmapInfo + pixelOffs,
                                bitmapInfo,
                                DIB_RGB_COLORS);

      if (scanLines == 0) {
        PTRACE(2, "AppInput\tFailed to convert image");
      } else {
        int srcPixelSize  = bitmap.bmBitsPixel / 8;
        int dstPixelSize  = (colourFormat == "BGR32") ? 4 : 3;

        // convert from 24/32 bit to 24/32 bit, and invert top to bottom
        BYTE * src = (BYTE *)bitmapInfo + pixelOffs + (bitmap.bmHeight-1) * bitmap.bmWidthBytes;
        BYTE * dst = (converter == NULL) ? buffer : tempPixelBuffer.GetPointer(bitmap.bmHeight*bitmap.bmWidth * dstPixelSize);
        for (long y = 0; y < bitmap.bmHeight; ++y) {
          for (long x = 0; x < bitmap.bmWidth; ++x) {
            memcpy(dst, src, 3);
            if (dstPixelSize == 4)
              *dst++ = 0;
            src += srcPixelSize;
            dst += dstPixelSize;
          }
          src -= bitmap.bmWidth*srcPixelSize + bitmap.bmWidthBytes;
        }
        *bytesReturned = bitmap.bmHeight * bitmap.bmWidth * dstPixelSize;
        if (converter != NULL && !converter->Convert(tempPixelBuffer.GetPointer(), buffer, bytesReturned)) {
          PTRACE(2, "AppInput\tConverter failed");
          retVal = false;
        }
      }
    }

    DeleteObject(hBitMap); 
    DeleteDC(hScrDC);
    DeleteDC(hMemDC);
  }

  /////////////////////////////////////////////////////////////////////

  return retVal;
}


PBoolean PVideoInputDevice_Application::TestAllFormats()
{
    return true;
}

PBoolean PVideoInputDevice_Application::SetChannel(int /*newChannel*/)
{
  return true;
}

void PVideoInputDevice_Application::AttachCaptureWindow(HWND _hwnd, bool _client)
{
    m_hWnd = _hwnd;
    m_client = _client;
}

#endif  // P_APPSHARE
