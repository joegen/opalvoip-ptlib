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
 *		   Mark Cooke (mpc@star.sr.bham.ac.uk)
 *
 * $Log: vconvert.cxx,v $
 * Revision 1.12  2001/07/20 05:23:51  robertj
 * Added YUV411P to RGB24 converter.
 *
 * Revision 1.11  2001/05/14 05:10:38  robertj
 * Fixed problems with video colour converters registration, could not rely
 *   on static PList being initialised before all registration instances.
 *
 * Revision 1.10  2001/03/20 02:21:57  robertj
 * More enhancements from Mark Cooke
 *
 * Revision 1.9  2001/03/08 23:36:03  robertj
 * Added backward compatibility SetFrameSize() function.
 * Added internal SimpleConvert() function for same type converters.
 * Fixed some documentation.
 *
 * Revision 1.8  2001/03/08 08:31:34  robertj
 * Numerous enhancements to the video grabbing code including resizing
 *   infrastructure to converters. Thanks a LOT, Mark Cooke.
 *
 * Revision 1.7  2001/03/07 01:39:56  dereks
 * Fix image flip (top to bottom) in YUV411P to RGB24 conversion
 *
 * Revision 1.6  2001/03/06 23:48:32  robertj
 * Fixed naming convention on video converter classes.
 *
 * Revision 1.5  2001/03/03 23:25:07  robertj
 * Fixed use of video conversion function, returning bytes in destination frame.
 *
 * Revision 1.4  2001/03/03 06:13:01  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
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

#ifdef __GNUC__
#pragma implementation "vconvert.h"
#endif

#include <ptlib/vconvert.h>


static PColourConverterRegistration * RegisteredColourConvertersListHead = NULL;


#define PSTANDARD_COLOUR_CONVERTER(from,to) \
  PCOLOUR_CONVERTER(P_##from##_##to,#from,#to)


///////////////////////////////////////////////////////////////////////////////
// PColourConverter

PColourConverterRegistration::PColourConverterRegistration(const PString & srcColourFormat,
                                                           const PString & destColourFormat)
  : PCaselessString(srcColourFormat+'\t'+destColourFormat)
{
  PColourConverterRegistration * test = RegisteredColourConvertersListHead;
  while (test != NULL) {
    if (*test == *this)
      return;
    test = test->link;
  }

  link = RegisteredColourConvertersListHead;
  RegisteredColourConvertersListHead = this;
}


PColourConverter * PColourConverter::Create(const PString & srcColourFormat,
                                            const PString & destColourFormat,
                                            unsigned width,
                                            unsigned height)
{
  PString converterName = srcColourFormat + '\t' + destColourFormat;

  PColourConverterRegistration * find = RegisteredColourConvertersListHead;
  while (find != NULL) {
    if (*find == converterName)
      return find->Create(width, height);
    find = find->link;
  }

  return NULL;
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
  BOOL ok1 = SetSrcFrameSize(width, height);
  BOOL ok2 = SetDstFrameSize(width, height, FALSE);
  return ok1 && ok2;
}


BOOL PColourConverter::SetSrcFrameSize(unsigned width, unsigned height)
{
  srcFrameWidth = width;
  srcFrameHeight = height;
  srcFrameBytes = PVideoDevice::CalculateFrameBytes(srcFrameWidth, srcFrameHeight, srcColourFormat);
  return srcFrameBytes != 0;
}


BOOL PColourConverter::SetDstFrameSize(unsigned width, unsigned height,
					  BOOL bScale)
{
  dstFrameWidth  = width;
  dstFrameHeight = height;
  scaleNotCrop   = bScale;
  
  dstFrameBytes = PVideoDevice::CalculateFrameBytes(dstFrameWidth, dstFrameHeight, dstColourFormat);
  return dstFrameBytes != 0;
}


BOOL PColourConverter::GetSrcFrameSize(unsigned &width, unsigned &height) const
{
    width = srcFrameWidth;
    height = srcFrameHeight;
    return TRUE;
}


BOOL PColourConverter::GetDstFrameSize(unsigned &width, unsigned &height) const
{
    width = dstFrameWidth;
    height = dstFrameHeight;
    return TRUE;
}


BOOL PColourConverter::ConvertInPlace(BYTE * frameBuffer,
                                      PINDEX * bytesReturned,
                                      BOOL noIntermediateFrame)
{
  if (Convert(frameBuffer, frameBuffer, bytesReturned))
    return TRUE;

  if (noIntermediateFrame)
    return FALSE;

  BYTE * intermediate = intermediateFrameStore.GetPointer(dstFrameBytes);
  PINDEX bytes;
  if (!Convert(frameBuffer, intermediate, &bytes))
    return FALSE;

  memcpy(frameBuffer, intermediate, bytes);
  if (bytesReturned != NULL)
    *bytesReturned = bytes;
  return TRUE;
}


BOOL PColourConverter::SimpleConvert(const BYTE * srcFrameBuffer,
                                     BYTE * dstFrameBuffer,
                                     PINDEX * bytesReturned)
{
  if (srcFrameBuffer != dstFrameBuffer)
    memcpy(dstFrameBuffer, srcFrameBuffer, dstFrameBytes);
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}



#define rgbtoyuv(b, g, r, y, u, v) \
  y=(BYTE)(((int)30*r  +(int)59*g +(int)11*b)/100); \
  u=(BYTE)(((int)-17*r  -(int)33*g +(int)50*b+12800)/100); \
  v=(BYTE)(((int)50*r  -(int)42*g -(int)8*b+12800)/100); \


static void RGBtoYUV411p(unsigned width, unsigned height,
                        const BYTE * rgb, BYTE * yuv,
                        unsigned rgbIncrement)
{
  int planeSize = width*height;
  const int halfWidth = width >> 1;
  
  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);

  for (unsigned y = 0; y < height; y++) 
  {
    BYTE * yline  = yplane + (y * width);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

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

#define BLACK_Y 0
#define BLACK_U 128
#define BLACK_V 128

// Simple crop/pad version.  Image aligned to top-left
// and cropped / padded with black borders as required.
static void RGBtoYUV411pWithResize(
    unsigned swidth, unsigned sheight, const BYTE * rgb,
    unsigned dwidth, unsigned dheight, BYTE * yuv,
    unsigned rgbIncrement)
{
  int planeSize = dwidth*dheight;
  const int halfWidth = dwidth >> 1;
  unsigned min_width, min_height;
  
  min_width  = (dwidth  < swidth)  ? dwidth  : swidth;
  min_height = (dheight < sheight) ? dheight : sheight;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);

  for (unsigned y = 0; y < min_height; y++) 
  {
    BYTE * yline  = yplane + (y * dwidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    for (unsigned x = 0; x < min_width; x+=2) 
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

    // Crop if source width > dest width
    if (swidth > dwidth)
      rgb += rgbIncrement * (swidth - dwidth);

    // Pad if dest width < source width
    if (dwidth > swidth) {
      memset(yline, BLACK_Y, dwidth - swidth);
      memset(uline, BLACK_U, (dwidth - swidth)>>1);
      memset(vline, BLACK_V, (dwidth - swidth)>>1);
    }
  }

  // Pad if dest height > source height
  if (dheight > sheight) {
    BYTE * yline  = yplane + (sheight * dwidth);
    BYTE * uline  = uplane + ((sheight >> 1) * halfWidth);
    BYTE * vline  = vplane + ((sheight >> 1) * halfWidth);
    unsigned fill = (dheight - sheight) * dwidth;

    memset(yline, BLACK_Y, fill);
    memset(uline, BLACK_U, fill >> 2);
    memset(vline, BLACK_V, fill >> 2);
  }
}

PSTANDARD_COLOUR_CONVERTER(RGB24,YUV411P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;
  
  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    RGBtoYUV411p(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 3);
  else
    RGBtoYUV411pWithResize(srcFrameWidth, srcFrameHeight, srcFrameBuffer,
                           dstFrameWidth, dstFrameHeight, dstFrameBuffer, 3);
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(RGB32,YUV411P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;
  
  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    RGBtoYUV411p(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 4);
  else
    RGBtoYUV411pWithResize(srcFrameWidth, srcFrameHeight, srcFrameBuffer,
                           dstFrameWidth, dstFrameHeight, dstFrameBuffer, 4);
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(YUV422,YUV411P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  unsigned  a,b;
  BYTE *u,*v;
  const BYTE * s = srcFrameBuffer;
  BYTE * y =  dstFrameBuffer;
  u = y + (srcFrameWidth * srcFrameHeight);
  v = u + (srcFrameWidth * srcFrameHeight / 4);

  for (a = srcFrameHeight; a > 0; a -= 2) {
    for (b = srcFrameWidth; b > 0; b -= 2) {           
      *(y++) = *(s++);
      *(u++) = *(s++);
      *(y++) = *(s++);
      *(v++) = *(s++);
    }
    for (b = srcFrameWidth; b > 0; b -= 2) {
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

static void YUV411PtoRGB(unsigned srcFrameWidth, unsigned srcFrameHeight,
                         const BYTE * srcFrameBuffer, BYTE * dstFrameBuffer,
                         unsigned rgbIncrement)
{
  unsigned int   nbytes    = srcFrameWidth*srcFrameHeight;
  const BYTE    *yplane    = srcFrameBuffer;           		// 1 byte Y (luminance) for each pixel
  const BYTE    *uplane    = yplane+nbytes;              	// 1 byte U for a block of 4 pixels
  const BYTE    *vplane    = uplane+(nbytes >> 2);       	// 1 byte V for a block of 4 pixels
  unsigned int   pixpos[4] = { 0, 1, srcFrameWidth, srcFrameWidth+1 };
  unsigned int   x, y, p;

  long     int   yvalue;
  long     int   cr, cb, rd, gd, bd;
  long     int   l, r, g, b;
            
  for (y = 0; y < srcFrameHeight; y += 2)
  {
    for (x = 0; x < srcFrameWidth; x += 2)
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
        
        *(dstFrameBuffer+rgbIncrement*pixpos[p])   = LIMIT(b);
        *(dstFrameBuffer+rgbIncrement*pixpos[p]+1) = LIMIT(g);
        *(dstFrameBuffer+rgbIncrement*pixpos[p]+2) = LIMIT(r);
        if (rgbIncrement == 4)
          *(dstFrameBuffer+4*pixpos[p]+3) = 0;
      }
      
      yplane += 2;
      dstFrameBuffer += rgbIncrement*2;
      
      uplane++;
      vplane++;
    }

    yplane += srcFrameWidth;
    dstFrameBuffer += rgbIncrement*srcFrameWidth;  
  }
}


PSTANDARD_COLOUR_CONVERTER(YUV411P,RGB24)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  YUV411PtoRGB(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 3);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(YUV411P,RGB32)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  YUV411PtoRGB(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 4);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(RGB24,RGB32)
{
  // Go from bottom to top so can do in place conversion
  const BYTE * src = srcFrameBuffer+srcFrameBytes-1;
  BYTE * dst = dstFrameBuffer+dstFrameBytes-1;

  for (unsigned x = 0; x < srcFrameWidth; x++) {
    for (unsigned y = 0; y < srcFrameHeight; y++) {
      *dst-- = 0;
      for (unsigned p = 0; p < 3; p++)
        *dst-- = *src--;
    }
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(RGB32,RGB24)
{
  const BYTE * src = srcFrameBuffer;
  BYTE * dst = dstFrameBuffer;

  for (unsigned x = 0; x < srcFrameWidth; x++) {
    for (unsigned y = 0; y < srcFrameHeight; y++) {
      for (unsigned p = 0; p < 3; p++)
        *dst++ = *src++;
      src++;
    }
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(YUV411P,YUV411P)
{
  return SimpleConvert(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}


PSTANDARD_COLOUR_CONVERTER(RGB24,RGB24)
{
  return SimpleConvert(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}


PSTANDARD_COLOUR_CONVERTER(RGB32,RGB32)
{
  return SimpleConvert(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}


// End Of File ///////////////////////////////////////////////////////////////
