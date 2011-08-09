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

#define INITGUID
#include <ptlib/msos/ptlib/sound_directsound.h>
#include <ptlib/msos/ptlib/sound_win32.h>

#include <math.h>


#ifdef P_DIRECTSOUND_DXERR_H
#include <dxerr.h>    // for DirectSound DXGetErrorDescription9
#pragma comment(lib, "dxerr.lib")
#endif


#ifdef _MSC_VER
  #pragma comment(lib, "dsound.lib")
  #pragma message("Direct Sound support enabled")
#endif


/* Instantiate the PWLIBsound plugin */ 
PCREATE_SOUND_PLUGIN(DirectSound, PSoundChannelDirectSound)


#if PTRACING
static PString GetErrorString(HRESULT error)
{
  if (error == S_OK)
    return "Ok";

#ifdef P_DIRECTSOUND_DXERR_H
  if (HRESULT_FACILITY(error) == _FACDS) // DirectX errors not m_availableBufferSpace in GetErrorDescription
    return DXGetErrorDescription(error);
#endif

  PString text;
  text.SetSize(1000);
  if (FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, 0, error, 0, text.GetPointerAndSetLength(0), text.GetSize(), 0) == 0)
    return psprintf("Error code 0x%l08x", error);

  text.MakeMinimumSize();
  return text;
}
#endif


///////////////////////////////////////////////////////////////////////////////


#ifdef _WIN32_WCE
#include <initguid.h>
#define IID_IDirectSoundBuffer8 IID_IDirectSoundBuffer
#define IID_IDirectSoundCaptureBuffer8 IID_IDirectSoundCaptureBuffer
DEFINE_GUID(DSDEVID_DefaultPlayback, 0xdef00000, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
DEFINE_GUID(DSDEVID_DefaultCapture, 0xdef00001, 0x9c6d, 0x47ed, 0xaa, 0xf1, 0x4d, 0xda, 0x8f, 0x2b, 0x5c, 0x03);
#endif

typedef struct 
{
  vector<GUID> guids;
  PStringArray names;

  void Append(const PString & name, const GUID & guid)
  {
    PINDEX size = names.GetSize();

    names.SetSize(size+1);
    names[size] = name.Left(MAXPNAMELEN-1).Trim(); // Do this so is compatible with MultiMedia version of name

    guids.resize(size+1);
    memcpy(&guids[size], &guid, sizeof(GUID));
  }
} DirectSoundDevices;


INT_PTR CALLBACK DSoundEnumCallback (GUID* pGUID, LPSTR strDesc, LPSTR /*strDrvName*/, void* arg)
{
  DirectSoundDevices & devices = *(DirectSoundDevices *)arg;

  if (pGUID != NULL)
    devices.Append(strDesc, *pGUID);

  return TRUE;
}

void EnumerateDSoundDevices (PSoundChannel::Directions dir, DirectSoundDevices & devices)
{
  if (dir == PSoundChannel::Recorder)
    DirectSoundCaptureEnumerate((LPDSENUMCALLBACK)DSoundEnumCallback, &devices);
  else
    DirectSoundEnumerate((LPDSENUMCALLBACK)DSoundEnumCallback, &devices);

  if (devices.names.GetSize() > 1)
    devices.Append("Default", (dir == PSoundChannel::Player) ? DSDEVID_DefaultPlayback  : DSDEVID_DefaultCapture);
}


PStringArray PSoundChannelDirectSound::GetDeviceNames (Directions dir)
{
  DirectSoundDevices devices;
  EnumerateDSoundDevices (dir, devices);
  return devices.names; 
}


///////////////////////////////////////////////////////////////////////////////


PSoundChannelDirectSound::PSoundChannelDirectSound()
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


void PSoundChannelDirectSound::Construct()
{
  m_audioPlaybackBuffer = NULL;
  m_audioPrimaryPlaybackBuffer = NULL;
  m_audioPlaybackDevice = NULL;
  m_audioCaptureDevice = NULL;
  m_audioCaptureBuffer = NULL;
  m_dxBufferSize = 0;
  m_isStreaming = true;
  m_volume = 0;

  memset (&m_waveFormat, 0, sizeof (m_waveFormat)); 
  m_notificationEvent[0] = CreateEvent(NULL, false, false, NULL);// auto-reset
  m_notificationEvent[1] = CreateEvent(NULL, false, false, NULL);// auto-reset

  SetBuffers(16000, 3); // 3 seconds at 8kHz
}


PSoundChannelDirectSound::~PSoundChannelDirectSound()
{
  Close();
  if (m_notificationEvent[0] != NULL)
    CloseHandle(m_notificationEvent[0]);

  if (m_notificationEvent[1] != NULL)
    CloseHandle(m_notificationEvent[1]);

  PTRACE(4, "dsound\t" << ((m_direction == Player) ? "Playback" : "Recording") << " destroyed");
}


PBoolean PSoundChannelDirectSound::Open(const PString & device,
                                        Directions dir,
                                        unsigned numChannels,
                                        unsigned sampleRate,
                                        unsigned bitsPerSample)
{
  Abort();

  m_deviceName = device;
  m_direction = dir;
  m_numChannels = numChannels;
  m_sampleRate = sampleRate;
  m_bitsPerSample = bitsPerSample;

  Close();
  ResetEvent(m_notificationEvent[1]);

  PTRACE(4, "dsound\tOpen " << ((m_direction == Player) ? "playback" : "recording") << " device " << m_deviceName);

  GUID deviceGUID;
  DirectSoundDevices devices;
  EnumerateDSoundDevices(m_direction, devices);

  PINDEX idx = devices.names.GetStringsIndex(m_deviceName);
  if (idx != P_MAX_INDEX)
    deviceGUID = devices.guids[idx];
  else {
    PTRACE(4, "dsound\tGetDeviceID device: " << m_deviceName << " not found, substituting default");
    deviceGUID = (m_direction == Player) ? DSDEVID_DefaultPlayback  : DSDEVID_DefaultCapture;
  }

  if (m_direction == Player) {
    HRESULT hr = DirectSoundCreate8(&deviceGUID, &m_audioPlaybackDevice, NULL);
    if (hr != S_OK) {
      PTRACE(4, "dsound\tCould not create playback device: " << GetErrorString(hr));
      return false;
    }

    HWND hWnd = GetForegroundWindow();
    if (hWnd == NULL)
      hWnd = GetDesktopWindow();

    hr = m_audioPlaybackDevice->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
    if (hr != S_OK) {
      PTRACE(4, "dsound\tCould not set playback device cooperative level: " << GetErrorString(hr));
      m_audioPlaybackDevice.Release();
      return false;
    }

    DSBUFFERDESC dsbd = {
      sizeof(DSBUFFERDESC),        // dwSize
      DSBCAPS_PRIMARYBUFFER        // dwFlags
    };                            // dwBufferBytes, dwReserved, lpwfxFormat, guid3DAlgorithm = 0
    if (FAILED(m_audioPlaybackDevice->CreateSoundBuffer(&dsbd, &m_audioPrimaryPlaybackBuffer, NULL))) {
      PTRACE(4, "dsound\tCould not create playback device primary buffer: " << GetErrorString(hr));
      m_audioPlaybackDevice.Release();
      return false;
    }
  }
  else {
    HRESULT hr = DirectSoundCaptureCreate8(&deviceGUID, &m_audioCaptureDevice, NULL);
    if (hr != S_OK) {
      PTRACE(4, "dsound\tCould not create Capture device: " << GetErrorString(hr));
      return false;
    }
  }

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


PBoolean PSoundChannelDirectSound::Abort()
{
  SetEvent(m_notificationEvent[1]); // abort waiting
  PWaitAndSignal mutex(m_bufferMutex);
  switch (m_direction) {
  case Player:
    if (m_audioPlaybackBuffer) {
      PTRACE(4, "dsound\tClose playback device " << m_deviceName);
      m_audioPlaybackBuffer->Stop();
    }
    m_audioPlaybackBuffer.Release();
    break;

  case Recorder:
    if (m_audioCaptureBuffer) {
      PTRACE(4, "dsound\tClose recording device " << m_deviceName);
      m_audioCaptureBuffer->Stop();
    }
    m_audioCaptureBuffer.Release();
    break;
  }
  return true;
}


PBoolean PSoundChannelDirectSound::Close()
{
  Abort(); // abort waiting for I/O & destroy buffers

  switch (m_direction) {
  case Player:
    m_audioPrimaryPlaybackBuffer.Release();
    m_audioPlaybackDevice.Release();
    break;

  case Recorder:
    m_audioCaptureDevice.Release();
    break;
  }
  return true;
}


PBoolean PSoundChannelDirectSound::SetFormat(unsigned numChannels,
                                             unsigned sampleRate,
                                             unsigned bitsPerSample)
{
  Abort(); // abort waiting for I/O & destroy buffers

  memset (&m_waveFormat, 0, sizeof (m_waveFormat)); 
  m_waveFormat.wFormatTag = WAVE_FORMAT_PCM;
  m_waveFormat.nChannels = (WORD)numChannels;
  m_waveFormat.nSamplesPerSec = sampleRate;
  m_waveFormat.wBitsPerSample = (WORD)bitsPerSample;
  m_waveFormat.nBlockAlign = m_waveFormat.nChannels * (m_waveFormat.wBitsPerSample / 8);
  m_waveFormat.nAvgBytesPerSec = m_waveFormat.nSamplesPerSec * m_waveFormat.nBlockAlign;

  PTRACE(4, "dsound\t" << ((m_direction == Player) ? "Playback" : "Recording") << " SetFormat\n"
    << "   -->  nChannels: " << m_waveFormat.nChannels << '\n'
    << "   -->  nSamplesPerSec: " << m_waveFormat.nSamplesPerSec << '\n'
    << "   -->  wBitsPerSample: " << m_waveFormat.wBitsPerSample << '\n'
    << "   -->  nBlockAlign: " << m_waveFormat.nBlockAlign << '\n'
    << "   -->  nAvgBytesPerSec: " << m_waveFormat.nAvgBytesPerSec);

  return true; // no buffers yet
}


PBoolean PSoundChannelDirectSound::SetBuffers(PINDEX size, PINDEX count)
{
  Abort(); // abort waiting for I/O & destroy buffers

  m_bufferCount = count;
  m_bufferSize = size; 
  m_dxBufferSize = m_bufferCount * m_bufferSize;

  PTRACE(4, "dsound\tSetBuffers ==>  size = " << m_bufferSize << "  |  count = " << m_bufferCount << "  | DXBufferSize = " << m_dxBufferSize);
  return true;
}


PBoolean PSoundChannelDirectSound::GetBuffers(PINDEX & size, PINDEX & count)
{
  count = m_bufferCount;
  size = m_bufferSize;
  return true;
}


PBoolean PSoundChannelDirectSound::InitPlaybackBuffer()
{
  PTRACE(4, "dsound\tInitPlaybackBuffer");

  PWaitAndSignal mutex(m_bufferMutex);
  if (!IsOpen())
    return false;

  if (m_audioPlaybackBuffer != NULL)
    return true;

  DSBUFFERDESC dsbdesc = {
    sizeof(DSBUFFERDESC),
    DSBCAPS_GLOBALFOCUS + DSBCAPS_CTRLPOSITIONNOTIFY + DSBCAPS_GETCURRENTPOSITION2,
    m_dxBufferSize,                // calculated by SetBuffers
    0,                            // reserved
    &m_waveFormat                       // format
  } ;
  HRESULT hr = m_audioPlaybackDevice->CreateSoundBuffer(&dsbdesc, &m_audioPlaybackBuffer, NULL); 
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlaybackBuffer: CreateSoundBuffer failed: " << GetErrorString(hr));
    return false;
  } 
  IDirectSoundNotify * Notify ;// temporary pointer to the interface
  hr = m_audioPlaybackBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &Notify);
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlayBuffer: Notify interface query failed: " << GetErrorString(hr));
    m_audioPlaybackBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitPlayBuffer: Setting up notification for " << m_bufferCount << " blocks of " << m_bufferSize << " bytes");
  DSBPOSITIONNOTIFY * Position = new DSBPOSITIONNOTIFY [m_bufferCount];
  if (Position == 0) {
    PTRACE(4, "dsound\tInitPlayBuffer: Notify allocation failed");
    Notify->Release();
    m_audioPlaybackBuffer.Release();
    return false;
  }
  DWORD BlockOffset = m_bufferSize - 1;
  for (PINDEX i = 0; i < m_bufferCount; i++) {
    Position [i].dwOffset = BlockOffset;
    Position [i].hEventNotify = m_notificationEvent[0]; // all use same event
    BlockOffset += m_bufferSize;
  }
  hr = Notify->SetNotificationPositions (m_bufferCount, Position);
  Notify->Release();
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlaybackBuffer: Notify interface query failed: " << GetErrorString(hr));
    m_audioPlaybackBuffer.Release();
    return false;
  }
  delete [] Position;
  m_bufferByteOffset = 0; // reset write position
  m_audioPlaybackBuffer->SetCurrentPosition (0);
  ResetEvent(m_notificationEvent[1]);
  return true;
}


PBoolean PSoundChannelDirectSound::InitCaptureBuffer() 
{
  PTRACE(4, "dsound\tInitCaptureBuffer");

  PWaitAndSignal mutex(m_bufferMutex);
  if (!IsOpen())
    return false;

  if (m_audioCaptureBuffer != NULL)
    return true;

  DSCBUFFERDESC dscbdesc = {
    sizeof(DSCBUFFERDESC),
    DSCBCAPS_WAVEMAPPED,        // DSCBCAPS_CTRLFX(support effects) | DSCBCAPS_WAVEMAPPED(use wave mapper for formats unsupported by device)
    m_dxBufferSize,                // calculated by SetBuffers
    0,                            // reserved
    &m_waveFormat                       // format
  } ;
  HRESULT hr = m_audioCaptureDevice->CreateCaptureBuffer(&dscbdesc, &m_audioCaptureBuffer, NULL); 
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer: Create Sound Buffer failed: " << GetErrorString(hr));
    return false;
  }
  IDirectSoundNotify * Notify ;// temporary pointer to the interface
  hr = m_audioCaptureBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &Notify);
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify interface query failed: " << GetErrorString(hr));
    m_audioCaptureBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitCaptureBuffer: Setting up notification for " << m_bufferCount << " blocks of " << m_bufferSize << " bytes");
  DSBPOSITIONNOTIFY * Position = new DSBPOSITIONNOTIFY [m_bufferCount];
  if (Position == 0) {
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify allocation failed");
    Notify->Release();
    m_audioCaptureBuffer.Release();
    return false;
  }
  DWORD BlockOffset = m_bufferSize - 1;
  for (PINDEX i = 0; i < m_bufferCount; i++) {
    Position [i].dwOffset = BlockOffset;
    Position [i].hEventNotify = m_notificationEvent[0];// all use same event
    BlockOffset += m_bufferSize;
  }
  hr = Notify->SetNotificationPositions (m_bufferCount, Position);
  Notify->Release();
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer : Notify interface query failed: " << GetErrorString(hr));
    m_audioCaptureBuffer.Release();
    return false;
  }
  delete [] Position ;
  m_bufferByteOffset = 0; // reset read position
  ResetEvent(m_notificationEvent[1]);
  return true;
}


PBoolean PSoundChannelDirectSound::Write(const void *buf, PINDEX len)
{
  lastWriteCount = 0;
  if (!InitPlaybackBuffer())
    return false;

  char * src = (char *) buf;
  do {
    if (!WaitForPlayBufferFree())       // wait for output space to become m_availableBufferSpace
      return false;                     // closed

    PWaitAndSignal mutex(m_bufferMutex);  // prevent closing while active

    LPVOID lpvWrite1, lpvWrite2;
    DWORD dwLength1, dwLength2;
    HRESULT hr = m_audioPlaybackBuffer->Lock(m_bufferByteOffset,
                                            PMIN((PINDEX)m_availableBufferSpace, len),
                                            &lpvWrite1,
                                            &dwLength1,
                                            &lpvWrite2,
                                            &dwLength2,
                                            0L);
    if (hr == DSERR_BUFFERLOST) { // Buffer was lost, need to restore it
      m_audioPlaybackBuffer->Restore();
      hr = m_audioPlaybackBuffer->Lock(m_bufferByteOffset,
                                      PMIN((PINDEX)m_availableBufferSpace, len),
                                      &lpvWrite1,
                                      &dwLength1,
                                      &lpvWrite2,
                                      &dwLength2,
                                      0L);
    }
    if (FAILED (hr)) {
      PTRACE(1, "dsound\tWriteToDXBuffer failed: " << GetErrorString(hr) << " len " << len << " pos " << m_bufferByteOffset);
      return false;
    }

    // Copy supplied buffer into locked DX memory
    memcpy (lpvWrite1, buf, dwLength1);
    if (lpvWrite2 != NULL)
      memcpy (lpvWrite2, (BYTE *) buf + dwLength1, dwLength2);

    PINDEX writeCount = dwLength1 + dwLength2;

    m_audioPlaybackBuffer->Unlock(lpvWrite1, dwLength1, lpvWrite2, dwLength2);

    src += writeCount;
    len -= writeCount;
    lastWriteCount += writeCount;
    m_bufferByteOffset += lastWriteCount;
    m_bufferByteOffset %= m_dxBufferSize;
                                        // tell DX to play
    m_audioPlaybackBuffer->Play (0, 0, m_isStreaming ? DSBPLAY_LOOPING : 0L);
  } while (lastWriteCount < len);

  return true;
}


PBoolean PSoundChannelDirectSound::Read (void * buf, PINDEX len)
{
  lastReadCount = 0;
  if (!StartRecording())                // Start the first read
    return false;

  char * dest = (char *) buf;
  do {
    if (!WaitForRecordBufferFull())     // sets m_bufferByteOffset and m_availableBufferSpace
      return false;                     // closed

    PWaitAndSignal mutex(m_bufferMutex);  // prevent closing while active

    LPVOID lpvRead1, lpvRead2;
    DWORD dwLength1, dwLength2;
    HRESULT hr = m_audioCaptureBuffer->Lock(m_bufferByteOffset,
                                           PMIN((PINDEX)m_availableBufferSpace, len),
                                           &lpvRead1,
                                           &dwLength1,
                                           &lpvRead2,
                                           &dwLength2,
                                           0L);
    if (FAILED(hr)) {
      PTRACE(1, "dsound\tReadFromDXBuffer Lock failed: " << GetErrorString(hr));
      return false;
    }

    // Copy from DX locked memory into return buffer
    memcpy((BYTE *)buf, lpvRead1, dwLength1);
    if (lpvRead2 != NULL)
      memcpy ((BYTE *) buf + dwLength1, lpvRead2, dwLength2);

    PINDEX readCount = dwLength1 + dwLength2;

    m_audioCaptureBuffer->Unlock(lpvRead1, dwLength1, lpvRead2, dwLength2);

    dest += readCount;
    len -= readCount;
    lastReadCount += readCount;
    m_bufferByteOffset += readCount;
    m_bufferByteOffset %= m_dxBufferSize;
  } while (lastReadCount < len);

  return true;
}


PBoolean PSoundChannelDirectSound::PlaySound (const PSound & sound, PBoolean wait)
{
  m_isStreaming = false;

  SetBuffers(sound.GetSize(), 1);

  if (!Write((const void *)sound, sound.GetSize()))
    return false;

  if (wait)
    return WaitForPlayCompletion();
  return true;
}


PBoolean PSoundChannelDirectSound::PlayFile (const PFilePath & filename, PBoolean wait)
{
  PMultiMediaFile mmio;
  PWaveFormat fileFormat;
  DWORD dataSize;
  if (!mmio.OpenWaveFile(filename, fileFormat, dataSize))
    return SetErrorValues(NotOpen, mmio.GetLastError() | PWIN32ErrorFlag, LastWriteError);

  Abort();
  m_dxBufferSize = 0; // so SetFormat doesn't create buffer

  // Save old format and set to one loaded from file.
  unsigned numChannels = m_waveFormat.nChannels;
  unsigned sampleRate = m_waveFormat.nSamplesPerSec;
  unsigned bitsPerSample = m_waveFormat.wBitsPerSample;

  SetFormat(fileFormat->nChannels, fileFormat->nSamplesPerSec, fileFormat->wBitsPerSample);

  int bufferSize = m_waveFormat.nAvgBytesPerSec / 2;
  if (!SetBuffers (bufferSize, 4))
    SetFormat(numChannels, sampleRate, bitsPerSample);// restore audio format

  PBYTEArray buffer;
  m_isStreaming = false;

  while (dataSize)
  {
    // Read the waveform data subchunk
    PINDEX count = PMIN(dataSize,((DWORD)bufferSize));
    if (!mmio.Read(buffer.GetPointer(bufferSize), count)) {
      PTRACE(4, "dsound\tPlayFile read error");
      return SetErrorValues(NotOpen, mmio.GetLastError() | PWIN32ErrorFlag, LastReadError);
    }
    if (!Write(buffer, count))
      break;

    dataSize -= count;
  }
  mmio.Close();

  if (wait)
    return WaitForPlayCompletion();

  return true;
}


PBoolean PSoundChannelDirectSound::IsPlayBufferFree()
{
  if (!InitPlaybackBuffer())
    return false;

  DWORD PlayPos; // byte offset from start of buffer to next byte directsound will play (end of where we can write)
  m_audioPlaybackBuffer->GetCurrentPosition (&PlayPos, 0);
  if (PlayPos <= m_bufferByteOffset) // wrapped around
    m_availableBufferSpace = m_dxBufferSize - m_bufferByteOffset + PlayPos;
  else
    m_availableBufferSpace = PlayPos - m_bufferByteOffset;

  //if (m_availableBufferSpace % m_waveFormat.nBlockAlign) // always write multiples of sample frames
  //  m_availableBufferSpace -= (m_availableBufferSpace % m_waveFormat.nBlockAlign);

  if ((PINDEX)m_availableBufferSpace == m_dxBufferSize) {
    DWORD dwStatus;
    m_audioPlaybackBuffer->GetStatus (&dwStatus);
    if ((dwStatus & DSBSTATUS_PLAYING) != 0) 
      m_availableBufferSpace = 0;

    //PTRACE(4, "dsound\tPlayer buffer " << m_availableBufferSpace << (((dwStatus & DSBSTATUS_PLAYING) == 0)? " empty" : " full"));
  }
  //else
  //  PTRACE(4, "dsound\tPlayer buffer " << m_availableBufferSpace);

  return m_availableBufferSpace >= (unsigned)m_bufferSize;
}


PBoolean PSoundChannelDirectSound::WaitForPlayBufferFree()
{
  ResetEvent(m_notificationEvent[0]);
  while (!IsPlayBufferFree()) { // always call last (after wait) to set member variables
    if (WaitForMultipleObjects(2, m_notificationEvent, FALSE, INFINITE) != WAIT_OBJECT_0) {
      PTRACE(4, "dsound\tPlayer abort");
      return false;
    }
  }
  return true;
}


PBoolean PSoundChannelDirectSound::HasPlayCompleted()
{
  DWORD dwStatus;
  if (m_audioPlaybackBuffer != NULL) {
    m_audioPlaybackBuffer->GetStatus (&dwStatus);
    if (dwStatus & DSBSTATUS_PLAYING)
      return false;
  }
  return true;
}


PBoolean PSoundChannelDirectSound::WaitForPlayCompletion()
{
  while (!HasPlayCompleted()) 
    Sleep (50);

  return true;
}


PBoolean PSoundChannelDirectSound::RecordSound (PSound & /*sound*/)
{
  PTRACE(4, "dsound\tRecordSound unimplemented");
  return false;
}


PBoolean PSoundChannelDirectSound::RecordFile (const PFilePath & /*filename*/)
{
  PTRACE(4, "dsound\tRecordFile unimplemented");
  return false;
}


PBoolean PSoundChannelDirectSound::StartRecording()
{
  if (!InitCaptureBuffer())
    return false;

  DWORD Status = 0;
  if (FAILED(m_audioCaptureBuffer->GetStatus(&Status))) {
    PTRACE(4, "dsound\tStartRecording: Failed GetStatus");
    //return SetErrorValues(NotOpen, EBADF, LastWriteError);
    return false;
  }

  if ((Status & DSCBSTATUS_CAPTURING) != 0)
    return true;

  m_bufferByteOffset = 0;
  if (FAILED(m_audioCaptureBuffer->Start (DSCBSTART_LOOPING))) {
    PTRACE(4, "dsound\tStartRecording: Failed Start");
    //return SetErrorValues(NotOpen, EBADF, LastWriteError);
    return false;
  }
  PTRACE(4, "dsound\tInitCaptureBuffer: Starting capture");
  return true;
}


PBoolean PSoundChannelDirectSound::IsRecordBufferFull()
{
  if (!StartRecording())                // Start the first read
    return false;

  DWORD ReadPos;                        // byte offset from start of buffer to the end of the data that has been fully captured
  m_audioCaptureBuffer->GetCurrentPosition (0, &ReadPos);
  if (ReadPos < m_bufferByteOffset)        // wrapped around
    m_availableBufferSpace = m_dxBufferSize - m_bufferByteOffset + ReadPos;
  else
    m_availableBufferSpace = ReadPos - m_bufferByteOffset;
                                        // always read multiples of sample frames
  m_availableBufferSpace -= (m_availableBufferSpace % m_waveFormat.nBlockAlign);
  return m_availableBufferSpace >= (unsigned)m_bufferSize;
}


PBoolean PSoundChannelDirectSound::AreAllRecordBuffersFull()
{
  PTRACE(4, "dsound\tAreAllRecordBuffersFull unimplemented");
  return true;
}


PBoolean PSoundChannelDirectSound::WaitForRecordBufferFull()
{
  ResetEvent(m_notificationEvent[0]);
  while (!IsRecordBufferFull()) {       // repeat after wait to set member variables
    if (WaitForMultipleObjects(2, m_notificationEvent, FALSE, INFINITE) != WAIT_OBJECT_0)
      return false;
  }
  return IsOpen();
}


PBoolean PSoundChannelDirectSound::WaitForAllRecordBuffersFull()
{

  PTRACE(4, "dsound\tWaitForAllRecordBuffersFull unimplemented");
  return false;
}


PBoolean PSoundChannelDirectSound::SetVolume (unsigned newVal)
{

  PBoolean no_error=true;
  HRESULT hr;

  switch (m_direction) {
  case Player:
    if (m_audioPlaybackBuffer) {
      // SetVolume is already logarithmic and is in 100ths of a decibel attenuation,
      // 0=max gain, 10,000 is min gain.
      if (FAILED (hr = m_audioPlaybackBuffer->SetVolume((MaxVolume - newVal)*100))) {
        PTRACE(4, "PSoundChannelDirectSound::SetVolume failed " << GetErrorString(hr));
        no_error = false;
      }
    }
    else
      PTRACE(4, "PSoundChannelDirectSound::SetVolume Failed m_audioPlaybackBuffer is NULLL (huh?)");
    break;

  case Recorder:
    // DirectX does not let you change the capture buffer volume
    m_volume = newVal;
    break;
  }
  return no_error;
}


PBoolean PSoundChannelDirectSound::GetVolume (unsigned &devVol)
{
  switch (m_direction) 
  {
    case Player:
      if (m_audioPlaybackBuffer) {
        long volume;
        HRESULT hr = m_audioPlaybackBuffer->GetVolume(&volume);
        if (SUCCEEDED(hr)) {
          devVol = (unsigned int)(MaxVolume - volume/100);
          return true;
        }
        PTRACE(4, "PSoundChannelDirectSound::GetVolume failed " << GetErrorString(hr));
      }
      break;

    case Recorder:
      // DirectX does not let you change the capture buffer volume
      devVol = m_volume;
      break;
  }
  return false;
}


#else

  #ifdef _MSC_VER
    #pragma message("Direct Sound support DISABLED")
  #endif

#endif // P_DIRECTSOUND
