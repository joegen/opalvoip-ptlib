/*
 * sound.cxx
 *
 * Implementation of sound classes for Win32
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: sound.cxx,v $
 * Revision 1.1  1999/02/16 06:02:07  robertj
 * Major implementation to Linux OSS model
 *
 */

#include <ptlib.h>
#include <mmsystem.h>
#include <process.h>


class PMultiMediaFile
{
  public:
    PMultiMediaFile();
    ~PMultiMediaFile();

    BOOL Open(const PFilePath & filename, DWORD dwOpenFlags, LPMMIOINFO lpmmioinfo = NULL);
    BOOL Close(UINT wFlags = 0);
    BOOL Ascend(MMCKINFO & ckinfo, UINT wFlags = 0);
    BOOL Descend(UINT wFlags, MMCKINFO & ckinfo, LPMMCKINFO lpckParent = NULL);
    BOOL Read(void * data, PINDEX len);
    BOOL CreateChunk(MMCKINFO & ckinfo, UINT wFlags = 0);
    BOOL Write(const void * data, PINDEX len);

    DWORD GetLastError() const { return dwLastError; }

  protected:
    HMMIO hmmio;
    DWORD dwLastError;
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PMultiMediaFile::PMultiMediaFile()
{
  hmmio = NULL;
}


PMultiMediaFile::~PMultiMediaFile()
{
  Close();
}


BOOL PMultiMediaFile::Open(const PFilePath & filename,
                          DWORD dwOpenFlags,
                          LPMMIOINFO lpmmioinfo)
{
  MMIOINFO local_mmioinfo;
  if (lpmmioinfo == NULL) {
    lpmmioinfo = &local_mmioinfo;
    memset(lpmmioinfo, 0, sizeof(local_mmioinfo));
  }

  hmmio = mmioOpen((char *)(const char *)filename, lpmmioinfo, dwOpenFlags);

  dwLastError = lpmmioinfo->wErrorRet;

  return hmmio != NULL;
}


BOOL PMultiMediaFile::Close(UINT wFlags)
{
  if (hmmio == NULL)
    return FALSE;

  mmioClose(hmmio, wFlags);
  hmmio = NULL;
  return TRUE;
}


BOOL PMultiMediaFile::Ascend(MMCKINFO & ckinfo, UINT wFlags)
{
  dwLastError = mmioAscend(hmmio, &ckinfo, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


BOOL PMultiMediaFile::Descend(UINT wFlags, MMCKINFO & ckinfo, LPMMCKINFO lpckParent)
{
  dwLastError = mmioDescend(hmmio, &ckinfo, lpckParent, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


BOOL PMultiMediaFile::Read(void * data, PINDEX len)
{
  return mmioRead(hmmio, (char *)data, len) == len;
}


BOOL PMultiMediaFile::CreateChunk(MMCKINFO & ckinfo, UINT wFlags)
{
  dwLastError = mmioCreateChunk(hmmio, &ckinfo, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


BOOL PMultiMediaFile::Write(const void * data, PINDEX len)
{
  return mmioWrite(hmmio, (char *)data, len) == len;
}


///////////////////////////////////////////////////////////////////////////////

PWaveFormat::PWaveFormat()
{
  size = 0;
  waveFormat = NULL;
}


PWaveFormat::~PWaveFormat()
{
  if (waveFormat != NULL)
    free(waveFormat);
}


PWaveFormat::PWaveFormat(const PWaveFormat & fmt)
{
  waveFormat = (WAVEFORMATEX *)malloc(fmt.size);
  PAssert(waveFormat != NULL, POutOfMemory);

  memcpy(waveFormat, fmt.waveFormat, fmt.size);
}


PWaveFormat & PWaveFormat::operator=(const PWaveFormat & fmt)
{
  if (waveFormat != NULL)
    free(waveFormat);

  waveFormat = (WAVEFORMATEX *)malloc(fmt.size);
  PAssert(waveFormat != NULL, POutOfMemory);

  memcpy(waveFormat, fmt.waveFormat, fmt.size);
  return *this;
}


void PWaveFormat::SetFormat(unsigned numChannels,
                            unsigned sampleRate,
                            unsigned bitsPerSample)
{
  if (waveFormat != NULL)
    free(waveFormat);

  waveFormat = (WAVEFORMATEX *)malloc(sizeof(WAVEFORMATEX));
  PAssert(waveFormat != NULL, POutOfMemory);

  waveFormat->wFormatTag = WAVE_FORMAT_PCM;
  waveFormat->nChannels = (WORD)numChannels;
  waveFormat->nSamplesPerSec = sampleRate;
  waveFormat->wBitsPerSample = (WORD)bitsPerSample;
  waveFormat->nBlockAlign = 2;
  waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec*waveFormat->nBlockAlign;
  waveFormat->cbSize = 0;
}


void PWaveFormat::SetFormat(const void * data, PINDEX size)
{
  SetSize(size);
  memcpy(waveFormat, data, size);
}


BOOL PWaveFormat::SetSize(PINDEX sz)
{
  if (waveFormat != NULL)
    free(waveFormat);

  size = sz;
  if (sz == 0)
    waveFormat = NULL;
  else
    waveFormat = (WAVEFORMATEX *)malloc(sz);

  return waveFormat != NULL;
}


///////////////////////////////////////////////////////////////////////////////

PSound::PSound(unsigned format,
               unsigned channels,
               unsigned samplesPerSecond,
               unsigned bitsPerSample,
               PINDEX   bufferSize,
               const BYTE * buffer)
{
  encoding = format;
  numChannels = channels;
  sampleRate = samplesPerSecond;
  sampleSize = bitsPerSample;
  SetSize(bufferSize);
  if (buffer != NULL)
    memcpy(GetPointer(), buffer, bufferSize);
}


PSound::PSound(const PFilePath & filename)
{
  encoding = 0;
  numChannels = 1;
  sampleRate = 8000;
  sampleSize = 16;
  Load(filename);
}


BOOL PSound::Load(const PFilePath & filename)
{
  // Open wave file
  PMultiMediaFile mmio;
  if (!mmio.Open(filename, MMIO_READ | MMIO_ALLOCBUF)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  MMCKINFO mmParentChunk, mmSubChunk;
  dwLastError = MMSYSERR_NOERROR;

  // Locate a 'RIFF' chunk with a 'WAVE' form type
  mmParentChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  if (!mmio.Descend(MMIO_FINDRIFF, mmParentChunk)) {
    if (dwLastError != WAVERR_BADFORMAT)
      dwLastError = mmio.GetLastError();
    return FALSE;
  }

  // Find the format chunk
  mmSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (!mmio.Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  // Get the size of the format chunk, allocate memory for it
  PWaveFormat waveFormat;
  if (!waveFormat.SetSize(mmSubChunk.cksize)) {
    dwLastError = MMSYSERR_NOMEM;
    return FALSE;
  }

  // Read the format chunk
  if (!mmio.Read(waveFormat.GetPointer(), waveFormat.GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  encoding = waveFormat->wFormatTag;
  numChannels = waveFormat->nChannels;
  sampleRate = waveFormat->nSamplesPerSec;
  sampleSize = waveFormat->wBitsPerSample;

  // Ascend out of the format subchunk
  mmio.Ascend(mmSubChunk);

  // Find the data subchunk
  mmSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (!mmio.Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  // Get the size of the data subchunk
  if (mmSubChunk.cksize == 0) {
    dwLastError = MMSYSERR_INVALPARAM;
    return FALSE;
  }
      
  // Allocate and lock memory for the waveform data.
  if (!SetSize(mmSubChunk.cksize)) {
    dwLastError = MMSYSERR_NOMEM;
    return FALSE;
  }

  // Read the waveform data subchunk
  if (!mmio.Read(GetPointer(), GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  return TRUE;
}


BOOL PSound::Save(const PFilePath & filename)
{
  // Open wave file
  PMultiMediaFile mmio;
  if (!mmio.Open(filename, MMIO_CREATE|MMIO_WRITE)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  PWaveFormat waveFormat;
  if (encoding == 0)
    waveFormat.SetFormat(numChannels, sampleRate, sampleSize);
  else {
    waveFormat.SetSize(GetFormatInfoSize());
    memcpy(waveFormat.GetPointer(), GetFormatInfoData(), GetFormatInfoSize());
  }

  MMCKINFO mmChunk;
  mmChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  mmChunk.cksize = 4 + // Form type
                   4 + sizeof(DWORD) + waveFormat.GetSize() + // fmt chunk
                   4 + sizeof(DWORD) + GetSize();             // data chunk

  // Create a RIFF chunk
  if (!mmio.CreateChunk(mmChunk, MMIO_CREATERIFF)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  // Save the format sub-chunk
  mmChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  mmChunk.cksize = waveFormat.GetSize();
  if (!mmio.CreateChunk(mmChunk)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  if (!mmio.Write(&waveFormat, waveFormat.GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  // Save the data sub-chunk
  mmChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  mmChunk.cksize = GetSize();
  if (!mmio.CreateChunk(mmChunk)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  if (!mmio.Write(GetPointer(), GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  return TRUE;
}



///////////////////////////////////////////////////////////////////////////////

PWaveBuffer::PWaveBuffer(PINDEX sz)
 : PBYTEArray(sz)
{
  hWaveOut = NULL;
  hWaveIn = NULL;
  link = NULL;
}


PWaveBuffer::~PWaveBuffer()
{
  Release();
}


void PWaveBuffer::PrepareCommon(PINDEX count)
{
  Release();

  memset(&header, 0, sizeof(header));
  header.lpData = (char *)GetPointer();
  header.dwBufferLength = count;
  header.dwUser = (DWORD)this;
}


DWORD PWaveBuffer::Prepare(HWAVEOUT hOut, PINDEX count)
{
  // Set up WAVEHDR structure and prepare it to be written to wave device
  PrepareCommon(count);
  hWaveOut = hOut;
  return waveOutPrepareHeader(hWaveOut, &header, sizeof(header));
}


DWORD PWaveBuffer::Prepare(HWAVEIN hIn)
{
  // Set up WAVEHDR structure and prepare it to be read from wave device
  PrepareCommon(GetSize());
  hWaveIn = hIn;
  return waveInPrepareHeader(hWaveIn, &header, sizeof(header));
}


DWORD PWaveBuffer::Release()
{
  DWORD err = MMSYSERR_NOERROR;

  if (hWaveOut != NULL) {
    err = waveOutUnprepareHeader(hWaveOut, &header, sizeof(header));
    hWaveOut = NULL;
  }

  if (hWaveIn != NULL) {
    err = waveInUnprepareHeader(hWaveIn, &header, sizeof(header));
    hWaveIn = NULL;
  }

  return err;
}


///////////////////////////////////////////////////////////////////////////////

PSoundChannel::PSoundChannel()
{
  Construct();
}


PSoundChannel::PSoundChannel(const PString & device,
                             Directions dir,
                             unsigned numChannels,
                             unsigned sampleRate,
                             unsigned bitsPerSample)
{
  Construct();
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}


void PSoundChannel::Construct()
{
  direction = Player;
  hWaveOut = NULL;
  hWaveIn = NULL;

  waveFormat.SetFormat(1, 8000, 16);

  hEventEnd  = CreateEvent(NULL, TRUE, FALSE, NULL);
  hEventDone = CreateEvent(NULL, TRUE, FALSE, NULL);

  int err = _beginthread(StaticThreadMain, 2048, this);
  if (err == -1)
    hThread = NULL;
  else {
    hThread = (HANDLE)err;
    SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
  }

  InitializeCriticalSection(&mutex);

  extractByteOffset = P_MAX_INDEX;
  extractBufferIndex = 0;
  insertBufferIndex = 0;

  SetBuffers(2, 32768);
}


PSoundChannel::~PSoundChannel()
{
  Close();

  if (hEventEnd != NULL)
    SetEvent(hEventEnd);

  if (hThread != NULL)
    WaitForSingleObject(hThread, 10000);

  if (hEventDone != NULL)
    DeleteObject(hEventDone);

  if (hEventEnd != NULL)
    DeleteObject(hEventEnd);

  DeleteCriticalSection(&mutex);
}


PString PSoundChannel::GetName() const
{
  return deviceName;
}


PStringArray PSoundChannel::GetDeviceNames(Directions dir)
{
  PStringArray array;

  unsigned numDevs, id;

  switch (dir) {
    case Player :
      numDevs = waveOutGetNumDevs();
      for (id = 0; id < numDevs; id++) {
        WAVEOUTCAPS caps;
        if (waveOutGetDevCaps(id, &caps, sizeof(caps)) == 0)
          array[array.GetSize()] = caps.szPname;
      }
      break;

    case Recorder :
      numDevs = waveInGetNumDevs();
      for (id = 0; id < numDevs; id++) {
        WAVEINCAPS caps;
        if (waveInGetDevCaps(id, &caps, sizeof(caps)) == 0)
          array[array.GetSize()] = caps.szPname;
      }
      break;
  }

  return array;
}


BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  Close();

  BOOL bad = TRUE;
  unsigned id = 0;

  if (device[0] == '#') {
    id = device.Mid(1).AsUnsigned();
    switch (dir) {
      case Player :
        if (id < waveOutGetNumDevs()) {
          WAVEOUTCAPS caps;
          osError = waveOutGetDevCaps(id, &caps, sizeof(caps));
          if (osError == 0) {
            deviceName = caps.szPname;
            bad = FALSE;
          }
        }
        break;

      case Recorder :
        if (id < waveInGetNumDevs()) {
          WAVEINCAPS caps;
          osError = waveInGetDevCaps(id, &caps, sizeof(caps));
          if (osError == 0) {
            deviceName = caps.szPname;
            bad = FALSE;
          }
        }
        break;
    }
  }
  else {
    switch (dir) {
      case Player :
        for (id = 0; id < waveOutGetNumDevs(); id++) {
          WAVEOUTCAPS caps;
          osError = waveOutGetDevCaps(id, &caps, sizeof(caps));
          if (osError == 0 && stricmp(caps.szPname, device) == 0) {
            deviceName = caps.szPname;
            bad = FALSE;
            break;
          }
        }
        break;

      case Recorder :
        for (id = 0; id < waveInGetNumDevs(); id++) {
          WAVEINCAPS caps;
          osError = waveInGetDevCaps(id, &caps, sizeof(caps));
          if (osError == 0 && stricmp(caps.szPname, device) == 0) {
            deviceName = caps.szPname;
            bad = FALSE;
            break;
          }
        }
        break;
    }
  }

  if (bad) {
    osError = MMSYSERR_BADDEVICEID;
    return FALSE;
  }

  waveFormat.SetFormat(numChannels, sampleRate, bitsPerSample);

  direction = dir;
  return OpenDevice(id);
}


BOOL PSoundChannel::OpenDevice(unsigned id)
{
  if (hWaveOut != NULL) {
    osError = waveOutClose(hWaveOut);
    hWaveOut = NULL;
  }

  if (hWaveIn != NULL) {
    osError = waveInClose(hWaveIn);
    hWaveIn = NULL;
  }

  switch (direction) {
    case Player :
      osError = waveOutOpen(&hWaveOut, id, waveFormat,
                            (DWORD)hEventDone, 0, CALLBACK_EVENT);
      break;

    case Recorder :
      osError = waveInOpen(&hWaveIn, id, waveFormat,
                           (DWORD)hEventDone, 0, CALLBACK_EVENT);
      break;
  }

  if (osError != MMSYSERR_NOERROR)
    return FALSE;

  EnterCriticalSection(&mutex);
    extractByteOffset = P_MAX_INDEX;
    extractBufferIndex = 0;
    insertBufferIndex = 0;
  LeaveCriticalSection(&mutex);

  os_handle = id;
  return TRUE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  Abort();

  waveFormat.SetFormat(numChannels, sampleRate, bitsPerSample);

  return OpenDevice(os_handle);
}


BOOL PSoundChannel::Close()
{
  if (!IsOpen())
    return FALSE;

  Abort();

  os_handle = -1;
  return TRUE;
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0, PInvalidParameter);

  BOOL ok = TRUE;

  EnterCriticalSection(&mutex);
    if (!buffers.SetSize(count))
      ok = FALSE;
    else {
      for (PINDEX i = 0; i < count; i++) {
        if (buffers.GetAt(i) == NULL)
          buffers.SetAt(i, new PWaveBuffer(size));
        if (!buffers[i].SetSize(size))
          ok = FALSE;
      }
    }

    extractByteOffset = P_MAX_INDEX;
    extractBufferIndex = 0;
    insertBufferIndex = 0;
  LeaveCriticalSection(&mutex);

  return ok;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  count = buffers.GetSize();

  if (count == 0)
    size = 0;
  else
    size = buffers[0].GetSize();

  return TRUE;
}


BOOL PSoundChannel::Read(void * data, PINDEX size)
{
  lastReadCount = 0;

  if (hWaveIn == NULL) {
    osError = MMSYSERR_NOTSUPPORTED;
    return FALSE;
  }

  if (extractByteOffset == P_MAX_INDEX) {
    // Start the first read, queue all the buffers
    for (PINDEX i = 0; i < buffers.GetSize(); i++) {
      buffers[i].Prepare(hWaveIn);
      osError = waveInAddBuffer(hWaveIn, &buffers[i].header, sizeof(WAVEHDR));
      if (osError != MMSYSERR_NOERROR)
        return FALSE;
    }

    insertBufferIndex = 0;
    extractBufferIndex = 0;
    extractByteOffset = 0;

    osError = waveInStart(hWaveIn); // start recording
    if (osError != MMSYSERR_NOERROR) {
      extractByteOffset = P_MAX_INDEX;
      return FALSE;
    }
  }

  PWaveBuffer & buffer = buffers[extractBufferIndex];
  WAVEHDR & header = buffer.header;

  while ((header.dwFlags&WHDR_DONE) == 0) {
    if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0)
      return FALSE;
  }

  lastReadCount = header.dwBytesRecorded - extractByteOffset;
  if (lastReadCount > size)
    lastReadCount = size;

  memcpy(data, &buffers[extractBufferIndex][extractByteOffset], lastReadCount);

  extractByteOffset += lastReadCount;

  if (extractByteOffset >= (PINDEX)header.dwBytesRecorded) {
    EnterCriticalSection(&mutex);
      PINDEX nextBufferIndex = (extractBufferIndex+1)%buffers.GetSize();
      if (nextBufferIndex != insertBufferIndex) {
        extractBufferIndex = nextBufferIndex;
        extractByteOffset = 0;
      }
    LeaveCriticalSection(&mutex);
  }

  return lastReadCount > 0;
}


BOOL PSoundChannel::Write(const void * data, PINDEX size)
{
  lastWriteCount = 0;

  if (hWaveOut == NULL) {
    osError = MMSYSERR_NOTSUPPORTED;
    return FALSE;
  }

  const BYTE * ptr = (const BYTE *)data;

  while (size > 0) {
    PINDEX nextBufferIndex;
    if (!GetNextPlayBuffer(nextBufferIndex))
      return FALSE; // No free buffers

    // Can't write more than a buffer full
    PINDEX count = buffers[nextBufferIndex].GetSize();
    if (count > size)
      count = size;
    memcpy(buffers[nextBufferIndex].GetPointer(), data, count);

    if (!QueuePlayBuffer(nextBufferIndex, count))
      return FALSE;

    lastWriteCount += count;
    size -= count;
    ptr += count;
  }

  return TRUE;
}


BOOL PSoundChannel::GetNextPlayBuffer(PINDEX & nextBufferIndex)
{
  // Get the next free buffer
  EnterCriticalSection(&mutex);
    nextBufferIndex = (insertBufferIndex+1)%buffers.GetSize();
    BOOL ok = nextBufferIndex != extractBufferIndex;
  LeaveCriticalSection(&mutex);

  return ok; // No free buffers
}


BOOL PSoundChannel::QueuePlayBuffer(PINDEX nextBufferIndex, PINDEX count)
{
  PWaveBuffer & buffer = buffers[nextBufferIndex];
  buffer.Prepare(hWaveOut, count);

  // Back into mutexed stuff
  EnterCriticalSection(&mutex);
    if (insertBufferIndex == extractBufferIndex) {
      // Empty queue, start the write directly
      osError = waveOutWrite(hWaveOut, &buffer.header, sizeof(WAVEHDR));
      if (osError != MMSYSERR_NOERROR) {
        LeaveCriticalSection(&mutex);
        buffer.Release();
        return FALSE;
      }
    }
    insertBufferIndex = nextBufferIndex;
  LeaveCriticalSection(&mutex);

  return TRUE;
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
  Abort();

  BOOL ok = FALSE;

  PINDEX bufferSize;
  PINDEX bufferCount;
  GetBuffers(bufferSize, bufferCount);

  unsigned numChannels = waveFormat->nChannels;
  unsigned sampleRate = waveFormat->nSamplesPerSec;
  unsigned bitsPerSample = waveFormat->wBitsPerSample;
  if (sound.GetEncoding() == 0)
    ok = SetFormat(sound.GetChannels(), sound.GetSampleRate(), sound.GetSampleSize());
  else {
    waveFormat.SetFormat(sound.GetFormatInfoData(), sound.GetFormatInfoSize());
    ok = OpenDevice(os_handle);
  }

  if (ok) {
    PINDEX soundSize = sound.GetSize();
    if (SetBuffers(soundSize, 1) && Write((const BYTE *)sound, soundSize)) {
      if (wait)
        ok = WaitForPlayCompletion();
    }
    else
      ok = FALSE;
  }

  SetFormat(numChannels, sampleRate, bitsPerSample);
  SetBuffers(bufferSize, bufferCount);
  return ok;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  Abort();

  // Open wave file
  PMultiMediaFile mmio;
  if (!mmio.Open(filename, MMIO_READ | MMIO_ALLOCBUF)) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  MMCKINFO mmParentChunk, mmSubChunk;
  osError = MMSYSERR_NOERROR;

  // Locate a 'RIFF' chunk with a 'WAVE' form type
  mmParentChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  if (!mmio.Descend(MMIO_FINDRIFF, mmParentChunk)) {
    if (osError != WAVERR_BADFORMAT)
      osError = mmio.GetLastError();
    return FALSE;
  }

  // Find the format chunk
  mmSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (!mmio.Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk)) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  // Get the size of the format chunk, allocate memory for it
  PWaveFormat fileFormat;
  if (!fileFormat.SetSize(mmSubChunk.cksize)) {
    osError = MMSYSERR_NOMEM;
    return FALSE;
  }

  // Read the format chunk
  if (!mmio.Read(fileFormat.GetPointer(), fileFormat.GetSize())) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  // Ascend out of the format subchunk
  mmio.Ascend(mmSubChunk);

  // Find the data subchunk
  mmSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (!mmio.Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk)) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  // Get the size of the data subchunk
  if (mmSubChunk.cksize == 0) {
    osError = MMSYSERR_INVALPARAM;
    return FALSE;
  }

  unsigned numChannels = waveFormat->nChannels;
  unsigned sampleRate = waveFormat->nSamplesPerSec;
  unsigned bitsPerSample = waveFormat->wBitsPerSample;
  waveFormat = fileFormat;
  if (!OpenDevice(os_handle)) {
    SetFormat(numChannels, sampleRate, bitsPerSample);
    return FALSE;
  }

  while (mmSubChunk.cksize > 0) {
    PINDEX nextBufferIndex;
    while (!GetNextPlayBuffer(nextBufferIndex)) {
      // No free buffers, so wait for one
      WaitForSingleObject(hEventDone, INFINITE);
    }

    PINDEX count = buffers[nextBufferIndex].GetSize();
    if (count > (PINDEX)mmSubChunk.cksize)
      count = mmSubChunk.cksize;

    // Read the waveform data subchunk
    if (!mmio.Read(buffers[nextBufferIndex].GetPointer(), count))
      break;

    if (!QueuePlayBuffer(nextBufferIndex, count))
      break;

    mmSubChunk.cksize -= count;
  }

  if (mmSubChunk.cksize == 0 && wait)
    WaitForPlayCompletion();

  SetFormat(numChannels, sampleRate, bitsPerSample);
  return TRUE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  EnterCriticalSection(&mutex);
    BOOL completed = insertBufferIndex == extractBufferIndex;
  LeaveCriticalSection(&mutex);
  return completed;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  while (!HasPlayCompleted()) {
    if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0)
      return FALSE;
  }

  return TRUE;
}


BOOL PSoundChannel::Abort()
{
  if (hWaveOut != NULL)
    osError = waveOutReset(hWaveOut);

  if (hWaveIn != NULL)
    osError = waveInReset(hWaveIn);

  EnterCriticalSection(&mutex);
    extractByteOffset = P_MAX_INDEX;
    extractBufferIndex = 0;
    insertBufferIndex = 0;
  LeaveCriticalSection(&mutex);

  for (PINDEX i = 0; i < buffers.GetSize(); i++)
    buffers[i].Release();

  return osError == MMSYSERR_NOERROR;
}


PString PSoundChannel::GetErrorText() const
{
  PString str;

  if (direction == Recorder) {
    if (waveInGetErrorText(osError, str.GetPointer(256), 256) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText();
  }
  else {
    if (waveOutGetErrorText(osError, str.GetPointer(256), 256) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText();
  }

  return str;
}


void PSoundChannel::StaticThreadMain(void * arg)
{
  ((PSoundChannel *)arg)->ThreadMain();
}


void PSoundChannel::ThreadMain()
{
  HANDLE handles[2];
  handles[0] = hEventEnd;
  handles[1] = hEventDone;

  for (;;) {
    switch (WaitForMultipleObjects(sizeof(handles), handles, FALSE, INFINITE)) {
      case WAIT_OBJECT_0 : // hEventEnd
        return;

      case WAIT_OBJECT_0+1 : // hEventDone
        if (hWaveOut != NULL) {
          EnterCriticalSection(&mutex);
            extractBufferIndex = (extractBufferIndex+1)%buffers.GetSize();
            if (extractBufferIndex != insertBufferIndex)
              waveOutWrite(hWaveOut, &buffers[extractBufferIndex].header, sizeof(WAVEHDR));
          LeaveCriticalSection(&mutex);
        }
        else if (hWaveIn != NULL && extractByteOffset != P_MAX_INDEX) {
          EnterCriticalSection(&mutex);
            while (insertBufferIndex != extractBufferIndex) {
              insertBufferIndex = (insertBufferIndex+1)%buffers.GetSize();
              PWaveBuffer & buffer = buffers[insertBufferIndex];
              buffer.Prepare(hWaveIn);
              waveInAddBuffer(hWaveIn, &buffer.header, sizeof(WAVEHDR));
            }
          LeaveCriticalSection(&mutex);
        }

        ResetEvent(hEventDone);
        break;

      default :
        if (::GetLastError() != 0)
          return;
    }
  }
}



// End of File ///////////////////////////////////////////////////////////////
