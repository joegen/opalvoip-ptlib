/*
 * opensl_es.cxx
 *
 * Code for OpenSL ES sound device, currently used by Android
 *
 * Portable Toold Library
 *
 * Copyright (c) 2013 Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tool Library.
 *
 * The Initial Developer of the Original Code is Vox Lucida
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>

#include <ptlib/sound.h>

#ifdef __ANDROID__
  #include <android\api-level.h>
#endif

#include "opensles_wrap.hpp"


static const PConstString OpenSL_EL("OpenSL ES");

#define PTraceModule() "OpenSLES"

#ifdef __ANDROID__
static struct {
  const char * m_name;
  SLuint32     m_type;
} StreamTypes[] = {
  { "voice",        SL_ANDROID_STREAM_VOICE },
  { "system",       SL_ANDROID_STREAM_SYSTEM },
  { "ring",         SL_ANDROID_STREAM_RING },
  { "media",        SL_ANDROID_STREAM_MEDIA },
  { "alarm",        SL_ANDROID_STREAM_ALARM },
  { "notification", SL_ANDROID_STREAM_NOTIFICATION }
};
#endif


#if PTRACING
  static bool CheckSlError(SLresult result, const char * fn)
  {
    const char * msg;
    switch (result) {
      case SL_RESULT_SUCCESS :
        return false;

      case SL_RESULT_PRECONDITIONS_VIOLATED :
        msg = "Preconditions violated";
        break;
      case SL_RESULT_PARAMETER_INVALID :
        msg = "Parameter invalid";
        break;
      case SL_RESULT_MEMORY_FAILURE :
        msg = "Memory failure";
        break;
      case SL_RESULT_RESOURCE_ERROR :
        msg = "Resource error";
        break;
      case SL_RESULT_RESOURCE_LOST :
        msg = "Resource lost";
        break;
      case SL_RESULT_IO_ERROR :
        msg = "I/O error";
        break;
      case SL_RESULT_BUFFER_INSUFFICIENT :
        msg = "Buffer insufficient";
        break;
      case SL_RESULT_CONTENT_CORRUPTED :
        msg = "Content corrupted";
        break;
      case SL_RESULT_CONTENT_UNSUPPORTED :
        msg = "Content unsupported";
        break;
      case SL_RESULT_CONTENT_NOT_FOUND :
        msg = "Content not found";
        break;
      case SL_RESULT_PERMISSION_DENIED :
        msg = "Permission denied";
        break;
      case SL_RESULT_FEATURE_UNSUPPORTED :
        msg = "Feature unsupported";
        break;
      case SL_RESULT_INTERNAL_ERROR :
        msg = "Internal error";
        break;
      case SL_RESULT_UNKNOWN_ERROR :
        msg = "Unknown error";
        break;
      case SL_RESULT_OPERATION_ABORTED :
        msg = "Operation aborted";
        break;
      case SL_RESULT_CONTROL_LOST :
        msg = "Control lost";
        break;

      default :
        PTRACE(1, "Error " << result << " in function " << fn);
        return true;
    }

    PTRACE(1, msg << " in function " << fn);
    return true;
  }
  #define CHECK_SL_SUCCESS(fn, args) (!CheckSlError(fn args, #fn))
  #define CHECK_SL_ERROR(fn, args)     CheckSlError(fn args, #fn)
#else
  #define CHECK_SL_SUCCESS(fn, args) (fn args == SL_RESULT_SUCCESS)
  #define CHECK_SL_ERROR(fn, args)   (fn args != SL_RESULT_SUCCESS)
#endif


class PSoundChannel_OpenSL_ES : public PSoundChannel
{
    PCLASSINFO(PSoundChannel_OpenSL_ES, PSoundChannel);
  protected:
    PString m_deviceName;

    OpenSLES::Object      m_object;
    OpenSLES::Engine      m_engine;
    OpenSLES::Object      m_outputMix;
    OpenSLES::Object      m_audioOut;
    OpenSLES::Play        m_player;
    OpenSLES::Volume      m_volume;
    OpenSLES::Object      m_audioIn;
    OpenSLES::Record      m_recorder;
    OpenSLES::BufferQueue m_bufferQueue;

    SLDataFormat_PCM m_format_pcm;

    std::vector< std::vector<SLuint16> > m_buffers;
    size_t     m_bufferPos;
    size_t     m_bufferLen;
    PSemaphore m_bufferReady;
    PMutex     m_bufferMutex;


    bool EnqueueBuffer(PINDEX index)
    {
      const void * ptr = &m_buffers[index][0];
      size_t count = m_buffers[index].size();
      PAssert(count > 0, PLogicError);
      PTRACE(5, "Queuing: index=" << index << ", ptr=" << ptr << ", bytes=" << count);
      return CHECK_SL_SUCCESS(m_bufferQueue.Enqueue, (ptr, count));
    }

    static void PlayBufferCallback(OpenSLES::BufferQueue::ItfType, void * context)
    {
      reinterpret_cast<PSoundChannel_OpenSL_ES *>(context)->PlayBufferCallback();
    }

    void PlayBufferCallback()
    {
      PTRACE(5, "Play buffer callback");
      m_bufferReady.Signal();
    }


    static void RecordBufferCallback(OpenSLES::BufferQueue::ItfType, void * context)
    {
      reinterpret_cast<PSoundChannel_OpenSL_ES *>(context)->RecordBufferCallback();
    }

    void RecordBufferCallback()
    {
      m_bufferMutex.Wait();

      EnqueueBuffer((m_bufferPos+m_bufferLen)%m_buffers.size());

      // Check for overflow
      if (m_bufferLen < m_buffers.size()) {
        ++m_bufferLen;
        m_bufferReady.Signal();
      }

      PTRACE(5, "Record buffer callback: m_bufferPos=" << m_bufferPos << ", m_bufferLen=" << m_bufferLen);

      m_bufferMutex.Signal();
    }

  public:
    PSoundChannel_OpenSL_ES()
      : m_bufferReady(0, UINT_MAX)
    {
      SetBuffers(320, 2);
      SetFormat(1, 8000, 16);
    }


    PSoundChannel_OpenSL_ES(
      const PString &device,
      PSoundChannel::Directions dir,
      unsigned numChannels,
      unsigned sampleRate,
      unsigned bitsPerSample
    )
      : m_bufferReady(0, UINT_MAX)
    {
      SetBuffers(320, 2);
      Open(device, dir, numChannels, sampleRate, bitsPerSample);
    }


    ~PSoundChannel_OpenSL_ES()
    {
      Close();
    }


    static PStringArray GetDeviceNames(PSoundChannel::Directions dir)
    {
      PStringArray devices;

      switch (dir) {
        case Player :
#ifdef __ANDROID__
          for (PINDEX i = 0; i < PARRAYSIZE(StreamTypes); ++i)
            devices += StreamTypes[i].m_name;
#else
          devices += "speaker";
#endif
          break;

        case Recorder :
          devices += "microphone";
          break;

        default :
          break;
      }

      return devices;
    }


    PBoolean Open(const PString & deviceName,
                  Directions dir,
                  unsigned numChannels,
                  unsigned sampleRate,
                  unsigned bitsPerSample)
    {
      Close();

      PTRACE(4, "Open(" << deviceName << ',' << dir << ',' << numChannels << ',' << sampleRate << ',' << bitsPerSample << ')');

      activeDirection = dir;

      SetFormat(numChannels, sampleRate, bitsPerSample);

      if (CHECK_SL_ERROR(slCreateEngine, (m_object.GetPtr(), 0, NULL, 0, NULL, NULL)))
        return false;

      if (CHECK_SL_ERROR(m_object.Realize, (SL_BOOLEAN_FALSE)))
        return false;

      if (CHECK_SL_ERROR(m_engine.Create, (m_object)))
        return false;

      switch (dir) {
        case Player :
          if (CHECK_SL_ERROR(m_engine.CreateOutputMix, (m_outputMix, OpenSLES::Interfaces())))
            return false;

          if (CHECK_SL_ERROR(m_outputMix.Realize, (SL_BOOLEAN_FALSE)))
            return false;

          {
            OpenSLES::BufferQueue::Locator bufferLocation(m_buffers.size());
PTRACE(1, "bufferLocation.numBuffers=" << bufferLocation.numBuffers);
            SLDataSource source = { &bufferLocation, &m_format_pcm };

            SLDataLocator_OutputMix mixerLocation = { SL_DATALOCATOR_OUTPUTMIX, m_outputMix };
            SLDataSink sink = { &mixerLocation, NULL };

            OpenSLES::Interfaces playIfs;
            playIfs.Add<OpenSLES::BufferQueue>().Add<OpenSLES::Volume>(SL_BOOLEAN_FALSE);
#ifdef __ANDROID__
            playIfs.Add<OpenSLES::AndroidConfiguration>();
#endif
            if (CHECK_SL_ERROR(m_engine.CreateAudioPlayer,(m_audioOut, source, sink, playIfs)))
              return false;
          }

#ifdef __ANDROID__
          {
            OpenSLES::AndroidConfiguration androidConfig;
            if (CHECK_SL_SUCCESS(androidConfig.Create, (m_audioOut))) {
              for (PINDEX i = 0; i < PARRAYSIZE(StreamTypes); ++i) {
                if (deviceName *= StreamTypes[i].m_name) {
                  CHECK_SL_ERROR(androidConfig.SetStreamType, (StreamTypes[i].m_type));
                  break;
                }
              }
            }
          }
#endif

          if (CHECK_SL_ERROR(m_audioOut.Realize, (SL_BOOLEAN_FALSE)))
            return false;

          if (CHECK_SL_ERROR(m_bufferQueue.Create, (m_audioOut)))
            return false;

          if (CHECK_SL_ERROR(m_bufferQueue.RegisterCallback, (&PSoundChannel_OpenSL_ES::PlayBufferCallback, this)))
            return false;

          if (CHECK_SL_ERROR(m_player.Create, (m_audioOut)))
            return false;

          m_volume.Create(m_audioOut);
          break;

        case Recorder :
          {
            SLDataLocator_IODevice deviceLocation = {
              SL_DATALOCATOR_IODEVICE,
              SL_IODEVICE_AUDIOINPUT,
              SL_DEFAULTDEVICEID_AUDIOINPUT,
              NULL
            };
            SLDataSource source = { &deviceLocation, NULL};

            OpenSLES::BufferQueue::Locator bufferLocation(m_buffers.size());
            SLDataSink sink = { &bufferLocation, &m_format_pcm };

            OpenSLES::Interfaces recIfs;
            recIfs.Add<OpenSLES::BufferQueue>().Add<OpenSLES::Volume>(SL_BOOLEAN_FALSE);
#ifdef __ANDROID__
            recIfs.Add<OpenSLES::AndroidConfiguration>();
#endif
            if (CHECK_SL_ERROR(m_engine.CreateAudioRecorder,(m_audioIn, source, sink, recIfs)))
              return false;
          }

#ifdef __ANDROID__
          {
            OpenSLES::AndroidConfiguration androidConfig;
            #if __ANDROID_API__ >= 14
              #define MY_ANDROID_RECORDING_STREAM SL_ANDROID_RECORDING_PRESET_VOICE_COMMUNICATION
            #else
              #define MY_ANDROID_RECORDING_STREAM SL_ANDROID_RECORDING_PRESET_GENERIC
            #endif
            if (CHECK_SL_SUCCESS(androidConfig.Create, (m_audioIn)))
              CHECK_SL_ERROR(androidConfig.SetRecordinPreset, (MY_ANDROID_RECORDING_STREAM));
          }
#endif

          if (CHECK_SL_ERROR(m_audioIn.Realize, (SL_BOOLEAN_FALSE)))
            return false;

          if (CHECK_SL_ERROR(m_recorder.Create, (m_audioIn)))
            return false;

          if (CHECK_SL_ERROR(m_bufferQueue.Create, (m_audioIn)))
            return false;

          if (CHECK_SL_ERROR(m_bufferQueue.RegisterCallback, (&PSoundChannel_OpenSL_ES::RecordBufferCallback, this)))
            return false;

          m_volume.Create(m_audioIn);
          break;

        default :
          PAssertAlways(PInvalidParameter);
          return false;
      }

      m_deviceName = deviceName;
      PTRACE(3, "Opened \"" << deviceName << '"');
      os_handle = 1;
      return true;
    }


    virtual PString GetName() const
    {
      return m_deviceName;
    }


    PBoolean IsOpen() const
    {
      return m_bufferQueue.IsValid();
    }


    PBoolean Close()
    {
      PTRACE_IF(4, IsOpen(), "Closing \"" << GetName() << '"');

      Abort();

      m_bufferQueue.Destroy();
      m_recorder.Destroy();
      m_audioIn.Destroy();
      m_player.Destroy();
      m_audioOut.Destroy();
      m_outputMix.Destroy();
      m_engine.Destroy();
      m_object.Destroy();

      os_handle = -1;
      return true;
    }


    PBoolean Abort()
    {
      if (!IsOpen())
        return false;

      bool ok =true;

      switch (activeDirection) {
        case Player :
          if (CHECK_SL_ERROR(m_player.SetPlayState, (SL_PLAYSTATE_STOPPED)))
            ok = false;
          break;

        case Recorder :
          if (CHECK_SL_ERROR(m_recorder.SetRecordState, (SL_RECORDSTATE_STOPPED)))
            ok = false;
          break;

        default :
          return true;
      }

      if (CHECK_SL_ERROR(m_bufferQueue.Clear,()))
        ok = false;

      // Break any read/write block
      m_bufferReady.Signal();
      PTRACE(5, activeDirection << " aborted: ok=" << ok);
      return ok;
    }


    PBoolean Write(const void * buf, PINDEX len)
    {
      if (!IsOpen())
        return false;

      SLuint32 state;
      if (CHECK_SL_ERROR(m_player.GetPlayState, (&state)))
        return false;

      // See if first time in
      if (state != SL_PLAYSTATE_PLAYING) {
        PTRACE(5, "Starting playback");
        // First time, start it up
        if (CHECK_SL_ERROR(m_player.SetPlayState, (SL_PLAYSTATE_PLAYING)))
          return false;

        m_bufferReady.Reset(m_buffers.size(), m_buffers.size());
      }

      const uint8_t * ptr = (const uint8_t *)buf;
      while (len > 0) {
        PTRACE(5, "Awaiting play buffer ready");
        m_bufferReady.Wait();
        if (!IsOpen())
          return false;

        PINDEX chunkSize = std::min(len, (PINDEX)m_buffers[m_bufferPos].size());
        m_buffers[m_bufferPos].resize(chunkSize);
        memcpy(&m_buffers[m_bufferPos][0], ptr, chunkSize);
        EnqueueBuffer(m_bufferPos);
        m_bufferPos = (m_bufferPos+1)%m_buffers.size();

        lastWriteCount += chunkSize;
        ptr += chunkSize;
        len -= chunkSize;
      }

      return true;
    }


    PBoolean HasPlayCompleted()
    {
      return m_bufferLen == 0;
    }


    PBoolean WaitForPlayCompletion()
    {
      while (!HasPlayCompleted()) {
        PTRACE(5, "Awaiting buffer ready for completion");
        if (!m_bufferReady.Wait(m_buffers[0].size()))
          return false;
      }

      return true;
    }


    PBoolean Read(void * buf, PINDEX len)
    {
      if (!StartRecording())
        return false;

      while (m_bufferLen == 0) {
        PTRACE(5, "Awaiting record buffer ready");
        m_bufferReady.Wait();
        if (!IsOpen())
          return false;
      }

      m_bufferMutex.Wait();

      lastReadCount = std::min((size_t)len, m_buffers[m_bufferPos].size());
      memcpy(buf, &m_buffers[m_bufferPos][0], lastReadCount);

      m_bufferPos = (m_bufferPos+1)%m_buffers.size();
      --m_bufferLen;

      m_bufferMutex.Signal();

      PTRACE(5, "Read " << lastReadCount << " bytes.");
      return true;
    }


    PBoolean StartRecording()
    {
      if (!IsOpen())
        return false;

      SLuint32 state;
      if (CHECK_SL_ERROR(m_recorder.GetRecordState, (&state)))
        return false;

      if (state == SL_RECORDSTATE_RECORDING)
        return true;

      PWaitAndSignal mutex(m_bufferMutex);

      m_bufferReady.Reset(0, m_buffers.size());

      m_bufferPos = m_bufferLen = 0;

      for (size_t i = 0; i < m_buffers.size(); ++i) {
        if (!EnqueueBuffer(i))
          return false;
      }

      if (CHECK_SL_ERROR(m_recorder.SetRecordState, (SL_RECORDSTATE_RECORDING)))
        return false;

      PTRACE(5, "Started record state");
      return true;
    }


    PBoolean SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
    {
      bool notOpen = !IsOpen();

      Close();

      m_format_pcm.formatType = SL_DATAFORMAT_PCM;
      m_format_pcm.numChannels = numChannels;
      m_format_pcm.samplesPerSec = sampleRate*1000; // yeah, member name is bogus, is really milliHertz
      m_format_pcm.bitsPerSample = bitsPerSample;
      m_format_pcm.containerSize = bitsPerSample;
      m_format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
      m_format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;

      return notOpen || Open(GetName(), GetDirection(), GetChannels(), GetSampleRate(), GetSampleSize());
    }


    unsigned GetChannels() const
    {
      return m_format_pcm.numChannels;
    }


    unsigned GetSampleRate() const
    {
      return m_format_pcm.samplesPerSec/1000; // yeah, member name is bogus, is really milliHertz
    }


    unsigned GetSampleSize() const
    {
      return m_format_pcm.bitsPerSample;
    }


    PBoolean SetBuffers(PINDEX size, PINDEX count)
    {
      bool notOpen = !IsOpen();

      Close();

      m_buffers.resize(count);
      for (size_t i = 0; i < m_buffers.size(); ++i)
        m_buffers[i].resize(size);

      m_bufferPos = m_bufferLen = 0;

      return notOpen || Open(GetName(), GetDirection(), GetChannels(), GetSampleRate(), GetSampleSize());
    }


    PBoolean GetBuffers(PINDEX & size, PINDEX & count)
    {
      size = m_buffers[0].size();
      count = m_buffers.size();
      return true;
    }


    PBoolean SetVolume(unsigned newVolume)
    {
      if (!m_volume.IsValid())
        return false;

      SLmillibel maxVol;
      if (CHECK_SL_ERROR(m_volume.GetMaxVolumeLevel, (&maxVol)))
        return false;

      return CHECK_SL_SUCCESS(m_volume.SetVolumeLevel, ((SLmillibel)((int)newVolume*(maxVol - SL_MILLIBEL_MIN)/100 + SL_MILLIBEL_MIN)));
    }


    PBoolean GetVolume(unsigned & oldVolume)
    {
      if (!m_volume.IsValid())
        return false;

      SLmillibel maxVol;
      if (CHECK_SL_ERROR(m_volume.GetMaxVolumeLevel, (&maxVol)))
        return false;

      SLmillibel currentVol;
      if (CHECK_SL_ERROR(m_volume.GetVolumeLevel, (&currentVol)))
        return false;

      oldVolume = (currentVol - SL_MILLIBEL_MIN)*100U/(maxVol - SL_MILLIBEL_MIN);
      return true;
    }


    bool SetMute(bool newMute)
    {
      if (!m_volume.IsValid())
        return false;

      return CHECK_SL_SUCCESS(m_volume.SetMute, (newMute ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE));
    }


    bool GetMute(bool & oldMute)
    {
      if (!m_volume.IsValid())
        return false;

      SLboolean currentMute;
      if (CHECK_SL_ERROR(m_volume.GetMute, (&currentMute)))
        return false;

      oldMute = currentMute != SL_BOOLEAN_FALSE;
      return true;
    }
};


PCREATE_SOUND_PLUGIN(OpenSL_ES, PSoundChannel_OpenSL_ES)

// End of file
