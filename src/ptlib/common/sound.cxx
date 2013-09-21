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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "sound.h"
#endif

#include <ptlib.h>

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>
#include <ptclib/delaychan.h>
#include <ptclib/dtmf.h>

#include <math.h>



static const char soundPluginBaseClass[] = "PSoundChannel";
static const PConstString NullAudio(P_NULL_AUDIO_DEVICE);


void PSoundChannel::Params::SetBufferCountFromMS(unsigned milliseconds)
{
  unsigned msPerBuffer = m_bufferSize*1000/m_sampleRate*8/m_bitsPerSample;
  m_bufferCount = (milliseconds+msPerBuffer-1)/msPerBuffer;
}


template <> PSoundChannel * PDevicePluginFactory<PSoundChannel>::Worker::Create(const PDefaultPFactoryKey & type) const
{
  return PSoundChannel::CreateChannel(type);
}

typedef PDevicePluginAdapter<PSoundChannel> PDevicePluginSoundChannel;
PFACTORY_CREATE(PFactory<PDevicePluginAdapterBase>, PDevicePluginSoundChannel, "PSoundChannel", true);


PStringArray PSoundChannel::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(soundPluginBaseClass);
}


PStringArray PSoundChannel::GetDriversDeviceNames(const PString & driverName,
                                                  PSoundChannel::Directions dir,
                                                  PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceNames(driverName, soundPluginBaseClass, dir);
}


PSoundChannel * PSoundChannel::CreateChannel(const PString & driverName, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PSoundChannel *)pluginMgr->CreatePluginsDevice(driverName, soundPluginBaseClass, 0);
}


PSoundChannel * PSoundChannel::CreateChannelByName(const PString & deviceName,
                                                   PSoundChannel::Directions dir,
                                                   PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PSoundChannel *)pluginMgr->CreatePluginsDeviceByName(deviceName, soundPluginBaseClass, dir);
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

  PTRACE(5, NULL, "Sound",
         params.m_direction << " finding"
         " driver=\"" << adjustedParams.m_driver << "\", "
         " device=\"" << adjustedParams.m_device << '"');

  PSoundChannel * sndChan = NULL;

  if (!adjustedParams.m_driver.IsEmpty())
    sndChan = CreateChannel(adjustedParams.m_driver, params.m_pluginMgr);
  else {
    PINDEX sep = adjustedParams.m_device.Find(PDevicePluginServiceDescriptor::SeparatorChar);
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

  if (sndChan != NULL && sndChan->Open(adjustedParams))
    return sndChan;

  PTRACE_IF(5, sndChan != NULL, sndChan, "Sound",
         params.m_direction << " opening, "
         " driver=\"" << adjustedParams.m_driver << "\", "
         " device=\"" << adjustedParams.m_device << '"');

  PTRACE(2, sndChan, "Sound",
         params.m_direction << " could not be opened,"
         " driver=\"" << adjustedParams.m_driver << "\", "
         " device=\"" << adjustedParams.m_device << "\": " <<
         (sndChan != NULL ? sndChan->GetErrorText() : "Unknown driver or device type"));

  delete sndChan;
  return NULL;
}


PStringArray PSoundChannel::GetDeviceNames(PSoundChannel::Directions dir, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsDeviceNames("*", soundPluginBaseClass, dir);
}


PString PSoundChannel::GetDefaultDevice(Directions dir)
{
#ifdef _WIN32
  RegistryKey registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Multimedia\\Sound Mapper",
                       RegistryKey::ReadOnly);

  PString str;

  if (dir == Player) {
    if (registry.QueryValue("ConsoleVoiceComPlayback", str) )
      return str;
    if (registry.QueryValue("Playback", str))
      return str;
  }
  else {
    if (registry.QueryValue("ConsoleVoiceComRecord", str))
      return str;
    if (registry.QueryValue("Record", str))
      return str;
  }
#endif

  PStringArray devices = GetDeviceNames(dir);

  for (PINDEX i = 0; i < devices.GetSize(); ++i) {
    PCaselessString device = devices[i];
    if (device != NullAudio && device != "*.wav" && device.NumCompare("Tones") != EqualTo)
      return devices[i];
  }

  return NullAudio;
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
  return readChannel != NULL && GetSoundChannel()->PlaySound(sound, wait);
}


PBoolean PSoundChannel::PlayFile(const PFilePath & file, PBoolean wait)
{
  PAssert(activeDirection == Player, PLogicError);
  PReadWaitAndSignal mutex(channelPointerMutex);
  return readChannel != NULL && GetSoundChannel()->PlayFile(file, wait);
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

  PString driver, device;
  if (channel.GetName().Split(PDevicePluginServiceDescriptor::SeparatorChar, driver, device))
    text << " driver=\"" << driver << "\", device=\"" << device << '"';
  else
    text << '"' << channel.GetName() << '"';

  text << '\n'
       << std::fixed << std::setprecision(2)
       << "expected=" << (size/2*1000/channel.GetSampleRate()) << "ms, "
          "average=" << average << "ms, "
          "deviation=" << sqrt(variance) << "ms";
  PTRACE(3, "Sound", text);
  return text;
}


PString PSoundChannel::TestPlayer(const Params & params, const PNotifier & progress, const char * toneSpec)
{
  if (params.m_direction != Player || params.m_channels != 1 || params.m_bitsPerSample != 16)
    return "Error: Invalid parameters";

  PTones tones(toneSpec != NULL ? toneSpec :
               "C:0.2/D:0.2/E:0.2/F:0.2/G:0.2/A:0.2/B:0.2/C5:0.2/"
               "C5:0.2/B:0.2/A:0.2/G:0.2/F:0.2/E:0.2/D:0.2/C:2.0",
               PTones::MaxVolume, params.m_sampleRate);

  unsigned samplesPerBuffer = params.m_bufferSize/2;
  unsigned totalBuffers = (tones.GetSize()+samplesPerBuffer-1)/samplesPerBuffer;

  tones.SetSize(samplesPerBuffer*totalBuffers); // Pad out with silence so exact match of writes
  
  PTRACE(3, &tones, "Sound", "Tones using " << tones.GetSize() << " samples, "
          << PTimeInterval(1000*tones.GetSize()/tones.GetSampleRate()) << " seconds, "
          << totalBuffers << 'x' << samplesPerBuffer << " sample buffers");

  std::vector<int64_t> times(totalBuffers+1);

  PSoundChannel player;
  if (!progress.IsNULL())
    progress(player, totalBuffers);

  if (!player.Open(params)) {
    PTRACE(2, &player, "Sound", "Error opening channel: " << player.GetErrorText());
    return "Error: Could not use sound player device: " + player.GetErrorText();
  }

#if PTRACING
  PTime then;
#endif

  for (unsigned i = 0; i < totalBuffers; ++i) {
    if (!progress.IsNULL())
      progress(player, i);

    times[i] = PTimer::Tick().GetMilliSeconds();

    if (!player.Write(tones.GetPointer()+i*samplesPerBuffer, params.m_bufferSize))
      return "Error: Could not write to sound player device: " + player.GetErrorText(PChannel::LastReadError);
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

  if (!recorder.Open(recorderParams)) {
    PTRACE(2, &recorder, "Sound", "Error opening channel: " << recorder.GetErrorText());
    return "Error: Could not use sound recorder device: " + recorder.GetErrorText();
  }
  
#if PTRACING
  PTime then;
#endif

  PTRACE(1, &recorder, "Sound", "Started recording");
  for (unsigned i = 0; i < totalBuffers; ++i) {
    if (!progress.IsNULL())
      progress(recorder, i);

    times[i] = PTimer::Tick().GetMilliSeconds();

    if (!recorder.ReadBlock(recording.GetPointer()+i*recorderParams.m_bufferSize, recorderParams.m_bufferSize))
      return "Error: Could not read from sound player device: " + recorder.GetErrorText(PChannel::LastWriteError);
  }
  times[totalBuffers] = PTimer::Tick().GetMilliSeconds();
  PTRACE(1, &recorder, "Sound", "Finished recording " << PTime() - then << " seconds");

  PSoundChannel player;
  if (!progress.IsNULL())
    progress(player, totalBuffers);

  if (!player.Open(playerParams))
    return "Error: Could not use sound player device: " + player.GetErrorText();

  PTRACE(1, &recorder, "Sound", "Started play back");
#if PTRACING
  then.SetCurrentTime();
#endif
  for (unsigned i = 0; i < totalBuffers; ++i) {
      if (!progress.IsNULL())
        progress(player, i);

    if (!player.Write(recording.GetPointer()+i*recorderParams.m_bufferSize, recorderParams.m_bufferSize))
      return "Error: Could not write to sound player device: " + player.GetErrorText(PChannel::LastReadError);
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


PBoolean PSound::PlayFile(const PFilePath & file, PBoolean wait)
{
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
  if (!channel.IsOpen())
    return false;

  return channel.PlayFile(file, wait);
}


#endif //_WIN32

///////////////////////////////////////////////////////////////////////////

class PSoundChannelNull : public PSoundChannel
{
 PCLASSINFO(PSoundChannelNull, PSoundChannel);
 public:
    PSoundChannelNull()
      : m_sampleRate(0)
    {
    }

    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player)
    {
      return NullAudio;
    }

    bool Open(const Params & params)
    {
      activeDirection = params.m_direction;
      return SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
    }

    virtual PString GetName() const
    {
      return NullAudio;
    }

    PBoolean Close()
    {
      m_sampleRate = 0;
      return true;
    }

    PBoolean IsOpen() const
    {
      return m_sampleRate > 0;
    }

    PBoolean Write(const void *, PINDEX len)
    {
      if (m_sampleRate <= 0)
        return false;

      lastWriteCount = len;
      m_Pacing.Delay(len/2*1000/m_sampleRate);
      return true;
    }

    PBoolean Read(void * buf, PINDEX len)
    {
      if (m_sampleRate <= 0)
        return false;

      memset(buf, 0, len);
      lastReadCount = len;
      m_Pacing.Delay(len/2*1000/m_sampleRate);
      return true;
    }

    PBoolean SetFormat(unsigned numChannels,
                   unsigned sampleRate,
                   unsigned bitsPerSample)
    {
      m_sampleRate = sampleRate;
      return numChannels == 1 && bitsPerSample == 16;
    }

    unsigned GetChannels() const
    {
      return 1;
    }

    unsigned GetSampleRate() const
    {
      return m_sampleRate;
    }

    unsigned GetSampleSize() const
    {
      return 16;
    }

    PBoolean SetBuffers(PINDEX, PINDEX)
    {
      return true;
    }

    PBoolean GetBuffers(PINDEX & size, PINDEX & count)
    {
      size = 2;
      count = 1;
      return true;
    }

protected:
    unsigned       m_sampleRate;
    PAdaptiveDelay m_Pacing;
};


PCREATE_SOUND_PLUGIN(NullAudio, PSoundChannelNull)
