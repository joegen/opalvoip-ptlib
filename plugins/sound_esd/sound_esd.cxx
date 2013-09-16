/*
 * esdaudio.cxx
 *
 * Sound driver implementation to use ESound.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Equivalence Pty. Ltd.
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
 * Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#pragma implementation "sound.h"

#include <ptlib.h>
#include <esd.h>

#pragma implementation "sound_esd.h"

#include "sound_esd.h"

PCREATE_SOUND_PLUGIN(ESD, PSoundChannelESD);

///////////////////////////////////////////////////////////////////////////////

PSoundChannelESD::PSoundChannelESD()
{
}


PSoundChannelESD::~PSoundChannelESD()
{
  Close();
}


PStringArray PSoundChannelESD::GetDeviceNames(Directions /*dir*/)
{
  PStringArray array;

  array[0] = "ESound";

  return array;
}


PString PSoundChannelESD::GetDefaultDevice(Directions /*dir*/)
{
  return "ESound";
}

bool PSoundChannelESD::Open(const Params & params)
{
  int bits, channels, rate, mode, func;
  esd_format_t format;
  char *host = NULL, *name = NULL;

  Close();

  // make sure we have proper bits per sample
  switch (params.m_bitsPerSample) {
  case 16:
    bits = ESD_BITS16;
    break;
  case 8:
    bits = ESD_BITS8;
    break;
  default:
    return (false);
  }

  // make sure we have proper number of channels
  switch (params.m_channels) {
  case 2:
    channels = ESD_STEREO;
    break;
  case 1:
    channels = ESD_MONO;
    break;
  default:
    return (false);
  }

  rate = params.m_sampleRate;
  mode = ESD_STREAM;

  // a separate stream for Player and Recorder
  switch (params.m_direction) {
  case Recorder:
    func = ESD_RECORD;
    break;
  case Player:
    func = ESD_PLAY;
    break;
  default:
    return (false);
  }

  format = bits | channels | mode | func;
  if (params.m_direction == Recorder) 
    os_handle = esd_record_stream_fallback( format, rate, host, name );
  else
    os_handle = esd_play_stream_fallback( format, rate, host, name );

  if ( os_handle <= 0 ) 
    return (false);

  return SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
}

PBoolean PSoundChannelESD::SetVolume(unsigned newVal)
{
  if (os_handle <= 0)  //Cannot set volume in loop back mode.
    return false;
  
  return false;
}

PBoolean  PSoundChannelESD::GetVolume(unsigned &devVol)
{
  if (os_handle <= 0)  //Cannot get volume in loop back mode.
    return false;
  
  devVol = 0;
  return false;
}
  


PBoolean PSoundChannelESD::Close()
{
  /* I think there is a bug here. We should be testing for loopback mode
   * and NOT calling PChannel::Close() when we are in loopback mode.
   * (otherwise we close file handle 0 which is stdin)
   */

  return PChannel::Close();
}


PBoolean PSoundChannelESD::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  PAssert(bitsPerSample == 8 || bitsPerSample == 16, PInvalidParameter);

  return true;
}


PBoolean PSoundChannelESD::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  return true;
}


PBoolean PSoundChannelESD::GetBuffers(PINDEX & size, PINDEX & count)
{
  return true;
}


PBoolean PSoundChannelESD::Write(const void * buf, PINDEX len)
{
  int rval;

  if (os_handle >= 0) {

    // Sends data to esd.
    rval = ::write(os_handle, buf, len);
    if (rval > 0) {
#if 0 //defined(P_MACOSX)
      // Mac OS X's esd has a big input buffer so we need to simulate
      // writing data out at the correct rate.
      writeDelay.Delay(len/16);
#endif
      return (true);
    }
  }

  return false;
}


PBoolean PSoundChannelESD::PlaySound(const PSound & sound, PBoolean wait)
{
  Abort();

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return false;

  if (wait)
    return WaitForPlayCompletion();

  return true;
}


PBoolean PSoundChannelESD::PlayFile(const PFilePath & filename, PBoolean wait)
{
  return true;
}


PBoolean PSoundChannelESD::HasPlayCompleted()
{
  return false;
}


PBoolean PSoundChannelESD::WaitForPlayCompletion()
{
  return true;
}


PBoolean PSoundChannelESD::Read(void * buf, PINDEX len)
{
  if (os_handle < 0) 
    return false;

  lastReadCount = 0;
  // keep looping until we have read 'len' bytes
  while (lastReadCount < len) {
    int retval = ::read(os_handle, ((char *)buf)+lastReadCount, len-lastReadCount);
    if (retval <= 0) return false;
    lastReadCount += retval;
  }
  return (true);
}


PBoolean PSoundChannelESD::RecordSound(PSound & sound)
{
  return true;
}


PBoolean PSoundChannelESD::RecordFile(const PFilePath & filename)
{
  return true;
}


PBoolean PSoundChannelESD::StartRecording()
{
  return (true);
}


PBoolean PSoundChannelESD::IsRecordBufferFull()
{
  return (true);
}


PBoolean PSoundChannelESD::AreAllRecordBuffersFull()
{
  return true;
}


PBoolean PSoundChannelESD::WaitForRecordBufferFull()
{
  if (os_handle < 0) {
    return false;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


PBoolean PSoundChannelESD::WaitForAllRecordBuffersFull()
{
  return false;
}


PBoolean PSoundChannelESD::Abort()
{
  return true;
}


// End of file

