/*
 * sound.cxx
 *
 * Sound driver implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: oss.cxx,v $
 * Revision 1.2  1999/05/22 12:49:05  craigs
 * Finished implementation for Linux OSS interface
 *
 * Revision 1.1  1999/02/25 03:45:00  robertj
 * Sound driver implementation changes for various unix platforms.
 *
 * Revision 1.1  1999/02/22 13:24:47  robertj
 * Added first cut sound implmentation.
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

#include <sys/soundcard.h>
#include <sys/time.h>


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


///////////////////////////////////////////////////////////////////////////////

PSoundChannel::PSoundChannel()
{
  Construct();
}


PSoundChannel::PSoundChannel(const PString & device,
                             Directions dir,
                             unsigned numChannels,
                             unsigned sampleRate,
                             unsigned bitsPerSample)
{
  Construct();
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}


void PSoundChannel::Construct()
{
}


PSoundChannel::~PSoundChannel()
{
  Close();
}


PStringArray PSoundChannel::GetDeviceNames(Directions /*dir*/)
{
  PStringArray array;

  array[0] = "/dev/audio";
  array[1] = "/dev/dsp";

  return array;
}


BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  Close();

  if (!ConvertOSError(os_handle = ::open(device, (dir != Player) ? O_RDONLY : O_WRONLY)))
    return FALSE;

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  return TRUE;

  Abort();

  // must always set paramaters in the following order:
  //   sample format (number of bits)
  //   number of channels (mon/stereo)
  //   speed (sampling rate)

  int arg, val;

  PAssert((bitsPerSample == 8) || (bitsPerSample == 16), PInvalidParameter);
  arg = val = (bitsPerSample == 16) ? AFMT_S16_LE : AFMT_S8;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SETFMT, &arg)) || (arg != val))
    return FALSE;

  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  arg = val = (numChannels == 2) ? 1 : 0;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_STEREO, &arg)) || (arg != val))
    return FALSE;

  arg = val = sampleRate;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SPEED, &arg)) || (arg != val))
    return FALSE;

  return TRUE;
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);
  int arg = 1;
  while (size < (PINDEX)(1 << arg))
    arg++;
  arg |= count << 16;
  return ConvertOSError(ioctl(os_handle, SNDCTL_DSP_SETFRAGMENT, &arg));
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  int arg;
  if (!ConvertOSError(ioctl(os_handle, SNDCTL_DSP_GETBLKSIZE, &arg)))
    return FALSE;

  count = arg >> 16;
  size = 1 << (arg&0xffff);
  return TRUE;
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
  Abort();

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return FALSE;

  if (wait)
    return WaitForPlayCompletion();

  return TRUE;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
}


BOOL PSoundChannel::HasPlayCompleted()
{
  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETOSPACE, &info)))
    return FALSE;

  return info.fragments == info.fragstotal;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  return ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SYNC, NULL));
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
}


BOOL PSoundChannel::StartRecording()
{
  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(os_handle, &fds);

  struct timeval timeout;
  memset(&timeout, 0, sizeof(timeout));

  return ConvertOSError(::select(1, &fds, NULL, NULL, &timeout));
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETISPACE, &info)))
    return FALSE;

  return info.fragments > 0;
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETISPACE, &info)))
    return FALSE;

  return info.fragments == info.fragstotal;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannel::Abort()
{
  return ConvertOSError(ioctl(os_handle, SNDCTL_DSP_RESET, NULL));
}


// End of file

