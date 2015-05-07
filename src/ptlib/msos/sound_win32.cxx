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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>


#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/sound.h>
#include <ptlib/msos/ptlib/sound_win32.h>

#include <math.h>


#ifndef _WIN32_WCE
#ifdef _MSC_VER
#pragma comment(lib, "winmm.lib")
#endif
#else
#include <ptlib/wm/mmsystemx.h>
#endif


class PSound;

PCREATE_SOUND_PLUGIN(WindowsMultimedia, PSoundChannelWin32);

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


PBoolean PMultiMediaFile::CreateWaveFile(const PFilePath & filename,
                                     const PWaveFormat & waveFormat,
                                     DWORD dataSize)
{
  if (!Open(filename, MMIO_CREATE|MMIO_WRITE))
    return false;

  MMCKINFO mmChunk;
  mmChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  mmChunk.cksize = 4 + // Form type
                   4 + sizeof(DWORD) + waveFormat.GetSize() + // fmt chunk
                   4 + sizeof(DWORD) + dataSize;              // data chunk

  // Create a RIFF chunk
  if (!CreateChunk(mmChunk, MMIO_CREATERIFF))
    return false;

  // Save the format sub-chunk
  mmChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  mmChunk.cksize = waveFormat.GetSize();
  if (!CreateChunk(mmChunk))
    return false;

  if (!Write(waveFormat, waveFormat.GetSize()))
    return false;

  // Save the data sub-chunk
  mmChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  mmChunk.cksize = dataSize;
  return CreateChunk(mmChunk);
}


PBoolean PMultiMediaFile::OpenWaveFile(const PFilePath & filename,
                                   PWaveFormat  & waveFormat,
                                   DWORD & dataSize)
{
  // Open wave file
  if (!Open(filename, MMIO_READ | MMIO_ALLOCBUF))
    return false;

  MMCKINFO mmParentChunk, mmSubChunk;
  dwLastError = MMSYSERR_NOERROR;

  // Locate a 'RIFF' chunk with a 'WAVE' form type
  mmParentChunk.fccType = mmioFOURCC('W', 'A', 'V', 'E');
  if (!Descend(MMIO_FINDRIFF, mmParentChunk))
    return false;

  // Find the format chunk
  mmSubChunk.ckid = mmioFOURCC('f', 'm', 't', ' ');
  if (!Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk))
    return false;

  // Get the size of the format chunk, allocate memory for it
  if (!waveFormat.SetSize(mmSubChunk.cksize))
    return false;

  // Read the format chunk
  if (!Read(waveFormat.GetPointer(), waveFormat.GetSize()))
    return false;

  // Ascend out of the format subchunk
  Ascend(mmSubChunk);

  // Find the data subchunk
  mmSubChunk.ckid = mmioFOURCC('d', 'a', 't', 'a');
  if (!Descend(MMIO_FINDCHUNK, mmSubChunk, &mmParentChunk))
    return false;

  // Get the size of the data subchunk
  if (mmSubChunk.cksize == 0) {
    dwLastError = MMSYSERR_INVALPARAM;
    return false;
  }

  dataSize = mmSubChunk.cksize;
  return true;
}


PBoolean PMultiMediaFile::Open(const PFilePath & filename,
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


PBoolean PMultiMediaFile::Close(UINT wFlags)
{
  if (hmmio == NULL)
    return false;

  mmioClose(hmmio, wFlags);
  hmmio = NULL;
  return true;
}


PBoolean PMultiMediaFile::Ascend(MMCKINFO & ckinfo, UINT wFlags)
{
  dwLastError = mmioAscend(hmmio, &ckinfo, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


PBoolean PMultiMediaFile::Descend(UINT wFlags, MMCKINFO & ckinfo, LPMMCKINFO lpckParent)
{
  dwLastError = mmioDescend(hmmio, &ckinfo, lpckParent, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


PBoolean PMultiMediaFile::Read(void * data, PINDEX len)
{
  return mmioRead(hmmio, (char *)data, len) == (int)len;
}


PBoolean PMultiMediaFile::CreateChunk(MMCKINFO & ckinfo, UINT wFlags)
{
  dwLastError = mmioCreateChunk(hmmio, &ckinfo, wFlags);
  return dwLastError == MMSYSERR_NOERROR;
}


PBoolean PMultiMediaFile::Write(const void * data, PINDEX len)
{
  return mmioWrite(hmmio, (char *)data, len) == (int)len;
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
  waveFormat->nBlockAlign = (WORD)(numChannels*bitsPerSample/8);
  waveFormat->nAvgBytesPerSec = waveFormat->nSamplesPerSec*waveFormat->nBlockAlign;
  waveFormat->cbSize = 0;
}


void PWaveFormat::SetFormat(const void * data, PINDEX size)
{
  SetSize(size);
  memcpy(waveFormat, data, size);
}


PBoolean PWaveFormat::SetSize(PINDEX sz)
{
  if (waveFormat != NULL)
    free(waveFormat);

  size = sz;
  if (sz == 0)
    waveFormat = NULL;
  else {
    if (sz < (PINDEX)sizeof(WAVEFORMATEX))
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


PBoolean PSound::Load(const PFilePath & filename)
{
  // Open wave file
  PMultiMediaFile mmio;
  PWaveFormat waveFormat;
  DWORD dataSize;
  if (!mmio.OpenWaveFile(filename, waveFormat, dataSize)) {
    dwLastError = mmio.GetLastError();
    return false;
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
    return false;
  }

  // Read the waveform data subchunk
  if (!mmio.Read(GetPointer(), GetSize())) {
    dwLastError = mmio.GetLastError();
    return false;
  }

  return true;
}


PBoolean PSound::Save(const PFilePath & filename)
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
    return false;
  }

  if (!mmio.Write(GetPointer(), GetSize())) {
    dwLastError = mmio.GetLastError();
    return false;
  }

  return true;
}


PBoolean PSound::Play()
{
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
  if (!channel.IsOpen())
    return false;

  return channel.PlaySound(*this, true);
}

PBoolean PSound::Play(const PString & device)
{

  PSoundChannel channel(device,
                       PSoundChannel::Player);
  if (!channel.IsOpen())
    return false;

  return channel.PlaySound(*this, true);
}

PBoolean PSound::PlayFile(const PFilePath & file, PBoolean wait)
{
  PVarString filename = file;
  return ::PlaySound(filename, NULL, SND_FILENAME|(wait ? SND_SYNC : SND_ASYNC));
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
  //header.dwUser = (DWORD)this;  // NOT USED
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

PSoundChannelWin32::PSoundChannelWin32()
  : hWaveIn(NULL)
  , hWaveOut(NULL)
  , hMixer(NULL)
  , hEventDone(CreateEvent(NULL, false, false, NULL))
  , opened(false)
  , m_reopened(false)
  , bufferIndex(0)
  , bufferByteOffset(P_MAX_INDEX)
{
  waveFormat.SetFormat(1, 8000, 16);
  SetBuffers(32768, 2);
}


PSoundChannelWin32::~PSoundChannelWin32()
{
  Close();

  if (hEventDone != NULL)
    CloseHandle(hEventDone);
}


const char * PSoundChannelWin32::GetDriverName()
{
  return PPlugin_PSoundChannel_WindowsMultimedia::ServiceName();
}


PString PSoundChannelWin32::GetName() const
{
  return PConstString("WindowsMultimedia") + PPluginServiceDescriptor::SeparatorChar + deviceName;
}


static bool GetWaveOutDeviceName(UINT id, PString & name)
{
  if (id == WAVE_MAPPER) {
    name = "Default";
    return true;
  }

  WAVEOUTCAPS caps;
  if (waveOutGetDevCaps(id, &caps, sizeof(caps)) != 0)
    return false;

  name = PString(caps.szPname).Trim();
  return true;
}


static bool GetWaveInDeviceName(UINT id, PString & name)
{
  if (id == WAVE_MAPPER) {
    name = "Default";
    return true;
  }

  WAVEINCAPS caps;
  if (waveInGetDevCaps(id, &caps, sizeof(caps)) != 0)
    return false;

  name = PString(caps.szPname).Trim();
  return true;
}


PString PSoundChannelWin32::GetDefaultDevice(Directions)
{
  return "Default";
}


PStringArray PSoundChannelWin32::GetDeviceNames(Directions dir)
{
  PStringArray devices;

  UINT numDevs;
  UINT id = WAVE_MAPPER;

  switch (dir) {
    case Player :
      numDevs = waveOutGetNumDevs();
      do {
        PCaselessString dev;
        if (GetWaveOutDeviceName(id, dev))
          devices.AppendString(dev);
      } while (++id < numDevs);
      break;

    case Recorder :
      numDevs = waveInGetNumDevs();
      do {
        PCaselessString dev;
        if (GetWaveInDeviceName(id, dev))
          devices.AppendString(dev);
      } while (++id < numDevs);
      break;
    case Closed:
      break;
  }

  return devices;
}


PBoolean PSoundChannelWin32::GetDeviceID(const PString & device, Directions dir, unsigned& id)
{
  PINDEX offset = device.Find(PPluginServiceDescriptor::SeparatorChar);
  if (offset == P_MAX_INDEX)
    offset = 0;
  else
    offset++;

  if (device[offset] == '#') {
    id = device.Mid(offset+1).AsUnsigned();
    switch (dir) {
      case Player :
        if (id < waveOutGetNumDevs())
          GetWaveOutDeviceName(id, deviceName);
        break;

      case Recorder :
        if (id < waveInGetNumDevs())
          GetWaveInDeviceName(id, deviceName);
        break;
      case Closed :
        break;
    }
  }
  else {
    id = WAVE_MAPPER;
    UINT numDevs;
    switch (dir) {
      case Player :
        numDevs = waveOutGetNumDevs();
        do {
          PCaselessString str;
          if (GetWaveOutDeviceName(id, str) && str == device.Mid(offset)) {
            deviceName = str;
            break;
          }
        } while (++id < numDevs);
        break;

      case Recorder :
        numDevs = waveInGetNumDevs();
        do {
          PCaselessString str;
          if (GetWaveInDeviceName(id, str) && str == device.Mid(offset)) {
            deviceName = str;
            break;
          }
        } while (++id < numDevs);
        break;

      case Closed :
        break;
    }
  }

  if (deviceName.IsEmpty())
    return SetErrorValues(NotFound, MMSYSERR_BADDEVICEID|PWIN32ErrorFlag);

  return true;
}


bool PSoundChannelWin32::Open(const Params & params)
{
  Close();
  unsigned id = 0;

  if( !GetDeviceID(params.m_device, params.m_direction, id) )
    return false;

  waveFormat.SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);
  SetBuffers(params.m_bufferSize, params.m_bufferCount);

  activeDirection = params.m_direction;
  return OpenDevice(id);
}

PBoolean PSoundChannelWin32::Open(const PString & device,
                                  Directions dir,
                                  const PWaveFormat& format)
{
  Close();
  unsigned id = 0;

  if( !GetDeviceID(device, dir, id) )
    return false;

  waveFormat = format;

  activeDirection = dir;
  return OpenDevice(id);
}


PBoolean PSoundChannelWin32::OpenDevice(P_INT_PTR id)
{
  Close();

  PWaitAndSignal mutex(bufferMutex);

  bufferByteOffset = P_MAX_INDEX;
  bufferIndex = 0;

  WAVEFORMATEX* format = (WAVEFORMATEX*) waveFormat;

  UINT mixerId = UINT_MAX;
  MIXERLINE line;

  MMRESULT osError = MMSYSERR_BADDEVICEID;
  switch (activeDirection) {
    case Player :
      PTRACE(4, "WinSnd\twaveOutOpen, id=" << id);
      osError = waveOutOpen(&hWaveOut, (UINT)id, format, (DWORD_PTR)hEventDone, 0, CALLBACK_EVENT);
      if (osError == MMSYSERR_NOERROR) {
        if (mixerGetID((HMIXEROBJ)hWaveOut, &mixerId, MIXER_OBJECTF_HWAVEOUT) == MMSYSERR_NOERROR)
          line.dwComponentType = MIXERLINE_COMPONENTTYPE_SRC_WAVEOUT;
      }
      break;

    case Recorder :
      PTRACE(4, "WinSnd\twaveInOpen, id=" << id);
      osError = waveInOpen(&hWaveIn, (UINT)id, format, (DWORD_PTR)hEventDone, 0, CALLBACK_EVENT);
      if (osError == MMSYSERR_NOERROR) {
        if (mixerGetID((HMIXEROBJ)hWaveIn, &mixerId, MIXER_OBJECTF_HWAVEIN) == MMSYSERR_NOERROR)
          line.dwComponentType = MIXERLINE_COMPONENTTYPE_DST_WAVEIN;
      }
      break;

    case Closed:
      break;
  }

  if (osError != MMSYSERR_NOERROR)
    return SetErrorValues(NotFound, osError|PWIN32ErrorFlag);

  opened = true;
  os_handle = id;

  if (mixerId == UINT_MAX) {
    PTRACE(2, "WinSnd\tNo mixer available");
    return true; // Still return true as have actual device
  }

  if ((osError = mixerOpen(&hMixer, mixerId, 0, 0, MIXER_OBJECTF_MIXER)) != MMSYSERR_NOERROR) {
    PTRACE(2, "WinSnd\tFailed to open mixer, error=" << osError);
    return true; // Still return true as have actual device
  }

  line.cbStruct = sizeof(line);
  if ((osError = mixerGetLineInfo((HMIXEROBJ)hMixer, &line,
            MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_COMPONENTTYPE)) != MMSYSERR_NOERROR) {
    PTRACE(2, "WinSnd\tFailed to get mixer info, error=" << osError);
  }
  else {
    bool haveControl = true;
    MIXERLINECONTROLS controls;

    if ((activeDirection == Recorder) && !PProcess::IsOSVersion(6,0,0)) { //5=XP/win2003
      /* There is no "master" for the recording side, so need to find the
         single selected input or at least individual microphone input
         No need to do all of these on Vista/Win7/Win2008 as there is
         a "master" for every input on those OS */

      DWORD iConn = line.cConnections; //should save first
      
      //1)Attempt to find currently selected input
      MIXERCONTROL muxControl;
      muxControl.cbStruct = sizeof(muxControl);
      controls.cbStruct = sizeof(controls);
      controls.dwLineID = line.dwLineID;
      controls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUX;
      controls.cControls = 1;
      controls.pamxctrl = &muxControl;
      controls.cbmxctrl = muxControl.cbStruct;
      if ((osError = mixerGetLineControls((HMIXEROBJ)hMixer, &controls,
                                          MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)) == MMSYSERR_NOERROR) {
        MIXERCONTROLDETAILS mxcd;
        mxcd.cbStruct = sizeof(mxcd);
        mxcd.dwControlID = muxControl.dwControlID;
        mxcd.cChannels = 1;
        mxcd.cMultipleItems = muxControl.cMultipleItems;
        mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_LISTTEXT);
        std::vector<MIXERCONTROLDETAILS_LISTTEXT> mxcdlt(mxcd.cChannels * mxcd.cMultipleItems); 
        mxcd.paDetails = &mxcdlt[0];
        // Get the control list text items (input lines id's)
        if ((osError = mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd,
                                              MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_LISTTEXT)) == MMSYSERR_NOERROR) {
          mxcd.cbDetails = sizeof(MIXERCONTROLDETAILS_BOOLEAN);
          std::vector<MIXERCONTROLDETAILS_BOOLEAN> mxcdb(mxcd.cChannels * mxcd.cMultipleItems);
          mxcd.paDetails = &mxcdb[0];
          // Get the control values for line states (selected/not selected)
          if ((osError = mixerGetControlDetails((HMIXEROBJ)hMixer, &mxcd,
                                                MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE)) == MMSYSERR_NOERROR) {
            DWORD iMult = mxcd.cMultipleItems;
            while (iMult > 0) {
              line.dwLineID = mxcdlt[--iMult].dwParam1;
              if ((osError = mixerGetLineInfo((HMIXEROBJ)hMixer, &line,
                                              MIXER_OBJECTF_HMIXER | MIXER_GETLINEINFOF_LINEID)) != MMSYSERR_NOERROR)
                PTRACE(2, "WinSnd\tRecorder: Failed to get mixer info, error=" << osError);
              else {
                if (mxcdb[iMult].fValue == 1) {
                  PTRACE(2, "WinSnd\tRecorder: Found selected input device: " << line.szName);
                  break;
                }

                if (iMult == 0) {
                  haveControl = false;
                  PTRACE(2, "WinSnd\tRecorder: Failed to find selected input device");
                }
              }
            }
          }
          else {
            haveControl = false;
            PTRACE(2, "WinSnd\tRecorder: failed to get info about states of input lines, error=" << osError);
          }
        }
        else {
          haveControl = false;
          PTRACE(2, "WinSnd\tRecorder: failed to get info about input lines, error=" << osError);
        }
      }
      else {
        haveControl = false;
        PTRACE(2, "WinSnd\tRecorder: failed to get mixer line mux control, error=" << osError);
      }

      //2) Attempt to find microphone input if not seccessfull with selected input
      if (!haveControl) {
        haveControl = true;
        PTRACE(2, "WinSnd\tRecorder: trying to find microphone now...");
        while (iConn > 0) {
          line.dwSource = --iConn;
          if ((osError = mixerGetLineInfo((HMIXEROBJ)hMixer, &line, MIXER_GETLINEINFOF_SOURCE)) != MMSYSERR_NOERROR)
            PTRACE(2, "WinSnd\tFailed to get mixer info, error=" << osError);
          else if (line.dwComponentType == MIXERLINE_COMPONENTTYPE_SRC_MICROPHONE) {
            PTRACE(5, "WinSnd\tFfound microphone, source=" << iConn);
            break;
          }

          if (iConn == 0) {
            PTRACE(2, "WinSnd\tFailed to find microphone info");
            haveControl = false;
          }
        }
      }
    }

    if (haveControl) {
      volumeControl.cbStruct = sizeof(volumeControl);

      controls.cbStruct = sizeof(controls);
      controls.dwLineID = line.dwLineID;
      controls.dwControlType = MIXERCONTROL_CONTROLTYPE_VOLUME;
      controls.cControls = 1;
      controls.pamxctrl = &volumeControl;
      controls.cbmxctrl = volumeControl.cbStruct;

      if ((osError = mixerGetLineControls((HMIXEROBJ)hMixer, &controls,
                MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)) == MMSYSERR_NOERROR) {
        muteControl.cbStruct = sizeof(muteControl);
        controls.dwControlType = MIXERCONTROL_CONTROLTYPE_MUTE;
        controls.pamxctrl = &muteControl;
        controls.cbmxctrl = muteControl.cbStruct;
        if ((osError = mixerGetLineControls((HMIXEROBJ)hMixer, &controls,
                MIXER_OBJECTF_HMIXER | MIXER_GETLINECONTROLSF_ONEBYTYPE)) != MMSYSERR_NOERROR) {
          PTRACE(2, "WinSnd\tFailed to get mixer line mute control ("
                 << (activeDirection == Recorder ? "Recorder" : "Player") << "), error=" << osError);
        }
        return true;
      }
      PTRACE(2, "WinSnd\tFailed to get mixer line control, error=" << osError);
    }
  }

  mixerClose(hMixer);
  hMixer = NULL;
  return true; // Still return true as have actual device
}

PBoolean PSoundChannelWin32::IsOpen() const
{ 
  return opened ? true : false;
}

PBoolean PSoundChannelWin32::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  Abort();

  waveFormat.SetFormat(numChannels, sampleRate, bitsPerSample);

  return OpenDevice(os_handle);
}


PBoolean PSoundChannelWin32::SetFormat(const PWaveFormat & format)
{
  Abort();

  waveFormat = format;

  return OpenDevice(os_handle);
}


unsigned PSoundChannelWin32::GetChannels() const
{
  return waveFormat->nChannels;
}


unsigned PSoundChannelWin32::GetSampleRate() const
{
  return waveFormat->nSamplesPerSec;
}


unsigned PSoundChannelWin32::GetSampleSize() const
{
  return waveFormat->wBitsPerSample;
}


PBoolean PSoundChannelWin32::Close()
{
  if (CheckNotOpen())
    return false;

  PWaitAndSignal mutex(bufferMutex);

  Abort();

  if (hWaveOut != NULL) {
    while (waveOutClose(hWaveOut) == WAVERR_STILLPLAYING)
      waveOutReset(hWaveOut);
    hWaveOut = NULL;
  }

  if (hWaveIn != NULL) {
    while (waveInClose(hWaveIn) == WAVERR_STILLPLAYING)
      waveInReset(hWaveIn);
    hWaveIn = NULL;
  }

  if (hMixer != NULL) {
    mixerClose(hMixer);
    hMixer = NULL;
  }

  opened = false;
  os_handle = -1;
  return true;
}


PBoolean PSoundChannelWin32::SetBuffers(PINDEX size, PINDEX count)
{
  if (count == buffers.GetSize() && size == buffers[0].GetSize())
      return true;

  Abort();

  PAssert(size > 0 && count > 0, PInvalidParameter);

  PTRACE(3, "WinSnd\tSetting sounds buffers to " << count << " x " << size);

  PBoolean ok = true;

  PWaitAndSignal mutex(bufferMutex);

  if (!buffers.SetSize(count))
    ok = false;
  else {
    for (PINDEX i = 0; i < count; i++) {
      if (buffers.GetAt(i) == NULL)
        buffers.SetAt(i, new PWaveBuffer(size));
      if (!buffers[i].SetSize(size))
        ok = false;
    }
  }

  bufferByteOffset = P_MAX_INDEX;
  bufferIndex = 0;

  return ok;
}


PBoolean PSoundChannelWin32::GetBuffers(PINDEX & size, PINDEX & count)
{
  PWaitAndSignal mutex(bufferMutex);

  count = buffers.GetSize();

  if (count == 0)
    size = 0;
  else
    size = buffers[0].GetSize();

  return true;
}


int PSoundChannelWin32::WaitEvent(ErrorGroup group)
{
  switch (WaitForSingleObject(hEventDone, std::max((int)buffers[0].GetSize(), 1000))) {
    case WAIT_OBJECT_0 :
      PTRACE_IF(3, m_reopened, "WinSnd\tRecovered from stuck device");
      m_reopened = false;
      return 1;

    case WAIT_TIMEOUT :
      if (CheckNotOpen())
        return false;

      if (m_reopened) {
        PTRACE(2, "WinSnd\tCould not recover from stuck device");
        return SetErrorValues(Timeout, ETIMEDOUT, group);
      }

      PTRACE(3, "WinSnd\tTimeout, reopening stuck device");
      if (!OpenDevice(os_handle))
        return false;

      // Throw away signal that is executed in Close() (via OpenDevice())
      if (WaitForSingleObject(hEventDone, 0) == WAIT_OBJECT_0) {
        m_reopened = true;
        return -1;
      }

      PTRACE(2, "WinSnd\tUnexpected error waiting for event, code=" << ::GetLastError());
  }

  return SetErrorValues(Miscellaneous, ::GetLastError()|PWIN32ErrorFlag, group);
}


PBoolean PSoundChannelWin32::Write(const void * data, PINDEX size)
{
  lastWriteCount = 0;

  if (hWaveOut == NULL)
    return SetErrorValues(NotOpen, EBADF, LastWriteError);

  const BYTE * ptr = (const BYTE *)data;

  bufferMutex.Wait();

  DWORD osError = MMSYSERR_NOERROR;
  while (size > 0) {
    PWaveBuffer & buffer = buffers[bufferIndex];
    while ((buffer.header.dwFlags&WHDR_DONE) == 0) {
      bufferMutex.Signal();
      // No free buffers, so wait for one
      switch (WaitEvent(LastWriteError)) {
        case 0 :
          return false;
        case -1 :
          return Write(data, size);
        case 1 :
          break;
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

  if (size != 0)
    return SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag, LastWriteError);

  return true;
}


PBoolean PSoundChannelWin32::PlaySound(const PSound & sound, PBoolean wait)
{
  Abort();

  PBoolean ok = false;

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

    DWORD osError;
    PINDEX count = sound.GetSize();
    if ((osError = buffer.Prepare(hWaveOut, count)) == MMSYSERR_NOERROR &&
        (osError = waveOutWrite(hWaveOut, &buffer.header, sizeof(WAVEHDR))) == MMSYSERR_NOERROR) {
      if (wait)
        ok = WaitForPlayCompletion();
    }
    else {
      SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag, LastWriteError);
      ok = false;
    }

    bufferMutex.Signal();
  }

  SetFormat(numChannels, sampleRate, bitsPerSample);
  SetBuffers(bufferSize, bufferCount);
  return ok;
}


PBoolean PSoundChannelWin32::HasPlayCompleted()
{
  PWaitAndSignal mutex(bufferMutex);

  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    if ((buffers[i].header.dwFlags&WHDR_DONE) == 0)
      return false;
  }

  return true;
}


PBoolean PSoundChannelWin32::WaitForPlayCompletion()
{
  while (!HasPlayCompleted()) {
    if (!WaitEvent(LastWriteError))
      return false;
  }

  return true;
}


PBoolean PSoundChannelWin32::StartRecording()
{
  PWaitAndSignal mutex(bufferMutex);

  // See if has started already.
  if (bufferByteOffset != P_MAX_INDEX)
    return true;

  DWORD osError;

  // Start the first read, queue all the buffers
  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    PWaveBuffer & buffer = buffers[i];
    if ((osError = buffer.Prepare(hWaveIn)) != MMSYSERR_NOERROR)
      return false;
    if ((osError = waveInAddBuffer(hWaveIn, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      return false;
  }

  bufferByteOffset = 0;

  if ((osError = waveInStart(hWaveIn)) == MMSYSERR_NOERROR) // start recording
    return true;

  bufferByteOffset = P_MAX_INDEX;
  return SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag, LastReadError);
}


PBoolean PSoundChannelWin32::Read(void * data, PINDEX size)
{
  lastReadCount = 0;

  if (hWaveIn == NULL)
    return SetErrorValues(NotOpen, EBADF, LastReadError);

  if (!WaitForRecordBufferFull())
    return false;

  PWaitAndSignal mutex(bufferMutex);

  // Check to see if Abort() was called in another thread
  if (bufferByteOffset == P_MAX_INDEX)
    return false;

  PWaveBuffer & buffer = buffers[bufferIndex];

  lastReadCount = buffer.header.dwBytesRecorded - bufferByteOffset;
  if (lastReadCount > size)
    lastReadCount = size;

  memcpy(data, &buffer[bufferByteOffset], lastReadCount);

  bufferByteOffset += lastReadCount;
  if (bufferByteOffset >= (PINDEX)buffer.header.dwBytesRecorded) {
    DWORD osError;
    if ((osError = buffer.Prepare(hWaveIn)) != MMSYSERR_NOERROR)
      return SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag, LastReadError);
    if ((osError = waveInAddBuffer(hWaveIn, &buffer.header, sizeof(WAVEHDR))) != MMSYSERR_NOERROR)
      return SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag, LastReadError);

    bufferIndex = (bufferIndex+1)%buffers.GetSize();
    bufferByteOffset = 0;
  }

  return true;
}


PBoolean PSoundChannelWin32::RecordSound(PSound & sound)
{
  if (!WaitForAllRecordBuffersFull())
    return false;

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

    if (!sound.SetSize(totalSize))
      return SetErrorValues(NoMemory, ENOMEM, LastReadError);

    BYTE * ptr = sound.GetPointer();
    for (i = 0; i < buffers.GetSize(); i++) {
      PINDEX sz = buffers[i].header.dwBytesRecorded;
      memcpy(ptr, buffers[i], sz);
      ptr += sz;
    }
  }

  return true;
}


PBoolean PSoundChannelWin32::RecordFile(const PFilePath & filename)
{
  if (!WaitForAllRecordBuffersFull())
    return false;

  PWaitAndSignal mutex(bufferMutex);

  PINDEX dataSize = 0;
  PINDEX i;
  for (i = 0; i < buffers.GetSize(); i++)
    dataSize += buffers[i].header.dwBytesRecorded;

  PMultiMediaFile mmio;
  if (!mmio.CreateWaveFile(filename, waveFormat, dataSize))
    return SetErrorValues(Miscellaneous, mmio.GetLastError()|PWIN32ErrorFlag, LastReadError);

  for (i = 0; i < buffers.GetSize(); i++) {
    if (!mmio.Write(buffers[i], buffers[i].header.dwBytesRecorded))
      return SetErrorValues(Miscellaneous, mmio.GetLastError()|PWIN32ErrorFlag, LastReadError);
  }

  return true;
}


PBoolean PSoundChannelWin32::IsRecordBufferFull()
{
  PWaitAndSignal mutex(bufferMutex);

  return (buffers[bufferIndex].header.dwFlags&WHDR_DONE) != 0 &&
          buffers[bufferIndex].header.dwBytesRecorded > 0;
}


PBoolean PSoundChannelWin32::AreAllRecordBuffersFull()
{
  PWaitAndSignal mutex(bufferMutex);

  for (PINDEX i = 0; i < buffers.GetSize(); i++) {
    if ((buffers[i].header.dwFlags&WHDR_DONE) == 0 ||
         buffers[i].header.dwBytesRecorded    == 0)
      return false;
  }

  return true;
}


PBoolean PSoundChannelWin32::WaitForRecordBufferFull()
{
  if (!StartRecording())  // Start the first read, queue all the buffers
    return false;

  while (!IsRecordBufferFull()) {
    switch (WaitEvent(LastReadError)) {
      case 0 :
        return false;

      case -1 :
        if (StartRecording())  // Restart the read
          break;
        return false;

      case  1 :
        PWaitAndSignal mutex(bufferMutex);
        if (bufferByteOffset == P_MAX_INDEX)
          return false;
    }
  }

  return true;
}


PBoolean PSoundChannelWin32::WaitForAllRecordBuffersFull()
{
  if (!StartRecording())  // Start the first read, queue all the buffers
    return false;

  while (!AreAllRecordBuffersFull()) {
    switch (WaitEvent(LastReadError)) {
      case 0 :
        return false;

      case -1 :
        if (StartRecording())  // Restart the read
          break;
        return false;

      case  1 :
        PWaitAndSignal mutex(bufferMutex);
        if (bufferByteOffset == P_MAX_INDEX)
          return false;
    }
  }

  return true;
}


PBoolean PSoundChannelWin32::Abort()
{
  DWORD osError = MMSYSERR_NOERROR;

  {
    PWaitAndSignal mutex(bufferMutex);

    if (hWaveOut != NULL || hWaveIn != NULL) {
      for (PINDEX i = 0; i < buffers.GetSize(); i++) {
        do {
          if (hWaveOut != NULL)
            waveOutReset(hWaveOut);
          if (hWaveIn != NULL)
            waveInReset(hWaveIn);
        } while (buffers[i].Release() == WAVERR_STILLPLAYING);
      }
    }

    bufferByteOffset = P_MAX_INDEX;
    bufferIndex = 0;

    // Signal any threads waiting on this event, they should then check
    // the bufferByteOffset variable for an abort.
    SetEvent(hEventDone);
  }

  if (osError != MMSYSERR_NOERROR)
    return SetErrorValues(Miscellaneous, osError|PWIN32ErrorFlag);

  return true;
}


PString PSoundChannelWin32::GetErrorText(ErrorGroup group) const
{
  char str[256];

  if ((lastErrorNumber[group]&PWIN32ErrorFlag) == 0)
    return PChannel::GetErrorText(group);

  DWORD osError = lastErrorNumber[group]&~PWIN32ErrorFlag;
  if (activeDirection == Recorder) {
    if (waveInGetErrorText(osError, str, sizeof(str)) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(group);
  }
  else {
    if (waveOutGetErrorText(osError, str, sizeof(str)) != MMSYSERR_NOERROR)
      return PChannel::GetErrorText(group);
  }

  return str;
}


PBoolean PSoundChannelWin32::SetVolume(unsigned newVolume)
{
  if (CheckNotOpen())
    return false;
  if (hMixer == NULL)
    return SetErrorValues(Unavailable, EBADF);

  MIXERCONTROLDETAILS_UNSIGNED volume;
  if (newVolume >= MaxVolume)
    volume.dwValue = volumeControl.Bounds.dwMaximum;
  else
    volume.dwValue = volumeControl.Bounds.dwMinimum +
            (DWORD)((volumeControl.Bounds.dwMaximum - volumeControl.Bounds.dwMinimum)*newVolume/MaxVolume);
  PTRACE(5, "WinSnd\tVolume set to " << newVolume << " -> " << volume.dwValue);

  MIXERCONTROLDETAILS details;
  details.cbStruct = sizeof(details);
  details.dwControlID = volumeControl.dwControlID;
  details.cChannels = 1;
  details.cMultipleItems = 0;
  details.cbDetails = sizeof(volume);
  details.paDetails = &volume;

  MMRESULT result = mixerSetControlDetails((HMIXEROBJ)hMixer, &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
  if (result != MMSYSERR_NOERROR)
    return SetErrorValues(Miscellaneous, result|PWIN32ErrorFlag);

  return true;
}



PBoolean PSoundChannelWin32::GetVolume(unsigned & oldVolume)
{
  if (CheckNotOpen())
    return false;
  if (hMixer == NULL)
    return SetErrorValues(Unavailable, EBADF);

  MIXERCONTROLDETAILS_UNSIGNED volume;

  MIXERCONTROLDETAILS details;
  details.cbStruct = sizeof(details);
  details.dwControlID = volumeControl.dwControlID;
  details.cChannels = 1;
  details.cMultipleItems = 0;
  details.cbDetails = sizeof(volume);
  details.paDetails = &volume;

  MMRESULT result = mixerGetControlDetails((HMIXEROBJ)hMixer, &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
  if (result != MMSYSERR_NOERROR)
    return SetErrorValues(Miscellaneous, result|PWIN32ErrorFlag);

  oldVolume = MaxVolume*(volume.dwValue + 1 - volumeControl.Bounds.dwMinimum)/(volumeControl.Bounds.dwMaximum - volumeControl.Bounds.dwMinimum);
  return true;
}


bool PSoundChannelWin32::SetMute(bool newMute)
{
  if (CheckNotOpen())
    false;
  if (hMixer == NULL)
    return SetErrorValues(Unavailable, EBADF);

  MIXERCONTROLDETAILS_UNSIGNED mute;
  mute.dwValue = newMute;
  PTRACE(5, "WinSnd\tMute set to " << newMute << " -> " << mute.dwValue);

  MIXERCONTROLDETAILS details;
  details.cbStruct = sizeof(details);
  details.dwControlID = muteControl.dwControlID;
  details.cChannels = 1;
  details.cMultipleItems = 0;
  details.cbDetails = sizeof(mute);
  details.paDetails = &mute;

  MMRESULT result = mixerSetControlDetails((HMIXEROBJ)hMixer, &details, MIXER_OBJECTF_HMIXER | MIXER_SETCONTROLDETAILSF_VALUE);
  if (result != MMSYSERR_NOERROR)
    return SetErrorValues(Miscellaneous, result|PWIN32ErrorFlag);

  return true;
}


bool PSoundChannelWin32::GetMute(bool & oldMute)
{
  if (CheckNotOpen())
    return false;
  if (hMixer == NULL)
    return SetErrorValues(Unavailable, EBADF);

  MIXERCONTROLDETAILS_UNSIGNED mute;

  MIXERCONTROLDETAILS details;
  details.cbStruct = sizeof(details);
  details.dwControlID = muteControl.dwControlID;
  details.cChannels = 1;
  details.cMultipleItems = 0;
  details.cbDetails = sizeof(mute);
  details.paDetails = &mute;

  MMRESULT result = mixerGetControlDetails((HMIXEROBJ)hMixer, &details, MIXER_OBJECTF_HMIXER | MIXER_GETCONTROLDETAILSF_VALUE);
  if (result != MMSYSERR_NOERROR)
    return SetErrorValues(Miscellaneous, result|PWIN32ErrorFlag);

  oldMute = mute.dwValue;
  return true;
}


// End of File ///////////////////////////////////////////////////////////////

