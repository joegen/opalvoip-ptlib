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
 * $Log: esdaudio.cxx,v $
 * Revision 1.2  2001/09/24 08:56:43  rogerh
 * Remove LastError, submitted by Andreas Wrede <awrede@mac.com>
 *
 * Revision 1.1  2001/07/19 09:27:12  rogerh
 * Add support for EsounD and esd (the Enlightenment Sound Daemon).
 * This allows OhPhone to run on platforms where EsounD and esd have been
 * ported which includes Mac OS X.
 * Code written by Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>.
 *
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>
#include <esd.h>

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

/* for loopback test */
#define LOOPBACK_BUFFER_SIZE 5000
#define BYTESINBUF ((startptr<endptr)?(endptr-startptr):\
                                      (LOOPBACK_BUFFER_SIZE+endptr-startptr)

static char buffer[LOOPBACK_BUFFER_SIZE];
static int  startptr, endptr;


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

  array[0] = "ESound";

  return array;
}


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "ESound";
}

BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  int bits, channels, rate, mode, func;
  esd_format_t format;
  char *host = NULL, *name = NULL;

  BOOL loopback_test = 0; /* for loopback test */

  Close();

  // make sure we have proper bits per sample
  switch (bitsPerSample) {
  case 16:
    bits = ESD_BITS16;
    break;
  case 8:
    bits = ESD_BITS8;
    break;
  default:
    return (FALSE);
  }

  // make sure we have proper number of channels
  switch (numChannels) {
  case 2:
    channels = ESD_STEREO;
    break;
  case 1:
    channels = ESD_MONO;
    break;
  default:
    return (FALSE);
  }

  rate = sampleRate;
  mode = ESD_STREAM;

  // a separate stream for Player and Recorder
  switch (dir) {
  case Recorder:
    func = ESD_RECORD;
    break;
  case Player:
    func = ESD_PLAY;
    break;
  default:
    return (FALSE);
  }

  if (!loopback_test) {
    format = bits | channels | mode | func;
    if (dir == Recorder) 
      os_handle = esd_record_stream_fallback( format, rate, host, name );
    else
      os_handle = esd_play_stream_fallback( format, rate, host, name );

    if ( os_handle <= 0 ) 
      return (FALSE);
  }
  else {
    // os_handle always equals to 0 in loopback_test
    os_handle = 0;
  }

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
  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  PAssert(bitsPerSample == 8 || bitsPerSample == 16, PInvalidParameter);

  return TRUE;
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  return TRUE;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  return TRUE;
}


BOOL PSoundChannel::Write(const void * buf, PINDEX len)
{
  int rval;

  if (os_handle > 0) {
    // see if it is a silence frame

    PINDEX i = 0;
    while (1) {
      if (((short *)buf)[i++] != 0) {
	break;
      }
      if (i >= len/sizeof(short))
	return (TRUE);
    }

    // Sends data to esd.
    rval = ::write(os_handle, buf, len);
    if (rval > 0) {
      return (TRUE);
    }
    else {
      return (FALSE);
    }
  }
  else {
    // loopback
    int index = 0;

    while (len > 0) {
      len--;
      buffer[endptr++] = ((char *)buf)[index++];
      if (endptr == LOOPBACK_BUFFER_SIZE)
	endptr = 0;
      while (((startptr - 1) == endptr) ||
	     ((endptr==LOOPBACK_BUFFER_SIZE - 1) && (startptr==0))) {
	usleep(5000);
      }
    }
    return TRUE;
    // end loopback
  }
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
  return TRUE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  return FALSE;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  return TRUE;
}


BOOL PSoundChannel::Read(void * buf, PINDEX len)
{
  int rval;

  if (os_handle > 0) {
    rval = ::read(os_handle, buf, len);
    if (rval > 0) {
      return (TRUE);
    }
    else {
      return (FALSE);
    }
  }
  else {
    // loopback
    int index = 0;

    while (len > 0) {
      while (startptr == endptr)
	usleep(5000);
      len--;
      ((char *)buf)[index++]=buffer[startptr++];
      if (startptr == LOOPBACK_BUFFER_SIZE)
	startptr = 0;
    }
    return TRUE;
    // end loopback
  }
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
  return TRUE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  return TRUE;
}


BOOL PSoundChannel::StartRecording()
{
  return (TRUE);
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  return (TRUE);
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  return TRUE;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0) {
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
  return TRUE;
}


// End of file

