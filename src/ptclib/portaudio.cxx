/*
 * portaudio.cxx
 *
 * PortAudio sound driver
 *
 * Portable Tools Library
 *
 * Copyright (C) 2013 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 28635 $
 * $Author: rjongbloed $
 * $Date: 2012-12-04 14:32:16 +1100 (Tue, 04 Dec 2012) $
 */

#include <ptlib.h>

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>

#include <ptlib/plugin.h>
#include <ptclib/portaudio.h>

PCREATE_SOUND_PLUGIN(PortAudio, PSoundChannelPortAudio);

#define new PNEW

static char PortAudioPrefix[] = "PortAudio - ";
#define PORTAUDIO_PREFIX_LEN	((PINDEX)sizeof(PortAudioPrefix)-1)

///////////////////////////////////////////////////////////////////////////////

PMutex & PSoundChannelPortAudio::GetInitMutex()
{
  static PMutex mutex;
  return mutex;
}

bool PSoundChannelPortAudio::Initialise()
{
  static bool first = true;
  static int err = 0;

  PWaitAndSignal m(GetInitMutex());
  if (!first) 
    return true;
  first = false;
  err = Pa_Initialize();
  //if (err != paNoError)
  //  PTRACE(1, "PortAudio\tPa_Initialise error " << Pa_GetErrorText(err));

  return err == paNoError;
}


PSoundChannelPortAudio::PSoundChannelPortAudio()
{
  Initialise();
  m_deviceInfo = NULL;
  m_started    = false;
  m_stream     = NULL;
  m_mute       = false;
  m_muteBuffer = NULL;

#ifdef P_PORTMIXER
  m_mixer      = NULL;
#endif
  m_volume     = 50;

  m_playBufferSize  = 480;
  m_playBufferCount = 3;
}


PSoundChannelPortAudio::~PSoundChannelPortAudio()
{
  Close();
}


PString PSoundChannelPortAudio::GetName() const
{
  return m_deviceName;
}


PStringArray PSoundChannelPortAudio::GetDeviceNames(Directions dir)
{
  Initialise();

  PStringArray devices;

  PWaitAndSignal m(GetInitMutex());

  int numDevices = Pa_GetDeviceCount();

  if (numDevices > 0) {
    int i;
    for (i = 0;i < numDevices; i++) {
      const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);
      if (
          (deviceInfo != NULL) &&
          (
           ((dir == Recorder) && (deviceInfo->maxInputChannels > 0)) ||
           ((dir == Player)   && (deviceInfo->maxOutputChannels > 0))
          )
         ) {
        devices.push_back(PString(PortAudioPrefix) & PString(deviceInfo->name));
      }
    }
  }

  return devices;
}


static const PaDeviceInfo * GetDeviceInfo(const PString & device, PSoundChannel::Directions dir, int & id)
{
  if ((device.GetLength() < PORTAUDIO_PREFIX_LEN) || (device.Left(PORTAUDIO_PREFIX_LEN) != PortAudioPrefix))
    return NULL;

  const char * paName = (const char *)device + PORTAUDIO_PREFIX_LEN;

  PWaitAndSignal m(PSoundChannelPortAudio::GetInitMutex());

  int numDevices = Pa_GetDeviceCount();

  if (numDevices > 0) {
    int i;
    for (i = 0;i < numDevices; i++) {
      const PaDeviceInfo * deviceInfo = Pa_GetDeviceInfo(i);
      if (strcmp(deviceInfo->name, paName) == 0) {
        id = i;
        return deviceInfo;
      }
    }
  }

  return NULL;
}


bool PSoundChannelPortAudio::OpenStream(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
{
  PWaitAndSignal m(m_mutex);

  m_channels       = numChannels;
  m_sampleRate     = sampleRate;
  m_bitsPerSample  = bitsPerSample;
  m_bytesPerSample = (bitsPerSample + 7) / 8;

  if (m_bitsPerSample == 16) {
    m_sampleFormat = paInt16;
  }
  else {
    PTRACE(3, "PortAudio\tUnsupported bits/sample = " << m_bitsPerSample);
    return false;  
  }

  memset(&m_streamParms, 0, sizeof(m_streamParms));
  m_streamParms.device           = m_deviceId;
  m_streamParms.channelCount     = m_channels;
  m_streamParms.sampleFormat     = m_sampleFormat;
  m_streamParms.suggestedLatency = (activeDirection == Recorder) ? m_deviceInfo->defaultLowInputLatency : m_deviceInfo->defaultLowOutputLatency;
  //m_streamParms.suggestedLatency = (activeDirection == Recorder) ? m_deviceInfo->defaultHighInputLatency : m_deviceInfo->defaultHighOutputLatency;
  m_streamParms.hostApiSpecificStreamInfo = NULL; // host API specific

  if (Pa_IsFormatSupported(
                      (activeDirection == Recorder) ? &m_streamParms : NULL,
                      (activeDirection == Player)   ? &m_streamParms : NULL,
                      m_sampleRate) != paFormatIsSupported) {
    PTRACE(1, "PortAudio\tformat not supported: sample/sec=" << m_sampleRate << ",bits/sample=" << m_bitsPerSample << "(" << m_bytesPerSample << "),channels=" << m_channels);
    return false;
  }

  int err = Pa_OpenStream(
                      &m_stream,
                      (activeDirection == Recorder) ? &m_streamParms : NULL,
                      (activeDirection == Player)   ? &m_streamParms : NULL,
                      m_sampleRate,
                      m_playBufferSize / m_bytesPerSample,
                      paClipOff,           /* we won't output out of range samples so don't bother clipping them */
                      NULL,
                      NULL
                     );

  if (err != paNoError) {
    PTRACE(1, "PortAudio\tOpen failed");
    return false;
  }

#ifdef P_PORTMIXER
  m_mixer = Px_OpenMixer(m_stream, 0);
  if (m_mixer == NULL) {
    PTRACE(1, "PortMixer\tOpen failed");
    return false;
  }
#endif

  StartStream();

  return true;
}

bool PSoundChannelPortAudio::Open(const Params & params)
{
  PWaitAndSignal m(m_mutex);

  Close();

  m_deviceName = params.m_device;
  activeDirection = params.m_direction;

  m_deviceInfo = GetDeviceInfo(params.m_device, activeDirection, m_deviceId);
  if (m_deviceInfo == NULL) {
    PTRACE(3, "PortAudio\tCannot get info for " << ((activeDirection == Player) ? "output" : "input") << " device '" << params.m_device << "'");
    return false;
  }

  SetBuffers(params.m_bufferSize, params.m_bufferCount);
  return OpenStream(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
}


bool PSoundChannelPortAudio::Close()
{
  PWaitAndSignal m(m_mutex);

  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

  StopStream();

#ifdef P_PORTMIXER
  Px_CloseMixer(m_mixer);
  m_mixer = NULL;
#endif

  Pa_CloseStream(m_stream);
  m_stream     = NULL;

  m_deviceInfo = NULL;

  if (m_muteBuffer != NULL) {
    free(m_muteBuffer);
    m_muteBuffer = NULL;
  }

  return true;
}

  
bool PSoundChannelPortAudio::IsOpen() const
{ 
  return m_deviceInfo != NULL;
}


bool PSoundChannelPortAudio::StartStream()
{
  PWaitAndSignal m(m_mutex);

  if (!IsOpen())
    return false;

  if (!m_started) {
    int err = Pa_StartStream(m_stream);
    if (err != paNoError) {
      PTRACE(1, "PortAudio\tStart error " << Pa_GetErrorText(err));
      return false;
    }

    PTRACE(1, "PortAudio\tstream started: sample/sec=" << m_sampleRate << ",bits/sample=" << m_bitsPerSample << "(" << m_bytesPerSample << "),channels=" << m_channels << ",bufferSize=" << m_playBufferSize / m_bytesPerSample);
    m_started = true;
  }

  return true;
}


bool PSoundChannelPortAudio::StopStream()
{
  PWaitAndSignal m(m_mutex);

  if (!IsOpen())
    return false;

  if (m_started) {
    PTRACE(1, "PortAudio\tstream stopped");
    Pa_AbortStream(m_stream);
    m_started = false;
  }

  return true;
}


bool PSoundChannelPortAudio::SetFormat(unsigned numChannels,
                                       unsigned sampleRate,
                                       unsigned bitsPerSample)
{
  PWaitAndSignal m(m_mutex);

  StopStream();
  return OpenStream(numChannels, sampleRate, bitsPerSample);
}


unsigned PSoundChannelPortAudio::GetChannels() const
{
  return m_channels;
}


unsigned PSoundChannelPortAudio::GetSampleRate() const
{
  return m_sampleRate;
}


unsigned PSoundChannelPortAudio::GetSampleSize() const
{
  return m_bitsPerSample;
}


bool PSoundChannelPortAudio::SetBuffers(PINDEX size, PINDEX count)
{
  PWaitAndSignal m(m_mutex);

  bool wasRunning = m_started;

  if ((size == m_playBufferSize)&& (count == m_playBufferCount))
    return true;

  PTRACE(1, "PortAudio\tSetting new buffer sizes " << size << " * " << count);

  StopStream();

  m_playBufferSize  = size;
  m_playBufferCount = count;

  if (wasRunning)
    StartStream();

  return true;
}


bool PSoundChannelPortAudio::GetBuffers(PINDEX & size, PINDEX & count)
{
  PWaitAndSignal m(m_mutex);

  size  = m_playBufferSize;
  count = m_playBufferCount;

  return true;
}


bool PSoundChannelPortAudio::Write(const void * data, PINDEX size)
{
  PWaitAndSignal m(m_mutex);

  if (!m_started) {
    PTRACE(1, "PortAudio\tWrite error - not started");
    return false;
  }
    
  lastWriteCount = size;

#ifndef P_PORT_MIXER
  if (m_mute || (m_volume == 0)) {
    if (m_muteBuffer == NULL) {
      m_muteBuffer = (unsigned char *)malloc(size);
      memset(m_muteBuffer, 0, size);
      m_muteBufferSize = size;
    }
    else if (size > m_muteBufferSize) {
      m_muteBuffer = (unsigned char *)realloc(m_muteBuffer, size);
      memset((unsigned char *)m_muteBuffer + m_muteBufferSize, 0, size - m_muteBufferSize);
      m_muteBufferSize = size;
    }
    data = m_muteBuffer;
  }
#endif

  int err = Pa_WriteStream(m_stream, data, size / m_bytesPerSample);
  if (err != paNoError) {
    PTRACE(1, "PortAudio\tWrite error " << Pa_GetErrorText(err));
  }

  return err == paNoError;
}


bool PSoundChannelPortAudio::StartRecording()
{
  PWaitAndSignal m(m_mutex);

  if (activeDirection != Recorder)
    return false;

  if (m_started)
    return true;

  StartStream();

  return true;
}


bool PSoundChannelPortAudio::Read(void * data, PINDEX size)
{
  PWaitAndSignal m(m_mutex);

  if (!m_started) {
    PTRACE(1, "PortAudio\tRead error - not started");
    return false;
  }
    
  lastReadCount = size;

  int err = Pa_ReadStream(m_stream, data, size / m_bytesPerSample);
  if (err != paNoError) {
    PTRACE(1, "PortAudio\tRead error " << Pa_GetErrorText(err));
  }

  if (m_mute || (m_volume == 0))
    memset(data, 0, size);

  return err == paNoError;
}


bool PSoundChannelPortAudio::PlaySound(const PSound & sound, bool wait)
{
  return false;
}


bool PSoundChannelPortAudio::PlayFile(const PFilePath & filename, bool wait)
{
  return false;
}


bool PSoundChannelPortAudio::HasPlayCompleted()
{
  return false;
}


bool PSoundChannelPortAudio::WaitForPlayCompletion()
{
  return false;
}


bool PSoundChannelPortAudio::RecordSound(PSound & sound)
{
  return false;
}


bool PSoundChannelPortAudio::RecordFile(const PFilePath & filename)
{
  return false;
}


bool PSoundChannelPortAudio::IsRecordBufferFull()
{
  return false;
}


bool PSoundChannelPortAudio::AreAllRecordBuffersFull()
{
  return false;
}


bool PSoundChannelPortAudio::WaitForRecordBufferFull()
{
  return false;
}


bool PSoundChannelPortAudio::WaitForAllRecordBuffersFull()
{
  return true;
}


bool PSoundChannelPortAudio::Abort()
{
  return true;
}


PString PSoundChannelPortAudio::GetErrorText(ErrorGroup group) const
{
  char str[256];

/*
  if ((lastErrorNumber[group]&PWIN32ErrorFlag) == 0)
    return PChannel::GetErrorText(group);

  DWORD osError = lastErrorNumber[group]&~PWIN32ErrorFlag;
  if (direction == Recorder) {
    if (waveInGetErrorText(osError, str, sizeof(str)) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(group);
  }
  else {
    if (waveOutGetErrorText(osError, str, sizeof(str)) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(group);
  }
*/

  return str;
}


bool PSoundChannelPortAudio::SetVolume(unsigned newVolume)
{
  PWaitAndSignal m(m_mutex);

  m_volume = newVolume;

  PTRACE(1, "PortAudio\tSetting " << ((activeDirection == Recorder) ? "Recorder" : "Player") << " volume to " << newVolume);

#ifdef P_PORTMIXER
  if (activeDirection == Player) {
    if (m_mixer == NULL)
      return false;
    Px_SetPCMOutputVolume(m_mixer, newVolume / 100.0);
  }
  else
#endif
  {
    if (newVolume != 0) {
      m_mute = FALSE;
      free(m_muteBuffer);
      m_muteBuffer = NULL;
    }
  }

  return true;
}



bool PSoundChannelPortAudio::GetVolume(unsigned & oldVolume)
{
  PWaitAndSignal m(m_mutex);

#ifdef P_PORTMIXER
  if (activeDirection == Player) {
    if (m_mixer == NULL)
      return false;
    oldVolume = Px_GetPCMOutputVolume(m_mixer) * 100;
  }
  else
#endif
  oldVolume = m_volume;

  return true;
}


bool PSoundChannelPortAudio::SetMute(bool newMute)
{
  PWaitAndSignal m(m_mutex);

  m_mute = newMute;

#ifdef P_PORTMIXER
  if (activeDirection == Player) {
    if (m_mute)
      Px_SetPCMOutputVolume(m_mixer, 0.0);
    else
      Px_SetPCMOutputVolume(m_mixer, m_volume / 100.0);
  }
  else
#endif
  {
    if (!m_mute && (m_muteBuffer != NULL)) {
      free(m_muteBuffer);
      m_muteBuffer = NULL;
    }
  }

  PTRACE(1, "PortAudio\t" << ((activeDirection == Recorder) ? "Recorder" : "Player") << " mute is " << (newMute ? "on" : "off"));
  
  return true;
}


bool PSoundChannelPortAudio::GetMute(bool & oldMute)
{
  return m_mute;
}


// End of File ///////////////////////////////////////////////////////////////




