/*
 * pwavfiledev.cxx
 *
 * Implementation of sound file device
 *
 * Portable Windows Library
 *
 * Copyright (C) 2007 Post Increment
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
 * Robert Jongbloed <robertj@postincrement.com>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifdef __GNUC__
#pragma implementation "pwavfiledev.h"
#endif

#include <ptlib.h>

#if P_WAVFILE

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/pwavfiledev.h>


PCREATE_SOUND_PLUGIN_EX(WAVFile, PSoundChannel_WAVFile,

  virtual const char * GetFriendlyName() const
  {
    return "Microsoft WAV File Sound Channel";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR userData) const
  {
    PFilePath pathname = deviceName;
    if (pathname.GetTitle().IsEmpty())
      return false;

    PINDEX last = pathname.GetLength()-1;
    if (userData == PSoundChannel::Recorder && pathname[last] == '*')
      pathname.Delete(last, 1);

    if (pathname.GetType() != ".wav")
      return false;

    if (userData == PSoundChannel::Recorder)
      return PFile::Access(pathname, PFile::ReadOnly);

    if (PFile::Exists(pathname))
      return PFile::Access(pathname, PFile::WriteOnly);

    return PFile::Access(pathname.GetDirectory(), PFile::WriteOnly);
  }
);


#define new PNEW
#define PTraceModule() "WAVFileDev"


///////////////////////////////////////////////////////////////////////////////

PSoundChannel_WAVFile::PSoundChannel_WAVFile()
  : m_bufferSize(320)
  , m_bufferCount(1)
  , m_bufferPos(0)
  , m_autoRepeat(false)
  , m_Pacing(1000)
  , m_muted(false)
{
}


PSoundChannel_WAVFile::~PSoundChannel_WAVFile()
{
  Close();
}


PString PSoundChannel_WAVFile::GetName() const
{
  return m_WAVFile.GetFilePath();
}


PStringArray PSoundChannel_WAVFile::GetDeviceNames(Directions)
{
  PStringArray devices;
  devices.AppendString("*.wav");
  return devices;
}


bool PSoundChannel_WAVFile::Open(const Params & params)
{
  Close();

  if (params.m_direction == PSoundChannel::Player) {
    SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
    if (m_WAVFile.Open(params.m_device, PFile::WriteOnly))
      return true;
    SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber());
    return false;
  }

  PString adjustedDevice = params.m_device;
  PINDEX lastCharPos = adjustedDevice.GetLength()-1;
  if (adjustedDevice[lastCharPos] == '*') {
    adjustedDevice.Delete(lastCharPos, 1);
    m_autoRepeat = true;
  }

  if (!m_WAVFile.Open(adjustedDevice, PFile::ReadOnly)) {
    SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber());
    return false;
  }

  if (params.m_sampleRate >= 8000 && m_WAVFile.GetSampleSize() == params.m_bitsPerSample) {
    SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
    return SetBuffers(320, 1);
  }

  Close();

  SetErrorValues(BadParameter, EINVAL);
  return false;
}


PBoolean PSoundChannel_WAVFile::IsOpen() const
{ 
  return m_WAVFile.IsOpen();
}

PBoolean PSoundChannel_WAVFile::SetFormat(unsigned numChannels,
                                      unsigned sampleRate,
                                      unsigned bitsPerSample)
{
  m_WAVFile.SetChannels(numChannels);
  m_WAVFile.SetSampleRate(sampleRate);
  m_WAVFile.SetSampleSize(bitsPerSample);

  return true;
}


unsigned PSoundChannel_WAVFile::GetChannels() const
{
  return m_WAVFile.GetChannels();
}


unsigned PSoundChannel_WAVFile::GetSampleRate() const
{
  return m_WAVFile.GetSampleRate();
}


unsigned PSoundChannel_WAVFile::GetSampleSize() const
{
  return m_WAVFile.GetSampleSize();
}


PBoolean PSoundChannel_WAVFile::Close()
{
  if (CheckNotOpen())
    return false;

  if (m_bufferPos > 0)
    m_WAVFile.Write(m_buffer, m_bufferPos);
  m_WAVFile.Close();
  os_handle = -1;
  return true;
}


PBoolean PSoundChannel_WAVFile::SetBuffers(PINDEX size, PINDEX count)
{
  m_bufferSize = size;
  m_bufferCount = count;
  if (!PAssert(m_buffer.SetSize(size*count), POutOfMemory))
    return false;

  PTRACE(3, "Setting write buffer to " << count << '*' << size << '=' << m_buffer.GetSize());
  m_bufferPos = 0;
  return true;
}


PBoolean PSoundChannel_WAVFile::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = m_bufferSize;
  count = m_bufferCount;
  return true;
}


PBoolean PSoundChannel_WAVFile::Write(const void * data, PINDEX size)
{
  if (!m_muted)
    return InternalWrite(data, size);

  short zero = 0;
  for (int i = 0; i < size; i += sizeof(zero)) {
    if (!InternalWrite(&zero, sizeof(zero)))
      return false;
  }

  SetLastWriteCount(size);
  return true;
}


bool PSoundChannel_WAVFile::InternalWrite(const void * data, PINDEX size)
{
  bool ok = true;
  if (m_bufferPos == 0 && size >= m_buffer.GetSize()) {
    ok = m_WAVFile.Write(data, size);
    if (ok)
      SetLastWriteCount(m_WAVFile.GetLastWriteCount());
  }
  else {
    if (m_bufferPos + size > m_buffer.GetSize()) {
      PTRACE(4, "Flushing write buffer: " << m_bufferPos << " bytes");
      ok = m_WAVFile.Write(m_buffer, m_bufferPos);
      m_bufferPos = 0;
    }

    if (ok) {
      memcpy(m_buffer.GetPointer() + m_bufferPos, data, size);
      m_bufferPos += size;
      SetLastWriteCount(size);
    }
  }

  if (ok)
    m_Pacing.Delay(GetLastWriteCount() * 8 / m_WAVFile.GetSampleSize() * 1000 / m_WAVFile.GetSampleRate() / m_WAVFile.GetChannels());
  else {
    SetLastWriteCount(0);
    SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber());
  }
  return ok;
}


PBoolean PSoundChannel_WAVFile::HasPlayCompleted()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::WaitForPlayCompletion()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::StartRecording()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::Read(void * data, PINDEX size)
{
  for (int retry = 0; retry < 2; ++retry) {
    if (m_WAVFile.Read(data, size)) {
      PINDEX count = m_WAVFile.GetLastReadCount();
      if (m_muted)
        memset(data, 0, count);
      m_Pacing.Delay(count * 8 / m_WAVFile.GetSampleSize() * 1000 / m_WAVFile.GetSampleRate());
      SetLastReadCount(count);
      return true;
    }

    if (m_WAVFile.GetErrorCode(LastReadError) != NoError) {
      PTRACE(2, "Error reading file: " << m_WAVFile.GetErrorText(LastReadError));
      return false;
    }

    if (!m_autoRepeat) {
      PTRACE(3, "End of file, stopping");
      return false;
    }

    PTRACE_IF(4, retry == 0, "End of file, repeating");
    m_WAVFile.SetPosition(0);
  }

  PTRACE(2, "File is empty, cannot repeat");
  return false;
}


PBoolean PSoundChannel_WAVFile::IsRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::AreAllRecordBuffersFull()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::WaitForRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::WaitForAllRecordBuffersFull()
{
  return true;
}


PBoolean PSoundChannel_WAVFile::SetVolume(unsigned volume)
{
  if (volume > 100)
    return false;

  m_muted = volume == 0;
  return true;
}


PBoolean PSoundChannel_WAVFile::GetVolume(unsigned & volume)
{
  volume = m_muted ? 0 : 100;
  return true;
}


bool PSoundChannel_WAVFile::SetMute(bool mute)
{
  m_muted = mute;
  return true;
}


bool PSoundChannel_WAVFile::GetMute(bool & mute)
{
  mute = m_muted;
  return true;
}


#endif // P_WAVFILE


// End of File ///////////////////////////////////////////////////////////////
