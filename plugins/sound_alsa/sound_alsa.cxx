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
 * $Revision$
 * $Author$
 * $Date$
 */

#pragma implementation "sound_alsa.h"

#include "sound_alsa.h"


PCREATE_SOUND_PLUGIN(ALSA, PSoundChannelALSA)


static PStringToOrdinal playback_devices;
static PStringToOrdinal capture_devices;
PMutex dictionaryMutex;

///////////////////////////////////////////////////////////////////////////////

PSoundChannelALSA::PSoundChannelALSA()
{
  PSoundChannelALSA::Construct();
}


PSoundChannelALSA::PSoundChannelALSA (const PString &device,
                                          Directions dir,
                                            unsigned numChannels,
                                            unsigned sampleRate,
                                            unsigned bitsPerSample)
{
  Construct();
  Open (device, dir, numChannels, sampleRate, bitsPerSample);
}


void PSoundChannelALSA::Construct()
{
  enum _snd_pcm_format val = SND_PCM_FORMAT_UNKNOWN;

#if PBYTE_ORDER == PLITTLE_ENDIAN
  val = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
#else
  val = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_U8;
#endif

  frameBytes = (mNumChannels * (snd_pcm_format_width (val) / 8));
  storedPeriods = 4;
  storedSize = frameBytes * 3;

  card_nr = 0;
  os_handle = NULL;
}


PSoundChannelALSA::~PSoundChannelALSA()
{
  Close();
}


void PSoundChannelALSA::UpdateDictionary (Directions dir)
{
  int card = -1, dev = -1;
  
  snd_ctl_t *handle = NULL;
  snd_ctl_card_info_t *info = NULL;
  snd_pcm_info_t *pcminfo = NULL;
  snd_pcm_stream_t stream;

  char *name = NULL;
  char card_id [32];

  PWaitAndSignal m(dictionaryMutex);

  if (dir == Recorder) {

    stream = SND_PCM_STREAM_CAPTURE;
    capture_devices = PStringToOrdinal ();
  }
  else {

    stream = SND_PCM_STREAM_PLAYBACK;
    playback_devices = PStringToOrdinal ();
  }

  snd_ctl_card_info_alloca (&info);
  snd_pcm_info_alloca (&pcminfo);

  /* No sound card found */
  if (snd_card_next (&card) < 0 || card < 0) {

    return;
  }

  while (card >= 0) {

    snprintf (card_id, 32, "hw:%d", card);
    
    if (snd_ctl_open (&handle, card_id, 0) == 0) {

      snd_ctl_card_info (handle, info);

      while (1) {

        snd_ctl_pcm_next_device (handle, &dev);

        if (dev < 0)
          break;

        snd_pcm_info_set_device (pcminfo, dev);
        snd_pcm_info_set_subdevice (pcminfo, 0);
        snd_pcm_info_set_stream (pcminfo, stream);

        if (snd_ctl_pcm_info (handle, pcminfo) >= 0) {

          snd_card_get_name (card, &name);
          if (dir == Recorder) 
            capture_devices.SetAt (name, card);
          else 
            playback_devices.SetAt (name, card);
          free (name);
        }
      }
      snd_ctl_close(handle);
    }

    snd_card_next (&card);
  }
}


PStringArray PSoundChannelALSA::GetDeviceNames (Directions dir)
{
  PStringArray devices;
 
  UpdateDictionary (dir);
  
  if (dir == Recorder) {
    
    if (capture_devices.GetSize () > 0)
      devices += "Default";
    for (PINDEX j = 0 ; j < capture_devices.GetSize () ; j++) 
      devices += capture_devices.GetKeyAt (j);
  }
  else {

    if (playback_devices.GetSize () > 0)
      devices += "Default";
    for (PINDEX j = 0 ; j < playback_devices.GetSize () ; j++) 
      devices += playback_devices.GetKeyAt (j);
  }
  
  return devices;
}


PString PSoundChannelALSA::GetDefaultDevice(Directions dir)
{
  PStringArray devicenames;
  devicenames = PSoundChannelALSA::GetDeviceNames (dir);

  return devicenames[0];
}


PBoolean PSoundChannelALSA::Open (const PString & _device,
                              Directions _dir,
                              unsigned _numChannels,
                              unsigned _sampleRate,
                              unsigned _bitsPerSample)
{
  PString real_device_name;
  POrdinalKey *i = NULL;
  snd_pcm_stream_t stream;

  Close();

  direction = _dir;
  mNumChannels = _numChannels;
  mSampleRate = _sampleRate;
  mBitsPerSample = _bitsPerSample;
  isInitialised = PFalse;

  os_handle = NULL;

  PWaitAndSignal m(device_mutex);

  if (_dir == Recorder)
    stream = SND_PCM_STREAM_CAPTURE;
  else
    stream = SND_PCM_STREAM_PLAYBACK;

  /* Open in NONBLOCK mode */
  if (_device == "Default") {

    real_device_name = "default";
    card_nr = -2;
  }
  else {

  if ((_dir == Recorder && capture_devices.IsEmpty ())
      || (_dir == Player && playback_devices.IsEmpty ()))
    UpdateDictionary (_dir);

    i = (_dir == Recorder) ? capture_devices.GetAt (_device) : playback_devices.GetAt (_device);

    if (i) {

      real_device_name = "plughw:" + PString (*i);
      card_nr = *i;
    }
    else {

      PTRACE (1, "ALSA\tDevice not found");
      return PFalse;
    }
  }
    
  if (snd_pcm_open (&os_handle, real_device_name, stream, SND_PCM_NONBLOCK) < 0) {

    PTRACE (1, "ALSA\tOpen Failed");
    return PFalse;
  }
  else 
    snd_pcm_nonblock (os_handle, 0);
   
  /* save internal parameters */
  device = real_device_name;

  Setup ();
  PTRACE (1, "ALSA\tDevice " << real_device_name << " Opened");

  return PTrue;
}


PBoolean PSoundChannelALSA::Setup()
{
  snd_pcm_hw_params_t *hw_params = NULL;
  PStringStream msg;

  int err = 0;
  enum _snd_pcm_format val = SND_PCM_FORMAT_UNKNOWN;
  PBoolean no_error = PTrue;


  if (os_handle == NULL) {

    PTRACE(6, "ALSA\tSkipping setup of " << device << " as not open");
    return PFalse;
  }

  if (isInitialised) {

    PTRACE(6, "ALSA\tSkipping setup of " << device << " as instance already initialised");
    return PTrue;
  }

#if PBYTE_ORDER == PLITTLE_ENDIAN
  val = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
#else
  val = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_U8;
#endif

  frameBytes = (mNumChannels * (snd_pcm_format_width (val) / 8));

  snd_pcm_hw_params_alloca (&hw_params);

  if ((err = snd_pcm_hw_params_any (os_handle, hw_params)) < 0) {
    msg << "Cannot initialize hardware parameter structure " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
    no_error = PFalse;
  }


  if ((err = snd_pcm_hw_params_set_access (os_handle, hw_params, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
    msg << "Cannot set access type " <<  snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
    no_error = PFalse;
  }


  if ((err = snd_pcm_hw_params_set_format (os_handle, hw_params, val)) < 0) {
    msg << "Cannot set sample format " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    no_error = PFalse;
  }


  if ((err = snd_pcm_hw_params_set_channels (os_handle, hw_params, mNumChannels)) < 0) {
    msg << "Cannot set channel count " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
    no_error = PFalse;
  }

  if ((err = snd_pcm_hw_params_set_rate_near (os_handle, hw_params, &mSampleRate, NULL)) < 0) {
    msg << "Cannot set sample rate " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    no_error = PFalse;
  }
  
  snd_pcm_uframes_t period_size = storedSize / (frameBytes ? frameBytes : 2);
  if ((err = snd_pcm_hw_params_set_period_size_near (os_handle, hw_params, &period_size, 0)) < 0) { 
    msg << "Cannot set period size " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg); 
    cerr << msg << endl;
  }
  
  if ((err = (int) snd_pcm_hw_params_set_periods_near (os_handle, hw_params, (unsigned int *) &storedPeriods, 0)) < 0) {
    msg << "Cannot set periods to " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg); 
    cerr << msg << endl;
  }

 /*****The buffer time is TWICE  the period time.
       bufferTime is the time to play the stored data.
       periodTime is the duration played in micro seconds.
       For GSM, period time is 20 milliseconds.
       For most other codecs, period time is 30 milliseconds.
  ******/
  unsigned int period_time = period_size * 1000 * 1000 / (2 * mSampleRate);
  unsigned int buffer_time = period_time * storedPeriods;
  PTRACE(3, "Alsa\tBuffer time is " << buffer_time);
  PTRACE(3, "Alsa\tPeriod time is " << period_time);


  // Ignore errors here 
  if ((err = snd_pcm_hw_params_set_buffer_time_near (os_handle, hw_params, &buffer_time, NULL)) < 0) {
    msg << "Cannot set buffer_time to  " << (buffer_time / 1000) << " ms " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
  }

  if ((err = snd_pcm_hw_params_set_period_time_near (os_handle, hw_params, &period_time, 0)) < 0) {
    msg << "Cannot set period_time to " << (period_time / 1000) << " ms   " << snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
  }
        
  if ((err = snd_pcm_hw_params (os_handle, hw_params)) < 0) {
    msg << "Cannot set parameters " <<      snd_strerror (err);
    PTRACE (1, "ALSA\t" << msg);
    cerr << msg << endl;
    no_error = PFalse;
  }
  
  isInitialised = PTrue;

  return no_error;
}


PBoolean PSoundChannelALSA::Close()
{
  PStringStream msg;
  PWaitAndSignal m(device_mutex);

  /* if the channel isn't open, do nothing */
  if (!os_handle)
    return PFalse;

  snd_pcm_close (os_handle);
  os_handle = NULL;
  isInitialised = PFalse;
  
  return PTrue;
}


PBoolean PSoundChannelALSA::Write (const void *buf, PINDEX len)
{
  long r = 0;
  char *buf2 = (char *) buf;
  int pos = 0, max_try = 0;
 
  lastWriteCount = 0;
  PWaitAndSignal m(device_mutex);

  if ((!isInitialised && !Setup()) || !len || !os_handle)
    return PFalse;

  do {

      
    /* the number of frames to read is the buffer length 
       divided by the size of one frame */
    r = snd_pcm_writei (os_handle, (char *) &buf2 [pos], len / frameBytes);

    if (r > 0) {
      pos += r * frameBytes;
      len -= r * frameBytes;
      lastWriteCount += r * frameBytes;
    }
    else {
      if (r == -EPIPE) {    /* under-run */
        r = snd_pcm_prepare (os_handle);
        if (r < 0) {
          PTRACE (1, "ALSA\tCould not prepare device: " << snd_strerror (r));
        }
      } else if (r == -ESTRPIPE) {

        while ((r = snd_pcm_resume (os_handle)) == -EAGAIN)
          sleep(1);       /* wait until the suspend flag is released */
      
        if (r < 0) 
          snd_pcm_prepare (os_handle);
      }

      PTRACE (1, "ALSA\tCould not write " << max_try << " " << len << " " << snd_strerror(r));
      max_try++;
    }  
  } while (len > 0 && max_try < 5);

  return PTrue;
}


PBoolean PSoundChannelALSA::Read (void * buf, PINDEX len)
{
  long r = 0;

  char *buf2 = (char *) buf;
  int pos = 0, max_try = 0;

  lastReadCount = 0;
  PWaitAndSignal m(device_mutex);

  if ((!isInitialised && !Setup()) || !len || !os_handle)
    return PFalse;

  memset ((char *) buf, 0, len);
  
  do {

    /* the number of frames to read is the buffer length 
       divided by the size of one frame */
    r = snd_pcm_readi (os_handle, (char *) &buf2 [pos], len / frameBytes);
    if (r > 0) {
      pos += r * frameBytes;
      len -= r * frameBytes;
      lastReadCount += r * frameBytes;
    }
    else {
      if (r == -EPIPE) {    /* under-run */
        snd_pcm_prepare (os_handle);
      } 
      else if (r == -ESTRPIPE) {
        while ((r = snd_pcm_resume (os_handle)) == -EAGAIN)
          sleep(1);       /* wait until the suspend flag is released */
        if (r < 0) 
          snd_pcm_prepare (os_handle);
      }

      PTRACE (1, "ALSA\tCould not read");
      max_try++;
    }
  } while (len > 0 && max_try < 5);

 
  if (len != 0) {

    memset ((char *) &buf2 [pos], 0, len);
    lastReadCount += len;

    PTRACE (1, "ALSA\tRead Error, filling with zeros");
  }
  
  
  return PTrue;
}


PBoolean PSoundChannelALSA::SetFormat (unsigned numChannels,
                                   unsigned sampleRate,
                                   unsigned bitsPerSample)
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  /* check parameters */
  PAssert((bitsPerSample == 8) || (bitsPerSample == 16), PInvalidParameter);
  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);

  mNumChannels   = numChannels;
  mSampleRate    = sampleRate;
  mBitsPerSample = bitsPerSample;
 
  /* mark this channel as uninitialised */
  isInitialised = PFalse;

  return PTrue;
}


unsigned PSoundChannelALSA::GetChannels()   const
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


PBoolean PSoundChannelALSA::SetBuffers (PINDEX size, PINDEX count)
{
  storedPeriods = count;
  storedSize = size;

  isInitialised = PFalse;

  return PTrue;
}


PBoolean PSoundChannelALSA::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = storedSize;
  count = storedPeriods;
  
  return PFalse;
}


PBoolean PSoundChannelALSA::PlaySound(const PSound & sound, PBoolean wait)
{
  PINDEX pos = 0;
  PINDEX len = 0;
  char *buf = (char *) (const BYTE *) sound;

  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  len = sound.GetSize();
  do {

    if (!Write(&buf [pos], PMIN(320, len - pos)))
      return PFalse;
    pos += 320;
  } while (pos < len);

  if (wait)
    return WaitForPlayCompletion();

  return PTrue;
}


PBoolean PSoundChannelALSA::PlayFile(const PFilePath & filename, PBoolean wait)
{
  BYTE buffer [512];
  
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  PFile file (filename, PFile::ReadOnly);

  if (!file.IsOpen())
    return PFalse;

  for (;;) {

    if (!file.Read (buffer, 512))
      break;

    PINDEX len = file.GetLastReadCount();
    if (len == 0)
      break;
    if (!Write(buffer, len))
      break;
  }

  file.Close();

  if (wait)
    return WaitForPlayCompletion();

  return PTrue;
}


PBoolean PSoundChannelALSA::HasPlayCompleted()
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  return (snd_pcm_state (os_handle) != SND_PCM_STATE_RUNNING);
}


PBoolean PSoundChannelALSA::WaitForPlayCompletion()
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  snd_pcm_drain (os_handle);

  return PTrue;
}


PBoolean PSoundChannelALSA::RecordSound(PSound & sound)
{
  return PFalse;
}


PBoolean PSoundChannelALSA::RecordFile(const PFilePath & filename)
{
  return PFalse;
}


PBoolean PSoundChannelALSA::StartRecording()
{
  return PFalse;
}


PBoolean PSoundChannelALSA::IsRecordBufferFull()
{
  return PTrue;
}


PBoolean PSoundChannelALSA::AreAllRecordBuffersFull()
{
  return PTrue;
}


PBoolean PSoundChannelALSA::WaitForRecordBufferFull()
{
  return PTrue;
}


PBoolean PSoundChannelALSA::WaitForAllRecordBuffersFull()
{
  return PFalse;
}


PBoolean PSoundChannelALSA::Abort()
{
  int r = 0;

  if (!os_handle)
    return PFalse;

  if ((r = snd_pcm_drain (os_handle)) < 0) {
    PTRACE (1, "ALSA\tCannot abort" << snd_strerror (r));
    return PFalse;
  }
  else
    return PTrue;
}



PBoolean PSoundChannelALSA::SetVolume (unsigned newVal)
{
  unsigned i = 0;

  return Volume (PTrue, newVal, i);
}


PBoolean  PSoundChannelALSA::GetVolume(unsigned &devVol)
{
  return Volume (PFalse, 0, devVol);
}
  

PBoolean PSoundChannelALSA::IsOpen () const
{
  return (os_handle != NULL);
}


PBoolean PSoundChannelALSA::Volume (PBoolean set, unsigned set_vol, unsigned &get_vol)
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

  if (!os_handle)
    return PFalse;

  if (card_nr == -2)
    card_name = "default";
  else
    card_name = "hw:" + PString (card_nr);

  //allocate simple id
  snd_mixer_selem_id_alloca (&sid);

  //sets simple-mixer index and name
  snd_mixer_selem_id_set_index (sid, 0);

  if ((err = snd_mixer_open (&handle, 0)) < 0) {
    PTRACE (1, "alsa-control: mixer open error: " << snd_strerror (err));
    return PFalse;
  }

  if ((err = snd_mixer_attach (handle, card_name)) < 0) {
    PTRACE (1, "alsa-control: mixer attach " << card_name << " error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return PFalse;
  }

  if ((err = snd_mixer_selem_register (handle, NULL, NULL)) < 0) {
    PTRACE (1, "alsa-control: mixer register error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return PFalse;
  }

  err = snd_mixer_load(handle);
  if (err < 0) {
    PTRACE (1, "alsa-control: mixer load error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return PFalse;
  }

  do {
    snd_mixer_selem_id_set_name (sid, (direction == Player)?play_mix_name[i]:rec_mix_name[i]);
    elem = snd_mixer_find_selem (handle, sid);
    i++;
  } while (!elem && ((direction == Player && play_mix_name[i] != NULL) || (direction == Recorder && rec_mix_name[i] != NULL)));

  if (!elem) {
    PTRACE (1, "alsa-control: unable to find simple control.");
    snd_mixer_close(handle);
    return PFalse;
  }

  if (set) {
    if (direction == Player) {
      
      snd_mixer_selem_get_playback_volume_range (elem, &pmin, &pmax);
      vol = (set_vol * (pmax?pmax:31)) / 100;
      snd_mixer_selem_set_playback_volume_all (elem, vol);
    }
    else {
      
      snd_mixer_selem_get_capture_volume_range (elem, &pmin, &pmax);
      vol = (set_vol * (pmax?pmax:31)) / 100;
      snd_mixer_selem_set_capture_volume_all (elem, vol);
    }
    PTRACE (4, "Set volume to " << vol);
  }
  else {

    if (direction == Player) {
      snd_mixer_selem_get_playback_volume_range (elem, &pmin, &pmax);
      snd_mixer_selem_get_playback_volume (elem, SND_MIXER_SCHN_FRONT_LEFT, &vol);
    }
    else {
      snd_mixer_selem_get_capture_volume_range (elem, &pmin, &pmax);
      snd_mixer_selem_get_capture_volume (elem, SND_MIXER_SCHN_FRONT_LEFT, &vol); 
    }
    get_vol = (vol * 100) / (pmax?pmax:31);

    PTRACE (4, "Got volume " << vol);
  }

  snd_mixer_close(handle);

  return PTrue;
}
