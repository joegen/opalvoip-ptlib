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
 * $Log: sound_alsa.cxx,v $
 * Revision 1.11  2003/12/18 11:16:41  dominance
 * Removed the ALSA Abort completely upon Damien's request ;)
 *
 * Revision 1.10  2003/12/18 10:38:55  dominance
 * Removed ALSA Abort as it segfaults in various circumstances.
 * Fix proposed by Damien Sandras <dsandras@seconix.com>.
 *
 * Revision 1.9  2003/12/09 22:47:10  dsandras
 * Use less aggressive Abort.
 *
 * Revision 1.8  2003/12/03 21:48:21  dsandras
 * Better handling of buffer sizes. Removed unuseful code.
 *
 * Revision 1.7  2003/11/25 20:13:48  dsandras
 * Added #pragma.
 *
 * Revision 1.6  2003/11/25 09:58:01  dsandras
 * Removed Abort call from PlaySound ().
 *
 * Revision 1.5  2003/11/25 09:52:07  dsandras
 * Modified WaitForPlayCompletion so that it uses snd_pcm_drain instead of active waiting.
 *
 * Revision 1.4  2003/11/23 22:09:57  dsandras
 * Removed unuseful stuff and added implementation for functions permitting to play a file or a PSound.
 *
 * Revision 1.3  2003/11/14 05:28:47  csoutheren
 * Updated for new plugin code thanks to Damien and Snark
 *
 */

#pragma implementation "sound_alsa.h"

#include "sound_alsa.h"


PCREATE_SOUND_PLUGIN(ALSA, PSoundChannelALSA)


static PStringArray playback_devices;
static PStringArray capture_devices;

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
  frame_bytes = 0;
  period_size = 0;
  periods = 0;
  card_nr = 0;
  os_handle = NULL;
}


PSoundChannelALSA::~PSoundChannelALSA()
{
  Close();
}


PStringArray PSoundChannelALSA::GetDeviceNames (Directions dir)
{
  int card = -1, dev = -1;
  
  snd_ctl_t *handle = NULL;
  snd_ctl_card_info_t *info = NULL;
  snd_pcm_info_t *pcminfo = NULL;
  snd_pcm_stream_t stream;

  char *name = NULL;
  char card_id [32];

  if (dir == Recorder) {

    stream = SND_PCM_STREAM_CAPTURE;
    capture_devices = PStringArray ();
  }
  else {

    stream = SND_PCM_STREAM_PLAYBACK;
    playback_devices = PStringArray ();
  }

  snd_ctl_card_info_alloca (&info);
  snd_pcm_info_alloca (&pcminfo);

  /* No sound card found */
  if (snd_card_next (&card) < 0 || card < 0) {

    return PStringArray ();
  }


  while (card >= 0) {

    snprintf (card_id, 32, "hw:%d", card);
    
    snd_ctl_open (&handle, card_id, 0);
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
	if (dir == Recorder) {

	  if (capture_devices.GetStringsIndex (name) == P_MAX_INDEX)
	    capture_devices.AppendString (name);
	}
	else {

	  if (playback_devices.GetStringsIndex (name) == P_MAX_INDEX)
	    playback_devices.AppendString (name);
	}
	    
	free (name);
      }
    }


    snd_ctl_close(handle);
    snd_card_next (&card);
  }


  if (dir == Recorder)
    return capture_devices;
  else 
    return playback_devices;
}


PString PSoundChannelALSA::GetDefaultDevice(Directions dir)
{
  PStringArray devicenames;
  devicenames = PSoundChannelALSA::GetDeviceNames (dir);

  return devicenames[0];
}


BOOL PSoundChannelALSA::Open (const PString & _device,
                              Directions _dir,
			      unsigned _numChannels,
			      unsigned _sampleRate,
			      unsigned _bitsPerSample)
{
  PString real_device_name;
  PINDEX i = 0;
  snd_pcm_stream_t stream;

  Close();

  os_handle = NULL;

  if (_dir == Recorder)
    stream = SND_PCM_STREAM_CAPTURE;
  else
    stream = SND_PCM_STREAM_PLAYBACK;

  /* Open in NONBLOCK mode */
  if ((i = (_dir == Recorder) ? capture_devices.GetStringsIndex (_device) : playback_devices.GetStringsIndex (_device)) != P_MAX_INDEX) {

    real_device_name = "plughw:" + PString (i);
    card_nr = i;
  }
  else {
    
    PTRACE (1, "ALSA\tDevice unavailable");
    return FALSE;
  }

  if (snd_pcm_open (&os_handle, real_device_name, stream, SND_PCM_NONBLOCK) < 0) {

    PTRACE (1, "ALSA\tOpen Failed");
    return FALSE;
  }
  else 
    snd_pcm_nonblock (os_handle, 0);
   
  /* save internal parameters */
  direction = _dir;
  device = real_device_name;
  mNumChannels = _numChannels;
  mSampleRate = _sampleRate;
  mBitsPerSample = _bitsPerSample;
  isInitialised = FALSE;

  PTRACE (1, "ALSA\tDevice " << real_device_name << " Opened");

  return TRUE;
}


BOOL PSoundChannelALSA::Setup()
{
  snd_pcm_hw_params_t *hw_params = NULL;
  
  int err = 0;
  enum _snd_pcm_format val = SND_PCM_FORMAT_UNKNOWN;
  BOOL no_error = TRUE;


  if (os_handle == NULL) {

    PTRACE(6, "ALSA\tSkipping setup of " << device << " as not open");
    return FALSE;
  }

  if (isInitialised) {

    PTRACE(6, "ALSA\tSkipping setup of " << device << " as instance already initialised");
    return TRUE;
  }


#if PBYTE_ORDER == PLITTLE_ENDIAN
  val = (mBitsPerSample == 16) ? SND_PCM_FORMAT_S16_LE : SND_PCM_FORMAT_U8;
#else
  val = (mbitsPerSample == 16) ? SND_PCM_FORMAT_S16_BE : SND_PCM_FORMAT_U8;
#endif

  frame_bytes = (mNumChannels * (snd_pcm_format_width (val) / 8));

  snd_pcm_hw_params_alloca (&hw_params);

  if ((err = snd_pcm_hw_params_any (os_handle, hw_params)) < 0) {

    PTRACE (1, "ALSA\tCannot initialize hardware parameter structure " <<
	    snd_strerror (err));
    no_error = FALSE;
  }


  if ((err = snd_pcm_hw_params_set_access (os_handle, hw_params, 
				    SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {

    PTRACE (1, "ALSA\tCannot set access type " <<
	    snd_strerror (err));
    no_error = FALSE;
  }


  if ((err = snd_pcm_hw_params_set_format (os_handle, hw_params, val)) < 0) {

    PTRACE (1, "ALSA\tCannot set sample format " <<
             snd_strerror (err));
    no_error = FALSE;
  }


  if ((err = snd_pcm_hw_params_set_rate (os_handle, hw_params, 
					 mSampleRate, 0)) < 0) {

    PTRACE (1, "ALSA\tCannot set sample rate " <<
	    snd_strerror (err));
    no_error = FALSE;
  }


  if ((err = snd_pcm_hw_params_set_channels (os_handle, hw_params, 
					     mNumChannels)) < 0) {

    PTRACE (1, "ALSA\tCannot set channel count " <<
             snd_strerror (err));
    no_error = FALSE;
  }


  // Ignore errors here 
  if (periods && period_size) {

    if ((err = snd_pcm_hw_params_set_period_size_near (os_handle, 
						       hw_params, 
						       period_size, 
						       0)) < 0)
      PTRACE (1, "ALSA\tCannot set period size " <<
	      snd_strerror (err));
    
    if ((err = snd_pcm_hw_params_set_periods (os_handle, 
					      hw_params, 
					      periods, 0)) < 0)
      PTRACE (1, "ALSA\tCannot set number of periods " <<
	      snd_strerror (err));
        
    if ((err = snd_pcm_hw_params_set_buffer_size_near (os_handle, 
						       hw_params, 
						       periods*period_size/frame_bytes)) < 0)
      PTRACE (1, "ALSA\tCannot set buffer size " <<
	      snd_strerror (err));
  }


  snd_pcm_hw_params_get_period_size(hw_params, &period_size);
    
  
  if ((err = snd_pcm_hw_params (os_handle, hw_params)) < 0) {

    PTRACE (1, "ALSA\tCannot set parameters " <<
	    snd_strerror (err));
    no_error = FALSE;
  }

  
  isInitialised = TRUE;

  return no_error;
}


BOOL PSoundChannelALSA::Close()
{
  PWaitAndSignal m(device_mutex);

  /* if the channel isn't open, do nothing */
  if (!os_handle)
    return FALSE;

  snd_pcm_close (os_handle);
  os_handle = NULL;

  return TRUE;
}


BOOL PSoundChannelALSA::Write (const void *buf, PINDEX len)
{
  long r = 0;
  char *buf2 = (char *) buf;
  int pos = 0, max_try = 0;

  PWaitAndSignal m(device_mutex);

  if (!isInitialised && !Setup() || !len || !os_handle)
    return FALSE;

  do {

      
    /* the number of frames to read is the buffer length 
       divided by the size of one frame */
    r = snd_pcm_writei (os_handle, (char *) &buf2 [pos], len / frame_bytes);

    if (r > 0) {

      pos += r * frame_bytes;
      len -= r * frame_bytes;
    }
    else {

      if (r == -EPIPE) {    /* under-run */

	r = snd_pcm_prepare (os_handle);
	if (r < 0)
	  PTRACE (1, "ALSA\tCould not prepare device: " << snd_strerror (r));

      } else if (r == -ESTRPIPE) {

	while ((r = snd_pcm_resume (os_handle)) == -EAGAIN)
	  sleep(1);       /* wait until the suspend flag is released */
      
	if (r < 0) 
	  snd_pcm_prepare (os_handle);
      }

      PTRACE (1, "ALSA\tCould not write "
	      << max_try << " " << len << " " << r);
      max_try++;
    }
    
  } while (len > 0 && max_try < 5);


  return TRUE;
}


BOOL PSoundChannelALSA::Read (void * buf, PINDEX len)
{
  long r = 0;

  char *buf2 = (char *) buf;
  int pos = 0, max_try = 0;

  lastReadCount = 0;

  PWaitAndSignal m(device_mutex);

  if (!isInitialised && !Setup() || !len || !os_handle)
    return FALSE;

  memset ((char *) buf, 0, len);
  
  do {

    /* the number of frames to read is the buffer length 
       divided by the size of one frame */
    r = snd_pcm_readi (os_handle, (char *) &buf2 [pos], len / frame_bytes);

    if (r > 0) {

      pos += r * frame_bytes;
      len -= r * frame_bytes;
      lastReadCount += r * frame_bytes;
    }
    else {

      if (r == -EPIPE) {    /* under-run */

	snd_pcm_prepare (os_handle);
      
      } else if (r == -ESTRPIPE) {

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

  return TRUE;
}


BOOL PSoundChannelALSA::SetFormat (unsigned numChannels,
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
  isInitialised = FALSE;

  return TRUE;
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


BOOL PSoundChannelALSA::SetBuffers (PINDEX size, PINDEX count)
{
  periods = count;
  period_size = size / (frame_bytes ? frame_bytes : 2);

  return TRUE;
}


BOOL PSoundChannelALSA::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = period_size * (frame_bytes ? frame_bytes : 2);
  count = periods;
  
  return FALSE;
}


BOOL PSoundChannelALSA::PlaySound(const PSound & sound, BOOL wait)
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return FALSE;

  if (wait)
    return WaitForPlayCompletion();

  return TRUE;
}


BOOL PSoundChannelALSA::PlayFile(const PFilePath & filename, BOOL wait)
{
  BYTE buffer [512];
  
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  PFile file (filename, PFile::ReadOnly);

  if (!file.IsOpen())
    return FALSE;

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

  return TRUE;
}


BOOL PSoundChannelALSA::HasPlayCompleted()
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  return (snd_pcm_state (os_handle) != SND_PCM_STATE_RUNNING);
}


BOOL PSoundChannelALSA::WaitForPlayCompletion()
{
  if (!os_handle)
    return SetErrorValues(NotOpen, EBADF);

  snd_pcm_drain (os_handle);

  return TRUE;
}


BOOL PSoundChannelALSA::RecordSound(PSound & sound)
{
  return FALSE;
}


BOOL PSoundChannelALSA::RecordFile(const PFilePath & filename)
{
  return FALSE;
}


BOOL PSoundChannelALSA::StartRecording()
{
  return FALSE;
}


BOOL PSoundChannelALSA::IsRecordBufferFull()
{
  return TRUE;
}


BOOL PSoundChannelALSA::AreAllRecordBuffersFull()
{
  return TRUE;
}


BOOL PSoundChannelALSA::WaitForRecordBufferFull()
{
  return TRUE;
}


BOOL PSoundChannelALSA::WaitForAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannelALSA::Abort()
{
  int r = 0;

  if (!os_handle)
    return FALSE;

  if ((r = snd_pcm_drain (os_handle)) < 0) {

    PTRACE (1, "ALSA\tCannot abort" <<
	    snd_strerror (r));
    return FALSE;
  }
  else
    return TRUE;
}



BOOL PSoundChannelALSA::SetVolume (unsigned newVal)
{
  unsigned i = 0;

  return Volume (TRUE, newVal, i);
}


BOOL  PSoundChannelALSA::GetVolume(unsigned &devVol)
{
  return Volume (FALSE, 0, devVol);
}
  

BOOL PSoundChannelALSA::IsOpen () const
{
  return (os_handle != NULL);
}


BOOL PSoundChannelALSA::Volume (BOOL set, unsigned set_vol, unsigned &get_vol)
{
  int err = 0;
  snd_mixer_t *handle;
  snd_mixer_elem_t *elem;
  snd_mixer_selem_id_t *sid;

  const char *play_mix_name = (direction == Player) ? "PCM": "Mic";
  PString card_name;

  long pmin = 0, pmax = 0;
  long int vol = 0;

  if (!os_handle)
    return FALSE;

  card_name = "hw:" + PString (card_nr);

  //allocate simple id
  snd_mixer_selem_id_alloca (&sid);

  //sets simple-mixer index and name
  snd_mixer_selem_id_set_index (sid, 0);
  snd_mixer_selem_id_set_name (sid, play_mix_name);

  if ((err = snd_mixer_open (&handle, 0)) < 0) {

    PTRACE (1, "alsa-control: mixer open error: " << snd_strerror (err));
    return FALSE;
  }


  if ((err = snd_mixer_attach (handle, card_name)) < 0) {

    PTRACE (1, "alsa-control: mixer attach " << card_name << " error: " 
	    << snd_strerror(err));
    snd_mixer_close(handle);
    return FALSE;
  }


  if ((err = snd_mixer_selem_register (handle, NULL, NULL)) < 0) {

    PTRACE (1, "alsa-control: mixer register error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return FALSE;
  }


  err = snd_mixer_load(handle);
  if (err < 0) {

    PTRACE (1, "alsa-control: mixer load error: " << snd_strerror(err));
    snd_mixer_close(handle);
    return FALSE;
  }

  elem = snd_mixer_find_selem (handle, sid);

  if (!elem) {

    PTRACE (1, "alsa-control: unable to find simple control "
	    << snd_mixer_selem_id_get_name(sid) << "," 
	    << snd_mixer_selem_id_get_index(sid));
    snd_mixer_close(handle);
    return FALSE;
  }

  snd_mixer_selem_get_playback_volume_range (elem, &pmin, &pmax);

  if (set) {

    vol = (set_vol * (pmax?pmax:31)) / 100;
    snd_mixer_selem_set_playback_volume (elem, 
					 SND_MIXER_SCHN_FRONT_LEFT, vol);
    snd_mixer_selem_set_playback_volume (elem, 
					 SND_MIXER_SCHN_FRONT_RIGHT, vol);
    
    PTRACE (4, "Set volume to " << vol);
  }
  else {

    snd_mixer_selem_get_playback_volume (elem, 
					 SND_MIXER_SCHN_FRONT_LEFT, &vol);
    get_vol = (vol * 100) / (pmax?pmax:31);
    PTRACE (4, "Got volume " << vol);
  }

  snd_mixer_close(handle);

  return TRUE;
}
