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
#define PTraceModule() "VidFileDev"

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
{
  SetColourFormat(PVideoFrameInfo::YUV420P());
}


PVideoInputDevice_VideoFile::~PVideoInputDevice_VideoFile()
{
  Close();
}


PStringArray PVideoInputDevice_VideoFile::GetInputDeviceNames()
{
  PStringArray names;

  PVideoFileFactory::KeyList_T keyList = PVideoFileFactory::GetKeyList();
  for (PVideoFileFactory::KeyList_T::iterator it = keyList.begin(); it != keyList.end(); ++it)
    names.AppendString("*" + *it);

  return names;
}


PStringArray PVideoInputDevice_VideoFile::GetDeviceNames() const
{
  return GetInputDeviceNames();
}


PBoolean PVideoInputDevice_VideoFile::Open(const PString & devName, PBoolean /*startImmediate*/)
{
  PWriteWaitAndSignal lock(m_mutex);

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
    PTRACE(1, "Searching directory \"" << dir << '"');
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
      PTRACE(1, "Cannot find any file using " << PDirectory()  << " as source for video input device");
      return false;
    }
  }

  m_file = PVideoFileFactory::CreateInstance(filePath.GetType());
  if (m_file == NULL) {
    PTRACE(1, "Cannot open file of type \"" << filePath.GetType() << "\" as video input device");
    return false;
  }

  if (!m_file->Open(filePath, PFile::ReadOnly, PFile::MustExist)) {
    PTRACE(1, "Cannot open file \"" << filePath << "\" as video input device: " << m_file->GetErrorText());
    return false;
  }

  PTRACE(3, "Opening file " << filePath);

  *static_cast<PVideoFrameInfo *>(this) = *m_file;

  if (m_file->IsFixedFrameRate())
    m_fixedFrameRate = m_file->GetFrameRate();

  m_deviceName = m_file->GetFilePath();
  return true;
}


PBoolean PVideoInputDevice_VideoFile::IsOpen() 
{
  PReadWaitAndSignal lock(m_mutex);
  return m_file != NULL && m_file->IsOpen();
}


PBoolean PVideoInputDevice_VideoFile::Close()
{
  PWriteWaitAndSignal lock(m_mutex);

  PBoolean ok = m_file != NULL && m_file->Close();

  PThread::Sleep(1000/m_frameRate);

  delete m_file;
  m_file = NULL;

  return ok;
}


bool PVideoInputDevice_VideoFile::InternalReadFrameData(BYTE * frame)
{
  PReadWaitAndSignal lock(m_mutex);
  m_file->SetPosition(m_frameNumber++);
  return m_file->ReadFrame(frame);
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
    PTRACE(1, "Cannot create file " << fileName << " as video output device");
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
  return (newFormat *= PVideoFrameInfo::YUV420P()) && PVideoDevice::SetColourFormat(newFormat);
}


PBoolean PVideoOutputDevice_VideoFile::SetFrameData(const FrameData & frameData)
{
  if (!m_opened || PAssertNULL(m_file) == NULL) {
    PTRACE(5, "Abort SetFrameData, closed.");
    return false;
  }

  if (frameData.x != 0 || frameData.y != 0 || frameData.width != m_frameWidth || frameData.height != m_frameHeight) {
    PTRACE(1, "Output device only supports full frame writes");
    return false;
  }

  if (!m_file->SetFrameSize(frameData.width, frameData.height))
    return false;

  if (m_converter == NULL)
    return m_file->WriteFrame(frameData.pixels);

  m_converter->Convert(frameData.pixels, m_frameStore.GetPointer(GetMaxFrameBytes()));
  return m_file->WriteFrame(m_frameStore);
}


#endif // P_VIDFILE
#endif

