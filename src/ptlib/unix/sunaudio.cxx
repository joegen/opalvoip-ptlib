/*
 * sunaudio.cxx
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
 * $Log: sunaudio.cxx,v $
 * Revision 1.5  2001/10/18 01:15:18  robertj
 * Fixed compile error.
 *
 * Revision 1.4  1999/09/03 02:01:34  robertj
 * Added missing functions so will at least link
 *
 * Revision 1.3  1999/06/28 09:28:02  robertj
 * Portability issues, especially n BeOS (thanks Yuri!)
 *
 * Revision 1.2  1999/03/02 04:34:10  robertj
 * Fixed header comments
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

#include <sys/audioio.h>


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


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "/dev/audio";
}


BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  Close();

  if (!ConvertOSError(os_handle = ::open(device, dir == Player ? O_RDONLY : O_WRONLY)))
    return FALSE;

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


BOOL PSoundChannel::Close()
{
  return PChannel::Close();
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  Abort();

  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  PAssert(bitsPerSample == 8 || bitsPerSample == 16, PInvalidParameter);
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  return TRUE;
}


BOOL PSoundChannel::Write(const void * buffer, PINDEX length)
{
  return PChannel::Write(buffer, length);
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
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
}


BOOL PSoundChannel::Read(void * buffer, PINDEX length)
{
  return PChannel::Read(buffer, length);
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
}


BOOL PSoundChannel::StartRecording()
{
}


BOOL PSoundChannel::IsRecordBufferFull()
{
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannel::Abort()
{
}


// End of file

