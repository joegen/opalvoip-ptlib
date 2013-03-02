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

#include <ptlib/vconvert.h>
#include <ptclib/pvfiledev.h>
#include <ptlib/pfactory.h>
#include <ptlib/pluginmgr.h>
#include <ptlib/videoio.h>


#define new PNEW


///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_VideoFile

class PVideoInputDevice_VideoFile_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    typedef PFactory<PVideoFile> FileTypeFactory_T;

    virtual PObject * CreateInstance(int /*userData*/) const
    {
      return new PVideoInputDevice_VideoFile;
    }

    virtual PStringArray GetDeviceNames(int /*userData*/) const
    {
      return PVideoInputDevice_VideoFile::GetInputDeviceNames();
    }

    virtual bool ValidateDeviceName(const PString & deviceName, int /*userData*/) const
    {
      PCaselessString adjustedDevice = deviceName;

      FileTypeFactory_T::KeyList_T keyList = FileTypeFactory_T::GetKeyList();
      FileTypeFactory_T::KeyList_T::iterator r;
      for (r = keyList.begin(); r != keyList.end(); ++r) {
        PString ext = *r;
        PINDEX extLen = ext.GetLength();
        PINDEX length = adjustedDevice.GetLength();
        if (length > (2+extLen) && adjustedDevice.NumCompare(PString(".") + ext + "*", 2+extLen, length-(2+extLen)) == PObject::EqualTo)
          adjustedDevice.Delete(length-1, 1);
        else if (length < (2+extLen) || adjustedDevice.NumCompare(PString(".") + ext, 1+extLen, length-(1+extLen)) != PObject::EqualTo)
          continue;
cout << "checking extension " << ext << " with '" << adjustedDevice << "'" << endl;
        if (PFile::Access(adjustedDevice, PFile::ReadOnly)) 
          return true;
        //PTRACE(1, "Unable to access file '" << adjustedDevice << "' for use as a video input device");
        //return false;
      }
      return false;
    }
} PVideoInputDevice_VideoFile_descriptor;

PCREATE_PLUGIN(VideoFile, PVideoInputDevice, &PVideoInputDevice_VideoFile_descriptor);

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

  PFilePath fileName;
  PString extension;

  if (devName.Left(2) != "*.") {
    fileName = devName;
    PINDEX pos = fileName.GetLength()-1;
    if (fileName[pos] == '*') {
      fileName.Delete(pos, 1);
      SetChannel(Channel_PlayAndRepeat);
    }
    extension = fileName.GetType();
    if (extension[0] == '.')
      extension = extension.Mid(1);
    PTRACE(1, "VidFileDev", "Opening file " << devName << "(" << fileName << ") with extension " << extension);
  }
  else {
    PTRACE(1, "VidFileDev", "Opening dir " << devName);
    PFactory<PVideoFile>::KeyList_T keyList = PFactory<PVideoFile>::GetKeyList();
    PFactory<PVideoFile>::KeyList_T::iterator r;
    bool found = false;
    for (r = keyList.begin(); !found && (r != keyList.end()); ++r) {
      extension = *r;
      PDirectory dir;
      if (dir.Open(PFileInfo::RegularFile|PFileInfo::SymbolicLink)) {
        do {
          if (dir.GetEntryName().Right(extension.GetLength()) == (PString(".") + extension)) {
            fileName = dir.GetEntryName();
            found = true;
            break;
          }
        } while (dir.Next());
      }
    }
    if (fileName.IsEmpty()) {
      PTRACE(1, "VidFileDev\tCannot find any file using " << PDirectory()  << " as source for video input device");
      return false;
    }
  }

  PTRACE(1, "VidFileDev", "Opening file with extension " << extension);

  m_file = PFactory<PVideoFile>::CreateInstance(extension);
  if (m_file == NULL || !m_file->Open(fileName, PFile::ReadOnly, PFile::MustExist)) {
    PTRACE(1, "VidFileDev\tCannot open file " << fileName << " as video input device");
    return false;
  }

  *static_cast<PVideoFrameInfo *>(this) = *static_cast<PVideoFrameInfo *>(m_file);

  deviceName = m_file->GetFilePath();
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

  PThread::Sleep(1000/frameRate);

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

  PFactory<PVideoFile>::KeyList_T keyList = PFactory<PVideoFile>::GetKeyList();
  PFactory<PVideoFile>::KeyList_T::iterator r;
  for (r = keyList.begin(); r != keyList.end(); ++r) {
    PString ext = *r;
    names.AppendString("*." + ext);
  }

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


PBoolean PVideoInputDevice_VideoFile::SetChannel(int newChannel)
{
  return newChannel < 0 || PVideoDevice::SetChannel(newChannel);
}


PBoolean PVideoInputDevice_VideoFile::SetColourFormat(const PString & newFormat)
{
  return (colourFormat *= newFormat);
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
  m_pacing.Delay(1000/frameRate);

  if (!m_opened || PAssertNULL(m_file) == NULL) {
    PTRACE(5, "VidFileDev\tAbort GetFrameData, closed.");
    return false;
  }

  off_t frameNumber = m_file->GetPosition();

  unsigned fileRate = m_file->GetFrameRate();
  if (fileRate > frameRate) {
    m_frameRateAdjust += fileRate;
    while (m_frameRateAdjust > frameRate) {
      m_frameRateAdjust -= frameRate;
      ++frameNumber;
    }
    --frameNumber;
  }
  else if (fileRate < frameRate) {
    if (m_frameRateAdjust < frameRate)
      m_frameRateAdjust += fileRate;
    else {
      m_frameRateAdjust -= frameRate;
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

  BYTE * readBuffer = converter != NULL ? frameStore.GetPointer(m_file->GetFrameBytes()) : frame;

  if (m_file->IsOpen()) {
    if (!m_file->ReadFrame(readBuffer))
      m_file->Close();
  }

  if (!m_file->IsOpen()) {
    switch (channelNumber) {
      case Channel_PlayAndClose:
      default:
        PTRACE(4, "VidFileDev\tCompleted play and close of " << m_file->GetFilePath());
        return false;

      case Channel_PlayAndRepeat:
        m_file->Open(deviceName, PFile::ReadOnly, PFile::MustExist);
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
                                      frameWidth, frameHeight,
                                      frameWidth, frameHeight,
                                      readBuffer,
                                      100, 100, 100);
        break;
    }
  }

  if (converter == NULL) {
    if (bytesReturned != NULL)
      *bytesReturned = m_file->GetFrameBytes();
  }
  else {
    converter->SetSrcFrameSize(frameWidth, frameHeight);
    if (!converter->Convert(readBuffer, frame, bytesReturned)) {
      PTRACE(2, "VidFileDev\tConversion failed with " << *converter);
      return false;
    }

    if (bytesReturned != NULL)
      *bytesReturned = converter->GetMaxDstFrameBytes();
  }

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice_VideoFile

class PVideoOutputDevice_VideoFile_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject * CreateInstance(int /*userData*/) const
    {
        return new PVideoOutputDevice_VideoFile;
    }
    virtual PStringArray GetDeviceNames(int /*userData*/) const
    {
        return PVideoOutputDevice_VideoFile::GetOutputDeviceNames();
    }
    virtual bool ValidateDeviceName(const PString & deviceName, int /*userData*/) const
    {
      return (deviceName.Right(4) *= ".yuv") && (!PFile::Exists(deviceName) || PFile::Access(deviceName, PFile::WriteOnly));
    }
} PVideoOutputDevice_VideoFile_descriptor;

PCREATE_PLUGIN(VideoFile, PVideoOutputDevice, &PVideoOutputDevice_VideoFile_descriptor);


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

  m_file = PFactory<PVideoFile>::CreateInstance("yuv");
  if (m_file == NULL || !m_file->Open(fileName, PFile::WriteOnly, PFile::Create|PFile::Truncate)) {
    PTRACE(1, "VideoFile\tCannot create file " << fileName << " as video output device");
    return false;
  }

  deviceName = m_file->GetFilePath();
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
  return m_file != NULL && m_file->SetFrameSize(frameHeight, frameWidth);
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

  PFactory<PVideoFile>::KeyList_T keyList = PFactory<PVideoFile>::GetKeyList();
  PFactory<PVideoFile>::KeyList_T::iterator r;
  for (r = keyList.begin(); r != keyList.end(); ++r) {
    PString ext = *r;
    names.AppendString("*." + ext);
  }

  return names;
}


PBoolean PVideoOutputDevice_VideoFile::SetColourFormat(const PString & newFormat)
{
  return (newFormat *= "YUV420P") && PVideoDevice::SetColourFormat(newFormat);
}


PINDEX PVideoOutputDevice_VideoFile::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(CalculateFrameBytes(frameWidth, frameHeight, colourFormat));
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

  if (x != 0 || y != 0 || width != frameWidth || height != frameHeight) {
    PTRACE(1, "VideoFile\tOutput device only supports full frame writes");
    return false;
  }

  if (!m_file->SetFrameSize(width, height))
    return false;

  if (converter == NULL)
    return m_file->WriteFrame(data);

  converter->Convert(data, frameStore.GetPointer(GetMaxFrameBytes()));
  return m_file->WriteFrame(frameStore);
}


#endif // P_VIDFILE
#endif

