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
 *
 * $Revision$
 * $Author$
 * $Date$
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


///////////////////////////////////////////////////////////////////////////////

PSoundChannel_WAVFile::PSoundChannel_WAVFile()
  : m_bufferSize(2)
  , m_autoRepeat(false)
  , m_Pacing(1000)
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

  if (params.m_sampleRate >= 8000 && m_WAVFile.GetSampleSize() == params.m_bitsPerSample)
    return true;

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

  m_WAVFile.Close();
  os_handle = -1;
  return true;
}


PBoolean PSoundChannel_WAVFile::SetBuffers(PINDEX size, PINDEX /*count*/)
{
  m_bufferSize = size;
  return true;
}


PBoolean PSoundChannel_WAVFile::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = m_bufferSize;
  count = 1;
  return true;
}


PBoolean PSoundChannel_WAVFile::Write(const void * data, PINDEX size)
{
  PBoolean ok = m_WAVFile.Write(data, size);
  lastWriteCount = m_WAVFile.GetLastWriteCount();
  m_Pacing.Delay(lastWriteCount*8/m_WAVFile.GetSampleSize()*1000/m_WAVFile.GetSampleRate());
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
  if (m_WAVFile.Read(data, size))
    return true;

  if (m_WAVFile.GetErrorCode(LastReadError) != NoError) {
    PTRACE(2, "WAVFileDev", "Error reading file: " << m_WAVFile.GetErrorText(LastReadError));
    return false;
  }

  if (!m_autoRepeat) {
    PTRACE(3, "WAVFileDev", "End of file, stopping");
    return false;
  }

  PTRACE(4, "WAVFileDev", "End of file, repeating");
  m_WAVFile.SetPosition(0);
  if (m_WAVFile.Read(data, size))
    return true;

  PTRACE(2, "WAVFileDev", "Error reading file: " << m_WAVFile.GetErrorText(LastReadError));
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


#endif // P_WAVFILE


// End of File ///////////////////////////////////////////////////////////////
