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
 * Revision 1.4  1999/06/30 13:49:26  craigs
 * Added code to allow full duplex audio
 *
 * Revision 1.3  1999/05/28 14:14:29  robertj
 * Added function to get default audio device.
 *
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

PSoundHandleDict PSoundChannel::handleDict;
PMutex           PSoundChannel::dictMutex;

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

PSoundHandleEntry::PSoundHandleEntry()
{
  handle    = -1;
  direction = 0;
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
  os_handle = -1;
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
  array[2] = "/dev/dspW";

  return array;
}


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "/dev/dsp";
}


BOOL PSoundChannel::Open(const PString & _device,
                         Directions _dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  Close();

  // make the direction value either 1 or 2
  int dir = _dir+1;

  // save the new device name
  device = _device;

  dictMutex.Wait();

  // if the entry is in the dictionary, then see if we can reuse the handle
  if (handleDict.Contains(device)) {
    PSoundHandleEntry & entry = handleDict[device];

    // see if the sound channel is already open in this direction
    if ((entry.direction & dir) != 0) {
      dictMutex.Signal();
      return FALSE;
    }

    // indicate the device is now open in both directions
    entry.direction |= dir;

    // and indicate that this channel is now open
    os_handle = entry.handle;
    direction = dir;

    dictMutex.Signal();

    return TRUE;
    
  } 

  // open the device in read/write mode always
  if (!ConvertOSError(os_handle = ::open(device, O_RDWR)))
    return FALSE;

  // always open in full duplex mode always
  //if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SETDUPLEX, 0)))
  //  return FALSE;

  // add the device to the dictionary
  PSoundHandleEntry * entry = PNEW PSoundHandleEntry;
  handleDict.SetAt(device, entry);

  // save the direction
  direction = dir;

  // save the information into the dictionary entry
  entry->handle    = os_handle;
  entry->direction = dir;
    
  dictMutex.Signal();

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


BOOL PSoundChannel::Close()
{
  // if the channel isn't open, do nothing
  if (os_handle < 0)
    return TRUE;

  // the device must be in the dictionary
  dictMutex.Wait();
  PSoundHandleEntry * entry;
  PAssert((entry = handleDict.GetAt(device)) != NULL, "Unknown sound device \"" + device + "\" found");

  // modify the directions bit mask in the dictionary
  entry->direction ^= direction;

  // if this is the last usage of this entry, then remove it
  if (entry->direction == 0) {
    handleDict.RemoveAt(device);
    dictMutex.Signal();
    return PChannel::Close();
  }

  // flag this channel as closed
  dictMutex.Signal();
  os_handle = -1;
  return TRUE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  Abort();

  // must always set paramaters in the following order:
  //   sample format (number of bits)
  //   number of channels (mon/stereo)
  //   speed (sampling rate)

  int arg, val;

  // reset the device first so it will accept the new parms
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_RESET, &arg)))
    return FALSE;

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
  return FALSE;
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
  return FALSE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  return FALSE;
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

