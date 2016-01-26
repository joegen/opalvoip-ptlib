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

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>
#include <ptclib/qchannel.h>

#ifdef __ANDROID__
  #include <android\api-level.h>
#endif

#include "opensles_wrap.hpp"


static const PConstString OpenSL_EL("OpenSL ES");

#define PTraceModule() "OpenSLES"
#define PTRACE_DETAILED(...) PTRACE(__VA_ARGS__)


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
    PString               m_deviceName;
    OpenSLES::Object      m_object;
    OpenSLES::Engine      m_engine;
    OpenSLES::Object      m_outputMix;
    OpenSLES::Object      m_audioOut;
    OpenSLES::Play        m_player;
    OpenSLES::Volume      m_volume;
    OpenSLES::Object      m_audioIn;
    OpenSLES::Record      m_recorder;
    OpenSLES::BufferQueue m_bufferQueue;
    SLDataFormat_PCM      m_format_pcm;
    PQueueChannel         m_queue;
    std::vector<uint8_t>  m_buffers[2];
    size_t                m_bufferPos;


    bool EnqueueBuffer()
    {
      const void * ptr = m_buffers[m_bufferPos].data();
      size_t count = m_buffers[m_bufferPos].size();
      PAssert(count > 0, PLogicError);
      if (CHECK_SL_ERROR(m_bufferQueue.Enqueue, (ptr, count)))
        return false;

      m_bufferPos = (m_bufferPos+1)%PARRAYSIZE(m_buffers);
      return true;
    }

    static void PlayBufferCallback(OpenSLES::BufferQueue::ItfType, void * context)
    {
      reinterpret_cast<PSoundChannel_OpenSL_ES *>(context)->PlayBufferCallback();
    }

    void PlayBufferCallback()
    {
      PINDEX bytes = m_buffers[m_bufferPos].size();
      uint8_t * data = m_buffers[m_bufferPos].data();
      m_queue.Read(data, bytes);
      PINDEX count = m_queue.GetLastReadCount();
      if (count < bytes) {
        PTRACE_IF(4, m_queue.IsOpen(), "PlayerCallback underflow: count=" << count << ", needed=" << bytes);
        memset(data+count, 0, bytes-count);
      }
      else {
        PTRACE_DETAILED(5, "PlayerCallback: count=" << count << ", bufferPos=" << m_bufferPos);
      }

      EnqueueBuffer();
    }


    static void RecordBufferCallback(OpenSLES::BufferQueue::ItfType, void * context)
    {
      reinterpret_cast<PSoundChannel_OpenSL_ES *>(context)->RecordBufferCallback();
    }

    void RecordBufferCallback()
    {
      PTRACE_DETAILED(5, "RecordCallback: size=" << m_buffers[m_bufferPos].size() << ", bufferPos=" << m_bufferPos);

      if (!m_queue.Write(m_buffers[m_bufferPos].data(), m_buffers[m_bufferPos].size())) {
        PTRACE_IF(4, m_queue.IsOpen(), "Overflow, queue full");
      }

      EnqueueBuffer();
    }

  
    bool InternalOpen()
    {
      PTRACE(5, "Opening " << activeDirection<< " \"" << m_deviceName << '"');

      if (CHECK_SL_ERROR(slCreateEngine, (m_object.GetPtr(), 0, NULL, 0, NULL, NULL)))
        return false;
      
      if (CHECK_SL_ERROR(m_object.Realize, (SL_BOOLEAN_FALSE)))
        return false;
      
      if (CHECK_SL_ERROR(m_engine.Create, (m_object)))
        return false;
      
      switch (activeDirection) {
        case Player :
          if (CHECK_SL_ERROR(m_engine.CreateOutputMix, (m_outputMix, OpenSLES::Interfaces())))
            return false;
          
          if (CHECK_SL_ERROR(m_outputMix.Realize, (SL_BOOLEAN_FALSE)))
            return false;
          
          {
            OpenSLES::BufferQueue::Locator bufferLocation(PARRAYSIZE(m_buffers));
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
                if (m_deviceName *= StreamTypes[i].m_name) {
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

          // Don't block the call back eading from queue
          m_queue.SetReadTimeout(0);

          // double the time for a buffer to be processed, minimum 1 second
          SetWriteTimeout(PTimeInterval(std::max(1000U, m_buffers[0].size()/2*1000/GetSampleRate())));
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
            
            OpenSLES::BufferQueue::Locator bufferLocation(PARRAYSIZE(m_buffers));
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

          // Make sure queue does not block the callback
          m_queue.SetWriteTimeout(0);

          // double the time for a buffer to be processed, minimum 1 second
          SetReadTimeout(PTimeInterval(std::max(1000U, m_buffers[0].size()/2*1000/GetSampleRate())));
          break;

        default :
          PAssertAlways(PInvalidParameter);
          return false;
      }

      PTRACE(3, "Opened " << activeDirection<< " \"" << m_deviceName << '"');
      os_handle = 1;
      return true;
    }
  
  
  public:
    PSoundChannel_OpenSL_ES()
    {
      InternalSetBuffers(320, 2);
      InternalSetFormat(1, 8000, 16);
      PIndirectChannel::Open(m_queue);
    }


    ~PSoundChannel_OpenSL_ES()
    {
      Close();
      PIndirectChannel::Close();
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


    virtual bool Open(const Params & params)
    {
      Close();

      activeDirection = params.m_direction;
      m_deviceName = params.m_device;

      return SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample) &&
             SetBuffers(params.m_bufferSize, params.m_bufferCount) &&
             InternalOpen();
    }
  
  
    virtual PString GetName() const
    {
      return m_deviceName;
    }


    virtual PBoolean IsOpen() const
    {
      return m_bufferQueue.IsValid();
    }


    virtual PBoolean Close()
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
      PTRACE_IF(5, !GetName().IsEmpty(), activeDirection << " closed \"" << GetName() << '"');
      return true;
    }


    virtual PBoolean Abort()
    {
      if (CheckNotOpen())
        return false;

      bool ok = true;

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
      m_queue.Close();

      PTRACE(5, activeDirection << " aborted: ok=" << ok);
      return ok;
    }


    virtual PBoolean Write(const void * buf, PINDEX len)
    {
      SetLastWriteCount(0);

      if (CheckNotOpen())
        return false;

      if (!PAssert(activeDirection == Player, "Trying to write to recorder"))
        return false;

      SLuint32 state;
      if (CHECK_SL_ERROR(m_player.GetPlayState, (&state)))
        return false;

      PTRACE_DETAILED(5, "Write: size=" << len << ", qlen=" << m_queue.GetLength() << ", bufferPos=" << m_bufferPos);
      if (!PIndirectChannel::Write(buf, len))
        return false;

      // See if first time in
      if (state == SL_PLAYSTATE_PLAYING)
        return true;

      PTRACE(5, "Starting playback");
      for (size_t i = 0; i < PARRAYSIZE(m_buffers); ++i) {
        if (!EnqueueBuffer())
          return false;
      }

      return CHECK_SL_SUCCESS(m_player.SetPlayState, (SL_PLAYSTATE_PLAYING));
    }


    virtual PBoolean HasPlayCompleted()
    {
      return m_queue.GetLength() == 0;
    }


    virtual PBoolean WaitForPlayCompletion()
    {
      while (!HasPlayCompleted()) {
        PThread::Sleep(m_buffers[0].size()*1000/GetSampleRate());
        if (CheckNotOpen())
          return false;
      }
      return true;
    }


    virtual PBoolean Read(void * buf, PINDEX len)
    {
      SetLastReadCount(0);

      if (CheckNotOpen())
        return false;

      if (!PAssert(activeDirection == Recorder, "Trying to read from player"))
        return false;

      if (!StartRecording())
        return false;

      PTRACE_DETAILED(5, "Read: size=" << len << ", qlen=" << m_queue.GetLength() << ", bufferPos=" << m_bufferPos);
      return PIndirectChannel::Read(buf, len);
    }


    virtual PBoolean StartRecording()
    {
      if (CheckNotOpen())
        return false;

      SLuint32 state;
      if (CHECK_SL_ERROR(m_recorder.GetRecordState, (&state)))
        return false;

      if (state == SL_RECORDSTATE_RECORDING)
        return true;

      m_queue.Open(m_queue.GetSize()); // Effectively flush

      m_bufferPos = 0;

      for (size_t i = 0; i < PARRAYSIZE(m_buffers); ++i) {
        if (!EnqueueBuffer())
          return false;
      }

      if (CHECK_SL_ERROR(m_recorder.SetRecordState, (SL_RECORDSTATE_RECORDING)))
        return false;

      PTRACE(5, "Started record state");
      return true;
    }


    virtual PBoolean SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
    {
      if (numChannels == GetChannels()&&
          sampleRate == GetSampleRate() &&
          bitsPerSample == GetSampleSize())
        return true;

      if (!PAssert(numChannels > 0 && sampleRate >= 8000 && bitsPerSample >= 8, PInvalidParameter))
        return false;

      bool notOpen = !IsOpen();

      PTRACE(4, "SetFormat(" << numChannels << ',' << sampleRate << ',' << bitsPerSample << (notOpen ? ") closed" : ") open"));

      Close();

      return notOpen || InternalOpen();
    }


    void InternalSetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
    {
      m_format_pcm.formatType = SL_DATAFORMAT_PCM;
      m_format_pcm.numChannels = numChannels;
      m_format_pcm.samplesPerSec = sampleRate*1000; // yeah, member name is bogus, is really milliHertz
      m_format_pcm.bitsPerSample = bitsPerSample;
      m_format_pcm.containerSize = bitsPerSample;
      m_format_pcm.channelMask = SL_SPEAKER_FRONT_CENTER;
      m_format_pcm.endianness = SL_BYTEORDER_LITTLEENDIAN;
    }


    virtual unsigned GetChannels() const
    {
      return m_format_pcm.numChannels;
    }


    virtual unsigned GetSampleRate() const
    {
      return m_format_pcm.samplesPerSec/1000; // yeah, member name is bogus, is really milliHertz
    }


    virtual unsigned GetSampleSize() const
    {
      return m_format_pcm.bitsPerSample;
    }


    virtual PBoolean SetBuffers(PINDEX size, PINDEX count)
    {
      if (size ==  m_buffers[0].size() && count == m_queue.GetSize()/size)
        return true;

      if (!PAssert(size > 80 && count > 1, PInvalidParameter))
        return false;

      bool notOpen = !IsOpen();
      PTRACE(4, "SetBuffers(" << size << ',' << count << (notOpen ? ") closed" : ") open"));
      Close();
      InternalSetBuffers(size, count);
      return notOpen || InternalOpen();
    }


    void InternalSetBuffers(PINDEX size, PINDEX count)
    {
      m_queue.Open(count*size);

      for (size_t i = 0; i < PARRAYSIZE(m_buffers); ++i)
        m_buffers[i].resize(size);

      m_bufferPos = 0;
    }


    virtual PBoolean GetBuffers(PINDEX & size, PINDEX & count)
    {
      size = m_buffers[0].size();
      count = m_queue.GetSize()/size;
      return true;
    }


    virtual PBoolean SetVolume(unsigned newVolume)
    {
      if (!m_volume.IsValid())
        return false;

      SLmillibel maxVol;
      if (CHECK_SL_ERROR(m_volume.GetMaxVolumeLevel, (&maxVol)))
        return false;

      return CHECK_SL_SUCCESS(m_volume.SetVolumeLevel, ((SLmillibel)((int)newVolume*(maxVol - SL_MILLIBEL_MIN)/100 + SL_MILLIBEL_MIN)));
    }


    virtual PBoolean GetVolume(unsigned & oldVolume)
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


    virtual bool SetMute(bool newMute)
    {
      if (!m_volume.IsValid())
        return false;

      return CHECK_SL_SUCCESS(m_volume.SetMute, (newMute ? SL_BOOLEAN_TRUE : SL_BOOLEAN_FALSE));
    }


    virtual bool GetMute(bool & oldMute)
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
