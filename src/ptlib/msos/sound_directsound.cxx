/*
 * sound_directsound.cxx
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

#pragma implementation "sound_directsound.h"

#include <ptlib.h>

#if P_DIRECTSOUND

#include <ptlib/pprocess.h>
#include <algorithm>

#define INITGUID
#include <ptlib/msos/ptlib/sound_directsound.h>
#include <ptlib/msos/ptlib/sound_win32.h>

#include <tchar.h>
#include <math.h>

#include <ks.h>
#include <dsconf.h>
typedef HRESULT (STDAPICALLTYPE *LPFNDLLGETCLASSOBJECT )(REFCLSID, REFIID, LPVOID * );

// It takes a lot of fiddling in configure to find dxerr.h
// but don't worry about it, it's not essential, see GetErrorText
#ifdef P_DIRECTSOUND_DXERR_H
#include <dxerr.h>    // for DirectSound DXGetErrorDescription9
#pragma comment(lib, "dxerr.lib")
#endif

#ifdef _MSC_VER
  #pragma comment(lib, "dsound.lib")
  #pragma message("Direct Sound support enabled")
#endif

// I made up these HRESULT facility codes to simplify MME error reporting in GetErrorText
#define FACILITY_WAVEIN  100
#define FACILITY_WAVEOUT 101

/* Instantiate the PWLIBsound plugin */ 
PCREATE_SOUND_PLUGIN(DirectSound, PSoundChannelDirectSound)


#ifdef _WIN32_WCE
#include <initguid.h>
#define IID_IDirectSoundBuffer8 IID_IDirectSoundBuffer
#define IID_IDirectSoundCaptureBuffer8 IID_IDirectSoundCaptureBuffer
DEFINE_GUID(DSDEVID_DefaultPlayback, 0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
DEFINE_GUID(DSDEVID_DefaultCapture, 0xdef00001, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
DEFINE_GUID(DSDEVID_DefaultVoicePlayback, 0xdef00002, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
DEFINE_GUID(DSDEVID_DefaultVoiceCapture, 0xdef00003, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
#endif


///////////////////////////////////////////////////////////////////////////////
// Enumeration of devices

// Establish access to system-wide DirectSound device properties

HRESULT DirectSoundPrivateCreate (OUT LPKSPROPERTYSET * outKsPropertySet) 
{ 
  HMODULE dSoundModule = LoadLibrary(TEXT("dsound.dll"));
  if (!dSoundModule)
    return DSERR_GENERIC;
  
  LPFNDLLGETCLASSOBJECT getClassObjectFn = (LPFNDLLGETCLASSOBJECT)GetProcAddress(dSoundModule, "DllGetClassObject");
  if (!getClassObjectFn) {
    FreeLibrary(dSoundModule);
    return DSERR_GENERIC;
  } 
  // Create a class factory object
  LPCLASSFACTORY classFactory = NULL;
  HRESULT result = getClassObjectFn(CLSID_DirectSoundPrivate, IID_IClassFactory, (LPVOID *)&classFactory);
  
  // Create the DirectSoundPrivate object and query for an IKsPropertySet interface
  LPKSPROPERTYSET ksPropertySet = NULL;
  if (SUCCEEDED(result))
    result = classFactory->CreateInstance(NULL, IID_IKsPropertySet, (LPVOID *)&ksPropertySet);
  
  if (classFactory)
    classFactory->Release();
  
  if (SUCCEEDED(result))
    *outKsPropertySet = ksPropertySet;
  
  else if (ksPropertySet)
    ksPropertySet->Release();
  
  FreeLibrary(dSoundModule);
  return result;
} 


// Enumerate all DSound devices, performing callback provided with context for each device

HRESULT EnumerateDSoundDeviceInfo (LPFNDIRECTSOUNDDEVICEENUMERATECALLBACK callback, LPVOID context)
{ 
  LPKSPROPERTYSET ksPropertySet = NULL;
  HRESULT result = DirectSoundPrivateCreate(&ksPropertySet);
  if (FAILED(result))
    return result;
  
  DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE_DATA enumerateData;
  enumerateData.Callback = callback;
  enumerateData.Context = context;
  result = ksPropertySet->Get(DSPROPSETID_DirectSoundDevice,
    DSPROPERTY_DIRECTSOUNDDEVICE_ENUMERATE, // DSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION doesn't return wave ID!
    NULL,
    0,
    &enumerateData,
    sizeof(enumerateData),
    NULL);
  ksPropertySet->Release();
  return result;
} 


// Structures for working with DirectSound device properties

typedef struct {
  GUID DeviceId;                       // directSound id of device we want wave id for
  PSoundChannelDirectSound::Directions Direction; // direction of the device
  TCHAR Description[MAXPNAMELEN];      // device name (this may be char or unicode)
  int WaveDeviceId;                    // matching multimedia system device id

  PString GetDescription (void)
  {
    return PString(Description); // convert from unicode if necessary
  }
  bool operator==(PString other)
  {
    return other == Description; // PStrings are always char
  }
  bool operator==(GUID other)
  {
    return IsEqualGUID(other, DeviceId) != 0;
  }
} DSoundDeviceInfo;

typedef vector<DSoundDeviceInfo> DSoundDeviceInfoVector;

typedef struct {
  DSoundDeviceInfo filter;
  DSoundDeviceInfoVector * result;
} DSoundDeviceFilterInfo;


// DIRECTSOUNDDEVICE_ENUMERATE callback adds new DSoundDeviceInfo to results list

BOOL CALLBACK DSoundDeviceEnumCallBackFilter (PDSPROPERTY_DIRECTSOUNDDEVICE_DESCRIPTION_DATA dsDescription, LPVOID context)
{
  DSoundDeviceFilterInfo * filterInfo = (DSoundDeviceFilterInfo *)context;
  if (filterInfo->result == NULL)
    return TRUE;

  if (!IsEqualGUID(GUID_NULL, filterInfo->filter.DeviceId) && dsDescription->DeviceId != filterInfo->filter.DeviceId)
    return TRUE;  // reject unmatching DeviceId

  PSoundChannelDirectSound::Directions direction = (dsDescription->DataFlow == DIRECTSOUNDDEVICE_DATAFLOW_RENDER)? PSoundChannelDirectSound::Player
                                                                                                             : PSoundChannelDirectSound::Recorder;
  if (filterInfo->filter.Direction != PSoundChannelDirectSound::Closed && direction != filterInfo->filter.Direction)
    return TRUE;  // reject unmatching Direction

  if (_tcslen(filterInfo->filter.Description) != 0 && _tcsnicmp(dsDescription->Description, filterInfo->filter.Description, MAXPNAMELEN) != 0)
    return TRUE;  // reject unmatching Description

  if (filterInfo->filter.WaveDeviceId >= 0 && dsDescription->WaveDeviceId != (ULONG)filterInfo->filter.WaveDeviceId)
    return TRUE;  // reject unmatching WaveDeviceId

  DSoundDeviceInfo info;
  info.DeviceId = dsDescription->DeviceId;
  info.Direction = direction;
  _tcsncpy_s(info.Description, MAXPNAMELEN, dsDescription->Description, _TRUNCATE); // Make name compatible with MultiMedia version
  info.WaveDeviceId = dsDescription->WaveDeviceId;
  filterInfo->result->push_back(info);
  return TRUE;
}


// Populate a DSoundDeviceInfo vector with devices that satisfy filter criteria
// Specify GUID_NULL to accept any DeviceId
// Specify Closed to accept any direction
// Specify empty name to accept any device name
// Specify -1 to accept any WaveDeviceId

HRESULT GetFilteredDSoundDeviceInfo (const GUID & deviceId, PSoundChannelDirectSound::Directions direction, PString name, int waveDeviceId, DSoundDeviceInfoVector & info)
{ 
  DSoundDeviceFilterInfo filterInfo;
  filterInfo.filter.DeviceId = deviceId;
  filterInfo.filter.Direction = direction;
  if (name.GetLength())
    _tcsncpy_s(filterInfo.filter.Description, name.GetPointer(MAXPNAMELEN), MAXPNAMELEN);
  else
    filterInfo.filter.Description[0] = 0;

  filterInfo.filter.WaveDeviceId = waveDeviceId;
  filterInfo.result = &info;
  return EnumerateDSoundDeviceInfo(DSoundDeviceEnumCallBackFilter, &filterInfo);
} 


// Get the DSoundDeviceInfo for the default device (communications or audio) in the given direction

HRESULT GetDefaultDeviceGUID (PString name, PSoundChannelDirectSound::Directions direction, GUID & guid) // static
{
  PAssert(direction == PSoundChannelDirectSound::Player || direction == PSoundChannelDirectSound::Recorder, "Invalid device direction parameter");

  LPCGUID defaultGUID = NULL;
  if ((name *= "Default communications") && PProcess::IsOSVersion(6, 1)) // Windows 7
    defaultGUID = (direction == PSoundChannelDirectSound::Player)? &DSDEVID_DefaultVoicePlayback  : &DSDEVID_DefaultVoiceCapture;
  else {
    defaultGUID = (direction == PSoundChannelDirectSound::Player)? &DSDEVID_DefaultPlayback  : &DSDEVID_DefaultCapture;
    if (!((name *= "Default audio") || (name *= "Default")))
      PTRACE(4, "dsound\tOpen " << PSoundChannelDirectSound::GetDirectionText(direction) << ": device \"" << name << "\" not found, substituting default");
  }
  return GetDeviceID(defaultGUID, &guid);
}


///////////////////////////////////////////////////////////////////////////////
// Construction

PSoundChannelDirectSound::PSoundChannelDirectSound ()
{
  Construct();
}


PSoundChannelDirectSound::PSoundChannelDirectSound (const PString &device,
                                                    Directions dir,
                                                    unsigned numChannels,
                                                    unsigned sampleRate,
                                                    unsigned bitsPerSample)
{
  Construct();
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}


void PSoundChannelDirectSound::Construct ()
{
  m_captureDevice = NULL;
  m_captureBuffer = NULL;

  m_playbackDevice = NULL;
  m_primaryPlaybackBuffer = NULL;
  m_playbackBuffer = NULL;

  m_isStreaming = true;
  m_bufferSize = 0;
  m_mixer = NULL;

  memset(&m_waveFormat, 0, sizeof(m_waveFormat)); 
  m_triggerEvent[SOUNDEVENT_SOUND] = CreateEvent(NULL, false, false, NULL);// auto-reset
  m_triggerEvent[SOUNDEVENT_CLOSE] = CreateEvent(NULL, false, false, NULL);// auto-reset

  SetBuffers(16000, 3); // 3 seconds at 8kHz
}


PSoundChannelDirectSound::~PSoundChannelDirectSound ()
{
  Close();
  if (m_triggerEvent[SOUNDEVENT_SOUND] != NULL)
    CloseHandle(m_triggerEvent[SOUNDEVENT_SOUND]);

  if (m_triggerEvent[SOUNDEVENT_CLOSE] != NULL)
    CloseHandle(m_triggerEvent[SOUNDEVENT_CLOSE]);

  PTRACE(4, "dsound\t" << GetDirectionText() << " destroyed");
}


///////////////////////////////////////////////////////////////////////////////
// Open


PString PSoundChannelDirectSound::GetDefaultDevice (Directions dir) // static
{
  PAssert(dir == Player || dir == Recorder, "Invalid device direction parameter");

  GUID deviceGUID;
  HRESULT result = GetDeviceID((dir == Player)? &DSDEVID_DefaultPlayback  : &DSDEVID_DefaultCapture, &deviceGUID);
  if (result != S_OK) {
    PTRACE(4, "dsound\tCould not find default device: " << GetErrorText(Miscellaneous, result));
    return PString();
  }
  DSoundDeviceInfoVector devices;
  result = GetFilteredDSoundDeviceInfo(deviceGUID, dir, PString::Empty(), -1, devices);
  if (result != S_OK) {
    PTRACE(4, "dsound\tOpen: Could not retrieve device information: " << GetErrorText(Miscellaneous, result));
    return PString();
  }
  if (devices.size() == 0) {
    PTRACE(4, "dsound\tOpen: Default device not found");
    return PString();
  }
  return devices[0].GetDescription(); // W2A char conversion if unicode
}


PStringArray PSoundChannelDirectSound::GetDeviceNames (Directions dir) // static
{
  PAssert(dir == Player || dir == Recorder, "Invalid device direction parameter");

  DSoundDeviceInfoVector devices;
  HRESULT result = GetFilteredDSoundDeviceInfo(GUID_NULL, dir, PString::Empty(), -1, devices);
  if (result != S_OK) {
    PTRACE(4, "dsound\tCould not get device list: " << GetErrorText(Miscellaneous, result));
    return PString();
  }
  PStringArray names;
  if (devices.size() > 0)
  {
    if (PProcess::IsOSVersion(6, 1)) { // Windows 7
      names.AppendString("Default audio");
      names.AppendString("Default communications");
    }
    else
      names.AppendString("Default");
  }
  DSoundDeviceInfoVector::const_iterator iterator = devices.begin();
  while (iterator != devices.end()) {
    names.AppendString(iterator->Description); // PString ctor does W2A char conversion if Description is unicode
    iterator++;
  }
  return names;
}


PBoolean PSoundChannelDirectSound::Open (const PString & device, // public
                                         Directions dir,
                                         unsigned numChannels,
                                         unsigned sampleRate,
                                         unsigned bitsPerSample)
{
  PAssert(dir == Player || dir == Recorder, "Invalid device direction parameter");

  Close();

  // remove the driver name prefix
  PINDEX tab = device.Find(PDevicePluginServiceDescriptor::SeparatorChar);
  m_deviceName = (tab == P_MAX_INDEX)? device : device.Mid(tab+1).Trim();

  activeDirection = dir;
  m_available = 0;
  m_dsMoved = 0ui64;
  m_moved = 0ui64;
  m_lost = 0ui64;

  // get all devices for direction
  DSoundDeviceInfoVector devices;
  HRESULT result = GetFilteredDSoundDeviceInfo(GUID_NULL, dir, PString::Empty(), -1, devices);
  if (result != S_OK) {
    SetErrorValues(Miscellaneous, result);
    PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not get device list: " << GetErrorText());
    return false;
  }
  // validate the device name
  DSoundDeviceInfoVector::iterator deviceInfo = find(devices.begin(), devices.end(), m_deviceName);
  GUID deviceGUID;
  if (deviceInfo == devices.end()) {
    result = GetDefaultDeviceGUID(m_deviceName, activeDirection, deviceGUID);
    if (result != S_OK) {
      SetErrorValues(Miscellaneous, result);
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not get default device ID: " << GetErrorText());
      return false;
    }
    deviceInfo = find(devices.begin(), devices.end(), deviceGUID);
    if (deviceInfo == devices.end()) {
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not find default device");
      return SetErrorValues(NotFound, HRESULT_FROM_WIN32(ERROR_NOT_FOUND));
    }
    m_deviceName = deviceInfo->GetDescription();
  }
  else
    deviceGUID = deviceInfo->DeviceId;

  PTRACE(4, "dsound\tOpening " << GetDirectionText() << " device \"" << m_deviceName << '"');

  // open for playback
  if (activeDirection == Player) {
    HRESULT result = DirectSoundCreate8(&deviceGUID, &m_playbackDevice, NULL);
    if (result != S_OK) {
      SetErrorValues(Miscellaneous, result);
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not create device: " << GetErrorText());
      return false;
    }

    HWND window = GetForegroundWindow();
    if (window == NULL)
      window = GetDesktopWindow();

    result = m_playbackDevice->SetCooperativeLevel(window, DSSCL_PRIORITY);
    if (result != S_OK) {
      SetErrorValues(Miscellaneous, result);
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not set cooperative level: " << GetErrorText());
      m_playbackDevice.Release();
      return false;
    }

    DSBUFFERDESC bufferDescription = {
      sizeof(DSBUFFERDESC),        // dwSize
      DSBCAPS_PRIMARYBUFFER        // dwFlags
    };                             // dwBufferBytes, dwReserved, lpwfxFormat, guid3DAlgorithm = 0
    result = m_playbackDevice->CreateSoundBuffer(&bufferDescription, &m_primaryPlaybackBuffer, NULL);
    if (FAILED(result)) {
      SetErrorValues(Miscellaneous, result);
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not create primary buffer: " << GetErrorText());
      m_playbackDevice.Release();
      return false;
    }
  }
  else { // open for recording
    HRESULT result = DirectSoundCaptureCreate8(&deviceGUID, &m_captureDevice, NULL);
    if (result != S_OK) {
      SetErrorValues(Miscellaneous, result);
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Could not create device: " << GetErrorText());
      return false;
    }
    // Hook up to mixer volume control for device
    mixerOpen(&m_mixer, (UINT)deviceInfo->WaveDeviceId, NULL, NULL, MIXER_OBJECTF_WAVEIN);
    if (m_mixer == NULL)
      PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Failed to open mixer - volume control will not function");
    else {
      MIXERLINE line = { sizeof(MIXERLINE) };
      line.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
      if (mixerGetLineInfo((HMIXEROBJ)m_mixer, &line, MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE) != MMSYSERR_NOERROR) {
        PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Failed to access mixer line - volume control will not function");
        mixerClose(m_mixer);
        m_mixer = NULL;
      }
      else {
        m_volumeControl.cbStruct = sizeof(m_volumeControl);

        MIXERLINECONTROLS controls;
        controls.cbStruct = sizeof(controls);
        controls.dwLineID = line.dwLineID & 0xffff;
        controls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
        controls.cControls = 1;
        controls.pamxctrl = &m_volumeControl;
        controls.cbmxctrl = m_volumeControl.cbStruct;

        if (mixerGetLineControls((HMIXEROBJ)m_mixer, &controls, MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE) != MMSYSERR_NOERROR) {
          PTRACE(4, "dsound\tOpen" << GetDirectionText() << ": Failed to configure volume control - volume control will not function");
          mixerClose(m_mixer);
          m_mixer = NULL;
        }
      }
    }
  }
  if (!SetFormat(numChannels, sampleRate, bitsPerSample))
    return false;

  if (m_notifier.GetObject() != NULL) { // notify that channel is starting
    m_notifier(*this, SOUNDNOTIFY_UNDERRUN);
    m_notifier(*this, SOUNDNOTIFY_OVERRUN);
  }
  return true;
}


PBoolean PSoundChannelDirectSound::Abort () // public
{
  if (IsOpen())
  {
    SetEvent(m_triggerEvent[SOUNDEVENT_CLOSE]); // signal read or write to yield
    PWaitAndSignal mutex(m_bufferMutex); // wait until read or write yields access to buffer
    switch (activeDirection) {
    case Player:
      if (m_playbackBuffer != NULL) {
        PTRACE(4, "dsound\tClosing playback device \"" << m_deviceName << '"');
        m_playbackBuffer->Stop();
        int notification;
        CheckPlayBuffer(notification); // last effort to see how many bytes played
      }
      m_playbackBuffer.Release();
      break;

    case Recorder:
      if (m_captureBuffer != NULL) {
        PTRACE(4, "dsound\tClosing recording device \"" << m_deviceName << '"');
        m_captureBuffer->Stop();
      }
      m_captureBuffer.Release();
      break;
    }
  }
  // Reset these even when opening
  m_movePos = 0; // reset public read/write position
  m_dsPos = 0; // DirectSound read/write position
  m_tick.SetInterval(0i64);
  ResetEvent(m_triggerEvent[SOUNDEVENT_CLOSE]);// (in case called while not reading or writing)
  return true;
}


PBoolean PSoundChannelDirectSound::Close () // public
{
  PWaitAndSignal mutex(m_bufferMutex);

  Abort(); // abort waiting for I/O & destroy buffers

  if (m_mixer != NULL) {
    mixerClose(m_mixer);
    m_mixer = NULL;
  }
  switch (activeDirection) {
  case Player:
    m_primaryPlaybackBuffer.Release();
    m_playbackDevice.Release();
    PTRACE(4, "dsound\tPlayback closed: wrote " << GetSamplesMoved() << ", played " << GetSamplesBuffered() << ", lost " <<  GetSamplesLost() << " samples");
    break;

  case Recorder:
    m_captureDevice.Release();
    PTRACE(4, "dsound\tRecording closed: captured " << GetSamplesBuffered() << ", read " << GetSamplesMoved() << ", lost " <<  GetSamplesLost() << " samples");
    break;
  }
  activeDirection = Closed;
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Setup

PBoolean PSoundChannelDirectSound::SetFormat (unsigned numChannels, // public
                                              unsigned sampleRate,
                                              unsigned bitsPerSample)
{
  Abort(); // abort waiting for I/O & destroy buffers

  memset(&m_waveFormat, 0, sizeof(m_waveFormat)); 
  m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  m_waveFormat.nChannels = (WORD)numChannels;
  m_waveFormat.nSamplesPerSec = sampleRate;
  m_waveFormat.wBitsPerSample = (WORD)bitsPerSample;
  m_waveFormat.nBlockAlign = m_waveFormat.nChannels * ((m_waveFormat.wBitsPerSample + 7) / 8);
  m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;
  PTRACE(4, "dsound\t" << GetDirectionText() << " SetFormat:"
    << " Channels " << m_waveFormat.nChannels
    << " Rate " << m_waveFormat.nSamplesPerSec
    << " Bits " << m_waveFormat.wBitsPerSample);
  return true; // no buffers yet
}


PBoolean PSoundChannelDirectSound::SetBuffers (PINDEX size, PINDEX count) // public
{
  Abort(); // abort waiting for I/O & destroy buffers

  if (size < DSBSIZE_MIN || size > DSBSIZE_MAX)
  {
    PTRACE(4, "dsound\t" << GetDirectionText() << " SetBuffers: invalid buffer size " << size << " bytes");
    return SetErrorValues(BadParameter, E_INVALIDARG);
  }
  m_bufferSectionCount = count;
  m_bufferSectionSize = size; 
  m_bufferSize = m_bufferSectionCount * m_bufferSectionSize;

  PTRACE(4, "dsound\t" << GetDirectionText() << " SetBuffers: count " << m_bufferSectionCount << " x size " << m_bufferSectionSize << " = " << m_bufferSize << " bytes");
  return true;
}


PBoolean PSoundChannelDirectSound::GetBuffers (PINDEX & size, PINDEX & count) // public
{
  count = m_bufferSectionCount;
  size = m_bufferSectionSize;
  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Error reporting


PString PSoundChannelDirectSound::GetErrorText (Errors lastError, int osError) // static
{
  PString text;
  DWORD facility = HRESULT_FACILITY((HRESULT)osError);
#ifdef P_DIRECTSOUND_DXERR_H
  if (facility == _FACDS) { // DirectX errors not available in FormatMessage
    return text = DXGetErrorDescription(osError);
  }
  else if (facility <= 84) { // Standard Windows errors (WinError.h)
#else
  if (facility == _FACDS || facility <= 84) { // handle DSound and standard Windows errors (WinError.h)
#endif
    if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 0, osError, 0, text.GetPointer(256), 255, 0) == 0) {
      // FormatMessage error or unknown HRESULT
      return psprintf("HRESULT 0x%08X", osError);
    }
  }
  else if (facility == FACILITY_WAVEIN) {
    MMRESULT mmError = HRESULT_CODE(osError);
    if (waveInGetErrorText(mmError, text.GetPointer(256), 256) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(lastError, mmError);
  }
  else if (facility == FACILITY_WAVEOUT) {
    MMRESULT mmError = HRESULT_CODE(osError);
    if (waveOutGetErrorText(mmError, text.GetPointer(256), 256) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(lastError, mmError);
  }
  else
    return PChannel::GetErrorText(lastError, osError);

  // strip trailing CR/LF
  int Length = text.GetLength();
  int Index = Length;
  while (Index > 0 && (text[Index - 1] == '\r' || text[Index - 1] == '\n'))
    Index--;

  return (Index < Length)? text.Mid(0, Index) : text;
}


// Override of PChannel virtual method to ensure PSoundChannelDirectSound's static method is used

PString PSoundChannelDirectSound::GetErrorText(ErrorGroup group) const
{
  return GetErrorText(lastErrorCode[group], lastErrorNumber[group]);
}


///////////////////////////////////////////////////////////////////////////////
// Performance monitoring


// measure time since last check

PTimeInterval PSoundChannelDirectSound::GetInterval (void)
{
  PTimeInterval tick = PTimer::Tick();
  PTimeInterval Interval = (m_tick.GetInterval() == 0i64)? 0i64 : (tick - m_tick);
  m_tick = tick;
  return Interval;
}


// Get the number of entire Buffers that DirectSound has filled or emptied since last
// time we checked, to detect buffer wrap around

DWORD PSoundChannelDirectSound::GetCyclesPassed (void)
{
  // measure time since last check
  PTimeInterval interval = GetInterval();
  // convert time to number of buffers filled
  DWORD bytes = interval.GetInterval() * m_waveFormat.nAvgBytesPerSec / 1000;
  return bytes / m_bufferSize;
}


///////////////////////////////////////////////////////////////////////////////
// Playing

PBoolean PSoundChannelDirectSound::InitPlaybackBuffer ()
{
  DSBUFFERDESC bufferDescription = {
    sizeof(DSBUFFERDESC),
    DSBCAPS_GLOBALFOCUS + DSBCAPS_CTRLPOSITIONNOTIFY + DSBCAPS_GETCURRENTPOSITION2 + DSBCAPS_CTRLVOLUME,
    m_bufferSize,                 // calculated by SetBuffers
    0,                            // reserved
    &m_waveFormat                 // format
  };
  HRESULT result = m_playbackDevice->CreateSoundBuffer(&bufferDescription, &m_playbackBuffer, NULL); 
  if (FAILED(result)) { 
    SetErrorValues(Miscellaneous, result);
    PTRACE(4, "dsound\tInitPlaybackBuffer: CreateSoundBuffer failed: " << GetErrorText());
    return false;
  } 
  IDirectSoundNotify * notify; // temporary pointer to the interface
  result = m_playbackBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &notify);
  if (FAILED(result)) { 
    SetErrorValues(Miscellaneous, result);
    PTRACE(4, "dsound\tInitPlayBuffer: notify interface query failed: " << GetErrorText());
    m_playbackBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitPlayBuffer: Setting up notification for " << m_bufferSectionCount << " blocks of " << m_bufferSectionSize << " bytes");
  DSBPOSITIONNOTIFY * position = new DSBPOSITIONNOTIFY[m_bufferSectionCount];
  if (position == 0) {
    SetErrorValues(NoMemory, E_OUTOFMEMORY);
    PTRACE(4, "dsound\tInitPlayBuffer: notify allocation failed");
    notify->Release();
    m_playbackBuffer.Release();
    return false;
  }
  DWORD blockOffset = m_bufferSectionSize - 1;
  for (PINDEX i = 0; i < m_bufferSectionCount; i++) {
    position[i].dwOffset = blockOffset;
    position[i].hEventNotify = m_triggerEvent[SOUNDEVENT_SOUND]; // all use same event
    blockOffset += m_bufferSectionSize;
  }
  result = notify->SetNotificationPositions(m_bufferSectionCount, position);
  notify->Release();
  if (FAILED(result)) { 
    SetErrorValues(Miscellaneous, result);
    PTRACE(4, "dsound\tInitPlaybackBuffer: Notify interface query failed: " << GetErrorText());
    m_playbackBuffer.Release();
    return false;
  }
  delete [] position;
  m_playbackBuffer->SetCurrentPosition(0);
  return true;
}


PBoolean PSoundChannelDirectSound::CheckPlayBuffer (int & notification)
{
  notification = SOUNDNOTIFY_NOTHING;
  PWaitAndSignal mutex(m_bufferMutex); // make Abort wait

  if (!m_playbackBuffer) // closed
    return false;
                  // Write is ahead of Play, DirectSound is playing data from Play to Write - do not put data between them.
                  // we can write ahead of Write, from m_movePos to PlayPos
  DWORD playPos;  // circular buffer byte offset to start of section DirectSound is playing (end of where we can write)
  DWORD writePos; // circular buffer byte offset ahead of which it is safe to write data (start of where we can write)
  HRESULT result = m_playbackBuffer->GetCurrentPosition(&playPos, &writePos);
  if (FAILED(result)) {
    SetErrorValues(Miscellaneous, result, LastWriteError);
    notification = SOUNDNOTIFY_ERROR;
    return false;
  }
  // Record the driver performance, include check for writer getting way behind player
  // bytes played since last check
  DWORD played = (playPos + m_bufferSize - m_dsPos) % m_bufferSize;
  m_dsPos = playPos;
  // add in bytes from possible complete buffer cyclings since last check
  played += GetCyclesPassed() * m_bufferSize;
  // count bytes played since open
  m_dsMoved += played;
  
  DWORD status;
  result = m_playbackBuffer->GetStatus(&status);
  if (FAILED(result)) {
    SetErrorValues(Miscellaneous, result, LastWriteError);
    notification = SOUNDNOTIFY_ERROR;
    return false;
  }
  if ((status & DSBSTATUS_PLAYING) == 0) { // not started yet, or we let it run empty (not looping)
    m_available = m_bufferSize;
    return true;
  }
  // check for underrun
  if (m_dsMoved > m_moved + m_lost)
  { // Write has been delayed so much that card is playing from empty or old space
    unsigned underruns = (unsigned)(m_dsMoved - (m_moved + m_lost));
    m_lost += underruns;
    PTRACE(3, "dsound\tPlayback underrun: wrote " << GetSamplesMoved() << " played " << GetSamplesBuffered() << " replayed " << underruns / m_waveFormat.nBlockAlign << " total loss " << GetSamplesLost() << " samples");
    notification = SOUNDNOTIFY_UNDERRUN;
    m_movePos = writePos;
  }
  // calculate available space in circular buffer (between our last write position and DirectSound read position)
  m_available = (playPos + m_bufferSize - m_movePos) % m_bufferSize;
  if (m_available >= (unsigned)m_bufferSectionSize)
    return true;

  // some space, but not enough yet
  PTRACE(6, "dsound\tPlayer buffer overrun, waiting for space");
  notification = SOUNDNOTIFY_OVERRUN;
  return false;
}


PBoolean PSoundChannelDirectSound::WaitForPlayBufferFree ()
{
  ResetEvent(m_triggerEvent[SOUNDEVENT_SOUND]);
  do {
    int notification = SOUNDNOTIFY_NOTHING;
    PBoolean isSpaceAvailable = CheckPlayBuffer(notification);
    // report errors
    if (notification != SOUNDNOTIFY_NOTHING && m_notifier.GetObject() != NULL)
      m_notifier(*this, notification); // can close here!

    if (!m_playbackBuffer) { // closed
      PTRACE(4, "dsound\tPlayback abort");
      return false;
    }
    if (isSpaceAvailable) // Ok to write
      return true;

  }  // wait for DirectSound to notify us that space is available
  while (WaitForMultipleObjects(2, m_triggerEvent, FALSE, INFINITE) == WAIT_OBJECT_0);
  PTRACE(4, "dsound\tPlayback abort");
  return false; // closed
}


PBoolean PSoundChannelDirectSound::Write (const void *buf, PINDEX len) // public
{
  PAssert(activeDirection == Player, "Invalid device direction");
  PAssertNULL(buf);
  {
    PWaitAndSignal mutex(m_bufferMutex); // prevent closing while active
    lastWriteCount = 0;
    if (!IsOpen()) {                    // closed
      SetErrorValues(NotOpen, EBADF, LastWriteError);
      PTRACE(4, "dsound\tWrite: Device not initialised");
      return false;
    }
    if (!m_playbackBuffer && !InitPlaybackBuffer())
      return false;
  }                                     // unlock to allow Abort while waiting for buffer
  char * src = (char *) buf;
  while (lastWriteCount < len) {
    // wait for output space to become available
    if (!WaitForPlayBufferFree()) {     // sets m_movePos and m_available
      SetErrorValues(NotOpen, EBADF, LastWriteError);
      return false;                     // closed
    }
    {
      PWaitAndSignal mutex(m_bufferMutex);  // prevent closing while writing
      LPVOID pointer1, pointer2;
      DWORD length1, length2;
      HRESULT Result = m_playbackBuffer->Lock(m_movePos, PMIN((PINDEX)m_available, len),
                                              &pointer1, &length1, &pointer2, &length2, 0L);
      if (Result == DSERR_BUFFERLOST) {   // Buffer was lost, need to restore it
        m_playbackBuffer->Restore();
        Result = m_playbackBuffer->Lock(m_movePos, PMIN((PINDEX)m_available, len),
                                        &pointer1, &length1, &pointer2, &length2, 0L);
      }
      if (FAILED(Result)) {
        SetErrorValues(Miscellaneous, Result, LastWriteError);
        PTRACE(1, "dsound\tWrite failure: " << GetErrorText() << " len " << len << " pos " << m_movePos);
        return false;
      }
      // Copy supplied buffer into locked DirectSound memory
      memcpy((char *)pointer1, src, length1);
      if (pointer2 != NULL)
        memcpy(pointer2, (char *)src + length1, length2);

      m_playbackBuffer->Unlock(pointer1, length1, pointer2, length2);

      PINDEX writeCount = length1 + length2;
      src += writeCount;
      len -= writeCount;
      lastWriteCount += writeCount;
      m_movePos += lastWriteCount;
      m_movePos %= m_bufferSize;
      m_moved += writeCount;
                                          // tell DirectSound to play
      m_playbackBuffer->Play(0, 0, m_isStreaming ? DSBPLAY_LOOPING : 0L);
    }
  }
  return true;
}


PBoolean PSoundChannelDirectSound::HasPlayCompleted () // public
{
  // only works for non-streaming player
  PWaitAndSignal mutex(m_bufferMutex); // prevent closing while active

  if (!m_playbackBuffer)
    return true;

  DWORD status;
  HRESULT result = m_playbackBuffer->GetStatus(&status);
  if (FAILED(result)) {
    SetErrorValues(Miscellaneous, result, LastWriteError);
    return true; // it's done if we get an error here
  }
  return ((status & DSBSTATUS_PLAYING) == 0);
}


PBoolean PSoundChannelDirectSound::WaitForPlayCompletion () // public
{
  // only works for non-streaming player
  while (!HasPlayCompleted()) 
    Sleep(50);

  return true;
}


PBoolean PSoundChannelDirectSound::PlaySound (const PSound & sound, PBoolean wait) // public
{
  PAssert(activeDirection == Player, "Invalid device direction");

  if (!SetBuffers(sound.GetSize(), 1)) // Aborts
    return false;

  m_isStreaming = false;

  if (!Write((const void *)sound, sound.GetSize()))
    return false;

  if (wait)
    return WaitForPlayCompletion();

  return true;
}


PBoolean PSoundChannelDirectSound::PlayFile (const PFilePath & filename, PBoolean wait) // public
{
  PAssert(activeDirection == Player, "Invalid device direction");

  PMultiMediaFile mediaFile;
  PWaveFormat fileFormat;
  DWORD dataSize;
  if (!mediaFile.OpenWaveFile(filename, fileFormat, dataSize))
    return SetErrorValues(NotOpen, mediaFile.GetLastError() | PWIN32ErrorFlag, LastWriteError);

  Abort();

  // Save old format and set to one loaded from file.
  unsigned numChannels = m_waveFormat.nChannels;
  unsigned sampleRate = m_waveFormat.nSamplesPerSec;
  unsigned bitsPerSample = m_waveFormat.wBitsPerSample;

  SetFormat(fileFormat->nChannels, fileFormat->nSamplesPerSec, fileFormat->wBitsPerSample);

  int bufferSize = m_waveFormat.nAvgBytesPerSec / 2; // 1/2 second
  if (!SetBuffers(bufferSize, 4))
    SetFormat(numChannels, sampleRate, bitsPerSample);// restore audio format

  PBYTEArray buffer;
  m_isStreaming = false;

  while (dataSize) {
    // Read the waveform data subchunk
    PINDEX count = PMIN(dataSize,((DWORD)bufferSize));
    if (!mediaFile.Read(buffer.GetPointer(bufferSize), count)) {
      SetErrorValues(Miscellaneous, mediaFile.GetLastError() | PWIN32ErrorFlag, LastReadError);
      PTRACE(4, "dsound\tPlayFile read error: " << GetErrorText());
      return false;
    }
    if (!Write(buffer, count))
      break;

    dataSize -= count;
  }
  mediaFile.Close();

  if (wait)
    return WaitForPlayCompletion();

  return true;
}


///////////////////////////////////////////////////////////////////////////////
// Recording

PBoolean PSoundChannelDirectSound::InitCaptureBuffer () 
{
  DSCBUFFERDESC dscbdesc = {
    sizeof(DSCBUFFERDESC),
    DSCBCAPS_WAVEMAPPED,        // DSCBCAPS_CTRLFX(support effects) | DSCBCAPS_WAVEMAPPED(use wave mapper for formats unsupported by device)
    m_bufferSize,               // calculated by SetBuffers
    0,                          // reserved
    &m_waveFormat               // format
  };
  HRESULT Result = m_captureDevice->CreateCaptureBuffer(&dscbdesc, &m_captureBuffer, NULL); 
  if (FAILED(Result)) { 
    SetErrorValues(Miscellaneous, Result);
    PTRACE(4, "dsound\tInitCaptureBuffer: Create Sound Buffer failed: " << GetErrorText());
    return false;
  }
  IDirectSoundNotify * Notify; // temporary pointer to the interface
  Result = m_captureBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &Notify);
  if (FAILED(Result)) { 
    SetErrorValues(Miscellaneous, Result);
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify interface query failed: " << GetErrorText());
    m_captureBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitCaptureBuffer: Setting up notification for " << m_bufferSectionCount << " blocks of " << m_bufferSectionSize << " bytes");
  DSBPOSITIONNOTIFY * Position = new DSBPOSITIONNOTIFY[m_bufferSectionCount];
  if (Position == 0) {
    SetErrorValues(NoMemory, E_OUTOFMEMORY);
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify allocation failed");
    Notify->Release();
    m_captureBuffer.Release();
    return false;
  }
  DWORD BlockOffset = m_bufferSectionSize - 1;
  for (PINDEX i = 0; i < m_bufferSectionCount; i++) {
    Position[i].dwOffset = BlockOffset;
    Position[i].hEventNotify = m_triggerEvent[SOUNDEVENT_SOUND];// all use same event
    BlockOffset += m_bufferSectionSize;
  }
  Result = Notify->SetNotificationPositions(m_bufferSectionCount, Position);
  Notify->Release();
  if (FAILED(Result)) { 
    SetErrorValues(Miscellaneous, Result);
    PTRACE(4, "dsound\tInitCaptureBuffer : Notify interface query failed: " << GetErrorText());
    m_captureBuffer.Release();
    return false;
  }
  delete [] Position;

  return true;
}


PBoolean PSoundChannelDirectSound::StartRecording () // public
{
  PWaitAndSignal mutex(m_bufferMutex);

  if (!IsOpen()) {
    SetErrorValues(NotOpen, EBADF, LastReadError);
    PTRACE(4, "dsound\tStartRecording: Device not initialised");
    return false;
  }
  if (!m_captureBuffer && !InitCaptureBuffer())
    return false;

  DWORD Status = 0;
  HRESULT Result = m_captureBuffer->GetStatus(&Status);
  if (FAILED(Result)) {
    SetErrorValues(Miscellaneous, Result, LastReadError);
    PTRACE(4, "dsound\tStartRecording: Failed GetStatus - " << GetErrorText());
      return false;
  }
  if ((Status & DSCBSTATUS_CAPTURING) != 0)
    return true; // already started

  Result = m_captureBuffer->Start(DSCBSTART_LOOPING);
  if (FAILED(Result)) {
    SetErrorValues(Miscellaneous, Result, LastReadError);
    PTRACE(4, "dsound\tStartRecording: Failed Start - " << GetErrorText());
      return false;
  }
  PTRACE(4, "dsound\tRecorder starting");
  return true;
}


PBoolean PSoundChannelDirectSound::CheckRecordBuffer (int & notification)
{
  notification = SOUNDNOTIFY_NOTHING;
  PWaitAndSignal mutex(m_bufferMutex); // make Abort wait

  if (!m_captureBuffer) // closed
    return false;
                    // Capture is ahead of Read, do not get data from between them. Data for us is at m_movePos behind Read.
  DWORD readPos;    // circular buffer byte offset to the end of the data that has been fully captured (end of what we can read)
  DWORD capturePos; // circular buffer byte offset to the head of the block that card has locked for new data
  HRESULT result = m_captureBuffer->GetCurrentPosition(&capturePos, &readPos);
  if (FAILED(result)) {
    SetErrorValues(Miscellaneous, result, LastReadError);
    notification = SOUNDNOTIFY_ERROR;
    return false;
  }
  // Record the driver performance, include check for reader getting way behind recorder
  // bytes recorded since last check (this count includes stuff we can't even read yet)
  DWORD captured = (capturePos + m_bufferSize - m_dsPos) % m_bufferSize;
  m_dsPos = capturePos;
  // add in bytes from possible complete buffer cycling since last check
  DWORD cycles = GetCyclesPassed();
  captured += cycles * m_bufferSize;
  // count bytes recorded since open
  m_dsMoved += captured;

  // check for overrun
  if (m_dsMoved > m_moved + m_lost + m_bufferSize)
  { // Read has been delayed so much that card is writing into space that we have not read yet
    // overruns are what will never be read
    unsigned overruns = (unsigned)((m_dsMoved - m_bufferSize) - (m_moved + m_lost));
    m_lost += overruns;
    PTRACE(3, "dsound\tRecorder overrun: captured " << GetSamplesBuffered() << " read " << GetSamplesMoved() << " lost " << overruns / m_waveFormat.nBlockAlign << " total loss " << GetSamplesLost() << " samples");
    notification = SOUNDNOTIFY_OVERRUN;
    // Move read position to DirectSound's capture position, where audio has not been overwritten yet
    m_movePos = m_dsPos;
  }
  // calculate available data in circular buffer (between our last write position and DirectSound read position)
  m_available = (readPos + m_bufferSize - m_movePos) % m_bufferSize;
  return m_available >= (unsigned)m_bufferSectionSize;
}


PBoolean PSoundChannelDirectSound::IsRecordBufferFull () // public
{
  if (!StartRecording()) // Start the first read
  {
    PTRACE(4, "dsound\tIsRecordBufferFull: Device not initialised");
    return false;
  }
  int notification = SOUNDNOTIFY_NOTHING;
  PBoolean isDataAvailable = CheckRecordBuffer(notification);
  // report errors
  if (notification != SOUNDNOTIFY_NOTHING && m_notifier.GetObject() != NULL)
    m_notifier(*this, notification); // can close here!

  return m_captureBuffer != NULL && isDataAvailable;
}


PBoolean PSoundChannelDirectSound::WaitForRecordBufferFull () // public
{
  if (!StartRecording()) // Start the first read
  {
    PTRACE(4, "dsound\tIsRecordBufferFull: Device not initialised");
    return false;
  }
  ResetEvent(m_triggerEvent[SOUNDEVENT_SOUND]);
  do {
    int notification = SOUNDNOTIFY_NOTHING;
    PBoolean isDataAvailable = CheckRecordBuffer(notification);
    // report errors
    if (notification != SOUNDNOTIFY_NOTHING && m_notifier.GetObject() != NULL)
      m_notifier(*this, notification); // can close here!

    if (!m_captureBuffer) // closed
    {
      PTRACE(4, "dsound\tRecording abort");
      return false;
    }
    if (isDataAvailable) // Ok to read
      return true;

  }  // wait for DirectSound to notify us that space is available
  while (WaitForMultipleObjects(2, m_triggerEvent, FALSE, INFINITE) == WAIT_OBJECT_0);
  PTRACE(4, "dsound\tRecording abort");
  return false; // closed
}


PBoolean PSoundChannelDirectSound::Read (void * buf, PINDEX len) // public
{
  PAssert(activeDirection == Recorder, "Invalid device direction");
  PAssertNULL(buf);

  lastReadCount = 0;
  if (!StartRecording())                // Start the first read
    return false;

  char * dest = (char *) buf;
  while (lastReadCount < len) {
    if (!WaitForRecordBufferFull()) {   // sets m_movePos and m_available
      SetErrorValues(NotOpen, EBADF, LastReadError);
      return false;                     // closed
    }
    {
      PWaitAndSignal mutex(m_bufferMutex);  // prevent closing while active

      // Read from device buffer minimum between the data required and data available
      LPVOID pointer1, pointer2;
      DWORD length1, length2;
      HRESULT Result = m_captureBuffer->Lock(m_movePos, PMIN((PINDEX)m_available, len),
                                             &pointer1, &length1, &pointer2, &length2, 0L);
      if (FAILED(Result)) {
        SetErrorValues(Miscellaneous, Result, LastReadError);
        PTRACE(1, "dsound\tRead Lock failed: " << GetErrorText());
        return false;
      }
      // Copy from DirectSound locked memory into return buffer
      memcpy((char *)dest, pointer1, length1);
      if (pointer2 != NULL)
        memcpy((char *)dest + length1, pointer2, length2);

      m_captureBuffer->Unlock(pointer1, length1, pointer2, length2);

      PINDEX readCount = length1 + length2;
      dest += readCount;
      len -= readCount;
      lastReadCount += readCount;
      m_movePos += readCount;
      m_movePos %= m_bufferSize;
      m_moved += readCount;
    }
  }
  return true;
}


PBoolean PSoundChannelDirectSound::AreAllRecordBuffersFull() // public
{
  PTRACE(4, "dsound\tAreAllRecordBuffersFull unimplemented");
  return true;
}


PBoolean PSoundChannelDirectSound::WaitForAllRecordBuffersFull() // public
{
  PTRACE(4, "dsound\tWaitForAllRecordBuffersFull unimplemented");
  return false;
}


PBoolean PSoundChannelDirectSound::RecordSound (PSound & /*sound*/) // public
{
  PAssert(activeDirection == Recorder, "Invalid device direction");
  PTRACE(4, "dsound\tRecordSound unimplemented");
  return false;
}


PBoolean PSoundChannelDirectSound::RecordFile (const PFilePath & /*filename*/) // public
{
  PAssert(activeDirection == Recorder, "Invalid device direction");
  PTRACE(4, "dsound\tRecordFile unimplemented");
  return false;
}


///////////////////////////////////////////////////////////////////////////////
// Volume

PBoolean PSoundChannelDirectSound::SetVolume (unsigned newVal) // public
{
  PWaitAndSignal mutex(m_bufferMutex);// prevent closing while active
  switch (activeDirection) {
  case Player:
    {
      if (!m_playbackBuffer) {
        PTRACE(4, "dsound\tPlayback SetVolume: Device not initialised");
        return SetErrorValues(NotOpen, EBADF);
      }
      // SetVolume is already logarithmic and is in 100ths of a decibel attenuation,
      // 0=max gain, 10,000 is min gain.
      HRESULT result = m_playbackBuffer->SetVolume((MaxVolume - newVal) * 100);
      if (FAILED(result)) {
        SetErrorValues(Miscellaneous, result);
        PTRACE(4, "dsound\tPlayback SetVolume failed: " << GetErrorText());
        return false;
      }
    }
    break;

  case Recorder:
    { // Use wave mixer because DirectX does not let you change the capture buffer volume
      if (!IsOpen() || m_mixer == NULL) {
        PTRACE(4, "dsound\tRecording SetVolume: Device not initialised");
        return SetErrorValues(NotOpen, EBADF);
      }
      MIXERCONTROLDETAILS_UNSIGNED volume;
      if (newVal >= MaxVolume)
        volume.dwValue = m_volumeControl.Bounds.dwMaximum;
      else
        volume.dwValue = m_volumeControl.Bounds.dwMinimum +
                (DWORD)((m_volumeControl.Bounds.dwMaximum - m_volumeControl.Bounds.dwMinimum) *
                                                       log10(9.0 * newVal / MaxVolume + 1.0));
      MIXERCONTROLDETAILS details;
      details.cbStruct = sizeof(details);
      details.dwControlID = m_volumeControl.dwControlID;
      details.cChannels = 1;
      details.cMultipleItems = 0;
      details.cbDetails = sizeof(volume);
      details.paDetails = &volume;

      MMRESULT result = mixerSetControlDetails((HMIXEROBJ)m_mixer, &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
      if (result != MMSYSERR_NOERROR) {
        SetErrorValues(Miscellaneous, MAKE_HRESULT(1, FACILITY_WAVEIN, result));
        PTRACE(4, "dsound\tRecording SetVolume failed: " << GetErrorText());
        return false;
      }
    }
    break;

  default:
    PTRACE(4, "dsound\t" << GetDirectionText() << " SetVolume: invalid direction");
    return SetErrorValues(NotOpen, EBADF);
  }
  return true;
}


PBoolean PSoundChannelDirectSound::GetVolume (unsigned &devVol) // public
{
  PWaitAndSignal mutex(m_bufferMutex); // prevent closing while active
  switch (activeDirection) {
  case Player:
    {
      if (!m_playbackBuffer) {
        PTRACE(4, "dsound\tPlayback GetVolume: Device not initialised");
        return SetErrorValues(NotOpen, EBADF, LastReadError);
      }
      long volume; // DSBVOLUME_MAX(=0)=loudest=0dB attenuation, DSBVOLUME_MIN(10,000)=quietest=100dB attenuation
      HRESULT result = m_playbackBuffer->GetVolume(&volume);
      if (FAILED(result)) {
        SetErrorValues(Miscellaneous, result);
        PTRACE(4, "dsound\tPlayback GetVolume failed: " << GetErrorText());
        return false;
      }
      devVol = (unsigned int)(MaxVolume - volume / 100);
    }
    break;
  
  case Recorder:
    {
      // Use wave mixer because DirectX does not let you change the capture buffer volume
      if (!IsOpen() || m_mixer == NULL) {
        PTRACE(4, "dsound\tRecording GetVolume: Device not initialised");
        return SetErrorValues(NotOpen, EBADF);
      }
      MIXERCONTROLDETAILS_UNSIGNED volume;

      MIXERCONTROLDETAILS details;
      details.cbStruct = sizeof(details);
      details.dwControlID = m_volumeControl.dwControlID;
      details.cChannels = 1;
      details.cMultipleItems = 0;
      details.cbDetails = sizeof(volume);
      details.paDetails = &volume;

      MMRESULT result = mixerGetControlDetails((HMIXEROBJ)m_mixer, &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
      if (result != MMSYSERR_NOERROR) {
        SetErrorValues(Miscellaneous, MAKE_HRESULT(1, FACILITY_WAVEIN, result));
        PTRACE(4, "dsound\tRecording GetVolume failed: " << GetErrorText());
        return false;
      }
      devVol = 100 * (volume.dwValue - m_volumeControl.Bounds.dwMinimum) / (m_volumeControl.Bounds.dwMaximum - m_volumeControl.Bounds.dwMinimum);
    }
    break;

  default:
    devVol = 0;
    break;
  }
  return true;
}


///////////////////////////////////////////////////////////////////////////////


#else

  #ifdef _MSC_VER
    #pragma message("Direct Sound support DISABLED")
  #endif

#endif // P_DIRECTSOUND
