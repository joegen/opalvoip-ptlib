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
 * Revision 1.1  2003/02/24 17:50:40  rogerh
 * Add Mac OS X Core Audio support.
 *
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

#include <CoreAudio/CoreAudio.h>

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
  caCBData = 0;

  os_handle = 0;  // pretends to be a loopback device?
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
  if (direction == Player) {
    AudioDeviceStop(caDevID, PlaybackIOProc);
    AudioDeviceRemoveIOProc(caDevID, PlaybackIOProc);
  }
  else {
    AudioDeviceStop(caDevID, RecordIOProc);
    AudioDeviceRemoveIOProc(caDevID, RecordIOProc);
  }

  if (caCBData) {
    free(caCBData);
    caCBData = NULL;
  }

  if (caBuf) {
    free(caBuf);
    caBuf = NULL;
  }

  return PChannel::Close();
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  OSStatus theStatus;
  UInt32 theSize;
  bool isInput = (direction == Player ? false : true);

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

  /* try to set it, but does not seem to work
  theDescription.mSampleRate = sampleRate;
  theDescription.mChannelsPerFrame = numChannels;
  theStatus = AudioDeviceSetProperty(caDevID, NULL, 0, isInput,
				     kAudioDevicePropertyStreamFormat,
				     theSize, &theDescription);
  if (theStatus != 0) {
    fprintf(stderr, "set status = %x\n", theStatus);
    return FALSE;
  }
  */

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
    samples*(44100*caNumChannels*sizeof(float))/(mSampleRate*mNumChannels);
  propertySize = sizeof(bufferByteCount);

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
  float scale = 1.0 / SHRT_MAX;

  int samples = len/(mBitsPerSample/8);

  if (!caBuf) {
    return FALSE;
  }

  pthread_mutex_lock(&caMutex);
  while (caBufLen > 0)
    pthread_cond_wait(&caCond, &caMutex);

  int duplicates = (44100*caNumChannels/mSampleRate);
  for (int i = 0; i < samples; i++) {
    short *src = (short *)buf;
    float *dst = caBuf;
    for (int j = 0; j < duplicates; j++) {
      dst[i*duplicates+j] = scale * src[i];
    }
  }
  caBufLen = samples * (sizeof(float)*duplicates);

  pthread_mutex_unlock(&caMutex);

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
  float scale = SHRT_MAX;

  int samples = len/(mBitsPerSample/8);

  if (!caBuf) {
    return FALSE;
  }

  pthread_mutex_lock(&caMutex);
  while (caBufLen == 0)
    pthread_cond_wait(&caCond, &caMutex);

  int shrinkWindow = 44100*caNumChannels/mSampleRate;
  for (int i = 0; i < samples; i++) {
    short *dst = (short *)buf;
    float *src = caBuf;
    /*
    float x = 0.0;
    for (int j = 0; j < shrinkWindow; j++) {
      x += src[i*(shrinkWindow)+j];
    }
    */
    dst[i] = (short)(scale * src[i*(shrinkWindow)]);
  }
  caBufLen = 0;

  pthread_mutex_unlock(&caMutex);

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

