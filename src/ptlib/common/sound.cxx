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

PStringArray PSoundChannel::GetPluginNames()
{
  return PPluginManager::GetPluginManager().GetDevicePluginNames(soundPluginBaseClass);
}

PPlugin * PSoundChannel::GetPluginByName(const PString & name)
{
  return PPluginManager::GetPluginManager().GetDevicePluginByName(soundPluginBaseClass, name);
}

PSoundChannel * PSoundChannel::CreateChannelByName(const PString & name)
{
  return (PSoundChannel *)PPluginManager::GetPluginManager().CreateDeviceChannelByName(soundPluginBaseClass, name);
}

PStringArray PSoundChannel::GetPluginDeviceNames(const PString & name, Directions dir)
{
  return PPluginManager::GetPluginManager().GetDevicePluginDeviceNames(soundPluginBaseClass, name, dir);
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
  PStringArray names = GetPluginNames();
  if (names.GetSize() == 0)
    return FALSE;

  baseChannel = CreateChannelByName(names[0]);
  if (baseChannel == NULL)
    return FALSE;

  return baseChannel->Open(device, dir, numChannels, sampleRate, bitsPerSample);
}

PStringArray PSoundChannel::GetDeviceNames(
  Directions dir    // Sound I/O direction
)
{
  PStringArray names = GetPluginNames();
  if (names.GetSize() == 0)
    return PStringArray();

  return GetPluginDeviceNames(names[0], dir);
}

PString PSoundChannel::GetDefaultDevice(
      Directions dir    // Sound I/O direction
)
{
  PStringArray names = PSoundChannel::GetDeviceNames(dir);
  return names[0];
}
