/*
 * sound_alsa.cxx
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
 * The Initial Developer of the Original ALSA Code is
 * Damien Sandras <dsandras@seconix.com>
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): /
 *
 */

#pragma implementation "sound_alsa.h"

#include "sound_alsa.h"
#include <ptclib/pwavfile.h>

PCREATE_SOUND_PLUGIN(ALSA, PSoundChannelALSA)


static PStringToOrdinal playback_devices;
static PStringToOrdinal capture_devices;
PMutex dictionaryMutex;

///////////////////////////////////////////////////////////////////////////////

PSoundChannelALSA::PSoundChannelALSA()
  : mNumChannels(1)
  , mSampleRate(8000)
  , mBitsPerSample(16)
  , isInitialised(false)
  , pcm_handle(NULL)
  , card_nr(-1)
  , m_bufferSize(320)
  , m_bufferCount(2)
  , frameBytes(2)
{
}


PSoundChannelALSA::~PSoundChannelALSA()
{
  Close();
}


void PSoundChannelALSA::UpdateDictionary(Directions dir)
{
  PWaitAndSignal mutex(dictionaryMutex);

  PStringToOrdinal & devices = dir == Recorder ? capture_devices : playback_devices;
  devices.RemoveAll();

  int cardNum = -1;
  if (snd_card_next(&cardNum) < 0 || cardNum < 0)
    return;  // No sound card found

  snd_ctl_card_info_t * info = NULL;
  snd_ctl_card_info_alloca(&info);

  snd_pcm_info_t * pcminfo = NULL;
  snd_pcm_info_alloca(&pcminfo);

  do {
    char card_id[32];
    snprintf(card_id, sizeof(card_id), "hw:%d", cardNum);

    snd_ctl_t * handle = NULL;
    if (snd_ctl_open(&handle, card_id, 0) == 0) {
      snd_ctl_card_info(handle, info);

      int dev = -1;
      for (;;) {
        snd_ctl_pcm_next_device(handle, &dev);
        if (dev < 0)
          break;

        snd_pcm_info_set_device(pcminfo, dev);
        snd_pcm_info_set_subdevice(pcminfo, 0);
        snd_pcm_info_set_stream(pcminfo, dir == Recorder ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK);

        if (snd_ctl_pcm_info(handle, pcminfo) >= 0) {
          char * rawName = NULL;
          snd_card_get_name(cardNum, &rawName);
          if (rawName != NULL) {
            int disambiguator = 1;
            PString uniqueName = rawName;
            uniqueName = uniqueName + " [" + snd_pcm_info_get_name (pcminfo) + "]";
            PString save = uniqueName;
            while (devices.Contains(uniqueName)) {
              uniqueName = save;
              uniqueName.sprintf(" (%d)", disambiguator++);
            }

            devices.SetAt(uniqueName, cardNum);
            free(rawName);
          }
        }
      }
      snd_ctl_close(handle);
    }

    snd_card_next(&cardNum);
  } while (cardNum >= 0);
}


PStringArray PSoundChannelALSA::GetDeviceNames(Directions dir)
{
  PStringArray devices;

  UpdateDictionary(dir);

  if (dir == Recorder) {
    if (capture_devices.GetSize() > 0)
      devices += "Default";
    for (PStringToOrdinal::iterator it = capture_devices.begin(); it != capture_devices.end() ; ++it)
      devices += it->first;
  }
  else {
    if (playback_devices.GetSize() > 0)
      devices += "Default";
    for (PStringToOrdinal::iterator it = playback_devices.begin(); it != playback_devices.end() ; ++it)
      devices += it->first;
  }

  PTRACE2(5, NULL, "ALSA\t" << dir << " devices: " << setfill(',') << devices);
  return devices;
}


PString PSoundChannelALSA::GetDefaultDevice(Directions dir)
{
  PStringArray devicenames = PSoundChannelALSA::GetDeviceNames(dir);
  if (devicenames.IsEmpty())
    return PString::Empty();
  return devicenames[0];
}


bool PSoundChannelALSA::Open(const Params & params)
{
  Close();

  PWaitAndSignal m(device_mutex);

  activeDirection = params.m_direction;
  mNumChannels = params.m_channels;
  mSampleRate = params.m_sampleRate;
  mBitsPerSample = params.m_bitsPerSample;

  device = params.m_device;

  PString real_device_name;
  if (params.m_device == "Default") {
    real_device_name = "default";
    card_nr = -2;
  }
  else {
    PStringToOrdinal & devices = activeDirection == Recorder ? capture_devices : playback_devices;
    if (devices.IsEmpty())
      UpdateDictionary(activeDirection);

    POrdinalKey * index = devices.GetAt(params.m_device);
    if (index == NULL) {
      PTRACE(1, "ALSA\tDevice not found");
      return false;
    }

    card_nr = *index;
    real_device_name.sprintf("plughw:%i", card_nr);
  }

  /* Open in NONBLOCK mode */
  if (snd_pcm_open(&pcm_handle,
                   real_device_name,
                   activeDirection == Recorder ? SND_PCM_STREAM_CAPTURE : SND_PCM_STREAM_PLAYBACK,
                   SND_PCM_NONBLOCK) < 0) {
    PTRACE(1, "ALSA\tOpen failed for \"" << params.m_device << "\", card=" << card_nr);
    return false;
  }

  snd_pcm_nonblock(pcm_handle, 0);

  if (!SetHardwareParams())
    return false;

  PTRACE(3, "ALSA\tDevice \"" << device << "\", card=" << card_nr << " opened");
  os_handle=1;

  return true;
}

bool PSoundChannelALSA::SetHardwareParams()
{
  if (isInitialised)
    return true;

  if (!pcm_handle)
    return SetErrorValues(NotOpen, EBADF);

  enum _snd_pcm_format sndFormat;
#if PBYTE_ORDER == PLITTLE_ENDIAN
  sndFormat = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
#else
  sndFormat = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_U8;
#endif

  frameBytes = (mNumChannels * (snd_pcm_format_width(sndFormat) / 8));
  if (frameBytes == 0)
    frameBytes = 2;

  PTRACE(4,"ALSA\tSetHardwareParams " << ((activeDirection == Player) ? "Player" : "Recorder")
         << " channels=" << mNumChannels
         << " sample rate=" << mSampleRate
         << " buffer size=" << m_bufferSize
         << " buffer count=" << m_bufferCount
         << " sndFormat=" << sndFormat
         << " frameBytes=" << frameBytes);

  int err;

  // Finally set the hardware parameters
  snd_pcm_hw_params_t *hw_params = NULL;
  snd_pcm_hw_params_alloca(&hw_params);

  if ((err = snd_pcm_hw_params_any(pcm_handle, hw_params)) < 0) {
    PTRACE(1, "ALSA\tCannot initialize hardware parameter structure: " << snd_strerror(err));
    return false;
  }


  if ((err = snd_pcm_hw_params_set_access(pcm_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    PTRACE(1, "ALSA\tCannot set access type: " <<  snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_set_format(pcm_handle, hw_params, sndFormat)) < 0) {
    PTRACE(1, "ALSA\tCannot set sample format: " << snd_strerror(err));
    return false;
  }


  if ((err = snd_pcm_hw_params_set_channels(pcm_handle, hw_params, mNumChannels)) < 0) {
    PTRACE(1, "ALSA\tCannot set channel count: " << snd_strerror(err));
    return false;
  }

  if ((err = snd_pcm_hw_params_set_rate_near(pcm_handle, hw_params, &mSampleRate, NULL)) < 0) {
    PTRACE(1, "ALSA\tCannot set sample rate: " << snd_strerror(err));
    return false;
  }

  int dir = 0;
  int totalBufferSize = m_bufferSize*m_bufferCount;
  snd_pcm_uframes_t desiredPeriodSize = m_bufferSize/frameBytes;

  /* use of get function (ie. snd_pcm_hw_params_get_period_size) and the check as done before was, in my opinion, was pretty unuseful
      because actually the set function (ie.snd_pcm_hw_params_set_period_size_near) returns the real set value
      in the argument passed (ie. desiredPeriodSize) */
  if ((err = snd_pcm_hw_params_set_period_size_near(pcm_handle, hw_params, &desiredPeriodSize, &dir)) < 0) {
      PTRACE(1, "ALSA\tCannot set period size: " << snd_strerror(err));
  }
  else {
      PTRACE(5, "ALSA\tSuccessfully set period size to " << desiredPeriodSize);
  }

  /* i experimented (3 different sound cards) that is better to rounds value to the nearest integer to avoid buffer underrun/overrun */
  unsigned desiredPeriods = (unsigned)(((float)totalBufferSize / (float)(desiredPeriodSize*frameBytes))+0.5);
  if (desiredPeriods < 2)
    desiredPeriods = 2;

  if ((err = (int) snd_pcm_hw_params_set_periods_near(pcm_handle, hw_params, &desiredPeriods, &dir)) < 0) {
    PTRACE(1, "ALSA\tCannot set periods to: " << snd_strerror(err));
  }
  else {
    PTRACE(5, "ALSA\tSuccessfully set periods to " << desiredPeriods);
  }

  for (unsigned retry = 0; retry < 100; ++retry) {
    if ((err = snd_pcm_hw_params(pcm_handle, hw_params)) >= 0) {
      PTRACE(4, "ALSA\thardware parameters set");
      isInitialised = true;
      return true;
    }

    if (err != -EAGAIN && err != -EBADFD)
      break;

    PTRACE(4, "ALSA\tRetrying after temporary error: " << snd_strerror(err));
    usleep(1000);
  }

  PTRACE(1, "ALSA\tCannot set parameters: " << snd_strerror(err));
  return false;
}


PBoolean PSoundChannelALSA::Close()
{
  PWaitAndSignal m(device_mutex);

  /* if the channel isn't open, do nothing */
  if (!pcm_handle)
    return false;

  PTRACE(3, "ALSA\tClosing \"" << device << "\", card=" << card_nr);
  snd_pcm_close(pcm_handle);
  pcm_handle = NULL;
  os_handle = -1;
  isInitialised = false;
  card_nr = 0;

  return true;
}


PBoolean PSoundChannelALSA::Write(const void *buf, PINDEX len)
{
  lastWriteCount = 0;

  PWaitAndSignal m(device_mutex);

  if (!SetHardwareParams())
    return false;

  int pos = 0, max_try = 0;
  const char * buf2 = (const char *)buf;
  do {
    /* the number of frames to read is the buffer length
    divided by the size of one frame */
    long r = snd_pcm_writei(pcm_handle, (char *) &buf2 [pos], len / frameBytes);

    if (r >= 0) {
      pos += r * frameBytes;
      len -= r * frameBytes;
      lastWriteCount += r * frameBytes;
    }
    else {
      PTRACE(5, "ALSA\tBuffer underrun detected. Recovering... ");
      if (r == -EPIPE) {    /* under-run */
        r = snd_pcm_prepare(pcm_handle);
        PTRACE_IF(1, r < 0, "ALSA\tCould not prepare device: " << snd_strerror(r));
      }
      else if (r == -ESTRPIPE) {
        PTRACE(5, "ALSA\tOutput suspended. Resuming... ");
        while ((r = snd_pcm_resume(pcm_handle)) == -EAGAIN)
          sleep(1);       /* wait until the suspend flag is released */

        if (r < 0) {
          r = snd_pcm_prepare(pcm_handle);
          PTRACE_IF(1, r < 0, "ALSA\tCould not prepare device: " << snd_strerror(r));
        }
      }
      else {
        PTRACE(1, "ALSA\tCould not write " << max_try << " " << len << " " << snd_strerror(r));
      }

      max_try++;
      if (max_try > 5)
        return false;
    }
  } while (len > 0);

  return true;
}


PBoolean PSoundChannelALSA::Read(void * buf, PINDEX len)
{
  lastReadCount = 0;

  PWaitAndSignal m(device_mutex);

  if (!SetHardwareParams())
    return false;

  memset((char *) buf, 0, len);

  int pos = 0, max_try = 0;
  char * buf2 = (char *)buf;

  do {
    /* the number of frames to read is the buffer length
    divided by the size of one frame */
    long r = snd_pcm_readi(pcm_handle, &buf2[pos],len/frameBytes);

    if (r >= 0) {
      pos += r * frameBytes;
      len -= r * frameBytes;
      lastReadCount += r * frameBytes;
    }
    else {
      if (r == -EPIPE) {    /* under-run */
	  snd_pcm_prepare(pcm_handle);
      }
      else if (r == -ESTRPIPE) {
        while ((r = snd_pcm_resume(pcm_handle)) == -EAGAIN)
          sleep(1);       /* wait until the suspend flag is released */

        if (r < 0)
          snd_pcm_prepare(pcm_handle);
      }

      PTRACE(1, "ALSA\tCould not read " << max_try << " " << len << " " << snd_strerror(r));

      max_try++;

      if (max_try > 5)
        return false;
    }
  } while (len > 0);

  return true;
}


PBoolean PSoundChannelALSA::SetFormat(unsigned numChannels,
                                      unsigned sampleRate,
                                      unsigned bitsPerSample)
{
  if (!pcm_handle)
    return SetErrorValues(NotOpen, EBADF);

  /* check parameters */
  PAssert((bitsPerSample == 8) || (bitsPerSample == 16), PInvalidParameter);
  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);

  mNumChannels   = numChannels;
  mSampleRate    = sampleRate;
  mBitsPerSample = bitsPerSample;

  /* mark this channel as uninitialised */
  isInitialised = false;

  return true;
}


unsigned PSoundChannelALSA::GetChannels() const
{
  return mNumChannels;
}


unsigned PSoundChannelALSA::GetSampleRate() const
{
  return mSampleRate;
}


unsigned PSoundChannelALSA::GetSampleSize() const
{
  return mBitsPerSample;
}


PBoolean PSoundChannelALSA::SetBuffers(PINDEX size, PINDEX count)
{
  PTRACE(4,"ALSA\tSetBuffers direction=" <<
	         ((activeDirection == Player) ? "Player" : "Recorder") << " size=" << size << " count=" << count);

  m_bufferSize = size;
  m_bufferCount = count;

  /* set actually new parameters */
  return SetHardwareParams();
}


PBoolean PSoundChannelALSA::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = m_bufferSize;
  count = m_bufferCount;
  return true;
}


PBoolean PSoundChannelALSA::HasPlayCompleted()
{
  if (!pcm_handle)
    return SetErrorValues(NotOpen, EBADF);

  return (snd_pcm_state(pcm_handle) != SND_PCM_STATE_RUNNING);
}


PBoolean PSoundChannelALSA::WaitForPlayCompletion()
{
  if (!pcm_handle)
    return SetErrorValues(NotOpen, EBADF);

  snd_pcm_drain(pcm_handle);

  return true;
}


PBoolean PSoundChannelALSA::StartRecording()
{
  return false;
}


PBoolean PSoundChannelALSA::IsRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannelALSA::AreAllRecordBuffersFull()
{
  return true;
}


PBoolean PSoundChannelALSA::WaitForRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannelALSA::WaitForAllRecordBuffersFull()
{
  return false;
}


PBoolean PSoundChannelALSA::Abort()
{
  int r = 0;

  if (!pcm_handle)
    return false;

  PTRACE(4, "ALSA\tAborting \"" << device << "\", card=" << card_nr);
  if ((r = snd_pcm_drain(pcm_handle)) < 0) {
    PTRACE(1, "ALSA\tCannot abort" << snd_strerror(r));
    return false;
  }

  return true;
}



PBoolean PSoundChannelALSA::SetVolume(unsigned newVal)
{
  unsigned i = 0;
  return Volume(true, newVal, i);
}


PBoolean  PSoundChannelALSA::GetVolume(unsigned &devVol)
{
  return Volume(false, 0, devVol);
}


PBoolean PSoundChannelALSA::IsOpen() const
{
  return pcm_handle != NULL;
}


PBoolean PSoundChannelALSA::Volume(PBoolean set, unsigned set_vol, unsigned &get_vol)
{
  int err = 0;
  snd_mixer_t *handle;
  snd_mixer_elem_t *elem;
  snd_mixer_selem_id_t *sid;

  const char *play_mix_name [] = { "PCM", "Master", "Speaker", NULL };
  const char *rec_mix_name [] = { "Capture", "Mic", NULL };
  PString card_name;

  long pmin = 0, pmax = 0;
  long int vol = 0;
  int i = 0;

  if (!pcm_handle)
    return false;

  if (card_nr == -2)
    card_name = "default";
  else
    card_name = "hw:" + PString(card_nr);

  //allocate simple id
  snd_mixer_selem_id_alloca(&sid);

  //sets simple-mixer index and name
  snd_mixer_selem_id_set_index(sid, 0);

  if ((err = snd_mixer_open(&handle, 0)) < 0) {
    PTRACE(1, "ALSA\tMixer open error: " << snd_strerror(err));
    return false;
  }

  if ((err = snd_mixer_attach(handle, card_name)) < 0) {
    PTRACE(1, "ALSA\tMixer attach " << card_name << " error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return false;
  }

  if ((err = snd_mixer_selem_register(handle, NULL, NULL)) < 0) {
    PTRACE(1, "ALSA\tMixer register error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return false;
  }

  err = snd_mixer_load(handle);
  if (err < 0) {
    PTRACE(1, "ALSA\tMixer load error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return false;
  }

  do {
    snd_mixer_selem_id_set_name(sid, (activeDirection == Player)?play_mix_name[i]:rec_mix_name[i]);
    elem = snd_mixer_find_selem(handle, sid);
    i++;
  } while (!elem && ((activeDirection == Player && play_mix_name[i] != NULL) || (activeDirection == Recorder && rec_mix_name[i] != NULL)));

  if (!elem) {
    PTRACE(1, "ALSA\tUnable to find simple control.");
    snd_mixer_close(handle);
    return false;
  }

  if (set) {
    if (activeDirection == Player) {
      snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax);
      vol = (set_vol * (pmax?pmax:31)) / 100;
      snd_mixer_selem_set_playback_volume_all(elem, vol);
    }
    else {
      snd_mixer_selem_get_capture_volume_range(elem, &pmin, &pmax);
      vol = (set_vol * (pmax?pmax:31)) / 100;
      snd_mixer_selem_set_capture_volume_all(elem, vol);
    }
    PTRACE(4, "ALSA\tSet volume to " << vol);
  }
  else {
    if (activeDirection == Player) {
      snd_mixer_selem_get_playback_volume_range(elem, &pmin, &pmax);
      snd_mixer_selem_get_playback_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
    }
    else {
      snd_mixer_selem_get_capture_volume_range(elem, &pmin, &pmax);
      snd_mixer_selem_get_capture_volume(elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
    }

    get_vol = (vol * 100) / (pmax?pmax:31);
    PTRACE(4, "ALSA\tGot volume " << vol);
  }

  snd_mixer_close(handle);

  return true;
}
