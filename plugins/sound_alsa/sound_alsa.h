/*
 * sound_alsa.h
 *
 * Sound driver for ALSA header files.
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
 * $Log: sound_alsa.h,v $
 * Revision 1.2  2003/11/12 03:24:15  csoutheren
 * Imported plugin code from crs_pwlib_plugin branch and combined with
 *   new plugin code from Snark of GnomeMeeting
 *
 * Revision 1.1.2.1  2003/10/28 02:55:53  dereksmithies
 * Initial release of alsa code. Thanks to Damien Sandras
 *
 *
 *
 *
 */

#include <ptlib.h>
#include <ptlib/socket.h>
#include <ptlib/devplugin.h>
 
#if !P_USE_INLINES
#include <ptlib/contain.inl>
#endif
 
#ifdef P_LINUX
#include <sys/soundcard.h>
#endif
 
#ifdef P_FREEBSD
#if P_FREEBSD >= 500000
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
#endif
 
#if defined(P_OPENBSD) || defined(P_NETBSD)
#include <soundcard.h>
#endif

#include <alsa/asoundlib.h>

class PAudioDelay : public PObject
{
  PCLASSINFO(PAudioDelay, PObject);

  public:
    PAudioDelay();
    BOOL Delay(int time);
    void Restart();
    int  GetError();

  protected:
    PTime  previousTime;
    BOOL   firstTime;
    int    error;
};

#define MIN_HEADROOM    30
#define MAX_HEADROOM    60

class SoundHandleEntry : public PObject {

  PCLASSINFO(SoundHandleEntry, PObject)

  public:
    SoundHandleEntry();

    int handle;
    int direction;

    unsigned numChannels;
    unsigned sampleRate;
    unsigned bitsPerSample;
    unsigned fragmentValue;
    BOOL isInitialised;
};

#define LOOPBACK_BUFFER_SIZE 5000
#define BYTESINBUF ((startptr<endptr)?(endptr-startptr):(LOOPBACK_BUFFER_SIZE+endptr-startptr))

class PSoundChannelALSA: public PSoundChannel
{
 public:
  PSoundChannelALSA();
  void Construct();
  PSoundChannelALSA(const PString &device,
		   PSoundChannel::Directions dir,
		   unsigned numChannels,
		   unsigned sampleRate,
		   unsigned bitsPerSample);
  ~PSoundChannelALSA();
  static PStringArray GetDeviceNames(PSoundChannel::Directions);
  static PString GetDefaultDevice(PSoundChannel::Directions);
  BOOL Open(const PString & _device,
       Directions _dir,
       unsigned _numChannels,
       unsigned _sampleRate,
       unsigned _bitsPerSample);
  BOOL Setup();
  BOOL Close();
  BOOL Write(const void * buf, PINDEX len);
  BOOL Read(void * buf, PINDEX len);
  BOOL SetFormat(unsigned numChannels,
	    unsigned sampleRate,
	    unsigned bitsPerSample);
  unsigned GetChannels() const;
  unsigned GetSampleRate() const;
  unsigned GetSampleSize() const;
  BOOL SetBuffers(PINDEX size, PINDEX count);
  BOOL GetBuffers(PINDEX & size, PINDEX & count);
  BOOL PlaySound(const PSound & sound, BOOL wait);
  BOOL PlayFile(const PFilePath & filename, BOOL wait);
  BOOL HasPlayCompleted();
  BOOL WaitForPlayCompletion();
  BOOL RecordSound(PSound & sound);
  BOOL RecordFile(const PFilePath & filename);
  BOOL StartRecording();
  BOOL IsRecordBufferFull();
  BOOL AreAllRecordBuffersFull();
  BOOL WaitForRecordBufferFull();
  BOOL WaitForAllRecordBuffersFull();
  BOOL Abort();
  BOOL SetVolume (unsigned);
  BOOL GetVolume (unsigned &);
  BOOL IsOpen() const;

 protected:
  unsigned mNumChannels;    
  unsigned mSampleRate;
  unsigned mBitsPerSample;
  unsigned actualSampleRate;
  Directions direction;
  PString device;
  BOOL isInitialised;
  
  BOOL Volume (BOOL, unsigned, unsigned &);

  snd_pcm_t *alsa_os_handle; /* Handle, different fromt the PChannel handle */
  int card_nr;
  int frame_bytes; /* Number of bytes in a frame */
  int period_size;
  int periods;
  PMutex device_mutex;
};
