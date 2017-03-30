/*
 * pvidfile.cxx
 *
 * Video file implementation
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is
 * Craig Southeren <craigs@postincrement.com>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifdef __GNUC__
#pragma implementation "pvidfile.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#include <ptclib/pvidfile.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>


#define PTraceModule() "VidFile"


///////////////////////////////////////////////////////////////////////////////

PVideoFile::PVideoFile()
  : m_fixedFrameSize(false)
  , m_fixedFrameRate(false)
  , m_headerOffset(0)
  , m_frameHeaderLen(0)
{
  m_frameBytes = m_videoInfo.CalculateFrameBytes();
}


bool PVideoFile::SetFrameSizeFromFilename(const PString & fn)
{
  static PRegularExpression res("_(sqcif|qcif|cif|cif4|cif16|HD[0-9]+|[0-9]+p|[0-9]+x[0-9]+)[^a-z0-9]",
                                PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  PINDEX pos, len;
  if (!fn.FindRegEx(res, pos, len) || !m_videoInfo.Parse(fn.Mid(pos + 1, len - 2)))
    return false;

  m_frameBytes = m_videoInfo.CalculateFrameBytes();
  return true;
}


bool PVideoFile::SetFPSFromFilename(const PString & fn)
{
  static PRegularExpression fps("_[0-9]+fps[^a-z]",
                                PRegularExpression::Extended|PRegularExpression::IgnoreCase);

  PINDEX pos;
  if ((pos = fn.FindRegEx(fps)) == P_MAX_INDEX)
    return false;

  m_fixedFrameRate = m_videoInfo.SetFrameRate(fn.Mid(pos + 1).AsUnsigned());
  return m_fixedFrameRate;
}


PBoolean PVideoFile::WriteFrame(const void * frame)
{
  return Write(frame, m_frameBytes);
}


PBoolean PVideoFile::ReadFrame(void * frame)
{
  if (Read(frame, m_frameBytes) && (GetLastReadCount() == m_frameBytes))
    return true;

#if PTRACING
  if (GetErrorCode(PFile::LastReadError) != PFile::NoError)
    PTRACE(2, "Error reading file \"" << GetFilePath() << "\" - " << GetErrorText(PFile::LastReadError));
  else
    PTRACE(4, "End of file \"" << GetFilePath() << '"');
#endif

  return false;
}


off_t PVideoFile::GetLength() const
{
  off_t len = PFile::GetLength();
  return len < m_headerOffset ? 0 : ((len - m_headerOffset)/(m_frameBytes+m_frameHeaderLen));
}


PBoolean PVideoFile::SetLength(off_t len)
{
  return PFile::SetLength(len*(m_frameBytes + m_frameHeaderLen) + m_headerOffset);
}


off_t PVideoFile::GetPosition() const
{
  off_t pos = PFile::GetPosition();
  return pos < m_headerOffset ? 0 : ((pos - m_headerOffset)/(m_frameBytes+m_frameHeaderLen));
}


PBoolean PVideoFile::SetPosition(off_t pos, PFile::FilePositionOrigin origin)
{
  pos *= m_frameBytes+m_frameHeaderLen;
  if (origin == PFile::Start)
    pos += m_headerOffset;

  return PFile::SetPosition(pos, origin);
}


PBoolean PVideoFile::SetFrameSize(unsigned width, unsigned height)
{
  if (m_videoInfo.GetFrameWidth() == width && m_videoInfo.GetFrameHeight() == height)
    return true;

  if (m_fixedFrameSize)
    return false;

  if (!m_videoInfo.SetFrameSize(width, height))
    return false;

  m_frameBytes = m_videoInfo.CalculateFrameBytes();
  return m_frameBytes > 0;
}


PBoolean PVideoFile::SetFrameRate(unsigned rate)
{
  if (m_videoInfo.GetFrameRate() == rate)
    return true;

  if (m_fixedFrameRate)
    return false;
  
  return m_videoInfo.SetFrameRate(rate);
}


///////////////////////////////////////////////////////////////////////////////

PFACTORY_CREATE(PVideoFileFactory, PYUVFile, ".yuv");
PFACTORY_SYNONYM(PVideoFileFactory, PYUVFile, y4m, ".y4m");

PYUVFile::PYUVFile()
  : m_y4mMode(false)
{
}


static PString ReadPrintable(PFile & file)
{
  PString str;

  int ch;
  while ((ch = file.ReadChar()) >= ' ' && ch < 0x7f)
    str += (char)ch;

  return str;
}


bool PYUVFile::InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions)
{
  SetFrameSizeFromFilename(GetFilePath());
  SetFPSFromFilename(GetFilePath());

  if (!PVideoFile::InternalOpen(mode, opts, permissions))
    return false;

  m_y4mMode = GetFilePath().GetType() *= ".y4m";
  m_fixedFrameSize = !m_y4mMode;

  if (m_y4mMode) {
    PString info = ReadPrintable(*this);

    PStringArray params = info.Tokenise(" \t", false); // Really is juts a space, but be forgiving
    if (params.IsEmpty() || params[0] != "YUV4MPEG2") {
      PTRACE(2, "Invalid file format, does not start with YUV4MPEG2");
      return false;
    }

    unsigned frameWidth = 0, frameHeight = 0, frameRate = 0, sarWidth = 0, sarHeight = 0;
    for (PINDEX i = 1; i < params.GetSize(); ++i) {
      PString param = params[i].ToUpper();
      switch (param[0]) {
        case 'W' :
          frameWidth = param.Mid(1).AsUnsigned();
          m_fixedFrameSize = true;
          break;

        case 'H' :
          frameHeight = param.Mid(1).AsUnsigned();
          m_fixedFrameSize = true;
          break;

        case 'F' :
          {
            unsigned denom = param.Mid(param.Find(':')+1).AsUnsigned();
            frameRate = (param.Mid(1).AsUnsigned()+denom/2)/denom;
            m_fixedFrameRate = true;
          }
          break;

        case 'I' :
          if (param[1] != 'P') {
            PTRACE(2, "Interlace modes are not supported");
            return false;
          }
          break;

        case 'A' :
          sarWidth = param.Mid(1).AsUnsigned();
          sarHeight = param.Mid(param.Find(':')+1).AsUnsigned();
          break;

        case 'C' :
          if (param == "C420")
            m_videoInfo.SetColourFormat("YUV420P");
          else if (param == "C422")
            m_videoInfo.SetColourFormat("YUV422P");
          else {
            PTRACE(2, "Interlace modes are not supported");
            return false;
          }
          break;
      }
    }

    PTRACE(4, "y4m \"" << info << '"');
    m_headerOffset = PFile::GetPosition();
    m_videoInfo.SetFrameSize(frameWidth, frameHeight);
    m_videoInfo.SetFrameRate(frameRate);
    m_videoInfo.SetFrameSar(sarWidth, sarHeight);
    m_frameBytes = m_videoInfo.CalculateFrameBytes();
  }

  return true;
}


PBoolean PYUVFile::WriteFrame(const void * frame)
{
  if (m_y4mMode) {
    if (PFile::GetPosition() > 0)
      WriteString("FRAME\n");
    else {
      *this << "YUV4MPEG2 W" << m_videoInfo.GetFrameWidth() << " H" << m_videoInfo.GetFrameHeight() << " F" << m_videoInfo.GetFrameRate() << ":1 Ip";
      if (m_videoInfo.GetSarWidth() > 0 && m_videoInfo.GetSarHeight() > 0)
        *this << " A" << m_videoInfo.GetSarWidth() << ':' << m_videoInfo.GetSarHeight();
      if (m_videoInfo.GetColourFormat() == "YUV422P")
        *this << " C422";
      *this << endl;
      m_headerOffset = PFile::GetPosition();
    }
  }

  return Write(frame, m_frameBytes);
}


PBoolean PYUVFile::ReadFrame(void * frame)
{
  if (m_y4mMode) {
    PString info = ReadPrintable(*this);
    if (m_frameHeaderLen == 0)
      m_frameHeaderLen = PFile::GetPosition() - m_headerOffset;
    if (info.NumCompare("FRAME") != EqualTo) {
      PTRACE(2, "Invalid frame header in y4m file");
      return false;
    }
    PTRACE(6, "y4m \"" << info << '"');
  }

  return PVideoFile::ReadFrame(frame);
}


///////////////////////////////////////////////////////////////////////////////

PFACTORY_CREATE(PVideoFileFactory, PBMPFile, ".bmp");
PFACTORY_SYNONYM(PVideoFileFactory, PBMPFile, dib, ".dib");

PBMPFile::PBMPFile()
{
}


bool PBMPFile::InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions)
{
  if (mode != PFile::ReadOnly)
    return false;

  if (!PVideoFile::InternalOpen(mode, opts, permissions))
    return false;

#pragma pack(1)
  struct {
    PUInt16l m_FileType;     /* File type, always 4D42h ("BM") */
    PUInt32l m_FileSize;     /* Size of the file in bytes */
    PUInt16l m_Reserved1;    /* Always 0 */
    PUInt16l m_Reserved2;    /* Always 0 */
    PUInt32l m_BitmapOffset; /* Starting position of image data in bytes */
  } fileHeader;
#pragma pack()

  if (!Read(&fileHeader, sizeof(fileHeader)))
    return false;

  if (fileHeader.m_FileType != 0x4D42)
    return false;

#pragma pack(1)
  struct {
    PUInt32l m_Size;            /* Size of this header in bytes */
    PInt32l  m_Width;           /* Image width in pixels */
    PInt32l  m_Height;          /* Image height in pixels */
    PUInt16l m_Planes;          /* Number of color planes */
    PUInt16l m_BitsPerPixel;    /* Number of bits per pixel */
    PUInt32l m_Compression;     /* Compression methods used */
    PUInt32l m_SizeOfBitmap;    /* Size of bitmap in bytes */
    PInt32l  m_HorzResolution;  /* Horizontal resolution in pixels per meter */
    PInt32l  m_VertResolution;  /* Vertical resolution in pixels per meter */
    PUInt32l m_ColorsUsed;      /* Number of colors in the image */
    PUInt32l m_ColorsImportant; /* Minimum number of important colors */
  } bitmapHeader;
#pragma pack()

  if (!Read(&bitmapHeader.m_Size, sizeof(bitmapHeader.m_Size)))
    return false;
  if (!PFile::SetPosition(sizeof(fileHeader)))
    return false;
  if (!Read(&bitmapHeader, std::min((uint32_t)bitmapHeader.m_Size, (uint32_t)sizeof(bitmapHeader))))
    return false;
  if (bitmapHeader.m_Planes != 1)
    return false;
  if (bitmapHeader.m_BitsPerPixel != 24 && bitmapHeader.m_BitsPerPixel != 32)
    return false;
  if (bitmapHeader.m_Compression != 0)
    return false;

  m_headerOffset =bitmapHeader.m_Size + sizeof(fileHeader);
  if (!SetPosition(0))
    return false;

  m_videoInfo.SetFrameSize(bitmapHeader.m_Width, std::abs(bitmapHeader.m_Height));
  m_frameBytes = m_videoInfo.CalculateFrameBytes();
  if (!m_imageData.SetSize(m_frameBytes))
    return false;

  PVideoFrameInfo rgb = m_videoInfo;
  rgb.SetColourFormat(bitmapHeader.m_BitsPerPixel == 24 ? "BGR24" : "BGR32");

  PBYTEArray temp;
  if (!temp.SetSize(rgb.CalculateFrameBytes()))
    return false;

  if (!Read(temp.GetPointer(), temp.GetSize()))
    return false;

  PColourConverter * converter = PColourConverter::Create(rgb, m_videoInfo);
  if (converter == NULL)
    return false;

  converter->SetVFlipState(bitmapHeader.m_Height > 0);
  bool converted = converter->Convert(temp, m_imageData.GetPointer());
  delete converter;
  return converted;
}


PBoolean PBMPFile::WriteFrame(const void *)
{
  return false;
}


PBoolean PBMPFile::ReadFrame(void * frame)
{
  if (m_imageData.IsEmpty() || !IsOpen())
    return false;

  memcpy(frame, m_imageData, m_frameBytes);
  return true;
}


///////////////////////////////////////////////////////////////////////////////

#if P_JPEG_DECODER

#include <ptlib/vconvert.h>

PFACTORY_CREATE(PVideoFileFactory, PJPEGFile, ".jpg");
PFACTORY_SYNONYM(PVideoFileFactory, PJPEGFile, jpeg, ".jpeg");

PJPEGFile::PJPEGFile()
{
}


PJPEGFile::~PJPEGFile()
{
  Close();
}

PBoolean PJPEGFile::IsOpen() const 
{ 
  return !m_pixelData.IsEmpty();
}


PBoolean PJPEGFile::Close() 
{ 
  m_pixelData.SetSize(0);
  return true;
}


bool PJPEGFile::InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions)
{
  if (mode != PFile::ReadOnly)
    return false;

  if (!PVideoFile::InternalOpen(mode, opts, permissions))
    return false;

  PJPEGConverter decoder(PVideoFrameInfo(m_videoInfo.GetFrameWidth(), m_videoInfo.GetFrameHeight(), "JPEG"), m_videoInfo);
  if (!decoder.Load(*this, m_pixelData))
    return false;

  decoder.GetDstFrameInfo(m_videoInfo);
  return true;
}


off_t PJPEGFile::GetLength() const
{
  return 1;
}


off_t PJPEGFile::GetPosition() const
{
  return 0;
}


bool PJPEGFile::SetPosition(off_t pos, PFile::FilePositionOrigin)
{
  return pos == 0;
}


bool PJPEGFile::WriteFrame(const void * )
{
  return false;
}


PBoolean PJPEGFile::ReadFrame(void * frame)
{
  memcpy(frame, m_pixelData, m_pixelData.GetSize());
  return true;
}
#endif  // P_JPEG_DECODER
#endif  // P_VIDFILE
#endif  // P_VIDEO
