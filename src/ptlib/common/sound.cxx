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
 * $Id$
 */

#ifdef __GNUC__
#pragma implementation "sound.h"
#endif

#include <ptlib.h>

#include <ptlib/sound.h>
#include <ptlib/pluginmgr.h>

static const char soundPluginBaseClass[] = "PSoundChannel";

PINSTANTIATE_FACTORY(PSoundChannel, Win32)

template <> PSoundChannel * PDevicePluginFactory<PSoundChannel>::Worker::Create(const PString & type) const
{
  return PSoundChannel::CreateChannel(type);
}

namespace PWLib {
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PSoundChannel> > soundChannelFactoryAdapter("PSoundChannel", TRUE);
};


PStringList PSoundChannel::GetDriverNames(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return pluginMgr->GetPluginsProviding(soundPluginBaseClass);
}


PStringList PSoundChannel::GetDriversDeviceNames(const PString & driverName,
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
  PString adjustedDeviceName = deviceName;
  PSoundChannel * sndChan;
  if (driverName.IsEmpty() || driverName == "*") {
    if (deviceName.IsEmpty() || deviceName == "*")
      adjustedDeviceName = PSoundChannel::GetDefaultDevice(dir);
    sndChan = CreateChannelByName(adjustedDeviceName, dir, pluginMgr);
  }
  else {
    if (deviceName.IsEmpty() || deviceName == "*") {
      PStringList devices = PSoundChannel::GetDriversDeviceNames(driverName, PSoundChannel::Player);
      if (devices.IsEmpty())
        return NULL;
      adjustedDeviceName = devices[0];
    }
    sndChan = CreateChannel(driverName, pluginMgr);
  }

  if (sndChan != NULL && sndChan->Open(adjustedDeviceName, dir, numChannels, sampleRate, bitsPerSample))
    return sndChan;

  delete sndChan;
  return NULL;
}


PStringList PSoundChannel::GetDeviceNames(PSoundChannel::Directions dir, PPluginManager * pluginMgr)
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
    if (!registry.QueryValue("ConsoleVoiceComPlayback", str) && !registry.QueryValue("Playback", str)) {
      WAVEOUTCAPS caps;
      if (waveOutGetDevCaps(0, &caps, sizeof(caps)) == 0)
        str = caps.szPname;
    }
  }
  else {
    if (!registry.QueryValue("ConsoleVoiceComRecord", str) && !registry.QueryValue("Record", str)) {
      WAVEINCAPS caps;
      if (waveInGetDevCaps(0, &caps, sizeof(caps)) == 0)
        str = caps.szPname;
    }
  }

  return str;
#else
  PStringList devices = GetDeviceNames(dir);
  if (devices.GetSize() > 0)
    return devices[0];

  return PString::Empty();
#endif
}


PSoundChannel::PSoundChannel()
{
  baseChannel = NULL;
}

PSoundChannel::~PSoundChannel()
{
  delete baseChannel;
}

PSoundChannel::PSoundChannel(const PString & device,
                             Directions dir,
                             unsigned numChannels,
                             unsigned sampleRate,
                             unsigned bitsPerSample)
{
  baseChannel = NULL;
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}

BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  if (baseChannel != NULL)
    return baseChannel->Open(device, dir, numChannels, sampleRate, bitsPerSample);

  baseChannel = CreateOpenedChannel(PString::Empty(), device, dir, numChannels, sampleRate, bitsPerSample);
  return baseChannel != NULL;
}

PString PSoundChannel::GetName() const
{
  if (baseChannel == NULL)
    return PString::Empty();

  return baseChannel->GetName();
}


BOOL PSoundChannel::IsOpen() const
{
    return baseChannel == NULL || baseChannel->PChannel::IsOpen();
}


BOOL PSoundChannel::Close()
{
    return baseChannel == NULL || baseChannel->Close();
}


int PSoundChannel::GetHandle() const
{
    return baseChannel == NULL ? -1 : baseChannel->PChannel::GetHandle();
}


BOOL PSoundChannel::Abort()
{
    return baseChannel == NULL || baseChannel->Abort();
}


BOOL PSoundChannel::SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
{
    return baseChannel != NULL && baseChannel->SetFormat(numChannels, sampleRate, bitsPerSample);
}


unsigned PSoundChannel::GetChannels() const
{
    return baseChannel == NULL ? 0 : baseChannel->GetChannels();
}


unsigned PSoundChannel::GetSampleRate() const
{
    return baseChannel == NULL ? 0 : baseChannel->GetSampleRate();
}


unsigned PSoundChannel::GetSampleSize() const 
{
    return baseChannel == NULL ? 0 : baseChannel->GetSampleSize();
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
    return baseChannel != NULL && baseChannel->SetBuffers(size, count);
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
    return baseChannel != NULL && baseChannel->GetBuffers(size, count);
}


BOOL PSoundChannel::SetVolume(unsigned volume)
{
    return baseChannel != NULL && baseChannel->SetVolume(volume);
}


BOOL PSoundChannel::GetVolume(unsigned & volume)
{
    return baseChannel != NULL && baseChannel->GetVolume(volume);
}


BOOL PSoundChannel::Write(const void * buf, PINDEX len)
{
    return baseChannel != NULL && baseChannel->Write(buf, len);
}


PINDEX PSoundChannel::GetLastWriteCount() const
{
    return baseChannel == NULL ? lastWriteCount : baseChannel->GetLastWriteCount();
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
    return baseChannel != NULL && baseChannel->PlaySound(sound, wait);
}


BOOL PSoundChannel::PlayFile(const PFilePath & file, BOOL wait)
{
    return baseChannel != NULL && baseChannel->PlayFile(file, wait);
}


BOOL PSoundChannel::HasPlayCompleted()
{
    return baseChannel == NULL || baseChannel->HasPlayCompleted();
}


BOOL PSoundChannel::WaitForPlayCompletion() 
{
    return baseChannel == NULL || baseChannel->WaitForPlayCompletion();
}


BOOL PSoundChannel::Read(void * buf, PINDEX len)
{
    return baseChannel != NULL && baseChannel->Read(buf, len);
}


PINDEX PSoundChannel::GetLastReadCount() const
{
    return baseChannel == NULL ? lastReadCount : baseChannel->GetLastReadCount();
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
    return baseChannel != NULL && baseChannel->RecordSound(sound);
}


BOOL PSoundChannel::RecordFile(const PFilePath & file)
{
    return baseChannel != NULL && baseChannel->RecordFile(file);
}


BOOL PSoundChannel::StartRecording()
{
    return baseChannel == NULL || baseChannel->StartRecording();
}


BOOL PSoundChannel::IsRecordBufferFull() 
{
    return baseChannel == NULL || baseChannel->IsRecordBufferFull();
}


BOOL PSoundChannel::AreAllRecordBuffersFull() 
{
    return baseChannel == NULL || baseChannel->AreAllRecordBuffersFull();
}


BOOL PSoundChannel::WaitForRecordBufferFull() 
{
    return baseChannel == NULL || baseChannel->WaitForRecordBufferFull();
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull() 
{
    return baseChannel == NULL || baseChannel->WaitForAllRecordBuffersFull();
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


BOOL PSound::Load(const PFilePath & /*filename*/)
{
  return FALSE;
}


BOOL PSound::Save(const PFilePath & /*filename*/)
{
  return FALSE;
}


BOOL PSound::Play()
{
  return Play(PSoundChannel::GetDefaultDevice(PSoundChannel::Player));
}


BOOL PSound::Play(const PString & device)
{

  PSoundChannel channel(device, PSoundChannel::Player);
  if (!channel.IsOpen())
    return FALSE;

  return channel.PlaySound(*this, TRUE);
}


BOOL PSound::PlayFile(const PFilePath & file, BOOL wait)
{
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
  if (!channel.IsOpen())
    return FALSE;

  return channel.PlayFile(file, wait);
}


#endif //_WIN32
