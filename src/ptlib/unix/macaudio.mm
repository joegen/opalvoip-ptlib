/*
 * macaudio.mm
 *
 * Classes to support audio input and output.
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
 */

#include <ptlib.h>

#if P_AUDIO

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>
#include <ptclib/qchannel.h>

#ifdef BOOL
#undef BOOL
#endif

#include <AudioUnit/AudioUnit.h>
#ifdef P_MACOSX
  #include <CoreAudio/CoreAudio.h>
  #include <AudioToolbox/AudioConverter.h>
#else
  #include <AudioToolbox/AudioSession.h>
  #include <AudioToolbox/AudioServices.h>
  typedef UInt32 AudioDeviceID;
#endif


#define PTraceModule() "MacAudio"
#define PTRACE_DETAILED(...) PTRACE(__VA_ARGS__)


// These two values are pretty fundamental, but missing from system headers
#define kAudioUnitOutputBus 0
#define kAudioUnitInputBus  1

static PConstCaselessString const DefaultDeviceName("Default");


#if PTRACING

static bool CheckError(OSStatus status, const char * fn, const char * prop = NULL)
{
  if (status == 0 || !PTrace::CanTrace(1))
    return false;

  static POrdinalToString::Initialiser ErrorMessagesInit[] = {
    { -50,                                    "Invalid Parameter(s)" },
    { kAudioUnitErr_InvalidProperty,          "Invalid Property" },
    { kAudioUnitErr_InvalidParameter,         "Invalid Parameter" },
    { kAudioUnitErr_FormatNotSupported,       "Format Not Supported" },
    { kAudioUnitErr_PropertyNotWritable,      "Property Not Writable" },
    { kAudioUnitErr_CannotDoInCurrentContext, "Cannot Do In Current Context" },
    { kAudioUnitErr_InvalidPropertyValue,     "Invalid Property Value" }
  };
  static POrdinalToString const ErrorMessages(PARRAYSIZE(ErrorMessagesInit), ErrorMessagesInit);

  const char * errCode = (const char *)&status;

  ostream & trace = PTRACE_BEGIN(1);
  trace << "Error ";
  if (ErrorMessages.Contains(status))
    trace << '"' << ErrorMessages[status] << "\" (" << status << ')';
  else if (status > -100000 && status < 100000)
    trace << status;
  else if (isprint(errCode[0]) && isprint(errCode[1]) && isprint(errCode[2]) && isprint(errCode[3]))
    trace << '\'' << errCode[0] << errCode[1] << errCode[2] << errCode[3] << '\'';
  else
    trace << "0x" << hex << status << dec;
  trace << " in function " << fn;
  if (prop != NULL)
    trace << ", property " << prop;
  trace << PTrace::End;
  return true;
}

#define CHECK_SUCCESS(fn, args) (!CheckError(fn args, #fn))
#define CHECK_ERROR(fn, args)     CheckError(fn args, #fn)
#define CHECK_ERROR_AudioSessionSetProperty(prop,size,ptr) \
            CheckError(AudioSessionSetProperty(prop,size,ptr), "AudioSessionSetProperty", #prop)
#define CHECK_ERROR_AudioSessionGetProperty(prop,size,ptr) \
            CheckError(AudioSessionGetProperty(prop,size,ptr), "AudioSessionGetProperty", #prop)
#define CHECK_ERROR_AudioUnitSetProperty(audioUnit,prop,scope,bus,ptr,size) \
            CheckError(AudioUnitSetProperty(audioUnit,prop,scope,bus,ptr,size), "AudioUnitSetProperty", #prop)
#define CHECK_ERROR_AudioUnitGetProperty(audioUnit,prop,scope,bus,ptr,size) \
            CheckError(AudioUnitGetProperty(audioUnit,prop,scope,bus,ptr,size), "AudioUnitGetProperty", #prop)

#else

#define CHECK_SUCCESS(fn, args) (fn args == 0)
#define CHECK_ERROR(fn, args)   (fn args != 0)
#define CHECK_ERROR_AudioSessionSetProperty(prop,size,ptr) \
            AudioSessionSetProperty(subFn,size,ptr),
#define CHECK_ERROR_AudioSessionGetProperty(prop,size,ptr) \
            AudioSessionGetProperty(subFn,size,ptr)
#define CHECK_ERROR_AudioUnitSetProperty(audioUnit,prop,scope,bus,ptr,size) \
            AudioUnitSetProperty(audioUnit,prop,scope,bus,ptr,size)
#define CHECK_ERROR_AudioUnitGetProperty(audioUnit,prop,scope,bus,ptr,size) \
            AudioUnitGetProperty(audioUnit,prop,scope,bus,ptr,size)

#endif


class PSoundChannel_Apple : public PSoundChannel
{
  PCLASSINFO(PSoundChannel_Apple, PSoundChannel);
protected:
  PString                     m_deviceName;
  AudioStreamBasicDescription m_dataFormat;
  AudioComponentInstance      m_audioUnit;
#ifdef P_MACOSX
  AudioConverterRef           m_resampler;
  std::vector<uint8_t>        m_resampleBuffer;
#endif
  PQueueChannel               m_queue;
  PINDEX                      m_bufferSize;
  PINDEX                      m_bufferCount;
  bool                        m_muted;

  
#ifndef P_MACOSX
  struct Session
  {
    set<PSoundChannel_Apple *> m_channels;

    Session()
    {
      CHECK_ERROR(AudioSessionInitialize,(NULL, NULL, NULL, NULL));

      // We want to be able to open playback and recording streams
      UInt32 audioCategory = kAudioSessionCategory_PlayAndRecord;
      CHECK_ERROR_AudioSessionSetProperty(kAudioSessionProperty_AudioCategory,
                                          sizeof(audioCategory), &audioCategory);
    }
    
    void CreatedChannel(PSoundChannel_Apple * channel)
    {
      if (m_channels.empty())
        CHECK_ERROR(AudioSessionSetActive,(true));

      m_channels.insert(channel);
    }

    void DestroyedChannel(PSoundChannel_Apple * channel)
    {
      m_channels.erase(channel);
      
      if ( m_channels.empty())
        CHECK_ERROR(AudioSessionSetActive,(false));
    }
  };
  
  static Session & GetSession()
  {
    static Session session;
    return session;
  }
#endif
  
  class Devices : public std::map<PCaselessString, AudioDeviceID>
  {
    public:
      Devices(Directions dir)
      {
#ifdef P_MACOSX
        UInt32 size = 0;
        static const AudioObjectPropertyAddress DevicesAddr =
            { kAudioHardwarePropertyDevices, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        
        if (CHECK_SUCCESS(AudioObjectGetPropertyDataSize,(kAudioObjectSystemObject, &DevicesAddr, 0, NULL, &size))) {
          std::vector<AudioDeviceID> devIDs(size/sizeof(AudioDeviceID));
          CHECK_ERROR(AudioObjectGetPropertyData,(kAudioObjectSystemObject, &DevicesAddr, 0, NULL, &size, devIDs.data()));
          
          for (size_t i = 0; i < devIDs.size(); ++i) {
            static const AudioObjectPropertyAddress InputStreamsAddr =
                { kAudioDevicePropertyStreams, kAudioDevicePropertyScopeInput, kAudioObjectPropertyElementMaster };
            static const AudioObjectPropertyAddress OutputStreamsAddr =
                { kAudioDevicePropertyStreams, kAudioDevicePropertyScopeOutput, kAudioObjectPropertyElementMaster };
            if (CHECK_SUCCESS(AudioObjectGetPropertyDataSize,(devIDs[i],
                                                              dir == Recorder ? &InputStreamsAddr
                                                                              : &OutputStreamsAddr
                                                              , 0, NULL, &size)) && size >= sizeof(AudioStreamID)) {
              static const AudioObjectPropertyAddress DeviceNameAddr =
                  { kAudioDevicePropertyDeviceName, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
              
              char name[100];
              size = sizeof(name);
              if (CHECK_SUCCESS(AudioObjectGetPropertyData,(devIDs[i], &DeviceNameAddr, 0, NULL, &size, name))) {
                PString devName(name, size);
                insert(value_type(devName, devIDs[i]));
              }
            }
          }
        }
        
        static const AudioObjectPropertyAddress DefaultInputDeviceAddr =
            { kAudioHardwarePropertyDefaultInputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };
        static const AudioObjectPropertyAddress DefaultOutputDeviceAddr =
            { kAudioHardwarePropertyDefaultOutputDevice, kAudioObjectPropertyScopeGlobal, kAudioObjectPropertyElementMaster };

        AudioDeviceID defID;
        size = sizeof(AudioDeviceID);
        if (CHECK_SUCCESS(AudioObjectGetPropertyData,(kAudioObjectSystemObject,
                                                      dir == Recorder ? &DefaultInputDeviceAddr
                                                                      : &DefaultOutputDeviceAddr,
                                                      0, NULL, &size, &defID)))
          insert(value_type(DefaultDeviceName, defID));
#else
        insert(value_type(DefaultDeviceName, 0));
#endif
      }
  };

  
  static OSStatus PlayerCallback(void                       *inRefCon,
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp       *inTimeStamp,
                                 UInt32                      inBusNumber,
                                 UInt32                      inNumberFrames,
                                 AudioBufferList            *ioData)
  {
    reinterpret_cast<PSoundChannel_Apple *>(inRefCon)->PlayerCallback(inNumberFrames, ioData);
    return noErr;
  }

  void PlayerCallback(UInt32 inNumberFrames,
                      AudioBufferList *ioData)
  {
    UInt32 bytes = inNumberFrames*m_dataFormat.mBytesPerFrame;
    uint8_t * data = (uint8_t *)ioData->mBuffers[0].mData;
    m_queue.Read(data, bytes);
    PINDEX count = m_queue.GetLastReadCount();
    PTRACE_DETAILED(5, "PlayerCallback: bytes needed=" << bytes << ", have=" << count);
    if (count < bytes) {
      PTRACE_IF(4, m_queue.IsOpen(), "Underflow: count=" << count << ", needed=" << bytes);
      memset(data+count, 0, bytes-count);
    }
  }
  
  static OSStatus RecordCallback(void                       *inRefCon,
                                 AudioUnitRenderActionFlags *ioActionFlags,
                                 const AudioTimeStamp       *inTimeStamp,
                                 UInt32                      inBusNumber,
                                 UInt32                      inNumberFrames,
                                 AudioBufferList            *ioData)
  {
    reinterpret_cast<PSoundChannel_Apple *>(inRefCon)->RecordCallback(ioActionFlags, inTimeStamp, inBusNumber, inNumberFrames);
    return noErr;
  }

  void RecordCallback(AudioUnitRenderActionFlags *ioActionFlags,
                      const AudioTimeStamp *inTimeStamp,
                      UInt32 inBusNumber,
                      UInt32 inNumberFrames)
  {
    UInt32 bytes = inNumberFrames*m_dataFormat.mBytesPerFrame;

    AudioBufferList bufferList;
    bufferList.mNumberBuffers = 1;
    bufferList.mBuffers[0].mNumberChannels = m_dataFormat.mChannelsPerFrame;
    bufferList.mBuffers[0].mDataByteSize = bytes;
    bufferList.mBuffers[0].mData = NULL;

    if (CHECK_ERROR(AudioUnitRender,(m_audioUnit,
                                     ioActionFlags,
                                     inTimeStamp,
                                     inBusNumber,
                                     inNumberFrames,
                                    &bufferList)))
      return;

    if (!m_queue.Write(bufferList.mBuffers[0].mData, bytes)) {
      PTRACE_IF(4, m_queue.IsOpen(), "Overflow, queue full");
    }

    PTRACE_DETAILED(5, "RecordCallback: bytes offerred=" << bytes << ", saved=" << m_queue.GetLastWriteCount());
  }
  

#ifdef P_MACOSX
  static OSStatus ConverterCallback(AudioConverterRef             inAudioConverter,
                                    UInt32                        *ioNumberDataPackets,
                                    AudioBufferList               *ioData,
                                    AudioStreamPacketDescription **outDataPacketDescription,
                                    void                          *inUserData)
  {
    reinterpret_cast<PSoundChannel_Apple *>(inUserData)->ConverterCallback(ioNumberDataPackets, ioData);
    return noErr;
  }

  void ConverterCallback(UInt32 *ioNumberDataPackets, AudioBufferList *ioData)
  {
    if (m_queue.Read(m_resampleBuffer.data(),
                     std::min((UInt32)m_resampleBuffer.size(),
                              *ioNumberDataPackets * m_dataFormat.mBytesPerPacket))) {
      UInt32 packets = m_queue.GetLastReadCount()/m_dataFormat.mBytesPerPacket;
      if (*ioNumberDataPackets > packets)
        *ioNumberDataPackets = packets;

      ioData->mNumberBuffers = 1;
      ioData->mBuffers[0].mNumberChannels = m_dataFormat.mChannelsPerFrame;
      ioData->mBuffers[0].mData = m_resampleBuffer.data();
      ioData->mBuffers[0].mDataByteSize = packets*m_dataFormat.mBytesPerPacket;
    }
    else {
      *ioNumberDataPackets = 0;
      ioData->mNumberBuffers = 0;
    }
  }


  bool InternalOpenDevice(AudioUnitElement busElement)
  {
    Devices devices(activeDirection);
    Devices::iterator it = devices.find(m_deviceName);
    if (it != devices.end())
      return !CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                               kAudioOutputUnitProperty_CurrentDevice,
                                               kAudioUnitScope_Global,
                                               busElement,
                                               &it->second,
                                               sizeof(it->second));

    PTRACE(1, "No such device as \"" << m_deviceName << '"');
    return false;
  }
#endif // P_MACOSX


  bool InternalOpenPlayer()
  {
    // Must enable I/O before setting device or it fails.
    UInt32 flag = 1;
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_EnableIO,
                                         kAudioUnitScope_Output,
                                         kAudioUnitOutputBus,
                                         &flag, sizeof(flag)))
      return false;

#ifdef P_MACOSX
    // Set the specific device
    if (!InternalOpenDevice(kAudioUnitOutputBus))
      return false;
#endif

    // The player will take our PCM-16 sample format as is
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioUnitProperty_StreamFormat,
                                         kAudioUnitScope_Input,
                                         kAudioUnitOutputBus,
                                         &m_dataFormat, sizeof(m_dataFormat)))
      return false;

    // Call back when it needs data from m_queue
    AURenderCallbackStruct callback;
    callback.inputProcRefCon = this;
    callback.inputProc = PlayerCallback;
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioUnitProperty_SetRenderCallback,
                                         kAudioUnitScope_Global,
                                         kAudioUnitOutputBus,
                                         &callback, sizeof(callback)))
      return false;

    m_queue.Open(m_bufferSize*m_bufferCount);
    
    // Don't block the call back eading from queue
    m_queue.SetReadTimeout(0);
    SetWriteTimeout(PTimeInterval(std::max(1000, (int)(m_bufferSize*1000/GetSampleRate()))));
    return true;
  }
  

  bool InternalOpenRecorder()
  {
    // Must enable I/O before setting device or it fails.
    UInt32 flag = 1;
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_EnableIO,
                                         kAudioUnitScope_Input,
                                         kAudioUnitInputBus,
                                         &flag, sizeof(flag)))
      return false;
    
    // When recording, disable the output side of pipeline, which is enabled by default
    flag = 0;
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_EnableIO,
                                         kAudioUnitScope_Output,
                                         kAudioUnitOutputBus,
                                         &flag, sizeof(flag)))
      return false;
    
#ifdef P_MACOSX
    if (!InternalOpenDevice(kAudioUnitInputBus))
      return false;
    
    // In OS-X we need to convert the sample rate, so need to get it ...
    AudioStreamBasicDescription deviceFormat;
    UInt32 size = sizeof(deviceFormat);
    if (CHECK_ERROR_AudioUnitGetProperty(m_audioUnit,
                                         kAudioUnitProperty_StreamFormat,
                                         kAudioUnitScope_Input,
                                         kAudioUnitInputBus,
                                         &deviceFormat, &size))
      return false;

    // Switch in all the other format parameters
    Float64 deviceSampleRate = deviceFormat.mSampleRate;
    deviceFormat = m_dataFormat;

    // Put back the devices sample rate
    deviceFormat.mSampleRate = deviceSampleRate;

    // And set that.
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioUnitProperty_StreamFormat,
                                         kAudioUnitScope_Output,
                                         kAudioUnitInputBus,
                                         &deviceFormat, sizeof(deviceFormat)))
      return false;

    // Get it back just in case it did not set exactly the same value.
    size = sizeof(deviceFormat);
    if (CHECK_ERROR_AudioUnitGetProperty(m_audioUnit,
                                         kAudioUnitProperty_StreamFormat,
                                         kAudioUnitScope_Output,
                                         kAudioUnitInputBus,
                                         &deviceFormat, &size))
      return false;

    if (deviceFormat.mSampleRate == m_dataFormat.mSampleRate) {
      if (!m_queue.Open(m_bufferSize*m_bufferCount))
        return false;
    }
    else {
      // Create converted from actual sample rate, format etc to ours.
      if (CHECK_ERROR(AudioConverterNew,(&deviceFormat, &m_dataFormat, &m_resampler)))
        return false;

      if (!m_queue.Open(m_bufferSize*m_bufferCount*deviceFormat.mSampleRate/m_dataFormat.mSampleRate))
        return false;

      m_resampleBuffer.resize(m_dataFormat.mBytesPerPacket*deviceFormat.mSampleRate/m_dataFormat.mSampleRate);
    }
#else
    // Weirdly iOS is easier the OS-X! We can simply set record sample rate
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioUnitProperty_StreamFormat,
                                         kAudioUnitScope_Output,
                                         kAudioUnitInputBus,
                                         &m_dataFormat, sizeof(m_dataFormat)))
      return false;

    if (!m_queue.Open(m_bufferSize*m_bufferCount))
      return false;
#endif // P_MACOSX

    // Call back for when devices has data to queue up
    AURenderCallbackStruct callback;
    callback.inputProcRefCon = this;
    callback.inputProc = RecordCallback;
    if (CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_SetInputCallback,
                                         kAudioUnitScope_Global,
                                         kAudioUnitInputBus,
                                         &callback, sizeof(callback)))
      return false;

    // Make sure queue does not block the callback
    m_queue.SetWriteTimeout(0);
    SetReadTimeout(PTimeInterval(std::max(1000, (int)(m_bufferSize*1000/GetSampleRate()))));
    return true;
  }
  
  
  bool InternalOpen()
  {
    PTRACE(5, "Opening " << activeDirection<< " \"" << m_deviceName << '"');

    AudioComponentDescription desc;
    desc.componentType = kAudioUnitType_Output;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;
    desc.componentFlagsMask = 0;
    desc.componentFlags = 0;
#ifdef P_MACOSX
    desc.componentSubType = kAudioUnitSubType_HALOutput;
#else
    desc.componentSubType = kAudioUnitSubType_RemoteIO;

    {
      Float32 bufferDuration = m_bufferSize/2 * 1000/GetSampleRate();
      if (CHECK_ERROR_AudioSessionSetProperty(kAudioSessionProperty_PreferredHardwareIOBufferDuration,
                                               sizeof(bufferDuration), &bufferDuration))
        return false;
    }
#endif

    AudioComponent component = AudioComponentFindNext(NULL, &desc);
    if (component == NULL)
      return false;

    if (CHECK_ERROR(AudioComponentInstanceNew,(component, &m_audioUnit)))
      return false;

    switch (activeDirection) {
      case Player :
        if (!InternalOpenPlayer())
          return false;
        break;
        
      case Recorder :
        if (!InternalOpenRecorder())
          return false;
        break;
        
      default :
        return false;
    }

    // Finally have everything, initialise it
    if (CHECK_ERROR(AudioUnitInitialize,(m_audioUnit)))
      return false;

    PTRACE(3, "Opened " << activeDirection<< " \"" << m_deviceName << '"');
    os_handle = 1;
    return true;
  }
  
  
public:
  PSoundChannel_Apple()
    : m_audioUnit(NULL)
#ifdef P_MACOSX
    , m_resampler(NULL)
#endif
    , m_bufferSize(320)
    , m_bufferCount(2)
    , m_muted(false)
  {
#ifndef P_MACOSX
    GetSession().CreatedChannel(this);
#endif

    InternalSetFormat(1, 8000, 16);
    PIndirectChannel::Open(m_queue);
  }


  ~PSoundChannel_Apple()
  {
    Close();
    PIndirectChannel::Close();

#ifndef P_MACOSX
    GetSession().DestroyedChannel(this);
#endif
  }
  
  
  static PStringArray GetDeviceNames(PSoundChannel::Directions dir)
  {
    Devices devices(dir);
    
    PStringArray names(devices.size());

    PINDEX count = 0;
    Devices::iterator it = devices.find(DefaultDeviceName);
    if (it != devices.end())
      names[count++] = DefaultDeviceName;

    for (it = devices.begin(); it != devices.end(); ++it) {
      if (DefaultDeviceName != it->first)
        names[count++] = it->first;
    }

    return names;
  }
  
  
  virtual bool Open(const Params & params)
  {
    Close();
    
    activeDirection = params.m_direction;
    m_deviceName = params.m_device;

    return SetBuffers(params.m_bufferSize, params.m_bufferCount) &&
           SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample) &&
           InternalOpen();    
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
    PTRACE_IF(4, IsOpen(), "Closing \"" << GetName() << '"');

    os_handle = -1;

    // Break any read/write block
    m_queue.Close();
    
    channelPointerMutex.StartWrite();
    
    if (m_audioUnit != NULL) {
      CHECK_SUCCESS(AudioComponentInstanceDispose,(m_audioUnit));
      m_audioUnit = NULL;
    }
    
#ifdef P_MACOSX
    if (m_resampler != NULL) {
      CHECK_SUCCESS(AudioConverterDispose,(m_resampler));
      m_resampler = NULL;
    }
#endif

    channelPointerMutex.EndWrite();
    return true;
  }
  
  
  virtual PBoolean Abort()
  {
    if (!IsOpen())
      return false;

    PTRACE(4, activeDirection << " aborted");
    
    CHECK_SUCCESS(AudioOutputUnitStop,(m_audioUnit));

    // Break any read/write block
    m_queue.Close();
    return true;
  }
  
  
  virtual PBoolean Write(const void * buf, PINDEX len)
  {
    SetLastWriteCount(0);
    
    if (!PAssert(activeDirection == Player, "Trying to write to recorder"))
      return false;

    if (m_muted) {
      SetLastWriteCount(len);
      return true;
    }
    
    PReadWaitAndSignal mutex(channelPointerMutex);
    
    UInt32 isRunning;
    UInt32 size = sizeof(isRunning);
    if (CHECK_ERROR_AudioUnitGetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_IsRunning,
                                         kAudioUnitScope_Global, kAudioUnitInputBus,
                                         &isRunning, &size))
      return false;
    
    if (!isRunning) {
      PTRACE(4, "Starting playback");
      if (CHECK_ERROR(AudioOutputUnitStart,(m_audioUnit)))
        return false;
    }

    return PIndirectChannel::Write(buf, len);
  }
  
  
  virtual PBoolean HasPlayCompleted()
  {
    return m_queue.GetLength() == 0;
  }
  
  
  virtual PBoolean WaitForPlayCompletion()
  {
    while (!HasPlayCompleted()) {
      PThread::Sleep(m_bufferSize*1000/GetSampleRate());
      if (!IsOpen())
        return false;
    }
    return true;
  }
  
  
  virtual PBoolean Read(void * buf, PINDEX len)
  {
    SetLastReadCount(0);
    
    if (!PAssert(activeDirection == Recorder, "Trying to read from player"))
      return false;
    
    PReadWaitAndSignal mutex(channelPointerMutex);
    
    if (!StartRecording())
      return false;

#ifdef P_MACOSX
    if (m_resampler != NULL) {
      AudioBufferList bufferList;
      bufferList.mNumberBuffers = 1;
      bufferList.mBuffers[0].mNumberChannels = m_dataFormat.mChannelsPerFrame;
      bufferList.mBuffers[0].mDataByteSize = len;
      bufferList.mBuffers[0].mData = buf;

      UInt32 ioOutputDataPacketSize = len/m_dataFormat.mBytesPerPacket;
      if (CHECK_ERROR(AudioConverterFillComplexBuffer,(m_resampler,
                                                       ConverterCallback, this,
                                                       &ioOutputDataPacketSize, &bufferList, NULL)))
        return false;

      SetLastReadCount(ioOutputDataPacketSize*m_dataFormat.mBytesPerPacket);
    }
    else
#endif
    if (!PIndirectChannel::Read(buf, len))
      return false;

    if (m_muted)
      memset(buf, 0, len);
    return true;
  }
  
  
  virtual PBoolean StartRecording()
  {
    if (!IsOpen() || !PAssert(activeDirection == Recorder, "Trying to start recording from player"))
      return false;

    PReadWaitAndSignal mutex(channelPointerMutex);
    
    UInt32 isRunning;
    UInt32 size = sizeof(isRunning);
    if (CHECK_ERROR_AudioUnitGetProperty(m_audioUnit,
                                         kAudioOutputUnitProperty_IsRunning,
                                         kAudioUnitScope_Global, kAudioUnitInputBus,
                                         &isRunning, &size))
      return false;

    if (isRunning)
      return true;
    
    m_queue.Open(m_queue.GetSize());

#ifdef P_MACOSX
    if (CHECK_ERROR(AudioConverterReset,(m_resampler)))
      return false;
#endif

    if (CHECK_ERROR(AudioOutputUnitStart,(m_audioUnit)))
      return false;

    PTRACE(5, "Started recording");
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
    InternalSetFormat(numChannels, sampleRate, bitsPerSample);
    return notOpen || InternalOpen();
  }
  
  
  void InternalSetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample)
  {
    memset(&m_dataFormat, 0, sizeof(m_dataFormat));
    
    m_dataFormat.mFormatID         = kAudioFormatLinearPCM;
    m_dataFormat.mChannelsPerFrame = numChannels;
    m_dataFormat.mSampleRate       = sampleRate;
    m_dataFormat.mBitsPerChannel   = bitsPerSample;
    m_dataFormat.mFramesPerPacket  = 1;
    m_dataFormat.mBytesPerFrame    = m_dataFormat.mChannelsPerFrame * (bitsPerSample+7)/8;
    m_dataFormat.mBytesPerPacket   = m_dataFormat.mBytesPerFrame * m_dataFormat.mFramesPerPacket;
    m_dataFormat.mFormatFlags      = kLinearPCMFormatFlagIsSignedInteger | kLinearPCMFormatFlagIsPacked;
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
    if (count == m_bufferCount && size == m_bufferSize)
      return true;

    if (!PAssert(size > 80 && count > 1, PInvalidParameter))
      return false;
    
    bool notOpen = !IsOpen();
    
    PTRACE(4, "SetBuffers(" << size << ',' << count << ')' << (notOpen ? " closed" : " open"));
    
    Close();
    
    m_bufferSize = size;
    m_bufferCount = count;

    return notOpen || InternalOpen();
  }
  
  
  virtual PBoolean GetBuffers(PINDEX & size, PINDEX & count)
  {
    size = m_bufferSize;
    count = m_bufferCount;
    return true;
  }
  
  
  virtual PBoolean SetVolume(unsigned newVolume)
  {
    if (!IsOpen() || activeDirection != Player)
      return false;
    
    Float32 volume = newVolume/100.0;
#ifdef P_MACOSX
    return !CHECK_ERROR_AudioUnitSetProperty(m_audioUnit,
                                             kAudioDevicePropertyVolumeScalar,
                                             kAudioUnitScope_Output, kAudioUnitOutputBus,
                                             &volume, sizeof(Float32));
#else
    return !CHECK_ERROR_AudioSessionSetProperty(kAudioSessionProperty_CurrentHardwareOutputVolume,
                                                sizeof(Float32), &volume);
#endif
  }
  
  
  virtual PBoolean GetVolume(unsigned & oldVolume)
  {
    oldVolume = 0;
    
    if (!IsOpen() || activeDirection != Player)
      return false;
    
    Float32 volume = 0;
    UInt32 size = sizeof(volume);
#ifdef P_MACOSX
    if (CHECK_ERROR_AudioUnitGetProperty(m_audioUnit,
                                         kAudioDevicePropertyVolumeScalar,
                                         kAudioUnitScope_Output, kAudioUnitOutputBus,
                                         &volume, &size))
      return false;
#else
    if (CHECK_ERROR_AudioSessionGetProperty(kAudioSessionProperty_CurrentHardwareOutputVolume,
                                            &size, &volume))
      return false;
#endif

    oldVolume = (unsigned)100*volume;
    return true;
  }
  
  
  virtual bool SetMute(bool newMute)
  {
    m_muted = newMute;
    return true;
  }
  
  
  virtual bool GetMute(bool & oldMute)
  {
    oldMute = m_muted;
    return true;
  }
};


PCREATE_SOUND_PLUGIN(Apple, PSoundChannel_Apple)

#endif // P_AUDIO
