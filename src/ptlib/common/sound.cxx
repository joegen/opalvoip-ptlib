/*
 * sound.cxx
 *
 * Code for pluigns sound device
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Post Increment
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
 * Contributor(s): Craig Southeren
 *                 Snark at GnomeMeeting
 */

#ifdef __GNUC__
#pragma implementation "sound.h"
#endif

#include <ptlib.h>

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>
#include <ptclib/delaychan.h>
#include <ptclib/dtmf.h>
#include <ptclib/pwavfile.h>

#include <math.h>

#if P_DIRECTSOUND
  #include <ptlib/msos/ptlib/directsound.h>
#endif

#ifdef _WIN32
  #include <ptlib/msos/ptlib/sound_win32.h>
#endif


#define PTraceModule() "Sound"


class PSoundChannelNull : public PSoundChannel
{
    PCLASSINFO(PSoundChannelNull, PSoundChannel);
  public:
    PSoundChannelNull();
    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);
    bool Open(const Params & params);
    virtual PString GetName() const;
    PBoolean Close();
    PBoolean IsOpen() const;
    PBoolean Write(const void *, PINDEX len);
    PBoolean Read(void * buf, PINDEX len);
    PBoolean SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample);
    unsigned GetChannels() const;
    unsigned GetSampleRate() const;
    unsigned GetSampleSize() const;
    PBoolean SetBuffers(PINDEX size, PINDEX);
    PBoolean GetBuffers(PINDEX & size, PINDEX & count);

  protected:
    unsigned       m_sampleRate;
    unsigned       m_channels;
    unsigned       m_bytesPerSample;
    unsigned       m_bufferSize;
    unsigned       m_bufferCount;
    PAdaptiveDelay m_Pacing;
};


PCREATE_SOUND_PLUGIN(NullAudio, PSoundChannelNull)


//////////////////////////////////////////////////////////////////////////////
    
PSoundChannel::Params::Params(Directions dir,
                              const PString & device,
                              const PString & driver,
                              unsigned channels,
                              unsigned sampleRate,
                              unsigned bitsPerSample,
                              unsigned bufferSize,
                              unsigned bufferCount,
                              PPluginManager * pluginMgr)
  : m_direction(dir)
  , m_device(device)
  , m_driver(driver)
  , m_channels(channels)
  , m_sampleRate(sampleRate)
  , m_bitsPerSample(bitsPerSample)
  , m_bufferSize(bufferSize)
  , m_bufferCount(bufferCount)
  , m_pluginMgr(pluginMgr)
{
  if (m_driver.IsEmpty())
    device.Split(PPluginServiceDescriptor::SeparatorChar, m_driver, m_device);
}


void PSoundChannel::Params::SetBufferCountFromMS(unsigned milliseconds)
{
  unsigned msPerBuffer = m_bufferSize*1000/m_sampleRate*8/m_bitsPerSample;
  m_bufferCount = (milliseconds+msPerBuffer-1)/msPerBuffer;
}


ostream & operator<<(ostream & strm, const PSoundChannel::Params & params)
{
  if (params.m_driver.IsEmpty())
    return strm << "device=\"" << params.m_device << '"';

  if (params.m_device.IsEmpty())
    return strm << "driver=\"" << params.m_driver << '"';

  return strm << "driver=\"" << params.m_driver << "\", device=\"" << params.m_device << '"';
}


PStringArray PSoundChannel::GetDriverNames(PPluginManager * pluginMgr)
{
  return PPluginManager::GetPluginsProviding(pluginMgr, PPlugin_PSoundChannel::ServiceType(), false);
}


PStringArray PSoundChannel::GetDriversDeviceNames(const PString & driverName,
                                                  PSoundChannel::Directions dir,
                                                  PPluginManager * pluginMgr)
{
  return PPluginManager::GetPluginDeviceNames(pluginMgr, driverName, PPlugin_PSoundChannel::ServiceType(), dir);
}


PSoundChannel * PSoundChannel::CreateChannel(const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return dynamic_cast<PSoundChannel *>(pluginMgr->CreatePlugin(driverName, PPlugin_PSoundChannel::ServiceType(), 0));
}


PSoundChannel * PSoundChannel::CreateChannelByName(const PString & deviceName,
                                                   PSoundChannel::Directions dir,
                                                   PPluginManager * pluginMgr)
{
  return PPluginManager::CreatePluginAs<PSoundChannel>(pluginMgr, deviceName, PPlugin_PSoundChannel::ServiceType(), dir);
}


PSoundChannel * PSoundChannel::CreateOpenedChannel(const PString & driverName,
                                                   const PString & deviceName,
                                                   PSoundChannel::Directions dir,
                                                   unsigned numChannels,
                                                   unsigned sampleRate,
                                                   unsigned bitsPerSample,
                                                   PPluginManager * pluginMgr)
{
  return CreateOpenedChannel(Params(dir,
                                    deviceName,
                                    driverName,
                                    numChannels,
                                    sampleRate,
                                    bitsPerSample,
                                    DefaultBufferSize,
                                    DefaultBufferCount,
                                    pluginMgr));
}


PSoundChannel * PSoundChannel::CreateOpenedChannel(const Params & params)
{
  Params adjustedParams = params;

  if (adjustedParams.m_driver == "*")
    adjustedParams.m_driver.MakeEmpty();

  if (adjustedParams.m_device == "*")
    adjustedParams.m_device.MakeEmpty();

  if (adjustedParams.m_device.IsEmpty()) {
    if (adjustedParams.m_driver.IsEmpty())
      adjustedParams.m_device = PSoundChannel::GetDefaultDevice(params.m_direction);
    else {
      PStringArray devices = PSoundChannel::GetDriversDeviceNames(adjustedParams.m_driver, PSoundChannel::Player);
      if (!devices.IsEmpty())
        adjustedParams.m_device = devices[0];
    }
  }

  PTRACE(5, NULL, "Sound", params.m_direction << " search for " << adjustedParams);

  PSoundChannel * sndChan = NULL;

  if (!adjustedParams.m_driver.IsEmpty())
    sndChan = CreateChannel(adjustedParams.m_driver, params.m_pluginMgr);
  else {
    PINDEX sep = adjustedParams.m_device.Find(PPluginServiceDescriptor::SeparatorChar);
    if (sep != P_MAX_INDEX) {
      adjustedParams.m_driver = adjustedParams.m_device.Left(sep);
      adjustedParams.m_device.Delete(0, sep+1);
      sndChan = CreateChannel(adjustedParams.m_driver, params.m_pluginMgr);
    }
    else {
      if ((sep = adjustedParams.m_device.Find(':')) != P_MAX_INDEX) {
        PString trialDriver = adjustedParams.m_device.Left(sep);
        sndChan = CreateChannel(trialDriver, params.m_pluginMgr);
        if (sndChan != NULL) {
          adjustedParams.m_driver = adjustedParams.m_driver;
          adjustedParams.m_device.Delete(0, sep+1);
        }
      }
      if (sndChan == NULL)
        sndChan = CreateChannelByName(adjustedParams.m_device, params.m_direction, params.m_pluginMgr);
    }
  }

  if (sndChan != NULL) {
    PTRACE(5, sndChan, "Sound", params.m_direction << " opening, " << adjustedParams);
    if (sndChan->Open(adjustedParams))
      return sndChan;
  }

  PTRACE(2, sndChan, "Sound",
         params.m_direction << " could not be opened, " << adjustedParams << ": " <<
         (sndChan != NULL ? sndChan->GetErrorText() : "Unknown driver or device type"));

  delete sndChan;
  return NULL;
}


PStringArray PSoundChannel::GetDeviceNames(PSoundChannel::Directions dir, PPluginManager * pluginMgr)
{
  return PPluginManager::GetPluginDeviceNames(pluginMgr, "*", PPlugin_PSoundChannel::ServiceType(), dir);
}


PString PSoundChannel::GetDefaultDevice(Directions dir)
{
  PCaselessString device;

#if P_DIRECTSOUND
  if (!(device = PSoundChannelDirectSound::GetDefaultDevice(dir)).IsEmpty())
    return PSTRSTRM(PSoundChannelDirectSound::GetDriverName() << PPluginServiceDescriptor::SeparatorChar << device);
#endif

#ifdef _WIN32
  if (!(device = PSoundChannelWin32::GetDefaultDevice(dir)).IsEmpty())
    return PSTRSTRM(PSoundChannelWin32::GetDriverName() << PPluginServiceDescriptor::SeparatorChar << device);
#endif

  PStringArray devices = GetDeviceNames(dir);

  for (PINDEX i = 0; i < devices.GetSize(); ++i) {
    device = devices[i];
    if (device != PPlugin_PSoundChannel_NullAudio::ServiceName() && device != "*.wav" && device.NumCompare("Tones") != EqualTo)
      break;
  }

  return PPlugin_PSoundChannel_NullAudio::ServiceName();
}


PSoundChannel::PSoundChannel()
  : activeDirection(Closed)
{
}


PSoundChannel::PSoundChannel(const Params & params)
  : activeDirection(Closed)
{
  Open(params);
}


PSoundChannel::PSoundChannel(const PString & device,
                             Directions dir,
                             unsigned numChannels,
                             unsigned sampleRate,
                             unsigned bitsPerSample)
  : activeDirection(dir)
{
  Open(Params(dir, device, PString::Empty(), numChannels, sampleRate, bitsPerSample));
}


PBoolean PSoundChannel::Open(const PString & devSpec,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample,
                         PPluginManager * pluginMgr)
{
  return Open(Params(dir, devSpec, PString::Empty(), numChannels, sampleRate, bitsPerSample, DefaultBufferSize, DefaultBufferCount, pluginMgr));
}


bool PSoundChannel::Open(const Params & params)
{
  channelPointerMutex.StartWrite();
  activeDirection = params.m_direction;
  PIndirectChannel::Open(CreateOpenedChannel(params));
  channelPointerMutex.EndWrite();

  return readChannel != NULL;
}


PBoolean PSoundChannel::Abort()
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel == NULL || GetSoundChannel()->Abort();
}


PBoolean PSoundChannel::SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->SetFormat(numChannels, sampleRate, bitsPerSample);
}


unsigned PSoundChannel::GetChannels() const
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel == NULL ? 0 : GetSoundChannel()->GetChannels();
}


unsigned PSoundChannel::GetSampleRate() const
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel == NULL ? 0 : GetSoundChannel()->GetSampleRate();
}


unsigned PSoundChannel::GetSampleSize() const 
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel == NULL ? 0 : GetSoundChannel()->GetSampleSize();
}


PBoolean PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->SetBuffers(size, count);
}


PBoolean PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->GetBuffers(size, count);
}


PBoolean PSoundChannel::SetVolume(unsigned volume)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->SetVolume(volume);
}


PBoolean PSoundChannel::GetMute(bool & mute)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->GetMute(mute);
}


PBoolean PSoundChannel::SetMute(bool mute)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->SetMute(mute);
}


PBoolean PSoundChannel::GetVolume(unsigned & volume)
{
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->GetVolume(volume);
}


PBoolean PSoundChannel::PlaySound(const PSound & sound, PBoolean wait)
{
  PAssert(activeDirection == Player, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  if (readChannel != NULL)
    return GetSoundChannel()->PlaySound(sound, wait);

  Abort();

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return false;

  if (wait)
    return WaitForPlayCompletion();

  return true;
}


bool PSoundChannel::PlayFile(const PFilePath & filename, bool wait, unsigned volume)
{
  PAssert(activeDirection == Player, PLogicError);

#if P_WAVFILE
  /* use PWAVFile instead of PFile -> skips wav header bytes */
  PWAVFile wavFile(filename, PFile::ReadOnly);
  if (!wavFile.IsOpen()) {
    PTRACE(2, "Could not open WAV file \"" << filename << '"');
    return false;
  }

  Abort();

  if (!SetFormat(wavFile.GetChannels(), wavFile.GetSampleRate(), wavFile.GetSampleSize()))
    return false;

  PBYTEArray buffer(std::min(wavFile.GetLength(), (off_t)(1024*1024))); // Max of a megabyte

  if (!SetBuffers(buffer.GetSize()))
    return false;

  PTRACE(4, "Starting play of WAV file \"" << filename << "\", "
            "volume=" << volume << "%, "
            "total size=" << wavFile.GetLength() << ", "
            "buffer size=" << buffer.GetSize() << ", "
            "sample rate=" << wavFile.GetSampleRate() << ", "
            "duration=" << PTimeInterval(wavFile.GetLength()*8000/wavFile.GetSampleSize()/wavFile.GetChannels()/wavFile.GetSampleRate()));

  while (wavFile.Read(buffer.GetPointer(), buffer.GetSize())) {
    if (volume < 100) {
      /* Adjust volume of audio data sent to the device without changing the
         volume of the device itself. Do this because of other ongoing or
         subsequent playing operations while volume of the device should not
         be changed. */
      if (wavFile.GetSampleSize() != 16)
        PTRACE(2, "Volume adjustment of WAV file \"" << filename << "\" with "
               << wavFile.GetSampleSize() << " bits/sample is not supported");
      else {
        /* Convert "volume" to a multiplier an a logarithmic scale. 50%, which
           divides by 2, does not actually give you half the volume. */
        static const int scale = 1000;
        int multiplier = (int)((pow(10, volume/100.0)-1.0)/9.0*scale);
        PINDEX nSample = buffer.GetSize() / 2;
        short * pSample = (short *)buffer.GetPointer();
        for (PINDEX iSample = 0; iSample < nSample; ++iSample, ++pSample)
          *pSample = (short)(*pSample * multiplier / scale);
      }
    }

    if (!Write(buffer, wavFile.GetLastReadCount())) {
      PTRACE(4, "Error writing sound: " << GetErrorText());
      return false;
    }
  }

  PTRACE(4, "Queued WAV file " << filename);

  if (wait)
   return WaitForPlayCompletion();

  return true;
#else
  return false;
#endif
}


PBoolean PSoundChannel::HasPlayCompleted()
{
  PAssert(activeDirection == Player, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->HasPlayCompleted();
}


PBoolean PSoundChannel::WaitForPlayCompletion() 
{
  PAssert(activeDirection == Player, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->WaitForPlayCompletion();
}


PBoolean PSoundChannel::RecordSound(PSound & sound)
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->RecordSound(sound);
}


PBoolean PSoundChannel::RecordFile(const PFilePath & file)
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->RecordFile(file);
}


PBoolean PSoundChannel::StartRecording()
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->StartRecording();
}


PBoolean PSoundChannel::IsRecordBufferFull() 
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->IsRecordBufferFull();
}


PBoolean PSoundChannel::AreAllRecordBuffersFull() 
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->AreAllRecordBuffersFull();
}


PBoolean PSoundChannel::WaitForRecordBufferFull() 
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->WaitForRecordBufferFull();
}


PBoolean PSoundChannel::WaitForAllRecordBuffersFull() 
{
  PAssert(activeDirection == Recorder, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->WaitForAllRecordBuffersFull();
}


const char * PSoundChannel::GetDirectionText(Directions dir)
{
  switch (dir) {
    case Player :
      return "Playback";
    case Recorder :
      return "Recording";
    case Closed :
      return "Closed";
    default :
      return "<Unknown>";
  }
}


static PString SuccessfulTestResult(std::vector<int64_t> & times, PSoundChannel & channel)
{
  // Skip first few chunks as they are not indicative, being very fast filling queues
  PINDEX size, count;
  channel.GetBuffers(size, count);
  size_t base = count*4;
  
  // Calculate times for each read/write
  for (size_t i = times.size()-1; i >= base; --i)
    times[i] -= times[i-1];
  
  // Calculate mean
  double average = 0;
  for (size_t i = base; i < times.size(); ++i)
    average += times[i];
  average /= times.size() - base;
  
  // Calculate standard deviation
  double variance = 0;
  for (size_t i = base; i < times.size(); ++i) {
    double diff = times[i] - average;
    variance += diff * diff;
  }
  variance /= times.size() - base - 1;

  PStringStream text;
  text << "Success: ";

  PString name = channel.GetName();
  PString driver, device;
  if (name.Split(PPluginServiceDescriptor::SeparatorChar, driver, device))
    text << "driver=\"" << driver << "\", device=\"" << device << '"';
  else
    text << '"' << name << '"';

  text << ", " << channel.GetSampleRate() << "Hz, " << count << 'x' << size << " byte buffers\n"
          "Expected=" << std::fixed << std::setprecision(2) << (size/2*1000/channel.GetSampleRate()) << "ms, "
          "average=" << average << "ms, "
          "deviation=" << sqrt(variance) << "ms";
  PTRACE(3, "Sound", text);
  return text;
}


static PString UnsuccessfulTestResult(const char * err, const PSoundChannel::Params & params, PSoundChannel & channel)
{
  PStringStream text;
  text << "Error: " << err << " using " << params;

  if (channel.IsOpen())
    text << ": " << channel.GetErrorText();

  PTRACE(2, &channel, "Sound", text);
  return text;
}


PString PSoundChannel::TestPlayer(const Params & params, const PNotifier & progress, const char * toneSpec)
{
  if (params.m_direction != Player || params.m_channels != 1 || params.m_bitsPerSample != 16)
    return "Error: Invalid parameters";

#if P_DTMF
  PTones tones(toneSpec != NULL ? toneSpec :
               "C:0.2/D:0.2/E:0.2/F:0.2/G:0.2/A:0.2/B:0.2/C5:0.2/"
               "C5:0.2/B:0.2/A:0.2/G:0.2/F:0.2/E:0.2/D:0.2/C:2.0",
               PTones::MaxVolume, params.m_sampleRate);
#else
  PShortArray tones(params.m_sampleRate*5); // 5 seconds
  if (toneSpec != NULL)
    return "Cannot do user defined specification";
#endif

  unsigned samplesPerBuffer = params.m_bufferSize/2;
  unsigned totalBuffers = (tones.GetSize()+samplesPerBuffer-1)/samplesPerBuffer;

  tones.SetSize(samplesPerBuffer*totalBuffers); // Pad out with silence so exact match of writes
  
  PTRACE(3, &tones, "Sound", "Tones using " << tones.GetSize() << " samples, "
         << PTimeInterval(1000 * tones.GetSize() / params.m_sampleRate) << " seconds, "
         << totalBuffers << 'x' << samplesPerBuffer << " sample buffers");

  std::vector<int64_t> times(totalBuffers+1);

  PSoundChannel player;
  if (!progress.IsNULL())
    progress(player, totalBuffers);

  if (!player.Open(params))
    return UnsuccessfulTestResult("Could not open", params, player);

#if PTRACING
  PTime then;
#endif

  for (unsigned i = 0; i < totalBuffers; ++i) {
    if (!progress.IsNULL())
      progress(player, i);

    times[i] = PTimer::Tick().GetMilliSeconds();

    if (!player.Write(tones.GetPointer()+i*samplesPerBuffer, params.m_bufferSize))
      return UnsuccessfulTestResult("Could not write to", params, player);
  }

  times[totalBuffers] = PTimer::Tick().GetMilliSeconds();

  PTRACE(3, &tones, "Sound", "Audio queued, waiting for completion");
  player.WaitForPlayCompletion();
  PTRACE(3, &tones, "Sound", "Finished tone output: " << PTime() - then << " seconds");

  return SuccessfulTestResult(times, player);
}


PString PSoundChannel::TestRecorder(const Params & recorderParams,
                                    const Params & playerParams,
                                    const PNotifier & progress,
                                    unsigned seconds)
{
  if (recorderParams.m_channels      != playerParams.m_channels ||
      recorderParams.m_sampleRate    != playerParams.m_sampleRate ||
      recorderParams.m_bitsPerSample != playerParams.m_bitsPerSample ||
      recorderParams.m_bufferSize    != playerParams.m_bufferSize ||
      recorderParams.m_direction != Recorder || playerParams.m_direction != Player)
    return "Error: Invalid parameters";

  PBYTEArray recording(seconds*recorderParams.m_sampleRate*recorderParams.m_channels*recorderParams.m_bitsPerSample/8);

  unsigned totalBuffers = (recording.GetSize()+recorderParams.m_bufferSize-1)/recorderParams.m_bufferSize;

  std::vector<int64_t> times(totalBuffers+1);

  PSoundChannel recorder;
  if (!progress.IsNULL())
    progress(recorder, totalBuffers);

  if (!recorder.Open(recorderParams))
    return UnsuccessfulTestResult("Could not open", recorderParams, recorder);
  
#if PTRACING
  PTime then;
#endif

  PTRACE(1, &recorder, "Sound", "Started recording");
  for (unsigned i = 0; i < totalBuffers; ++i) {
    if (!progress.IsNULL())
      progress(recorder, i);

    times[i] = PTimer::Tick().GetMilliSeconds();

    if (!recorder.ReadBlock(recording.GetPointer()+i*recorderParams.m_bufferSize, recorderParams.m_bufferSize))
      return UnsuccessfulTestResult("Could not read from", recorderParams, recorder);
  }
  times[totalBuffers] = PTimer::Tick().GetMilliSeconds();
  PTRACE(1, &recorder, "Sound", "Finished recording " << PTime() - then << " seconds");

  PSoundChannel player;
  if (!progress.IsNULL())
    progress(player, totalBuffers);

  if (!player.Open(playerParams))
    return UnsuccessfulTestResult("Could not open", playerParams, player);

  PTRACE(1, &recorder, "Sound", "Started play back");
#if PTRACING
  then.SetCurrentTime();
#endif
  for (unsigned i = 0; i < totalBuffers; ++i) {
      if (!progress.IsNULL())
        progress(player, i);

    if (!player.Write(recording.GetPointer()+i*recorderParams.m_bufferSize, recorderParams.m_bufferSize))
      return UnsuccessfulTestResult("Could not write to", playerParams, player);
  }
  player.WaitForPlayCompletion();
  PTRACE(1, &recorder, "Sound", "Finished play back " << PTime() - then << " seconds");

  return SuccessfulTestResult(times, recorder);
}


///////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32) && !defined(__BEOS__) && !defined(__APPLE__)

PSound::PSound(unsigned channels,
               unsigned samplesPerSecond,
               unsigned bitsPerSample,
               PINDEX   bufferSize,
               const BYTE * buffer)
{
  encoding = 0;
  numChannels = channels;
  sampleRate = samplesPerSecond;
  sampleSize = bitsPerSample;
  SetSize(bufferSize);
  if (buffer != NULL)
    memcpy(GetPointer(), buffer, bufferSize);
}


PSound::PSound(const PFilePath & filename)
{
  encoding = 0;
  numChannels = 1;
  sampleRate = 8000;
  sampleSize = 16;
  Load(filename);
}


PSound & PSound::operator=(const PBYTEArray & data)
{
  PBYTEArray::operator=(data);
  return *this;
}


void PSound::SetFormat(unsigned channels,
                       unsigned samplesPerSecond,
                       unsigned bitsPerSample)
{
  encoding = 0;
  numChannels = channels;
  sampleRate = samplesPerSecond;
  sampleSize = bitsPerSample;
  formatInfo.SetSize(0);
}


PBoolean PSound::Load(const PFilePath & /*filename*/)
{
  return false;
}


PBoolean PSound::Save(const PFilePath & /*filename*/)
{
  return false;
}


PBoolean PSound::Play()
{
  return Play(PSoundChannel::GetDefaultDevice(PSoundChannel::Player));
}


PBoolean PSound::Play(const PString & device)
{

  PSoundChannel channel(device, PSoundChannel::Player);
  if (!channel.IsOpen())
    return false;

  return channel.PlaySound(*this, true);
}

#endif //_WIN32


PBoolean PSound::PlayFile(const PFilePath & file, bool wait, unsigned volume)
{
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player), PSoundChannel::Player);
  return channel.IsOpen() && channel.PlayFile(file, wait, volume);
}


///////////////////////////////////////////////////////////////////////////

PSoundChannelNull::PSoundChannelNull()
  : m_sampleRate(0)
  , m_channels(1)
  , m_bytesPerSample(2)
  , m_bufferSize(320)
{
}

PStringArray PSoundChannelNull::GetDeviceNames(PSoundChannel::Directions)
{
  return PPlugin_PSoundChannel_NullAudio::ServiceName();
}

bool PSoundChannelNull::Open(const Params & params)
{
  activeDirection = params.m_direction;
  return SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
}

PString PSoundChannelNull::GetName() const
{
  return PPlugin_PSoundChannel_NullAudio::ServiceName();
}

PBoolean PSoundChannelNull::Close()
{
  m_sampleRate = 0;
  return true;
}

PBoolean PSoundChannelNull::IsOpen() const
{
  return m_sampleRate > 0;
}

PBoolean PSoundChannelNull::Write(const void *, PINDEX len)
{
  if (m_sampleRate <= 0)
    return false;

  SetLastWriteCount(len);
  m_Pacing.Delay(1000*len/m_sampleRate/m_channels/m_bytesPerSample);
  return true;
}

PBoolean PSoundChannelNull::Read(void * buf, PINDEX len)
{
  if (m_sampleRate <= 0)
    return false;

  memset(buf, 0, len);
  SetLastReadCount(len);
  m_Pacing.Delay(1000*len/m_sampleRate/m_channels/m_bytesPerSample);
  return true;
}

PBoolean PSoundChannelNull::SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
{
  if (bitsPerSample % 8 != 0) {
    PTRACE(1, "NullAudio\tBits per sample must even bytes.");
    return false;
  }

  m_sampleRate = sampleRate;
  m_channels = numChannels;
  m_bytesPerSample = bitsPerSample / 8;
  return true;
}

unsigned PSoundChannelNull::GetChannels() const
{
  return m_channels;
}

unsigned PSoundChannelNull::GetSampleRate() const
{
  return m_sampleRate;
}

unsigned PSoundChannelNull::GetSampleSize() const
{
  return m_bytesPerSample*2;
}

PBoolean PSoundChannelNull::SetBuffers(PINDEX size, PINDEX)
{
  m_bufferSize = size;
  return true;
}

PBoolean PSoundChannelNull::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = m_bufferSize;
  count = 1;
  return true;
}


///////////////////////////////////////////////////////////////////////////

static void MergeSampleValues(const short * & srcSample,
                              PINDEX & srcCount,
                              unsigned srcChannels,
                              short * & dstSample,
                              PINDEX & dstCount,
                              unsigned dstChannels)
{
  if (dstChannels == srcChannels) {
    PINDEX bytes = dstChannels*sizeof(short);
    memcpy(dstSample, srcSample, bytes);
    srcSample += srcChannels;
    srcCount += srcChannels*sizeof(short);
    dstSample += dstChannels;
    dstCount += bytes;
  }

  else if (srcChannels == 1) {
    for (unsigned c = 0; c < dstChannels; ++c)
      *dstSample++ = *srcSample;
    srcSample++;
    srcCount += sizeof(short);
    dstCount += dstChannels*sizeof(short);
  }

  else if (dstChannels == 1) {
    int sum = 0;
    for (unsigned c = 0; c < srcChannels; ++c)
      sum += *srcSample++;
    srcCount += srcChannels*sizeof(short);
    *dstSample++ = (short)(sum/dstChannels);
    dstCount += sizeof(short);
  }

  else {
    // Complicated ones, we cheat mightily and make it all mono
    int sum = 0;
    for (unsigned c = 0; c < dstChannels; ++c)
      sum += *srcSample++;
    for (unsigned c = 0; c < srcChannels; ++c)
      *dstSample++ = (short)(sum/dstChannels);
    srcCount += srcChannels*sizeof(short);
    dstCount += srcChannels*sizeof(short);
  }
}


bool PSound::ConvertPCM(const short * srcPtr,
                        PINDEX & srcSize,
                        unsigned srcRate,
                        unsigned srcChannels,
                        short * dstPtr,
                        PINDEX & dstSize,
                        unsigned dstRate,
                        unsigned dstChannels)
{
  if (srcRate == dstRate && srcChannels == dstChannels) {
    dstSize = std::min(srcSize, dstSize);
    if (srcPtr != dstPtr)
      memcpy(dstPtr, srcPtr, dstSize);
    return srcSize <= dstSize;
  }

  PINDEX srcCount = 0;
  PINDEX dstCount = 0;

  /* Reduce size by one "sample", makes sure we don't overflow on last copy
     in case the input/output buffers are not exact multiples of the sample */
  srcSize -= srcChannels*sizeof(short);
  dstSize -= dstChannels*sizeof(short);

  // Variable Duty Cycle algorithm
  if (srcRate < dstRate) {
    // Have less samples than we want, so we need to interpolate
    unsigned iDutyCycle = dstRate - srcRate;
    while (srcCount <= srcSize && dstCount <= dstSize) {
      iDutyCycle += srcRate;
      if (iDutyCycle >= dstRate) {
        iDutyCycle -= dstRate;
        MergeSampleValues(srcPtr, srcCount, srcChannels, dstPtr, dstCount, dstChannels);
      }
      else {
        const short * tempPtr = srcPtr;
        PINDEX tempCount = srcCount;
        MergeSampleValues(tempPtr, tempCount, srcChannels, dstPtr, dstCount, dstChannels);
      }
    }
  }
  else if (srcRate > dstRate) {
    // File has more samples than we want, so we need to throw some away
    size_t sumsSize = srcChannels*sizeof(int);
    int * sums = (int *)alloca(sumsSize);

    unsigned iDutyCycle = 0;
    while (srcCount <= srcSize && dstCount <= dstSize) {
      memset(sums, 0, sumsSize);
      unsigned count = 0;
      do {
        for (unsigned c = 0; c < srcChannels; ++c)
          sums[c] += *srcPtr++;
        ++count;
        iDutyCycle += dstRate;
      } while (iDutyCycle < srcRate);
      iDutyCycle -= srcRate;

      const short * avg = (const short *)alloca(srcChannels*sizeof(short));
      for (unsigned c = 0; c < srcChannels; ++c)
        const_cast<short *>(avg)[c] = (short)(sums[c]/count);
      PINDEX tempCount = 0;
      MergeSampleValues(avg, tempCount, srcChannels, dstPtr, dstCount, dstChannels);
      srcCount += count*tempCount;
    }
  }
  else {
    while (srcCount <= srcSize && dstCount <= dstSize)
      MergeSampleValues(srcPtr, srcCount, srcChannels, dstPtr, dstCount, dstChannels);
  }

  srcSize = srcCount;
  dstSize = dstCount;
  return srcCount > srcSize && dstCount <= dstSize;
}

    
///////////////////////////////////////////////////////////////////////////
