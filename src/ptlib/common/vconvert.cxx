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
 * Revision 1.39  2005/01/04 07:44:03  csoutheren
 * More changes to implement the new configuration methodology, and also to
 * attack the global static problem
 *
 * Revision 1.38  2004/11/07 06:28:00  rjongbloed
 * Removed warnings about unused parameters in SBGGR8 conversion functions. Left one
 *   in (flip vertical) as it should be implemented but wasn't.
 *
 * Revision 1.37  2004/10/27 09:24:18  dsandras
 * Added patch from Nicola Orru' to convert from SBGGR8 to YUV420P. Thanks!
 *
 * Revision 1.36  2004/09/21 13:01:08  dsandras
 * Added conversion from sbggr to rgb thanks to an anonymous patcher.
 *
 * Revision 1.35  2003/11/23 22:17:35  dsandras
 * Added YUV420P to BGR24 and BGR32 conversion.
 *
 * Revision 1.34  2003/06/14 02:57:36  rjongbloed
 * REmoved redundent parameter, grey scale does not have rgb increment!
 *
 * Revision 1.33  2003/06/09 22:37:24  dereksmithies
 * Fix from Clive Nicolson to make b/w colour conversions work (i.e. grey palette).
 * many thanks!
 *
 * Revision 1.32  2003/04/03 09:28:37  robertj
 * Added reversed RGB byte order versions (BGR24), thanks Damien Sandras
 *
 * Revision 1.31  2003/03/31 11:30:14  rogerh
 * make 'cb' and 'cr' contain the values that their name implies.
 *
 * Revision 1.30  2002/09/01 23:00:05  dereks
 * Fix noise in flipped RGB image. Thanks Alex Phtahov.
 *
 * Revision 1.29  2002/02/26 02:23:21  dereks
 * Reduced verbosity in PTRACE output for when video is enabled.
 *
 * Revision 1.28  2002/02/20 02:37:26  dereks
 * Initial release of Firewire camera support for linux.
 * Many thanks to Ryutaroh Matsumoto <ryutaroh@rmatsumoto.org>.
 *
 * Revision 1.27  2002/02/03 19:55:57  dereks
 * *** empty log message ***
 *
 * Revision 1.26  2002/01/08 01:32:50  robertj
 * Tidied up some PTRACE debug output.
 *
 * Revision 1.25  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
 * Revision 1.24  2001/12/08 00:33:11  robertj
 * Changed some (unsigned int *) to (DWORD *) as the latter is assured to be a
 *   pointer to a 32 bit integer and the former is not.
 *
 * Revision 1.23  2001/12/06 22:14:45  dereks
 * Improve YUV 422 resize routine so it now subsamples as required.
 *
 * Revision 1.22  2001/12/03 02:21:50  dereks
 * Add YUV420P to RGB24F, RGB32F converters.
 *
 * Revision 1.21  2001/12/02 21:53:56  dereks
 * Additional debug information
 *
 * Revision 1.20  2001/11/28 04:43:10  robertj
 * Added synonym colour class for equivalent colour format strings.
 * Allowed for setting ancestor classes in PCOLOUR_CONVERTER() macro.
 * Moved static functions into internal class to avoid pasing lots of parameters.
 * Added conversions for flipped RGB colour formats.
 *
 * Revision 1.19  2001/09/06 02:06:36  robertj
 * Fixed bug in detecting size mismatch, thanks Vjacheslav Andrejev
 *
 * Revision 1.18  2001/08/22 02:14:08  robertj
 * Fixed MSVC compatibility.
 *
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

PSYNONYM_COLOUR_CONVERTER(SBGGR8,  SBGGR8);
PSYNONYM_COLOUR_CONVERTER(Grey,   Grey);
PSYNONYM_COLOUR_CONVERTER(GreyF,  GreyF);
PSYNONYM_COLOUR_CONVERTER(RGB24,  RGB24);
PSYNONYM_COLOUR_CONVERTER(RGB24F, RGB24F);
PSYNONYM_COLOUR_CONVERTER(BGR24,  BGR24);
PSYNONYM_COLOUR_CONVERTER(BGR24F, BGR24F);
PSYNONYM_COLOUR_CONVERTER(RGB32,  RGB32);
PSYNONYM_COLOUR_CONVERTER(BGR32,  BGR32);
PSYNONYM_COLOUR_CONVERTER(RGB32F, RGB32F);
PSYNONYM_COLOUR_CONVERTER(YUV411P,YUV411P);
PSYNONYM_COLOUR_CONVERTER(YUV420P,YUV420P);
PSYNONYM_COLOUR_CONVERTER(YUV420P,IYUV);
PSYNONYM_COLOUR_CONVERTER(IYUV,   YUV420P);
PSYNONYM_COLOUR_CONVERTER(YUV420P,I420);
PSYNONYM_COLOUR_CONVERTER(I420,   YUV420P);


class PStandardColourConverter : public PColourConverter
{
  PCLASSINFO(PStandardColourConverter, PColourConverter);
  protected:
    PStandardColourConverter(
      const PString & srcFmt,
      const PString & dstFmt,
      unsigned w, unsigned h
    ) : PColourConverter(srcFmt, dstFmt, w, h) { }

  BOOL SBGGR8toYUV420P(
     const BYTE * srgb,
      BYTE * rgb,
      PINDEX * bytesReturned,
      BOOL flipVertical
    ) const;
    BOOL SBGGR8toRGB(
      const BYTE * srgb,
      BYTE * rgb,
      PINDEX * bytesReturned,
      BOOL flipVertical
    ) const;
    void GreytoYUV420PSameSize(
      const BYTE * rgb,
      BYTE * yuv,
      BOOL flipVertical
    ) const;
    void GreytoYUV420PWithResize(
      const BYTE * rgb,
      BYTE * yuv,
      BOOL flipVertical
    ) const;
    BOOL GreytoYUV420P(
      const BYTE * rgb,
      BYTE * yuv,
      PINDEX * bytesReturned,
      BOOL flipVertical
    ) const;
    void RGBtoYUV420PSameSize(
      const BYTE * rgb,
      BYTE * yuv,
      unsigned rgbIncrement,
      BOOL flipVertical,
      BOOL flipBR
    ) const;
    void RGBtoYUV420PWithResize(
      const BYTE * rgb,
      BYTE * yuv,
      unsigned rgbIncrement,
      BOOL flipVertical,
      BOOL flipBR
    ) const;
    BOOL RGBtoYUV420P(
      const BYTE * rgb,
      BYTE * yuv,
      PINDEX * bytesReturned,
      unsigned rgbIncrement,
      BOOL flipVertical,
      BOOL flipBR
    ) const;
    BOOL YUV420PtoRGB(
      const BYTE * yuv,
      BYTE * rgb,
      PINDEX * bytesReturned,
      unsigned rgbIncrement,
      BOOL flipVertical,
      BOOL flipBR
    ) const;
    void ResizeYUV422(
      const BYTE * src,
      BYTE * dest
    ) const;
};


#define PSTANDARD_COLOUR_CONVERTER(from,to) \
  PCOLOUR_CONVERTER2(P_##from##_##to,PStandardColourConverter,#from,#to)


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
    if (*find == converterName) {
      return find->Create(width, height);
    }
    find = find->link;
  }

  PTRACE(2,"PColCnv\tCreate error. Did not find " << srcColourFormat << "->" << destColourFormat);
  return NULL;
}


PColourConverter::PColourConverter(const PString & src,
                                   const PString & dst,
                                   unsigned width,
                                   unsigned height)
  : srcColourFormat(src),
    dstColourFormat(dst)
{
  PTRACE(6,"PColCnv\tPColourConverter constructed: " << src << "->" << dst << ' ' << width << 'x'<< height);

  doVFlip = FALSE;
  SetFrameSize(width,height);
}


BOOL PColourConverter::SetFrameSize(unsigned width, unsigned height)
{
  BOOL ok1 = SetSrcFrameSize(width, height);
  BOOL ok2 = SetDstFrameSize(width, height, FALSE);
  PTRACE(6,"PColCnv\tSetFrameSize: " << width << 'x' << height
         << (ok1 && ok2 ? " OK" : " Failed"));
  return ok1 && ok2;
}


BOOL PColourConverter::SetSrcFrameSize(unsigned width, unsigned height)
{
  srcFrameWidth = width;
  srcFrameHeight = height;
  srcFrameBytes = PVideoDevice::CalculateFrameBytes(srcFrameWidth, srcFrameHeight, srcColourFormat);
  PTRACE(6, "PColCnv\tSetSrcFrameSize "
         << ((srcFrameBytes != 0) ? "Succeed": "Fail") << "ed, "
         << srcColourFormat << ' ' << srcFrameWidth << 'x' << srcFrameHeight
         << ", " << srcFrameBytes << " bytes.");

  return srcFrameBytes != 0;
}


BOOL PColourConverter::SetDstFrameSize(unsigned width, unsigned height, BOOL bScale)
{
  dstFrameWidth  = width;
  dstFrameHeight = height;
  scaleNotCrop   = bScale;

  dstFrameBytes = PVideoDevice::CalculateFrameBytes(dstFrameWidth, dstFrameHeight, dstColourFormat);

  PTRACE(6, "PColCnv\tSetDstFrameSize "
         << ((dstFrameBytes != 0) ? "Succeed": "Fail") << "ed, "
         << dstColourFormat << ' ' << dstFrameWidth << 'x' << dstFrameHeight
         << ", " << dstFrameBytes << " bytes.");

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


///////////////////////////////////////////////////////////////////////////////

PColourConverter * PSynonymColourRegistration::Create(unsigned w, unsigned h) const
{
  PINDEX tab = Find('\t');
  return new PSynonymColour(Left(tab), Mid(tab+1), w, h);
}


BOOL PSynonymColour::Convert(const BYTE *srcFrameBuffer,
                             BYTE *dstFrameBuffer,
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

///////////////////////////////////////////////////////////////////////////////

#define BLACK_Y 0
#define BLACK_U 128
#define BLACK_V 128

#define greytoy(r, y) y=r
#define greytoyuv(r, y, u, v) greytoy(r,y); u=BLACK_U; v=BLACK_V

void PStandardColourConverter::GreytoYUV420PSameSize(const BYTE * grey,
						     BYTE * yuv,
						     BOOL flip) const
{
  const unsigned planeSize = srcFrameWidth*srcFrameHeight;
  const unsigned halfWidth = srcFrameWidth >> 1;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * greyIndex = grey;

  for (unsigned y = 0; y < srcFrameHeight; y++) {
    BYTE * yline  = yplane + (y * srcFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      greyIndex = grey + srcFrameWidth*(srcFrameHeight-1-y);

    for (unsigned x = 0; x < srcFrameWidth; x+=2) {
      greytoy(*greyIndex, *yline);
      greyIndex++;
      yline++;
      greytoyuv(*greyIndex, *yline, *uline, *vline);
      greyIndex++;
      yline++;
      uline++;
      vline++;
    }
  }
}


// Simple crop/pad version.  Image aligned to top-left
// and cropped / padded with black borders as required.
void PStandardColourConverter::GreytoYUV420PWithResize(const BYTE * grey,
						       BYTE * yuv,
						       BOOL   flip) const
{
  int planeSize = dstFrameWidth*dstFrameHeight;
  const int halfWidth = dstFrameWidth >> 1;
  unsigned min_width, min_height;

  min_width  = (dstFrameWidth  < srcFrameWidth)  ? dstFrameWidth  : srcFrameWidth;
  min_height = (dstFrameHeight < srcFrameHeight) ? dstFrameHeight : srcFrameHeight;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * greyIndex = grey;

  for (unsigned y = 0; y < min_height; y++) 
  {
    BYTE * yline  = yplane + (y * dstFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      greyIndex = grey + srcFrameWidth*(min_height-1-y);

    for (unsigned x = 0; x < min_width; x+=2) 
    {
     greytoy(*greyIndex, *yline);
     greyIndex++;
     yline++;
     greytoyuv(*greyIndex, *yline, *uline, *vline);
     greyIndex++;
     yline++;
     uline++;
     vline++;
    }

    // Crop if source width > dest width
    if (srcFrameWidth > dstFrameWidth)
      greyIndex += srcFrameWidth - dstFrameWidth;

    // Pad if dest width < source width
    if (dstFrameWidth > srcFrameWidth) {
      memset(yline, BLACK_Y, dstFrameWidth - srcFrameWidth);
      memset(uline, BLACK_U, (dstFrameWidth - srcFrameWidth)>>1);
      memset(vline, BLACK_V, (dstFrameWidth - srcFrameWidth)>>1);
    }
  }

  // Pad if dest height > source height
  if (dstFrameHeight > srcFrameHeight) {
    BYTE * yline  = yplane + (srcFrameHeight * dstFrameWidth);
    BYTE * uline  = uplane + ((srcFrameHeight >> 1) * halfWidth);
    BYTE * vline  = vplane + ((srcFrameHeight >> 1) * halfWidth);
    unsigned fill = (dstFrameHeight - srcFrameHeight) * dstFrameWidth;

    memset(yline, BLACK_Y, fill);
    memset(uline, BLACK_U, fill >> 2);
    memset(vline, BLACK_V, fill >> 2);
  }
}


BOOL PStandardColourConverter::GreytoYUV420P(const BYTE * grey,
					     BYTE * yuv,
					     PINDEX * bytesReturned,
					     BOOL flip) const
{
  if (grey == yuv)
    return FALSE; // Cannot do in place conversion

  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    GreytoYUV420PSameSize(grey, yuv, flip);
  else
    GreytoYUV420PWithResize(grey, yuv, flip);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}

#define rgbtoy(b, g, r, y) \
  y=(BYTE)(((int)30*r  +(int)59*g +(int)11*b)/100)

#define rgbtoyuv(b, g, r, y, u, v) \
  rgbtoy(b, g, r, y); \
  u=(BYTE)(((int)-17*r  -(int)33*g +(int)50*b+12800)/100); \
  v=(BYTE)(((int)50*r  -(int)42*g -(int)8*b+12800)/100)

void PStandardColourConverter::RGBtoYUV420PSameSize(const BYTE * rgb,
                                                    BYTE * yuv,
                                                    unsigned rgbIncrement,
                                                    BOOL flip,
						    BOOL flipBR) const
{
  const unsigned planeSize = srcFrameWidth*srcFrameHeight;
  const unsigned halfWidth = srcFrameWidth >> 1;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * rgbIndex = rgb;

  for (unsigned y = 0; y < srcFrameHeight; y++) {
    BYTE * yline  = yplane + (y * srcFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      rgbIndex = rgb + (srcFrameWidth*(srcFrameHeight-1-y)*rgbIncrement);

    for (unsigned x = 0; x < srcFrameWidth; x+=2) {
      if (!flipBR) {
	
	rgbtoy(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline);
      }
      else {

	rgbtoy(rgbIndex[2], rgbIndex[1], rgbIndex[0],*yline);
      }
      
      rgbIndex += rgbIncrement;
      yline++;
      if (!flipBR) {
	
	rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline, *uline, *vline);
      }
      else {

	rgbtoyuv(rgbIndex[2], rgbIndex[1], rgbIndex[0],*yline, *uline, *vline);
      }

      rgbIndex += rgbIncrement;
      yline++;
      uline++;
      vline++;
    }
  }
}


// Simple crop/pad version.  Image aligned to top-left
// and cropped / padded with black borders as required.
void PStandardColourConverter::RGBtoYUV420PWithResize(const BYTE * rgb,
                                                      BYTE * yuv,
                                                      unsigned rgbIncrement,
						      BOOL   flip,
						      BOOL flipBR) const
{
  int planeSize = dstFrameWidth*dstFrameHeight;
  const int halfWidth = dstFrameWidth >> 1;
  unsigned min_width, min_height;

  min_width  = (dstFrameWidth  < srcFrameWidth)  ? dstFrameWidth  : srcFrameWidth;
  min_height = (dstFrameHeight < srcFrameHeight) ? dstFrameHeight : srcFrameHeight;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * rgbIndex = rgb;

  for (unsigned y = 0; y < min_height; y++) 
  {
    BYTE * yline  = yplane + (y * dstFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (flip)
      rgbIndex = rgb + (srcFrameWidth*(min_height-1-y)*rgbIncrement); 

    for (unsigned x = 0; x < min_width; x+=2) {
      if (!flipBR) {
	rgbtoy(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline);
      }
      else {
	rgbtoy(rgbIndex[2], rgbIndex[1], rgbIndex[0],*yline);
      }
     rgbIndex += rgbIncrement;
     yline++;
     if (!flipBR) {
       
       rgbtoyuv(rgbIndex[0], rgbIndex[1], rgbIndex[2],*yline, *uline, *vline);
     }
     else {
       
       rgbtoyuv(rgbIndex[2], rgbIndex[1], rgbIndex[0],*yline, *uline, *vline);
     }
     
     rgbIndex += rgbIncrement;
     yline++;
     uline++;
     vline++;
    }

    // Crop if source width > dest width
    if (srcFrameWidth > dstFrameWidth)
      rgbIndex += rgbIncrement * (srcFrameWidth - dstFrameWidth);

    // Pad if dest width < source width
    if (dstFrameWidth > srcFrameWidth) {
      memset(yline, BLACK_Y, dstFrameWidth - srcFrameWidth);
      memset(uline, BLACK_U, (dstFrameWidth - srcFrameWidth)>>1);
      memset(vline, BLACK_V, (dstFrameWidth - srcFrameWidth)>>1);
    }
  }

  // Pad if dest height > source height
  if (dstFrameHeight > srcFrameHeight) {
    BYTE * yline  = yplane + (srcFrameHeight * dstFrameWidth);
    BYTE * uline  = uplane + ((srcFrameHeight >> 1) * halfWidth);
    BYTE * vline  = vplane + ((srcFrameHeight >> 1) * halfWidth);
    unsigned fill = (dstFrameHeight - srcFrameHeight) * dstFrameWidth;

    memset(yline, BLACK_Y, fill);
    memset(uline, BLACK_U, fill >> 2);
    memset(vline, BLACK_V, fill >> 2);
  }
}


BOOL PStandardColourConverter::RGBtoYUV420P(const BYTE * rgb,
                                            BYTE * yuv,
                                            PINDEX * bytesReturned,
                                            unsigned rgbIncrement,
                                            BOOL flip,
					    BOOL flipBR) const
{
  if (rgb == yuv)
    return FALSE; // Cannot do in place conversion

  if ((srcFrameWidth == dstFrameWidth) && (srcFrameHeight == dstFrameHeight)) 
    RGBtoYUV420PSameSize(rgb, yuv, rgbIncrement, flip, flipBR);
  else
    RGBtoYUV420PWithResize(rgb, yuv, rgbIncrement, flip, flipBR);

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}


PSTANDARD_COLOUR_CONVERTER(Grey,YUV420P)
{
  return GreytoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, doVFlip);
}


PSTANDARD_COLOUR_CONVERTER(RGB24,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3,  doVFlip, FALSE);
}


PSTANDARD_COLOUR_CONVERTER(BGR24,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3,  doVFlip, TRUE);
}


PSTANDARD_COLOUR_CONVERTER(RGB32,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, doVFlip, FALSE);
}


PSTANDARD_COLOUR_CONVERTER(BGR32,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, doVFlip, TRUE);
}


PSTANDARD_COLOUR_CONVERTER(RGB24F,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, doVFlip, FALSE);
}


PSTANDARD_COLOUR_CONVERTER(GreyF,YUV420P)
{
  return GreytoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, doVFlip);
}


PSTANDARD_COLOUR_CONVERTER(RGB32F,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, doVFlip, FALSE);
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
// YUV422 is stored as Y U Y V 
//   thus, a 4x4 image requires 32 bytes of storage.
//
// Image has two possible transformations.
//        padded                 (src smaller than dst)      
//        subsampled and padded  (src bigger than dst)  

void PStandardColourConverter::ResizeYUV422(const BYTE * src, BYTE * dest) const
{
  DWORD *result = (DWORD *)dest;
  DWORD black   = (DWORD)(BLACK_U<<24) + (BLACK_Y<<16) + (BLACK_U<<8) + BLACK_Y;
  unsigned maxIndex    = dstFrameWidth*dstFrameHeight/2;

  for (unsigned i = 0; i < maxIndex; i++) 
    *result++ = black;

  if ( (dstFrameWidth*dstFrameHeight) > (srcFrameWidth*srcFrameHeight) ) { 
    //dest is bigger than the source. No subsampling.
    //Place the src in the middle of the destination.
    unsigned yOffset = dstFrameHeight - srcFrameHeight;
    unsigned xOffset = dstFrameWidth - srcFrameWidth;

    BYTE *s_ptr,*d_ptr;
    d_ptr = (yOffset * dstFrameWidth) + xOffset + dest;
    s_ptr = (BYTE *)src;
    for (unsigned y = 0; y < srcFrameHeight; y++) {
      memcpy(d_ptr,s_ptr, srcFrameWidth*2);
      d_ptr += 2*dstFrameWidth;
      s_ptr += 2*srcFrameWidth;
    }
  } else {  
    // source is bigger than the destination.
    //
    unsigned subSample  = 1 + (srcFrameHeight/dstFrameHeight) ;
    unsigned yOffset    = dstFrameHeight - (srcFrameHeight/subSample);
    unsigned xOffset    = dstFrameWidth - (srcFrameWidth/subSample);
    unsigned subSample2 = subSample*2;

    DWORD *s_ptr = (DWORD * )src;
    DWORD *d_ptr = (DWORD *) dest + ((yOffset * dstFrameWidth) + xOffset)/4 ;
    DWORD *sl_ptr, *dl_ptr;

    for (unsigned y = 0; y < srcFrameHeight; y+= subSample) {
      sl_ptr = s_ptr;
      dl_ptr = d_ptr;
      for (unsigned x = 0; x < srcFrameWidth; x+= subSample2) {
	*dl_ptr++ = *sl_ptr;
	sl_ptr += subSample;
      }	
      d_ptr += dstFrameWidth/2;
      s_ptr += srcFrameWidth*subSample/2;
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
    ResizeYUV422(srcFrameBuffer, dstFrameBuffer);

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

    ResizeYUV422(srcFrameBuffer, intermed);
    Yuv422ToYuv420P(dstFrameWidth, dstFrameHeight, intermed, dstFrameBuffer);
  }

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


#define LIMIT(x) (unsigned char) (((x > 0xffffff) ? 0xff0000 : ((x <= 0xffff) ? 0 : x & 0xff0000)) >> 16)
static inline int clip(int a, int limit) {
  return a<limit?a:limit;
}

BOOL PStandardColourConverter::SBGGR8toYUV420P(const BYTE * src,
					       BYTE * dst,
					       PINDEX * bytesReturned,
					       BOOL     /*flipVertical*/) const
{
#define USE_SBGGR8_NATIVE 1 // set to 0 to use the double conversion algorithm (Bayer->RGB->YUV420P)
  
#if USE_SBGGR8_NATIVE

  // kernels for Y conversion, normalised by 2^16
  const int kR[]={1802,9667,1802,9667,19661,9667,1802,9667,1802}; 
  const int kG1[]={7733,9830,7733,3604,7733,3604,7733,9830,7733};
  const int kG2[]={7733,3604,7733,9830,7733,9830,7733,3604,7733};
  const int kB[]={4915,9667,4915,9667,7209,9667,4915,9667,4915};
  //  const int kID[]={0,0,0,0,65536,0,0,0,0}; identity kernel, use to test

  int B, G, G1, G2, R;
  const int stride = srcFrameWidth;
  unsigned const int hSize =srcFrameHeight/2;
  unsigned const int vSize =srcFrameWidth/2;
  unsigned const int lastRow=srcFrameHeight-1;
  unsigned const int lastCol=srcFrameWidth-1;
  unsigned int i,j;
  const BYTE *sBayer = src;

  //	Y = round( 0.256788 * R + 0.504129 * G + 0.097906 * B) +  16;
  //  Y = round( 0.30 * R + 0.59 * G + 0.11 * B ) use this!
  //	U = round(-0.148223 * R - 0.290993 * G + 0.439216 * B) + 128;
  //	V = round( 0.439216 * R - 0.367788 * G - 0.071427 * B) + 128;

  // Compute U and V planes using EXACT values, reading 2x2 pixels at a time
  BYTE *dU = dst+srcFrameHeight*srcFrameWidth;
  BYTE *dV = dU+hSize*vSize;
  for (i=0; i<hSize; i++) {      
    for (j=0; j<vSize; j++) {
      B=sBayer[0];
      G1=sBayer[1];
      G2=sBayer[stride];
      R=sBayer[stride+1];
      G=G1+G2;
      *dU = (BYTE)( ( (-19428 * R -19071*G +57569 * B) >> 17) + 128 );
      *dV = (BYTE)( ( ( 57569 * R -24103*G -9362 * B) >> 17) + 128 );
      sBayer+=2;
      dU++;
      dV++;
    }
    sBayer+=stride; // skip odd lines
  }
  // Compute Y plane
  BYTE *dY = dst;
  sBayer=src;
  const int * k; // kernel pointer
  int dxLeft, dxRight; // precalculated offsets, needed for first and last column
  const BYTE *sBayerTop, *sBayerBottom;
  for (i=0; i<srcFrameHeight; i++) {
    // Pointer to previous row, to the next if we are on the first one
    sBayerTop=sBayer+(i?(-stride):stride);
    // Pointer to next row, to the previous one if we are on the last
    sBayerBottom=sBayer+((i<lastRow)?stride:(-stride));
    // offset to previous column, to the next if we are on the first col
    dxLeft=1;
    for (j=0; j<srcFrameWidth; j++) {
      // offset to next column, to previous if we are on the last one
      dxRight=j<lastCol?1:(-1);
      // find the proper kernel according to the current pixel color
      if ( (i ^ j) & 1)  k=(j&1)?kG1:kG2; // green 1 or green 2
      else if (!(i & 1))  k=kB; // blue
      else /* if (!(j & 1)) */ k=kR; // red
      
      // apply the proper kernel to this pixel and surrounding ones
      *dY= (BYTE)(clip( (k[0])*(int)sBayerTop[dxLeft]+
			(k[1])*(int)(*sBayerTop)+
			(k[2])*(int)sBayerTop[dxRight]+
			(k[3])*(int)sBayer[dxLeft]+
			(k[4])*(int)(*sBayer)+
			(k[5])*(int)sBayer[dxRight]+
			(k[6])*(int)sBayerBottom[dxLeft]+
			(k[7])*(int)(*sBayerBottom)+
			(k[8])*(int)sBayerBottom[dxRight], (1<<24)) >> 16);
      dY++;
      sBayer++;
      sBayerTop++;
      sBayerBottom++;
      dxLeft=-1;
    }
  }

  if (bytesReturned)
    *bytesReturned = srcFrameHeight*srcFrameWidth+2*hSize*vSize;

  return true;

#else //USE_SBGGR8_NATIVE

  // shortest but less efficient (one malloc per conversion!)
  BYTE * tempDest=(BYTE*)malloc(3*srcFrameWidth*srcFrameHeight);
  SBGGR8toRGB(src, tempDest, NULL, FALSE);
  BOOL r = RGBtoYUV420P(tempDest, dst, bytesReturned, 3, flipVertical, TRUE);
  free(tempDest);
  return r;

#endif //USE_SBGGR8_NATIVE
}

BOOL PStandardColourConverter::SBGGR8toRGB(const BYTE * src,
                                           BYTE       * dst,
                                           PINDEX     * bytesReturned,
					   BOOL         flipVertical) const
{
  if (src == dst || flipVertical)
    return FALSE;

    long int i;
    const BYTE *rawpt;
    BYTE *scanpt;
    long int size;

    rawpt = src;
    scanpt = dst;
    long int WIDTH = srcFrameWidth, HEIGHT = srcFrameHeight;
    size = WIDTH*HEIGHT;

    for ( i = 0; i < size; i++ ) {
      if ( (i/WIDTH) % 2 == 0 ) {
	  if ( (i % 2) == 0 ) {
	      /* B */
	      if ( (i > WIDTH) && ((i % WIDTH) > 0) ) {
		  *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
				*(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;	/* R */
		  *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
				*(rawpt+WIDTH)+*(rawpt-WIDTH))/4;	/* G */
		  *scanpt++ = *rawpt;					/* B */
	      } else {
		  /* first line or left column */
		  *scanpt++ = *(rawpt+WIDTH+1);		/* R */
		  *scanpt++ = (*(rawpt+1)+*(rawpt+WIDTH))/2;	/* G */
		  *scanpt++ = *rawpt;				/* B */
	      }
	  } else {
	      /* (B)G */
	      if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) ) {
		  *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;	/* R */
		  *scanpt++ = *rawpt;					/* G */
		  *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;		/* B */
	      } else {
		  /* first line or right column */
		  *scanpt++ = *(rawpt+WIDTH);	/* R */
		  *scanpt++ = *rawpt;		/* G */
		  *scanpt++ = *(rawpt-1);	/* B */
	      }
	  }
      } else {
	  if ( (i % 2) == 0 ) {
	      /* G(R) */
	      if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) ) {
		  *scanpt++ = (*(rawpt-1)+*(rawpt+1))/2;		/* R */
		  *scanpt++ = *rawpt;					/* G */
		  *scanpt++ = (*(rawpt+WIDTH)+*(rawpt-WIDTH))/2;	/* B */
	      } else {
		  /* bottom line or left column */
		  *scanpt++ = *(rawpt+1);		/* R */
		  *scanpt++ = *rawpt;			/* G */
		  *scanpt++ = *(rawpt-WIDTH);		/* B */
	      }
	  } else {
	      /* R */
	      if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) ) {
		  *scanpt++ = *rawpt;					/* R */
		  *scanpt++ = (*(rawpt-1)+*(rawpt+1)+
				*(rawpt-WIDTH)+*(rawpt+WIDTH))/4;	/* G */
		  *scanpt++ = (*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+
				*(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4;	/* B */
	      } else {
		  /* bottom line or right column */
		  *scanpt++ = *rawpt;				/* R */
		  *scanpt++ = (*(rawpt-1)+*(rawpt-WIDTH))/2;	/* G */
		  *scanpt++ = *(rawpt-WIDTH-1);		/* B */
	      }
	  }
      }
      rawpt++;
  }

  if (bytesReturned)
    *bytesReturned = scanpt - dst;

  return TRUE;
}

BOOL PStandardColourConverter::YUV420PtoRGB(const BYTE * srcFrameBuffer,
                                            BYTE * dstFrameBuffer,
                                            PINDEX * bytesReturned,
                                            unsigned rgbIncrement,
					    BOOL     flipVertical,
                                            BOOL     flipBR) const
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  BYTE          *dstImageFrame;
  unsigned int   nbytes    = srcFrameWidth*srcFrameHeight;
  const BYTE    *yplane    = srcFrameBuffer;           		// 1 byte Y (luminance) for each pixel
  const BYTE    *uplane    = yplane+nbytes;              	// 1 byte U for a block of 4 pixels
  const BYTE    *vplane    = uplane+(nbytes >> 2);       	// 1 byte V for a block of 4 pixels

  unsigned int   pixpos[4] = {0, 1, srcFrameWidth, srcFrameWidth + 1};
  unsigned int   originalPixpos[4] = {0, 1, srcFrameWidth, srcFrameWidth + 1};

  unsigned int   x, y, p;

  long     int   yvalue;
  long     int   l, r, g, b;

  if(flipVertical) {
    dstImageFrame = dstFrameBuffer + ((srcFrameHeight - 2) * srcFrameWidth * rgbIncrement);
    pixpos[0] = srcFrameWidth;
    pixpos[1] = srcFrameWidth +1;
    pixpos[2] = 0;
    pixpos[3] = 1;
  }
  else
    dstImageFrame = dstFrameBuffer;

  for (y = 0; y < srcFrameHeight; y += 2)
  {
    for (x = 0; x < srcFrameWidth; x += 2)
    {
      // The RGB value without luminance
      long int  cr, cb, rd, gd, bd;

      cb = *uplane-128;
      cr = *vplane-128;
      rd = 104635*cr;			// 		  106986*cr
      gd = -25690*cb-53294*cr;		// -26261*cb  +   -54496*cr 
      bd = 132278*cb;			// 135221*cb

      // Add luminance to each of the 4 pixels

      for (p = 0; p < 4; p++)
      {
        yvalue = *(yplane + originalPixpos[p])-16;

        if (yvalue < 0) yvalue = 0;

        l = 76310*yvalue;

        r = l+rd;
        g = l+gd;
        b = l+bd;

        BYTE * rgpPtr = dstImageFrame + rgbIncrement*pixpos[p];
        if (flipBR) {
          *rgpPtr++ = LIMIT(r);
          *rgpPtr++ = LIMIT(g);
          *rgpPtr++ = LIMIT(b);
        }
        else {
          *rgpPtr++= LIMIT(b);
          *rgpPtr++= LIMIT(g);
          *rgpPtr++ = LIMIT(r);
        }
        if (rgbIncrement == 4)
          *rgpPtr = 0;
      }

      yplane += 2;
      dstImageFrame += rgbIncrement*2;

      uplane++;
      vplane++;
    }
 
    yplane += srcFrameWidth;
    if (flipVertical)
      dstImageFrame -= 3*rgbIncrement*srcFrameWidth;
    else
      dstImageFrame += rgbIncrement*srcFrameWidth;  
  }

  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;

  return TRUE;
}

PSTANDARD_COLOUR_CONVERTER(SBGGR8,RGB24)
{
  return SBGGR8toRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, doVFlip);
}

PSTANDARD_COLOUR_CONVERTER(SBGGR8,YUV420P)
{
  return SBGGR8toYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, doVFlip);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB24)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, doVFlip, FALSE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,BGR24)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, doVFlip, TRUE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB32)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, doVFlip, FALSE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,BGR32)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, doVFlip, TRUE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB24F)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, !doVFlip, FALSE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,BGR24F)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, !doVFlip, TRUE);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB32F)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, !doVFlip, FALSE);
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

  unsigned x, y;

  // U plane
  for (y = 0; y < dstFrameHeight; y += 2) {
    for (x = 0; x < dstFrameWidth; x += 4) {
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
  for (y = 0; y < dstFrameHeight; y += 2) {
    for (x = 0; x < dstFrameWidth; x += 4) {
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

  unsigned x, y;

  // U plane
  for (y = 0; y < dstFrameHeight; y += 2) {
    for (x = 0; x < dstFrameWidth; x += 4) {

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
  for (y = 0; y < dstFrameHeight; y += 2) {
    for (x = 0; x < dstFrameWidth; x += 4) {

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

/*
 * The following functions converts video from IEEE 1394 cameras into
 * YUV420P format. The video format of IEEE 1394 cameras can be found
 *  at Section 2.1.3 of
http://www.1394ta.org/Download/Technology/Specifications/2000/IIDC_Spec_v1_30.pdf
 * 320x240 and 160x120 resolutions are used.
 *
 *
 * UYVY422 is just a byte permutation of YUV422. I believe this is not
 * due to endian problem
 *
 * These functions should accept arbitrary size of image.
 */


PSTANDARD_COLOUR_CONVERTER(UYVY422,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  unsigned int row,column;
  unsigned char *y = dstFrameBuffer;  //Initialise y,u,v here, to stop compiler warnings.
  unsigned char *u = dstFrameBuffer + dstFrameWidth*dstFrameHeight;
  unsigned char *v = dstFrameBuffer + dstFrameWidth*(dstFrameHeight + dstFrameHeight/4);
  const unsigned char *src = srcFrameBuffer;

  for(row=0; row < PMIN(srcFrameHeight, dstFrameHeight); row+=2) {
    y = dstFrameBuffer + dstFrameWidth*row;
    u = dstFrameBuffer + dstFrameWidth*dstFrameHeight + dstFrameWidth*row/4;
    v = dstFrameBuffer + dstFrameWidth*(dstFrameHeight + dstFrameHeight/4) + dstFrameWidth*row/4;
    src = srcFrameBuffer + row*srcFrameWidth*2;
    for(column=0; column < PMIN(srcFrameWidth, dstFrameWidth); column+=2) {
      *(u++) = (unsigned char)(((int)src[0] + src[srcFrameWidth*2])/2);
      *(y++) = src[1];
      *(v++) = (unsigned char)(((int)src[2] + src[2+srcFrameWidth*2])/2);
      *(y++) = src[3];
      src += 4;
    }
    for(column = PMIN(srcFrameWidth, dstFrameWidth);
	column < dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    y = dstFrameBuffer + dstFrameWidth*(row+1);
    src = srcFrameBuffer + (row+1)*srcFrameWidth*2;
    for(column=0; column < PMIN(srcFrameWidth,dstFrameWidth); column+=2) {
      src++;
      *(y++) = *(src++);
      src++;
      *(y++) = *(src++);
    }
    for(column = PMIN(srcFrameWidth, dstFrameWidth);
	column < dstFrameWidth; column+=2) {
      *(y++) = BLACK_Y;
      *(y++) = BLACK_Y;
    }
  }
  for(row = PMIN(srcFrameHeight, dstFrameHeight);
      row < dstFrameHeight; row+=2) {
    for(column = 0; column < dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    for(column = 0; column < dstFrameWidth; column+=2) {
      *(y++) = BLACK_Y;
      *(y++) = BLACK_Y;
    }
  }
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}

PSTANDARD_COLOUR_CONVERTER(UYV444,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer)
    return FALSE;

  unsigned int row,column;
  unsigned char *y = dstFrameBuffer;  //Initialise y,u,v here, to stop compiler warnings.
  unsigned char *u = dstFrameBuffer + dstFrameWidth*dstFrameHeight;
  unsigned char *v = dstFrameBuffer + dstFrameWidth*(dstFrameHeight + dstFrameHeight/4);
  const unsigned char *src = srcFrameBuffer;

  for(row=0; row < PMIN(srcFrameHeight, dstFrameHeight); row+=2) {
    y = dstFrameBuffer + dstFrameWidth*row;
    u = dstFrameBuffer + dstFrameWidth*dstFrameHeight + dstFrameWidth*row/4;
    v = dstFrameBuffer + dstFrameWidth*(dstFrameHeight + dstFrameHeight/4) + dstFrameWidth*row/4;
    src = srcFrameBuffer + row*srcFrameWidth*3;
    for(column=0; column < PMIN(srcFrameWidth, dstFrameWidth); column+=2) {
      *(u++) = (unsigned char)(((unsigned int)src[0] + src[3] + src[srcFrameWidth*3] + src[3+srcFrameWidth*3])/4);
      *(y++) = src[1];
      *(v++) = (unsigned char)(((unsigned int)src[2] + src[5] + src[srcFrameWidth*3] +src[3+srcFrameWidth*3])/4);
      *(y++) = src[4];
      src += 6;
    }
    for(column = PMIN(srcFrameWidth, dstFrameWidth);
	column < dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    y = dstFrameBuffer + dstFrameWidth*(row+1);
    src = srcFrameBuffer + (row+1)*srcFrameWidth*3;
    for(column=0; column < PMIN(srcFrameWidth, dstFrameWidth); column++) {
      src++;
      *(y++) = *(src++);
      src++;
    }
    for(column = PMIN(srcFrameWidth, dstFrameWidth);
	column < dstFrameWidth; column++)
      *(y++) = BLACK_Y;
  }
  for(row = PMIN(srcFrameHeight, dstFrameHeight);
      row<dstFrameHeight; row+=2) {
    for(column = 0; column < dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    for(column = 0; column < dstFrameWidth; column+=2) {
      *(y++) = BLACK_Y;
      *(y++) = BLACK_Y;
    }
  }
  if (bytesReturned != NULL)
    *bytesReturned = dstFrameBytes;
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
