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
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *		   Thorsten Westheider (thorsten.westheider@teleos-web.de)
 *
 * $Log: vconvert.cxx,v $
 * Revision 1.3  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
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

///////////////////////////////////////////////////////////////////////////////
// PColourConverter

PColourConverterRegistration::PColourConverterRegistration(const PString & srcColourFormat,
                                                           const PString & destColourFormat)
  : PString(srcColourFormat+'\t'+destColourFormat)
{
  PColourConverter::converters.Register(this);
}


PColourConverterRegistrations::PColourConverterRegistrations()
{
  DisallowDeleteObjects();
}


void PColourConverterRegistrations::Register(PColourConverterRegistration * reg)
{
  if (GetValuesIndex(*reg) ==P_MAX_INDEX)
    Append(reg);
}


PColourConverterRegistrations PColourConverter::converters;

PColourConverter * PColourConverter::Create(const PString & srcColourFormat,
                                            const PString & destColourFormat,
                                            unsigned width,
                                            unsigned height)
{
  PINDEX idx = converters.GetValuesIndex(srcColourFormat+'\t'+destColourFormat);
  if (idx == P_MAX_INDEX)
    return NULL;
  return ((PColourConverterRegistration&)converters[idx]).Create(width, height);
}


PColourConverter::PColourConverter(const PString & src,
                                   const PString & dst,
                                   unsigned width,
                                   unsigned height)
  : srcColourFormat(src),
    dstColourFormat(dst)
{
  SetFrameSize(width, height);
}


BOOL PColourConverter::SetFrameSize(unsigned width, unsigned height)
{
  frameWidth = width;
  frameHeight = height;
  srcFrameBytes = PVideoDevice::CalculateFrameBytes(frameWidth, frameHeight, srcColourFormat);
  dstFrameBytes = PVideoDevice::CalculateFrameBytes(frameWidth, frameHeight, dstColourFormat);
  return srcFrameBytes != 0 && dstFrameBytes != 0;
}


BOOL PColourConverter::ConvertInPlace(BYTE * frameBuffer, BOOL noIntermediateFrame)
{
  if (Convert(frameBuffer, frameBuffer))
    return TRUE;

  if (noIntermediateFrame)
    return FALSE;

  BYTE * intermediate = intermediateFrameStore.GetPointer(dstFrameBytes);
  if (!Convert(frameBuffer, intermediate))
    return FALSE;

  memcpy(frameBuffer, intermediate, dstFrameBytes);
  return TRUE;
}


#define rgbtoyuv(b, g, r, y, u, v) \
  y=(BYTE)(((int)30*r  +(int)59*g +(int)11*b)/100); \
  u=(BYTE)(((int)-17*r  -(int)33*g +(int)50*b+12800)/100); \
  v=(BYTE)(((int)50*r  -(int)42*g -(int)8*b+12800)/100); \


static void RGBtoYUV411(unsigned width, unsigned height,
                        const BYTE * rgb, BYTE * yuv,
                        unsigned rgbIncrement)
{
  int planeSize = width*height;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);

  for (int y = height-1; y >=0; y--) 
  {
     BYTE * yline  = yplane + (y * width);
     BYTE * uline  = uplane + ((y >> 1) * (width >> 1));
     BYTE * vline  = vplane + ((y >> 1) * (width >> 1));

    for (unsigned x = 0; x < width; x+=2) 
    {
     rgbtoyuv(rgb[0], rgb[1], rgb[2],*yline, *uline, *vline);
     rgb += rgbIncrement;
     yline++;
     rgbtoyuv(rgb[0], rgb[1], rgb[2],*yline, *uline, *vline);
     rgb += rgbIncrement;
     yline++;
     uline++;
     vline++;
    }
  }
}


PCOLOUR_CONVERTER(RGB24toYUV411,"RGB24","YUV411P")
{
  RGBtoYUV411(frameWidth, frameHeight, srcFrameBuffer, dstFrameBuffer, 3);
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PCOLOUR_CONVERTER(RGB32toYUV411,"RGB32","YUV411P")
{
  RGBtoYUV411(frameWidth, frameHeight, srcFrameBuffer, dstFrameBuffer, 4);
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PCOLOUR_CONVERTER(Yuv422ToYuv411p,"YUV422","YUV411P")
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  unsigned  a,b;
  BYTE *u,*v;
  const BYTE * s = srcFrameBuffer;
  BYTE * y =  dstFrameBuffer;
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

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


#define LIMIT(x) (unsigned char) (((x > 0xffffff) ? 0xff0000 : ((x <= 0xffff) ? 0 : x & 0xff0000)) >> 16)

PCOLOUR_CONVERTER(Yuv411ToRgb32,"YUV411","RGB32")
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  unsigned int   nbytes    = frameWidth*frameHeight;
  const BYTE    *yplane    = srcFrameBuffer;           		// 1 byte Y (luminance) for each pixel
  const BYTE    *uplane    = yplane+nbytes;              	// 1 byte U for a block of 4 pixels
  const BYTE    *vplane    = uplane+(nbytes >> 2);       	// 1 byte V for a block of 4 pixels
  unsigned int   pixpos[4] = { 0, 1, frameWidth, frameWidth+1 };
  unsigned int   x, y, p;

  long     int   yvalue;
  long     int   cr, cb, rd, gd, bd;
  long     int   l, r, g, b;
            
  for (y = 0; y < frameHeight; y += 2)
  {
    for (x = 0; x < frameWidth; x += 2)
    {
      // The RGB value without luminance
     
      cr = *uplane-128;
      cb = *vplane-128;
      rd = 104635*cb;			// 		  106986*cb
      gd = -25690*cr-53294*cb;		// -26261*cr  +   -54496*cb 
      bd = 132278*cr;			// 135221*cr
     
      // Add luminance to each of the 4 pixels
      
      for (p = 0; p < 4; p++)
      {
        yvalue = *(yplane+pixpos[p])-16;
        
        if (yvalue < 0) yvalue = 0;
        
        l = 76310*yvalue;
        
        r = l+rd;
        g = l+gd;
        b = l+bd;
        
        *(dstFrameBuffer+4*pixpos[p])   = LIMIT(b);
        *(dstFrameBuffer+4*pixpos[p]+1) = LIMIT(g);
        *(dstFrameBuffer+4*pixpos[p]+2) = LIMIT(r);
        *(dstFrameBuffer+4*pixpos[p]+3) = 0;
      }
      
      yplane += 2;
      dstFrameBuffer += 8;
      
      uplane++;
      vplane++;
    }

    yplane += frameWidth;
    dstFrameBuffer += 4*frameWidth;  
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PCOLOUR_CONVERTER(Rgb24ToRgb32,"RGB24","RGB32")
{
  // Go from bottom to top so can do in place conversion
  const BYTE * src = srcFrameBuffer+srcFrameBytes-1;
  BYTE * dst = dstFrameBuffer+dstFrameBytes-1;

  for (unsigned x = 0; x < frameWidth; x++) {
    for (unsigned y = 0; y < frameHeight; y++) {
      *dst-- = 0;
      for (unsigned p = 0; p < 3; p++)
        *dst-- = *src--;
    }
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
