/*
 * dummyaudio.cxx
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
 * $Log: dummyaudio.cxx,v $
 * Revision 1.5  2005/07/13 13:02:35  csoutheren
 * Unified interface across Windows and Unix
 *
 * Revision 1.4  2002/02/09 00:52:01  robertj
 * Slight adjustment to API and documentation for volume functions.
 *
 * Revision 1.3  2002/02/07 20:57:21  dereks
 * add SetVolume and GetVolume methods to PSoundChannel
 *
 * Revision 1.2  2001/09/27 08:37:45  rogerh
 * remove unwanted lastError
 *
 * Revision 1.1  2001/02/23 08:48:10  rogerh
 * Implement a dummy PSoundChannel class. There is no functionality
 * but it allows OpenH323 to link.
 *
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

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
  return PFalse;
}


PBoolean PSound::Save(const PFilePath & /*filename*/)
{
  return PFalse;
}

PBoolean PSound::Play(const PString & device)
{

  PSoundChannel channel(device,
                       PSoundChannel::Player);
  if (!channel.IsOpen())
    return PFalse;

  return channel.PlaySound(*this, PTrue);
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


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "/dev/audio";
}


PBoolean PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  Close();

  if (!ConvertOSError(os_handle = ::open(device, dir == Player ? O_RDONLY : O_WRONLY)))
    return PFalse;

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


PBoolean PSoundChannel::Close()
{
  return PChannel::Close();
}


PBoolean PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  Abort();

  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  PAssert(bitsPerSample == 8 || bitsPerSample == 16, PInvalidParameter);

  return PTrue;
}


PBoolean PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  return PTrue;
}


PBoolean PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  return PTrue;
}


PBoolean PSoundChannel::Write(const void * buffer, PINDEX length)
{
  return PChannel::Write(buffer, length);
}


PBoolean PSoundChannel::PlaySound(const PSound & sound, PBoolean wait)
{
  Abort();

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return PFalse;

  if (wait)
    return WaitForPlayCompletion();

  return PTrue;
}


PBoolean PSoundChannel::PlayFile(const PFilePath & filename, PBoolean wait)
{
  return PTrue;
}


PBoolean PSoundChannel::HasPlayCompleted()
{
  return PTrue;
}


PBoolean PSoundChannel::WaitForPlayCompletion()
{
  return PTrue;
}


PBoolean PSoundChannel::Read(void * buffer, PINDEX length)
{
  return PChannel::Read(buffer, length);
}


PBoolean PSoundChannel::RecordSound(PSound & sound)
{
  return PTrue;
}


PBoolean PSoundChannel::RecordFile(const PFilePath & filename)
{
  return PTrue;
}


PBoolean PSoundChannel::StartRecording()
{
  return PTrue;
}


PBoolean PSoundChannel::IsRecordBufferFull()
{
  return PTrue;
}


PBoolean PSoundChannel::AreAllRecordBuffersFull()
{
  return PTrue;
}


PBoolean PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0) {
    return PFalse;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


PBoolean PSoundChannel::WaitForAllRecordBuffersFull()
{
  return PFalse;
}


PBoolean PSoundChannel::Abort()
{
  return PTrue;
}

PBoolean PSoundChannel::SetVolume(unsigned newVolume)
{
  return PFalse;
}

PBoolean  PSoundChannel::GetVolume(unsigned & volume)
{
  return PFalse;
}


// End of file

