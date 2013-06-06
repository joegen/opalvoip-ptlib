/*
 * portaudio.h
 *
 * PortAudio sound driver
 *
 * Portable Windows Library
 *
 * Copyright (C) 2013 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision: 28885 $
 * $Author: csoutheren $
 * $Date: 2013-01-14 18:38:08 +1100 (Mon, 14 Jan 2013) $
 */


///////////////////////////////////////////////////////////////////////////////
// PSound

#ifndef _PSOUND_PORTAUDIO
#define _PSOUND_PORTAUDIO

#include <portaudio.h>

#ifdef P_PORTMIXER
#include <portmixer.h>
#endif

class PSoundChannelPortAudio: public PSoundChannel
{
 public:
    PSoundChannelPortAudio();
    ~PSoundChannelPortAudio();
    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);
    bool Open(const Params & params);
    bool Setup();
    bool Close();
    bool IsOpen() const;
    bool Write(const void * buf, PINDEX len);
    bool Read(void * buf, PINDEX len);
    bool SetFormat(unsigned numChannels,
                   unsigned sampleRate,
                   unsigned bitsPerSample);
    unsigned GetChannels() const;
    unsigned GetSampleRate() const;
    unsigned GetSampleSize() const;
    bool SetBuffers(PINDEX size, PINDEX count);
    bool GetBuffers(PINDEX & size, PINDEX & count);
    bool PlaySound(const PSound & sound, bool wait);
    bool PlayFile(const PFilePath & filename, bool wait);
    bool HasPlayCompleted();
    bool WaitForPlayCompletion();
    bool RecordSound(PSound & sound);
    bool RecordFile(const PFilePath & filename);
    bool StartRecording();
    bool IsRecordBufferFull();
    bool AreAllRecordBuffersFull();
    bool WaitForRecordBufferFull();
    bool WaitForAllRecordBuffersFull();
    bool Abort();
    bool SetVolume(unsigned newVal);
    bool GetVolume(unsigned &devVol);
    bool SetMute(bool mute);
    bool GetMute(bool & mute);

  public:
    static PMutex & GetInitMutex();
    static bool Initialise();

    bool OpenStream(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample);

    bool StartStream();
    bool StopStream();

    // Overrides from class PChannel
    virtual PString GetName() const;
      // Return the name of the channel.
      
    PString GetErrorText(ErrorGroup group = NumErrorGroups) const;
    // Get a text form of the last error encountered.

  protected:
    PMutex               m_mutex;
    int                  m_channels;
    int                  m_sampleRate;
    int                  m_bitsPerSample;
    int                  m_bytesPerSample;
    int                  m_sampleFormat;

    const PaDeviceInfo * m_deviceInfo;
    int                  m_deviceId;
    PString              m_deviceName;
    bool                 m_started;
    bool                 m_mute;
    unsigned char *      m_muteBuffer;
    PINDEX               m_muteBufferSize;

    PaStream             * m_stream;
    PaStreamParameters   m_streamParms;

#ifdef P_PORTMIXER
    PxMixer              * m_mixer;
#endif
    int                  m_volume;

    int                  m_playBufferSize;
    int                  m_playBufferCount;
};


#endif

// End Of File ///////////////////////////////////////////////////////////////
