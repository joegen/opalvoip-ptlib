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
 * $Log: sound.cxx,v $
 * Revision 1.8  2004/08/16 06:40:59  csoutheren
 * Added adapters template to make device plugins available via the abstract factory interface
 *
 * Revision 1.7  2004/04/03 23:53:10  csoutheren
 * Added various changes to improce compatibility with the Sun Forte compiler
 *   Thanks to Brian Cameron
 * Added detection of readdir_r version
 *
 * Revision 1.6  2004/04/02 04:07:54  ykiryanov
 * Added ifndef BeOS for PSound
 *
 * Revision 1.5  2003/11/12 08:55:58  csoutheren
 * Added newline at end of file to remove gcc warning
 *
 * Revision 1.4  2003/11/12 05:17:25  csoutheren
 * Added more backwards compatibility functions for PSoundChannel
 *
 * Revision 1.3  2003/11/12 04:42:02  csoutheren
 * Removed non-specific code when compiling for WIn32
 *
 * Revision 1.2  2003/11/12 03:27:25  csoutheren
 * Initial version of plugin code from Snark of GnomeMeeting with changes
 *    by Craig Southeren of Post Increment
 *
 * Revision 1.1.2.1  2003/10/07 01:33:19  csoutheren
 * Initial checkin of pwlib code to do plugins.
 * Modified from original code and concept provided by Snark of Gnomemeeting
 *
 */

#ifdef __GNUC__
#pragma implementation "sound.h"
#endif

#include <ptlib.h>

#include <ptlib/sound.h>
#include <ptlib/pluginmgr.h>

static const char soundPluginBaseClass[] = "PSoundChannel";

PSoundChannel * PDevicePluginFactory<PSoundChannel>::Worker::Create(const PString & type) const
{
  return PSoundChannel::CreateChannel(type);
}

namespace PWLib {
  PFactory<PDevicePluginAdapterBase>::Worker< PDevicePluginAdapter<PSoundChannel> > soundChannelFactoryAdapter("PSoundChannel", TRUE);
};

namespace PWLibStupidWindowsHacks {
  int loadSoundStuff;
};

PStringList PSoundChannel::GetDriverNames(PPluginManager * _pluginMgr)
{
  PPluginManager * pluginMgr = (_pluginMgr != NULL) ? _pluginMgr : &PPluginManager::GetPluginManager();
  return pluginMgr->GetPluginsProviding(soundPluginBaseClass);
}

PStringList PSoundChannel::GetDeviceNames(const PString & driverName,
                          const PSoundChannel::Directions dir,
                                         PPluginManager * _pluginMgr)
{
  PPluginManager * pluginMgr = (_pluginMgr != NULL) ? _pluginMgr : &PPluginManager::GetPluginManager();
  PSoundChannelPluginServiceDescriptor * descr =
    (PSoundChannelPluginServiceDescriptor *)pluginMgr->GetServiceDescriptor(driverName, soundPluginBaseClass);

  if (descr != NULL)
    return PStringList(descr->GetDeviceNames(dir));
  else
    return PStringList();
}

PSoundChannel * PSoundChannel::CreateChannel(const PString & driverName, PPluginManager * _pluginMgr)
{
  PPluginManager * pluginMgr = (_pluginMgr != NULL) ? _pluginMgr : &PPluginManager::GetPluginManager();
  PSoundChannelPluginServiceDescriptor * descr =
    (PSoundChannelPluginServiceDescriptor *)pluginMgr->GetServiceDescriptor(driverName, soundPluginBaseClass);

  if (descr != NULL)
    return (PSoundChannel *)descr->CreateInstance ();
  else
    return NULL;
}

PSoundChannel * PSoundChannel::CreateOpenedChannel(
      const PString & driverName,
      const PString & deviceName,
      const PSoundChannel::Directions dir,
      unsigned numChannels,
      unsigned sampleRate,
      unsigned bitsPerSample)
{
  PSoundChannel * sndChan = PSoundChannel::CreateChannel(driverName);
  if (sndChan != NULL) {
    if (!sndChan->Open(deviceName, dir, numChannels, sampleRate, bitsPerSample)) {
      delete sndChan;
      sndChan = NULL;
    }
  }
  return sndChan;
}


PStringList PSoundChannel::GetDeviceNames(
      Directions dir    // Sound I/O direction
    )
{
  PStringArray names = GetDriverNames();
  if (names.GetSize() > 0)
    return GetDeviceNames(names[0], dir);
  return PStringList();
}

PString PSoundChannel::GetDefaultDevice(
      Directions dir    // Sound I/O direction
)
{
  PStringList devices = GetDeviceNames(dir);
  if (devices.GetSize() > 0)
    return devices[0];

  return PString::Empty();
}


PSoundChannel::PSoundChannel()
{
  baseChannel = NULL;
}

PSoundChannel::~PSoundChannel()
{
  delete baseChannel;
}

PSoundChannel::PSoundChannel(
      const PString & device,       /// Name of sound driver/device
      Directions dir,               /// Sound I/O direction
      unsigned numChannels,         /// Number of channels eg mono/stereo
      unsigned sampleRate,          /// Samples per second
      unsigned bitsPerSample        /// Number of bits per sample
)
{
  baseChannel = NULL;
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}

BOOL PSoundChannel::Open(
      const PString & device,   /// Name of sound driver/device
      Directions dir,           /// Sound I/O direction
      unsigned numChannels,     /// Number of channels eg mono/stereo
      unsigned sampleRate,      /// Samples per second
      unsigned bitsPerSample    /// Number of bits per sample
    )
{
  if (baseChannel == NULL) {
    PStringArray names = GetDriverNames();
    if (names.GetSize() == 0)
      return FALSE;

    baseChannel = CreateChannel(names[0]);
  }

  if (baseChannel == NULL)
    return FALSE;

  return baseChannel->Open(device, dir, numChannels, sampleRate, bitsPerSample);
}

///////////////////////////////////////////////////////////////////////////

#if !defined(_WIN32) && !defined(__BEOS__)

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
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
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

