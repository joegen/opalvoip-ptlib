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
 * Revision 1.17  2001/08/22 02:06:17  robertj
 * Resolved confusion with YUV411P and YUV420P video formats, thanks Mark Cooke.
 *
 * Revision 1.16  2001/08/20 07:01:26  robertj
 * Fixed wierd problems with YUV411P and YUV420P formats, thanks Mark Cooke.
 *
 * Revision 1.15  2001/08/16 23:17:29  robertj
 * Added 420P to 411P converter, thanks Mark Cooke.
 *
 * Revision 1.14  2001/08/03 10:13:56  robertj
 * Changes to previous check in to support MSVC.
 *
 * Revision 1.13  2001/08/03 04:21:51  dereks
 * Add colour/size conversion for YUV422->YUV411P
 * Add Get/Set Brightness,Contrast,Hue,Colour for PVideoDevice,  and
 * Linux PVideoInputDevice.
 * Add lots of PTRACE statement for debugging colour conversion.
 * Add support for Sony Vaio laptop under linux. Requires 2.4.7 kernel.
 *
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
  PTRACE(3,"PColourConverter\t Create Require "<<converterName);

  PColourConverterRegistration * find = RegisteredColourConvertersListHead;
  while (find != NULL) {
    PTRACE(3,"PColourConverter\tCreate test for "<< *find);
    if (*find == converterName) {
      PTRACE(3,"PColourConverter\t converter exists for "<<*find);
      return find->Create(width, height);
    }
    find = find->link;
  }

  PTRACE(3,"PColourConverter::\t Create Error. Did not find "<<converterName);
  return NULL;
}


PColourConverter::PColourConverter(const PString & src,
                                   const PString & dst,
                                   unsigned width,
                                   unsigned height)
  : srcColourFormat(src),
    dstColourFormat(dst)
{
  PTRACE(3,"PColourconverter constructor. "<<src<<"->"<<dst<<" "<<width<<"x"<<height);
  SetFrameSize(width,height);
}


BOOL PColourConverter::SetFrameSize(unsigned width, unsigned height)
{
  BOOL ok1 = SetSrcFrameSize(width, height);
  BOOL ok2 = SetDstFrameSize(width, height, FALSE);
  PTRACE(3,"PColourConverter::SetFrameSize "<<width<<"x"<<height << ( ( ok1 && ok2 ) ? " OK" : " Failed" ) );
  return ok1 && ok2;
}


BOOL PColourConverter::SetSrcFrameSize(unsigned width, unsigned height)
{
  srcFrameWidth = width;
  srcFrameHeight = height;
  srcFrameBytes = PVideoDevice::CalculateFrameBytes(srcFrameWidth, srcFrameHeight, srcColourFormat);

  PTRACE(3,"PColourConvert::SetSrcFrameSize "<< ( (srcFrameBytes != 0) ? "Succeeded, ": " Failed" ) 
                       <<srcFrameWidth<<"x"<<srcFrameHeight<<"-->"<<srcFrameBytes);

  return srcFrameBytes != 0;
}


BOOL PColourConverter::SetDstFrameSize(unsigned width, unsigned height,
					  BOOL bScale)
{
  dstFrameWidth  = width;
  dstFrameHeight = height;
  scaleNotCrop   = bScale;
  
  dstFrameBytes = PVideoDevice::CalculateFrameBytes(dstFrameWidth, dstFrameHeight, dstColourFormat);

  PTRACE(3,"PColourConvert::SetDstFrameSize "<< ( (dstFrameBytes != 0) ? "Succeeded, ": " Failed" ) 
                       <<dstFrameWidth<<"x"<<dstFrameHeight<<"-->"<<dstFrameBytes);

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
  if ((srcFrameWidth != dstFrameWidth) || (srcFrameHeight != dstFrameHeight))
    return FALSE;
  
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

static void RGBtoYUV420p(unsigned width, unsigned height,
                         const BYTE * rgb, BYTE * yuv,
                         unsigned rgbIncrement)
{
  const unsigned planeSize = width*height;
  const unsigned halfWidth = width >> 1;
  
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
static void RGBtoYUV420pWithResize(unsigned swidth, unsigned sheight, const BYTE * rgb,
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


PSTANDARD_COLOUR_CONVERTER(RGB24,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;
  
  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    RGBtoYUV420p(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 3);
  else
    RGBtoYUV420pWithResize(srcFrameWidth, srcFrameHeight, srcFrameBuffer,
                           dstFrameWidth, dstFrameHeight, dstFrameBuffer, 3);
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(RGB32,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;
  
  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    RGBtoYUV420p(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 4);
  else
    RGBtoYUV420pWithResize(srcFrameWidth, srcFrameHeight, srcFrameBuffer,
                           dstFrameWidth, dstFrameHeight, dstFrameBuffer, 4);
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


// Consider a YUV422P image of 8x2 pixels.
//
// A plane of Y values    A B C D E F G H
//                        I J K L M N O P
//
// A plane of U values    1 . 2 . 3 . 4 .
//                        5 . 6 . 7 . 8 .
//
// A plane of V values    1 . 2 . 3 . 4 .
//                        5 . 6 . 7 . 8 .
//
// Simple crop/pad version.  
// Image cropped / padded with black borders as required.
//
static void Yuv422ToYuv422WithResize(unsigned swidth, unsigned sheight, const BYTE * src,
                                     unsigned dwidth, unsigned dheight, BYTE * dest)
{
  if ( (dwidth*dheight) > (swidth*sheight) ) { 
    //     destination is bigger than source. ADD border.
    unsigned maxIndex = 2*dwidth*dheight;
    for (unsigned i = 0; i < maxIndex; i+=2) {
      dest[i]  = BLACK_Y;
      dest[i+1]= BLACK_U;
    }

    unsigned yOffset = dheight - sheight;
    unsigned xOffset = dwidth - swidth;
    if ( (yOffset<0) || (xOffset<0) ) {
      PTRACE(1,"YUV422 to YUV422. Err. dest src size mismatch");
      memset(dest,64,(dwidth*dheight*2));
      return;
    }
    
    BYTE *s_ptr,*d_ptr;
    d_ptr = (yOffset * dwidth) + xOffset + dest;
    s_ptr = (BYTE *)src;
    for (unsigned y = 0; y < sheight; y++) {
      memcpy(d_ptr,s_ptr, swidth*2);
      d_ptr += 2*dwidth;
      s_ptr += 2*swidth;
    }
  } else {  
    // source is bigger than the destination. Remove the
    // appropriate border from the source.
    unsigned yOffset = sheight - dheight;
    unsigned xOffset = swidth - dwidth;
    if ( (yOffset<0) || (xOffset<0) ) {
      PTRACE(1,"YUV422 to YUV422. Err. srce dest size mismatch");
      memset(dest,64,(dwidth*dheight*2));
      return;
    }

    BYTE *s_ptr,*d_ptr;
    d_ptr = dest;
    s_ptr = (yOffset * swidth) + xOffset + (BYTE *)src;
    for (unsigned y = 0; y < dheight; y++) {
      memcpy(d_ptr,s_ptr, dwidth*2);
      d_ptr += dwidth*2;
      s_ptr += swidth*2;
    }
  }
}


PSTANDARD_COLOUR_CONVERTER(YUV422,YUV422)
{
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  
  if (srcFrameBuffer == dstFrameBuffer)
    return TRUE;
  
  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    memcpy(dstFrameBuffer,srcFrameBuffer,srcFrameWidth*srcFrameHeight*2);
  else
    Yuv422ToYuv422WithResize(srcFrameWidth, srcFrameHeight, (BYTE *)srcFrameBuffer,
                             dstFrameWidth, dstFrameHeight, dstFrameBuffer);
  
  return TRUE;
}



///No resize here.
//Colour format change only, YUV422 is turned into YUV420P.
static void Yuv422ToYuv420P(unsigned dstFrameWidth, unsigned dstFrameHeight, 
                            const BYTE * srcFrame, BYTE * dstFrame)
{
  unsigned  a,b;
  BYTE *u,*v;
  const BYTE * s =  srcFrame;
  BYTE * y =  dstFrame;

  u = y + (dstFrameWidth * dstFrameHeight);
  v = u + (dstFrameWidth * dstFrameHeight / 4);

  for (a = 0; a < dstFrameHeight; a+=2) {
    for (b = 0; b < dstFrameWidth; b+=2) {
      *(y++) = *(s++);
      *(u++) = *(s++);
      *(y++) = *(s++);
      *(v++) = *(s++);
    }
    for (b = 0; b < dstFrameWidth; b+=2) {
      *(y++) = *(s++);
      s++;
      *(y++) = *(s++);
      s++;
    }
  }
}


PSTANDARD_COLOUR_CONVERTER(YUV422,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  if ((srcFrameWidth==dstFrameWidth) && (srcFrameHeight==dstFrameHeight))
    Yuv422ToYuv420P(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer);
  else {
    //do a resize.  then convert to yuv420p.
    BYTE * intermed = intermediateFrameStore.GetPointer(dstFrameWidth*dstFrameHeight*2);

    Yuv422ToYuv422WithResize(srcFrameWidth, srcFrameHeight, srcFrameBuffer,
                             dstFrameWidth, dstFrameHeight, intermed);
    Yuv422ToYuv420P(dstFrameWidth, dstFrameHeight, intermed, dstFrameBuffer);
  }

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


#define LIMIT(x) (unsigned char) (((x > 0xffffff) ? 0xff0000 : ((x <= 0xffff) ? 0 : x & 0xff0000)) >> 16)

static void YUV420PtoRGB(unsigned srcFrameWidth, unsigned srcFrameHeight,
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


PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB24)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  YUV420PtoRGB(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 3);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB32)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  YUV420PtoRGB(srcFrameWidth, srcFrameHeight, srcFrameBuffer, dstFrameBuffer, 4);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(RGB24,RGB32)
{
  if ((dstFrameWidth != srcFrameWidth) || (dstFrameHeight != srcFrameHeight))
    return FALSE;
  
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
  if ((dstFrameWidth != srcFrameWidth) || (dstFrameHeight != srcFrameHeight))
    return FALSE;
  
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


// Consider a YUV420P image of 8x2 pixels.
//
// A plane of Y values    A B C D E F G H
//                        I J K L M N O P
//
// A plane of U values    1   2   3   4 
// A plane of V values    1   2   3   4 ....
//
// The U1/V1 samples correspond to the ABIJ pixels.
//     U2/V2 samples correspond to the CDKL pixels.
//
// Consider a YUV411P image of 8x2 pixels.
//
// A plane of Y values as before.
//
// A plane of U values    1       2
//                        3       4
//
// A plane of V values    1       2
//                        3       4
//
// The U1/V1 samples correspond to the ABCD pixels.
//     U2/V2 samples correspond to the EFGH pixels.
//
// I choose to reoganize the U and V samples by using
// using U1 for ABCD, U3 for EFGH, U2 for IJKL, U4 for MNOP
//
// Possibly discarding U2/U4 completely, or using the
// average of U1 and U2 might be easier for compression
//
// TODO:
//
// - Inplace converter
// - Resizing / padding / scaling converter
//
PSTANDARD_COLOUR_CONVERTER(YUV420P,YUV411P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  if ((dstFrameWidth != srcFrameWidth) || (dstFrameHeight != srcFrameHeight))
    return FALSE;
  
  // Copy over the Y plane.
  memcpy(dstFrameBuffer, srcFrameBuffer, srcFrameWidth*srcFrameHeight);
  
  unsigned linewidth = dstFrameWidth / 4;
  
  // Source data is the start of the U plane
  const BYTE* src = srcFrameBuffer + srcFrameWidth * srcFrameHeight;
  
  // Two output lines at a time
  BYTE *dst0 = dstFrameBuffer + dstFrameWidth * dstFrameHeight;
  BYTE *dst1 = dst0 + linewidth;

  // U plane
  for (unsigned y = 0; y < dstFrameHeight; y += 2) {
    for (unsigned x = 0; x < dstFrameWidth; x += 4) {
      
      *dst0++ = *src++;
      *dst1++ = *src++;
    }

    // Skip over the 2nd line we already did.
    dst0 += linewidth;
    dst1 = dst0 + linewidth;
  }
  
  // Source data is the start of the U plane
  src = srcFrameBuffer + srcFrameWidth * srcFrameHeight * 5 / 4;
  
  // Two output lines at a time
  dst0 = dstFrameBuffer + dstFrameWidth * dstFrameHeight * 5 / 4;
  dst1 = dst0 + linewidth;
  
  // V plane
  for (unsigned y = 0; y < dstFrameHeight; y += 2) {
    for (unsigned x = 0; x < dstFrameWidth; x += 4) {
      
      *dst0++ = *src++;
      *dst1++ = *src++;
    }

    // Skip over the 2nd line we already did.
    dst0 += linewidth;
    dst1 = dst0 + linewidth;
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  
  return TRUE;
}


// YUV411P to YUV420P conversion
//
// Consider YUV411P U plane (. = pixel) :
//
// A... B... C... D...
// E... F... G... H...
// I... J... K... L...
// M... N... O... P...
//
// We map this to a YUV420P plane by
// discarding odd rows, and doubling up
// the even row samples:
//
// A.A. B.B. C.C. D.D.
// .... .... .... ....
// I.I. J.J. K.K. L.L.
// .... .... .... ....
//
// TODO:
//
// - Inplace converter
// - Resizing / padding / scaling converter
//
PSTANDARD_COLOUR_CONVERTER(YUV411P,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  if ((dstFrameWidth != srcFrameWidth) || (dstFrameHeight != srcFrameHeight))
    return FALSE;
  
  // Copy over the Y plane.
  memcpy(dstFrameBuffer, srcFrameBuffer, srcFrameWidth*srcFrameHeight);
  
  unsigned linewidth = dstFrameWidth / 4;
  
  // Source data is the start of the U plane
  const BYTE* src = srcFrameBuffer + srcFrameWidth * srcFrameHeight;
  
  // Output line
  BYTE *dst0 = dstFrameBuffer + dstFrameWidth * dstFrameHeight;

  // U plane
  for (unsigned y = 0; y < dstFrameHeight; y += 2) {
    for (unsigned x = 0; x < dstFrameWidth; x += 4) {
      
      // Double up the horizontal samples
      *dst0++ = *src;
      *dst0++ = *src++;
    }
    
    // Skip over the 2nd line we are decimating
    src += linewidth;
  }
  
  // Source data is the start of the U plane
  src = srcFrameBuffer + srcFrameWidth * srcFrameHeight * 5 / 4;
  
  // Output line
  dst0 = dstFrameBuffer + dstFrameWidth * dstFrameHeight * 5 / 4;
  
  // V plane
  for (unsigned y = 0; y < dstFrameHeight; y += 2) {
    for (unsigned x = 0; x < dstFrameWidth; x += 4) {
      
      // Double up the samples horizontal samples
      *dst0++ = *src;
      *dst0++ = *src++;
    }

    // Skip over the 2nd source line we already did.
    src += linewidth;
  }
  
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  
  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(YUV420P,YUV420P)
{
  return SimpleConvert(srcFrameBuffer, dstFrameBuffer, bytesReturned);
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
