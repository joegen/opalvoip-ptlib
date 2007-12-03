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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */


#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#ifdef P_APPSHARE

#include <windows.h>
#include <ptlib/vidinput_app.h>

#if defined(_WIN32) && !defined(P_FORCE_STATIC_PLUGIN)
#error "vidinput_app.cxx must be compiled without precompiled headers"
#endif

//////////////////////////////////////////////////////////////////////////////////////

class PVideoInputDevice_Application_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *   CreateInstance(int /*userData*/) const { return new PVideoInputDevice_Application; }
    virtual PStringList GetDeviceNames(int /*userData*/) const { return PVideoInputDevice_Application::GetInputDeviceNames(); }
    virtual bool GetDeviceCapabilities(const PString & deviceName, void * caps) const
      { return PVideoInputDevice_Application::GetDeviceCapabilities(deviceName, (PVideoInputDevice::Capabilities *)caps); }

} PVideoInputDevice_Application_descriptor;

PCREATE_PLUGIN(Application, PVideoInputDevice, &PVideoInputDevice_Application_descriptor);


//////////////////////////////////////////////////////////////////////////////////////////////////
// Input device

PVideoInputDevice_Application::PVideoInputDevice_Application()
{
    m_client = true;
    preferredColourFormat = "BGR24";  
}


PStringList PVideoInputDevice_Application::GetInputDeviceNames()
{
    return PString("Application");
}

PBoolean PVideoInputDevice_Application::GetDeviceCapabilities(const PString & /*deviceName*/, Capabilities * /*caps*/)  
{ 
    return false; 
}

void CaptureScreenToBYTEArray(LPRECT lpRect, PBYTEArray & array)
{
    HDC         hScrDC, hMemDC;         // screen DC and memory DC     
    int         nX, nY, nX2, nY2;       // coordinates of rectangle to grab     
    int         nWidth, nHeight;        // DIB width and height     
    int         xScrn, yScrn;           // screen resolution      

    HGDIOBJ     hOldBitmap , hBitmap;
        
        // check for an empty rectangle 
    if (IsRectEmpty(lpRect))       
       return;      
       // create a DC for the screen and create     
       // a memory DC compatible to screen DC          

   hScrDC = CreateDC("DISPLAY", NULL, NULL, NULL);     
   hMemDC = CreateCompatibleDC(hScrDC);      // get points of rectangle to grab  
   
   nX = lpRect->left;     
   nY = lpRect->top;     
   nX2 = lpRect->right;     
   nY2 = lpRect->bottom;      // get screen resolution      
   
   xScrn = GetDeviceCaps(hScrDC, HORZRES);     
   yScrn = GetDeviceCaps(hScrDC, VERTRES);      
   
   //make sure bitmap rectangle is visible      
   
   if (nX < 0)         
      nX = 0;     
   
   if (nY < 0)         
      nY = 0;     
   
   if (nX2 > xScrn)         
      nX2 = xScrn;     
   
   if (nY2 > yScrn)         
      nY2 = yScrn;      

   nWidth = nX2 - nX;     
   nHeight = nY2 - nY;      
   
   // create a bitmap compatible with the screen DC     
   
   hBitmap = CreateCompatibleBitmap(hScrDC, nWidth, nHeight);      
   
   // select new bitmap into memory DC     
   
   hOldBitmap =   SelectObject (hMemDC, hBitmap);      
   
   // bitblt screen DC to memory DC     
   
   BitBlt(hMemDC, 0, 0, nWidth, nHeight, hScrDC, nX, nY, SRCCOPY);     
   
   // select old bitmap back into memory DC and get handle to     
   // bitmap of the screen          
   
   hBitmap = SelectObject(hMemDC, hOldBitmap);    

   // now we have a screencapture to bitmap
   BITMAPINFO    bmpInfo;
   bmpInfo.bmiHeader.biBitCount=0;
   bmpInfo.bmiHeader.biSize=sizeof(BITMAPINFOHEADER);
   GetDIBits(hMemDC,(HBITMAP)hBitmap,0,0,NULL,&bmpInfo,DIB_RGB_COLORS);
   bmpInfo.bmiHeader.biCompression=BI_RGB;

   // Store the data
   array = PBYTEArray((BYTE *)&bmpInfo, bmpInfo.bmiHeader.biSize);

   // clean up      
   DeleteDC(hScrDC);     
   DeleteDC(hMemDC);      
}

void PVideoInputDevice_Application::AttachCaptureWindow(HWND _hwnd, bool _client)
{
    m_hWnd = _hwnd;
    m_client = _client;
}

PBoolean PVideoInputDevice_Application::Open(const PString & /*DeviceName*/, PBoolean /*startImmediate*/)
{
    Close();

    if (!m_hWnd) {
        PTRACE(4,"APP/tOpen Fail no Window to capture specified!");
        return false;
    }

    RECT _rect;
    ::GetWindowRect(m_hWnd,&_rect);
     PVideoDevice::SetFrameSize(_rect.right,_rect.bottom);

    return true;
}

PBoolean PVideoInputDevice_Application::IsOpen()
{
    return false;
}

PBoolean PVideoInputDevice_Application::Close()
{
    if (!IsOpen())
        return false;

    return true;
}

PBoolean PVideoInputDevice_Application::Start()
{
    return false;
}

PBoolean PVideoInputDevice_Application::Stop()
{
    return false;
}

PBoolean PVideoInputDevice_Application::IsCapturing()
{
    return false;
}

PBoolean PVideoInputDevice_Application::SetColourFormat(
      const PString & /*ColourFormat*/ 
    )
{
     return true;
}

PBoolean PVideoInputDevice_Application::SetFrameRate(
      unsigned Rate 
    )
{
    return PVideoDevice::SetFrameRate(Rate);
}

PBoolean PVideoInputDevice_Application::SetFrameSize(
      unsigned /*Width*/,   
      unsigned /*Height*/   
    )
{
    PTRACE(4,"APP/tFrame size cannot be set! Must be detected from Application!");
    return true;
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

PBoolean PVideoInputDevice_Application::GetFrameDataNoDelay(
      BYTE * buffer,                 
      PINDEX * bytesReturned  
    )
{

   PWaitAndSignal m(lastFrameMutex);

   PTRACE(6,"AppInput\tGrabbing Frame");

   // Get the current application window
   RECT _rect;
   if (!m_client) {
     ::GetWindowRect(m_hWnd,&_rect);
     if ((_rect.right != (signed)frameWidth) ||
        (_rect.bottom != (signed)frameHeight))
          PVideoDevice::SetFrameSize(_rect.right,_rect.bottom);
   } else {
     ::GetClientRect(m_hWnd,&_rect);
     if ((_rect.right != (signed)frameWidth) ||
        (_rect.bottom != (signed)frameHeight))
          PVideoDevice::SetFrameSize(_rect.right,_rect.bottom);

       POINT pt1,pt2;
         pt1.x = _rect.left;             
         pt1.y = _rect.top;             
         pt2.x = _rect.right;             
         pt2.y = _rect.bottom;             
         ::ClientToScreen(m_hWnd,&pt1);             
         ::ClientToScreen(m_hWnd,&pt2);             
         _rect.left = pt1.x;             
         _rect.top = pt1.y;             
         _rect.right = pt2.x;             
         _rect.bottom = pt2.y;  
   }

    PBYTEArray frame;
    CaptureScreenToBYTEArray(&_rect,frame);

    bool retval = false;
    long pBufferSize = frame.GetSize();
    BYTE * pBuffer = frame.GetPointer();
    
   PTRACE(6,"AppInput\tBuffer obtained." << pBufferSize );
   if (pBuffer != NULL) {
  
    // Convert the image for output
      if (NULL != converter) {
         retval = converter->Convert(pBuffer,buffer, bytesReturned);
         PTRACE(6,"AppInput\tBuffer converted." << *bytesReturned );
      } else {

         PTRACE(6,"AppInput\tBuffer copied." << pBufferSize );
         memcpy(buffer, pBuffer, pBufferSize);
         if (buffer != NULL)
           *bytesReturned = pBufferSize;
        retval = true;
      }
   }

  PTRACE(6,"App\tBuffer Transcoded "  << retval);
  
  return retval;  
}


PBoolean PVideoInputDevice_Application::TestAllFormats()
{
    return true;
}

PBoolean PVideoInputDevice_Application::SetChannel(int /*newChannel*/)
{

  return true;
}

#endif  // P_APPSHARE
