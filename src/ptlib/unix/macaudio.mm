/*
 * macvideo.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2013 Equivalence Pty. Ltd.
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
 * Contributor(s):
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptbuildopts.h>

#if P_AUDIO

#include <ptlib/sound.h>

#include <AudioToolbox/AudioQueue.h>
#include <AudioToolbox/AudioFormat.h>
#include <Foundation/NSError.h>
#include <Foundation/NSString.h>


#define PTraceModule() "MacAudio"
#define PTRACE_DETAILED(...) PTRACE(__VA_ARGS__)


#if PTRACING
static bool CheckError(OSStatus status, const char * fn)
{
  if (status == 0 || !PTrace::CanTrace(1))
    return false;
 
  PTrace::Begin(1, __FILE__, __LINE__) << "Error \""
         << [[[NSError errorWithDomain:NSOSStatusErrorDomain code:status userInfo:nil] localizedDescription] UTF8String]
  << "\" in function " << fn << PTrace::End;
  return true;
}
#define CHECK_SUCCESS(fn, args) (!CheckError(fn args, #fn))
#define CHECK_ERROR(fn, args)     CheckError(fn args, #fn)
#else
#define CHECK_SUCCESS(fn, args) (fn args == 0)
#define CHECK_ERROR(fn, args)   (fn args != 0)
#endif



class PSoundChannel_Apple : public PSoundChannel
{
  PCLASSINFO(PSoundChannel_Apple, PSoundChannel);
protected:
  PString                          m_deviceName;
  AudioStreamBasicDescription      m_dataFormat;
  AudioQueueRef                    m_queue;
  PINDEX                           m_bufferSize;
  std::vector<AudioQueueBufferRef> m_buffers;
  PINDEX                           m_bufferPos;
  PAtomicInteger                   m_buffersInUse;
  PSemaphore                       m_bufferReady;
  PTimeInterval                    m_bufferTimeout;
  bool                             m_running;
  

  static void PlayerCallback(void * inUserData, AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
  {
    reinterpret_cast<PSoundChannel_Apple *>(inUserData)->PlayerCallback(inAQ, inBuffer);
  }
  
  void PlayerCallback(AudioQueueRef inAQ, AudioQueueBufferRef inBuffer)
  {
    PTRACE_DETAILED(5, "PlayerCallback " << m_bufferPos << ' ' << m_buffersInUse);
    --m_buffersInUse;
    m_bufferReady.Signal();
  }
  
  
  static void RecorderCallback(void                                *inUserData,
                               AudioQueueRef                       inAQ,
                               AudioQueueBufferRef                 inBuffer,
                               const AudioTimeStamp                *inStartTime,
                               UInt32                              inNumberPackets,
                               const AudioStreamPacketDescription  *inPacketDescs)
  {
    reinterpret_cast<PSoundChannel_Apple *>(inUserData)->RecorderCallback(inAQ,
                                                                          inBuffer,
                                                                          inStartTime,
                                                                          inNumberPackets,
                                                                          inPacketDescs);
  }
  
  void RecorderCallback(AudioQueueRef                       inAQ,
                        AudioQueueBufferRef                 inBuffer,
                        const AudioTimeStamp                *inStartTime,
                        UInt32                              inNumberPackets,
                        const AudioStreamPacketDescription  *inPacketDescs)
  {
    PTRACE_DETAILED(5, "RecorderCallback " << m_bufferPos << ' ' << m_buffersInUse);
    ++m_buffersInUse;
    m_bufferReady.Signal();
  }
  
  
  bool InternalOpen()
  {
    switch (activeDirection) {
      case Player :
        if (CHECK_ERROR(AudioQueueNewOutput,(&m_dataFormat,
                                             &PSoundChannel_Apple::PlayerCallback,  this,
                                             NULL, kCFRunLoopCommonModes, 0, &m_queue)))
          return false;
        m_bufferReady.Reset(m_buffers.size(), m_buffers.size());
        break;
        
      case Recorder :
        if (CHECK_ERROR(AudioQueueNewInput,(&m_dataFormat,
                                            &PSoundChannel_Apple::RecorderCallback,  this,
                                            NULL, kCFRunLoopCommonModes, 0, &m_queue)))
          return false;
        
        {
          AudioStreamBasicDescription dataFormat;
          memset(&dataFormat, 0, sizeof(dataFormat));
          UInt32 sz = sizeof(dataFormat);
#define CHK_PARAM(p) \
   setw(18) << #p << ':' << m_dataFormat.p << (m_dataFormat.p == dataFormat.p ? '=' : '!') << '=' << dataFormat.p << '\n'
          if (CHECK_SUCCESS(AudioQueueGetProperty,(m_queue, kAudioQueueProperty_StreamDescription, &dataFormat, &sz)))
            PTRACE(4, "Opened with parameters:\n"
                   << std::setiosflags(ios::fixed) << std::setprecision(0) << std::right
                   << std::hex
                   << CHK_PARAM(mFormatID)
                   << std::dec
                   << CHK_PARAM(mChannelsPerFrame)
                   << CHK_PARAM(mSampleRate)
                   << CHK_PARAM(mBitsPerChannel)
                   << CHK_PARAM(mFramesPerPacket)
                   << CHK_PARAM(mBytesPerFrame)
                   << CHK_PARAM(mBytesPerPacket)
                   << CHK_PARAM(mFormatFlags));
        }
        m_bufferReady.Reset(0, m_buffers.size());
        break;
        
      default :
        break;
    }
    
    for (size_t i = 0; i < m_buffers.size(); ++i) {
      if (CHECK_ERROR(AudioQueueAllocateBuffer,(m_queue, m_bufferSize, &m_buffers[i]))) {
        AudioQueueDispose(m_queue, true);
        return false;
      }
    }
    
    m_bufferPos = 0;
    m_buffersInUse = 0;
    m_bufferTimeout.SetInterval(std::max(1000U, m_bufferSize*1000U/GetSampleRate()));
    
    PTRACE(3, "Opened " << activeDirection<< " \"" << m_deviceName << '"');
    os_handle = 1;
    return true;
  }
  
  
public:
  PSoundChannel_Apple()
    : m_bufferReady(0, UINT_MAX)
    , m_running(false)
  {
    SetBuffers(320, 2);
    SetFormat(1, 8000, 16);
  }


  ~PSoundChannel_Apple()
  {
    Close();
  }
  
  
  static PStringArray GetDeviceNames(PSoundChannel::Directions dir)
  {
    PStringArray devices;
    
    switch (dir) {
      case Player :
        devices += "speaker";
        break;
        
      case Recorder :
        devices += "microphone";
        break;
        
      default :
        break;
    }
    
    return devices;
  }
  
  
  virtual PBoolean Open(const PString & deviceName,
                        Directions dir,
                        unsigned numChannels,
                        unsigned sampleRate,
                        unsigned bitsPerSample)
  {
    Close();
    
    PTRACE(4, "Open(" << deviceName << ',' << dir << ',' << numChannels << ',' << sampleRate << ',' << bitsPerSample << ')');
    
    activeDirection = dir;
    m_deviceName = deviceName;
    
    return SetFormat(numChannels, sampleRate, bitsPerSample) && InternalOpen();    
  }
  
  
  virtual PString GetName() const
  {
    return m_deviceName;
  }
  
  
  virtual PBoolean IsOpen() const
  {
    return os_handle > 0;
  }
  
  
  virtual PBoolean Close()
  {
    if (!IsOpen())
      return false;
    
    PTRACE(4, "Closing \"" << GetName() << '"');

    os_handle = -1;
    
    CHECK_SUCCESS(AudioQueueDispose,(m_queue, true));
    
    m_running = false;
    
    // Break any read/write block
    m_bufferReady.Signal();
    return true;
  }
  
  
  virtual PBoolean Abort()
  {
    if (!IsOpen())
      return false;

    PTRACE(4, activeDirection << " aborted");
    
    CHECK_SUCCESS(AudioQueueStop,(m_queue, true));

    m_running = false;
    
    // Break any read/write block
    m_bufferReady.Signal();
    return true;
  }
  
  
  virtual PBoolean Write(const void * buf, PINDEX len)
  {
    if (!IsOpen()) {
      PTRACE(1, "Audio channel not open");
      return false;
    }
    
    if (!PAssert(activeDirection == Player, "Trying to write to recorder"))
      return false;

    const uint8_t * ptr = (const uint8_t *)buf;
    while (len > 0) {
      PTRACE_DETAILED(5, "Awaiting play buffer ready");
      if (!m_bufferReady.Wait(m_bufferTimeout)) {
        PTRACE(1, "Timed out waiting for play out: " << m_bufferTimeout);
        return false;
      }
      
      if (!IsOpen())
        return false;

      PINDEX chunkSize = std::min(len, m_bufferSize);
      memcpy(m_buffers[m_bufferPos]->mAudioData, ptr, chunkSize);
      m_buffers[m_bufferPos]->mAudioDataByteSize = chunkSize;
      if (CHECK_ERROR(AudioQueueEnqueueBuffer,(m_queue, m_buffers[m_bufferPos], 0, NULL)))
        return false;
      
      m_bufferPos = (m_bufferPos+1)%m_buffers.size();
      ++m_buffersInUse;
      
      if (!m_running) {
        if (CHECK_ERROR(AudioQueueStart,(m_queue, NULL)))
          return false;
        m_running = true;
      }
      
      lastWriteCount += chunkSize;
      ptr += chunkSize;
      len -= chunkSize;
    }
    
    return true;
  }
  
  
  virtual PBoolean HasPlayCompleted()
  {
    return m_buffersInUse == 0;
  }
  
  
  virtual PBoolean WaitForPlayCompletion()
  {
    while (!HasPlayCompleted()) {
      PTRACE_DETAILED(5, "Awaiting buffer ready for completion");
      if (!m_bufferReady.Wait(m_bufferTimeout))
        return false;
    }
    
    return true;
  }
  
  
  virtual PBoolean Read(void * buf, PINDEX len)
  {
    if (!IsOpen()) {
      PTRACE(1, "Audio channel not open");
      return false;
    }
    
    if (!PAssert(activeDirection == Recorder, "Trying to read from player"))
      return false;
    
    if (!StartRecording())
      return false;
    
    while (m_buffersInUse == 0) {
      PTRACE_DETAILED(5, "Awaiting buffer ready for completion");
      if (!m_bufferReady.Wait(m_bufferTimeout)) {
        PTRACE(1, "Timed out waiting for recorded data: " << m_bufferTimeout);
        return false;
      }
          
      if (!IsOpen())
        return false;
    }
    
    lastReadCount = std::min(len, (PINDEX)m_buffers[m_bufferPos]->mAudioDataByteSize);
    memcpy(buf, m_buffers[m_bufferPos]->mAudioData, lastReadCount);

    // Requeue it
    if (CHECK_ERROR(AudioQueueEnqueueBuffer,(m_queue, m_buffers[m_bufferPos], 0, NULL)))
      return false;

    m_bufferPos = (m_bufferPos+1)%m_buffers.size();
    --m_buffersInUse;

    PTRACE_DETAILED(5, "Read " << lastReadCount << " bytes.");
    return true;
  }
  
  
  virtual PBoolean StartRecording()
  {
    if (!IsOpen())
      return false;
    
    if (m_running)
      return true;

    m_bufferPos = 0;
    m_buffersInUse = 0;
    
    for (size_t i = 0; i < m_buffers.size(); ++i) {
      if (CHECK_ERROR(AudioQueueEnqueueBuffer,(m_queue, m_buffers[i], 0, NULL)))
        return false;
    }
    
    if (CHECK_ERROR(AudioQueueStart,(m_queue, NULL)))
      return false;
    
    m_running = true;
    PTRACE(5, "Started recording");
    return true;
  }
  
  
  virtual PBoolean SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
  {
    if (IsOpen()) {
      Close();
      return InternalOpen();
    }
    
    memset(&m_dataFormat, 0, sizeof(m_dataFormat));
    
    m_dataFormat.mFormatID         = kAudioFormatLinearPCM;
    m_dataFormat.mChannelsPerFrame = numChannels;
    m_dataFormat.mSampleRate       = sampleRate;
    m_dataFormat.mBitsPerChannel   = bitsPerSample;
    m_dataFormat.mFramesPerPacket  = 1;
    m_dataFormat.mBytesPerFrame    = m_dataFormat.mChannelsPerFrame * sizeof(SInt16);
    m_dataFormat.mBytesPerPacket   = m_dataFormat.mBytesPerFrame * m_dataFormat.mFramesPerPacket;
    m_dataFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;

    return true;
  }
  
  
  virtual unsigned GetChannels() const
  {
    return m_dataFormat.mChannelsPerFrame;
  }
  
  
  virtual unsigned GetSampleRate() const
  {
    return m_dataFormat.mSampleRate;
  }
  
  
  virtual unsigned GetSampleSize() const
  {
    return m_dataFormat.mBitsPerChannel;
  }
  
  
  virtual PBoolean SetBuffers(PINDEX size, PINDEX count)
  {
    if (size == m_bufferSize && count == m_buffers.size())
      return true;
    
    bool notOpen = !IsOpen();
    
    PTRACE(4, "SetBuffers(" << size << ',' << count << ')' << (notOpen ? " closed" : " open"));
    
    Close();

    m_bufferSize = size;
    m_buffers.resize(count);
    
    return notOpen || InternalOpen();
  }
  
  
  virtual PBoolean GetBuffers(PINDEX & size, PINDEX & count)
  {
    size = m_bufferSize;
    count = m_buffers.size();
    return true;
  }
  
  
  virtual PBoolean SetVolume(unsigned newVolume)
  {
    return false;
  }
  
  
  virtual PBoolean GetVolume(unsigned & oldVolume)
  {
    return false;
  }
  
  
  virtual bool SetMute(bool newMute)
  {
    return false;
  }
  
  
  virtual bool GetMute(bool & oldMute)
  {
    return false;
  }
};


PCREATE_SOUND_PLUGIN(Apple, PSoundChannel_Apple)

#endif // P_AUDIO
