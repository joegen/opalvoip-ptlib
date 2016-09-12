/*
 * pvfiledev.cxx
 *
 * Video file declaration
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
#pragma implementation "pvfiledev.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/pvfiledev.h>
#include <ptlib/pfactory.h>


#define new PNEW


///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_VideoFile

PCREATE_VIDINPUT_PLUGIN_EX(VideoFile,

  virtual const char * GetFriendlyName() const
  {
    return "Raw YUV File Video Input";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    PVideoFileFactory::KeyList_T keyList = PVideoFileFactory::GetKeyList();
    return std::find(keyList.begin(), keyList.end(), PFilePath(deviceName).GetType()) != keyList.end();
  }
);



PVideoInputDevice_VideoFile::PVideoInputDevice_VideoFile()
  : m_file(NULL)
  , m_pacing(500)
  , m_frameRateAdjust(0)
  , m_opened(false)
{
  SetColourFormat("YUV420P");
}


PVideoInputDevice_VideoFile::~PVideoInputDevice_VideoFile()
{
  Close();
}


PBoolean PVideoInputDevice_VideoFile::Open(const PString & devName, PBoolean /*startImmediate*/)
{
  Close();

  if (devName.IsEmpty())
    return false;

  PFilePath filePath;

  PINDEX pos = devName.GetLength()-1;
  if (devName[pos] != '*')
    filePath = devName;
  else {
    filePath = devName.Left(pos);
    SetChannel(Channel_PlayAndRepeat);
  }

  if (filePath.Find('*') != P_MAX_INDEX) {
    bool noFilesOfType = true;
    PDirectory dir = filePath.GetDirectory();
    PTRACE(1, "VidFileDev", "Searching directory \"" << dir << '"');
    if (dir.Open(PFileInfo::RegularFile|PFileInfo::SymbolicLink)) {
      do {
        PFilePath dirFile = dir + dir.GetEntryName();
        if (dirFile.GetType() == filePath.GetType()) {
          filePath = dirFile;
          noFilesOfType = false;
          break;
        }
      } while (dir.Next());
    }
    if (noFilesOfType) {
      PTRACE(1, "VidFileDev\tCannot find any file using " << PDirectory()  << " as source for video input device");
      return false;
    }
  }

  m_file = PVideoFileFactory::CreateInstance(filePath.GetType());
  if (m_file == NULL || !m_file->Open(filePath, PFile::ReadOnly, PFile::MustExist)) {
    PTRACE(1, "VidFileDev\tCannot open file \"" << filePath << "\" as video input device");
    return false;
  }

  PTRACE(3, "VidFileDev", "Opening file " << filePath);

  *static_cast<PVideoFrameInfo *>(this) = *m_file;

  m_deviceName = m_file->GetFilePath();
  m_opened = true;
  return true;
}


PBoolean PVideoInputDevice_VideoFile::IsOpen() 
{
  return m_opened;
}


PBoolean PVideoInputDevice_VideoFile::Close()
{
  m_opened = false;

  PBoolean ok = m_file != NULL && m_file->Close();

  PThread::Sleep(1000/m_frameRate);

  delete m_file;
  m_file = NULL;

  return ok;
}


PBoolean PVideoInputDevice_VideoFile::Start()
{
  return true;
}


PBoolean PVideoInputDevice_VideoFile::Stop()
{
  return true;
}


PBoolean PVideoInputDevice_VideoFile::IsCapturing()
{
  return IsOpen();
}


PStringArray PVideoInputDevice_VideoFile::GetInputDeviceNames()
{
  PStringArray names;

  PVideoFileFactory::KeyList_T keyList = PVideoFileFactory::GetKeyList();
  for (PVideoFileFactory::KeyList_T::iterator it = keyList.begin(); it != keyList.end(); ++it)
    names.AppendString("*" + *it);

  return names;
}


PBoolean PVideoInputDevice_VideoFile::SetVideoFormat(VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat(newFormat);
}


int PVideoInputDevice_VideoFile::GetNumChannels() 
{
  return ChannelCount;  
}


PStringArray PVideoInputDevice_VideoFile::GetChannelNames()
{
  PStringArray names(ChannelCount);
  names[0] = "Once, then close";
  names[1] = "Repeat";
  names[2] = "Once, then still";
  names[3] = "Once, then black";
  return names;
}


PBoolean PVideoInputDevice_VideoFile::SetColourFormat(const PString & newFormat)
{
  return (m_colourFormat *= newFormat);
}


PBoolean PVideoInputDevice_VideoFile::SetFrameRate(unsigned rate)
{
  // Set file, if it will change, if not convert in GetFrameData
  if (m_file != NULL)
    m_file->SetFrameRate(rate);

  return PVideoDevice::SetFrameRate(rate);
}


PBoolean PVideoInputDevice_VideoFile::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  if (m_file == NULL) {
    PTRACE(2, "VidFileDev\tCannot get frame size limits, no file opened.");
    return false;
  }

  unsigned width, height;
  if (!m_file->GetFrameSize(width, height))
    return false;

  minWidth  = maxWidth  = width;
  minHeight = maxHeight = height;
  return true;
}


PBoolean PVideoInputDevice_VideoFile::SetFrameSize(unsigned width, unsigned height)
{
  if (m_file == NULL) {
    PTRACE(2, "VidFileDev\tCannot set frame size, no file opened.");
    return false;
  }

  return m_file->SetFrameSize(width, height) && PVideoDevice::SetFrameSize(width, height);
}


PINDEX PVideoInputDevice_VideoFile::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(m_file->GetFrameBytes());
}


PBoolean PVideoInputDevice_VideoFile::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  m_pacing.Delay(1000/m_frameRate);

  if (!m_opened || PAssertNULL(m_file) == NULL) {
    PTRACE(5, "VidFileDev\tAbort GetFrameData, closed.");
    return false;
  }

  off_t frameNumber = m_file->GetPosition();

  unsigned fileRate = m_file->GetFrameRate();
  if (fileRate > m_frameRate) {
    m_frameRateAdjust += fileRate;
    while (m_frameRateAdjust > m_frameRate) {
      m_frameRateAdjust -= m_frameRate;
      ++frameNumber;
    }
    --frameNumber;
  }
  else if (fileRate < m_frameRate) {
    if (m_frameRateAdjust < m_frameRate)
      m_frameRateAdjust += fileRate;
    else {
      m_frameRateAdjust -= m_frameRate;
      --frameNumber;
    }
  }

  PTRACE(6, "VidFileDev\tPlaying frame number " << frameNumber);
  m_file->SetPosition(frameNumber);

  return GetFrameDataNoDelay(buffer, bytesReturned);
}


PBoolean PVideoInputDevice_VideoFile::GetFrameDataNoDelay(BYTE * frame, PINDEX * bytesReturned)
{
  if (!m_opened || PAssertNULL(m_file) == NULL) {
    PTRACE(5, "VidFileDev\tAbort GetFrameDataNoDelay, closed.");
    return false;
  }

  BYTE * readBuffer = m_converter != NULL ? m_frameStore.GetPointer(m_file->GetFrameBytes()) : frame;

  if (m_file->IsOpen()) {
    if (!m_file->ReadFrame(readBuffer))
      m_file->Close();
  }

  if (!m_file->IsOpen()) {
    switch (m_channelNumber) {
      case Channel_PlayAndClose:
      default:
        PTRACE(4, "VidFileDev\tCompleted play and close of " << m_file->GetFilePath());
        return false;

      case Channel_PlayAndRepeat:
        m_file->Open(m_deviceName, PFile::ReadOnly, PFile::MustExist);
        if (!m_file->SetPosition(0)) {
          PTRACE(2, "VidFileDev\tCould not rewind " << m_file->GetFilePath());
          return false;
        }
        if (!m_file->ReadFrame(readBuffer))
          return false;
        break;

      case Channel_PlayAndKeepLast:
        PTRACE(4, "VidFileDev\tCompleted play and keep last of " << m_file->GetFilePath());
        break;

      case Channel_PlayAndShowBlack:
        PTRACE(4, "VidFileDev\tCompleted play and show black of " << m_file->GetFilePath());
        PColourConverter::FillYUV420P(0, 0,
                                      m_frameWidth, m_frameHeight,
                                      m_frameWidth, m_frameHeight,
                                      readBuffer,
                                      100, 100, 100);
        break;
    }
  }

  if (m_converter == NULL) {
    if (bytesReturned != NULL)
      *bytesReturned = m_file->GetFrameBytes();
  }
  else {
    m_converter->SetSrcFrameSize(m_frameWidth, m_frameHeight);
    if (!m_converter->Convert(readBuffer, frame, bytesReturned)) {
      PTRACE(2, "VidFileDev\tConversion failed with " << *m_converter);
      return false;
    }

    if (bytesReturned != NULL)
      *bytesReturned = m_converter->GetMaxDstFrameBytes();
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice_VideoFile

PCREATE_VIDOUTPUT_PLUGIN_EX(VideoFile,

  virtual const char * GetFriendlyName() const
  {
    return "Raw YUV File Video Output";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    return (deviceName.Right(4) *= ".yuv") && (!PFile::Exists(deviceName) || PFile::Access(deviceName, PFile::WriteOnly));
  }
);


PVideoOutputDevice_VideoFile::PVideoOutputDevice_VideoFile()
  : m_file(NULL)
  , m_opened(false)
{
}


PVideoOutputDevice_VideoFile::~PVideoOutputDevice_VideoFile()
{
  Close();
}

static const char DefaultYUVFileName[] = "*.yuv";

PBoolean PVideoOutputDevice_VideoFile::Open(const PString & devName, PBoolean /*startImmediate*/)
{
  PFilePath fileName;
  if (devName != DefaultYUVFileName)
    fileName = devName;
  else {
    unsigned unique = 0;
    do {
      fileName.Empty();
      fileName.sprintf("video%03u.yuv", ++unique);
    } while (PFile::Exists(fileName));
  }

  m_file = PVideoFileFactory::CreateInstance(fileName.GetType());
  if (m_file == NULL || !m_file->Open(fileName, PFile::WriteOnly, PFile::Create|PFile::Truncate)) {
    PTRACE(1, "VideoFile\tCannot create file " << fileName << " as video output device");
    return false;
  }

  m_deviceName = m_file->GetFilePath();
  m_opened = true;
  return true;
}

PBoolean PVideoOutputDevice_VideoFile::Close()
{
  m_opened = false;

  PBoolean ok = m_file == NULL || m_file->Close();

  PThread::Sleep(10);

  delete m_file;
  m_file = NULL;

  return ok;
}

PBoolean PVideoOutputDevice_VideoFile::Start()
{
  return m_file != NULL && m_file->SetFrameSize(m_frameHeight, m_frameWidth);
}

PBoolean PVideoOutputDevice_VideoFile::Stop()
{
  return true;
}

PBoolean PVideoOutputDevice_VideoFile::IsOpen()
{
  return m_opened;
}


PStringArray PVideoOutputDevice_VideoFile::GetOutputDeviceNames()
{
  PStringArray names;

  PVideoFileFactory::KeyList_T keyList = PVideoFileFactory::GetKeyList();
  for (PVideoFileFactory::KeyList_T::iterator it = keyList.begin(); it != keyList.end(); ++it)
    names.AppendString("*" + *it);

  return names;
}


PBoolean PVideoOutputDevice_VideoFile::SetColourFormat(const PString & newFormat)
{
  return (newFormat *= "YUV420P") && PVideoDevice::SetColourFormat(newFormat);
}


PINDEX PVideoOutputDevice_VideoFile::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat));
}


PBoolean PVideoOutputDevice_VideoFile::SetFrameData(unsigned x, unsigned y,
                                              unsigned width, unsigned height,
                                              const BYTE * data,
                                              PBoolean /*endFrame*/)
{
  if (!m_opened || PAssertNULL(m_file) == NULL) {
    PTRACE(5, "VidFileDev\tAbort SetFrameData, closed.");
    return false;
  }

  if (x != 0 || y != 0 || width != m_frameWidth || height != m_frameHeight) {
    PTRACE(1, "VideoFile\tOutput device only supports full frame writes");
    return false;
  }

  if (!m_file->SetFrameSize(width, height))
    return false;

  if (m_converter == NULL)
    return m_file->WriteFrame(data);

  m_converter->Convert(data, m_frameStore.GetPointer(GetMaxFrameBytes()));
  return m_file->WriteFrame(m_frameStore);
}


#endif // P_VIDFILE
#endif

