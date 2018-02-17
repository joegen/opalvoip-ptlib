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

  m_activeDirection = params.m_direction;

  if (params.m_direction == PSoundChannel::Player) {
    SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
    if (m_WAVFile.Open(params.m_device, PFile::WriteOnly)) {
      m_WAVFile.SetChannels(m_channels);
      m_WAVFile.SetSampleRate(m_sampleRate);
      m_WAVFile.SetSampleSize(m_bytesPerSample*8);
      return true;
    }

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


bool PSoundChannel_WAVFile::RawWrite(const void * data, PINDEX size)
{
  if (m_WAVFile.Write(data, size)) {
    SetLastWriteCount(m_WAVFile.GetLastWriteCount());
    return true;
  }

  SetLastWriteCount(0);
  SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber(), LastWriteError);
  return false;
}


bool PSoundChannel_WAVFile::RawRead(void * data, PINDEX size)
{
  if (m_WAVFile.Read(data, size)) {
    SetLastReadCount(m_WAVFile.GetLastReadCount() * 8 / m_WAVFile.GetSampleSize() * 1000 / m_WAVFile.GetSampleRate());
    return true;
  }

  SetLastReadCount(0);
  SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber(), LastReadError);
  return false;
}


bool PSoundChannel_WAVFile::Rewind()
{
  if (m_WAVFile.SetPosition(0))
    return true;

  SetErrorValues(m_WAVFile.GetErrorCode(), m_WAVFile.GetErrorNumber());
  return false;
}


#endif // P_WAVFILE


// End of File ///////////////////////////////////////////////////////////////
