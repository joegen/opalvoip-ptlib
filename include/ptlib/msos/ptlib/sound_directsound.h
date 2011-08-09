/*
 * sound_directsound.h
 *
 * DirectX Sound driver implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2006-2007 Novacom, a division of IT-Optics
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
 * The Initial Developer of the Original DirectSound Code is 
 * Vincent Luba <vincent.luba@novacom.be>
 *
 * Contributor(s): Ted Szoczei, Nimajin Software Consulting
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef __DIRECTSOUND_H__
#define __DIRECTSOUND_H__


#include <ptlib.h>
#include <ptbuildopts.h>

#if defined(P_DIRECTSOUND)

#include <ptlib/sound.h>

#include <dsound.h>

#include <ptlib/msos/ptlib/pt_atl.h>

#ifdef _WIN32_WCE
#define LPDIRECTSOUND8 LPDIRECTSOUND
#define LPDIRECTSOUNDBUFFER8 LPDIRECTSOUNDBUFFER
#define LPDIRECTSOUNDCAPTURE8 LPDIRECTSOUNDCAPTURE
#define LPDIRECTSOUNDCAPTUREBUFFER8 LPDIRECTSOUNDCAPTUREBUFFER
#define DirectSoundCreate8 DirectSoundCreate
#define IID_IDirectSoundBuffer8 IID_IDirectSoundBuffer
#define DirectSoundCaptureCreate8 DirectSoundCaptureCreate
#define IID_IDirectSoundCaptureBuffer8 IID_IDirectSoundCaptureBuffer
#endif

/** Usage
    1. instantiate with audio format or instantiate and call Open
	2. SetBuffers
	3. Read/Write
	4. destroy or Close
  */

class PSoundChannelDirectSound: public PSoundChannel
{
public:
  /**@name Construction */
  //@{
  /** Initialise with no device
   */
  PSoundChannelDirectSound();

  /** Initialise and open device
    */
  PSoundChannelDirectSound(const PString &device,
			     PSoundChannel::Directions dir,
			     unsigned numChannels,
			     unsigned sampleRate,
			     unsigned bitsPerSample);

  ~PSoundChannelDirectSound();
  //@}

  /** Provides a list of detected devices human readable names
      Returns the names array of enumerated devices as PStringArray
   */
  static PStringArray GetDeviceNames(PSoundChannel::Directions);

  /** Open a device with format specifications
      Device name corresponds to Multimedia name (first 32 characters)
      Device is prepared for operation, but not activated yet (no I/O
	  buffer - call SetBuffers for that).
	  Or you can use PlaySound or PlayFile - they call SetBuffers themselves)
    */
  PBoolean Open(const PString & device,
            Directions dir,
            unsigned numChannels,
            unsigned sampleRate,
            unsigned bitsPerSample);

  PString GetName() const { return m_deviceName; }

  PBoolean IsOpen() const
  {
    return (m_direction == Player && m_audioPlaybackDevice != NULL) || m_audioCaptureDevice != NULL;
  }

  /** Stop I/O and destroy I/O buffer
   */
  PBoolean Abort();

  /** Destroy device
   */
  PBoolean Close();

  /** Change the audio format
      Resets I/O
    */
  PBoolean SetFormat(unsigned numChannels,
                 unsigned sampleRate,
                 unsigned bitsPerSample);

  unsigned GetChannels() const { return m_numChannels; }
  unsigned GetSampleRate() const { return m_sampleRate; }
  unsigned GetSampleSize() const { return m_bitsPerSample; }

  /** Configure the device's transfer buffers.
      No audio can be played or captured until after this method is set!
	  (PlaySound and PlayFile can be used though - they call here.)
      Read and write functions wait for input or space (blocking thread)
	  in increments of buffer size.
	  Best to make size the same as the len to be given to Read or Write.
      Best performance requires count of 4
      Resets I/O
    */
  PBoolean SetBuffers(PINDEX size, PINDEX count);
  PBoolean GetBuffers(PINDEX & size, PINDEX & count);

  /** Write specified number of bytes from buf to playback device
      Blocks thread until all bytes have been transferred to device
    */
  PBoolean Write(const void * buf, PINDEX len);

  /** Read specified number of bytes from capture device into buf
      Number of bytes actually read is a multiple of format frame size
	  Blocks thread until number of bytes have been received
    */
  PBoolean Read(void * buf, PINDEX len);

  /** Resets I/O, changes audio format to match sound and configures the 
      device's transfer buffers into one huge buffer, into which the entire
	  sound is loaded and started playing.
	  Returns immediately when wait is false, so you can do other stuff while
	  sound plays.
    */
  PBoolean PlaySound(const PSound & sound, PBoolean wait);

  /** Resets I/O, changes audio format to match file and reconfigures the
      device's transfer buffers. Accepts .wav files. Plays audio from file in
	  1/2 second chunks. Wait refers to waiting for completion of last chunk.
    */
  PBoolean PlayFile(const PFilePath & filename, PBoolean wait);

  /** Checks space m_availableBufferSpace for writing audio to play.
	  Returns true if space enough for one buffer as set by SetBuffers.
	  Sets 'm_availableBufferSpace' member for use by Write.
    */
  PBoolean IsPlayBufferFree();

  /** Repeatedly checks until there's space to fit buffer.
      Yields thread between checks.
	  Loop can be ended by calling Abort()
    */
  PBoolean WaitForPlayBufferFree();

  // all below are untested

  PBoolean HasPlayCompleted();
  PBoolean WaitForPlayCompletion();

  PBoolean RecordSound(PSound & sound);
  PBoolean RecordFile(const PFilePath & filename);
  PBoolean StartRecording();
  PBoolean IsRecordBufferFull();
  PBoolean AreAllRecordBuffersFull();
  PBoolean WaitForRecordBufferFull();
  PBoolean WaitForAllRecordBuffersFull();

  PBoolean SetVolume (unsigned);
  PBoolean GetVolume (unsigned &);

private:
  void Construct();

  unsigned m_numChannels;// 1=mono, 2=stereo, etc.
  unsigned m_sampleRate;
  unsigned m_bitsPerSample;
  
  PString m_deviceName;
  Directions m_direction;

  CComPtr<IDirectSoundCapture8>      m_audioCaptureDevice;
  CComPtr<IDirectSoundCaptureBuffer> m_audioCaptureBuffer;

  CComPtr<IDirectSound8>      m_audioPlaybackDevice;
  CComPtr<IDirectSoundBuffer> m_audioPlaybackBuffer;
  CComPtr<IDirectSoundBuffer> m_audioPrimaryPlaybackBuffer;
  
  PBoolean InitPlaybackBuffer();
  PBoolean InitCaptureBuffer();
  
  PBoolean SetFormat ();

  PBoolean m_isStreaming;
  PINDEX m_bufferSize;
  PINDEX m_dxBufferSize;
  PINDEX m_bufferCount;
  DWORD m_bufferByteOffset;   // byte offset from start of DX buffer to where we can write or read
  DWORD m_availableBufferSpace;  // number of bytes space available to write, or available to read

  PINDEX m_volume;

  WAVEFORMATEX m_waveFormat;        // audio format supplied to DirectSound
  HANDLE m_notificationEvent[2];  // [0]triggered by DirectSound at buffer boundaries, [1]by Close
  
  PMutex m_bufferMutex;
};

#endif // P_DIRECTSOUND

#endif  /* __DIRECTSOUND_H__ */
