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
  if (HRESULT_FACILITY(error) == _FACDS) // DirectX errors not available in GetErrorDescription
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

  if (devices.names.GetSize () > 1)
    devices.Append("Default", (dir == PSoundChannel::Player) ? DSDEVID_DefaultPlayback  : DSDEVID_DefaultCapture);
}


PStringArray PSoundChannelDirectSound::GetDeviceNames (Directions dir)
{
  DirectSoundDevices devices;
  EnumerateDSoundDevices (dir, devices);
  return devices.names; 
}


///////////////////////////////////////////////////////////////////////////////


PSoundChannelDirectSound::PSoundChannelDirectSound ()
: mAudioPlaybackBuffer(NULL),
  mAudioPrimaryPlaybackBuffer(NULL),
  sAudioPlaybackDevice(NULL),
  sAudioCaptureDevice(NULL),
  mAudioCaptureBuffer(NULL),
  mDXBufferSize(0),
  mStreaming(true),
  mVolume(0)
{
  memset (&mWFX, 0, sizeof (mWFX)); 
  notificationEvent[0] = CreateEvent(NULL, PFalse, PFalse, NULL);// auto-reset
  notificationEvent[1] = CreateEvent(NULL, PFalse, PFalse, NULL);// auto-reset
}


PSoundChannelDirectSound::PSoundChannelDirectSound (const PString &device,
                                                    Directions dir,
                                                    unsigned numChannels,
                                                    unsigned sampleRate,
                                                    unsigned bitsPerSample)
: mAudioPlaybackBuffer(NULL),
  mAudioPrimaryPlaybackBuffer(NULL),
  sAudioPlaybackDevice(NULL),
  sAudioCaptureDevice(NULL),
  mAudioCaptureBuffer(NULL),
  mDXBufferSize(0),
  mStreaming(true),
  mVolume(0)
{
  notificationEvent[0] = CreateEvent(NULL, PFalse, PFalse, NULL);// auto-reset
  notificationEvent[1] = CreateEvent(NULL, PFalse, PFalse, NULL);// auto-reset
  Open (device, dir, numChannels, sampleRate, bitsPerSample);
}


PSoundChannelDirectSound::~PSoundChannelDirectSound ()
{
  Close();
  if (notificationEvent[0] != NULL)
    CloseHandle(notificationEvent[0]);

  if (notificationEvent[1] != NULL)
    CloseHandle(notificationEvent[1]);

  PTRACE(4, "dsound\t" << ((mDirection == Player) ? "Playback" : "Recording") << " destroyed");
}


PBoolean PSoundChannelDirectSound::Open (const PString & _device,
                                Directions _dir,
                                unsigned _numChannels,
                                unsigned _sampleRate,
                                unsigned _bitsPerSample)
{
  deviceName = _device;
  mDirection = _dir;
  mNumChannels = _numChannels;
  mSampleRate = _sampleRate;
  mBitsPerSample = _bitsPerSample;

  GUID deviceGUID;
  if (!GetDeviceID(_device, &deviceGUID))
    return false;

  Close();
  ResetEvent(notificationEvent[1]);

  PTRACE(4, "dsound\tOpen " << ((mDirection == Player) ? "playback" : "recording") << " device " << deviceName);

  SetFormat(_numChannels, _sampleRate, _bitsPerSample);
  return (mDirection == Player) ? InitPlaybackDevice(&deviceGUID) : InitCaptureDevice(&deviceGUID);
}


PBoolean PSoundChannelDirectSound::GetDeviceID (PString deviceName, GUID *pGUID)
{
  *pGUID = (mDirection == Player) ? DSDEVID_DefaultPlayback  : DSDEVID_DefaultCapture;
  DirectSoundDevices devices;
  EnumerateDSoundDevices(mDirection, devices);

  PINDEX idx = devices.names.GetStringsIndex(deviceName);
  if (idx != P_MAX_INDEX)
    *pGUID = devices.guids[idx];
  else
    PTRACE(4, "dsound\tGetDeviceID device: " << deviceName << " not found, substituting default");

  return true;
}


PBoolean PSoundChannelDirectSound::InitPlaybackDevice (GUID *pGUID)
{
  HRESULT hr = DirectSoundCreate8(pGUID, &sAudioPlaybackDevice, NULL);
  if (hr != S_OK) {
    PTRACE(4, "dsound\tCould not create playback device: " << GetErrorString(hr));
    return false;
  }
  HWND hWnd = GetForegroundWindow();
  if (hWnd == NULL)
    hWnd = GetDesktopWindow();

  hr = sAudioPlaybackDevice->SetCooperativeLevel(hWnd, DSSCL_PRIORITY);
  if (hr != S_OK) {
    PTRACE(4, "dsound\tCould not set playback device cooperative level: " << GetErrorString(hr));
    sAudioPlaybackDevice.Release();
    return false;
  }

  DSBUFFERDESC dsbd = {
    sizeof(DSBUFFERDESC),        // dwSize
    DSBCAPS_PRIMARYBUFFER        // dwFlags
  };                            // dwBufferBytes, dwReserved, lpwfxFormat, guid3DAlgorithm = 0
  if (FAILED(sAudioPlaybackDevice->CreateSoundBuffer(&dsbd, &mAudioPrimaryPlaybackBuffer, NULL))) {
    PTRACE(4, "dsound\tCould not create playback device primary buffer: " << GetErrorString(hr));
    sAudioPlaybackDevice.Release();
    return false;
  }
  return PTrue;
}


PBoolean PSoundChannelDirectSound::InitCaptureDevice(GUID *pGUID)
{
  HRESULT hr = DirectSoundCaptureCreate8 (pGUID, &sAudioCaptureDevice, NULL);
  if (hr != S_OK) {
    PTRACE(4, "dsound\tCould not create Capture device: " << GetErrorString(hr));
    return false;
  }
  return true;
}


PBoolean PSoundChannelDirectSound::Abort()
{
  SetEvent(notificationEvent[1]); // abort waiting
  PWaitAndSignal mutex(bufferMutex);
  switch (mDirection) {
  case Player:
    if (mAudioPlaybackBuffer) {
      PTRACE(4, "dsound\tClose playback device " << deviceName);
      mAudioPlaybackBuffer->Stop ();
    }
    mAudioPlaybackBuffer.Release();
    break;

  case Recorder:
    if (mAudioCaptureBuffer) {
      PTRACE(4, "dsound\tClose recording device " << deviceName);
      mAudioCaptureBuffer->Stop ();
    }
    mAudioCaptureBuffer.Release();
    break;
  }
  return PTrue;
}


PBoolean PSoundChannelDirectSound::Close()
{
  Abort(); // abort waiting for I/O & destroy buffers

  switch (mDirection) {
  case Player:
    mAudioPrimaryPlaybackBuffer.Release();
    sAudioPlaybackDevice.Release();
    break;

  case Recorder:
    sAudioCaptureDevice.Release();
    break;
  }
  return PTrue;
}


PBoolean PSoundChannelDirectSound::SetFormat (unsigned numChannels,
                                     unsigned sampleRate,
                                     unsigned bitsPerSample)
{
  memset (&mWFX, 0, sizeof (mWFX)); 
  mWFX.wFormatTag = WAVE_FORMAT_PCM;
  mWFX.nChannels = (WORD)numChannels;
  mWFX.nSamplesPerSec = sampleRate;
  mWFX.wBitsPerSample = (WORD)bitsPerSample;
  mWFX.nBlockAlign = mWFX.nChannels * (mWFX.wBitsPerSample / 8);
  mWFX.nAvgBytesPerSec = mWFX.nSamplesPerSec * mWFX.nBlockAlign;

  PTRACE(4, "dsound\t" << ((mDirection == Player) ? "Playback" : "Recording") << " SetFormat\n"
    << "   -->  nChannels: " << mWFX.nChannels << '\n'
    << "   -->  nSamplesPerSec: " << mWFX.nSamplesPerSec << '\n'
    << "   -->  wBitsPerSample: " << mWFX.wBitsPerSample << '\n'
    << "   -->  nBlockAlign: " << mWFX.nBlockAlign << '\n'
    << "   -->  nAvgBytesPerSec: " << mWFX.nAvgBytesPerSec);

  if (!mDXBufferSize)
    return PTrue; // no buffers yet

  // resize the buffers to match
  Abort(); // abort waiting for I/O & destroy buffers
  if (mDirection == Player)
    return InitPlaybackBuffer();

  return InitCaptureBuffer();
}


PBoolean PSoundChannelDirectSound::SetBuffers (PINDEX size, PINDEX count)
{
  if (mDXBufferSize)
    Abort(); // abort waiting for I/O & destroy buffers

  mBufferCount = count;
  mBufferSize = size; 
  mDXBufferSize = mBufferCount * mBufferSize;

  PTRACE(4, "dsound\tSetBuffers ==>  size = " << mBufferSize << "  |  count = " << mBufferCount << "  | DXBufferSize = " << mDXBufferSize);
  if (mDirection == Player)
    return InitPlaybackBuffer();

  return InitCaptureBuffer();
}


PBoolean PSoundChannelDirectSound::GetBuffers (PINDEX & size, PINDEX & count)
{
  count = mBufferCount;
  size = mBufferSize;
  return PTrue;
}


PBoolean PSoundChannelDirectSound::InitPlaybackBuffer ()
{
  PTRACE(4, "dsound\tInitPlaybackBuffer");

  PWaitAndSignal mutex(bufferMutex);
  if (!IsOpen())
    return false;

  if (mAudioPlaybackBuffer) {
    PTRACE(4, "dsound\tInitPlayBuffer: Forgot to Abort");
    return false;
  }
  DSBUFFERDESC dsbdesc = {
    sizeof(DSBUFFERDESC),
    DSBCAPS_GLOBALFOCUS + DSBCAPS_CTRLPOSITIONNOTIFY + DSBCAPS_GETCURRENTPOSITION2,
    mDXBufferSize,                // calculated by SetBuffers
    0,                            // reserved
    &mWFX                       // format
  } ;
  HRESULT hr = sAudioPlaybackDevice->CreateSoundBuffer(&dsbdesc, &mAudioPlaybackBuffer, NULL); 
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlaybackBuffer: CreateSoundBuffer failed: " << GetErrorString(hr));
    return false;
  } 
  IDirectSoundNotify * Notify ;// temporary pointer to the interface
  hr = mAudioPlaybackBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &Notify);
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlayBuffer: Notify interface query failed: " << GetErrorString(hr));
    mAudioPlaybackBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitPlayBuffer: Setting up notification for " << mBufferCount << " blocks of " << mBufferSize << " bytes");
  DSBPOSITIONNOTIFY * Position = new DSBPOSITIONNOTIFY [mBufferCount];
  if (Position == 0) {
    PTRACE(4, "dsound\tInitPlayBuffer: Notify allocation failed");
    Notify->Release();
    mAudioPlaybackBuffer.Release();
    return false;
  }
  DWORD BlockOffset = mBufferSize - 1;
  for (PINDEX i = 0; i < mBufferCount; i++) {
    Position [i].dwOffset = BlockOffset;
    Position [i].hEventNotify = notificationEvent[0]; // all use same event
    BlockOffset += mBufferSize;
  }
  hr = Notify->SetNotificationPositions (mBufferCount, Position);
  Notify->Release();
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitPlaybackBuffer: Notify interface query failed: " << GetErrorString(hr));
    mAudioPlaybackBuffer.Release();
    return false;
  }
  delete [] Position;
  bufferByteOffset = 0; // reset write position
  mAudioPlaybackBuffer->SetCurrentPosition (0);
  ResetEvent(notificationEvent[1]);
  return true;
}


PBoolean PSoundChannelDirectSound::InitCaptureBuffer () 
{
  PTRACE(4, "dsound\tInitCaptureBuffer");

  PWaitAndSignal mutex(bufferMutex);
  if (!IsOpen())
    return false;

  if (mAudioCaptureBuffer) {
    PTRACE(4, "dsound\tInitCaptureBuffer: Forgot to Abort");
    return false;
  }
  DSCBUFFERDESC dscbdesc = {
    sizeof(DSCBUFFERDESC),
    DSCBCAPS_WAVEMAPPED,        // DSCBCAPS_CTRLFX(support effects) | DSCBCAPS_WAVEMAPPED(use wave mapper for formats unsupported by device)
    mDXBufferSize,                // calculated by SetBuffers
    0,                            // reserved
    &mWFX                       // format
  } ;
  HRESULT hr = sAudioCaptureDevice->CreateCaptureBuffer(&dscbdesc, &mAudioCaptureBuffer, NULL); 
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer: Create Sound Buffer failed: " << GetErrorString(hr));
    return false;
  }
  IDirectSoundNotify * Notify ;// temporary pointer to the interface
  hr = mAudioCaptureBuffer->QueryInterface(IID_IDirectSoundNotify, (LPVOID *) &Notify);
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify interface query failed: " << GetErrorString(hr));
    mAudioCaptureBuffer.Release();
    return false;
  }
  PTRACE(4, "dsound\tInitCaptureBuffer: Setting up notification for " << mBufferCount << " blocks of " << mBufferSize << " bytes");
  DSBPOSITIONNOTIFY * Position = new DSBPOSITIONNOTIFY [mBufferCount];
  if (Position == 0) {
    PTRACE(4, "dsound\tInitCaptureBuffer: Notify allocation failed");
    Notify->Release();
    mAudioCaptureBuffer.Release();
    return false;
  }
  DWORD BlockOffset = mBufferSize - 1;
  for (PINDEX i = 0; i < mBufferCount; i++) {
    Position [i].dwOffset = BlockOffset;
    Position [i].hEventNotify = notificationEvent[0];// all use same event
    BlockOffset += mBufferSize;
  }
  hr = Notify->SetNotificationPositions (mBufferCount, Position);
  Notify->Release();
  if (FAILED(hr)) { 
    PTRACE(4, "dsound\tInitCaptureBuffer : Notify interface query failed: " << GetErrorString(hr));
    mAudioCaptureBuffer.Release();
    return false;
  }
  delete [] Position ;
  bufferByteOffset = 0; // reset read position
  ResetEvent(notificationEvent[1]);
  return true;
}


PBoolean PSoundChannelDirectSound::Write (const void *buf, PINDEX len)
{
  lastWriteCount = 0;
  if (!IsOpen())
    return false;

  if (!mAudioPlaybackBuffer)
  {
    PTRACE(4, "dsound\tWrite Failed: Device not initialised :");
    return SetErrorValues(NotOpen, EBADF, LastWriteError);
  }
  char * src = (char *) buf;
  do {
    if (!WaitForPlayBufferFree())       // wait for output space to become available
      return false;                     // closed

    PWaitAndSignal mutex(bufferMutex);  // prevent closing while active

    // Write data from buf to circular buffer
    PINDEX writeCount = WriteToDXBuffer (src, PMIN ((PINDEX)available, len), bufferByteOffset);
    if (writeCount == P_MAX_INDEX)
      return false;

    src += writeCount;
    len -= writeCount;
    lastWriteCount += writeCount;
    bufferByteOffset += lastWriteCount;
    bufferByteOffset %= mDXBufferSize;
                                        // tell DX to play
    mAudioPlaybackBuffer->Play (0, 0, mStreaming ? DSBPLAY_LOOPING : 0L);
  }
  while (lastWriteCount < len);
  return true;
}


PBoolean PSoundChannelDirectSound::Read (void * buf, PINDEX len)
{
  lastReadCount = 0;
  if (!IsOpen())
    return false;

  if (!mAudioCaptureBuffer)
  {
    PTRACE(4, "dsound\tRead : Device not initialised ");
    return SetErrorValues(NotOpen, EBADF, LastWriteError);
  }
  char * dest = (char *) buf;
  do {
    if (!WaitForRecordBufferFull())     // sets bufferByteOffset and available
      return false;                     // closed

    PWaitAndSignal mutex(bufferMutex);  // prevent closing while active

    // Read from device buffer minimum between the data required and data available
    PINDEX readCount = ReadFromDXBuffer (dest, PMIN((PINDEX)available, len), bufferByteOffset);
    if (readCount == P_MAX_INDEX)
      return false;

    dest += readCount;
    len -= readCount;
    lastReadCount += readCount;
    bufferByteOffset += readCount;
    bufferByteOffset %= mDXBufferSize;
  }
  while (lastReadCount < len);
  return true;
}


PINDEX PSoundChannelDirectSound::WriteToDXBuffer (const void *buf,  PINDEX len, DWORD position) 
{
  if (!IsOpen())
    return P_MAX_INDEX;

  LPVOID lpvWrite1, lpvWrite2;
  DWORD dwLength1, dwLength2;
  PINDEX written = 0;
  HRESULT hr = mAudioPlaybackBuffer->Lock (position, len, &lpvWrite1, &dwLength1, &lpvWrite2, &dwLength2, 0L);
  if (hr == DSERR_BUFFERLOST) { // Buffer was lost, need to restore it
    mAudioPlaybackBuffer->Restore ();
    hr = mAudioPlaybackBuffer->Lock (position, len, &lpvWrite1, &dwLength1, &lpvWrite2, &dwLength2, 0L);
  }
  if (FAILED (hr)) {
    PTRACE(4, "dsound\tWriteToDXBuffer failed: " << GetErrorString(hr) << " len " << len << " pos " << position);
    return P_MAX_INDEX;
  }
  // Copy supplied buffer into locked DX memory
  memcpy (lpvWrite1, buf, dwLength1);
  if (lpvWrite2 != NULL)
    memcpy (lpvWrite2, (BYTE *) buf + dwLength1, dwLength2);

  written = dwLength1 + dwLength2;

  mAudioPlaybackBuffer->Unlock (lpvWrite1, dwLength1, lpvWrite2, dwLength2);

  return written;
}


PINDEX PSoundChannelDirectSound::ReadFromDXBuffer (const void * buf, PINDEX len, DWORD position)
{
  if (!IsOpen())
    return P_MAX_INDEX;

  LPVOID lpvRead1, lpvRead2;
  DWORD dwLength1, dwLength2;
  PINDEX read = 0;

  HRESULT hr = mAudioCaptureBuffer->Lock (position, len, &lpvRead1, &dwLength1, &lpvRead2, &dwLength2, 0L);
  if (FAILED(hr)) {
    PTRACE(4, "dsound\tReadFromDXBuffer Lock failed: " << GetErrorString(hr));
    return P_MAX_INDEX;
  }
  // Copy from DX locked memory into return buffer
  memcpy ((BYTE *)buf, lpvRead1, dwLength1);

  if (lpvRead2 != NULL)
    memcpy ((BYTE *) buf + dwLength1, lpvRead2, dwLength2);

  read = dwLength1 + dwLength2;

  mAudioCaptureBuffer->Unlock (lpvRead1, dwLength1, lpvRead2, dwLength2);

  return read;
}


PBoolean PSoundChannelDirectSound::PlaySound (const PSound & sound, PBoolean wait)
{
  mStreaming = false;

  if (!mAudioPlaybackBuffer)
      SetBuffers (sound.GetSize(), 1);

  if (!Write((const void *)sound, sound.GetSize()))
    return PFalse;

  if (wait)
    return WaitForPlayCompletion();
  return PTrue;
}


PBoolean PSoundChannelDirectSound::PlayFile (const PFilePath & filename, PBoolean wait)
{
  PMultiMediaFile mmio;
  PWaveFormat fileFormat;
  DWORD dataSize;
  if (!mmio.OpenWaveFile(filename, fileFormat, dataSize))
    return SetErrorValues(NotOpen, mmio.GetLastError() | PWIN32ErrorFlag, LastWriteError);

  Abort();
  mDXBufferSize = 0; // so SetFormat doesn't create buffer

  // Save old format and set to one loaded from file.
  unsigned numChannels = mWFX.nChannels;
  unsigned sampleRate = mWFX.nSamplesPerSec;
  unsigned bitsPerSample = mWFX.wBitsPerSample;

  SetFormat(fileFormat->nChannels, fileFormat->nSamplesPerSec, fileFormat->wBitsPerSample);

  int bufferSize = mWFX.nAvgBytesPerSec / 2;
  if (!SetBuffers (bufferSize, 4))
    SetFormat(numChannels, sampleRate, bitsPerSample);// restore audio format

  PBYTEArray buffer;
  mStreaming = false;

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

  return PTrue;
}


PBoolean PSoundChannelDirectSound::IsPlayBufferFree ()
{
  DWORD PlayPos; // byte offset from start of buffer to next byte directsound will play (end of where we can write)
  mAudioPlaybackBuffer->GetCurrentPosition (&PlayPos, 0);
  if (PlayPos <= bufferByteOffset) // wrapped around
    available = mDXBufferSize - bufferByteOffset + PlayPos;
  else
    available = PlayPos - bufferByteOffset;

  //if (available % mWFX.nBlockAlign) // always write multiples of sample frames
  //  available -= (available % mWFX.nBlockAlign);

  if ((PINDEX)available == mDXBufferSize) {
    DWORD dwStatus;
    mAudioPlaybackBuffer->GetStatus (&dwStatus);
    if ((dwStatus & DSBSTATUS_PLAYING) != 0) 
      available = 0;

    //PTRACE(4, "dsound\tPlayer buffer " << available << (((dwStatus & DSBSTATUS_PLAYING) == 0)? " empty" : " full"));
  }
  //else
  //  PTRACE(4, "dsound\tPlayer buffer " << available);

  return available >= (unsigned)mBufferSize;
}


PBoolean PSoundChannelDirectSound::WaitForPlayBufferFree ()
{
  ResetEvent(notificationEvent[0]);
  while (!IsPlayBufferFree()) { // always call last (after wait) to set member variables
    if (WaitForMultipleObjects(2, notificationEvent, FALSE, INFINITE) != WAIT_OBJECT_0) {
      PTRACE(4, "dsound\tPlayer abort");
      return false;
    }
  }
  return true;
}


PBoolean PSoundChannelDirectSound::HasPlayCompleted ()
{
  DWORD dwStatus;
  if (mAudioPlaybackBuffer != NULL) {
    mAudioPlaybackBuffer->GetStatus (&dwStatus);
    if (dwStatus & DSBSTATUS_PLAYING)
      return PFalse;
  }
  return PTrue;
}


PBoolean PSoundChannelDirectSound::WaitForPlayCompletion ()
{
  while (!HasPlayCompleted()) 
    Sleep (50);

  return PTrue;
}


PBoolean PSoundChannelDirectSound::RecordSound (PSound & /*sound*/)
{
  PTRACE(4, "dsound\tRecordSound unimplemented");
  return PFalse;
}


PBoolean PSoundChannelDirectSound::RecordFile (const PFilePath & /*filename*/)
{
  PTRACE(4, "dsound\tRecordFile unimplemented");
  return PFalse;
}


PBoolean PSoundChannelDirectSound::StartRecording ()
{
  if (!mAudioCaptureBuffer)
  {
    PTRACE(4, "dsound\tStartRecording: Device not initialised");
    // TODO: proper error reporting
    //return SetErrorValues(NotOpen, EBADF, LastWriteError);
    return false;
  }
  DWORD Status = 0;
  if (FAILED(mAudioCaptureBuffer->GetStatus(&Status))) {
    PTRACE(4, "dsound\tStartRecording: Failed GetStatus");
    //return SetErrorValues(NotOpen, EBADF, LastWriteError);
    return false;
  }
  if ((Status & DSCBSTATUS_CAPTURING) != 0)
    return true;

  bufferByteOffset = 0;
  if (FAILED(mAudioCaptureBuffer->Start (DSCBSTART_LOOPING))) {
    PTRACE(4, "dsound\tStartRecording: Failed Start");
    //return SetErrorValues(NotOpen, EBADF, LastWriteError);
    return false;
  }
  PTRACE(4, "dsound\tInitCaptureBuffer: Starting capture");
  return true;
}


PBoolean PSoundChannelDirectSound::IsRecordBufferFull ()
{
  if (!StartRecording())                // Start the first read
    return false;

  DWORD ReadPos;                        // byte offset from start of buffer to the end of the data that has been fully captured
  mAudioCaptureBuffer->GetCurrentPosition (0, &ReadPos);
  if (ReadPos < bufferByteOffset)        // wrapped around
    available = mDXBufferSize - bufferByteOffset + ReadPos;
  else
    available = ReadPos - bufferByteOffset;
                                        // always read multiples of sample frames
  available -= (available % mWFX.nBlockAlign);
  return available >= (unsigned)mBufferSize;
}


PBoolean PSoundChannelDirectSound::AreAllRecordBuffersFull ()
{
  PTRACE(4, "dsound\tAreAllRecordBuffersFull unimplemented");
  return PTrue;
}


PBoolean PSoundChannelDirectSound::WaitForRecordBufferFull ()
{
  ResetEvent(notificationEvent[0]);
  while (!IsRecordBufferFull()) {       // repeat after wait to set member variables
    if (WaitForMultipleObjects(2, notificationEvent, FALSE, INFINITE) != WAIT_OBJECT_0)
      return false;
  }
  return IsOpen();
}


PBoolean PSoundChannelDirectSound::WaitForAllRecordBuffersFull ()
{

  PTRACE(4, "dsound\tWaitForAllRecordBuffersFull unimplemented");
  return PFalse;
}


PBoolean PSoundChannelDirectSound::SetVolume (unsigned newVal)
{

  PBoolean no_error=PTrue;
  HRESULT hr;

  switch (mDirection) {
  case Player:
    if (mAudioPlaybackBuffer) {
      // SetVolume is already logarithmic and is in 100ths of a decibel attenuation,
      // 0=max gain, 10,000 is min gain.
      if (FAILED (hr = mAudioPlaybackBuffer->SetVolume((MaxVolume - newVal)*100))) {
        PTRACE(4, "PSoundChannelDirectSound::SetVolume failed " << GetErrorString(hr));
        no_error = PFalse;
      }
    }
    else
      PTRACE(4, "PSoundChannelDirectSound::SetVolume Failed mAudioPlaybackBuffer is NULLL (huh?)");
    break;

  case Recorder:
    // DirectX does not let you change the capture buffer volume
    mVolume = newVal;
    break;
  }
  return no_error;
}


PBoolean PSoundChannelDirectSound::GetVolume (unsigned &devVol)
{
  switch (mDirection) 
  {
    case Player:
      if (mAudioPlaybackBuffer) {
        long volume;
        HRESULT hr = mAudioPlaybackBuffer->GetVolume(&volume);
        if (SUCCEEDED(hr)) {
          devVol = (unsigned int)(MaxVolume - volume/100);
          return true;
        }
        PTRACE(4, "PSoundChannelDirectSound::GetVolume failed " << GetErrorString(hr));
      }
      break;

    case Recorder:
      // DirectX does not let you change the capture buffer volume
      devVol = mVolume;
      break;
  }
  return false;
}


#else

  #ifdef _MSC_VER
    #pragma message("Direct Sound support DISABLED")
  #endif

#endif // P_DIRECTSOUND
