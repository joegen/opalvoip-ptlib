/*
 * sound.cxx
 *
 * Sound driver implementation.
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
 * Contributor(s): Loopback feature: Philip Edelbrock <phil@netroedge.com>.
 *
 * $Log: oss.cxx,v $
 * Revision 1.32  2001/09/18 05:56:03  robertj
 * Fixed numerous problems with thread suspend/resume and signals handling.
 *
 * Revision 1.31  2001/09/14 05:10:57  robertj
 * Fixed compatibility issue with FreeBSD versionof OSS.
 *
 * Revision 1.30  2001/09/14 04:53:04  robertj
 * Improved the detection of sound cards, thanks Miguel Rodríguez Pérez for the ideas.
 *
 * Revision 1.29  2001/09/10 03:03:36  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.28  2001/09/03 09:15:40  robertj
 * Changed GetDeviceNames to try and find actual devices and real devices.
 *
 * Revision 1.27  2001/08/22 02:23:07  robertj
 * Fixed duplicate class name. All PWlib classes should start with P
 *
 * Revision 1.26  2001/08/21 12:33:25  rogerh
 * Make loopback mode actually work. Added the AudioDelay class from OpenMCU
 * and made Read() return silence when the buffer is empty.
 *
 * Revision 1.25  2001/05/14 06:33:19  rogerh
 * Add exit cases to loopback mode polling loops to allow the sound channel
 * to close properly when a connection closes.
 *
 * Revision 1.24  2001/02/07 03:34:29  craigs
 * Added ability get sound channel parameters
 *
 * Revision 1.23  2000/10/05 00:04:20  robertj
 * Fixed some warnings.
 *
 * Revision 1.22  2000/07/04 20:34:16  rogerh
 * Only use ioctl SNDCTL_DSP_SETDUPLEX is Linux. It is not defined in FreeBSD
 * In NetBSD and OpenBSD (using liboss), the ioctl returns EINVAL.
 *
 * Revision 1.21  2000/07/02 14:18:27  craigs
 * Fixed various problems with buffer handling
 *
 * Revision 1.20  2000/07/02 05:49:43  craigs
 * Really fixed race condition in OSS open
 *
 * Revision 1.19  2000/07/02 04:55:18  craigs
 * Fixed stupid mistake with fix for OSS race condition
 *
 * Revision 1.18  2000/07/02 04:50:44  craigs
 * Fixed potential race condition in OSS initialise
 *
 * Revision 1.17  2000/05/11 02:05:54  craigs
 * Fixed problem with PLayFile not recognizing wait flag
 *
 * Revision 1.16  2000/05/10 02:10:44  craigs
 * Added implementation for PlayFile command
 *
 * Revision 1.15  2000/05/02 08:30:26  craigs
 * Removed "memory leaks" caused by brain-dead GNU linker
 *
 * Revision 1.14  2000/04/09 18:19:23  rogerh
 * Add my changes for NetBSD support.
 *
 * Revision 1.13  2000/03/08 12:17:09  rogerh
 * Add OpenBSD support
 *
 * Revision 1.12  2000/03/04 13:02:28  robertj
 * Added simple play functions for sound files.
 *
 * Revision 1.11  2000/02/15 23:11:34  robertj
 * Audio support for FreeBSD, thanks Roger Hardiman.
 *
 * Revision 1.10  2000/01/08 06:41:08  craigs
 * Fixed problem whereby failure to open sound device returns TRUE
 *
 * Revision 1.9  1999/08/24 13:40:26  craigs
 * Fixed problem with EINTR causing sound channel reads and write to fail
 * Thanks to phil@netroedge.com!
 *
 * Revision 1.8  1999/08/17 09:42:22  robertj
 * Fixed close of sound channel in loopback mode closing stdin!
 *
 * Revision 1.7  1999/08/17 09:28:47  robertj
 * Added audio loopback psuedo-device (thanks Philip Edelbrock)
 *
 * Revision 1.6  1999/07/19 01:31:49  craigs
 * Major rewrite to assure ioctls are all done in the correct order as OSS seems
 *    to be incredibly sensitive to this.
 *
 * Revision 1.5  1999/07/11 13:42:13  craigs
 * pthreads support for Linux
 *
 * Revision 1.4  1999/06/30 13:49:26  craigs
 * Added code to allow full duplex audio
 *
 * Revision 1.3  1999/05/28 14:14:29  robertj
 * Added function to get default audio device.
 *
 * Revision 1.2  1999/05/22 12:49:05  craigs
 * Finished implementation for Linux OSS interface
 *
 * Revision 1.1  1999/02/25 03:45:00  robertj
 * Sound driver implementation changes for various unix platforms.
 *
 * Revision 1.1  1999/02/22 13:24:47  robertj
 * Added first cut sound implmentation.
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

#ifdef P_LINUX
#include <sys/soundcard.h>
#include <sys/time.h>
#endif

#ifdef P_FREEBSD
#include <machine/soundcard.h>
#endif

#if defined(P_OPENBSD) || defined(P_NETBSD)
#include <soundcard.h>
#endif

///////////////////////////////////////////////////////////////////////////////
class PAudioDelay : public PObject
{
  PCLASSINFO(PAudioDelay, PObject);

  public:
    PAudioDelay();
    BOOL Delay(int time);
    void Restart();
    int  GetError();

  protected:
    PTime  previousTime;
    BOOL   firstTime;
    int    error;
};

#define MIN_HEADROOM    30
#define MAX_HEADROOM    60
    
PAudioDelay::PAudioDelay()
{
  firstTime = TRUE;
  error = 0;
}
    
void PAudioDelay::Restart()
{
  firstTime = TRUE;
}
  
BOOL PAudioDelay::Delay(int frameTime)
{
  if (firstTime) {
    firstTime = FALSE;
    previousTime = PTime();
    return TRUE;
  }

  error += frameTime;

  PTime now;
  PTimeInterval delay = now - previousTime;
  error -= (int)delay.GetMilliSeconds();
  previousTime = now;

  if (error > 0)
#ifdef P_LINUX
    usleep(error * 1000);
#else
    PThread::Current()->Sleep(error);
#endif

  return error <= -frameTime;

  //if (headRoom > MAX_HEADROOM)
  //  PThread::Current()->Sleep(headRoom - MIN_HEADROOM);
}

///////////////////////////////////////////////////////////////////////////////
// declare type for sound handle dictionary

class SoundHandleEntry : public PObject {

  PCLASSINFO(SoundHandleEntry, PObject)

  public:
    SoundHandleEntry();

    int handle;
    int direction;

    unsigned numChannels;
    unsigned sampleRate;
    unsigned bitsPerSample;
    unsigned fragmentValue;
    BOOL isInitialised;
};

PDICTIONARY(SoundHandleDict, PString, SoundHandleEntry);

#define LOOPBACK_BUFFER_SIZE 5000
#define BYTESINBUF ((startptr<endptr)?(endptr-startptr):(LOOPBACK_BUFFER_SIZE+endptr-startptr))

static char buffer[LOOPBACK_BUFFER_SIZE];
static int  startptr, endptr;
static unsigned int bufferLen;
static PMutex audioBufferMutex;
PAudioDelay readDelay;
PAudioDelay writeDelay;


PMutex PSoundChannel::dictMutex;

static SoundHandleDict & handleDict()
{
  static SoundHandleDict dict;
  return dict;
}

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


BOOL PSound::Load(const PFilePath & /*filename*/)
{
  return FALSE;
}


BOOL PSound::Save(const PFilePath & /*filename*/)
{
  return FALSE;
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
  PSoundChannel channel(PSoundChannel::GetDefaultDevice(PSoundChannel::Player),
                        PSoundChannel::Player);
  if (!channel.IsOpen())
    return FALSE;

  return channel.PlayFile(file, wait);
}


///////////////////////////////////////////////////////////////////////////////

SoundHandleEntry::SoundHandleEntry()
{
  handle    = -1;
  direction = 0;
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
  os_handle = -1;
}


PSoundChannel::~PSoundChannel()
{
  Close();
}


static void CollectSoundDevices(PDirectory devdir, POrdinalToString & dsp, POrdinalToString & mixer)
{
  if (!devdir.Open())
    return;

  do {
    PString devname = devdir + devdir.GetEntryName();
    if (devdir.IsSubDir())
      CollectSoundDevices(devname, dsp, mixer);
    else {
      PFileInfo info;
      if (devdir.GetInfo(info) &&info.type == PFileInfo::CharDevice) {
        struct stat s;
        if (lstat(devname, &s) == 0) {
          // OSS compatible audio major device numbers (OSS, ALSA, SAM9407, etc)
          static const unsigned deviceNumbers[] = { 14, 116, 145 };
          for (PINDEX i = 0; i < PARRAYSIZE(deviceNumbers); i++) {
            if ((s.st_rdev >> 8) == deviceNumbers[i]) {
              PINDEX cardnum = (s.st_rdev >> 4) & 15;
              if ((s.st_rdev & 15) == 3)   // Digital audio minor device number
                dsp.SetAt(cardnum, devname);
              else if ((s.st_rdev & 15) == 0)   // Digital audio minor device number
                mixer.SetAt(cardnum, devname);
            }
          }
        }
      }
    }
  } while (devdir.Next());
}


PStringArray PSoundChannel::GetDeviceNames(Directions /*dir*/)
{
  // First scan all of the devices and look for ones with major device
  // numbers corresponding to OSS compatible drivers.
  POrdinalToString dsp, mixer;
  CollectSoundDevices("/dev", dsp, mixer);

  // Now we go through the collected devices and see if any have a phyisical reality
  PStringList devices;

  for (PINDEX i = 0; i < dsp.GetSize(); i++) {
    PINDEX cardnum = dsp.GetKeyAt(i);
    // Try and open mixer if have one as this is unlikely to fail for any
    // reason other than there not being a physical device
    if (mixer.Contains(cardnum)) {
      int fd = ::open(mixer[cardnum], O_RDONLY);
      if (fd >= 0) {
        // Do something with the mixer to be sure it is there
        int dummy;
        if (::ioctl(fd, SOUND_MIXER_READ_DEVMASK, &dummy) >= 0)
          devices.AppendString(dsp[cardnum]);
        ::close(fd);
      }
    }
    else {
      // No mixer available, try and open it directly, this could fail if
      // the device happens to be open already
      int fd = ::open(dsp[cardnum], O_RDONLY);
      if (fd >= 0) {
        devices.AppendString(dsp[cardnum]);
        ::close(fd);
      }
    }
  }

  devices.AppendString("loopback");

  return devices;
}


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "/dev/dsp";
}


BOOL PSoundChannel::Open(const PString & _device,
                              Directions _dir,
                                unsigned _numChannels,
                                unsigned _sampleRate,
                                unsigned _bitsPerSample)
{
  Close();

  // lock the dictionary
  PWaitAndSignal mutex(dictMutex);

  // make the direction value 1 or 2
  int dir = _dir + 1;

  // if this device in in the dictionary
  if (handleDict().Contains(_device)) {

    PTRACE(6, "OSS\tOpen occured for existing entry");

    SoundHandleEntry & entry = handleDict()[_device];

    // see if the sound channel is already open in this direction
    if ((entry.direction & dir) != 0) 
      return FALSE;

    // flag this entry as open in this direction
    entry.direction |= dir;
    os_handle = entry.handle;

  } else {

    PTRACE(6, "OSS\tOpen occured for new entry");

    // this is the first time this device has been used
    // open the device in read/write mode always
    if (_device == "loopback") {
      startptr = endptr = 0;
      bufferLen = 0;
      os_handle = 0; // Use os_handle value 0 to indicate loopback, cannot ever be stdin!
    }
    else if (!ConvertOSError(os_handle = ::open((const char *)_device, O_RDWR))) 
      return FALSE;

    // add the device to the dictionary
    SoundHandleEntry * entry = PNEW SoundHandleEntry;
    handleDict().SetAt(_device, entry); 

    // save the information into the dictionary entry
    entry->handle        = os_handle;
    entry->direction     = dir;
    entry->numChannels   = mNumChannels     = _numChannels;
    entry->sampleRate    = actualSampleRate = mSampleRate    = _sampleRate;
    entry->bitsPerSample = mBitsPerSample   = _bitsPerSample;
    entry->isInitialised = FALSE;
    entry->fragmentValue = 0x7fff0008;
  }
   
  // save the direction and device
  direction     = _dir;
  device        = _device;
  isInitialised = FALSE;

  return TRUE;
}

BOOL PSoundChannel::Setup()
{
  PWaitAndSignal mutex(dictMutex);

  if (os_handle < 0) {
    PTRACE(6, "OSS\tSkipping setup of " << device << " as not open");
    return FALSE;
  }

  if (isInitialised) {
    PTRACE(6, "OSS\tSkipping setup of " << device << " as instance already initialised");
    return TRUE;
  }

  // the device must always be in the dictionary
  PAssertOS(handleDict().Contains(device));

  // get record for the device
  SoundHandleEntry & entry = handleDict()[device];

  // set default return status
  BOOL stat = TRUE;

  // do not re-initialise initialised devices
  if (entry.isInitialised || (device == "loopback")) {
    PTRACE(6, "OSS\tSkipping setup for " << device << " as already initialised");

  } else {
    PTRACE(6, "OSS\tInitialising " << device << "(" << (void *)(&entry) << ")");


#if defined(P_LINUX)
    // enable full duplex (maybe).
    ::ioctl(os_handle, SNDCTL_DSP_SETDUPLEX, 0);
#endif

    stat = FALSE;

  // must always set paramaters in the following order:
  //   buffer paramaters
  //   sample format (number of bits)
  //   number of channels (mon/stereo)
  //   speed (sampling rate)

    int arg, val;

    // reset the device first so it will accept the new parms
    if (ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_RESET, &arg))) {

      // set the write fragment size (applies to sound output only)
      arg = val = entry.fragmentValue;
      ::ioctl(os_handle, SNDCTL_DSP_SETFRAGMENT, &arg); 

      audio_buf_info info;
      ::ioctl(os_handle, SNDCTL_DSP_GETOSPACE, &info);
      PTRACE(6, "OSS\tOutput: fragments = " << info.fragments
                     << ", total frags = " << info.fragstotal
                     << ", frag size   = " << info.fragsize
                     << ", bytes       = " << info.bytes);

      ::ioctl(os_handle, SNDCTL_DSP_GETISPACE, &info);
      PTRACE(6, "OSS\tInput: fragments = " << info.fragments
                     << ", total frags = " << info.fragstotal
                     << ", frag size   = " << info.fragsize
                     << ", bytes       = " << info.bytes);

      mBitsPerSample = entry.bitsPerSample;
      arg = val = (entry.bitsPerSample == 16) ? AFMT_S16_LE : AFMT_S8;
      if (ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SETFMT, &arg)) || (arg != val)) {

        mNumChannels = entry.numChannels;
        arg = val = (entry.numChannels == 2) ? 1 : 0;
        if (ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_STEREO, &arg)) || (arg != val)) {

          mSampleRate = entry.sampleRate;
          arg = val = entry.sampleRate;
          if (ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SPEED, &arg))) {
            stat = TRUE;
            actualSampleRate = arg;
          }
        }
      }
    }
  }

  // ensure device is marked as initialised
  isInitialised       = TRUE;
  entry.isInitialised = TRUE;

  return stat;
}

BOOL PSoundChannel::Close()
{
  // if the channel isn't open, do nothing
  if (os_handle < 0)
    return TRUE;

  // the device must be in the dictionary
  dictMutex.Wait();
  SoundHandleEntry * entry;
  PAssert((entry = handleDict().GetAt(device)) != NULL, "Unknown sound device \"" + device + "\" found");

  // modify the directions bit mask in the dictionary
  entry->direction ^= (direction+1);

  // if this is the last usage of this entry, then remove it
  if (entry->direction == 0) {
    handleDict().RemoveAt(device);
    dictMutex.Signal();
    if (os_handle == 0) { // indicates loopback device
      os_handle = -1;
      usleep(2000); // this delay will ensure the other usleep loops notice that
                    // os_handle is no longer 0
      return TRUE;
    } else {
      return PChannel::Close();
    }
  }

  // flag this channel as closed
  dictMutex.Signal();
  if (os_handle == 0) {
    os_handle = -1;
    usleep(2000); // this delay will ensure the other usleep loops notice that
                  // os_handle is no longer 0
  } else {
    os_handle = -1;
  }
  return TRUE;
}

BOOL PSoundChannel::Write(const void * buf, PINDEX len)
{
  if (!Setup())
    return FALSE;

  if (os_handle > 0) {
    while (!ConvertOSError(::write(os_handle, (void *)buf, len)))
      if (GetErrorCode() != Interrupted)
        return FALSE;
    return TRUE;
  }

  // Write to the Circular Buffer

  // Do a sleep to simulate the amount of time the hardware would take
  // to write out this data.
  writeDelay.Delay(len/16);

  // Obtain the audioBufferMutex. Release it at the end of this function.
  PWaitAndSignal muxex(audioBufferMutex);

  // Check if there is space to write the audio.
  if (bufferLen + len > LOOPBACK_BUFFER_SIZE) {
    PTRACE(1,"buffer is full. Cannot write\n");
    return TRUE; // the write failed, but we return OK
  }

  unsigned int index = 0;
  while (index < len) {
    buffer[endptr++] = ((char *)buf)[index++];
    if (endptr == LOOPBACK_BUFFER_SIZE)
      endptr = 0;
  }
  PTRACE(1,"Write. Buffer was "<<bufferLen<<" and goes up by "<<len);
  bufferLen = bufferLen + len;
  return TRUE;
}

BOOL PSoundChannel::Read(void * buf, PINDEX len)
{
  if (!Setup())
    return FALSE;

  if (os_handle > 0) {
    PTRACE(6, "OSS\tRead start");
    int stat;
    while (!ConvertOSError(stat = ::read(os_handle, (void *)buf, len))) {
      if (GetErrorCode() != Interrupted) {
        PTRACE(6, "OSS\tRead failed");
        return FALSE;
      }
      PTRACE(6, "OSS\tRead interrupted");
    }

    if (stat != (int)len)
      PTRACE(6, "OSS\tRead completed short - " << stat << " vs " << len);
    else
      PTRACE(6, "OSS\tRead completed");

    return TRUE;
  }

  // os_handle = 0 indicated loopback mode


  // Read from the Circular Buffer

  // This sleep simulates the time taken to read from real hardware
  readDelay.Delay(len/16);


  // Obtain the audioBufferMutex. Release it at the end of this function.
  PWaitAndSignal mutex(audioBufferMutex);

  // If there is no data in the buffer, we output silence
  if (bufferLen == 0) {
    PTRACE(1,"all zero\n");
    memset(buf, 0, len);
    return TRUE;
  }

  // There may not be enough audio data in the buffer, so find out
  // how much we can copy.
  unsigned int copy = ((len < bufferLen) ? len : bufferLen);

  for (unsigned int index = 0; index < copy; index++) {
    ((char *)buf)[index]=buffer[startptr++];
    if (startptr == LOOPBACK_BUFFER_SIZE)
      startptr = 0;
  }
  PTRACE(1,"Read - buffer len is "<<bufferLen<< " and goes down by "<<copy);
  bufferLen = bufferLen - copy;


  // If there was some data in the buffer, but not enough for the
  // amount required, use the data in the buffer followed by silence.
  if (copy < len) {
    memset(&(((char *)buf)[copy]), 0, len - copy);
  }

  return TRUE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  // check parameters
  PAssert((bitsPerSample == 8) || (bitsPerSample == 16), PInvalidParameter);
  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);

  // lock the dictionary
  PWaitAndSignal mutex(dictMutex);

  // the device must always be in the dictionary
  PAssertOS(handleDict().Contains(device));

  // get record for the device
  SoundHandleEntry & entry = handleDict()[device];

  if (entry.isInitialised) {
    if ((numChannels   != entry.numChannels) ||
        (sampleRate    != entry.sampleRate) ||
        (bitsPerSample != entry.bitsPerSample)) {
      PTRACE(6, "OSS\tTried to change read/write format without stopping");
      return FALSE;
    }
    return TRUE;
  }

  Abort();

  entry.numChannels   = numChannels;
  entry.sampleRate    = sampleRate;
  entry.bitsPerSample = bitsPerSample;
  entry.isInitialised  = FALSE;

  // mark this channel as uninitialised
  isInitialised = FALSE;

  return TRUE;
}

// Get  the number of channels (mono/stereo) in the sound.
unsigned PSoundChannel::GetChannels()   const
{
  return mNumChannels;
}

// Get the sample rate in samples per second.
unsigned PSoundChannel::GetSampleRate() const
{
  return actualSampleRate;
}

// Get the sample size in bits per sample.
unsigned PSoundChannel::GetSampleSize() const
{
  return mBitsPerSample;
}


BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  //PINDEX totalSize = size * count;

  //size = 16;
  //count = (totalSize + 15) / 16;

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);
  int arg = 1;
  while (size > (PINDEX)(1 << arg))
    arg++;

  arg |= count << 16;

  // lock the dictionary
  PWaitAndSignal mutex(dictMutex);

  // the device must always be in the dictionary
  PAssertOS(handleDict().Contains(device));

  // get record for the device
  SoundHandleEntry & entry = handleDict()[device];

  if (entry.isInitialised) {
    if (entry.fragmentValue != (unsigned)arg) {
      PTRACE(6, "OSS\tTried to change buffers without stopping");
      return FALSE;
    }
    return TRUE;
  }

  Abort();

  // set information in the common record
  entry.fragmentValue = arg;
  entry.isInitialised = FALSE;

  // flag this channel as not initialised
  isInitialised       = FALSE;

  return TRUE;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  // lock the dictionary
  PWaitAndSignal mutex(dictMutex);

  // the device must always be in the dictionary
  PAssertOS(handleDict().Contains(device));

  SoundHandleEntry & entry = handleDict()[device];

  int arg = entry.fragmentValue;

  count = arg >> 16;
  size = 1 << (arg&0xffff);
  return TRUE;
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  Abort();

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return FALSE;

  if (wait)
    return WaitForPlayCompletion();

  return TRUE;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  PFile file(filename, PFile::ReadOnly);
  if (!file.IsOpen())
    return FALSE;

  for (;;) {
    BYTE buffer[256];
    if (!file.Read(buffer, 256))
      break;
    PINDEX len = file.GetLastReadCount();
    if (len == 0)
      break;
    if (!Write(buffer, len))
      break;
  }

  file.Close();

  if (wait)
    return WaitForPlayCompletion();

  return TRUE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  if (os_handle == 0)
    return BYTESINBUF <= 0;

  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETOSPACE, &info)))
    return FALSE;

  return info.fragments == info.fragstotal;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  if (os_handle == 0) {
    while (BYTESINBUF > 0) {
      usleep(1000);
      if (os_handle != 0) { // check if the loopback audio has been closed
        return FALSE;
      }
    }
    return TRUE;
  }

  return ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_SYNC, NULL));
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  return FALSE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  return FALSE;
}


BOOL PSoundChannel::StartRecording()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  if (os_handle == 0)
    return TRUE;

  fd_set fds;
  FD_ZERO(&fds);
  FD_SET(os_handle, &fds);

  struct timeval instant = {0, 0};
  return ConvertOSError(::select(1, &fds, NULL, NULL, &instant));
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  if (os_handle == 0)
    return (BYTESINBUF > 0);

  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETISPACE, &info)))
    return FALSE;

  return info.fragments > 0;
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  if (os_handle == 0)
    return (BYTESINBUF == LOOPBACK_BUFFER_SIZE);

  audio_buf_info info;
  if (!ConvertOSError(::ioctl(os_handle, SNDCTL_DSP_GETISPACE, &info)))
    return FALSE;

  return info.fragments == info.fragstotal;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF);

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannel::Abort()
{
  if (os_handle == 0) {
    startptr = endptr = 0;
    return TRUE;
  }

  return ConvertOSError(ioctl(os_handle, SNDCTL_DSP_RESET, NULL));
}


// End of file
