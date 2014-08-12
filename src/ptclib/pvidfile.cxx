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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "pvidfile.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#include <ptclib/pvidfile.h>
#include <ptlib/videoio.h>

#define PTraceModule() "VidFile"


///////////////////////////////////////////////////////////////////////////////

PVideoFile::PVideoFile()
  : m_fixedFrameSize(false)
  , m_fixedFrameRate(false)
  , m_frameBytes(CalculateFrameBytes())
  , m_headerOffset(0)
  , m_frameHeaderLen(0)
{
}


PBoolean PVideoFile::Open(const PFilePath & name, PFile::OpenMode mode, PFile::OpenOptions opts)
{
  return m_file.Open(name, mode, opts);
}


bool PVideoFile::SetFrameSizeFromFilename(const PString & fn)
{
  static PRegularExpression res("_(sqcif|qcif|cif|cif4|cif16|HD[0-9]+|[0-9]+p|[0-9]+x[0-9]+)[^a-z0-9]",
                                PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  PINDEX pos, len;
  if (!fn.FindRegEx(res, pos, len) || !Parse(fn.Mid(pos+1, len-2)))
    return false;

  m_frameBytes = CalculateFrameBytes();
  return true;
}


bool PVideoFile::SetFPSFromFilename(const PString & fn)
{
  static PRegularExpression fps("_[0-9]+fps[^a-z]",
                                PRegularExpression::Extended|PRegularExpression::IgnoreCase);

  PINDEX pos;
  if ((pos = fn.FindRegEx(fps)) == P_MAX_INDEX)
    return false;

  m_fixedFrameRate = PVideoFrameInfo::SetFrameRate(fn.Mid(pos+1).AsUnsigned());
  return m_fixedFrameRate;
}


PBoolean PVideoFile::WriteFrame(const void * frame)
{
  return m_file.Write(frame, m_frameBytes);
}


PBoolean PVideoFile::ReadFrame(void * frame)
{
  if (m_file.Read(frame, m_frameBytes) && (m_file.GetLastReadCount() == m_frameBytes))
    return true;

#if PTRACING
  if (m_file.GetErrorCode(PFile::LastReadError) != PFile::NoError)
    PTRACE(2, "Error reading file \"" << m_file.GetFilePath()
           << "\" - " << m_file.GetErrorText(PFile::LastReadError));
  else
    PTRACE(4, "End of file \"" << m_file.GetFilePath() << '"');
#endif

  return false;
}


off_t PVideoFile::GetLength() const
{
  off_t len = m_file.GetLength();
  return len < m_headerOffset ? 0 : ((len - m_headerOffset)/(m_frameBytes+m_frameHeaderLen));
}


PBoolean PVideoFile::SetLength(off_t len)
{
  return m_file.SetLength(len*(m_frameBytes+m_frameHeaderLen) + m_headerOffset);
}


off_t PVideoFile::GetPosition() const
{
  off_t pos = m_file.GetPosition();
  return pos < m_headerOffset ? 0 : ((pos - m_headerOffset)/(m_frameBytes+m_frameHeaderLen));
}


PBoolean PVideoFile::SetPosition(off_t pos, PFile::FilePositionOrigin origin)
{
  pos *= m_frameBytes+m_frameHeaderLen;
  if (origin == PFile::Start)
    pos += m_headerOffset;

  return m_file.SetPosition(pos, origin);
}


PBoolean PVideoFile::SetFrameSize(unsigned width, unsigned height)
{
  if (frameWidth == width && frameHeight == height)
    return true;

  if (m_fixedFrameSize)
    return false;

  if (!PVideoFrameInfo::SetFrameSize(width, height))
    return false;

  m_frameBytes = CalculateFrameBytes();
  return m_frameBytes > 0;
}


PBoolean PVideoFile::SetFrameRate(unsigned rate)
{
  if (frameRate == rate)
    return true;

  if (m_fixedFrameRate)
    return false;
  
  return PVideoFrameInfo::SetFrameRate(rate);
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


PBoolean PYUVFile::Open(const PFilePath & name, PFile::OpenMode mode, PFile::OpenOptions opts)
{
  SetFrameSizeFromFilename(name);
  SetFPSFromFilename(name);

  if (!PVideoFile::Open(name, mode, opts))
    return false;

  m_y4mMode        = name.GetType() *= ".y4m";
  m_fixedFrameSize = !m_y4mMode;

  if (m_y4mMode) {
    PString info = ReadPrintable(m_file);

    PStringArray params = info.Tokenise(" \t", false); // Really is juts a space, but be forgiving
    if (params.IsEmpty() || params[0] != "YUV4MPEG2") {
      PTRACE(2, "Invalid file format, does not start with YUV4MPEG2");
      return false;
    }

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
            colourFormat = "YUV420P";
          else if (param == "C422")
            colourFormat = "YUV422P";
          else {
            PTRACE(2, "Interlace modes are not supported");
            return false;
          }
          break;
      }
    }

    PTRACE(4, "y4m \"" << info << '"');
    m_headerOffset = m_file.GetPosition();
    m_frameBytes = CalculateFrameBytes();
  }

  return true;
}


PBoolean PYUVFile::WriteFrame(const void * frame)
{
  if (m_y4mMode) {
    if (m_file.GetPosition() > 0)
      m_file.WriteString("FRAME\n");
    else {
      m_file << "YUV4MPEG2 W" << frameWidth << " H" << frameHeight << " F" << frameRate << ":1 Ip";
      if (sarWidth > 0 && sarHeight > 0)
        m_file << " A" << sarWidth << ':' << sarHeight;
      if (colourFormat == "YUV422P")
        m_file << " C422";
      m_file << endl;
      m_headerOffset = m_file.GetPosition();
    }
  }

  return m_file.Write(frame, m_frameBytes);
}


PBoolean PYUVFile::ReadFrame(void * frame)
{
  if (m_y4mMode) {
    PString info = ReadPrintable(m_file);
    if (m_frameHeaderLen == 0)
      m_frameHeaderLen = m_file.GetPosition() - m_headerOffset;
    if (info.NumCompare("FRAME") != EqualTo) {
      PTRACE(2, "Invalid frame header in y4m file");
      return false;
    }
    PTRACE(6, "y4m \"" << info << '"');
  }

  return PVideoFile::ReadFrame(frame);
}


///////////////////////////////////////////////////////////////////////////////

#if P_JPEG_DECODER

#include <ptlib/vconvert.h>

PFACTORY_CREATE(PVideoFileFactory, PJPEGFile, ".jpg");
PFACTORY_SYNONYM(PVideoFileFactory, PJPEGFile, jpeg, ".jpeg");

PJPEGFile::PJPEGFile()
{
  frameWidth = frameHeight = INT_MAX;
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


PBoolean PJPEGFile::Open(const PFilePath & name, PFile::OpenMode mode, PFile::OpenOptions opts)
{
  if (mode != PFile::ReadOnly)
    return false;

  if (!PVideoFile::Open(name, mode, opts))
    return false;

  PJPEGConverter decoder(PVideoFrameInfo(frameWidth, frameHeight, "JPEG"), *this);
  if (!decoder.Load(m_file, m_pixelData))
    return false;

  decoder.GetDstFrameInfo(*this);
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
