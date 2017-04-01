/*
 * opensles.hpp
 *
 * C++ Wrapper for OpenSL ES interface
 *
 * Portable Tools Library
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
 */

#include <SLES/OpenSLES.h>

#ifdef __ANDROID__
  #include <SLES/OpenSLES_Android.h>
  #include <SLES/OpenSLES_AndroidConfiguration.h>
#endif

#include <vector>


namespace OpenSLES {

  class Interfaces {
    protected:
      std::vector<SLInterfaceID> m_id;
      std::vector<SLboolean>     m_req;

    public:
      Interfaces() { }
      Interfaces(SLInterfaceID id, SLboolean req = SL_BOOLEAN_TRUE)
        : m_id(1)
        , m_req(1)
      {
        m_id[0] = id;
        m_req[0] = req;
      }

      Interfaces & Add(SLInterfaceID id, SLboolean req = SL_BOOLEAN_TRUE)
      {
        m_id.push_back(id);
        m_req.push_back(req);
        return *this;
      }

      template <class T> Interfaces & Add(SLboolean req = SL_BOOLEAN_TRUE)
      {
        return Add(T::GetID(), req);
      }

      void Clear()
      {
        m_id.clear();
        m_req.clear();
      }

      const SLuint32        GetCount() const { return (SLuint32)m_id.size(); }
      const SLInterfaceID * GetIdentifiers() const { return m_id.empty() ? NULL : &m_id[0]; }
      const SLboolean     * GetRequired() const { return m_req.empty() ? NULL : &m_req[0]; }

      #define OPENSLES_INTERFACES(i) (i).GetCount(),(i).GetIdentifiers(),(i).GetRequired()
  };

  template <typename T>
  class Wrapper {
    protected:
      T m_itf;

    private:
      Wrapper(const Wrapper &) { }
      void operator=(const Wrapper &) { }

    public:
      Wrapper()
        : m_itf(NULL)
      {
      }

      virtual ~Wrapper()
      {
        Destroy();
      }

      operator T() const
      {
        return  m_itf;
      }

      T Get() const
      {
        return m_itf;
      }

      T * GetPtr()
      {
        Destroy();
        return &m_itf;
      }

      void Attach(T itf)
      {
        Destroy();
        m_itf = itf;
      }

      T Detach()
      {
        T itf = m_itf;
        m_itf = NULL;
        return itf;
      }

      bool IsValid() const
      {
        return m_itf != NULL;
      }

      virtual void Destroy()
      {
        m_itf = NULL;
      }
  };


  #define OPENSLES_OBJECT_FN_BODY(name,args,...) \
       SLresult name args { return IsValid() ? (**this)->name(*this, ##__VA_ARGS__) : SL_RESULT_PARAMETER_INVALID; }
  #define OPENSLES_OBJECT_FN_0(name) \
       OPENSLES_OBJECT_FN_BODY(name, ())
  #define OPENSLES_OBJECT_FN_1(name, type1,arg1) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1),                                                        arg1)
  #define OPENSLES_OBJECT_FN_2(name, type1,arg1,type2,arg2) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1,type2 arg2),                                             arg1,arg2)
  #define OPENSLES_OBJECT_FN_3(name, type1,arg1,type2,arg2,type3,arg3) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1,type2 arg2,type3 arg3),                                  arg1,arg2,arg3)
  #define OPENSLES_OBJECT_FN_4(name, type1,arg1,type2,arg2,type3,arg3,type4,arg4) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1,type2 arg2,type3 arg3,type4 arg4),                       arg1,arg2,arg3,arg4)
  #define OPENSLES_OBJECT_FN_5(name, type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5),            arg1,arg2,arg3,arg4,arg5)
  #define OPENSLES_OBJECT_FN_6(name, type1,arg1,type2,arg2,type3,arg3,type4,arg4,type5,arg5,type6,arg6) \
       OPENSLES_OBJECT_FN_BODY(name,(type1 arg1,type2 arg2,type3 arg3,type4 arg4,type5 arg5,type6 arg6), arg1,arg2,arg3,arg4,arg5,arg6)


  class Object : public Wrapper<SLObjectItf>
  {
    public:
      virtual void Destroy()
      {
        if (IsValid())
          (*m_itf)->Destroy(Detach());
      }

      void AbortAsyncOperation()
      {
        if (IsValid())
          (*m_itf)->AbortAsyncOperation(m_itf);
      }

      OPENSLES_OBJECT_FN_1(Realize,SLboolean,async)
      OPENSLES_OBJECT_FN_1(Resume,SLboolean,async)
      OPENSLES_OBJECT_FN_1(GetState,SLuint32*,pState)
      OPENSLES_OBJECT_FN_2(GetInterface,const SLInterfaceID,iid,void *,pInterface)
      OPENSLES_OBJECT_FN_2(RegisterCallback,slObjectCallback,callback,void *,pContext)
      OPENSLES_OBJECT_FN_2(SetPriority,SLint32,priority,SLboolean,preemptable)
      OPENSLES_OBJECT_FN_2(GetPriority,SLint32*,pPriority,SLboolean*,pPreemptable)
      OPENSLES_OBJECT_FN_3(SetLossOfControlInterfaces,SLint16,numInterfaces,SLInterfaceID *,pInterfaceIDs,SLboolean,enabled)
  };


  template <typename T, const SLInterfaceID * iid>
  class Interface : public Wrapper<T>
  {
    public:
      typedef T ItfType;

      static const SLInterfaceID GetID() { return *iid; }

      SLresult Create(Object & obj)
      {
        return obj.GetInterface(*iid, this->GetPtr());
      }
  };

  class Engine : public Interface<SLEngineItf, &SL_IID_ENGINE>
  {
    public:
      OPENSLES_OBJECT_FN_5(CreateLEDDevice,
        SLObjectItf *,pDevice,
        SLuint32, deviceID,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_5(CreateVibraDevice,
        SLObjectItf *, pDevice,
        SLuint32, deviceID,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      );
      OPENSLES_OBJECT_FN_6(CreateAudioPlayer,
        SLObjectItf *, pPlayer,
        SLDataSource *, pAudioSrc,
        SLDataSink *, pAudioSnk,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_6(CreateAudioRecorder,
        SLObjectItf *, pRecorder,
        SLDataSource *, pAudioSrc,
        SLDataSink *, pAudioSnk,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_4(CreateListener,
        SLObjectItf *, pListener,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_4(Create3DGroup,
        SLObjectItf *, pGroup,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      );
      OPENSLES_OBJECT_FN_4(CreateOutputMix,
        SLObjectItf *, pMix,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_5(CreateMetadataExtractor,
        SLObjectItf *, pMetadataExtractor,
        SLDataSource *, pDataSource,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_6(CreateExtensionObject,
        SLObjectItf *, pObject,
        void *, pParameters,
        SLuint32, objectID,
        SLuint32, numInterfaces,
        const SLInterfaceID *, pInterfaceIds,
        const SLboolean *, pInterfaceRequired
      )
      OPENSLES_OBJECT_FN_2(QueryNumSupportedInterfaces,
        SLuint32, objectID,
        SLuint32 *, pNumSupportedInterfaces
      )
      OPENSLES_OBJECT_FN_3(QuerySupportedInterfaces,
        SLuint32, objectID,
        SLuint32, index,
        SLInterfaceID *, pInterfaceId
      )
      OPENSLES_OBJECT_FN_1(QueryNumSupportedExtensions,
        SLuint32 *, pNumExtensions
      )
      OPENSLES_OBJECT_FN_3(QuerySupportedExtension,
        SLuint32, index,
        SLchar *, pExtensionName,
        SLint16 *, pNameLength
      )
      OPENSLES_OBJECT_FN_2(IsExtensionSupported,
        const SLchar *, pExtensionName,
        SLboolean *, pSupported  
      )

      SLresult CreateAudioPlayer(
        Object & player,
        SLDataSource & source,
        SLDataSink & sink,
        const Interfaces & ifaces
      ) {
        return CreateAudioPlayer(player.GetPtr(), &source, &sink, OPENSLES_INTERFACES(ifaces));
      }

      SLresult CreateAudioRecorder(
        Object & recorder,
        SLDataSource & source,
        SLDataSink & sink,
        const Interfaces & ifaces
      ) {
        return CreateAudioRecorder(recorder.GetPtr(), &source, &sink, OPENSLES_INTERFACES(ifaces));
      }
      SLresult CreateOutputMix(
        Object & mix,
        const Interfaces & ifaces
      ) {
        return CreateOutputMix(mix.GetPtr(), OPENSLES_INTERFACES(ifaces));
      }
  };


  class Play : public Interface<SLPlayItf, &SL_IID_PLAY>
  {
    public:
      OPENSLES_OBJECT_FN_1(SetPlayState,
        SLuint32, state
      )
      OPENSLES_OBJECT_FN_1(GetPlayState,
        SLuint32 *, pState
      )
      OPENSLES_OBJECT_FN_1(GetDuration,
        SLmillisecond *, pMsec
      )
      OPENSLES_OBJECT_FN_1(GetPosition,
        SLmillisecond *, pMsec
      )
      OPENSLES_OBJECT_FN_2(RegisterCallback,
        slPlayCallback, callback,
        void *, pContext
      )
      OPENSLES_OBJECT_FN_1(SetCallbackEventsMask,
        SLuint32, eventFlags
      )
      OPENSLES_OBJECT_FN_1(GetCallbackEventsMask,
        SLuint32 *, pEventFlags
      )
      OPENSLES_OBJECT_FN_1(SetMarkerPosition,
        SLmillisecond, mSec
      )
      OPENSLES_OBJECT_FN_0(ClearMarkerPosition)
      OPENSLES_OBJECT_FN_1(GetMarkerPosition,
        SLmillisecond *, pMsec
      )
      OPENSLES_OBJECT_FN_1(SetPositionUpdatePeriod,
        SLmillisecond, mSec
      )
      OPENSLES_OBJECT_FN_1(GetPositionUpdatePeriod,
        SLmillisecond *, pMsec
      )
  };


  class Record : public Interface<SLRecordItf, &SL_IID_RECORD>
  {
    public:
      OPENSLES_OBJECT_FN_1(SetRecordState,
        SLuint32, state
      )
      OPENSLES_OBJECT_FN_1(GetRecordState,
        SLuint32 *, pState
      )
      OPENSLES_OBJECT_FN_1(SetDurationLimit,
        SLmillisecond, msec
      )
      OPENSLES_OBJECT_FN_1(GetPosition,
        SLmillisecond *, pMsec
      )
      OPENSLES_OBJECT_FN_2(RegisterCallback,
        slRecordCallback, callback,
        void *, pContext
      )
      OPENSLES_OBJECT_FN_1(SetCallbackEventsMask,
        SLuint32, eventFlags
      )
      OPENSLES_OBJECT_FN_1(GetCallbackEventsMask,
        SLuint32 *, pEventFlags
      )
      OPENSLES_OBJECT_FN_1(SetMarkerPosition,
        SLmillisecond, mSec
      )
      OPENSLES_OBJECT_FN_0(ClearMarkerPosition)
      OPENSLES_OBJECT_FN_1(GetMarkerPosition,
        SLmillisecond *, pMsec
      )
      OPENSLES_OBJECT_FN_1(SetPositionUpdatePeriod,
        SLmillisecond, mSec
      )
      OPENSLES_OBJECT_FN_1(GetPositionUpdatePeriod,
        SLmillisecond *, pMsec
      )
  };


  class AudioIODeviceCapabilities : public Interface<SLAudioIODeviceCapabilitiesItf, &SL_IID_AUDIOIODEVICECAPABILITIES>
  {
    public:
      OPENSLES_OBJECT_FN_2(GetAvailableAudioInputs,
        SLint32  *, pNumInputs,
        SLuint32 *, pInputDeviceIDs
      );
      OPENSLES_OBJECT_FN_2(QueryAudioInputCapabilities,
        SLuint32, deviceId,
        SLAudioInputDescriptor *, pDescriptor
      );
      OPENSLES_OBJECT_FN_2(RegisterAvailableAudioInputsChangedCallback,
        slAvailableAudioInputsChangedCallback, callback,
        void *, pContext
      );
      OPENSLES_OBJECT_FN_2(GetAvailableAudioOutputs,
        SLint32 *, pNumOutputs,
        SLuint32 *, pOutputDeviceIDs
      );
      OPENSLES_OBJECT_FN_2(QueryAudioOutputCapabilities,
        SLuint32, deviceId,
        SLAudioOutputDescriptor *, pDescriptor
      );
      OPENSLES_OBJECT_FN_2(RegisterAvailableAudioOutputsChangedCallback,
        slAvailableAudioOutputsChangedCallback, callback,
        void *, pContext
      );
      OPENSLES_OBJECT_FN_2(RegisterDefaultDeviceIDMapChangedCallback,
        slDefaultDeviceIDMapChangedCallback, callback,
        void *, pContext
      );
      OPENSLES_OBJECT_FN_3(GetAssociatedAudioInputs,
        SLuint32, deviceId,
        SLint32 *, pNumAudioInputs,
        SLuint32 *, pAudioInputDeviceIDs
      );
      OPENSLES_OBJECT_FN_3(GetAssociatedAudioOutputs,
        SLuint32, deviceId,
        SLint32 *, pNumAudioOutputs,
        SLuint32 *, pAudioOutputDeviceIDs
      );
      OPENSLES_OBJECT_FN_3(GetDefaultAudioDevices,
        SLuint32, defaultDeviceID,
        SLint32 *, pNumAudioDevices,
        SLuint32 *, pAudioDeviceIDs
      );
      OPENSLES_OBJECT_FN_4(QuerySampleFormatsSupported,
        SLuint32, deviceId,
        SLmilliHertz, samplingRate,
        SLint32 *, pSampleFormats,
        SLint32 *, pNumOfSampleFormats
      );
  };


  class Volume : public Interface<SLVolumeItf, &SL_IID_VOLUME>
  {
    public:
      OPENSLES_OBJECT_FN_1(SetVolumeLevel,
        SLmillibel, level
      )
      OPENSLES_OBJECT_FN_1(GetVolumeLevel,
        SLmillibel *, pLevel
      )
      OPENSLES_OBJECT_FN_1(GetMaxVolumeLevel,
        SLmillibel *, pMaxLevel
      )
      OPENSLES_OBJECT_FN_1(SetMute,
        SLboolean, mute
      )
      OPENSLES_OBJECT_FN_1(GetMute,
        SLboolean *, pMute
      )
      OPENSLES_OBJECT_FN_1(EnableStereoPosition,
        SLboolean, enable
      )
      OPENSLES_OBJECT_FN_1(IsEnabledStereoPosition,
        SLboolean *, pEnable
      )
      OPENSLES_OBJECT_FN_1(SetStereoPosition,
        SLpermille, stereoPosition
      )
      OPENSLES_OBJECT_FN_1(GetStereoPosition,
        SLpermille *, pStereoPosition
      )
  };


#ifdef __ANDROID__

  class BufferQueue : public Interface<SLAndroidSimpleBufferQueueItf, &SL_IID_ANDROIDSIMPLEBUFFERQUEUE>
  {
    public:
      typedef SLAndroidSimpleBufferQueueState    State;
      typedef slAndroidSimpleBufferQueueCallback Callback;

      struct Locator : public SLDataLocator_BufferQueue
      {
        Locator(SLuint32 bufs)
        {
          locatorType = SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE;
          numBuffers = bufs;
        }
      };

      OPENSLES_OBJECT_FN_2(Enqueue,
	const void *, pBuffer,
	SLuint32, size
      )
      OPENSLES_OBJECT_FN_0(Clear)
      OPENSLES_OBJECT_FN_1(GetState,
	SLAndroidSimpleBufferQueueState *, pState
      )
      OPENSLES_OBJECT_FN_2(RegisterCallback,
        slAndroidSimpleBufferQueueCallback, callback,
        void *, pContext
      )
  };


  class AndroidConfiguration : public Interface<SLAndroidConfigurationItf, &SL_IID_ANDROIDCONFIGURATION>
  {
    public:
      OPENSLES_OBJECT_FN_3(SetConfiguration,
        const SLchar *, configKey,
        const void *, pConfigValue,
        SLuint32, valueSize
      )

      OPENSLES_OBJECT_FN_3(GetConfiguration,
        const SLchar *, configKey,
        SLuint32 *, pValueSize,
        void *, pConfigValue
      )

      SLresult SetStreamType(SLint32 stream)
      {
        return SetConfiguration(SL_ANDROID_KEY_STREAM_TYPE, &stream, sizeof(stream));
      }

      SLresult SetRecordinPreset(SLint32 stream)
      {
        return SetConfiguration(SL_ANDROID_KEY_RECORDING_PRESET, &stream, sizeof(stream));
      }
  };

#else // __ANDROID__

  class BufferQueue : public Interface<SLBufferQueueItf, &SL_IID_BUFFERQUEUE>
  {
    public:
      typedef SLBufferQueueState    State;
      typedef slBufferQueueCallback Callback;

      struct Locator : public SLDataLocator_BufferQueue
      {
        Locator(SLuint32 bufs)
        {
          locatorType = SL_DATALOCATOR_BUFFERQUEUE;
          numBuffers = bufs;
        }
      };

      OPENSLES_OBJECT_FN_2(Enqueue,
	const void *, pBuffer,
	SLuint32, size
      )
      OPENSLES_OBJECT_FN_0(Clear)
      OPENSLES_OBJECT_FN_1(GetState,
	SLBufferQueueState *, pState
      )
      OPENSLES_OBJECT_FN_2(RegisterCallback,
        slBufferQueueCallback, callback,
        void *, pContext
      )
  }

#endif // __ANDROID__

};


// End of file
