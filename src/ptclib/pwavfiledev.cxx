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
  : m_Pacing(1000)
  , m_autoRepeat(false)
  , m_sampleRate(8000)
  , m_bufferSize(2)
  , m_samplePosition(P_MAX_INDEX)
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

  m_sampleRate = params.m_sampleRate;
  m_channels = params.m_channels;

  if (m_sampleRate >= 8000 && m_WAVFile.GetSampleSize() == params.m_bitsPerSample) {
#if PTRACING
    static unsigned const Level = 3;
    if (PTrace::CanTrace(Level)) {
      ostream & trace = PTRACE_BEGIN(Level, "WAVFileDev");
      trace << "Opened \"" << m_WAVFile.GetFilePath() << "\" at " << m_sampleRate << "Hz";
      if (m_sampleRate != m_WAVFile.GetSampleRate())
        trace << " (converted from " << m_WAVFile.GetSampleRate() << "Hz)";
      trace << " using " << m_channels;
      if (m_channels != m_WAVFile.GetChannels())
        trace << " (converted from " << m_WAVFile.GetChannels() << ')';
      trace << " channel";
      if (m_channels > 1)
        trace << 's';
      trace << '.'
            << PTrace::End;
    }
#endif
    return true;
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
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

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
  lastReadCount = 0;

  unsigned wavSampleRate = m_WAVFile.GetSampleRate();
  unsigned wavChannels = m_WAVFile.GetChannels();
  short * wavSample = (short *)alloca(wavChannels*sizeof(short));
  short * pPCM = (short *)data;

  if (wavSampleRate < m_sampleRate) {
    // File has less samples than we want, so we need to interpolate
    unsigned iDutyCycle = m_sampleRate - wavSampleRate;
    while (lastReadCount < size) {
      iDutyCycle += wavSampleRate;
      if (iDutyCycle >= m_sampleRate) {
        iDutyCycle -= m_sampleRate;
        if (!ReadSample(wavSample, wavChannels))
          return false;
      }
      CopyMerge(pPCM, wavSample, wavChannels);
    }
  }
  else if (wavSampleRate > m_sampleRate) {
    // File has more samples than we want, so we need to throw some away
    unsigned iDutyCycle = 0;
    while (lastReadCount < size) {
      do {
        if (!ReadSample(wavSample, wavChannels))
          return false;
        iDutyCycle += m_sampleRate;
      } while (iDutyCycle < wavSampleRate);
      iDutyCycle -= wavSampleRate;
      CopyMerge(pPCM, wavSample, wavChannels);
    }
  }
  else if (wavChannels != m_channels) {
    while (lastReadCount < size) {
      if (!ReadSample(wavSample, wavChannels))
        return false;
      CopyMerge(pPCM, wavSample, wavChannels);
    }
  }
  else {
    if (!ReadSamples(data, size))
      return false;
    lastReadCount = m_WAVFile.GetLastReadCount();
    PINDEX pad = ((lastReadCount+m_bufferSize-1)/m_bufferSize)*m_bufferSize - lastReadCount;
    if (lastReadCount+pad > size)
      pad = size - lastReadCount;
    PTRACE(6, "WAVFileDev", "Direct read of " << lastReadCount << " bytes, pad=" << pad << ", pos=" << m_WAVFile.GetPosition());
    memset((char *)data+lastReadCount, 0, pad);
    lastReadCount += pad;
  }

  m_Pacing.Delay(lastReadCount*8/m_WAVFile.GetSampleSize()*1000/m_sampleRate/m_channels);
  return true;
}


void PSoundChannel_WAVFile::CopyMerge(short * & output, const short * wavSample, unsigned wavChannels)
{
  if (wavChannels == m_channels) {
    PINDEX bytes = wavChannels*sizeof(short);
    memcpy(output, wavSample, bytes);
    lastReadCount += bytes;
    output += m_channels;
  }

  else if (wavChannels == 1) {
    for (unsigned c = 0; c < m_channels; ++c)
      *output++ = *wavSample;
    lastReadCount += m_channels*sizeof(short);
  }

  else if (m_channels == 1) {
    int sum = 0;
    for (unsigned c = 0; c < wavChannels; ++c)
      sum += *wavSample++;
    *output++ = (short)(sum/wavChannels);
    lastReadCount += sizeof(short);
  }

  else {
    // Complicated ones, we cheat mightily and make it all mono
    int sum = 0;
    for (unsigned c = 0; c < wavChannels; ++c)
      sum += *wavSample++;
    for (unsigned c = 0; c < m_channels; ++c)
      *output++ = (short)(sum/wavChannels);
    lastReadCount += m_channels*sizeof(short);
  }
}


bool PSoundChannel_WAVFile::ReadSample(short * sample, unsigned channels)
{
  while (channels-- > 0) {
    if (m_samplePosition >= m_sampleBuffer.GetSize()) {
      static const PINDEX BufferSize = 10000;
      if (!ReadSamples(m_sampleBuffer.GetPointer(BufferSize), BufferSize*sizeof(short)))
        return false;
      m_sampleBuffer.SetSize(m_WAVFile.GetLastReadCount()/sizeof(short));
      m_samplePosition = 0;
    }
    *sample++ = m_sampleBuffer[m_samplePosition++];
  }
  return true;
}


bool PSoundChannel_WAVFile::ReadSamples(void * data, PINDEX size)
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
  return m_WAVFile.Read(data, size);
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
