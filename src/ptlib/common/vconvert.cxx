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
 *   Thorsten Westheider (thorsten.westheider@teleos-web.de)
 *   Mark Cooke (mpc@star.sr.bham.ac.uk)
 */

#include <ptlib.h>

#if P_VIDEO

#include <ptlib/video.h>

#ifdef __GNUC__
#pragma implementation "vconvert.h"
#endif

#include <ptlib/vconvert.h>

#if P_TINY_JPEG
  #include "tinyjpeg.h"
#endif

#if P_LIBJPEG
  extern"C" {
    #include <jpeglib.h>
  };
  #ifdef _MSC_VER
    #pragma comment(lib, P_LIBJPEG_LIBRARY)
  #endif
#endif

#if P_FFMPEG_SWSCALE
  extern "C" {
    P_PUSH_MSVC_WARNINGS(4244)
    #include <libswscale/swscale.h>
    P_POP_MSVC_WARNINGS()
  };

  #ifdef P_SWSCALE_LIB
    #pragma comment(lib, P_SWSCALE_LIB)
  #endif
#endif // P_FFMPEG_SWSCALE

#ifdef P_MEDIALIB
  #include <mlib.h>
#endif

#if P_IPP
  #include <ippcc.h>
  static struct P_IPP_DLL : PDynaLink
  {
    EntryPoint<IppStatus(__STDCALL*)(const Ipp8u*pSrc[3],int srcStep[3],Ipp8u*,int,IppiSize)>       ippiYCbCr420ToBGR_8u_P3C3R;
    EntryPoint<IppStatus(__STDCALL*)(const Ipp8u*pSrc[3],int srcStep[3],Ipp8u*,int,IppiSize,Ipp8u)> ippiYCbCr420ToBGR_8u_P3C4R;
    EntryPoint<IppStatus(__STDCALL*)(const Ipp8u*pSrc[3],               Ipp8u*,    IppiSize)>       ippiYUV420ToRGB_8u_P3C3;
    EntryPoint<IppStatus(__STDCALL*)(const Ipp8u*pSrc[3],int srcStep[3],Ipp8u*,int,IppiSize)>       ippiYUV420ToRGB_8u_P3AC4R;

    P_IPP_DLL()
      : PDynaLink("ippcc-8.0.dll\nippcc-7.1.dll\nippcc-7.0.dll")
      , P_DYNALINK_ENTRY_POINT(ippiYCbCr420ToBGR_8u_P3C3R)
      , P_DYNALINK_ENTRY_POINT(ippiYCbCr420ToBGR_8u_P3C4R)
      , P_DYNALINK_ENTRY_POINT(ippiYUV420ToRGB_8u_P3C3)
      , P_DYNALINK_ENTRY_POINT(ippiYUV420ToRGB_8u_P3AC4R)
    {
    }

  } g_intel;
#endif


#if _MSC_VER
  #pragma intrinsic(memcpy)
#if P_64BIT
  #define PRAGMA_OPTIMISE_ON()        __pragma(optimize("tg", on)) __pragma(runtime_checks("scu", off))
#else
  #define PRAGMA_OPTIMISE_ON()        __pragma(optimize("tgy", on)) __pragma(runtime_checks("scu", off))
#endif
  #define PRAGMA_OPTIMISE_DEFAULT()   __pragma(optimize("", on))    __pragma(runtime_checks("", restore))
#else
  #define PRAGMA_OPTIMISE_ON()
  #define PRAGMA_OPTIMISE_DEFAULT()
#endif


class PStandardColourConverter : public PColourConverter
{
    PCLASSINFO(PStandardColourConverter, PColourConverter);
  protected:
#if P_FFMPEG_SWSCALE
    SwsContext * m_swsContext;

    void AdjustRGBPixelFormat(AVPixelFormat & fmt, unsigned rgbIncrement, unsigned redOffset)
    {
      if (fmt == AV_PIX_FMT_NONE) {
        if (rgbIncrement == 3)
          fmt = redOffset == 0 ? AV_PIX_FMT_RGB24 : AV_PIX_FMT_BGR24;
        else
#ifdef P_MACOSX
          fmt = redOffset == 0 ? AV_PIX_FMT_ABGR : AV_PIX_FMT_ARGB;
#else
          fmt = redOffset == 0 ? AV_PIX_FMT_RGBA : AV_PIX_FMT_BGRA;
#endif
      }
    }

    bool CanUseFFMPEG(AVPixelFormat srcFmt, AVPixelFormat dstFmt, unsigned rgbIncrement, unsigned redOffset)
    {
      if (m_swsContext != NULL)
        return true;
      
      AdjustRGBPixelFormat(srcFmt, rgbIncrement, redOffset);
      AdjustRGBPixelFormat(dstFmt, rgbIncrement, redOffset);
      
      m_swsContext = sws_getContext(m_srcFrameWidth, m_srcFrameHeight, srcFmt,
                                    m_dstFrameWidth, m_dstFrameHeight, dstFmt,
                                    SWS_BILINEAR,
                                    NULL, NULL, NULL);
      if (m_swsContext != NULL)
        return true;
      
      PTRACE(2, "Cannot create FFMPEG scaler from " << srcFmt << " to " << dstFmt);
      return false;
    }
#endif

    PStandardColourConverter(const PColourPair & colours)
      : PColourConverter(colours)
#if P_FFMPEG_SWSCALE
      , m_swsContext(NULL)
#endif
    { }

#if P_FFMPEG_SWSCALE
    ~PStandardColourConverter()
    {
      sws_freeContext(m_swsContext); // NULL is OK
    }
#endif

    bool SBGGR8toYUV420P(
     const BYTE * srgb,
      BYTE * rgb,
      PINDEX * bytesReturned
    );
    bool SBGGR8toRGB(
      const BYTE * srgb,
      BYTE * rgb,
      PINDEX * bytesReturned
    ) const;
    void GreytoYUV420PSameSize(
      const BYTE * rgb,
      BYTE * yuv
    ) const;
    void GreytoYUV420PWithCrop(
      const BYTE * rgb,
      BYTE * yuv
    ) const;
    bool GreytoYUV420P(
      const BYTE * rgb,
      BYTE * yuv,
      PINDEX * bytesReturned
    ) const;
    bool RGBtoYUV420P(
      const BYTE * rgb,
      BYTE * yuv,
      PINDEX * bytesReturned,
      unsigned rgbIncrement,
      unsigned redOffset,
      unsigned blueOffset
    );
    bool YUV420PtoRGB(
      const BYTE * yuv,
      BYTE * rgb,
      PINDEX * bytesReturned,
      unsigned rgbIncrement,
      unsigned redOffset,
      unsigned blueOffset
    );
    PBoolean YUV420PtoRGB565(
      const BYTE * yuv,
      BYTE * rgb,
      PINDEX * bytesReturned
    ) const;
    bool SwapRedAndBlue(
      const BYTE * src,
      BYTE * dst,
      PINDEX * bytesReturned,
      unsigned srcIncrement,
      unsigned dstIncrement
    ) const;
    void UYVY422WithCrop(
      const BYTE *src_uyvy,
      BYTE *dst_uyvy
    ) const;
    void YUV422WithCrop(
      const BYTE * src,
      BYTE * dest,
      bool centred
    ) const;
    void UYVY422toYUV420PSameSize(
      const BYTE *uyvy,
      BYTE *yuv420p
    ) const;
    void UYVY422toYUV420PWithCrop(
      const BYTE *uyvy,
      BYTE *yuv420p
    ) const;
    void YUY2toYUV420PSameSize(
      const BYTE *yuy2,
      BYTE *yuv420p
    ) const;
    void YUY2toYUV420PWithGrow(
      const BYTE *yuy2,
      BYTE *yuv420p
    ) const;
    void YUY2toYUV420PWithShrink(
      const BYTE *yuy2,
      BYTE *yuv420p
    ) const;
};


#define PSTANDARD_COLOUR_CONVERTER(from,to) \
  PCOLOUR_CONVERTER2(PColourConverter_##from##_##to,PStandardColourConverter,#from,#to)


#define BLACK_Y 0
#define BLACK_U 128
#define BLACK_V 128


#define new PNEW
#define PTraceModule() "ColourConvert"


///////////////////////////////////////////////////////////////////////////////
// PColourConverter

PObject::Comparison PColourPair::Compare(const PObject & obj) const
{
  const PColourPair & other = dynamic_cast<const PColourPair &>(obj);
  Comparison result = m_srcColourFormat.Compare(other.m_srcColourFormat);
  if (result == EqualTo)
    result = m_dstColourFormat.Compare(other.m_dstColourFormat);
  return result;
}


PColourConverter * PColourConverter::Create(const PVideoFrameInfo & src,
                                            const PVideoFrameInfo & dst)
{
  PColourConverter *converter = PColourConverterFactory::CreateInstance(PColourPair(src.GetColourFormat(), dst.GetColourFormat()));
  if (converter == NULL) {
    PTRACE(2, "PColCnv", "Create error. Did not find " << src.GetColourFormat() << "->" << dst.GetColourFormat());
    return NULL;
  }

  converter->SetSrcFrameInfo(src);
  converter->SetDstFrameInfo(dst);
  return converter;
}

PColourConverter * PColourConverter::Create(const PString & srcColourFormat,
                                            const PString & destColourFormat,
                                            unsigned width,
                                            unsigned height)
{
  PVideoFrameInfo src;
  src.SetColourFormat(srcColourFormat);
  src.SetFrameSize(width, height);

  PVideoFrameInfo dst;
  dst.SetColourFormat(destColourFormat);

  return Create(src, dst);
}


PColourConverter::PColourConverter(const PColourPair & colours)
  : PColourPair(colours)
  , m_srcFrameWidth(0)
  , m_srcFrameHeight(0)
  , m_srcFrameBytes(0)
  , m_dstFrameWidth(0)
  , m_dstFrameHeight(0)
  , m_dstFrameBytes(0)
  , m_resizeMode(PVideoFrameInfo::eScale)
  , m_verticalFlip(false)
{
}


void PColourConverter::PrintOn(ostream & strm) const
{
  strm << m_srcColourFormat << ':' << m_srcFrameWidth << 'x' << m_srcFrameHeight << "->"
       << m_dstColourFormat << ':' << m_dstFrameWidth << 'x' << m_dstFrameHeight << '/'
       << m_resizeMode;
}


PBoolean PColourConverter::SetFrameSize(unsigned width, unsigned height)
{
  bool ok1 = SetSrcFrameSize(width, height);
  bool ok2 = SetDstFrameSize(width, height);
  PTRACE(ok1 && ok2 ? 4 : 2,"SetFrameSize: " << width << 'x' << height
         << (ok1 && ok2 ? " OK" : " Failed"));
  return ok1 && ok2;
}


PBoolean PColourConverter::SetSrcFrameInfo(const PVideoFrameInfo & info)
{
  if (!PAssert(info.GetColourFormat() == GetSrcColourFormat(), "Cannot change colour format"))
    return false;

  unsigned w, h;
  return info.GetFrameSize(w, h) && SetSrcFrameSize(w, h);
}


PBoolean PColourConverter::SetDstFrameInfo(const PVideoFrameInfo & info)
{
  if (!PAssert(info.GetColourFormat() == GetDstColourFormat(), "Cannot change colour format"))
    return false;

  SetResizeMode(info.GetResizeMode());

  unsigned w, h;
  return info.GetFrameSize(w, h) && SetDstFrameSize(w, h);
}


void PColourConverter::GetSrcFrameInfo(PVideoFrameInfo & info)
{
  info.SetColourFormat(GetSrcColourFormat());
  info.SetFrameSize(m_srcFrameWidth, m_srcFrameHeight);
}


void PColourConverter::GetDstFrameInfo(PVideoFrameInfo & info)
{
  info.SetColourFormat(GetDstColourFormat());
  info.SetFrameSize(m_dstFrameWidth, m_dstFrameHeight);
}


PBoolean PColourConverter::SetSrcFrameSize(unsigned width, unsigned height)
{
  if (m_srcFrameWidth == width && m_srcFrameHeight == height)
    return true;

  m_srcFrameWidth = width;
  m_srcFrameHeight = height;
  m_srcFrameBytes = PVideoDevice::CalculateFrameBytes(m_srcFrameWidth, m_srcFrameHeight, m_srcColourFormat);
  PTRACE(m_srcFrameBytes != 0 ? 6 : 2, "SetSrcFrameSize "
         << ((m_srcFrameBytes != 0) ? "Succeed": "Fail") << "ed, "
         << m_srcColourFormat << ' ' << m_srcFrameWidth << 'x' << m_srcFrameHeight
         << ", " << m_srcFrameBytes << " bytes.");

  return m_srcFrameBytes != 0;
}


PBoolean PColourConverter::SetDstFrameSize(unsigned width, unsigned height)
{
  m_dstFrameWidth  = width;
  m_dstFrameHeight = height;

  m_dstFrameBytes = PVideoDevice::CalculateFrameBytes(m_dstFrameWidth, m_dstFrameHeight, m_dstColourFormat);

  PTRACE(m_dstFrameBytes != 0 ? 6 : 2, "SetDstFrameSize "
         << ((m_dstFrameBytes != 0) ? "Succeed": "Fail") << "ed, "
         << m_dstColourFormat << ' ' << m_dstFrameWidth << 'x' << m_dstFrameHeight
         << ", " << m_dstFrameBytes << " bytes.");

  return m_dstFrameBytes != 0;
}

PBoolean PColourConverter::SetDstFrameSize(unsigned width, unsigned height, PBoolean bScale)
{
  if (!SetDstFrameSize(width, height))
    return false;

  if (bScale)
    SetResizeMode(PVideoFrameInfo::eScale);
  else
    SetResizeMode(PVideoFrameInfo::eCropCentre);

  return true;
}

PBoolean PColourConverter::GetSrcFrameSize(unsigned &width, unsigned &height) const
{
  width = m_srcFrameWidth;
  height = m_srcFrameHeight;
  return true;
}


PBoolean PColourConverter::GetDstFrameSize(unsigned &width, unsigned &height) const
{
  width = m_dstFrameWidth;
  height = m_dstFrameHeight;
  return true;
}


PBoolean PColourConverter::ConvertInPlace(BYTE * frameBuffer,
                                      PINDEX * bytesReturned,
                                      PBoolean noIntermediateFrame)
{
  if (Convert(frameBuffer, frameBuffer, bytesReturned))
    return true;

  if (noIntermediateFrame) {
    PTRACE(2,"Error in ConvertInPlace, no intermediate frame available.");
    return false;
  }

  BYTE * intermediate = m_intermediateFrameStore.GetPointer(m_dstFrameBytes);
  PINDEX bytes;
  if (!Convert(frameBuffer, intermediate, &bytes))
    return false;

  memcpy(frameBuffer, intermediate, bytes);
  if (bytesReturned != NULL)
    *bytesReturned = bytes;
  return true;
}


__inline BYTE RGBtoY(int r, int g, int b)
{
  int y = 299*r + 587*g + 114*b;
  return (BYTE)(y < 255000 ? (y/1000) : 255);
}

__inline BYTE RGBtoU(int r, int g, int b)
{
  int u = -147*r - 289*g + 436*b;
  return (BYTE)(u < -127000 ? 0 : u > 127000 ? 255 : (u/1000 + 128));
}

__inline BYTE RGBtoV(int r, int g, int b)
{
  int v =615*r - 515*g - 100*b;
  return (BYTE)(v < -127000 ? 0 : v > 127000 ? 255 : (v/1000 + 128));
}

void PColourConverter::RGBtoYUV(unsigned r, unsigned g, unsigned b,
                                BYTE   & y, BYTE   & u, BYTE   & v)
{
  y = RGBtoY(r,g,b);
  u = RGBtoU(r,g,b);
  v = RGBtoV(r,g,b);
}


class PRasterDutyCycle
{
  public:
    PRasterDutyCycle(
      PVideoFrameInfo::ResizeMode resizeMode,
      unsigned srcWidth, unsigned srcHeight,
      unsigned dstWidth, unsigned dstHeight,
      unsigned incrementX, unsigned incrementY
    ) : m_x(srcWidth,  dstWidth,  resizeMode, incrementX)
      , m_y(srcHeight, dstHeight, resizeMode, incrementY)
    {
    }

    __inline bool RunningX() { return m_x.Running(); }
    __inline bool RunningY() { return m_y.Running(); }

    __inline bool HasDutyX() { return m_x.HasDuty(); }
    __inline bool HasDutyY() { return m_y.HasDuty(); }

    __inline bool IsBlack() const { return m_x.IsBlack() || m_y.IsBlack(); }

    __inline unsigned GetX() const { return m_x.m_pixel; }
    __inline unsigned GetY() const { return m_y.m_pixel; }

  protected:
    struct Info {
      unsigned m_lower, m_upper, m_pixel, m_start, m_increment, m_count;
      bool m_growing, m_toggle;

      Info(unsigned srcMax, unsigned dstMax, PVideoFrameInfo::ResizeMode resizeMode, unsigned increment)
        : m_lower(std::min(srcMax, dstMax))
        , m_upper(std::max(srcMax, dstMax))
        , m_pixel(0)
        , m_start(0)
        , m_increment(increment)
        , m_count(0)
        , m_growing(srcMax < dstMax)
        , m_toggle(false)
      {
        switch (resizeMode) {
          default :
          case PVideoFrameInfo::eScale :
            break;

          case PVideoFrameInfo::eCropTopLeft :
            m_upper = m_lower;
            break;

          case PVideoFrameInfo::eCropCentre :
            m_start = (m_upper-m_lower)/2;
            m_upper -= m_start;
            m_lower = m_upper;
            break;
        }
      }

      __inline bool Running()
      {
        if (m_pixel < m_upper)
          return true;

        m_pixel = m_count = 0;
        return false;
      }

      __inline bool HasDuty()
      {
        if (m_toggle) {
          m_toggle = false;
          return false;
        }

        m_pixel += m_increment;
        if (m_pixel > m_upper)
          return false;
        if (m_pixel < m_start)
          return m_growing;

        m_count += m_lower;
        if (m_count < m_upper)
          return m_growing;

        m_count -= m_upper;
        m_toggle = true;
        return true;
      }

      __inline bool IsBlack() const
      {
        return m_growing && m_pixel < m_start;
      }
    } m_x, m_y;
};


#if P_FFMPEG_SWSCALE

// given yuv buffer, get ptr to location (x,y)
static BYTE* ffmpeg_yuvptr(const BYTE* frame, unsigned planeWidth, unsigned planeHeight, int x, int y, int plane)
{
    // 0 == y, 1 == u, 2 == v
    if (plane < 0 || plane > 2)
        return 0;

    if (plane >= 1)
    {
        frame += planeWidth * planeHeight;
        planeWidth /= 2;
        planeHeight /= 2;
        x /= 2;
        y /= 2;
        if (plane >= 2)
            frame += planeWidth * planeHeight;
    }
    frame += y * planeWidth;
    frame += x;
    return const_cast<BYTE*>(frame);
}

#endif // P_FFMPEG_SWSCALE


// Consider a YUV420P image of 4x4 pixels.
//
// A plane of Y values    A B C D
//                        E F G H
//                        I J K L 
//                        M N O P
//
// A plane of U values    1 . 2 . 
//                           . . . .
//                           3 . 4 .
//                        . . . .
//
// A plane of V values    1 . 2 .
//                           . . . .
//                        3 . 4 .
//                        . . . .
// 
// YUV420P is stored as all Y (w*h), then U (w*h/4), then V
//   thus, a 4x4 image requires 24 bytes of storage.
//
// Grow and Shrink utilise the Variable Duty Cycle algorithm, currently
// no interpolation is used, just pixel dropping or doubling

PRAGMA_OPTIMISE_ON()
static void GrowBothYUV420P(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                                  BYTE * dstPtr, unsigned dstWidth, unsigned dstHeight, int      dstLineSpan)
{
  unsigned repeatRow = 0;
  for (unsigned y = 0; y < srcHeight; y++) {

    const BYTE * srcPixel = srcPtr;
    BYTE * dstPixel = dstPtr;
    unsigned repeatPixel = 0;

    for (unsigned x = 0; x < srcWidth; x++) {
      do {
        *dstPixel++ = *srcPixel;
        repeatPixel += srcWidth;
      } while (repeatPixel < dstWidth);
      repeatPixel -= dstWidth;

      srcPixel++;
    }

    BYTE * repeatPtr = dstPtr;

    repeatRow += srcHeight;
    while (repeatRow < dstHeight) {
      dstPtr += dstLineSpan;
      memcpy(dstPtr, repeatPtr, dstWidth);
      repeatRow += srcHeight;
    }
    repeatRow -= dstHeight;

    srcPtr += srcLineSpan;
    dstPtr += dstLineSpan;
  }
}


static void ShrinkBothYUV420P(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                                    BYTE * dstPtr, unsigned dstWidth, unsigned dstHeight, int      dstLineSpan)
{
  unsigned srcAdvance = 0;
  for (unsigned y = 0; y < dstHeight; y++) {

    const BYTE * srcPixel = srcPtr;
    BYTE * dstPixel = dstPtr;
    unsigned repeatPixel = 0;

    for (unsigned x = 0; x < dstWidth; x++) {
      *dstPixel++ = *srcPixel;

      do {
        srcPixel++;
        repeatPixel += dstWidth;
      } while (repeatPixel < srcWidth);
      repeatPixel -= srcWidth;
    }

    do {
      srcPtr += srcLineSpan;
      srcAdvance += dstHeight;
    } while (srcAdvance < srcHeight);
    srcAdvance -= srcHeight;

    dstPtr += dstLineSpan;
  }
}


static void GrowRowsYUV420P(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                                  BYTE * dstPtr, unsigned         , unsigned dstHeight, int      dstLineSpan)
{
  unsigned repeatRow = 0;
  for (unsigned y = 0; y < srcHeight; y++) {
    memcpy(dstPtr, srcPtr, srcWidth);

    repeatRow += srcHeight;
    while (repeatRow < dstHeight) {
      dstPtr += dstLineSpan;
      memcpy(dstPtr, srcPtr, srcWidth);
      repeatRow += srcHeight;
    }
    repeatRow -= dstHeight;

    srcPtr += srcLineSpan;
    dstPtr += dstLineSpan;
  }
}


static void ShrinkRowsYUV420P(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                                    BYTE * dstPtr, unsigned         , unsigned dstHeight, int      dstLineSpan)
{
  unsigned srcAdvance = 0;
  for (unsigned y = 0; y < dstHeight; y++) {
    memcpy(dstPtr, srcPtr, srcWidth);

    do {
      srcPtr += srcLineSpan;
      srcAdvance += dstHeight;
    } while (srcAdvance < srcHeight);
    srcAdvance -= srcHeight;

    dstPtr += dstLineSpan;
  }
}


static void CropYUV420P(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                              BYTE * dstPtr, unsigned         , unsigned          , int      dstLineSpan)
{
  for (unsigned y = 0; y < srcHeight; y++) {
    memcpy(dstPtr, srcPtr, srcWidth);
    srcPtr += srcLineSpan;
    dstPtr += dstLineSpan;
  }
}
PRAGMA_OPTIMISE_DEFAULT()


static bool ValidateDimensions(unsigned srcFrameWidth, unsigned srcFrameHeight,
                               unsigned dstFrameWidth, unsigned dstFrameHeight,
                               PVideoFrameInfo::ResizeMode resizeMode,
                               std::ostream * error)
{
  if (srcFrameWidth == 0 || dstFrameWidth == 0 || srcFrameHeight == 0 || dstFrameHeight == 0) {
    if (error != NULL)
      *error << "Dimensions cannot be zero: "
             << srcFrameWidth << 'x' << srcFrameHeight << " -> " << dstFrameWidth << 'x' << dstFrameHeight;
    return false;
  }

  if (resizeMode != PVideoFrameInfo::eScale)
      return true;

  if (srcFrameWidth <= dstFrameWidth && srcFrameHeight <= dstFrameHeight)
    return true;

  if (srcFrameWidth >= dstFrameWidth && srcFrameHeight >= dstFrameHeight)
    return true;

  if (error != NULL)
    *error << "Cannot do one dimension shrinking and the other one growing: "
           << srcFrameWidth << 'x' << srcFrameHeight << " -> " << dstFrameWidth << 'x' << dstFrameHeight;
  return false;
}


bool PColourConverter::CopyYUV420P(unsigned srcX, unsigned srcY, unsigned srcWidth, unsigned srcHeight,
                                   unsigned srcFrameWidth, unsigned srcFrameHeight, const BYTE * srcYUV,
                                   unsigned dstX, unsigned dstY, unsigned dstWidth, unsigned dstHeight,
                                   unsigned dstFrameWidth, unsigned dstFrameHeight, BYTE * dstYUV,
                                   PVideoFrameInfo::ResizeMode resizeMode, bool verticalFlip, std::ostream * error)
{
  if (srcX == 0 && srcY == 0 && dstX == 0 && dstY == 0 &&
      srcWidth == dstWidth && srcHeight == dstHeight &&
      srcFrameWidth == dstFrameWidth && srcFrameHeight == dstFrameHeight &&
      srcWidth == srcFrameWidth && srcHeight == srcFrameHeight) {
    memcpy(dstYUV, srcYUV, PVideoFrameInfo::CalculateFrameBytes(dstFrameWidth, dstFrameHeight));
    return true;
  }

  if (resizeMode == PVideoFrameInfo::eScaleKeepAspect) {
	unsigned srcWidthByDstHeight = srcWidth * dstHeight;
	unsigned dstWidthBySrcHeight = dstWidth * srcHeight;
    if (srcWidthByDstHeight < dstWidthBySrcHeight) {
      unsigned outputWidth = (srcWidthByDstHeight/srcHeight)&~1;
      unsigned ouputX = ((dstWidth - outputWidth)/2)&~1;
      FillYUV420P(dstX                , dstY, ouputX, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
      FillYUV420P(dstX+dstWidth-ouputX, dstY, ouputX, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
      return CopyYUV420P(srcX, srcY, srcWidth, srcHeight, srcFrameWidth, srcFrameHeight, srcYUV,
                         dstX+ouputX, dstY, outputWidth, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV,
                         PVideoFrameInfo::eScale, verticalFlip, error);
    }
    else if (srcWidthByDstHeight > dstWidthBySrcHeight) {
      unsigned outputHeight = (dstWidthBySrcHeight/srcWidth)&~1;
      unsigned outputY = ((dstHeight - outputHeight)/2)&~1;
      FillYUV420P(dstX, dstY                  , dstWidth, outputY, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
      FillYUV420P(dstX, dstY+dstHeight-outputY, dstWidth, outputY, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
      return CopyYUV420P(srcX, srcY, srcWidth, srcHeight, srcFrameWidth, srcFrameHeight, srcYUV,
                         dstX, dstY+outputY, dstWidth, outputHeight, dstFrameWidth, dstFrameHeight, dstYUV,
                         PVideoFrameInfo::eScale, verticalFlip, error);
    }
  }

  if (!ValidateDimensions(srcWidth, srcHeight, dstWidth, dstHeight, resizeMode, error))
    return false;

  if (srcFrameWidth == 0)
     srcFrameWidth = srcWidth;
  if (srcFrameHeight == 0)
     srcFrameHeight = srcHeight;
  if (dstFrameWidth == 0)
     dstFrameWidth = dstWidth;
  if (dstFrameHeight == 0)
     dstFrameHeight = dstHeight;

  if (srcX + srcWidth > srcFrameWidth) {
    if (error != NULL)
      *error << "Invalid rectangle bounds:"
                " srcX=" << srcX << " + srcWidth=" << srcWidth << " > frameWidth=" << srcFrameWidth;
    return false;
  }
  if (srcY + srcHeight > srcFrameHeight) {
    if (error != NULL)
      *error << "Invalid rectangle bounds:"
                " srcY=" << srcY << " + srcHeight=" << srcHeight << " > frameHeight=" << srcFrameHeight;
    return false;
  }
  if (dstX + dstWidth > dstFrameWidth) {
    if (error != NULL)
      *error << "Invalid rectangle bounds:"
                " dstX=" << dstX << " + dstWidth=" << dstWidth << " > frameWidth=" << dstFrameWidth;
    return false;
  }
  if (dstY + dstHeight > dstFrameHeight) {
    if (error != NULL)
      *error << "Invalid rectangle bounds:"
                " dstY=" << dstY << " + dstHeight=" << dstHeight << " > frameHeight=" << dstFrameHeight;
    return false;
  }

  srcFrameWidth  = (srcFrameWidth+1)&~1;
  srcFrameHeight = (srcFrameHeight+1)&~1;
  dstFrameWidth  = (dstFrameWidth+1)&~1;
  dstFrameHeight = (dstFrameHeight+1)&~1;

#if P_FFMPEG_SWSCALE

  struct SwsContext * context = sws_getContext(srcWidth, srcHeight, AV_PIX_FMT_YUV420P,
                                               dstWidth, dstHeight, AV_PIX_FMT_YUV420P,
                                               SWS_BILINEAR, NULL, NULL, NULL);
  if (context != NULL) {
    const uint8_t* srcSlice[] = {
        ffmpeg_yuvptr(srcYUV, srcFrameWidth, srcFrameHeight, srcX, srcY, 0),
        ffmpeg_yuvptr(srcYUV, srcFrameWidth, srcFrameHeight, srcX, srcY, 1),
        ffmpeg_yuvptr(srcYUV, srcFrameWidth, srcFrameHeight, srcX, srcY, 2)
    };
    const int srcStride[] = {
        (int)srcFrameWidth,
        (int)srcFrameWidth/2,
        (int)srcFrameWidth/2
    };

    uint8_t* dstSlice[] = {
        ffmpeg_yuvptr(dstYUV, dstFrameWidth, dstFrameHeight, dstX, dstY, 0),
        ffmpeg_yuvptr(dstYUV, dstFrameWidth, dstFrameHeight, dstX, dstY, 1),
        ffmpeg_yuvptr(dstYUV, dstFrameWidth, dstFrameHeight, dstX, dstY, 2)
    };
    const int dstStride[] = {
        (int)dstFrameWidth,
        (int)dstFrameWidth/2,
        (int)dstFrameWidth/2
    };

    sws_scale(context, srcSlice, srcStride, 0, srcHeight, dstSlice, dstStride);

    sws_freeContext(context);
    return true;
  }

#endif // P_FFMPEG_SWSCALE

  void(*rowFunction)(const BYTE * srcPtr, unsigned srcWidth, unsigned srcHeight, unsigned srcLineSpan,
                     BYTE * dstPtr, unsigned dstWidth, unsigned dstHeight, int dstFrameWidth) = CropYUV420P;

  switch (resizeMode) {
    default : // Scaling options
      if (srcWidth > dstWidth)
        rowFunction = ShrinkBothYUV420P;
      else if (srcWidth < dstWidth)
        rowFunction = GrowBothYUV420P;
      else if (srcHeight > dstHeight)
        rowFunction = ShrinkRowsYUV420P; // More efficient version for same width case
      else if (srcHeight < dstHeight)
        rowFunction = GrowRowsYUV420P;
      // else use crop
      break;

    case PVideoFrameInfo::eCropTopLeft :
      if (srcWidth >= dstWidth)
        srcWidth = dstWidth;
      else {
        FillYUV420P(dstX + srcWidth, dstY, dstWidth - srcWidth, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        dstWidth = srcWidth;
      }

      if (srcHeight >= dstHeight)
        srcHeight = dstHeight;
      else {
        FillYUV420P(dstX, dstY + srcHeight, dstWidth, dstHeight - srcHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        dstHeight = srcHeight;
      }
      break;

    case PVideoFrameInfo::eCropCentre :
      if (dstWidth > srcWidth) {
        unsigned fillWidth = ((dstWidth - srcWidth)/2)&~1;
        FillYUV420P(              dstX, dstY, fillWidth, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        FillYUV420P(dstWidth-fillWidth, dstY, fillWidth, dstHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        dstX += fillWidth;
        dstWidth = srcWidth;
      }
      else {
        srcX += (srcWidth - dstWidth)/2;
        srcWidth = dstWidth;
      }

      if (dstHeight > srcHeight) {
        unsigned fillHeight = ((dstHeight - srcHeight)/2)&~1;
        FillYUV420P(dstX,                 dstY, dstWidth, fillHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        FillYUV420P(dstX, dstHeight-fillHeight, dstWidth, fillHeight, dstFrameWidth, dstFrameHeight, dstYUV, 0, 0, 0);
        dstY += fillHeight;
        dstWidth = srcHeight;
      }
      else {
        srcY += (srcHeight - dstHeight)/2;
        srcHeight = dstHeight;
      }
      break;
  }

  const BYTE * srcPtr = srcYUV + srcY * srcFrameWidth + srcX;
  BYTE * dstPtr = dstYUV + dstY * dstFrameWidth + dstX;
  int dstLineSpan = dstFrameWidth;
  if (verticalFlip) {
    dstPtr += (dstHeight - 1) * dstFrameWidth;
    dstLineSpan = -dstLineSpan;
  }

  // Copy plane Y
  rowFunction(srcPtr, srcWidth, srcHeight, srcFrameWidth, dstPtr, dstWidth, dstHeight, dstLineSpan);

  srcYUV += srcFrameWidth*srcFrameHeight;
  dstYUV += dstFrameWidth*dstFrameHeight;

  // U & V planes half size
  srcX /= 2;
  srcY /= 2;
  dstX /= 2;
  dstY /= 2;
  srcWidth /= 2;
  srcHeight /= 2;
  dstWidth /= 2;
  dstHeight /= 2;
  srcFrameWidth /= 2;
  srcFrameHeight /= 2;
  dstFrameWidth /= 2;
  dstFrameHeight /= 2;
  dstLineSpan /= 2;

  srcPtr = srcYUV + srcY * srcFrameWidth + srcX;
  dstPtr = dstYUV + dstY * dstFrameWidth + dstX;
  if (verticalFlip)
    dstPtr += (dstHeight - 1) * dstFrameWidth;

  // Copy plane U
  rowFunction(srcPtr, srcWidth, srcHeight, srcFrameWidth, dstPtr, dstWidth, dstHeight, dstLineSpan);

  srcPtr += srcFrameWidth*srcFrameHeight;
  dstPtr += dstFrameWidth*dstFrameHeight;

  // Copy plane V
  rowFunction(srcPtr, srcWidth, srcHeight, srcFrameWidth, dstPtr, dstWidth, dstHeight, dstLineSpan);

  return true;
}


PRAGMA_OPTIMISE_ON()
bool PColourConverter::RotateYUV420P(int angle, unsigned width, unsigned height, BYTE * srcYUV, BYTE * dstYUV)
{
  if (!PAssert(width > 16 && height > 16, PInvalidParameter))
    return false;

  unsigned size = PVideoFrameInfo::CalculateFrameBytes(width, height);

  if (angle == 0) {
    if (dstYUV != NULL && srcYUV != dstYUV)
      memcpy(dstYUV, srcYUV, size);
    return true;
  }

  if (!PAssert(angle == -90 || angle == 90 || angle == 180, PInvalidParameter))
    return false;

  PBYTEArray storage;
  if (dstYUV == NULL || srcYUV == dstYUV)
    dstYUV = storage.GetPointer(size);

  width  = (width+1)&~1;
  height = (height+1)&~1;
  struct PlaneInfo {
    unsigned int width;
    unsigned int height;
    BYTE * src;
    BYTE * dst;
  } plane[3] = {
    { width,   height,   srcYUV,                  dstYUV                  },
    { width/2, height/2, srcYUV+width*height,     dstYUV+width*height     },
    { width/2, height/2, srcYUV+width*height*5/4, dstYUV+width*height*5/4 }
  };

  switch (angle) {
    case -90 :
      for (int p = 0; p < 3; ++p) {
        plane[p].dst += plane[p].width*plane[p].height;
        for (int y = plane[p].height; y > 0; --y) {
          BYTE * tempY = plane[p].dst - y;
          for (int x = plane[p].width; x > 0; --x) {
            *tempY = *plane[p].src++;
            tempY -= plane[p].height;
          }
        }
      }
      break;

    case 90 :
      for (int p = 0; p < 3; ++p) {
        for (int y = plane[p].height-1; y >= 0; --y) {
          BYTE * tempY = plane[p].dst + y;
          for (int x = plane[p].width; x > 0; --x) {
            *tempY = *plane[p].src++;
            tempY += plane[p].height;
          }
        }
      }
      break;

    case 180 :
      for (int p = 0; p < 3; ++p) {
        plane[p].dst += plane[p].width*plane[p].height;
        for (int y = plane[p].height; y > 0; --y) {
          for (int x = plane[p].width; x > 0; --x)
            *--plane[p].dst = *plane[p].src++;
        }
      }
      break;
  }

  if (!storage.IsEmpty())
    memcpy(srcYUV, dstYUV, size);

  return true;
}


bool PColourConverter::FillYUV420P(unsigned x, unsigned y, unsigned width, unsigned height,
                                   unsigned frameWidth, unsigned frameHeight, BYTE * yuv,
                                   unsigned r_or_y, unsigned g_or_u, unsigned b_or_v, bool rgb)
{
  if (frameWidth == 0)
     frameWidth = width;
  if (frameHeight == 0)
     frameHeight = height;

  if (frameWidth == 0 || frameHeight == 0 || x + width > frameWidth || y + height > frameHeight) {
    PAssertAlways(PInvalidParameter);
    return false;
  }

  BYTE Y, U, V;
  if (rgb)
    PColourConverter::RGBtoYUV(r_or_y, g_or_u, b_or_v, Y, U, V);
  else {
      Y = (BYTE)r_or_y;
      U = (BYTE)g_or_u;
      V = (BYTE)b_or_v;
  }

  unsigned planeWidth  = (frameWidth+1)&~1;
  unsigned planeHeight = (frameHeight+1)&~1;

  int halfRectWidth  = width/2;
  int halfPlaneWidth = planeWidth/2;

  unsigned char * Yptr  = yuv + y*planeWidth + x;
  unsigned char * Uptr = yuv + (planeWidth * planeHeight) + y/2*halfPlaneWidth + x/2;
  unsigned char * Vptr = Uptr + planeWidth * planeHeight / 4;

  for (unsigned dy = 0; dy < height; dy += 2) {
    memset(Yptr, Y, width);
    Yptr += planeWidth;
    memset(Yptr, Y, width);
    Yptr += planeWidth;

    memset(Uptr, U, halfRectWidth);
    memset(Vptr, V, halfRectWidth);

    Uptr += halfPlaneWidth;
    Vptr += halfPlaneWidth;
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////

#define greytoy(r, y) y=r
#define greytoyuv(r, y, u, v) greytoy(r,y); u=BLACK_U; v=BLACK_V

void PStandardColourConverter::GreytoYUV420PSameSize(const BYTE * grey, BYTE * yuv) const
{
  const unsigned planeSize = m_srcFrameWidth*m_srcFrameHeight;
  const unsigned halfWidth = m_srcFrameWidth >> 1;

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * greyIndex = grey;

  for (unsigned y = 0; y < m_srcFrameHeight; y++) {
    BYTE * yline  = yplane + (y * m_srcFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (m_verticalFlip)
      greyIndex = grey + m_srcFrameWidth*(m_srcFrameHeight-1-y);

    for (unsigned x = 0; x < m_srcFrameWidth; x+=2) {
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
void PStandardColourConverter::GreytoYUV420PWithCrop(const BYTE * grey, BYTE * yuv) const
{
  int planeSize = m_dstFrameWidth*m_dstFrameHeight;
  const int halfWidth = m_dstFrameWidth >> 1;
  unsigned min_width, min_height;

  min_width  = std::min(m_dstFrameWidth, m_srcFrameWidth);
  min_height = std::min(m_dstFrameHeight, m_srcFrameHeight);

  // get pointers to the data
  BYTE * yplane  = yuv;
  BYTE * uplane  = yuv + planeSize;
  BYTE * vplane  = yuv + planeSize + (planeSize >> 2);
  const BYTE * greyIndex = grey;

  for (unsigned y = 0; y < min_height; y++) 
  {
    BYTE * yline  = yplane + (y * m_dstFrameWidth);
    BYTE * uline  = uplane + ((y >> 1) * halfWidth);
    BYTE * vline  = vplane + ((y >> 1) * halfWidth);

    if (m_verticalFlip)
      greyIndex = grey + m_srcFrameWidth*(min_height-1-y);

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
    if (m_srcFrameWidth > m_dstFrameWidth)
      greyIndex += m_srcFrameWidth - m_dstFrameWidth;

    // Pad if dest width < source width
    if (m_dstFrameWidth > m_srcFrameWidth) {
      memset(yline, BLACK_Y, m_dstFrameWidth - m_srcFrameWidth);
      memset(uline, BLACK_U, (m_dstFrameWidth - m_srcFrameWidth)>>1);
      memset(vline, BLACK_V, (m_dstFrameWidth - m_srcFrameWidth)>>1);
    }
  }

  // Pad if dest height > source height
  if (m_dstFrameHeight > m_srcFrameHeight) {
    BYTE * yline  = yplane + (m_srcFrameHeight * m_dstFrameWidth);
    BYTE * uline  = uplane + ((m_srcFrameHeight >> 1) * halfWidth);
    BYTE * vline  = vplane + ((m_srcFrameHeight >> 1) * halfWidth);
    unsigned fill = (m_dstFrameHeight - m_srcFrameHeight) * m_dstFrameWidth;

    memset(yline, BLACK_Y, fill);
    memset(uline, BLACK_U, fill >> 2);
    memset(vline, BLACK_V, fill >> 2);
  }
}


bool PStandardColourConverter::GreytoYUV420P(const BYTE * grey, BYTE * yuv, PINDEX * bytesReturned) const
{
  if (grey == yuv) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if ((m_srcFrameWidth == m_dstFrameWidth) && (m_srcFrameHeight == m_dstFrameHeight)) 
    GreytoYUV420PSameSize(grey, yuv);
  else
    GreytoYUV420PWithCrop(grey, yuv);

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}


bool PStandardColourConverter::RGBtoYUV420P(const BYTE * srcFrameBuffer,
                                            BYTE * dstFrameBuffer,
                                            PINDEX * bytesReturned,
                                            unsigned rgbIncrement,
                                            unsigned redOffset,
                                            unsigned blueOffset)
{
  if (srcFrameBuffer == dstFrameBuffer) {
    PAssertAlways(PUnimplementedFunction);
    return false;
  }

  static const unsigned greenOffset = 1;

  const BYTE * scanLinePtrRGB = srcFrameBuffer;
  int scanLineSizeRGB = (rgbIncrement*m_srcFrameWidth+3)&~3;

  unsigned planeHeight = (m_dstFrameHeight+1)&~1;
  unsigned scanLineSizeY = (m_dstFrameWidth+1)&~1;
  unsigned scanLineSizeUV = scanLineSizeY/2;
  BYTE * scanLinePtrY = dstFrameBuffer;                                 // 1 byte Y (luminance) for each pixel
  BYTE * scanLinePtrU = scanLinePtrY+planeHeight*scanLineSizeY;    // 1 byte U for a block of 4 pixels
  BYTE * scanLinePtrV = scanLinePtrU+planeHeight*scanLineSizeUV/2; // 1 byte V for a block of 4 pixels

  if (m_verticalFlip) {
    scanLinePtrRGB += (m_srcFrameHeight - 1) * scanLineSizeRGB;
    scanLineSizeRGB = -scanLineSizeRGB;
  }

#if P_FFMPEG_SWSCALE

  if (CanUseFFMPEG(AV_PIX_FMT_NONE, AV_PIX_FMT_YUV420P, rgbIncrement, redOffset)) {
    const uint8_t* srcSlice[] = { scanLinePtrRGB };
    const int srcStride[] = { scanLineSizeRGB };

    uint8_t* dstSlice[] = { scanLinePtrY, scanLinePtrU, scanLinePtrV };
    const int dstStride[] = { (int)scanLineSizeY, (int)scanLineSizeUV, (int)scanLineSizeUV };

    sws_scale(m_swsContext, srcSlice, srcStride, 0, m_srcFrameHeight, dstSlice, dstStride);
    return true;
  }

#endif // P_FFMPEG_SWSCALE

  if (m_srcFrameWidth == scanLineSizeY && m_srcFrameHeight == planeHeight) {
    int RGBOffset[4] = { 0, (int)rgbIncrement, scanLineSizeRGB, scanLineSizeRGB+(int)rgbIncrement };
    unsigned YUVOffset[4] = { 0, 1, scanLineSizeY, scanLineSizeY + 1 };
    scanLineSizeRGB *= 2;
    rgbIncrement *= 2;
    for (unsigned y = 0; y < m_srcFrameHeight; y += 2) {
      const BYTE * pixelPtrRGB = scanLinePtrRGB;
      for (unsigned x = 0; x < m_srcFrameWidth; x += 2) {
        unsigned rSum = 0, gSum = 0, bSum = 0;
        for (unsigned p = 0; p < 4; ++p) {
          unsigned r = pixelPtrRGB[RGBOffset[p] +  redOffset];
          unsigned g = pixelPtrRGB[RGBOffset[p] + greenOffset];
          unsigned b = pixelPtrRGB[RGBOffset[p] +  blueOffset];
          scanLinePtrY[YUVOffset[p]] = RGBtoY(r, g, b);
          rSum += r;
          gSum += g;
          bSum += b;
        }
        rSum /= 4;
        gSum /= 4;
        bSum /= 4;
        *scanLinePtrU++ = RGBtoU(rSum, gSum, bSum);
        *scanLinePtrV++ = RGBtoV(rSum, gSum, bSum);
        pixelPtrRGB += rgbIncrement;
        scanLinePtrY += 2;
      }
      scanLinePtrY += m_srcFrameWidth;
      scanLinePtrRGB += scanLineSizeRGB;
    }
  }
  else {
    bool evenLine = true;
    PRasterDutyCycle raster(m_resizeMode, m_srcFrameWidth, m_srcFrameHeight, scanLineSizeY, planeHeight, 2, 1);
    do {
      while (raster.HasDutyY()) {
        const BYTE * pixelRGB1 = scanLinePtrRGB;
        const BYTE * pixelRGB2 = scanLinePtrRGB + rgbIncrement;
        BYTE * pixelY = scanLinePtrY;
        BYTE * pixelU = scanLinePtrU;
        BYTE * pixelV = scanLinePtrV;

        do {
          while (raster.HasDutyX()) {
            if (raster.IsBlack())
              pixelY[0] = pixelY[1] = 0;
            else {
              pixelY[0] = RGBtoY(pixelRGB1[redOffset], pixelRGB1[greenOffset], pixelRGB1[blueOffset]);
              pixelY[1] = RGBtoY(pixelRGB2[redOffset], pixelRGB2[greenOffset], pixelRGB2[blueOffset]);
              if (evenLine) {
                int rAvg = (pixelRGB1[  redOffset] + pixelRGB2[  redOffset]) / 2;
                int gAvg = (pixelRGB1[greenOffset] + pixelRGB2[greenOffset]) / 2;
                int bAvg = (pixelRGB1[ blueOffset] + pixelRGB2[ blueOffset]) / 2;
                *pixelU = RGBtoU(rAvg, gAvg, bAvg);
                *pixelV = RGBtoV(rAvg, gAvg, bAvg);
              }
            }

            pixelY += 2;
            pixelU++;
            pixelV++;
          }

          pixelRGB1 += rgbIncrement * 2;
          pixelRGB2 += rgbIncrement * 2;
        } while (raster.RunningX());

        scanLinePtrY += scanLineSizeY;
        if (evenLine) {
          scanLinePtrU += scanLineSizeUV;
          scanLinePtrV += scanLineSizeUV;
        }
        evenLine = !evenLine;
      }

      scanLinePtrRGB += scanLineSizeRGB;
    } while (raster.RunningY());
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}


PSTANDARD_COLOUR_CONVERTER(Grey,YUV420P)
{
  return GreytoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}


PSTANDARD_COLOUR_CONVERTER(RGB24,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3,  0, 2);
}


PSTANDARD_COLOUR_CONVERTER(BGR24,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3,  2, 0);
}


PSTANDARD_COLOUR_CONVERTER(RGB32,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 0, 2);
}


PSTANDARD_COLOUR_CONVERTER(BGR32,YUV420P)
{
  return RGBtoYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 2, 0);
}

/*
 * Format YUY2 or YUV422(non planar):
 *
 * off: 0  Y00 U00 Y01 V00 Y02 U01 Y03 V01
 * off: 8  Y10 U10 Y11 V10 Y12 U11 Y13 V11
 * off:16  Y20 U20 Y21 V20 Y22 U21 Y23 V21
 * off:24  Y30 U30 Y31 V30 Y32 U31 Y33 V31
 * length:32 bytes
 *
 * Format YUV420P:
 * off: 00  Y00 Y01 Y02 Y03
 * off: 04  Y10 Y11 Y12 Y13
 * off: 08  Y20 Y21 Y22 Y23
 * off: 12  Y30 Y31 Y32 Y33
 * off: 16  U00 U02 U20 U22
 * off: 20  V00 V02 V20 V22
 * 
 * So, we lose some bit of information when converting YUY2 to YUV420P
 *
 * NOTE: This algorithm works only if the width and the height is pair.
 */
void  PStandardColourConverter::YUY2toYUV420PSameSize(const BYTE *yuy2, BYTE *yuv420p) const
{
  const BYTE *s;
  BYTE *y, *u, *v;
  unsigned int x, h;  
  int npixels = m_srcFrameWidth * m_srcFrameHeight;

  s = yuy2;
  y = yuv420p;
  u = yuv420p + npixels;
  v = u + npixels/4;

  for (h=0; h<m_srcFrameHeight; h+=2) {

     /* Copy the first line keeping all information */
     for (x=0; x<m_srcFrameWidth; x+=2) {
        *y++ = *s++;
        *u++ = *s++;
        *y++ = *s++;
        *v++ = *s++;
     }
     /* Copy the second line discarding u and v information */
     for (x=0; x<m_srcFrameWidth; x+=2) {
        *y++ = *s++;
        s++;
        *y++ = *s++;
        s++;
     }
  }
}

/*
 * Format YUY2 or YUV422(non planar):
 *
 * off: 0  Y00 U00 Y01 V00 Y02 U01 Y03 V01
 * off: 8  Y10 U10 Y11 V10 Y12 U11 Y13 V11
 * off:16  Y20 U20 Y21 V20 Y22 U21 Y23 V21
 * off:24  Y30 U30 Y31 V30 Y32 U31 Y33 V31
 * length:32 bytes
 *
 * Format YUV420P:
 * off: 00  Y00 Y01 Y02 Y03
 * off: 04  Y10 Y11 Y12 Y13
 * off: 08  Y20 Y21 Y22 Y23
 * off: 12  Y30 Y31 Y32 Y33
 * off: 16  U00 U02 U20 U22
 * off: 20  V00 V02 V20 V22
 * 
 * So, we lose some bit of information when converting YUY2 to YUV420P
 *
 * NOTE: This algorithm works only if the width and the height are even numbers.
 */
void PStandardColourConverter::YUY2toYUV420PWithGrow(const BYTE *yuy2, BYTE *yuv420p) const
{
  const BYTE *s;
  BYTE *y, *u, *v;
  unsigned int x, h;  
  unsigned int npixels = m_dstFrameWidth * m_dstFrameHeight;

  s = yuy2;
  y = yuv420p;
  u = yuv420p + npixels;
  v = u + npixels/4;

  // dest is bigger than the source. No subsampling.
  // Place the src in the middle of the destination.
  unsigned int yOffset = (m_dstFrameHeight - m_srcFrameHeight)/2;
  unsigned int xOffset = (m_dstFrameWidth - m_srcFrameWidth)/2;
  unsigned int bpixels = yOffset * m_dstFrameWidth;

  /* Top border */
  memset(y, BLACK_Y, bpixels);   y += bpixels;
  memset(u, BLACK_U, bpixels/4); u += bpixels/4;
  memset(v, BLACK_V, bpixels/4); v += bpixels/4;

  for (h=0; h<m_srcFrameHeight; h+=2)
  {
    /* Left border */
    memset(y, BLACK_Y, xOffset);   y += xOffset;
    memset(u, BLACK_U, xOffset/2); u += xOffset/2;
    memset(v, BLACK_V, xOffset/2); v += xOffset/2;

    /* Copy the first line keeping all information */
    for (x=0; x<m_srcFrameWidth; x+=2)
    {
      *y++ = *s++;
      *u++ = *s++;
      *y++ = *s++;
      *v++ = *s++;
    }
    /* Right and Left border */
    for (x=0; x<xOffset*2; x++)
      *y++ = BLACK_Y;

    /* Copy the second line discarding u and v information */
    for (x=0; x<m_srcFrameWidth; x+=2)
    {
      *y++ = *s++;
      s++;
      *y++ = *s++;
      s++;
    }
    /* Fill the border with black (right side) */
    memset(y, BLACK_Y, xOffset);        y += xOffset;
    memset(u, BLACK_U, xOffset/2);        u += xOffset/2;
    memset(v, BLACK_V, xOffset/2);        v += xOffset/2;
  }
  memset(y, BLACK_Y, bpixels);
  memset(u, BLACK_U, bpixels/4);
  memset(v, BLACK_V, bpixels/4);
}


void PStandardColourConverter::YUY2toYUV420PWithShrink(const BYTE *yuy2, BYTE *yuv420p) const
{
  const BYTE *s;
  BYTE *y, *u, *v;
  unsigned int x, h;  
  unsigned int npixels = m_dstFrameWidth * m_dstFrameHeight;

  s = yuy2;
  y = yuv420p;
  u = yuv420p + npixels;
  v = u + npixels/4;

  // source is bigger than the destination
  // We are doing linear interpolation to find value.
  // Note this algorithm only works if dst is an even multple of src
  unsigned int dx = m_srcFrameWidth/m_dstFrameWidth;
  unsigned int dy = m_srcFrameHeight/m_dstFrameHeight;
  unsigned int fy, fx;

  for (fy=0, h=0; h<m_dstFrameHeight; h+=2, fy+=dy*2)
  {
    /* Copy the first line with U&V */
    unsigned int yy = fy;
    unsigned int yy2 = (fy+dy);
    const unsigned char *line1, *line2;
    unsigned char lastU, lastV;

    line1 = s + (yy*2*m_srcFrameWidth);
    line2 = s + (yy2*2*m_srcFrameWidth);
    lastU = line1[1];
    lastV = line1[3];
    for (fx=0, x=0; x<m_dstFrameWidth; x+=2, fx+=dx*2)
    {
      unsigned int xx = fx*2;
      *y++ = line1[xx];
      if ( (xx&2) == 0)
      {
        *u++ = lastU = (line1[xx+1] + line2[xx+1])/2;
        *v++ = lastV = (line1[xx+3] + line2[xx+3])/2;
      }
      else
      {
        *u++ = lastU;
        *v++ = lastV = (line1[xx+1] + line2[xx+1])/2;
      }
      
      xx = (fx+dx);
      *y++ = line1[xx];
      if ( (xx&2) == 0)
        lastU = (line1[xx+1] + line2[xx+1])/2;
      else
        lastV = (line1[xx+3] + line2[xx+3])/2;
    }

    /* Copy the second line without U&V */
    for (fx=0, x=0; x<m_dstFrameWidth; x++, fx+=dx)
    {
      unsigned int xx = fx*2;
      *y++ = line2[xx];
    }
  } /* end of for (fy=0, h=0; h<m_dstFrameHeight; h+=2, fy+=dy*2) */
}


PSTANDARD_COLOUR_CONVERTER(YUY2,YUV420P)
{
  if (!ValidateDimensions(m_srcFrameWidth, m_srcFrameHeight, m_dstFrameWidth, m_dstFrameHeight, m_resizeMode, NULL))
    return false;

  if (m_dstFrameWidth == m_srcFrameWidth)
    YUY2toYUV420PSameSize(srcFrameBuffer, dstFrameBuffer);
  else if (m_dstFrameWidth < m_srcFrameWidth)
    YUY2toYUV420PWithShrink(srcFrameBuffer, dstFrameBuffer);
  else
    YUY2toYUV420PWithGrow(srcFrameBuffer, dstFrameBuffer);

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
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

void PStandardColourConverter::YUV422WithCrop(const BYTE * src, BYTE * dest, bool centred) const
{
  DWORD *result = (DWORD *)dest;
  DWORD black   = (DWORD)(BLACK_U<<24) + (BLACK_Y<<16) + (BLACK_U<<8) + BLACK_Y;
  unsigned maxIndex    = m_dstFrameWidth*m_dstFrameHeight/2;

  if ( (m_dstFrameWidth*m_dstFrameHeight) > (m_srcFrameWidth*m_srcFrameHeight) ) { 
    for (unsigned i = 0; i < maxIndex; i++) 
      *result++ = black;

    //dest is bigger than the source. No subsampling.
    //Place the src in the middle of the destination.
    unsigned yOffset = centred ? m_dstFrameHeight - m_srcFrameHeight : 0;
    unsigned xOffset = centred ? m_dstFrameWidth - m_srcFrameWidth : 0;

    BYTE *s_ptr,*d_ptr;
    d_ptr = (yOffset * m_dstFrameWidth) + xOffset + dest;
    s_ptr = (BYTE *)src;
    for (unsigned y = 0; y < m_srcFrameHeight; y++) {
      memcpy(d_ptr,s_ptr, m_srcFrameWidth*2);
      d_ptr += 2*m_dstFrameWidth;
      s_ptr += 2*m_srcFrameWidth;
    }
  } else {  
    // source is bigger than the destination.
    //
    unsigned subSample  = 1 + (m_srcFrameHeight/m_dstFrameHeight) ;
    unsigned yOffset    = m_dstFrameHeight - (m_srcFrameHeight/subSample);
    unsigned xOffset    = m_dstFrameWidth - (m_srcFrameWidth/subSample);
    unsigned subSample2 = subSample*2;

    DWORD *s_ptr = (DWORD * )src;
    DWORD *d_ptr = (DWORD *) dest + ((yOffset * m_dstFrameWidth) + xOffset)/4 ;
    DWORD *sl_ptr, *dl_ptr;

    for (unsigned y = 0; y < m_srcFrameHeight; y+= subSample) {
      sl_ptr = s_ptr;
      dl_ptr = d_ptr;
      for (unsigned x = 0; x < m_srcFrameWidth; x+= subSample2) {
        *dl_ptr++ = *sl_ptr;
        sl_ptr += subSample;
      }
      d_ptr += m_dstFrameWidth/2;
      s_ptr += m_srcFrameWidth*subSample/2;
    }
  }
}


PSTANDARD_COLOUR_CONVERTER(YUV422,YUV422)
{
  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  
  if (srcFrameBuffer == dstFrameBuffer)
    return true;

  if ((m_srcFrameWidth == m_dstFrameWidth) && (m_srcFrameHeight == m_dstFrameHeight)) 
    memcpy(dstFrameBuffer,srcFrameBuffer,m_srcFrameWidth*m_srcFrameHeight*2);
  else
    YUV422WithCrop(srcFrameBuffer, dstFrameBuffer, m_resizeMode == PVideoFrameInfo::eCropCentre);

  return true;
}


PSTANDARD_COLOUR_CONVERTER(YUV420P,YUV420P)
{
  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  
  if (srcFrameBuffer == dstFrameBuffer) {
    if (m_srcFrameWidth == m_dstFrameWidth && m_srcFrameHeight == m_dstFrameHeight) 
      return true;
    if(m_srcFrameWidth < m_dstFrameWidth || m_srcFrameHeight < m_dstFrameHeight) {
      PTRACE(2,"Cannot do in place conversion, increasing image size.");
      return false;
    }
  }

  return CopyYUV420P(0, 0, m_srcFrameWidth, m_srcFrameHeight, m_srcFrameWidth, m_srcFrameHeight, srcFrameBuffer,
                     0, 0, m_dstFrameWidth, m_dstFrameHeight, m_dstFrameWidth, m_dstFrameHeight, dstFrameBuffer,
                     m_resizeMode, m_verticalFlip);
}

/*
 * Format YUY2 or YUV422(non planar):
 *
 * off: 0  Y00 U00 Y01 V00 Y02 U01 Y03 V01
 * off: 8  Y10 U10 Y11 V10 Y12 U11 Y13 V11
 * off:16  Y20 U20 Y21 V20 Y22 U21 Y23 V21
 * off:24  Y30 U30 Y31 V30 Y32 U31 Y33 V31
 * length:32 bytes
 *
 * Format YUV420P:
 * off: 00  Y00 Y01 Y02 Y03
 * off: 04  Y10 Y11 Y12 Y13
 * off: 08  Y20 Y21 Y22 Y23
 * off: 12  Y30 Y31 Y32 Y33
 * off: 16  U00 U02 U20 U22
 * off: 20  V00 V02 V20 V22
 * 
 * So, we lose some bit of information when converting YUY2 to YUV420 
 *
 */
PSTANDARD_COLOUR_CONVERTER(YUV422,YUV420P)
{
  if (!ValidateDimensions(m_srcFrameWidth, m_srcFrameHeight, m_dstFrameWidth, m_dstFrameHeight, m_resizeMode, NULL))
    return false;

  if (m_dstFrameWidth == m_srcFrameWidth)
    YUY2toYUV420PSameSize(srcFrameBuffer, dstFrameBuffer);
  else if (m_dstFrameWidth < m_srcFrameWidth)
    YUY2toYUV420PWithShrink(srcFrameBuffer, dstFrameBuffer);
  else
    YUY2toYUV420PWithGrow(srcFrameBuffer, dstFrameBuffer);

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}


static inline int clip(int a, int limit) {
  return a<limit?a:limit;
}

bool PStandardColourConverter::SBGGR8toYUV420P(const BYTE * src, BYTE * dst, PINDEX * bytesReturned)
{
#define USE_SBGGR8_NATIVE 1 // set to 0 to use the double conversion algorithm (Bayer->RGB->YUV420P)
  
#if USE_SBGGR8_NATIVE
if ((m_srcFrameWidth == m_dstFrameWidth) && (m_srcFrameHeight == m_dstFrameHeight))
{
  // kernels for Y conversion, normalised by 2^16
  const int kR[]={1802,9667,1802,9667,19661,9667,1802,9667,1802}; 
  const int kG1[]={7733,9830,7733,3604,7733,3604,7733,9830,7733};
  const int kG2[]={7733,3604,7733,9830,7733,9830,7733,3604,7733};
  const int kB[]={4915,9667,4915,9667,7209,9667,4915,9667,4915};
  //  const int kID[]={0,0,0,0,65536,0,0,0,0}; identity kernel, use to test

  int B, G, G1, G2, R;
  const int stride = m_srcFrameWidth;
  unsigned const int hSize =m_srcFrameHeight/2;
  unsigned const int vSize =m_srcFrameWidth/2;
  unsigned const int lastRow=m_srcFrameHeight-1;
  unsigned const int lastCol=m_srcFrameWidth-1;
  unsigned int i,j;
  const BYTE *sBayer = src;

  //  Y = round( 0.256788 * R + 0.504129 * G + 0.097906 * B) +  16;
  //  Y = round( 0.30 * R + 0.59 * G + 0.11 * B ) use this!
  //  U = round(-0.148223 * R - 0.290993 * G + 0.439216 * B) + 128;
  //  V = round( 0.439216 * R - 0.367788 * G - 0.071427 * B) + 128;

  // Compute U and V planes using EXACT values, reading 2x2 pixels at a time
  BYTE *dU = dst+m_srcFrameHeight*m_srcFrameWidth;
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
  for (i=0; i<m_srcFrameHeight; i++) {
    // Pointer to previous row, to the next if we are on the first one
    sBayerTop=sBayer+(i?(-stride):stride);
    // Pointer to next row, to the previous one if we are on the last
    sBayerBottom=sBayer+((i<lastRow)?stride:(-stride));
    // offset to previous column, to the next if we are on the first col
    dxLeft=1;
    for (j=0; j<m_srcFrameWidth; j++) {
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
    *bytesReturned = m_srcFrameHeight*m_srcFrameWidth+2*hSize*vSize;

  return true;
}
else
#endif //USE_SBGGR8_NATIVE
{
  // shortest but less efficient (one malloc per conversion!)
  BYTE * tempDest=(BYTE*)malloc(3*m_srcFrameWidth*m_srcFrameHeight);
  SBGGR8toRGB(src, tempDest, NULL);
  bool r = RGBtoYUV420P(tempDest, dst, bytesReturned, 3, 0, 2);
  free(tempDest);
  return r;
}
}

bool PStandardColourConverter::SBGGR8toRGB(const BYTE * src,
                                           BYTE       * dst,
                                           PINDEX     * bytesReturned) const
{
  if (src == dst) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if (m_verticalFlip) {
    PTRACE(2,"Cannot do vertical flip, not implemented.");
    return false;
  }

  long int i;
  const BYTE *rawpt;
  BYTE *scanpt;
  long int size;

  rawpt = src;
  scanpt = dst;
  long int WIDTH = m_srcFrameWidth, HEIGHT = m_srcFrameHeight;
  size = WIDTH*HEIGHT;

  for ( i = 0; i < size; i++ ) {
    if ( (i/WIDTH) % 2 == 0 ) {
      if ( (i % 2) == 0 ) {
        /* B */
        if ( (i > WIDTH) && ((i % WIDTH) > 0) ) {
          *scanpt++ = (BYTE) ((*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+ *(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4);  /* R */
          *scanpt++ = (BYTE) ((*(rawpt-1)+*(rawpt+1)+ *(rawpt+WIDTH)+*(rawpt-WIDTH))/4);  /* G */
          *scanpt++ = *rawpt;         /* B */
        } else {
          /* first line or left column */
          *scanpt++ = *(rawpt+WIDTH+1);   /* R */
          *scanpt++ = (BYTE) ((*(rawpt+1)+*(rawpt+WIDTH))/2); /* G */
          *scanpt++ = *rawpt;       /* B */
        }
      } else {
        /* (B)G */
        if ( (i > WIDTH) && ((i % WIDTH) < (WIDTH-1)) ) {
          *scanpt++ = (BYTE) ((*(rawpt+WIDTH)+*(rawpt-WIDTH))/2); /* R */
          *scanpt++ = *rawpt;         /* G */
          *scanpt++ = (BYTE) ((*(rawpt-1)+*(rawpt+1))/2);   /* B */
        } else {
          /* first line or right column */
          *scanpt++ = *(rawpt+WIDTH); /* R */
          *scanpt++ = *rawpt;   /* G */
          *scanpt++ = *(rawpt-1); /* B */
        }
      }
    } else {
      if ( (i % 2) == 0 ) {
        /* G(R) */
        if ( (i < (WIDTH*(HEIGHT-1))) && ((i % WIDTH) > 0) ) {
          *scanpt++ = (BYTE) ((*(rawpt-1)+*(rawpt+1))/2);   /* R */
          *scanpt++ = *rawpt;         /* G */
          *scanpt++ = (BYTE) ((*(rawpt+WIDTH)+*(rawpt-WIDTH))/2); /* B */
        } else {
          /* bottom line or left column */
          *scanpt++ = *(rawpt+1);   /* R */
          *scanpt++ = *rawpt;         /* G */
          *scanpt++ = *(rawpt-WIDTH);   /* B */
        }
      } else {
        /* R */
        if ( i < (WIDTH*(HEIGHT-1)) && ((i % WIDTH) < (WIDTH-1)) ) {
          *scanpt++ = *rawpt;         /* R */
          *scanpt++ = (BYTE) ((*(rawpt-1)+*(rawpt+1)+*(rawpt-WIDTH)+*(rawpt+WIDTH))/4);  /* G */
          *scanpt++ = (BYTE) ((*(rawpt-WIDTH-1)+*(rawpt-WIDTH+1)+*(rawpt+WIDTH-1)+*(rawpt+WIDTH+1))/4);  /* B */
        } else {
          /* bottom line or right column */
          *scanpt++ = *rawpt;        /* R */
          *scanpt++ = (BYTE) ((*(rawpt-1)+*(rawpt-WIDTH))/2);  /* G */
          *scanpt++ = *(rawpt-WIDTH-1);    /* B */
        }
      }
    }
    rawpt++;
  }

  if (bytesReturned)
    *bytesReturned = scanpt - dst;

  return true;
}


typedef int FixedPoint; // Best to be native integer size
#define ScaleBitShift 12
static FixedPoint const HalfFixedScaling = 1 << (ScaleBitShift - 1);

#define ROUND(x) ((x) + HalfFixedScaling)
#define CLAMP(x) (BYTE)(((x) < 0 ? 0 : ((x) >= (255<<ScaleBitShift) ? 255 : ((x)>>ScaleBitShift))))

#define FIX_FROM_FLOAT(x)    ((int) ((x) * (1UL<<ScaleBitShift) + 0.5))
static FixedPoint const YUVtoR_Coeff  =  FIX_FROM_FLOAT(1.40200);
static FixedPoint const YUVtoG_Coeff1 = -FIX_FROM_FLOAT(0.34414);
static FixedPoint const YUVtoG_Coeff2 =  FIX_FROM_FLOAT(0.71414);
static FixedPoint const YUVtoB_Coeff  =  FIX_FROM_FLOAT(1.77200);
#undef FIX_FROM_FLOAT

/* 
 * Please note when converting colorspace from YUV to RGB.
 * Not all YUV have the same colorspace. 
 *
 * For instance Jpeg use this formula
 * YCbCr is defined per CCIR 601-1, except that Cb and Cr are
 * normalized to the range 0..MAXJSAMPLE rather than -0.5 .. 0.5.
 * The conversion equations to be implemented are therefore
 *      Y  =  0.29900 * R + 0.58700 * G + 0.11400 * B
 *      Cb = -0.16874 * R - 0.33126 * G + 0.50000 * B  + CENTERJSAMPLE
 *      Cr =  0.50000 * R - 0.41869 * G - 0.08131 * B  + CENTERJSAMPLE
 * (These numbers are derived from TIFF 6.0 section 21, dated 3-June-92.)
 * So
 * R = Y + 1.402 (Cr-128)
 * G = Y - 0.34414 (Cb-128) - 0.71414 (Cr-128)
 * B = Y + 1.772 (Cb-128)
 * 
 */

void PColourConverter::YUVtoRGB(unsigned y, unsigned u, unsigned v,
                                BYTE   & r, BYTE   & g, BYTE   & b)
{
    FixedPoint cb = u - 128;
    FixedPoint cr = v - 128;
    FixedPoint rd = ROUND(YUVtoR_Coeff * cr);
    FixedPoint gd = ROUND(YUVtoG_Coeff1 * cb - YUVtoG_Coeff2 * cr);
    FixedPoint bd = ROUND(YUVtoB_Coeff * cb);
    FixedPoint yvalue = y << ScaleBitShift;
    FixedPoint rvalue = yvalue + rd;
    FixedPoint gvalue = yvalue + gd;
    FixedPoint bvalue = yvalue + bd;
    r = CLAMP(rvalue);
    g = CLAMP(gvalue);
    b = CLAMP(bvalue);
}


bool PStandardColourConverter::YUV420PtoRGB(const BYTE * srcFrameBuffer,
                                            BYTE * dstFrameBuffer,
                                            PINDEX * bytesReturned,
                                            unsigned rgbIncrement,
                                            unsigned redOffset,
                                            unsigned blueOffset)
{
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  unsigned planeWidth = (m_srcFrameWidth+1)&~1;
  unsigned planeHeight = (m_srcFrameHeight+1)&~1;
  unsigned yPlaneSize = planeWidth*planeHeight;
  const BYTE * scanLinePtrY = srcFrameBuffer;            // 1 byte Y (luminance) for each pixel
  const BYTE * scanLinePtrU = scanLinePtrY+yPlaneSize;   // 1 byte U for a block of 4 pixels
  const BYTE * scanLinePtrV = scanLinePtrU+yPlaneSize/4; // 1 byte V for a block of 4 pixels

#if P_IPP
  if (!m_verticalFlip && m_srcFrameWidth == m_dstFrameWidth && m_srcFrameHeight == m_dstFrameHeight && g_intel.IsLoaded()) {
    const Ipp8u * srcPlanes[3] = { scanLinePtrY, scanLinePtrU, scanLinePtrV };
    int srcStep[3] = { (int)m_srcFrameWidth, (int)m_srcFrameWidth/2, (int)m_srcFrameWidth/2 };
    int dstStep = m_srcFrameWidth*rgbIncrement;
    IppiSize size;
    size.width = m_srcFrameWidth;
    size.height = m_srcFrameHeight;
    IppStatus status;
    if (blueOffset == 0) {
      if (rgbIncrement == 3)
        status = g_intel.ippiYCbCr420ToBGR_8u_P3C3R(srcPlanes, srcStep, dstFrameBuffer, dstStep, size);
      else
        status = g_intel.ippiYCbCr420ToBGR_8u_P3C4R(srcPlanes, srcStep, dstFrameBuffer, dstStep, size, 0);
    }
    else {
      if (rgbIncrement == 3)
        status = g_intel.ippiYUV420ToRGB_8u_P3C3(srcPlanes, dstFrameBuffer, size);
      else
        status = g_intel.ippiYUV420ToRGB_8u_P3AC4R(srcPlanes, srcStep, dstFrameBuffer, dstStep, size);
    }
    if (status == ippStsNoErr)
      return true;
  }
#endif

  BYTE * scanLinePtrRGB = dstFrameBuffer;
  int scanLineSizeRGB = (int)((rgbIncrement*m_dstFrameWidth+3)&~3);
  if (m_verticalFlip) {
    scanLinePtrRGB += (m_dstFrameHeight - 1) * scanLineSizeRGB;
    scanLineSizeRGB = -scanLineSizeRGB;
  }

#if P_FFMPEG_SWSCALE

  if (CanUseFFMPEG(AV_PIX_FMT_YUV420P, AV_PIX_FMT_NONE, rgbIncrement, redOffset)) {
    const uint8_t* srcSlice[] = { scanLinePtrY, scanLinePtrU, scanLinePtrV };
    const int srcStride[] = { (int)planeWidth, (int)planeWidth/2, (int)planeWidth/2 };

    uint8_t* dstSlice[] = { scanLinePtrRGB };
    const int dstStride[] = { scanLineSizeRGB };

    sws_scale(m_swsContext, srcSlice, srcStride, 0, m_srcFrameHeight, dstSlice, dstStride);
    return true;
  }

#endif // P_FFMPEG_SWSCALE

  unsigned srcPixpos[4] = { 0, 1, planeWidth, planeWidth + 1 };
  unsigned dstPixpos[4];

  if (m_verticalFlip) {
    scanLinePtrRGB -= scanLineSizeRGB;
    dstPixpos[0] = dstPixpos[2];
    dstPixpos[1] = dstPixpos[3];
    dstPixpos[2] = 0;
    dstPixpos[3] = rgbIncrement;
  }
  else {
    dstPixpos[0] = 0;
    dstPixpos[1] = rgbIncrement;
    dstPixpos[2] = (unsigned)scanLineSizeRGB;
    dstPixpos[3] = (unsigned)scanLineSizeRGB+rgbIncrement;
  }

  scanLineSizeRGB *= 2;

  static const unsigned greenOffset = 1;

#define YUV420PtoRGB_PIXEL_UV(pixelU, pixelV) \
    FixedPoint cb = *pixelU - 128; \
    FixedPoint cr = *pixelV - 128; \
    FixedPoint rd = ROUND(YUVtoR_Coeff * cr); \
    FixedPoint gd = ROUND(YUVtoG_Coeff1 * cb - YUVtoG_Coeff2 * cr); \
    FixedPoint bd = ROUND(YUVtoB_Coeff * cb);
#define YUV420PtoRGB_PIXEL_RGB(pixelY) \
    FixedPoint yvalue = pixelY[srcPixpos[p]] << ScaleBitShift; \
    FixedPoint rvalue = yvalue + rd; \
    FixedPoint gvalue = yvalue + gd; \
    FixedPoint bvalue = yvalue + bd; \
    rgbPtr[redOffset]   = CLAMP(rvalue); \
    rgbPtr[greenOffset] = CLAMP(gvalue); \
    rgbPtr[blueOffset]  = CLAMP(bvalue);

  if (m_srcFrameWidth == m_dstFrameWidth && m_srcFrameHeight == m_dstFrameHeight) {
    for (unsigned y = 0; y < m_srcFrameHeight; y += 2) {
      BYTE * pixelRGB = scanLinePtrRGB;
      for (unsigned x = 0; x < m_srcFrameWidth; x += 2) {
        unsigned pixels = x < m_srcFrameWidth-1 ? 4 : 2;
        YUV420PtoRGB_PIXEL_UV(scanLinePtrU, scanLinePtrV);
        for (unsigned p = 0; p < pixels; p++) {
          BYTE * rgbPtr = pixelRGB + dstPixpos[p];
          YUV420PtoRGB_PIXEL_RGB(scanLinePtrY);
          if (rgbIncrement == 4)
            rgbPtr[3] = 0;
        }
        pixelRGB += rgbIncrement*2;
        scanLinePtrY += 2;
        scanLinePtrU++;
        scanLinePtrV++;
      }
      scanLinePtrRGB += scanLineSizeRGB;
      scanLinePtrY += planeWidth;
    }
  }
  else {
    unsigned scanLineSizeY = planeWidth*2; // Actually two scan lines
    unsigned scanLineSizeUV = planeWidth/2;

    PRasterDutyCycle raster(m_resizeMode, m_srcFrameWidth, m_srcFrameHeight, m_dstFrameWidth, m_dstFrameHeight, 2, 2);
    do {
      while (raster.HasDutyY()) {
        BYTE * pixelRGB = scanLinePtrRGB;
        const BYTE * pixelY = scanLinePtrY;
        const BYTE * pixelU = scanLinePtrU;
        const BYTE * pixelV = scanLinePtrV;

        do {
          // The RGB value without luminance
          YUV420PtoRGB_PIXEL_UV(pixelU, pixelV);

          while (raster.HasDutyX()) {
            // Add luminance to each of the 4 pixels
            for (unsigned p = 0; p < 4; p++) {
              BYTE * rgbPtr = pixelRGB + dstPixpos[p];

              if (raster.IsBlack())
                rgbPtr[0] = rgbPtr[1] = rgbPtr[2] = 0;
              else {
                YUV420PtoRGB_PIXEL_RGB(pixelY);
              }

              if (rgbIncrement == 4)
                rgbPtr[3] = 0;
            }

            pixelRGB += rgbIncrement*2;
          }

          pixelY += 2;
          pixelU++;
          pixelV++;
        } while (raster.RunningX());

        scanLinePtrRGB += scanLineSizeRGB;
      }

      scanLinePtrY += scanLineSizeY;
      scanLinePtrU += scanLineSizeUV;
      scanLinePtrV += scanLineSizeUV;
    } while (raster.RunningY());
  }

  return true;
}


PBoolean PStandardColourConverter::YUV420PtoRGB565(const BYTE * srcFrameBuffer,
                                            BYTE * dstFrameBuffer,
                                            PINDEX * bytesReturned) const
{
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  static const unsigned rgbIncrement = 2;

  unsigned height = PMIN(m_srcFrameHeight, m_dstFrameHeight)&(UINT_MAX-1); // Must be even
  unsigned width = PMIN(m_srcFrameWidth, m_dstFrameWidth)&(UINT_MAX-1);

  unsigned    yplanesize = m_srcFrameWidth*m_srcFrameHeight;
  const BYTE *yplane     = srcFrameBuffer;        // 1 byte Y (luminance) for each pixel
  const BYTE *uplane     = yplane+yplanesize;     // 1 byte U for a block of 4 pixels
  const BYTE *vplane     = uplane+(yplanesize/4); // 1 byte V for a block of 4 pixels

  BYTE * dstScanLine   = dstFrameBuffer;

  unsigned int srcPixpos[4] = { 0, 1, m_srcFrameWidth, m_srcFrameWidth + 1 };
  unsigned int dstPixpos[4] = { 0, rgbIncrement, m_dstFrameWidth*rgbIncrement, (m_dstFrameWidth+1)*rgbIncrement };

  if (m_verticalFlip) {
    dstScanLine += (m_dstFrameHeight - 2) * m_dstFrameWidth * rgbIncrement;
    dstPixpos[0] = m_dstFrameWidth*rgbIncrement;
    dstPixpos[1] = (m_dstFrameWidth +1)*rgbIncrement;
    dstPixpos[2] = 0;
    dstPixpos[3] = 1*rgbIncrement;
  }

  for (unsigned y = 0; y < height; y += 2)
  {
    BYTE * dstPixelGroup = dstScanLine;
    for (unsigned x = 0; x < width; x += 2)
    {
      // The RGB value without luminance
      FixedPoint cb = *uplane-128;
      FixedPoint cr = *vplane-128;
      FixedPoint rd = ROUND(YUVtoR_Coeff * cr);
      FixedPoint gd = ROUND(YUVtoG_Coeff1 * cb - YUVtoG_Coeff2 * cr);
      FixedPoint bd = ROUND(YUVtoB_Coeff * cb);

      // Add luminance to each of the 4 pixels

      for (unsigned p = 0; p < 4; p++) {
        FixedPoint yvalue = *(yplane + srcPixpos[p]) << ScaleBitShift;
        FixedPoint rvalue = yvalue + rd;
        FixedPoint gvalue = yvalue + gd;
        FixedPoint bvalue = yvalue + bd;
        *(WORD *)(dstPixelGroup + dstPixpos[p]) = ((((CLAMP(rvalue) >> 3) & 0x001f) << 11) & (0xf800))
                                                | ((((CLAMP(gvalue) >> 2) & 0x003f) << 5 ) & (0x07e0))
                                                | ((((CLAMP(bvalue) >> 3) & 0x001f) ) & (0x001f));
      }

      yplane += 2;
      dstPixelGroup += rgbIncrement*2;

      uplane++;
      vplane++;
    }
 
    yplane += m_srcFrameWidth;

    dstScanLine += (m_verticalFlip?-2:2)*rgbIncrement*m_dstFrameWidth;
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}


#if P_FFMPEG_SWSCALE

PSTANDARD_COLOUR_CONVERTER(YUV420B,YUV420P)
{
  if (!CanUseFFMPEG(AV_PIX_FMT_NV12, AV_PIX_FMT_YUV420P, 0, 0))
    return false;
  
  const uint8_t* srcSlice[] = { srcFrameBuffer, srcFrameBuffer+m_srcFrameWidth*m_srcFrameHeight };
  const int srcStride[] = { (int)m_srcFrameWidth, (int)m_srcFrameWidth };
  
  int planeHeight = (m_dstFrameHeight+1)&~1;
  int scanLineSizeY = (m_dstFrameWidth+1)&~1;
  int scanLineSizeUV = scanLineSizeY/2;
  int planeSizeY = planeHeight*scanLineSizeY;
  int planeSizeUV = planeHeight/2*scanLineSizeUV;

  uint8_t* dstSlice[] = { dstFrameBuffer, dstFrameBuffer+planeSizeY, dstFrameBuffer+planeSizeY+planeSizeUV };
  const int dstStride[] = { scanLineSizeY, scanLineSizeUV, scanLineSizeUV };
    
  sws_scale(m_swsContext, srcSlice, srcStride, 0, m_srcFrameHeight, dstSlice, dstStride);
  
  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  
  return true;
}

#endif // P_FFMPEG_SWSCALE

PSTANDARD_COLOUR_CONVERTER(SBGGR8,RGB24)
{
  return SBGGR8toRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}

PSTANDARD_COLOUR_CONVERTER(SBGGR8,YUV420P)
{
  return SBGGR8toYUV420P(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB24)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 0, 2);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,BGR24)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 2, 0);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB32)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 0, 2);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,BGR32)
{
  return YUV420PtoRGB(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 2, 0);
}


PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB565)
{
  return YUV420PtoRGB565(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}

PSTANDARD_COLOUR_CONVERTER(YUV420P,RGB16)
{
  return YUV420PtoRGB565(srcFrameBuffer, dstFrameBuffer, bytesReturned);
}


static void SwapRedAndBlueRow(const BYTE * srcRowPtr,
                              BYTE * dstRowPtr,
                              unsigned width,
                              unsigned srcIncrement,
                              unsigned dstIncrement)
{
  for (unsigned x = 0; x < width; x++) {
    BYTE temp = srcRowPtr[0]; // Do it this way in case src and dst are same buffer
    dstRowPtr[0] = srcRowPtr[2];
    dstRowPtr[1] = srcRowPtr[1];
    dstRowPtr[2] = temp;

    srcRowPtr += srcIncrement;
    dstRowPtr += dstIncrement;
  }
}

bool PStandardColourConverter::SwapRedAndBlue(const BYTE * srcFrameBuffer,
                                              BYTE * dstFrameBuffer,
                                              PINDEX * bytesReturned,
                                              unsigned srcIncrement,
                                              unsigned dstIncrement) const
{
  if ((m_dstFrameWidth != m_srcFrameWidth) || (m_dstFrameHeight != m_srcFrameHeight)) {
    PTRACE(2,"Cannot do different sized RGB swap, not implemented.");
    return false;
  }

  unsigned srcRowSize = m_srcFrameBytes/m_srcFrameHeight;
  const BYTE * srcRowPtr = srcFrameBuffer;

  unsigned dstRowSize = m_dstFrameBytes/m_dstFrameHeight;
  BYTE * dstRowPtr = dstFrameBuffer;

  if (m_verticalFlip) {
    dstRowPtr += m_dstFrameHeight*dstRowSize;

    if (srcFrameBuffer == dstFrameBuffer) {
      PBYTEArray tempRow(PMAX(srcRowSize, dstRowSize));
      unsigned halfHeight = (m_srcFrameHeight+1)/2;
      for (unsigned y = 0; y < halfHeight; y++) {
        dstRowPtr -= dstRowSize;
        SwapRedAndBlueRow(dstRowPtr, tempRow.GetPointer(), m_dstFrameWidth, srcIncrement, dstIncrement);
        SwapRedAndBlueRow(srcRowPtr, dstRowPtr, m_srcFrameWidth, srcIncrement, dstIncrement);
        memcpy((BYTE *)srcRowPtr, tempRow, srcRowSize);
        srcRowPtr += srcRowSize;
      }
    }
    else {
      for (unsigned y = 0; y < m_srcFrameHeight; y++) {
        dstRowPtr -= dstRowSize;
        SwapRedAndBlueRow(srcRowPtr, dstRowPtr, m_srcFrameWidth, srcIncrement, dstIncrement);
        srcRowPtr += srcRowSize;
      }
    }
  }
  else {
    for (unsigned y = 0; y < m_srcFrameHeight; y++) {
      SwapRedAndBlueRow(srcRowPtr, dstRowPtr, m_srcFrameWidth, srcIncrement, dstIncrement);
      srcRowPtr += srcRowSize;
      dstRowPtr += dstRowSize;
    }
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  return true;
}


PSTANDARD_COLOUR_CONVERTER(RGB24,BGR24)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 3);
}


PSTANDARD_COLOUR_CONVERTER(BGR24,RGB24)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 3);
}


PSTANDARD_COLOUR_CONVERTER(RGB24,BGR32)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 4);
}


PSTANDARD_COLOUR_CONVERTER(BGR24,RGB32)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 3, 4);
}


PSTANDARD_COLOUR_CONVERTER(RGB32,BGR24)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 3);
}


PSTANDARD_COLOUR_CONVERTER(BGR32,RGB24)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 3);
}


PSTANDARD_COLOUR_CONVERTER(RGB32,BGR32)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 4);
}


PSTANDARD_COLOUR_CONVERTER(BGR32,RGB32)
{
  return SwapRedAndBlue(srcFrameBuffer, dstFrameBuffer, bytesReturned, 4, 4);
}


PSTANDARD_COLOUR_CONVERTER(RGB24,RGB32)
{
  if ((m_dstFrameWidth != m_srcFrameWidth) || (m_dstFrameHeight != m_srcFrameHeight)) {
    PTRACE(2,"Cannot do RGB 24/32 conversion on different sized image, not implemented.");
    return false;
  }

  // Go from bottom to top so can do in place conversion
  const BYTE * src = srcFrameBuffer+m_srcFrameBytes-1;
  BYTE * dst = dstFrameBuffer+m_dstFrameBytes-1;

  for (unsigned x = 0; x < m_srcFrameWidth; x++) {
    for (unsigned y = 0; y < m_srcFrameHeight; y++) {
      *dst-- = 0;
      for (unsigned p = 0; p < 3; p++)
        *dst-- = *src--;
    }
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  return true;
}


PSTANDARD_COLOUR_CONVERTER(RGB32,RGB24)
{
  if ((m_dstFrameWidth != m_srcFrameWidth) || (m_dstFrameHeight != m_srcFrameHeight)) {
    PTRACE(2,"Cannot do RGB 32/24 conversion on different sized image, not implemented.");
    return false;
  }

  const BYTE * src = srcFrameBuffer;
  BYTE * dst = dstFrameBuffer;

  for (unsigned x = 0; x < m_srcFrameWidth; x++) {
    for (unsigned y = 0; y < m_srcFrameHeight; y++) {
      for (unsigned p = 0; p < 3; p++)
        *dst++ = *src++;
      src++;
    }
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  return true;
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
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if ((m_dstFrameWidth != m_srcFrameWidth) || (m_dstFrameHeight != m_srcFrameHeight)) {
    PTRACE(2,"Cannot do YUV 420/411 conversion on different sized image, not implemented.");
    return false;
  }

  // Copy over the Y plane.
  memcpy(dstFrameBuffer, srcFrameBuffer, m_srcFrameWidth*m_srcFrameHeight);

  unsigned linewidth = m_dstFrameWidth / 4;

  // Source data is the start of the U plane
  const BYTE* src = srcFrameBuffer + m_srcFrameWidth * m_srcFrameHeight;

  // Two output lines at a time
  BYTE *dst0 = dstFrameBuffer + m_dstFrameWidth * m_dstFrameHeight;
  BYTE *dst1 = dst0 + linewidth;

  unsigned x, y;

  // U plane
  for (y = 0; y < m_dstFrameHeight; y += 2) {
    for (x = 0; x < m_dstFrameWidth; x += 4) {
      *dst0++ = *src++;
      *dst1++ = *src++;
    }

    // Skip over the 2nd line we already did.
    dst0 += linewidth;
    dst1 = dst0 + linewidth;
  }

  // Source data is the start of the U plane
  src = srcFrameBuffer + m_srcFrameWidth * m_srcFrameHeight * 5 / 4;

  // Two output lines at a time
  dst0 = dstFrameBuffer + m_dstFrameWidth * m_dstFrameHeight * 5 / 4;
  dst1 = dst0 + linewidth;

  // V plane
  for (y = 0; y < m_dstFrameHeight; y += 2) {
    for (x = 0; x < m_dstFrameWidth; x += 4) {
      *dst0++ = *src++;
      *dst1++ = *src++;
    }

    // Skip over the 2nd line we already did.
    dst0 += linewidth;
    dst1 = dst0 + linewidth;
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  
  return true;
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
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if ((m_dstFrameWidth != m_srcFrameWidth) || (m_dstFrameHeight != m_srcFrameHeight)) {
    PTRACE(2,"Cannot do YUV 411/420 conversion on different sized image, not implemented.");
    return false;
  }

  // Copy over the Y plane.
  memcpy(dstFrameBuffer, srcFrameBuffer, m_srcFrameWidth*m_srcFrameHeight);

  unsigned linewidth = m_dstFrameWidth / 4;

  // Source data is the start of the U plane
  const BYTE* src = srcFrameBuffer + m_srcFrameWidth * m_srcFrameHeight;

  // Output line
  BYTE *dst0 = dstFrameBuffer + m_dstFrameWidth * m_dstFrameHeight;

  unsigned x, y;

  // U plane
  for (y = 0; y < m_dstFrameHeight; y += 2) {
    for (x = 0; x < m_dstFrameWidth; x += 4) {

      // Double up the horizontal samples
      *dst0++ = *src;
      *dst0++ = *src++;
    }

    // Skip over the 2nd line we are decimating
    src += linewidth;
  }

  // Source data is the start of the U plane
  src = srcFrameBuffer + m_srcFrameWidth * m_srcFrameHeight * 5 / 4;

  // Output line
  dst0 = dstFrameBuffer + m_dstFrameWidth * m_dstFrameHeight * 5 / 4;

  // V plane
  for (y = 0; y < m_dstFrameHeight; y += 2) {
    for (x = 0; x < m_dstFrameWidth; x += 4) {

      // Double up the samples horizontal samples
      *dst0++ = *src;
      *dst0++ = *src++;
    }

    // Skip over the 2nd source line we already did.
    src += linewidth;
  }

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}


/*
 * Format UYVY or UYVY422(non planar) 4x4
 *
 * off: 0  U00 Y00 V01 Y00 U02 Y01 V03 Y01
 * off: 8  U10 Y10 V11 Y10 U12 Y11 V13 Y11
 * off:16  U20 Y20 V21 Y20 U22 Y21 V23 Y21
 * off:24  U30 Y30 V31 Y30 U32 Y31 V33 Y31
 * length:32 bytes
 */
PSTANDARD_COLOUR_CONVERTER(UYVY422, UYVY422)
{
  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  
  if (srcFrameBuffer == dstFrameBuffer) {
    if (m_srcFrameWidth == m_dstFrameWidth && m_srcFrameHeight == m_dstFrameHeight) 
      return true;
    if(m_srcFrameWidth < m_dstFrameWidth || m_srcFrameHeight < m_dstFrameHeight) {
      PTRACE(2,"Cannot do in place conversion, increasing image size.");
      return false;
    }
  }

  if ((m_srcFrameWidth == m_dstFrameWidth) && (m_srcFrameHeight == m_dstFrameHeight)) 
    memcpy(dstFrameBuffer,srcFrameBuffer,m_srcFrameWidth*m_srcFrameHeight*2);
  else
    UYVY422WithCrop(srcFrameBuffer, dstFrameBuffer);

  return true;
}

/*
 * Format UYVY (or UYVY422) non planar (4x4)
 *
 * off: 0  U00 Y00 V01 Y00 U02 Y01 V03 Y01
 * off: 8  U10 Y10 V11 Y10 U12 Y11 V13 Y11
 * off:16  U20 Y20 V21 Y20 U22 Y21 V23 Y21
 * off:24  U30 Y30 V31 Y30 U32 Y31 V33 Y31
 * length:32 bytes
 *
 * NOTE: This algorithm works only if the width and the height is pair.
 */
void PStandardColourConverter::UYVY422WithCrop(const BYTE *src_uyvy, BYTE *dst_uyvy) const
{
  const BYTE *s;
  BYTE *d;
  unsigned int x, h;  
  unsigned int npixels = m_dstFrameWidth * m_dstFrameHeight;

  s = src_uyvy;
  d = dst_uyvy;

  if ( (m_srcFrameWidth * m_srcFrameHeight) < npixels ) {

     // dest is bigger than the source. No subsampling.
     // Place the src in the middle of the destination.
     unsigned int yOffset = (m_dstFrameHeight - m_srcFrameHeight)/2;
     unsigned int xOffset = (m_dstFrameWidth - m_srcFrameWidth)/2;

     /* Top border */
     for (h=0; h<yOffset; h++)
      {
        for (x=0; x<m_dstFrameWidth/2; x++)
         {
           *d++ = BLACK_U;
           *d++ = BLACK_Y;
           *d++ = BLACK_V;
           *d++ = BLACK_Y;
         }
      }

     for (h=0; h<m_srcFrameHeight; h+=2)
      {
        /* Left border */
               for (x=0; x<xOffset/2; x++)
         {
           *d++ = BLACK_U;
           *d++ = BLACK_Y;
           *d++ = BLACK_V;
           *d++ = BLACK_Y;
         }

        /* Copy the first line keeping all information */
        memcpy(d, s, m_srcFrameWidth*2);
        d += m_srcFrameWidth*2;
        /* Right and Left border */
               for (x=0; x<xOffset/2; x++)
         {
           *d++ = BLACK_U;
           *d++ = BLACK_Y;
           *d++ = BLACK_V;
           *d++ = BLACK_Y;
         }
      }
     for (h=0; h<yOffset; h++)
      {
        for (x=0; x<m_dstFrameWidth/2; x++)
         {
           *d++ = BLACK_U;
           *d++ = BLACK_Y;
           *d++ = BLACK_V;
           *d++ = BLACK_Y;
         }
      }

  } else {

     /* FIXME */

   }

}


/*
 * Format UYVY or UYVY422(non planar) 4x4
 *
 * off: 0  U00 Y00 V01 Y00 U02 Y01 V03 Y01
 * off: 8  U10 Y10 V11 Y10 U12 Y11 V13 Y11
 * off:16  U20 Y20 V21 Y20 U22 Y21 V23 Y21
 * off:24  U30 Y30 V31 Y30 U32 Y31 V33 Y31
 * length:32 bytes
 *
 * Format YUV420P:
 * off: 00  Y00 Y01 Y02 Y03
 * off: 04  Y10 Y11 Y12 Y13
 * off: 08  Y20 Y21 Y22 Y23
 * off: 12  Y30 Y31 Y32 Y33
 * off: 16  U00 U02 U20 U22
 * off: 20  V00 V02 V20 V22
 * 
 * So, we lose some bit of information when converting UYVY to YUV420 
 *
 * NOTE: This algorithm works only if the width and the height is pair.
 */
void  PStandardColourConverter::UYVY422toYUV420PSameSize(const BYTE *uyvy, BYTE *yuv420p) const
{
  const BYTE *s;
  BYTE *y, *u, *v;
  unsigned int x, h;  
  int npixels = m_srcFrameWidth * m_srcFrameHeight;

  s = uyvy;
  y = yuv420p;
  u = yuv420p + npixels;
  v = u + npixels/4;

  for (h=0; h<m_srcFrameHeight; h+=2) {

     /* Copy the first line keeping all information */
     for (x=0; x<m_srcFrameWidth; x+=2) {
        *u++ = *s++;
        *y++ = *s++;
        *v++ = *s++;
        *y++ = *s++;
     }
     /* Copy the second line discarding u and v information */
     for (x=0; x<m_srcFrameWidth; x+=2) {
        s++;
        *y++ = *s++;
        s++;
        *y++ = *s++;
     }
  }
}


/*
 * Format UYVY (or UYVY422) non planar (4x4)
 *
 * off: 0  U00 Y00 V01 Y00 U02 Y01 V03 Y01
 * off: 8  U10 Y10 V11 Y10 U12 Y11 V13 Y11
 * off:16  U20 Y20 V21 Y20 U22 Y21 V23 Y21
 * off:24  U30 Y30 V31 Y30 U32 Y31 V33 Y31
 * length:32 bytes
 *
 * Format YUV420P:
 * off: 00  Y00 Y01 Y02 Y03
 * off: 04  Y10 Y11 Y12 Y13
 * off: 08  Y20 Y21 Y22 Y23
 * off: 12  Y30 Y31 Y32 Y33
 * off: 16  U00 U02 U20 U22
 * off: 20  V00 V02 V20 V22
 * 
 * So, we lose some bit of information when converting YUY2 to YUV420 
 *
 * NOTE: This algorithm works only if the width and the height is pair.
 */
void PStandardColourConverter::UYVY422toYUV420PWithCrop(const BYTE *uyvy, BYTE *yuv420p) const
{
  const BYTE *s;
  BYTE *y, *u, *v;
  unsigned int x, h;  
  unsigned int npixels = m_dstFrameWidth * m_dstFrameHeight;

  s = uyvy;
  y = yuv420p;
  u = yuv420p + npixels;
  v = u + npixels/4;

  if ( (m_srcFrameWidth * m_srcFrameHeight) < npixels ) {

     // dest is bigger than the source. No subsampling.
     // Place the src in the middle of the destination.
     unsigned int yOffset = (m_dstFrameHeight - m_srcFrameHeight)/2;
     unsigned int xOffset = (m_dstFrameWidth - m_srcFrameWidth)/2;
     unsigned int bpixels = yOffset * m_dstFrameWidth;

     /* Top border */
     memset(y, BLACK_Y, bpixels);        y += bpixels;
     memset(u, BLACK_U, bpixels/4);        u += bpixels/4;
     memset(v, BLACK_V, bpixels/4);        v += bpixels/4;

     for (h=0; h<m_srcFrameHeight; h+=2)
      {
        /* Left border */
        memset(y, BLACK_Y, xOffset);        y += xOffset;
        memset(u, BLACK_U, xOffset/2);        u += xOffset/2;
        memset(v, BLACK_V, xOffset/2);        v += xOffset/2;

        /* Copy the first line keeping all information */
        for (x=0; x<m_srcFrameWidth; x+=2)
         {
           *u++ = *s++;
           *y++ = *s++;
           *v++ = *s++;
           *y++ = *s++;
         }
        /* Right and Left border */
        for (x=0; x<xOffset*2; x++)
          *y++ = BLACK_Y;

        /* Copy the second line discarding u and v information */
        for (x=0; x<m_srcFrameWidth; x+=2)
         {
           s++;
           *y++ = *s++;
           s++;
           *y++ = *s++;
         }
        /* Fill the border with black (right side) */
        memset(y, BLACK_Y, xOffset);        y += xOffset;
        memset(u, BLACK_U, xOffset/2);        u += xOffset/2;
        memset(v, BLACK_V, xOffset/2);        v += xOffset/2;
      }
     memset(y, BLACK_Y, bpixels);
     memset(u, BLACK_U, bpixels/4);
     memset(v, BLACK_V, bpixels/4);


  } else {

     // source is bigger than the destination
     // We are doing linear interpolation to find value.
#define FIX_FLOAT       12
     unsigned int dx = (m_srcFrameWidth<<FIX_FLOAT)/m_dstFrameWidth;
     unsigned int dy = (m_srcFrameHeight<<FIX_FLOAT)/m_dstFrameHeight;
     unsigned int fy, fx;

     for (fy=0, h=0; h<m_dstFrameHeight; h+=2, fy+=dy*2)
      {
        /* Copy the first line with U&V */
        unsigned int yy = fy>>FIX_FLOAT;
        unsigned int yy2 = (fy+dy)>>FIX_FLOAT;
        const unsigned char *line1, *line2;
        unsigned char lastU, lastV;

        line1 = s + (yy*2*m_srcFrameWidth);
        line2 = s + (yy2*2*m_srcFrameWidth);
        lastU = line1[0];
        lastV = line1[2];
        for (fx=0, x=0; x<m_dstFrameWidth; x+=2, fx+=dx*2)
         {
           unsigned int xx = (fx>>FIX_FLOAT)*2;
           if ( (xx&2) == 0)
            {
              *u++ = lastU = (line1[xx+0] + line2[xx+0])/2;
              *v++ = lastV = (line1[xx+2] + line2[xx+2])/2;
            }
           else
            {
              *u++ = lastU;
              *v++ = lastV = (line1[xx+0] + line2[xx+0])/2;
            }
           *y++ = line1[xx+1];
           xx = ((fx+dx)>>FIX_FLOAT)*2;
           if ( (xx&2) == 0)
             lastU = (line1[xx+0] + line2[xx+0])/2;
           else
             lastV = (line1[xx+0] + line2[xx+0])/2;
           *y++ = line1[xx+1];
         }

        /* Copy the second line without U&V */
        for (fx=0, x=0; x<m_dstFrameWidth; x++, fx+=dx)
         {
           unsigned int xx = (fx>>FIX_FLOAT)*2;
           *y++ = line2[xx+1];
         }
      } /* end of for (fy=0, h=0; h<m_dstFrameHeight; h+=2, fy+=dy*2) */

   }

}


/*
 * The following functions converts video from IEEE 1394 cameras into
 * YUV420P format. The video format of IEEE 1394 cameras can be found
 *  at Section 2.1.3 of
http://www.1394ta.org/Download/Technology/Specifications/2000/IIDC_Spec_v1_30.pdf
 * 320x240 and 160x120 resolutions are used.
 *
 */
PSTANDARD_COLOUR_CONVERTER(UYVY422,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  if ((m_srcFrameWidth==m_dstFrameWidth) && (m_srcFrameHeight==m_dstFrameHeight))
    UYVY422toYUV420PSameSize(srcFrameBuffer, dstFrameBuffer);
  else
    UYVY422toYUV420PWithCrop(srcFrameBuffer, dstFrameBuffer);

  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;

  return true;
}

PSTANDARD_COLOUR_CONVERTER(UYV444,YUV420P)
{
  if (srcFrameBuffer == dstFrameBuffer) {
    PTRACE(2,"Cannot do in-place conversion, not implemented.");
    return false;
  }

  unsigned int row,column;
  unsigned char *y = dstFrameBuffer;  //Initialise y,u,v here, to stop compiler warnings.
  unsigned char *u = dstFrameBuffer + m_dstFrameWidth*m_dstFrameHeight;
  unsigned char *v = dstFrameBuffer + m_dstFrameWidth*(m_dstFrameHeight + m_dstFrameHeight/4);
  const unsigned char *src = srcFrameBuffer;

  for(row=0; row < PMIN(m_srcFrameHeight, m_dstFrameHeight); row+=2) {
    y = dstFrameBuffer + m_dstFrameWidth*row;
    u = dstFrameBuffer + m_dstFrameWidth*m_dstFrameHeight + m_dstFrameWidth*row/4;
    v = dstFrameBuffer + m_dstFrameWidth*(m_dstFrameHeight + m_dstFrameHeight/4) + m_dstFrameWidth*row/4;
    src = srcFrameBuffer + row*m_srcFrameWidth*3;
    for(column=0; column < PMIN(m_srcFrameWidth, m_dstFrameWidth); column+=2) {
      *(u++) = (unsigned char)(((unsigned int)src[0] + src[3] + src[m_srcFrameWidth*3] + src[3+m_srcFrameWidth*3])/4);
      *(y++) = src[1];
      *(v++) = (unsigned char)(((unsigned int)src[2] + src[5] + src[m_srcFrameWidth*3] +src[3+m_srcFrameWidth*3])/4);
      *(y++) = src[4];
      src += 6;
    }
    for(column = PMIN(m_srcFrameWidth, m_dstFrameWidth);
  column < m_dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    y = dstFrameBuffer + m_dstFrameWidth*(row+1);
    src = srcFrameBuffer + (row+1)*m_srcFrameWidth*3;
    for(column=0; column < PMIN(m_srcFrameWidth, m_dstFrameWidth); column++) {
      src++;
      *(y++) = *(src++);
      src++;
    }
    for(column = PMIN(m_srcFrameWidth, m_dstFrameWidth);
  column < m_dstFrameWidth; column++)
      *(y++) = BLACK_Y;
  }
  for(row = PMIN(m_srcFrameHeight, m_dstFrameHeight);
      row<m_dstFrameHeight; row+=2) {
    for(column = 0; column < m_dstFrameWidth; column+=2) {
      *(u++) = BLACK_U;
      *(y++) = BLACK_Y;
      *(v++) = BLACK_V;
      *(y++) = BLACK_Y;
    }
    for(column = 0; column < m_dstFrameWidth; column+=2) {
      *(y++) = BLACK_Y;
      *(y++) = BLACK_Y;
    }
  }
  if (bytesReturned != NULL)
    *bytesReturned = m_dstFrameBytes;
  return true;
}
PRAGMA_OPTIMISE_DEFAULT()


///////////////////////////////////////////////////////////////////////////////

#if P_JPEG_DECODER

struct PJPEGConverter::Context
{
#if P_TINY_JPEG

  typedef int ColourSpace;
  #define MY_JPEG_Grey      TINYJPEG_FMT_GREY
  #define MY_JPEG_RGB24     TINYJPEG_FMT_RGB24
  #define MY_JPEG_BGR24     TINYJPEG_FMT_BGR24
  #define MY_JPEG_YUV420P   TINYJPEG_FMT_YUV420P

  jdec_private * m_decoder;

  Context()
  {
    m_decoder = tinyjpeg_init();
    if (m_decoder == NULL) {
      PTRACE(2, NULL, "JPEG", "TinyJpeg error: Can't allocate memory");
      return;
    }

    tinyjpeg_set_flags(m_decoder, TINYJPEG_FLAGS_MJPEG_TABLE);
    PTRACE(4, NULL, "JPEG", "TinyJpeg decoder created");
  }


  ~Context()
  {
    if (m_decoder != NULL)
      free(m_decoder);
    PTRACE(4, NULL, "JPEG", "TinyJpeg decoder destroyed");
  }


  bool Start(const BYTE * srcFrameBuffer, PINDEX srcFrameBytes, unsigned & width, unsigned & height)
  {
    if (tinyjpeg_parse_header(m_decoder, srcFrameBuffer, srcFrameBytes) < 0) {
      PTRACE(2, NULL, "JPEG", "Header parse error: " << tinyjpeg_get_errorstring(m_decoder));
      return false;
    }

    tinyjpeg_get_size(m_decoder, &width, &height);
    return true;
  }


  bool Finish(BYTE * dstFrameBuffer, unsigned width, unsigned height)
  {
    if (PAssertNULL(dstFrameBuffer) == NULL)
      return false;

    int componentCount = 1;
    BYTE *components[4];
    components[0] = dstFrameBuffer;

    if (m_colourSpace == TINYJPEG_FMT_YUV420P) {
      componentCount = 4;
      int npixels = width * height;
      components[1] = dstFrameBuffer + npixels;
      components[2] = dstFrameBuffer + npixels + npixels/4;
      components[3] = NULL;
    }
 
    tinyjpeg_set_components(m_decoder, components, componentCount);

    if (tinyjpeg_decode(m_decoder, m_colourSpace) >= 0)
      return true;

    PTRACE(2, NULL, "JPEG", "Decode error: " << tinyjpeg_get_errorstring(m_decoder));
    return false;
  }


#elif P_LIBJPEG

  typedef J_COLOR_SPACE ColourSpace;
  #define MY_JPEG_Grey    JCS_GRAYSCALE
  #define MY_JPEG_RGB24   JCS_RGB
  #define MY_JPEG_YUV420P JCS_YCbCr
#if JCS_EXTENSIONS
  #define MY_JPEG_BGR24   JCS_EXT_BGR
  #define MY_JPEG_RGB32   JCS_EXT_RGBX
  #define MY_JPEG_BGR32   JCS_EXT_BGRX
#endif

  jpeg_error_mgr         m_error_mgr;
  jpeg_decompress_struct m_decoder;
  PBYTEArray             m_scan_line;


  Context()
  {
    m_decoder.err = jpeg_std_error(&m_error_mgr);
    jpeg_create_decompress(&m_decoder);
    PTRACE(4, NULL, "JPEG", "libjpeg decoder created");
  }


  ~Context()
  {
    jpeg_destroy_decompress(&m_decoder);
    PTRACE(4, NULL, "JPEG", "libjpeg decoder destroyed");
  }


  void Error()
  {
#if PTRACING
    if (PTrace::CanTrace(2)) {
      ostream & trace = PTRACE_BEGIN(2, NULL, "JPEG");
      trace << "Turbo-JPEG failed: ";
      switch (m_error_mgr.last_jpeg_message) {
        case 0 :
          trace << "unknown error";
          break;
        case 1 :
          trace << *m_error_mgr.jpeg_message_table;
          break;
        default :
          for (int msg = 0; msg < m_error_mgr.last_jpeg_message; ++msg)
            trace << "\n  Error " << (msg+1) << ": " << m_error_mgr.jpeg_message_table[msg];
      }
      trace << PTrace::End;
    }
#endif
  }


  bool Start(const BYTE * srcFrameBuffer, PINDEX srcFrameBytes, unsigned & width, unsigned & height)
  {
    jpeg_mem_src(&m_decoder, const_cast<unsigned char *>(srcFrameBuffer), srcFrameBytes);

    if (jpeg_read_header(&m_decoder, TRUE) == JPEG_HEADER_OK) {
      m_decoder.out_color_space = m_colourSpace;
      m_decoder.dct_method = JDCT_IFAST;
      if (jpeg_start_decompress(&m_decoder)) {
        if (width > m_decoder.output_width)
          width = m_decoder.output_width;
        if (height > m_decoder.output_height)
          height = m_decoder.output_height;
        return true;
      }
    }

    Error();
    return false;
  }


  bool Finish(BYTE * dstFrameBuffer, unsigned width, unsigned height)
  {
    JSAMPROW row, y, u, v;
    row = y = u = v = dstFrameBuffer;

    if (m_colourSpace == JCS_YCbCr) {
      row = m_scan_line.GetPointer(m_decoder.output_width*3); // Do not sue width here, it could be smaller.
      u += width*height;
      v += width*height*5/4;
    }

    while (jpeg_read_scanlines(&m_decoder, &row, 1) == 1) {
      switch (m_colourSpace) {
        default :
          PAssertAlways(PUnsupportedFeature);
          return false;

#if JCS_EXTENSIONS
        case JCS_EXT_RGBX :
        case JCS_EXT_BGRX :
          row += width*4;
          break;
          
        case JCS_EXT_RGB :
        case JCS_EXT_BGR :
#endif
        case JCS_RGB :
          row += width*3;
          break;

        case JCS_GRAYSCALE :
          row += width;
          break;

        case JCS_YCbCr :
          if (m_decoder.output_scanline < height) {
            for (JDIMENSION x = 0; x < width; ++x) {
              *y++ = row[0];
              if (((m_decoder.output_scanline|x) & 1) == 0) {
                *u++ = row[1];
                *v++ = row[2];
              }
              row += 3;
            }
            row = m_scan_line.GetPointer();
          }
      }

      if (m_decoder.output_scanline >= m_decoder.output_height) {
        jpeg_finish_decompress(&m_decoder);
        return true;
      }
    }

    Error();
    return false;
  }

#endif // P_TINY_JPEG/P_LIBJPEG


  ColourSpace m_colourSpace;

  bool SetColourSpace(const PCaselessString & colourFormat)
  {
    if (colourFormat == PVideoFrameInfo::YUV420P())
      m_colourSpace = MY_JPEG_YUV420P;
    else if (colourFormat == "RGB24")
      m_colourSpace = MY_JPEG_RGB24;
#ifdef MY_JPEG_BGR24
    else if (colourFormat == "BGR24")
      m_colourSpace = MY_JPEG_BGR24;
#endif
#ifdef MY_JPEG_RGB32
    else if (colourFormat == "RGB32")
      m_colourSpace = MY_JPEG_RGB32;
#endif
#ifdef MY_JPEG_BGR32
    else if (colourFormat == "BGR32")
      m_colourSpace = MY_JPEG_BGR32;
#endif
    else if (colourFormat == "Grey")
      m_colourSpace = MY_JPEG_Grey;
    else {
      PTRACE(2, NULL, "JPEG", "Unsupported colour format: " << colourFormat);
      return false;
    }

    return true;
  }


  PINDEX GetBufferSizeForColour(unsigned width, unsigned height)
  {
    switch (m_colourSpace) {
      case MY_JPEG_RGB24 :
#ifdef MY_JPEG_BGR24
      case MY_JPEG_BGR24 :
#endif
        return width*height*3;

#ifdef MY_JPEG_RGB32
      case MY_JPEG_RGB32 :
#endif
#ifdef MY_JPEG_BGR32
      case MY_JPEG_BGR32 :
#endif
#if defined(MY_JPEG_RGB32) || defined(MY_JPEG_BGR32)
        return width*height*4;
#endif

      case MY_JPEG_Grey :
        return width*height;

      case MY_JPEG_YUV420P :
        return PVideoFrameInfo::CalculateFrameBytes(width, height);

      default :
        PAssertAlways(PInvalidParameter);
        return 0;
    }
  }


  PBYTEArray m_temporaryBuffer;

  bool Convert(const BYTE * srcFrameBuffer, PINDEX srcFrameBytes,
               BYTE * dstFrameBuffer, PINDEX dstFrameBytes,
               unsigned & outputWidth, unsigned & outputHeight,
               PINDEX * bytesReturned, ColourSpace colourSpace,
               PVideoFrameInfo::ResizeMode resizeMode)
  {
    m_colourSpace = colourSpace;

    unsigned nativeWidth = 0, nativeHeight = 0;
    if (!Start(srcFrameBuffer, srcFrameBytes, nativeWidth, nativeHeight))
        return false;

    if (outputWidth == 0 && outputHeight == 0) {
      outputWidth = nativeWidth;
      outputHeight = nativeHeight;
    }

    if (!PAssert(outputWidth != 0 && outputHeight != 0, PInvalidParameter))
      return false;

    PINDEX actualFrameBytes = GetBufferSizeForColour(outputWidth, outputHeight);
    if (dstFrameBytes < actualFrameBytes) {
      PTRACE(2, NULL, "JPEG", "Output buffer size too small: " << dstFrameBytes << ", required " << actualFrameBytes);
      return false;
    }

    if (bytesReturned != NULL)
      *bytesReturned = actualFrameBytes;

    if (nativeWidth == outputWidth && nativeHeight == outputHeight)
        return Finish(dstFrameBuffer, nativeWidth, nativeHeight);

    if (colourSpace != MY_JPEG_YUV420P) {
      PTRACE(2, NULL, "JPEG", "Cannot resize output unless YUV420P");
      return false;
    }

    if (!Finish(m_temporaryBuffer.GetPointer(PVideoFrameInfo::CalculateFrameBytes(nativeWidth, nativeHeight)), nativeWidth, nativeHeight))
      return false;

    PStringStream error;
    if (CopyYUV420P(0, 0, nativeWidth, nativeHeight, nativeWidth, nativeHeight, m_temporaryBuffer,
                    0, 0, outputWidth, outputHeight, outputWidth, outputHeight, dstFrameBuffer, resizeMode, false, &error))
      return true;

    PTRACE(3, NULL, "JPEG", "Cannot resize output: " << error);
    return false;
  }


  bool Load(PFile & file, PBYTEArray & frameBuffer, unsigned & width, unsigned & height, PVideoFrameInfo::ResizeMode resizeMode)
  {
    PBYTEArray jpegData;
    if (!PAssert(jpegData.SetSize(file.GetLength()),POutOfMemory))
      return false;

    if (!file.Read(jpegData.GetPointer(), jpegData.GetSize())) {
      PTRACE(2, NULL, "JPEG", "File read error: " << file.GetErrorText());
      return false;
    }

    unsigned nativeWidth = 0, nativeHeight = 0;
    if (!Start(jpegData, jpegData.GetSize(), nativeWidth, nativeHeight))
        return false;

    if (m_colourSpace != MY_JPEG_YUV420P || width == 0 || width == 0 || (width == nativeWidth && height == nativeHeight)) {
      width = nativeWidth;
      height = nativeHeight;
      return Finish(frameBuffer.GetPointer(GetBufferSizeForColour(width, height)), width, height);
    }

    PBYTEArray temporaryBuffer;
    if (!Finish(m_temporaryBuffer.GetPointer(GetBufferSizeForColour(nativeWidth, nativeHeight)), nativeWidth, nativeHeight))
      return false;

    return CopyYUV420P(0, 0, nativeWidth, nativeHeight, nativeWidth, nativeHeight, m_temporaryBuffer,
                       0, 0, width, height, width, height, frameBuffer.GetPointer(GetBufferSizeForColour(width, height)),
                       resizeMode);
  }
};


PJPEGConverter::PJPEGConverter()
  : PColourConverter(PColourPair("JPEG", PVideoFrameInfo::YUV420P()))
  , m_context(new Context)
{
  SetResizeMode(PVideoFrameInfo::eScaleKeepAspect);
}


PJPEGConverter::PJPEGConverter(unsigned width, unsigned height, PVideoFrameInfo::ResizeMode resizeMode, const PString & colourFormat)
  : PColourConverter(PColourPair("JPEG", colourFormat))
  , m_context(new Context)
{
  SetResizeMode(resizeMode);
  SetDstFrameSize(width, height);
}


PJPEGConverter::PJPEGConverter(const PColourPair & colours)
  : PColourConverter(colours)
  , m_context(new Context)
{
  SetResizeMode(PVideoFrameInfo::eScaleKeepAspect);
}


PJPEGConverter::PJPEGConverter(const PVideoFrameInfo & src, const PVideoFrameInfo & dst)
  : PColourConverter(PColourPair(src.GetColourFormat(), dst.GetColourFormat()))
  , m_context(new Context)
{
  SetSrcFrameInfo(src);
  SetDstFrameInfo(dst);
}


PJPEGConverter::~PJPEGConverter()
{
  delete m_context;
}


PBoolean PJPEGConverter::Convert(const BYTE * srcFrameBuffer, BYTE * dstFrameBuffer, PINDEX * bytesReturned)
{
  if (!m_context->SetColourSpace(m_dstColourFormat))
    return false;

  return m_context->Convert(srcFrameBuffer, m_srcFrameBytes,
                            dstFrameBuffer, m_dstFrameBytes,
                            m_dstFrameWidth, m_dstFrameHeight,
                            bytesReturned, m_context->m_colourSpace,
                            m_resizeMode);
}


bool PJPEGConverter::Load(const PFilePath & filename, PBYTEArray & dstFrameBuffer)
{
  PFile file;
  return file.Open(filename, PFile::ReadOnly) && Load(file, dstFrameBuffer);
}


bool PJPEGConverter::Load(PFile & file, PBYTEArray & dstFrameBuffer)
{
  if (!m_context->SetColourSpace(m_dstColourFormat))
    return false;

  return m_context->Load(file, dstFrameBuffer, m_dstFrameWidth, m_dstFrameHeight, m_resizeMode);
}


#define JPEG_CONVERTER(from,to) \
  PCOLOUR_CONVERTER2(PColourConverter_##from##_##to,PJPEGConverter,#from,#to) \
  { return m_context->Convert(srcFrameBuffer, m_srcFrameBytes, dstFrameBuffer, m_dstFrameBytes, \
                              m_dstFrameWidth, m_dstFrameHeight, bytesReturned, MY_JPEG_##to, m_resizeMode); }

JPEG_CONVERTER(MJPEG, RGB24)
#ifdef MY_JPEG_BGR24
JPEG_CONVERTER(MJPEG, BGR24)
JPEG_CONVERTER(JPEG,  BGR24)
#endif
JPEG_CONVERTER(MJPEG, Grey)
JPEG_CONVERTER(MJPEG, YUV420P)
JPEG_CONVERTER(JPEG,  RGB24)
JPEG_CONVERTER(JPEG,  Grey)
JPEG_CONVERTER(JPEG,  YUV420P)
#ifdef MY_JPEG_RGB32
JPEG_CONVERTER(MJPEG, RGB32)
JPEG_CONVERTER(JPEG,  RGB32)
#endif
#ifdef MY_JPEG_BGR32
JPEG_CONVERTER(MJPEG, BGR32)
JPEG_CONVERTER(JPEG,  BGR32)
#endif

#endif // P_JPEG_DECODER

#endif // P_VIDEO

// End Of File ///////////////////////////////////////////////////////////////
