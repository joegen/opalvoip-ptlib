/*
 * vconvert.cxx
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
 * $Log: vconvert.cxx,v $
 * Revision 1.2  2000/12/19 23:58:14  robertj
 * Fixed MSVC compatibility issues.
 *
 * Revision 1.1  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 */

#include <ptlib.h>
#include <ptlib/vconvert.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoConvert

PVideoConvert::PVideoConvert(PVideoDevice::ColourFormat srcFormat,
                             PVideoDevice::ColourFormat destFormat,
                             unsigned width,
                             unsigned height)
{
  internalBuffer = NULL;
  if ( (width==0) || (height==0) )
    PAssertAlways("Invalid width / height passed to image conversion class");
    
  srcFrameSize = PVideoDevice::CalcFrameSize(width,height,srcFormat);
  destFrameSize = PVideoDevice::CalcFrameSize(width,height,destFormat);
  
  int bufferSize;
  srcFrameSize<destFrameSize ? bufferSize = destFrameSize : bufferSize = srcFrameSize;
  internalBuffer = new BYTE[ bufferSize ];

  frameWidth = width;
  frameHeight = height;
  srcColourFormat = srcFormat;
  destColourFormat = destFormat;
}


BOOL PVideoConvert::Close()
{
  delete internalBuffer;
  return TRUE;
}  
    
BOOL PVideoConvert::ConvertWithCopy(BYTE * src, BYTE * dest)
{
  memcpy(internalBuffer,src,srcFrameSize);
  
  return ConvertDirectly(internalBuffer,dest);
 }
      
BOOL PVideoConvert::ConvertInternalBuffer(BYTE *dest)
{
 return ConvertDirectly(internalBuffer,dest);
}

BOOL PVideoConvert::ConvertDirectly(BYTE * src, BYTE * dest)
{
  if( (srcColourFormat  == (int)PVideoDevice::YUV422) && 
      (destColourFormat == (int)PVideoDevice::YUV411P)   )
    return Yuv422ToYuv411p(src,dest);
    
  return FALSE;
}

BOOL PVideoConvert::Yuv422ToYuv411p(BYTE *src, BYTE * dest)
{
       unsigned  a,b;
       BYTE *s, *y,*u,*v;
       s =  src;
       y =  dest;
       u = y + (frameWidth * frameHeight);
       v = u + (frameWidth * frameHeight / 4);

       for (a = frameHeight; a > 0; a -= 2) {
          for (b = frameWidth; b > 0; b -= 2) {           
             *(y++) = *(s++);
             *(u++) = *(s++);
             *(y++) = *(s++);
             *(v++) = *(s++);
          }
          for (b = frameWidth; b > 0; b -= 2) {
             *(y++) = *(s++);
             s++;
             *(y++) = *(s++);
             s++;
          }
   	  }
	    return TRUE;
}
// End Of File ///////////////////////////////////////////////////////////////
