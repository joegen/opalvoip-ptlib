/*
 * pffvdev.cxx
 *
 * Video device for ffmpeg
 *
 * Portable Windows Library
 *
 * Copyright (C) 2008 Post Increment
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
#pragma implementation "pffvdev.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_FFVDEV

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/vconvert.h>
#include <ptclib/pffvdev.h>
#include <ptlib/pfactory.h>
#include <ptlib/pluginmgr.h>
#include <ptlib/videoio.h>


#define new PNEW

static const char DefaultAVIFileName[] = "*.avi";

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_FFMPEG

static const char * ffmpegExtensions[] = {
  "avi", "mpg", "wmv", "mov",
  NULL
};

#if _WIN32
static const char * ffmpegExe = "ffmpeg.exe";
#else
static const char * ffmpegExe = "ffmpeg";
#endif


PCREATE_VIDINPUT_PLUGIN_EX(FFMPEG,

  virtual const char * GetFriendlyName() const
  {
    return "File Video source using FFMPEG";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    PCaselessString adjustedDevice = deviceName;

    PINDEX r = 0;
    while (ffmpegExtensions[r] != NULL) {
      PString ext = ffmpegExtensions[r];
      PINDEX extLen = ext.GetLength();
      PINDEX length = adjustedDevice.GetLength();
      if (length > (2+extLen) && adjustedDevice.NumCompare(PString(".") + ext + "*", 2+extLen, length-(2+extLen)) == PObject::EqualTo)
        adjustedDevice.Delete(length-1, 1);
      else if (length < (2+extLen) || adjustedDevice.NumCompare(PString(".") + ext, 1+extLen, length-(1+extLen)) != PObject::EqualTo) {
        ++r;
        continue;
      }
      if (PFile::Access(adjustedDevice, PFile::ReadOnly)) 
        return true;
      PTRACE(1, "FFVDev\tUnable to access file '" << adjustedDevice << "' for use as a video input device");
      return false;
    }
    return false;
  }
);



PVideoInputDevice_FFMPEG::PVideoInputDevice_FFMPEG()
{
  SetColourFormat(PVideoFrameInfo::YUV420P());
  m_channelNumber = 0; 
  grabCount = 0;
  SetFrameRate(10);
}


PVideoInputDevice_FFMPEG::~PVideoInputDevice_FFMPEG()
{
  Close();
}


PBoolean PVideoInputDevice_FFMPEG::Open(const PString & _deviceName, PBoolean /*startImmediate*/)
{
  Close();

  m_ffmpegFrameWidth = m_ffmpegFrameHeight = 0;
  m_ffmpegFrameRate = 25;

  PString cmd = PString(ffmpegExe) & "-i" & _deviceName & "-f rawvideo -";

  // file information comes in on stderr
  if (!m_command.Open(cmd, PPipeChannel::ReadOnly, true, true)) {
    PTRACE(1, "FFVDev\tCannot open command " << cmd);
    return false;
  }

  //if (!m_command.Execute()) {
  //  PTRACE(1, "VidFFMPEG\tCannot execute command " << cmd);
  //  return false;
  //}

  // parse out file size information
  {
    int state = 0;
    PString text;
    PString line;
    PINDEX offs = 0, len = 0;
    while (m_command.IsOpen() && state != -1) {
      if (offs == len) {
        if (!m_command.ReadStandardError(text, true)) {
          PTRACE(1, "FFVDev\tFailure while reading file information for " << cmd);
          return false;
        }
        offs = 0;
        len = text.GetLength();
      }
      else {
        char ch = text[offs++];
        if (ch == '\n') {
          line = line.Trim();
          // Stream #0.0: Video: mpeg4, yuv420p, 640x352 [PAR 1:1 DAR 20:11], 25.00 tb(r)
          if (line.Left(8) *= "Stream #") {
            PStringArray tokens = line.Tokenise(' ', false);
            if (tokens.GetSize() >= 6 && (tokens[2] *= "Video:")) {
              PString size = tokens[5];
              PINDEX x = size.Find('x');
              if (x != P_MAX_INDEX) {
                m_ffmpegFrameWidth = size.Left(x).AsUnsigned();
                m_ffmpegFrameHeight = size.Mid(x+1).AsUnsigned();
                PTRACE(3, "FFVDev\tVideo size parsed as " << m_ffmpegFrameWidth << "x" << m_ffmpegFrameHeight);
                state = -1;
              }
              if (tokens.GetSize() >= 11) {
                m_ffmpegFrameRate = tokens[10].AsUnsigned();
                PTRACE(3, "FFVDev\tVideo frame rate parsed as " << m_ffmpegFrameRate);
              }
            }
          }
          line.MakeEmpty();
        }
        else if (ch != '\n')
          line += ch;
      }
    }
  }

  // file is now open
  m_ffmpegFrameSize = CalculateFrameBytes(m_ffmpegFrameWidth, m_ffmpegFrameHeight, PVideoFrameInfo::YUV420P());
  SetFrameSize(m_ffmpegFrameWidth, m_ffmpegFrameHeight);

  m_deviceName = _deviceName;
  return true;    
}


PBoolean PVideoInputDevice_FFMPEG::IsOpen() 
{
  return m_command.IsOpen();
}


PBoolean PVideoInputDevice_FFMPEG::Close()
{
  m_command.Close();
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::Start()
{
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::Stop()
{
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::IsCapturing()
{
  return IsOpen();
}


PStringArray PVideoInputDevice_FFMPEG::GetInputDeviceNames()
{

  return PString(DefaultAVIFileName);
}


PBoolean PVideoInputDevice_FFMPEG::SetVideoFormat(VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat(newFormat);
}


PBoolean PVideoInputDevice_FFMPEG::SetColourFormat(const PString & newFormat)
{
  if (!(newFormat *= PVideoFrameInfo::YUV420P()))
    return false;

  return PVideoDevice::SetColourFormat(newFormat);
}


PBoolean PVideoInputDevice_FFMPEG::SetFrameRate(unsigned rate)
{
  return PVideoDevice::SetFrameRate(rate);
}


PBoolean PVideoInputDevice_FFMPEG::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  // can't set unless the file is open
  if (!m_command.IsOpen())
    return false;

  minWidth  = maxWidth  = m_ffmpegFrameWidth;
  minHeight = maxHeight = m_ffmpegFrameHeight;
  return true;
}

PBoolean PVideoInputDevice_FFMPEG::SetFrameSize(unsigned width, unsigned height)
{
  // can't set unless the file is open
  if (!m_command.IsOpen())
    return false;

  return width == m_ffmpegFrameWidth && height == m_ffmpegFrameHeight;
}


PINDEX PVideoInputDevice_FFMPEG::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(m_ffmpegFrameSize);
}


PBoolean PVideoInputDevice_FFMPEG::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{    
  pacing.Delay(1000/m_ffmpegFrameRate);    
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_FFMPEG::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  if (!m_command.IsOpen())
    return false;

  // make sure that stderr is emptied, as too much unread data 
  // will cause ffmpeg to silently stop 
  {
    PString text;
    m_command.ReadStandardError(text, false);
    PTRACE(5, "FFVDev\t" << text);
  }

  grabCount++;

  BYTE * readBuffer = destFrame;

  if (m_converter != NULL)
    readBuffer = m_frameStore.GetPointer(m_ffmpegFrameSize);

  unsigned len = 0;
  while (len < m_ffmpegFrameSize) {
    if (!m_command.Read(readBuffer+len, m_ffmpegFrameSize-len)) {
      m_command.Close();
      return false;
    }
    len += m_command.GetLastReadCount();
  }

  if (m_converter == NULL) {
    if (bytesReturned != NULL)
      *bytesReturned = m_ffmpegFrameSize;
  } else {
    m_converter->SetSrcFrameSize(m_ffmpegFrameWidth, m_ffmpegFrameHeight);
    if (!m_converter->Convert(readBuffer, destFrame, bytesReturned))
      return false;
    if (bytesReturned != NULL)
      *bytesReturned = m_converter->GetMaxDstFrameBytes();
  }

  return true;
}

#endif
#endif

