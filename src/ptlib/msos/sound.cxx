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
 * Revision 1.22  2001/09/09 02:17:11  yurik
 * Returned to 1.20
 *
 * Revision 1.20  2001/07/01 02:45:01  yurik
 * WinCE compiler wants implicit cast to format
 *
 * Revision 1.19  2001/05/04 09:38:07  robertj
 * Fixed problem with some WAV files having small WAVEFORMATEX chunk.
 *
 * Revision 1.18  2001/04/10 00:51:11  robertj
 * Fixed bug in using incorrect function to delete event handle, thanks Victor H.
 *
 * Revision 1.17  2001/03/15 23:39:29  robertj
 * Fixed bug with trying to write block larger than one buffer, thanks Norbert Oertel
 *
 * Revision 1.16  2001/02/07 04:45:54  robertj
 * Added functions to get current sound channel format parameters.
 *
 * Revision 1.15  2000/07/04 04:30:47  robertj
 * Fixed shutdown issues with buffers in use, again.
 *
 * Revision 1.14  2000/07/01 09:39:31  robertj
 * Fixed shutdown issues with buffers in use.
 *
 * Revision 1.13  2000/06/29 00:39:29  robertj
 * Fixed bug when PWaveFormat is assigned to itself.
 *
 * Revision 1.12  2000/05/22 07:17:50  robertj
 * Fixed missing initialisation of format data block in Win32 PSound::Load().
 *
 * Revision 1.11  2000/05/01 05:59:11  robertj
 * Added mutex to PSoundChannel buffer structure.
 *
 * Revision 1.10  2000/03/04 10:15:32  robertj
 * Added simple play functions for sound files.
 *
 * Revision 1.9  2000/02/17 11:33:33  robertj
 * Changed PSoundChannel::Write so blocks instead of error if no buffers available.
 *
 * Revision 1.8  1999/10/09 01:22:07  robertj
 * Fixed error display for sound channels.
 *
 * Revision 1.7  1999/09/23 04:28:44  robertj
 * Allowed some Win32 only access to wave format in sound channel
 *
 * Revision 1.6  1999/07/08 08:39:53  robertj
 * Fixed bug when breaking block by closing the PSoundChannel in other thread.
 *
 * Revision 1.5  1999/06/24 14:01:25  robertj
 * Fixed bug in not returning correct default recorder (waveIn) device.
 *
 * Revision 1.4  1999/06/07 01:36:28  robertj
 * Fixed incorrect;ly set block alignment in sound structure.
 *
 * Revision 1.3  1999/05/28 14:04:51  robertj
 * Added function to get default audio device.
 *
 * Revision 1.2  1999/02/22 10:15:15  robertj
 * Sound driver interface implementation to Linux OSS specification.
 *
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

    BOOL CreateWaveFile(const PFilePath & filename,
                        const PWaveFormat & waveFormat,
                        DWORD dataSize);
    BOOL OpenWaveFile(const PFilePath & filename,
                      PWaveFormat & waveFormat,
                      DWORD & dataSize);

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


BOOL PMultiMediaFile::CreateWaveFile(const PFilePath & filename,
                                     const PWaveFormat & waveFormat,
                                     DWORD dataSize)
{
  if (!Open(filename, MMIO_CREATE|MMIO_WRITE))
    return FALSE;

  MMCKINFO mmChunk;
  mmChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  mmChunk.cksize = 4 + // Form type
                   4 + sizeof(DWORD) + waveFormat.GetSize() + // fmt chunk
                   4 + sizeof(DWORD) + dataSize;              // data chunk

  // Create a RIFF chunk
  if (!CreateChunk(mmChunk, MMIO_CREATERIFF))
    return FALSE;

  // Save the format sub-chunk
  mmChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  mmChunk.cksize = waveFormat.GetSize();
  if (!CreateChunk(mmChunk))
    return FALSE;

  if (!Write(waveFormat, waveFormat.GetSize()))
    return FALSE;

  // Save the data sub-chunk
  mmChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  mmChunk.cksize = dataSize;
  return CreateChunk(mmChunk);
}


BOOL PMultiMediaFile::OpenWaveFile(const PFilePath & filename,
                                   PWaveFormat  & waveFormat,
                                   DWORD & dataSize)
{
  // Open wave file
  if (!Open(filename, MMIO_READ | MMIO_ALLOCBUF))
    return FALSE;

  MMCKINFO mmParentChunk, mmSubChunk;
  dwLastError = MMSYSERR_NOERROR;

  // Locate a 'RIFF' chunk with a 'WAVE' form type
  mmParentChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  if (!Descend(MMIO_FINDRIFF, mmParentChunk))
    return FALSE;

  // Find the format chunk
  mmSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (!Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk))
    return FALSE;

  // Get the size of the format chunk, allocate memory for it
  if (!waveFormat.SetSize(mmSubChunk.cksize))
    return FALSE;

  // Read the format chunk
  if (!Read(waveFormat.GetPointer(), waveFormat.GetSize()))
    return FALSE;

  // Ascend out of the format subchunk
  Ascend(mmSubChunk);

  // Find the data subchunk
  mmSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (!Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk))
    return FALSE;

  // Get the size of the data subchunk
  if (mmSubChunk.cksize == 0) {
    dwLastError = MMSYSERR_INVALPARAM;
    return FALSE;
  }

  dataSize = mmSubChunk.cksize;
  return TRUE;
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
  size = fmt.size;
  waveFormat = (WAVEFORMATEX *)malloc(size);
  PAssert(waveFormat != NULL, POutOfMemory);

  memcpy(waveFormat, fmt.waveFormat, size);
}


PWaveFormat & PWaveFormat::operator=(const PWaveFormat & fmt)
{
  if (this == &fmt)
    return *this;

  if (waveFormat != NULL)
    free(waveFormat);

  size = fmt.size;
  waveFormat = (WAVEFORMATEX *)malloc(size);
  PAssert(waveFormat != NULL, POutOfMemory);

  memcpy(waveFormat, fmt.waveFormat, size);
  return *this;
}


void PWaveFormat::PrintOn(ostream & out) const
{
  if (waveFormat == NULL)
    out << "<null>";
  else {
    out << waveFormat->wFormatTag << ','
        << waveFormat->nChannels << ','
        << waveFormat->nSamplesPerSec << ','
        << waveFormat->nAvgBytesPerSec << ','
        << waveFormat->nBlockAlign << ','
        << waveFormat->wBitsPerSample;
    if (waveFormat->cbSize > 0) {
      out << hex << setfill('0');
      const BYTE * ptr = (const BYTE *)&waveFormat[1];
      for (PINDEX i = 0; i < waveFormat->cbSize; i++)
        out << ',' << setw(2) << (unsigned)*ptr++;
      out << dec << setfill(' ');
    }
  }
}


void PWaveFormat::ReadFrom(istream &)
{
}


void PWaveFormat::SetFormat(unsigned numChannels,
                            unsigned sampleRate,
                            unsigned bitsPerSample)
{
  PAssert(numChannels == 1 || numChannels == 2, PInvalidParameter);
  PAssert(bitsPerSample == 8 || bitsPerSample == 16, PInvalidParameter);

  if (waveFormat != NULL)
    free(waveFormat);

  size = sizeof(WAVEFORMATEX);
  waveFormat = (WAVEFORMATEX *)malloc(sizeof(WAVEFORMATEX));
  PAssert(waveFormat != NULL, POutOfMemory);

  waveFormat->wFormatTag = WAVE_FORMAT_PCM;
  waveFormat->nChannels = (WORD)numChannels;
  waveFormat->nSamplesPerSec = sampleRate;
  waveFormat->wBitsPerSample = (WORD)bitsPerSample;
  waveFormat->nBlockAlign = (WORD)(numChannels*(bitsPerSample+7)/8);
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
  else {
    if (sz < sizeof(WAVEFORMATEX))
      sz = sizeof(WAVEFORMATEX);
    waveFormat = (WAVEFORMATEX *)calloc(sz, 1);
    waveFormat->cbSize = (WORD)(sz - sizeof(WAVEFORMATEX));
  }

  return waveFormat != NULL;
}


///////////////////////////////////////////////////////////////////////////////

PSound::PSound(unsigned channels,
               unsigned samplesPerSecond,
               unsigned bitsPerSample,
               PINDEX   bufferSize,
               const BYTE * buffer)
{
  encoding = 0;
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


PSound & PSound::operator=(const PBYTEArray & data)
{
  PBYTEArray::operator=(data);
  return *this;
}


void PSound::SetFormat(unsigned channels,
                       unsigned samplesPerSecond,
                       unsigned bitsPerSample)
{
  encoding = 0;
  numChannels = channels;
  sampleRate = samplesPerSecond;
  sampleSize = bitsPerSample;
  formatInfo.SetSize(0);
}


BOOL PSound::Load(const PFilePath & filename)
{
  // Open wave file
  PMultiMediaFile mmio;
  PWaveFormat waveFormat;
  DWORD dataSize;
  if (!mmio.OpenWaveFile(filename, waveFormat, dataSize)) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  encoding = waveFormat->wFormatTag;
  numChannels = waveFormat->nChannels;
  sampleRate = waveFormat->nSamplesPerSec;
  sampleSize = waveFormat->wBitsPerSample;

  if (encoding != 0) {
    PINDEX formatSize = waveFormat->cbSize + sizeof(WAVEFORMATEX);
    memcpy(formatInfo.GetPointer(formatSize), waveFormat, formatSize);
  }

  // Allocate and lock memory for the waveform data.
  if (!SetSize(dataSize)) {
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
  PWaveFormat waveFormat;
  if (encoding == 0)
    waveFormat.SetFormat(numChannels, sampleRate, sampleSize);
  else {
    waveFormat.SetSize(GetFormatInfoSize());
    memcpy(waveFormat.GetPointer(), GetFormatInfoData(), GetFormatInfoSize());
  }

  // Open wave file
  PMultiMediaFile mmio;
  if (!mmio.CreateWaveFile(filename, waveFormat, GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  if (!mmio.Write(GetPointer(), GetSize())) {
    dwLastError = mmio.GetLastError();
    return FALSE;
  }

  return TRUE;
}


BOOL PSound::Play()
{
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
  if (!channel.IsOpen())
    return FALSE;

  return channel.PlaySound(*this, TRUE);
}


BOOL PSound::PlayFile(const PFilePath & file, BOOL wait)
{
  return ::PlaySound(file, NULL, SND_FILENAME|(wait ? SND_SYNC : SND_ASYNC));
}


///////////////////////////////////////////////////////////////////////////////

PWaveBuffer::PWaveBuffer(PINDEX sz)
 : PBYTEArray(sz)
{
  hWaveOut = NULL;
  hWaveIn = NULL;
  header.dwFlags = WHDR_DONE;
}


PWaveBuffer::~PWaveBuffer()
{
  Release();
}


PWaveBuffer & PWaveBuffer::operator=(const PSound & sound)
{
  PBYTEArray::operator=(sound);
  return *this;
}


void PWaveBuffer::PrepareCommon(PINDEX count)
{
  Release();

  memset(&header, 0, sizeof(header));
  header.lpData = (char *)GetPointer();
  header.dwBufferLength = count;
  header.dwUser = (DWORD)this;
}


DWORD PWaveBuffer::Prepare(HWAVEOUT hOut, PINDEX & count)
{
  // Set up WAVEHDR structure and prepare it to be written to wave device
  if (count > GetSize())
    count = GetSize();

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

  // There seems to be some pathalogical cases where on an Abort() call the buffers
  // still are "in use", even though waveOutReset() was called. So wait until the
  // sound driver has finished with the buffer before releasing it.

  if (hWaveOut != NULL) {
    if ((err = waveOutUnprepareHeader(hWaveOut, &header, sizeof(header))) == WAVERR_STILLPLAYING)
      return err;
    hWaveOut = NULL;
  }

  if (hWaveIn != NULL) {
    if ((err = waveInUnprepareHeader(hWaveIn, &header, sizeof(header))) == WAVERR_STILLPLAYING)
      return err;
    hWaveIn = NULL;
  }

  header.dwFlags |= WHDR_DONE;
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
  hEventDone = CreateEvent(NULL, FALSE, FALSE, NULL);

  waveFormat.SetFormat(1, 8000, 16);

  bufferByteOffset = P_MAX_INDEX;

  SetBuffers(32768, 2);
}


PSoundChannel::~PSoundChannel()
{
  Close();

  if (hEventDone != NULL)
    CloseHandle(hEventDone);
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


PString PSoundChannel::GetDefaultDevice(Directions dir)
{
  RegistryKey registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Multimedia\\Sound Mapper",
                       RegistryKey::ReadOnly);

  PString str;

  if (dir == Player) {
    if (!registry.QueryValue("Playback", str)) {
      WAVEOUTCAPS caps;
      if (waveOutGetDevCaps(0, &caps, sizeof(caps)) == 0)
        str = caps.szPname;
    }
  }
  else {
    if (!registry.QueryValue("Record", str)) {
      WAVEINCAPS caps;
      if (waveInGetDevCaps(0, &caps, sizeof(caps)) == 0)
        str = caps.szPname;
    }
  }

  return str;
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
  Close();

  PWaitAndSignal mutex(bufferMutex);

  bufferByteOffset = P_MAX_INDEX;
  bufferIndex = 0;

  WAVEFORMATEX* format = (WAVEFORMATEX*) waveFormat;
  
  switch (direction) {
    case Player :
      osError = waveOutOpen(&hWaveOut, id, format,
                            (DWORD) hEventDone, 0, CALLBACK_EVENT);
      break;

    case Recorder :
      osError = waveInOpen(&hWaveIn, id, format,
                           (DWORD) hEventDone, 0, CALLBACK_EVENT);
      break;
  }

  if (osError != MMSYSERR_NOERROR)
    return FALSE;

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


BOOL PSoundChannel::SetFormat(const PWaveFormat & format)
{
  Abort();

  waveFormat = format;

  return OpenDevice(os_handle);
}


unsigned PSoundChannel::GetChannels() const
{
  return waveFormat->nChannels;
}


unsigned PSoundChannel::GetSampleRate() const
{
  return waveFormat->nSamplesPerSec;
}


unsigned PSoundChannel::GetSampleSize() const
{
  return waveFormat->wBitsPerSample;
}


BOOL PSoundChannel::Close()
{
  if (!IsOpen())
    return FALSE;

  Abort();

  if (hWaveOut != NULL) {
    while ((osError = waveOutClose(hWaveOut)) == WAVERR_STILLPLAYING)
      waveOutReset(hWaveOut);
    hWaveOut = NULL;
  }

  if (hWaveIn != NULL) {
    while ((osError = waveInClose(hWaveIn)) == WAVERR_STILLPLAYING)
      waveInReset(hWaveIn);
    hWaveIn = NULL;
  }

  Abort();

  os_handle = -1;
  return TRUE;
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0, PInvalidParameter);

  BOOL ok = TRUE;

  PWaitAndSignal mutex(bufferMutex);

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

  bufferByteOffset = P_MAX_INDEX;
  bufferIndex = 0;

  return ok;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  PWaitAndSignal mutex(bufferMutex);

  count = buffers.GetSize();

  if (count == 0)
    size = 0;
  else
    size = buffers[0].GetSize();

  return TRUE;
}


BOOL PSoundChannel::Write(const void * data, PINDEX size)
{
  lastWriteCount = 0;

  if (hWaveOut == NULL) {
    osError = MMSYSERR_NOTSUPPORTED;
    return FALSE;
  }

  const BYTE * ptr = (const BYTE *)data;

  bufferMutex.Wait();

  while (size > 0) {
    PWaveBuffer & buffer = buffers[bufferIndex];
    while ((buffer.header.dwFlags&WHDR_DONE) == 0) {
      bufferMutex.Signal();
      // No free buffers, so wait for one
      if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0) {
        osError = MMSYSERR_ERROR;
        return FALSE; // No free buffers
      }
      bufferMutex.Wait();
    }

    // Can't write more than a buffer full
    PINDEX count = size;
    if ((osError = buffer.Prepare(hWaveOut, count)) != MMSYSERR_NOERROR)
      break;

    memcpy(buffer.GetPointer(), ptr, count);

    if ((osError = waveOutWrite(hWaveOut, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      break;

    bufferIndex = (bufferIndex+1)%buffers.GetSize();
    lastWriteCount += count;
    size -= count;
    ptr += count;
  }

  bufferMutex.Signal();

  return size == 0;
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
    bufferMutex.Wait();

    // To avoid lots of copying of sound data, we fake the PSound buffer into
    // the internal buffers and play directly from the PSound object.
    buffers.SetSize(1);
    PWaveBuffer & buffer = buffers[0];
    buffer = sound;

    PINDEX count = sound.GetSize();
    if ((osError = buffer.Prepare(hWaveOut, count)) == MMSYSERR_NOERROR &&
        (osError = waveOutWrite(hWaveOut, &buffer.header, sizeof(WAVEHDR))) == MMSYSERR_NOERROR) {
      if (wait)
        ok = WaitForPlayCompletion();
    }
    else
      ok = FALSE;

    bufferMutex.Signal();
  }

  SetFormat(numChannels, sampleRate, bitsPerSample);
  SetBuffers(bufferSize, bufferCount);
  return ok;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  Abort();

  PMultiMediaFile mmio;
  PWaveFormat fileFormat;
  DWORD dataSize;
  if (!mmio.OpenWaveFile(filename, fileFormat, dataSize)) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  // Save old format and set to one loaded from file.
  unsigned numChannels = waveFormat->nChannels;
  unsigned sampleRate = waveFormat->nSamplesPerSec;
  unsigned bitsPerSample = waveFormat->wBitsPerSample;
  waveFormat = fileFormat;
  if (!OpenDevice(os_handle)) {
    SetFormat(numChannels, sampleRate, bitsPerSample);
    return FALSE;
  }

  bufferMutex.Wait();

  while (dataSize > 0) {
    PWaveBuffer & buffer = buffers[bufferIndex];
    while ((buffer.header.dwFlags&WHDR_DONE) == 0) {
      bufferMutex.Signal();
      // No free buffers, so wait for one
      if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0) {
        SetFormat(numChannels, sampleRate, bitsPerSample);
        return FALSE;
      }
      bufferMutex.Wait();
    }

    // Can't write more than a buffer full
    PINDEX count = dataSize;
    if ((osError = buffer.Prepare(hWaveOut, count)) != MMSYSERR_NOERROR)
      break;

    // Read the waveform data subchunk
    if (!mmio.Read(buffer.GetPointer(), count)) {
      osError = mmio.GetLastError();
      break;
    }

    if ((osError = waveOutWrite(hWaveOut, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      break;

    bufferIndex = (bufferIndex+1)%buffers.GetSize();
    dataSize -= count;
  }

  bufferMutex.Signal();

  if (dataSize == 0 && wait)
    WaitForPlayCompletion();

  SetFormat(numChannels, sampleRate, bitsPerSample);
  return TRUE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  PWaitAndSignal mutex(bufferMutex);

  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    if ((buffers[i].header.dwFlags&WHDR_DONE) == 0)
      return FALSE;
  }

  return TRUE;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  while (!HasPlayCompleted()) {
    if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0)
      return FALSE;
  }

  return TRUE;
}


BOOL PSoundChannel::StartRecording()
{
  PWaitAndSignal mutex(bufferMutex);

  // See if has started already.
  if (bufferByteOffset != P_MAX_INDEX)
    return TRUE;

  // Start the first read, queue all the buffers
  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    PWaveBuffer & buffer = buffers[i];
    if ((osError = buffer.Prepare(hWaveIn)) != MMSYSERR_NOERROR)
      return FALSE;
    if ((osError = waveInAddBuffer(hWaveIn, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      return FALSE;
  }

  bufferByteOffset = 0;

  if ((osError = waveInStart(hWaveIn)) == MMSYSERR_NOERROR) // start recording
    return TRUE;

  bufferByteOffset = P_MAX_INDEX;
  return FALSE;
}


BOOL PSoundChannel::Read(void * data, PINDEX size)
{
  lastReadCount = 0;

  if (hWaveIn == NULL) {
    osError = MMSYSERR_NOTSUPPORTED;
    return FALSE;
  }

  if (!StartRecording())  // Start the first read, queue all the buffers
    return FALSE;

  if (!WaitForRecordBufferFull())
    return FALSE;

  PWaitAndSignal mutex(bufferMutex);

  PWaveBuffer & buffer = buffers[bufferIndex];

  PINDEX bytesRecorded = buffer.header.dwBytesRecorded;
  lastReadCount = bytesRecorded - bufferByteOffset;
  if (lastReadCount > size)
    lastReadCount = size;

  if (lastReadCount == 0 || bufferByteOffset == P_MAX_INDEX)
    return FALSE;

  memcpy(data, &buffer[bufferByteOffset], lastReadCount);

  bufferByteOffset += lastReadCount;
  if (bufferByteOffset >= buffer.GetSize()) {
    if ((osError = buffer.Prepare(hWaveIn)) != MMSYSERR_NOERROR)
      return FALSE;
    if ((osError = waveInAddBuffer(hWaveIn, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      return FALSE;

    bufferIndex = (bufferIndex+1)%buffers.GetSize();
    bufferByteOffset = 0;
  }

  return TRUE;
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
  if (!StartRecording())  // Start the first read, queue all the buffers
    return FALSE;

  if (!WaitForAllRecordBuffersFull())
    return FALSE;

  sound.SetFormat(waveFormat->nChannels,
                  waveFormat->nSamplesPerSec,
                  waveFormat->wBitsPerSample);

  PWaitAndSignal mutex(bufferMutex);

  if (buffers.GetSize() == 1 &&
          (PINDEX)buffers[0].header.dwBytesRecorded == buffers[0].GetSize())
    sound = buffers[0];
  else {
    PINDEX totalSize = 0;
    PINDEX i;
    for (i = 0; i < buffers.GetSize(); i++)
      totalSize += buffers[i].header.dwBytesRecorded;

    if (!sound.SetSize(totalSize)) {
      osError = MMSYSERR_NOMEM;
      return FALSE;
    }

    BYTE * ptr = sound.GetPointer();
    for (i = 0; i < buffers.GetSize(); i++) {
      PINDEX sz = buffers[i].header.dwBytesRecorded;
      memcpy(ptr, buffers[i], sz);
      ptr += sz;
    }
  }

  return TRUE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  if (!StartRecording())  // Start the first read, queue all the buffers
    return FALSE;

  if (!WaitForAllRecordBuffersFull())
    return FALSE;

  PWaitAndSignal mutex(bufferMutex);

  PINDEX dataSize = 0;
  PINDEX i;
  for (i = 0; i < buffers.GetSize(); i++)
    dataSize += buffers[i].header.dwBytesRecorded;

  PMultiMediaFile mmio;
  if (!mmio.CreateWaveFile(filename, waveFormat, dataSize)) {
    osError = mmio.GetLastError();
    return FALSE;
  }

  for (i = 0; i < buffers.GetSize(); i++) {
    if (!mmio.Write(buffers[i], buffers[i].header.dwBytesRecorded)) {
      osError = mmio.GetLastError();
      return FALSE;
    }
  }

  return TRUE;
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  PWaitAndSignal mutex(bufferMutex);

  if (bufferByteOffset == P_MAX_INDEX)
    return TRUE;

  return (buffers[bufferIndex].header.dwFlags&WHDR_DONE) != 0;
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  PWaitAndSignal mutex(bufferMutex);

  if (bufferByteOffset == P_MAX_INDEX)
    return TRUE;

  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    if ((buffers[i].header.dwFlags&WHDR_DONE) == 0)
      return FALSE;
  }

  return TRUE;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (bufferByteOffset == P_MAX_INDEX)
    return FALSE;

  while (!IsRecordBufferFull()) {
    if (WaitForSingleObject(hEventDone, INFINITE) != WAIT_OBJECT_0)
      return FALSE;
  }

  return TRUE;
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  if (bufferByteOffset == P_MAX_INDEX)
    return FALSE;

  while (!AreAllRecordBuffersFull()) {
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

  PWaitAndSignal mutex(bufferMutex);

  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    if (buffers[i].Release() == WAVERR_STILLPLAYING) {
      if (hWaveOut != NULL)
        waveOutReset(hWaveOut);
      if (hWaveIn != NULL)
        waveInReset(hWaveIn);
    }
  }

  bufferByteOffset = P_MAX_INDEX;
  bufferIndex = 0;

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


// End of File ///////////////////////////////////////////////////////////////

