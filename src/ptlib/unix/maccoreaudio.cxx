/*
 * maccoreaudio.cxx
 *
 * Sound driver implementation to use CoreAudio on Mac OS X.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is
 * Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: maccoreaudio.cxx,v $
 * Revision 1.6  2003/03/12 23:07:22  rogerh
 * Changes from Shawn.
 *
 * Revision 1.5  2003/03/05 09:23:51  rogerh
 * Fixes from Shawn Pai-Hsiang Hsiao
 *
 * Revision 1.4  2003/03/03 09:04:23  rogerh
 * Add changes from Shawn. Plus I added some os_handle fixes.
 *
 * Revision 1.3  2003/03/02 06:44:51  rogerh
 * More updates from Shawn.
 *
 * Revision 1.2  2003/03/01 17:05:05  rogerh
 * Mac OS X updates from Shawn Pai-Hsiang Hsiao
 *
 * Revision 1.1  2003/02/24 17:50:40  rogerh
 * Add Mac OS X Core Audio support.
 *
 *
 */

#pragma implementation "sound.h"

#ifndef MAC_CA_USE_AUDIO_CONVERTER
#define MAC_CA_USE_AUDIO_CONVERTER 1
#endif

#include <ptlib.h>

#include <CoreAudio/CoreAudio.h>
#if MAC_CA_USE_AUDIO_CONVERTER == 1
#include <AudioToolbox/AudioConverter.h>
#endif

#define CA_NULL_DEVICE_NAME "Null"

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
  caBuf = NULL;
  caBufLen = 0;
  caCBData = NULL;
  caConverterRef = NULL;

  os_handle = -1; // set channel closed.
  // set to a non negative value so IsOpen() returns true
}


PSoundChannel::~PSoundChannel()
{
  Close();
}

/*
 * Returns the number of audio devices found.
 * The ID list is stored at *devlist
 */
static int CADeviceList(AudioDeviceID **devlist)
{
  OSStatus theStatus;
  UInt32 theSize;
  int numDevices;
  AudioDeviceID *deviceList;

  theStatus = AudioHardwareGetPropertyInfo(kAudioHardwarePropertyDevices,
					   &theSize, NULL);
  if (theStatus != 0) {
    (*devlist) = NULL;
    return 0;
  }

  numDevices = theSize/sizeof(AudioDeviceID);

  deviceList = (AudioDeviceID *)malloc(theSize);
  if (deviceList == NULL) {
    (*devlist) = NULL;
    return 0;
  }

  theStatus = AudioHardwareGetProperty(kAudioHardwarePropertyDevices,
				       &theSize, deviceList);
  if (theStatus != 0) {
    // free(deviceList);
    (*devlist) = NULL;
    return 0;
  }

  (*devlist) = deviceList;
  return numDevices;
}

/*
 * Retrieves the name of the device, given its AudioDeviceID
 */
static PString CADeviceName(AudioDeviceID id)
{
  OSStatus theStatus;
  UInt32 theSize;
  char name[128];

  theSize = sizeof(name);
  theStatus = AudioDeviceGetProperty(id, 0, false,
				     kAudioDevicePropertyDeviceName,
				     &theSize, name);
  if (theStatus != 0 || *name == NULL)
    return NULL;

  return (PString(name));
}

/*
 * XXX: don't know what is the best way to decide if a device supports
 * input/output or not yet
 *
 * See if the device supports player/recorder direction.  This test is
 * based on how many streams the device has for that direction.
 */
static int CADeviceSupportDirection(AudioDeviceID id,
				    int dir /* 1 for Player, 0 for Recorder */)
{
  OSStatus theStatus;
  UInt32 theSize;

  UInt32 numStreams;

  bool isInput = (dir == 0? true : false);

  theStatus = AudioDeviceGetPropertyInfo(id, 0, isInput,
					 kAudioDevicePropertyStreams,
					 &theSize, NULL );

  if (theStatus == 0) {
    numStreams = theSize/sizeof(AudioStreamID);
    return (numStreams);
  }
  else {
    return (0);
  }
}

PStringArray PSoundChannel::GetDeviceNames(Directions dir)
{
  PStringList devices;

  int numDevices;
  AudioDeviceID *deviceList;

  numDevices = CADeviceList(&deviceList);

  for (int i = 0; i < numDevices; i++) {
    PString s = CADeviceName(deviceList[i]);
    if (CADeviceSupportDirection(deviceList[i], dir) > 0) {
      devices.AppendString(s);
    }
  }
  devices.AppendString(CA_NULL_DEVICE_NAME);

  free(deviceList);
  deviceList = NULL;

  return devices;
}


PString PSoundChannel::GetDefaultDevice(Directions dir)
{
  OSStatus theStatus;
  UInt32 theSize;
  AudioDeviceID theID;

  theSize = sizeof(AudioDeviceID);

  if (dir == Player) {
    theStatus =
      AudioHardwareGetProperty(kAudioHardwarePropertyDefaultOutputDevice,
			       &theSize, &theID);
  }
  else {
    theStatus =
      AudioHardwareGetProperty(kAudioHardwarePropertyDefaultInputDevice,
			       &theSize, &theID);
  }
  if (theStatus == 0) {
    return CADeviceName(theID);
  } else {
    return PString("None");
  }
}

/*
 * A local type to aggregate data to the callback functions
 */
struct myData {
  int *bufLen;
  float *buf;
  pthread_mutex_t *myMutex;
  pthread_cond_t *myCond;
};

/*
 * CoreAudio Player callback function
 */
OSStatus PlaybackIOProc(AudioDeviceID inDevice,
                        const AudioTimeStamp *inNow,
                        const AudioBufferList *inInputData,
                        const AudioTimeStamp *inInputTime,
                        AudioBufferList *outOutputData,
                        const AudioTimeStamp *inOutputTime,
                        void *inClientData)
{
  float *bufPtr = (float *)outOutputData->mBuffers[0].mData;
  /*
  unsigned int bufLen = (int)inOutputData->mBuffers[0].mDataByteSize;
  */
  struct myData *data = (struct myData *)inClientData;
  unsigned int numFloats = *(data->bufLen)/sizeof(float);

  pthread_mutex_lock(data->myMutex);

  for (unsigned int i = 0; i < numFloats; i++)
    bufPtr[i] = data->buf[i];
  (*data->bufLen) = 0;

  pthread_mutex_unlock(data->myMutex);
  pthread_cond_signal(data->myCond);

  return (kAudioHardwareNoError);
}


/*
 * CoreAudio Recorder callback function
 */
OSStatus RecordIOProc(AudioDeviceID inDevice,
                      const AudioTimeStamp *inNow,
                      const AudioBufferList *inInputData,
                      const AudioTimeStamp *inInputTime,
                      AudioBufferList *outOutputData,
                      const AudioTimeStamp *inOutputTime,
                      void *inClientData)
{
  float *bufPtr = (float *)inInputData->mBuffers[0].mData;
  unsigned int bufLen = (int)inInputData->mBuffers[0].mDataByteSize;
  struct myData *data = (struct myData *)inClientData;

  pthread_mutex_lock(data->myMutex);

  for (unsigned int i = 0; i < (bufLen/sizeof(float)); i++)
    data->buf[i] = bufPtr[i];
  (*data->bufLen) = bufLen;

  pthread_mutex_unlock(data->myMutex);
  pthread_cond_signal(data->myCond);

  return (kAudioHardwareNoError);
}

/*
 * The major task of Open() is to find the matching device ID.
 *
 */
BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
  int rval;
  int numDevices;
  AudioDeviceID *deviceList;

  direction = dir;
  caDevID = -1;

  if (strcmp(device, CA_NULL_DEVICE_NAME) == 0) {
    os_handle = 0;
    return TRUE;
  }

  numDevices = CADeviceList(&deviceList);

  for (int i = 0; i < numDevices; i++) {
    PString s = CADeviceName(deviceList[i]);
    if ((CADeviceSupportDirection(deviceList[i], dir) > 0) && device == s) {
      caDevID = deviceList[i];
      break;
    }
  }
  free(deviceList);
  deviceList = NULL;

  /* returns if there is no match */
  if (caDevID < 0) {
    return FALSE;
  }

  PTRACE(1, "Looking for " << device << " and found " << caDevID
       << " with " << CADeviceSupportDirection(caDevID, dir) << " streams"
       << endl);

  rval = pthread_mutex_init(&caMutex, NULL);
  if (rval) {
    PTRACE(1, "can not init mutex\n");
    return FALSE;
  }
  rval = pthread_cond_init(&caCond, NULL);
  if (rval) {
    PTRACE(1, "can not init cond\n");
    return FALSE;
  }

  os_handle = 0;  // tell PChannel (and IsOpen()) that the channel is open.

  return SetFormat(numChannels, sampleRate, bitsPerSample);
}

BOOL PSoundChannel::SetVolume(unsigned newVal)
{
  OSStatus theStatus;
  Boolean isWritable;
  bool isInput = (direction == Player ? false : true);

  if (caDevID < 0)
    return FALSE;

  /* changing volume can not go through the master channel (0) */
  theStatus = AudioDeviceGetPropertyInfo(caDevID, 1, isInput,
					 kAudioDevicePropertyVolumeScalar,
					 NULL, &isWritable);

  if ((theStatus == kAudioHardwareNoError) && isWritable) {
    /* newVal is between 0 and 100 ? */
    float theValue = ((float)newVal)/100.0;
    theStatus = AudioDeviceSetProperty(caDevID, NULL, 1, isInput,
				       kAudioDevicePropertyVolumeScalar,
				       sizeof(float), &theValue);
    if (theStatus == 0) {
      return TRUE;
    }
  }

  return FALSE;
}

BOOL  PSoundChannel::GetVolume(unsigned &devVol)
{
  OSStatus theStatus;
  UInt32 theSize;
  Float32 theValue;
  bool isInput = (direction == Player ? false : true);

  if (caDevID < 0)
    return FALSE;

  theSize = sizeof(theValue);
  /* changing volume can not go through the master channel (0) */
  theStatus = AudioDeviceGetProperty(caDevID, 1, isInput,
				     kAudioDevicePropertyVolumeScalar,
				     &theSize, &theValue);
  if (theStatus == 0) {
    /* devVal is between 0 and 100? */
    devVol = (unsigned) (theValue * 100);
    return TRUE;
  }

  return FALSE;
}
  


BOOL PSoundChannel::Close()
{
  if (caDevID > 0) {
    if (direction == Player) {
      AudioDeviceStop(caDevID, PlaybackIOProc);
      AudioDeviceRemoveIOProc(caDevID, PlaybackIOProc);
    }
    else {
      AudioDeviceStop(caDevID, RecordIOProc);
      AudioDeviceRemoveIOProc(caDevID, RecordIOProc);
    }
    caDevID = -1;

#if (MAC_CA_USE_AUDIO_CONVERTER == 1)
    OSStatus theStatus;

    if (caConverterRef) {
      theStatus = AudioConverterDispose((AudioConverterRef)caConverterRef);
      if (theStatus != 0) {
	PTRACE(1, "AudioConverterDispose failed");
      }
      caConverterRef = NULL;
    }
#endif

    if (caCBData) {
      free(caCBData);
      caCBData = NULL;
    }

    if (caBuf) {
      free(caBuf);
      caBuf = NULL;
    }

    pthread_mutex_destroy(&caMutex);
    pthread_cond_destroy(&caCond);
  }

  os_handle = -1;  // tell PChannel (and IsOpen()) that the channel is closed.
  return TRUE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  OSStatus theStatus;
  UInt32 theSize;
  bool isInput = (direction == Player ? false : true);

  if (caDevID < 0) {
    return TRUE;  /* Null device */
  }

  mNumChannels = numChannels;
  actualSampleRate = mSampleRate = sampleRate;
  mBitsPerSample = bitsPerSample;

  // making some assumptions about input format for now
  PAssert(mSampleRate == 8000 && mNumChannels == 1 && mBitsPerSample == 16,
	  PUnsupportedFeature);

  /*
   * It looks like 44.1Khz, 2 ch, 32-bit set up is the fixed default
   * for all the devices?
   */

  /* get default stream description */
  AudioStreamBasicDescription theDescription;
	
  theSize = sizeof(AudioStreamBasicDescription);
  theStatus = AudioDeviceGetProperty(caDevID, 0, isInput,
				     kAudioDevicePropertyStreamFormat,
				     &theSize, &theDescription);
  if (theStatus != 0) {
    PTRACE(1, "can not lookup stream format (" << theStatus << ")");
    return FALSE;
  }

  PAssert(theDescription.mSampleRate == 44100.0 &&
	  (theDescription.mChannelsPerFrame == 2 ||
	   theDescription.mChannelsPerFrame == 1   ) &&
	  theDescription.mBitsPerChannel == 32,
	  PUnsupportedFeature);

  caNumChannels = theDescription.mChannelsPerFrame;

#if (MAC_CA_USE_AUDIO_CONVERTER == 1)
  /* the converter code is adapted from vlc */

  /* XXX: here might be a problem.
   *
   * We allocate the description on the stack to create converter,
   * let's hope the created converter copies the description over.
   */
  AudioStreamBasicDescription pwlibDescription;
  memcpy(&pwlibDescription, &theDescription, sizeof(theDescription));

  /*
   * XXX: yet another problem
   *
   * the endian of the 16bit sample is platform dependent, however,
   * Mac only runs on big-endian machine, right?
   */
  pwlibDescription.mFormatFlags &= ~kLinearPCMFormatFlagIsFloat;
  pwlibDescription.mFormatFlags |= kLinearPCMFormatFlagIsBigEndian;
  pwlibDescription.mFormatFlags |= kLinearPCMFormatFlagIsSignedInteger;

  if(theDescription.mFormatFlags & kLinearPCMFormatFlagIsFloat ) {
    pwlibDescription.mBytesPerPacket = pwlibDescription.mBytesPerPacket / 2;
    pwlibDescription.mBytesPerFrame = pwlibDescription.mBytesPerFrame / 2;
    pwlibDescription.mBitsPerChannel = pwlibDescription.mBitsPerChannel / 2;
  }

  if(theDescription.mChannelsPerFrame == 2) {
    pwlibDescription.mBytesPerPacket = pwlibDescription.mBytesPerPacket / 2;
    pwlibDescription.mBytesPerFrame = pwlibDescription.mBytesPerFrame / 2;
  }

  pwlibDescription.mSampleRate = (Float64)sampleRate;
  pwlibDescription.mChannelsPerFrame = numChannels;


#define PRINT_DATA(field, str) \
  cerr << str << ": " << pwlibDescription.field << " " \
                      << theDescription.field << endl
/*
  PRINT_DATA(mSampleRate, "sampleRate");
  PRINT_DATA(mFormatID, "formatID");
  PRINT_DATA(mFormatFlags, "formatFlags");
  PRINT_DATA(mBytesPerPacket, "bytesPerPacket");
  PRINT_DATA(mFramesPerPacket, "framesPerPacket");
  PRINT_DATA(mBytesPerFrame, "bytesPerFrame");
  PRINT_DATA(mChannelsPerFrame, "channelsPerFrame");
  PRINT_DATA(mBitsPerChannel, "bitsPerChannel");
*/

  if (direction == Player) {
    theStatus = AudioConverterNew(&pwlibDescription,
				  &theDescription,
				  &(AudioConverterRef)caConverterRef);
  }
  else {
    theStatus = AudioConverterNew(&theDescription,
				  &pwlibDescription,
				  &(AudioConverterRef)caConverterRef);
  }
  if (theStatus != 0) {
    PTRACE(1, "can not create audio converter for streams " << theStatus);
    return FALSE;
  }

  UInt32 quality = kAudioConverterQuality_Low;

  theStatus =
    AudioConverterSetProperty((AudioConverterRef)caConverterRef,
			      kAudioConverterSampleRateConverterQuality,
			      sizeof(UInt32),
			      &quality);
  if (theStatus != 0) {
    PTRACE(1, "can not set converter quality " << theStatus);
  }

#undef PRINT_DATA
#endif

  return TRUE;
}

/*
 * SetBuffers does most of the work before it starts playback or
 * recording.
 *
 * A device can not be used after calling Open(), SetBuffers() must
 * also be called before it can start functioning.
 *
 */
BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  int samples = size/(mBitsPerSample/8);

  OSStatus theStatus;
  UInt32 theSize;
  AudioValueRange theRange;

  int bufferByteCount, propertySize;

  if (caDevID < 0) {
    return TRUE;  /* Null device */
  }

  bool isInput = (direction == Player ? false : true);

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  /* find the largest possible buffer size, and allocate a buffer of
   * the size for the callback function
   */
  theSize = sizeof(AudioValueRange);
  theStatus = AudioDeviceGetProperty(caDevID, 0, isInput,
				     kAudioDevicePropertyBufferSizeRange,
				     &theSize, &theRange);
  if (theStatus != 0) {
    PTRACE(1, "can not find buffer size (" << theStatus << ")");
    return FALSE;
  }

  caBuf = (float *)malloc((int)theRange.mMaximum);
  if (!caBuf) {
    PTRACE(1, "can not init caBuf for callback function\n");
    return FALSE;
  }

  /*
   * Prepares clientData for callback function
   */
  caBufLen = 0;

  struct myData *clientData;
  clientData = (struct myData *)malloc(sizeof(struct myData));
  if (!clientData) {
    PTRACE(1, "can not allocate memory for clientData\n");
    return FALSE;
  }
  clientData->bufLen  = &caBufLen;
  clientData->buf     = caBuf;
  clientData->myMutex = &caMutex;
  clientData->myCond  = &caCond;

  caCBData = (void *)clientData; // remembers it so we can free it

  /*
   * Registers callback function
   */
  if (direction == Player) {
    PTRACE(6, "Player buffer @ "<< caBuf);
    theStatus =
      AudioDeviceAddIOProc(caDevID, PlaybackIOProc, (void *)clientData);
  }
  else {
    PTRACE(6, "Recorder buffer @ "<< caBuf);
    theStatus =
      AudioDeviceAddIOProc(caDevID, RecordIOProc, (void *)clientData);
  }

  /*
   * Computes the actual buffer size used for the callback function
   *
   * The computation is based on the format of streams and the
   * requested buffer size.
   */
  bufferByteCount =
    samples*
    (44100*caNumChannels*sizeof(float))/
    (mSampleRate*mNumChannels);
  propertySize = sizeof(bufferByteCount);

  PTRACE(1, "CoreAudio buffer size set to " << bufferByteCount);

  theStatus = AudioDeviceSetProperty(caDevID,
				     0,
				     0,
				     isInput,
				     kAudioDevicePropertyBufferSize,
				     propertySize,
				     &bufferByteCount);
  if (theStatus) {
    PTRACE(1, "set device property failed, status = (" << theStatus << ")");
    return (FALSE);
  }

  /*
   * Now the device can start working
   */
  if (direction == Player) {
    AudioDeviceStart(caDevID, PlaybackIOProc);
  }
  else {
    AudioDeviceStart(caDevID, RecordIOProc);
  }

  return TRUE;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::Write(const void * buf, PINDEX len)
{
  UInt32 ChunkSize, theSize;
  OSStatus theStatus;

  int totalSamples, chunkSamples, playedSamples;
  double c;
  UInt32 chunkLen;
  void *chunkBuffer;
  char *playedOffset;

  if (caDevID < 0) {
    usleep(20*1000);
    return TRUE;  /* Null device */
  }

  /* CoreAudio are not properly initialized */
  if (!caBuf) {
    return FALSE;
  }

  theSize = sizeof(ChunkSize);
  theStatus = AudioDeviceGetProperty(caDevID,
				     0, 0,
				     kAudioDevicePropertyBufferSize,
				     &theSize, &ChunkSize);
  if (theStatus) {
    PTRACE(1, "get device property failed, status = (" << theStatus << ")");
    return (FALSE);
  }

  /*
   * CoreAudio wants a chunk at a time, so let's divide incoming
   * buffer into chunks
   */

  totalSamples = len/(mBitsPerSample/8);
  c = (double)ChunkSize*mSampleRate/(44100*caNumChannels*sizeof(float));
  chunkSamples = (int)ceil(c);
  chunkLen = chunkSamples*sizeof(short);
  chunkBuffer = (void *)calloc(chunkSamples, sizeof(short));
  if (!chunkBuffer) {
    return FALSE;
  }

  playedSamples = 0;
  while (playedSamples < totalSamples) {
    if (playedSamples + chunkSamples <= totalSamples) {
      chunkLen = chunkSamples*sizeof(short);
    }
    else {
      chunkLen = (totalSamples-playedSamples)*sizeof(short);
      bzero(chunkBuffer, chunkSamples*sizeof(short));
    }
    playedOffset = ((char *)buf)+(playedSamples*sizeof(short));
    memcpy(chunkBuffer, playedOffset, chunkLen);

    pthread_mutex_lock(&caMutex);
    while (caBufLen > 0)
      pthread_cond_wait(&caCond, &caMutex);

    caBufLen = chunkSamples*
      (44100*caNumChannels*sizeof(float))/
      (mSampleRate*mNumChannels);

#if (MAC_CA_USE_AUDIO_CONVERTER == 1)
    theStatus = AudioConverterConvertBuffer((AudioConverterRef)caConverterRef,
					    chunkLen,
					    chunkBuffer,
					    (UInt32 *)&caBufLen,
					    (void *)caBuf);
    if (theStatus != 0) {
      PTRACE(6, "Warning: audio converter (write) failed "<<theStatus<<endl);
    }
#else
    bzero(caBuf, caBufLen);

    int duplicates = (44100*caNumChannels/mSampleRate);
    float scale = 1.0 / SHRT_MAX;

    for (int i = 0; i < chunkSamples; i++) {
      short *src = (short *)chunkBuffer;
      float *dst = caBuf;
      for (int j = 0; j < duplicates; j++) {
	dst[i*duplicates+j] = scale * src[i];
      }
    }

#endif

    caBufLen = chunkSamples*
      (44100*caNumChannels*sizeof(float))/
      (mSampleRate*mNumChannels);

    if (caNumChannels == 2) {
      /* let's make it two-channel audio */
      for (int i = 0; i < caBufLen; i+=2) {
        caBuf[i+1] = caBuf[i];
      }
    }

    playedSamples += chunkSamples;

    pthread_mutex_unlock(&caMutex);
  }

  free(chunkBuffer);
  chunkBuffer = NULL;

  return (TRUE);
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
  PAssert(0, PUnimplementedFunction);

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return FALSE;

  if (wait)
    return WaitForPlayCompletion();

  return TRUE;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  PAssert(0, PUnimplementedFunction);

  return FALSE;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::Read(void * buf, PINDEX len)
{
  UInt32 ChunkSize, theSize;
  OSStatus theStatus;

  int totalSamples, chunkSamples, chunkLen, recordedSamples;
  double c;
  void *chunkBuffer;
  char *recordedOffset;

  if (caDevID < 0) {
    usleep(20*1000);
    bzero(buf, len);
    lastReadCount = len;
    return TRUE;  /* Null device */
  }


  /* CoreAudio are not properly initialized */
  if (!caBuf) {
    return FALSE;
  }

  theSize = sizeof(ChunkSize);
  theStatus = AudioDeviceGetProperty(caDevID,
				     0, 1,
				     kAudioDevicePropertyBufferSize,
				     &theSize, &ChunkSize);
  if (theStatus) {
    PTRACE(1, "get device property failed, status = (" << theStatus << ")");
    return (FALSE);
  }

  /*
   * CoreAudio returns a chunk at a time, so let's combine incoming
   * chunks to buffer
   */

  totalSamples = len/(mBitsPerSample/8);
  c = (double)ChunkSize*mSampleRate/(44100*caNumChannels*sizeof(float));
  chunkSamples = (int)ceil(c);
  chunkLen = chunkSamples*sizeof(short);
  chunkBuffer = (void *)calloc(chunkSamples, sizeof(short));
  if (!chunkBuffer) {
    return FALSE;
  }

  recordedSamples = 0;
  while (recordedSamples < totalSamples) {
    pthread_mutex_lock(&caMutex);
    while (caBufLen == 0)
      pthread_cond_wait(&caCond, &caMutex);

    chunkLen = chunkSamples*sizeof(short);

#if (MAC_CA_USE_AUDIO_CONVERTER == 1)

    if (caNumChannels == 1) {
      theStatus =
	AudioConverterConvertBuffer((AudioConverterRef)caConverterRef,
				    caBufLen+chunkLen,
				    /* use larger size to fool converter */
				    caBuf,
				    (UInt32 *)&chunkLen,
				    chunkBuffer);
    }
    else {
      theStatus =
	AudioConverterConvertBuffer((AudioConverterRef)caConverterRef,
				    caBufLen,
				    caBuf,
				    (UInt32 *)&chunkLen,
				    chunkBuffer);
    }
    if (theStatus != 0) {
      PTRACE(6, "Warning: audio converter (read) failed "<<theStatus<<endl);
    }
#else
    float scale = SHRT_MAX;
    int shrinkWindow = 44100*caNumChannels/mSampleRate;

    for (int i = 0; i < chunkSamples; i++) {
      short *dst = (short *)chunkBuffer;
      float *src = caBuf;
      /*
	float x = 0.0;
	for (int j = 0; j < shrinkWindow; j++) {
	x += src[i*(shrinkWindow)+j];
	}
      */
      dst[i] = (short)(scale * src[i*(shrinkWindow)]);
    }
#endif

    recordedOffset = ((char *)buf)+(recordedSamples*sizeof(short));
    if (recordedSamples + chunkSamples <= totalSamples) {
      memcpy(recordedOffset, chunkBuffer, chunkSamples*sizeof(short));
    }
    else {
      memcpy(recordedOffset, chunkBuffer,
	     (totalSamples-recordedSamples)*sizeof(short));
    }

    recordedSamples += chunkSamples;
    caBufLen = 0;

    pthread_mutex_unlock(&caMutex);
  }

  free(chunkBuffer);
  chunkBuffer = NULL;

  lastReadCount = len;

  return (TRUE);
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::StartRecording()
{
  PAssert(0, PUnimplementedFunction);

  return (TRUE);
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  PAssert(0, PUnimplementedFunction);

  return (TRUE);
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  PAssert(0, PUnimplementedFunction);

  if (os_handle < 0) {
    return FALSE;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  PAssert(0, PUnimplementedFunction);

  return FALSE;
}


BOOL PSoundChannel::Abort()
{
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}


// End of file

