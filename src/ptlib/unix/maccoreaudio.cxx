/*
 * maccoreaudio.cxx
 *
 * Copyright (c) 2004 Network for Educational Technology ETH
 *
 * Written by Hannes Friederich, Andreas Fenkart.
 * Based on work of Shawn Pai-Hsiang Hsiao
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
 */
 
#pragma implementation "maccoreaudio.h" 

//#include <ptlib.h>
//#include <ptlib/sound.h>
#include <ptlib/unix/ptlib/maccoreaudio.h>
 
//#include <CoreAudio/CoreAudio.h>
//#include <AudioToolbox/AudioConverter.h>

#ifdef __MACOSX__

namespace PWLibStupidOSXHacks {
	int loadCoreAudioStuff;
};

#endif

PCREATE_SOUND_PLUGIN(CoreAudio, PSoundChannelCoreAudio);




/************** util *******************/


#ifdef PTRACING

ostream& operator<<(ostream &os, AudioStreamBasicDescription &inDesc)
{
  os << "- - - - - - - - - - - - - - - - - - - -\n";
  os << "  Sample Rate: " << inDesc.mSampleRate << endl;
  os << "  Format ID: " << (char*)&inDesc.mFormatID << endl;
  os << "  Format Flags: " << hex << inDesc.mFormatFlags << dec << endl;
  os << "  Bytes per Packet: " << inDesc.mBytesPerPacket << endl;
  os << "  Frames per Packet: " << inDesc.mFramesPerPacket << endl;
  os << "  Bytes per Frame: " << inDesc.mBytesPerFrame << endl;
  os << "  Channels per Frame: " << inDesc.mChannelsPerFrame << endl; 
  os << "  Bits per Channel: " << inDesc.mBitsPerChannel << endl;
  os << "- - - - - - - - - - - - - - - - - - - -\n";
  return os;
}


ostream& operator<<(ostream &os, PSoundChannel::Directions &dir)
{
   if(dir == PSoundChannel::Player)
      os << " Player ";
   else if(dir == PSoundChannel::Recorder)
      os << " Recorder ";
   else
      os << " Unknown directon ";
   return os;
}

ostream& operator<<(ostream &os, AudioValueRange range)
{
	os << range.mMinimum << " " << range.mMaximum ;
	return os;

}

#endif


/***** PSound implementation *****/

PSound::PSound(unsigned   channels,
			   unsigned   sampleRate,
			   unsigned   bitsPerSample,
			   PINDEX     bufferSize,
			   const BYTE *data)
{
  PAssert(0, PUnimplementedFunction); 
}

PSound::PSound(const PFilePath & filename)
{
  PAssert(0, PUnimplementedFunction); 
}

PSound & PSound::operator=(const PBYTEArray & data)
{
  PAssert(0, PUnimplementedFunction); 
  return *this;
}

BOOL PSound::Load(const PFilePath & filename)
{	
  PAssert(0, PUnimplementedFunction); 
  return false;
}

BOOL PSound::Save(const PFilePath & filename)
{	
  PAssert(0, PUnimplementedFunction); 
	return false;
}

BOOL PSound::Play()
{
  PAssert(0, PUnimplementedFunction); 
	return false;
}

void PSound::SetFormat(unsigned numChannels,
					   unsigned sampleRate,
					   unsigned bitsPerSample)
{
  this->numChannels = numChannels;
  this->sampleRate = sampleRate;
  this->sampleSize = bitsPerSample;
  formatInfo.SetSize(0);
}

BOOL PSound::PlayFile(const PFilePath & file,
			  BOOL wait)
{
  PAssert(0, PUnimplementedFunction); 
	return false;
}

void PSound::Beep()
{
  PAssert(0, PUnimplementedFunction); 
}


#include "maccoreaudio/CircularBuffer.inc"

/***** PSoundChannel implementation *****/

void PSoundChannelCoreAudio::Init(){
   PWLIB_StaticLoader_CoreAudio_PSoundChannel();
}

PSoundChannelCoreAudio::PSoundChannelCoreAudio() 
   : isRunning(FALSE), mCircularBuffer(NULL), converter_buffer(NULL),
	  mInputCircularBuffer(NULL), mInputBufferList(NULL), mOutputBufferList(NULL)
{
   PTRACE(1, "no arg constructor");
   CommonConstruct();
}

PSoundChannelCoreAudio::PSoundChannelCoreAudio(const PString & device,
					 Directions dir,
					 unsigned numChannels,
					 unsigned sampleRate,
					 unsigned bitsPerSample)
   : mCircularBuffer(NULL), converter_buffer(NULL),
	  mInputCircularBuffer(NULL), mInputBufferList(NULL), mOutputBufferList(NULL)
{
	CommonConstruct();
	Open(device, dir, numChannels, sampleRate, bitsPerSample);
}



PSoundChannelCoreAudio::~PSoundChannelCoreAudio()
{
   PTRACE(1, "Destructor " << direction );
   Close();
}

BOOL PSoundChannelCoreAudio::Close()
{
   OSStatus err = noErr;
   PTRACE(1, "Close" << direction );

   /* OutputUnit also for input device,
    * Stop everything before deallocating buffers */
   //if(mDeviceID != kAudioDeviceDummy){
   if(isRunning){
	   isRunning = FALSE;

   	err = AudioOutputUnitStop(mAudioUnit);
      checkStatus(err);
   	//usleep(6000000);
   	PTRACE(1, direction  << " Output unit stopped"  );
   	// better -> isOpen
      err = AudioUnitUninitialize(mAudioUnit);
   	checkStatus(err);
      // exc_bad_access when closing channel
      err = CloseComponent(mAudioUnit);
	   checkStatus(err);
      err = AudioConverterDispose(converter);
      checkStatus(err);
   }

   /* now free all buffers */
   if(this->converter_buffer != NULL){
      free(this->converter_buffer);
      this->converter_buffer = NULL;
   }
   if(this->mCircularBuffer != NULL){
      delete this->mCircularBuffer;
      this->mCircularBuffer = NULL;
   }
   if(this->mInputCircularBuffer !=NULL) {
      delete this->mInputCircularBuffer;
      this->mInputCircularBuffer = NULL;
   }
   if(this->mInputBufferList != NULL){
      free(this->mInputBufferList);
      this->mInputBufferList = NULL;
   }
   if(this->mOutputBufferList != NULL){
      free(this->mOutputBufferList);
      this->mOutputBufferList = NULL;
   }

   // tell PChannel (and IsOpen()) that the channel is closed.
   os_handle = -1;  
   return true;
}


unsigned PSoundChannelCoreAudio::GetChannels() const
{
  PAssert(0, PUnimplementedFunction); 
	return 0;
}

unsigned PSoundChannelCoreAudio::GetSampleRate() const
{
  PAssert(0, PUnimplementedFunction); 
	return 0;
}

unsigned PSoundChannelCoreAudio::GetSampleSize() const
{
  PAssert(0, PUnimplementedFunction); 
	return 0;
}


/*
 * Functions for retrieving AudioDevice list
 */
#include "maccoreaudio/maccoreaudio_devices.cxx"


PString PSoundChannelCoreAudio::GetDefaultDevice(Directions dir)
{
  OSStatus err = noErr;
  UInt32 theSize;
  AudioDeviceID theID;

  theSize = sizeof(AudioDeviceID);

  if (dir == Player) {
    err = AudioHardwareGetProperty(
	       kAudioHardwarePropertyDefaultOutputDevice,
               &theSize, &theID);
  }
  else {
    err =  AudioHardwareGetProperty(
	        kAudioHardwarePropertyDefaultInputDevice,
                &theSize, &theID);
  }

  if (err == 0) {
     return CADeviceName(theID);
  } else {
     return CA_DUMMY_DEVICE_NAME;
  }
}

PStringList PSoundChannelCoreAudio::GetDeviceNames(Directions dir)
{
  PStringList devices;

  int numDevices;
  AudioDeviceID *deviceList;

  numDevices = CADeviceList(&deviceList);

  for (int i = 0; i < numDevices; i++) {
    PString s = CADeviceName(deviceList[i]);
    PTRACE(1, "device name " << s );
    if (CADeviceSupportDirection(deviceList[i], dir) > 0) {
      devices.AppendString(s);
    }
  }
  devices.AppendString(CA_DUMMY_DEVICE_NAME);

  if(deviceList != NULL) {
     free(deviceList);
     deviceList = NULL;
  }

  return devices;
}

/*
 * Here start of functions responsible for converting 8kHz to 44k1Hz
 * It goes like this. The conversion is done by the AudioToolbox framework
 * of Mac CoreAudio. To start conversion this we need to install two callback 
 * functions that are called once the AudioUnit needs more data. The callback 
 * functions which are installed in SetBuffers, are: PlaybackIOProc, 
 * RecordIOProc
 *
 * Inside these callback functions an AudioConverter, which is part of the Mac 
 * AudioToolbox, is used to convert the data from 8khz to 44kHz1 or back
 * on-the-fly (same thread). The converter object needs another callback 
 * function for reading and writing the user data. The name of these
 * callback functions are: ACwriteInputProc, ACreadInputProc
 *
 * To summarize, in SetBuffer callback functions are installed that are called
 * when the AudioUnit device is ready to handle more data. Within these 
 * functions a converter object is used, that in turn calls a callback function
 * to access the data buffer filled by SoundChannel::Read 
 * or SoundChannel::Write function calls.
 */



/*
 *  Callback function called by the converter to request more data for 
 *  playback.
 *  
 *  outDataPacketDesc is unused, because all our packets have the same
 *  format and do not need individual description
 */
OSStatus PSoundChannelCoreAudio::ComplexBufferFillPlayback( 
         AudioConverterRef            inAudioConverter,
			UInt32		             *ioNumberDataPackets,
			AudioBufferList		     *ioData,
			AudioStreamPacketDescription **outDataPacketDesc,
			void			     *inUserData)
{
   OSStatus err = noErr;
   PSoundChannelCoreAudio *This = 
             static_cast<PSoundChannelCoreAudio*>(inUserData);
   AudioStreamBasicDescription pwlibASBD = This->pwlibASBD;

	if(This->mCircularBuffer->Empty()){
		PTRACE(1, This->direction << " Ringbuffer is empty. " 
				<< "Stopping output");
		*ioNumberDataPackets = 0;
		return noErr;
	}

	PTRACE(1, "Buffer empty? " << This->mCircularBuffer->Empty());
	PTRACE(1, "Buffer fill " << This->mCircularBuffer->size());

   UInt32 requestSizeBytes = *ioNumberDataPackets * pwlibASBD.mBytesPerPacket;
	PTRACE_IF(1, requestSizeBytes > This->converter_buffer_size, 
			This->direction << " Converter buffer too small");

	// doesn't matter if not everything fits into the buffer
	// the converter will ask right again
   UInt32 reqBytes = MIN( requestSizeBytes, This->converter_buffer_size );
	UInt32 reqPackets = reqBytes / pwlibASBD.mBytesPerPacket;
	reqBytes = reqPackets * pwlibASBD.mBytesPerPacket;
   
   PTRACE(1, __func__ << " requested " << *ioNumberDataPackets << 
			" packets, " <<  " fetching " << reqPackets << " packets");

   // dequeue data from circular buffer, without locking(false)
	// converter_buffer_size might not be multiple of packet
   UInt32 ioBytes = reqBytes;
   ioBytes = This->mCircularBuffer->Drain(This->converter_buffer, ioBytes,\
	                                                             false);

   if(ioBytes < reqBytes) {
      PTRACE(1, "Too few data frames, filling up with silence " 
				<< (reqBytes - ioBytes) << " bytes ");
		bzero(This->converter_buffer+ioBytes, reqBytes - ioBytes );
   }

   // fill structure that gets returned to converter
   ioData->mBuffers[0].mData = (char*)This->converter_buffer;
   ioData->mBuffers[0].mDataByteSize = reqBytes;

   *ioNumberDataPackets = reqPackets;

   return err;
}



/*
 * CoreAudio Player callback function
 */
OSStatus PSoundChannelCoreAudio::PlayRenderProc(
         void*                         inRefCon,
			AudioUnitRenderActionFlags*   ioActionFlags,
			const struct AudioTimeStamp*  TimeStamp,
			UInt32                        inBusNumber,
			UInt32                        inNumberFrames,
			struct AudioBufferList*       ioData)
{  
  OSStatus err = noErr;
  PSoundChannelCoreAudio *This = 
		  static_cast<PSoundChannelCoreAudio *>(inRefCon);

  PTRACE(1, __func__ << ", frames " << inNumberFrames);

  if( !This->isRunning || This->mCircularBuffer->Empty() ) {
	  return noErr;
  /*   AURenderCallbackStruct callback;
	  bzero(&callback, sizeof(AURenderCallbackStruct));
     err = AudioUnitSetProperty(This->mAudioUnit, 
			 		kAudioUnitProperty_SetRenderCallback,
			 		kAudioUnitScope_Input,
			 		0,
			 		&callback,
			 		sizeof(callback));
	  return noErr;*/
  }

  err = AudioConverterFillComplexBuffer(This->converter,
	        PSoundChannelCoreAudio::ComplexBufferFillPlayback, 
		     This, 
			  &inNumberFrames, // should be packets
			  ioData,
			  NULL /*outPacketDescription*/);
  checkStatus(err);


  // make fake stereo from mono 
  //ioData->mBuffers[1] = ioData->mBuffers[0];

   /* now that cpu intensive work is done, make stereo from mono
    * assume non-interleaved ==> 1 channel/buffer */
   if(ioData->mBuffers[0].mDataByteSize > 0 && This->isRunning){
      UInt32 len = ioData->mBuffers[0].mDataByteSize;
      unsigned i = 1;
      while(i < ioData->mNumberBuffers) {
    	  PTRACE(1, __func__ << " copy channel 0 to channel " << i);
         memcpy(ioData->mBuffers[i].mData, ioData->mBuffers[0].mData, len);  
         ioData->mBuffers[i].mDataByteSize = len;
    	  i++;
      }
   }

  return err;
}




OSStatus PSoundChannelCoreAudio::RecordProc(
                     void*                        inRefCon,
		     AudioUnitRenderActionFlags*  ioActionFlags,
		     const AudioTimeStamp*        inTimeStamp,
		     UInt32                       inBusNumber,
		     UInt32                       inNumberFrames,
		     AudioBufferList *            ioData)
{
   PTRACE(1,  __func__ << ", frames  " << inNumberFrames );

   OSStatus err = noErr;
   PSoundChannelCoreAudio *This =
		static_cast<PSoundChannelCoreAudio *>(inRefCon);
   CircularBuffer* inCircBuf   = This->mInputCircularBuffer;
   AudioBufferList*	inputData =  This->mInputBufferList;
   AudioBuffer *audio_buf;
   
	if(!This->isRunning){
		return noErr;
	}

   /* fetch the data from the microphone or other input device */
	err= AudioUnitRender(This->mAudioUnit,
				ioActionFlags,
				inTimeStamp, 
				inBusNumber,
				inNumberFrames, //# of frames  requested
				inputData);// Audio Buffer List to hold data    
	checkStatus(err);

   /* in any case reduce to mono by taking only the first buffer */
	audio_buf = &inputData->mBuffers[0];
	inCircBuf->Fill((char *)audio_buf->mData,	audio_buf->mDataByteSize, 
			false, true); // do not wait, overwrite oldest frames 

	/*
	 * Sample Rate Conversion(SRC)
	 */
	unsigned int frames = inCircBuf->size() / This->hwASBD.mBytesPerFrame;


	/* given the number of Microphone frames how many 8kHz frames are
	 * to expect, keeping a minimum buffer fill of 20 frames to have some 
	 * data handy in case the converter requests more than expected */
	if(frames > MIN_INPUT_FILL){
		UInt32 pullFrames = int(float(frames-MIN_INPUT_FILL)/This->rateTimes8kHz);
		UInt32 pullBytes = 
				// it is not effective to convert frames that get overriden,
				// better to override them in the mInputCircularBuffer
				MIN(UInt32(This->mCircularBuffer->capacity - 
					             This->mCircularBuffer->size()),
				   MIN( This->converter_buffer_size,
						pullFrames * This->pwlibASBD.mBytesPerFrame));

		UInt32 pullPackets = pullBytes / This->pwlibASBD.mBytesPerPacket;

		PTRACE(1, __func__ << " going to pull " << pullPackets << " packets");

		if(pullPackets > 0) {
			/* now pull the frames through the converter */
			AudioBufferList* outputData = This->mOutputBufferList;
         err = AudioConverterFillComplexBuffer(This->converter,
			        PSoundChannelCoreAudio::ComplexBufferFillRecord, 
				     This, 
					  &pullPackets, 
					  outputData, 
					  NULL /*outPacketDescription*/);
			checkStatus(err);

			/* put the converted data into the main CircularBuffer for later 
			 * fetching by the public Read function */
			audio_buf = &outputData->mBuffers[0];
			This->mCircularBuffer->Fill((char*)audio_buf->mData, 
					audio_buf->mDataByteSize, 
					false, true); // do not wait, overwrite oldest frames
		}
	}

   return err;
}

/** 
 * Callback function called by the converter to fetch more date 
 */
OSStatus PSoundChannelCoreAudio::ComplexBufferFillRecord( 
         AudioConverterRef            inAudioConverter,
			UInt32		             *ioNumberDataPackets,
			AudioBufferList		     *ioData,
			AudioStreamPacketDescription **outDataPacketDesc,
			void			     *inUserData)
{

   OSStatus err = noErr;
   PSoundChannelCoreAudio *This = 
		  static_cast<PSoundChannelCoreAudio *>(inUserData);
   CircularBuffer* inCircBuf   = This->mInputCircularBuffer;
   AudioStreamBasicDescription& hwASBD = This->hwASBD;
	

   UInt32 minPackets = MIN(*ioNumberDataPackets,
   		    inCircBuf->size() / hwASBD.mBytesPerPacket );
   
   PTRACE(1, __func__ << " " << *ioNumberDataPackets << " requested " 
   		<< " fetching " << minPackets << " packets");

	// assuming non-interleaved or mono
   UInt32 ioBytes = minPackets * hwASBD.mBytesPerPacket;

   if(ioBytes > This->converter_buffer_size){
      PTRACE(1, "converter_buffer too small " << ioBytes << " requested "
             << " but only " << This->converter_buffer_size << " fit in");
		ioBytes = This->converter_buffer_size;
   }
   
   ioBytes = inCircBuf->Drain((char*)This->converter_buffer, ioBytes, false);
   
   if(ioBytes  != minPackets * hwASBD.mBytesPerPacket) {
      PTRACE(1, "Failed to fetch the computed number of packets");
		//minPackets = ioBytes / hwASBD.mBytesPerFrame;
   }

	ioData->mBuffers[0].mData = This->converter_buffer;
	ioData->mBuffers[0].mDataByteSize = ioBytes;

	// assuming mono or non-interleaved
   *ioNumberDataPackets = ioBytes / hwASBD.mBytesPerFrame;

	return err;
	
}


OSStatus PSoundChannelCoreAudio::CallbackSetup(){
   OSStatus err = noErr;
   AURenderCallbackStruct callback;

   callback.inputProcRefCon = this;

   if (direction == Recorder) {
		callback.inputProc = RecordProc;
      /* kAudioOutputUnit stands for both Microphone/Speaker */
		err = AudioUnitSetProperty(mAudioUnit,
					kAudioOutputUnitProperty_SetInputCallback,
					kAudioUnitScope_Global,
					0,
					&callback,
					sizeof(callback));
				
   }
   else {
		callback.inputProc = PlayRenderProc;
   	err = AudioUnitSetProperty(mAudioUnit, 
			 		kAudioUnitProperty_SetRenderCallback,
			 		kAudioUnitScope_Input,
			 		0,
			 		&callback,
			 		sizeof(callback));
   }
   checkStatus(err);
   return err;
}


/* 
 * Configures the builtin converter of the HAL AudioUnit, which in contrast
 * to the InputUnit contains buffer and could do SRC already.
 * Unfortunately it is of little use because currently we do not know how 
 * to make a channel mapping so that a mono input channel gets copied to all 
 * output channels.
 * So we turn the converter off by setting the same formats on both ends.
 */ 
OSStatus PSoundChannelCoreAudio::MatchHALOutputFormat()
{
   OSStatus err = noErr;
	UInt32 size = sizeof (AudioStreamBasicDescription);

   memset(&hwASBD, 0, size);
        
   err = AudioDeviceGetProperty(mDeviceID, 
			0,     // channel
			//true,  // isInput
		   false,  // isInput
			kAudioDevicePropertyStreamFormat,
			&size, &hwASBD);
	checkStatus(err);

	/*
   //Gets the size of the Stream Format Property and if it is writable
   Boolean             outWritable;                            
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           0,  // output bus 
                           &size, 
                           &outWritable);

   PTRACE_IF(1, outWritable, "AUHAL output unit, output format is writable");

   //Get the current stream format of the output
   err = AudioUnitGetProperty (mAudioUnit,
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           0,  // output bus 
                           &hwASBD,
                           &size);
  checkStatus(err);  
  */

	PTRACE(1, direction << "before " << endl << hwASBD);

   // make sure it is non-interleaved

	BOOL isInterleaved = 
	         !(hwASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);

	PTRACE_IF(1, isInterleaved, "channels are interleaved ");

   hwASBD.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;	
	if(isInterleaved){
	   // so its only one buffer containing all data, according to 
		//list.apple.com: You only multiple out by mChannelsPerFrame 
		//if you are doing interleaved.
	   hwASBD.mBytesPerPacket /= hwASBD.mChannelsPerFrame;
	   hwASBD.mBytesPerFrame  /= hwASBD.mChannelsPerFrame;
	}
  
   //Set the stream format of the output to match the input
   err = AudioUnitSetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Input,
              0,
              &hwASBD,
              size);
                                                        

	PTRACE(1, direction << "after" << endl << hwASBD);
  
	// make sure we really know the current format
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitGetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Input,
              0,  // input bus
              &hwASBD,
              &size);
              
  return err;
}

/** hack for usb devices, we are not getting the format of the output
 * but of the input side */
OSStatus PSoundChannelCoreAudio::MatchHALOutputFormat1()
{
   OSStatus err = noErr;
	UInt32 size = sizeof (AudioStreamBasicDescription);
   memset(&hwASBD, 0, size);
   Boolean             outWritable;                            
        
   //Gets the size of the Stream Format Property and if it is writable
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Input,
                           0,  /* output bus */
                           &size, 
                           &outWritable);


   //Get the current stream format of the output
   err = AudioUnitGetProperty (mAudioUnit,
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Input,
                           0,  /* output bus */
                           &hwASBD,
                           &size);
  checkStatus(err);  


	PTRACE(1, direction << "before " << endl << hwASBD);
	
   // make sure it is non-interleaved
	Boolean isInterleaved = 
	         !(hwASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
	         //((hwASBD.mFormatID & kAudioFormatFlagIsNonInterleaved) 
				//   ^ kAudioFormatFlagIsNonInterleaved);

	PTRACE_IF(1, isInterleaved, "channels are interleaved ");

   hwASBD.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;	
	if(isInterleaved){
	   // so its only one buffer containing all data, according to 
		//list.apple.com: You only multiple out by mChannelsPerFrame 
		//if you are doing interleaved.
	   hwASBD.mBytesPerPacket /= hwASBD.mChannelsPerFrame;
	   hwASBD.mBytesPerFrame  /= hwASBD.mChannelsPerFrame;
	}
  
   //Set the stream format of the output to match the input
   err = AudioUnitSetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Input,
              0,
              &hwASBD,
              size);
                                                        
  
	// make sure we really know the current format
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitGetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Input,
              0,  // input bus
              &hwASBD,
              &size);

	PTRACE(1, direction << "before " << endl << hwASBD);
              
  return err;
}

/*
OSStatus PSoundChannelCoreAudio::FetchAUHAL_ASBD(){
   UInt32 size = sizeof (AudioStreamBasicDescription);
   OSStatus result;
        
   //Gets the size of the Stream Format Property and if it is writable
   Boolean             outWritable;                            
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           0,  // output bus 
                           &size, 
                           &outWritable);

	if(direction == Player){
      //Get the current stream input format of the output channel
      result = AudioUnitGetProperty (mAudioUnit,
                              kAudioUnitProperty_StreamFormat,
                              kAudioUnitScope_Input,
                              0,  // output bus 
                              &hwASBD,
                              &size);
	} else {
      //Get the current stream output format of the input channel
      result = AudioUnitGetProperty (mAudioUnit,
                              kAudioUnitProperty_StreamFormat,
                              kAudioUnitScope_Output,
                              1,  // output bus 
                              &hwASBD,
                              &size);
	}
   checkStatus(result);  
   return result;
}
*/


/**  hack for usb audio we are getting the format of the output port 
 * not input port */
OSStatus PSoundChannelCoreAudio::MatchHALInputFormat()
{
	OSStatus err = noErr;
	AudioStreamBasicDescription& asbd = hwASBD;
   UInt32 size = sizeof (AudioStreamBasicDescription);

	memset(&asbd, 0, size);

	PTRACE(1, "mDeviceID == " << mDeviceID);
   err = AudioDeviceGetProperty(mDeviceID, 
			0,     // channel
			true,  // isInput
			kAudioDevicePropertyStreamFormat,
			&size, 
			&asbd);
	checkStatus(err);

	
	/*
   //Gets the size of the Stream Format Property and if it is writable
	Boolean outWritable;
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           1,  // input bus 
                           &size, 
                           &outWritable);

	if(outWritable)
		PTRACE(1, "ASBD of input port of input device is writable");
	
   //Get the current stream format of the output
   err = AudioUnitGetProperty (mAudioUnit,
                        kAudioUnitProperty_StreamFormat,
                        kAudioUnitScope_Output,
                        1,  // input bus/
                        &asbd,
                        &size);
								*/

	/*
	 * make it one-channel, non-interleaved, keeping same sample rate 
	 */

	PTRACE(1, "before " << endl << asbd);

	BOOL isInterleaved = 
	         !(hwASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
	         //!(asbd.mFormatID & kAudioFormatFlagIsNonInterleaved); 

	PTRACE_IF(1, isInterleaved, "channels are interleaved ");

	// mFormatID -> assume lpcm !!!
	// set non-interleave flag 
   asbd.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;	
	if(isInterleaved){
	   // so its only one buffer containing all channels, according to 
		//list.apple.com: You only multiple out by mChannelsPerFrame 
		//if you are doing interleaved.
	   asbd.mBytesPerPacket /= asbd.mChannelsPerFrame;
	   asbd.mBytesPerFrame  /= asbd.mChannelsPerFrame;
	}
	asbd.mChannelsPerFrame = 1;
	
   // Set it to output side of input bus
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitSetProperty (mAudioUnit,
           kAudioUnitProperty_StreamFormat,
           kAudioUnitScope_Output,
           1,  // input bus
           &asbd,
           size);
	checkStatus(err);

	// make sure we really know the current format
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitGetProperty (mAudioUnit,
           kAudioUnitProperty_StreamFormat,
           kAudioUnitScope_Output,
           1,  // input bus
           &hwASBD,
           &size);


  PTRACE(1, "after" << endl <<  hwASBD);

	return err;
}


/**
 * Configure internal AudioConverter of AUHAL to make conversion
 * of interleaved to non-interleaved buffers and reduce bitsPerSample 
 */
OSStatus PSoundChannelCoreAudio::MatchHALInputFormat1()
{
	OSStatus err = noErr;
	AudioStreamBasicDescription& asbd = hwASBD;

   UInt32 size = sizeof (AudioStreamBasicDescription);
	Boolean outWritable;

	memset(&asbd, 0, size);

   //Gets the size of the Stream Format Property and if it is writable
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Input,
                           1,  /* input bus */
                           &size, 
                           &outWritable);

	if(outWritable)
		PTRACE(1, "ASBD of input port of input device is writable");
	
   //Get the current stream format of the output
   err = AudioUnitGetProperty (mAudioUnit,
                        kAudioUnitProperty_StreamFormat,
                        kAudioUnitScope_Input,
                        1,  /* input bus */
                        &asbd,
                        &size);

	/*
	 * make it one-channel, non-interleaved, keeping same sample rate 
	 */

	PTRACE(1, direction << " before " << endl << asbd);

	BOOL isInterleaved = 
	         !(hwASBD.mFormatFlags & kAudioFormatFlagIsNonInterleaved);
	         //((hwASBD.mFormatID & kAudioFormatFlagIsNonInterleaved)
	         //			^ kAudioFormatFlagIsNonInterleaved);


	PTRACE_IF(1, isInterleaved, "channels are interleaved ");

	// mFormatID -> assume lpcm !!!
	// set non-interleave flag 
   asbd.mFormatFlags |= kAudioFormatFlagIsNonInterleaved;	
	if(isInterleaved){
	   // so its only one buffer containing all channels, according to 
		//list.apple.com: You only multiple out by mChannelsPerFrame 
		//if you are doing interleaved.
	   asbd.mBytesPerPacket /= asbd.mChannelsPerFrame;
	   asbd.mBytesPerFrame  /= asbd.mChannelsPerFrame;
	}
	asbd.mChannelsPerFrame = 1;
	
   // Set it to output side of input bus
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitSetProperty (mAudioUnit,
           kAudioUnitProperty_StreamFormat,
           kAudioUnitScope_Output,
           1,  // input bus
           &asbd,
           size);
	checkStatus(err);

	// make sure we really know the current format
   size = sizeof (AudioStreamBasicDescription);
   err = AudioUnitGetProperty (mAudioUnit,
           kAudioUnitProperty_StreamFormat,
           kAudioUnitScope_Output,
           1,  // input bus
           &hwASBD,
           &size);


  PTRACE(1, direction << " after" << endl <<  hwASBD);

	return err;
}


OSStatus PSoundChannelCoreAudio::SetAudioUnitFormat2()
{
	OSStatus err = noErr;
   UInt32 size;
   Boolean             outWritable;                            
        
   //Gets the size of the Stream Format Property and if it is writable
	size = sizeof (AudioStreamBasicDescription);
   AudioUnitGetPropertyInfo(mAudioUnit,  
                           kAudioUnitProperty_StreamFormat,
                           kAudioUnitScope_Output,
                           0,  /* output bus */
                           &size, 
                           &outWritable);

	if(direction == Player){
	  size = sizeof (AudioStreamBasicDescription);
    /* err = AudioUnitSetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Input,
              0, // output bus
              &pwlibASBD,
              size);*/
	} else {
		// get the supported nominal sampling rates
		UInt32 count, numRanges;
	   err = AudioDeviceGetPropertyInfo ( mDeviceID, 
			         0, true,
			       kAudioDevicePropertyAvailableNominalSampleRates, 
					 &count, NULL );

		numRanges = count / sizeof(AudioValueRange);
 		AudioValueRange* rangeArray = (AudioValueRange*)malloc ( count );

      err = AudioDeviceGetProperty ( mDeviceID, 
				0, true, 
				kAudioDevicePropertyAvailableNominalSampleRates, 
				&count, (void*)rangeArray );
		checkStatus(err);


		PTRACE(1, "Available sample rates of input device " );
		for(UInt32 i = 0; i < numRanges; i++)
			cout << rangeArray[i] << endl;

		free(rangeArray);
				
		// set sample rate specified by SetFormat
		Float64 rate = 8000;
	   size = sizeof (Float64);
	   err =  AudioDeviceSetProperty(mDeviceID, 
				 0,  /* timestamp */
				 0,  /* channel, probably all */ 
				 true,  /* isInput */
		       kAudioDevicePropertyNominalSampleRate, size, &rate);
		checkStatus(err);
		
		AudioStreamBasicDescription inASBD;
	   size = sizeof (AudioStreamBasicDescription);
		err = AudioUnitGetProperty(mAudioUnit, 
				kAudioUnitProperty_StreamFormat, 
				kAudioUnitScope_Input, 
				1,  // input bus
				&inASBD, &size);
		checkStatus(err);
		PTRACE(1, "input side of input bus" << endl << inASBD);

      // Set output side of input bus
	   size = sizeof (AudioStreamBasicDescription);
      err = AudioUnitSetProperty (mAudioUnit,
              kAudioUnitProperty_StreamFormat,
              kAudioUnitScope_Output,
              1,  // input bus
              &pwlibASBD,
              size);
	}
                                                        
  
  return err;
}

/*
 * Functions to access input/output units/devices
 */
OSStatus PSoundChannelCoreAudio::SetupInputUnit(AudioDeviceID in)
{  
   OSStatus err = noErr;
         
   Component comp;            
   ComponentDescription desc;

   //There are several different types of Audio Units.
   //Some audio units serve as Outputs, Mixers, or DSP
   //units. See AUComponent.h for listing
   desc.componentType = kAudioUnitType_Output;

   //Every Component has a subType, which will give a clearer picture
   //of what this components function will be.
   desc.componentSubType = kAudioUnitSubType_HALOutput;

   //all Audio Units in AUComponent.h must use 
   //"kAudioUnitManufacturer_Apple" as the Manufacturer
   desc.componentManufacturer = kAudioUnitManufacturer_Apple;
   desc.componentFlags = 0;
   desc.componentFlagsMask = 0;

   //Finds a component that meets the desc spec's
   comp = FindNextComponent(NULL, &desc);
   if (comp == NULL) return kAudioCodecUnspecifiedError;

   //gains access to the services provided by the component
   err = OpenAComponent(comp, &mAudioUnit);
   checkStatus(err);

   err = EnableIO();
   checkStatus(err);

   err= SetDeviceAsCurrent(in);
   checkStatus(err);

   /*
   err = CallbackSetup();
   checkErr(err);

   err = SetupBuffers();
   checkErr(err);

   err = AudioUnitInitialize(mInputUnit);
   */
   return err;
}
	       
OSStatus PSoundChannelCoreAudio::EnableIO()
{
   OSStatus err = noErr;
   UInt32 enableIO;

   ///////////////
   //ENABLE IO (INPUT)
   //You must enable the Audio Unit (AUHAL) for input and disable output 
   //BEFORE setting the AUHAL's current device.

   //Enable input on the AUHAL
   enableIO = 1;
   err =  AudioUnitSetProperty(mAudioUnit,
	       kAudioOutputUnitProperty_EnableIO,
	       kAudioUnitScope_Input,
	       1, // input element
	       &enableIO,
	       sizeof(enableIO));
   checkStatus(err);

   //disable Output on the AUHAL
   enableIO = 0;
   err = AudioUnitSetProperty(mAudioUnit,
	      kAudioOutputUnitProperty_EnableIO,
	      kAudioUnitScope_Output,
	      0,   //output element
	      &enableIO,
	      sizeof(enableIO));
   return err;
}
	       

/*
 * Function to initialize OutputUnit and assign it to the default
 * AudioDevice. 
 */
OSStatus PSoundChannelCoreAudio::SetupOutputUnit(AudioDeviceID out){
   OSStatus err;

  //An Audio Unit is a OS component
  //The component description must be setup, then used to 
  //initialize an AudioUnit
  ComponentDescription desc;  

  desc.componentType = kAudioUnitType_Output;
  desc.componentSubType = kAudioUnitSubType_HALOutput;
  //desc.componentSubType = kAudioUnitSubType_DefaultOutput;
  desc.componentManufacturer = kAudioUnitManufacturer_Apple;
  desc.componentFlags = 0;
  desc.componentFlagsMask = 0;
  
  //Finds an component that meets the desc spec's 
  Component comp = FindNextComponent(NULL, &desc);  
  if (comp == NULL) return kAudioCodecUnspecifiedError;
    
  //gains access to the services provided by the component
  err = OpenAComponent(comp, &mAudioUnit);  
  checkStatus(err);

  err = SetDeviceAsCurrent(out);
  return err;
}

OSStatus PSoundChannelCoreAudio::SetDeviceAsCurrent(AudioDeviceID id)
{                       
   UInt32 size = sizeof(AudioDeviceID);
   OSStatus err = noErr;

   //get the default input device if device is    unknown
   if(in == kAudioDeviceUnknown) 
   {  
		if(direction == Recorder) {
         err = AudioHardwareGetProperty(
					   kAudioHardwarePropertyDefaultOutputDevice, &size, &id);
		} else {
         err = AudioHardwareGetProperty(
					kAudioHardwarePropertyDefaultInputDevice, &size, &id);
		}
      checkStatus(err);   
   }                   

   mDeviceID = id;

   //Set the Current Device to the AUHAL.
   //this should be done only after IO has been enabled on the AUHAL.
   err = AudioUnitSetProperty(mAudioUnit,
	    kAudioOutputUnitProperty_CurrentDevice,
	    kAudioUnitScope_Global,
	    0,  
	    &mDeviceID,
	    sizeof(mDeviceID));
   checkStatus(err);

   return err;
}  



/*
 * The major task of Open() is to find the matching device ID.
 *
 */
BOOL PSoundChannelCoreAudio::Open(const PString & deviceName,
				  Directions dir,
				  unsigned numChannels,
				  unsigned sampleRate,
				  unsigned bitsPerSample)
{
  OSStatus err;

  PTRACE(1, "Open " << deviceName );

  /* Save whether this is a Player or Recorder */
  this->direction = dir;

  /*
   * Init the AudioUnit and assign it to the requested AudioDevice
   */

  if (strcmp(deviceName, CA_DUMMY_DEVICE_NAME) == 0) {
	  /* Dummy device */
     PTRACE(1, "dummy device " << direction);
	  mDeviceID = kAudioDeviceUnknown;
  } else {

    AudioDeviceID deviceID = GetDeviceID(deviceName, direction == Recorder);
    if(direction == Player)
       err = SetupOutputUnit(deviceID);
    else 
       err = SetupInputUnit(deviceID);
    checkStatus(err);
  }

  os_handle = 8;  // tell PChannel (and IsOpen()) that the channel is open.
  return SetFormat(numChannels, sampleRate, bitsPerSample);
}


BOOL PSoundChannelCoreAudio::SetFormat(unsigned numChannels,
					   unsigned sampleRate,
					   unsigned bitsPerSample)
{
   PTRACE(1, "SetFormat " <<  direction );

	
  // making some assumptions about input format for now
  PAssert(sampleRate == 8000 && numChannels == 1 && bitsPerSample == 16,
	  PUnsupportedFeature);


  /*
   * Initialize the pwlibASBD
   */
  memset((void *)&pwlibASBD, 0, sizeof(AudioStreamBasicDescription)); 

  /* pwlibASBD->mReserved */
  pwlibASBD.mFormatID		     = kAudioFormatLinearPCM;
  pwlibASBD.mFormatFlags       = kLinearPCMFormatFlagIsSignedInteger;
#if PBYTE_ORDER == PBIG_ENDIAN
  pwlibASBD.mFormatFlags      |= kLinearPCMFormatFlagIsBigEndian;
#endif
  pwlibASBD.mSampleRate        = sampleRate;
  pwlibASBD.mChannelsPerFrame = numChannels;
  pwlibASBD.mBitsPerChannel   = bitsPerSample;
  pwlibASBD.mBytesPerFrame	   = numChannels * bitsPerSample / 8;
  pwlibASBD.mBytesPerPacket   =  pwlibASBD.mBytesPerFrame;
  pwlibASBD.mFramesPerPacket  =  1;


	if(mDeviceID == kAudioDeviceDummy){
	   PTRACE(1, "Dummy device");
	   return TRUE;
	}

  /* AudioOutputUnits have a builtin converter. It would be nice if we 
   * could configure it to spit out/consume the data in the format the
   * data are passed by Read/Write function calls.
	* Unfortunately this is not possible for the microphone, because this 
	* converter does not have a buffer inside, so it cannot do any Sample
	* Rate Conversion(SRC), so we would have to set the device nominal sample
	* rate itself to 8kHz. Unfortunately not all microphones can do that,
	* so this doesn't work until there are some changes by Apple. But we can
	* still use it to reduce from interleaved stereo -> non-interleaved mono */


  /*
   * Set AudioUnit input/output ports to match the ADSB of the 
	* underlying hardware device
   */
  OSStatus err;
  if(direction == Player)
     err = MatchHALOutputFormat();  
  else 
     err = MatchHALInputFormat();
  checkStatus(err);


  //err = FetchAUHAL_ASBD();
  //checkStatus(err);


  return TRUE;
}


BOOL PSoundChannelCoreAudio::IsOpen() const
{
   PTRACE(1, "IsOpen");
   /*PAssert(0, PUnimplementedFunction); */
   return (os_handle != -1);
}


int PSoundChannelCoreAudio::GetHandle() const
{
   PTRACE(1, "GetHandle");
   PAssert(0, PUnimplementedFunction);
   return os_handle; 
}

BOOL PSoundChannelCoreAudio::Abort()
{
   PTRACE(1, "Abort");
   PAssert(0, PUnimplementedFunction);
   return false;
}




/*
 * SetBuffers does most of the work before it starts playback or
 * recording.
 *
 * A device can not be used after calling Open(), SetBuffers() must
 * also be called before it can start functioning.
 *
 * size: 	Size of each buffer
 * count:	Number of buffers
 *
 */
BOOL PSoundChannelCoreAudio::SetBuffers(PINDEX bufferSize,
						PINDEX bufferCount)
{
  OSStatus err = noErr;

  PTRACE(1, __func__ << direction << " : "
		  << bufferSize << " BufferSize "<< bufferCount << " BufferCount");

	if(mDeviceID == kAudioDeviceDummy){
	   PTRACE(1, "Dummy device");
	   return TRUE;
	}

  if (mDeviceID < 0) {
    PTRACE(1, "No hardware device selected mDeviceID " << mDeviceID );
    return TRUE;  /* Works with Null device */
  }

  PAssert(bufferSize > 0 && bufferCount > 0 && bufferCount < 65536, \
	                                                    PInvalidParameter);

  /*
	* Sample Rate Conversion (SRC)
	* Create AudioConverters, input/output buffers, compute conversion rate 
	*/

  PTRACE(1, "ASBD of PwLib Audio format:" << endl << pwlibASBD );
  PTRACE(1, "ASBD of Hardware Audio format:" << endl << hwASBD);


  // how many samples has the output device compared to pwlib sample rate?
  rateTimes8kHz  = hwASBD.mSampleRate / pwlibASBD.mSampleRate;


  /*
   * Create Converter for Sample Rate conversion
   */
  if (direction == Player) 
    err = AudioConverterNew(&pwlibASBD, &hwASBD, &converter);
  else 
    err = AudioConverterNew(&hwASBD, &pwlibASBD, &converter);
  checkStatus(err);

  UInt32 quality = kAudioConverterQuality_Max;
  //kAudioConverterQuality_Low);
  //&kAudioConverterQuality_High);
  err = AudioConverterSetProperty(converter,
			      kAudioConverterSampleRateConverterQuality,
					sizeof(UInt32),
  					&quality);
  checkStatus(err);

  if(direction == Recorder){
     UInt32 primeMethod = kConverterPrimeMethod_None;
     err = AudioConverterSetProperty(converter,
                  kAudioConverterPrimeMethod,
                  sizeof(UInt32),
                  &primeMethod);
      checkStatus(err);
   }
  /** 
   * Allocate the RingBuffer for public Read/Write methods
   */
  mCircularBuffer = new CircularBuffer(bufferSize, bufferCount );


   /**
	 * In case of Recording we need a couple of buffers more 
	 */
  if(direction == Recorder){
	  SetupAdditionalRecordBuffers();
  }

  /**
   * Register callback function
   */
  err = CallbackSetup();


  /**
   * Tune the buffer size of the underlying audio device 
   */
  /*
  if (mDeviceID != NULL ) {
		if(direction == Player){
			// not implemented
			err = noErr;
		} else {
			err = AudioDeviceSetProperty( mDeviceID,
				NULL, //&ts, // I think NULL may work here too
				0,
				0,
				kAudioDevicePropertyBufferSize,
				/ *kAudioDevicePropertyBufferFrameSize,* /
				sizeof(UInt32),
				&bufferSize);
		}
		check(err);
	}
	*/


	/** 
	 * Allocate byte array passed as input to the converter 
	 */
   UInt32 bufferSizeFrames, bufferSizeBytes;
	UInt32 propertySize = sizeof(UInt32);
   err = AudioDeviceGetProperty( mDeviceID,
  	  	  	  	0,  /* output channel,  */ 
  	  	  	  	true,  /* isInput */
  	  	  	  	kAudioDevicePropertyBufferFrameSize,
				&propertySize,
  	  	  	  	&bufferSizeFrames);
   checkStatus(err);
   bufferSizeBytes = bufferSizeFrames * hwASBD.mBytesPerFrame;

   if (direction == Player) {
	   propertySize = sizeof(UInt32);
      err = AudioConverterGetProperty(converter,
				kAudioConverterPropertyCalculateInputBufferSize,
				&propertySize,
				&bufferSizeBytes);
		checkStatus(err);
		converter_buffer_size = bufferSizeBytes;
	} else {
		// on each turn the device spits out bufferSizeBytes bytes
		// the input ringbuffer has at most MIN_INPUT_FILL frames in it 
		// all other frames were converter during the last callback
		converter_buffer_size = bufferSizeBytes + 
			         2 * MIN_INPUT_FILL * hwASBD.mBytesPerFrame;
	}
	converter_buffer = (char*)malloc(converter_buffer_size);
	if(converter_buffer == NULL)
		PTRACE(1, "Failed to allocate converter_buffer");
	else
	   PTRACE(1, "Allocated converter_buffer of size " 
				<< converter_buffer_size );


  /**
   * Computes the actual buffer size used for the callback function
   *
   * The computation is based on the format of streams and the
   * requested buffer size.
   */
  /*
  UInt32 bufferByteCount;
  propertySize = sizeof(bufferByteCount);
  if (direction == Player) {
    err =
      AudioConverterGetProperty(cRef,
				kAudioConverterPropertyCalculateOutputBufferSize,
				&propertySize,
				&bufferByteCount);
  } else {
    err =
      AudioConverterGetProperty(cRef,
				kAudioConverterPropertyCalculateInputBufferSize,
				&propertySize,
				&bufferByteCount);
  }
  if (err != 0) {
    PTRACE(1, "can not compute converter buffer size " << theStatus);
    return FALSE;
  }

  PTRACE(1, "CoreAudio buffer size set to " << bufferByteCount);

  err = AudioDeviceSetProperty(caDevID,
				     0,
				     0,
				     isInput,
				     kAudioDevicePropertyBufferSize,
				     propertySize,
				     &bufferByteCount);
  if (err) {
    PTRACE(1, "set device property failed, status = (" << theStatus << ")");
    return (FALSE);
  }
  */


  /*
	* AU Setup, allocates necessary buffers... 
	*/
  err = AudioUnitInitialize(mAudioUnit);
  checkStatus(err);

  /*
   * Now the device can start working
   */
  /*err = AudioOutputUnitStart(mAudioUnit);
  checkStatus(err);
  isRunning = TRUE;*/
  isRunning = FALSE;

  return TRUE;

}

OSStatus PSoundChannelCoreAudio::SetupAdditionalRecordBuffers()
{

	OSStatus err = noErr;
	UInt32 bufferSizeFrames, bufferSizeBytes;
	
	/** 
	 * build buffer list to take over the data from the microphone 
	 */
	UInt32 propertySize = sizeof(UInt32);
	err = AudioDeviceGetProperty( mDeviceID,
		0,  /* channel, probably all */ 
		true,  /* isInput */
		kAudioDevicePropertyBufferFrameSize,
		&propertySize,
		&bufferSizeFrames);
	checkStatus(err);
	bufferSizeBytes = bufferSizeFrames * hwASBD.mBytesPerFrame;

	//calculate number of buffers from channels
	UInt32 propsize = 
		offsetof(AudioBufferList, mBuffers[hwASBD.mChannelsPerFrame]);

	//malloc buffer lists
	mInputBufferList = (AudioBufferList *)malloc(propsize);
	mInputBufferList->mNumberBuffers = hwASBD.mChannelsPerFrame;

	//pre-malloc buffers for AudioBufferLists
	for(UInt32 i =0; i< mInputBufferList->mNumberBuffers ; i++) {
		mInputBufferList->mBuffers[i].mNumberChannels = 1;
		mInputBufferList->mBuffers[i].mDataByteSize = bufferSizeBytes;
		mInputBufferList->mBuffers[i].mData = malloc(bufferSizeBytes);
	}
	mRecordInputBufferSize = bufferSizeBytes;

	/** allocate ringbuffer to cache data before passing the to the converter */
	// take only one buffer -> mono, use double buffering
	mInputCircularBuffer = new CircularBuffer(bufferSizeBytes, 2);


	/** 
	 * Build buffer list passed to Converter to return the number of requested
	 * frames 
	 */
	// given the number of input bytes how many bytes to expect at the output?
	bufferSizeBytes += MIN_INPUT_FILL * hwASBD.mBytesPerFrame;
	propertySize = sizeof(UInt32);
   err = AudioConverterGetProperty(converter,
			kAudioConverterPropertyCalculateOutputBufferSize,
			&propertySize,
			&bufferSizeBytes);
	checkStatus(err);


	//calculate number of buffers from channels
	propsize = offsetof(AudioBufferList, mBuffers[pwlibASBD.mChannelsPerFrame]);

	//malloc buffer lists
	mOutputBufferList = (AudioBufferList *)malloc(propsize);
	mOutputBufferList->mNumberBuffers = pwlibASBD.mChannelsPerFrame;

	//pre-malloc buffers for AudioBufferLists
	for(UInt32 i =0; i< mOutputBufferList->mNumberBuffers ; i++) {
		mOutputBufferList->mBuffers[i].mNumberChannels = 1;
		mOutputBufferList->mBuffers[i].mDataByteSize = bufferSizeBytes;
		mOutputBufferList->mBuffers[i].mData = malloc(bufferSizeBytes);
	}
	mRecordOutputBufferSize = bufferSizeBytes;


	return err;

}
 

BOOL PSoundChannelCoreAudio::GetBuffers(PINDEX & size,
						PINDEX & count)
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);

  return TRUE;
}

BOOL PSoundChannelCoreAudio::SetVolume(unsigned volume)
{
   OSStatus err = noErr;
   Boolean isWritable;
   bool isInput = (direction == Player ? false : true);

	if(mDeviceID == kAudioDeviceDummy){
	   PTRACE(1, "Dummy device");
      return FALSE;
	}

   // changing volume can not go through the master channel (0) 
   err = AudioDeviceGetPropertyInfo(mDeviceID, 1, isInput,
					 kAudioDevicePropertyVolumeScalar,
					 NULL, &isWritable);
   checkStatus(err);

   if ((err == kAudioHardwareNoError) && isWritable) {
      // volume is between 0 and 100 ? 
      float theValue = ((float)volume)/100.0;
      err = AudioDeviceSetProperty(mDeviceID, NULL, 1, isInput,
				       kAudioDevicePropertyVolumeScalar,
				       sizeof(float), &theValue);
   }

   if (!err)
      return TRUE;
   else
      return FALSE;
}

BOOL PSoundChannelCoreAudio::GetVolume(unsigned & volume)
{
   OSStatus err = noErr;
   UInt32 theSize;
   Float32 theValue;
   bool isInput = (direction == Player ? false : true);

	if(mDeviceID == kAudioDeviceDummy){
	   PTRACE(1, "Dummy device");
      return FALSE;
	}

   theSize = sizeof(theValue);
   // changing volume can not go through the master channel (0) 
   err = AudioDeviceGetProperty(mDeviceID, 1, isInput,
  				     kAudioDevicePropertyVolumeScalar,
 				     &theSize, &theValue);
   if (!err) {
     // volume is between 0 and 100? 
     volume = (unsigned) (theValue * 100);
     return TRUE;
   } else 
      return FALSE;
   }
  
 

BOOL PSoundChannelCoreAudio::Write(const void *buf,
				   PINDEX len)
{
   PTRACE(1, "Write called with len " << len);

  if (mDeviceID == kAudioDeviceDummy) {
     lastWriteCount =  len; 

	  // ugly, assuming interleaved or mono
	  UInt32 nr_samples = len / pwlibASBD.mBytesPerFrame; 
     usleep(UInt32(nr_samples/pwlibASBD.mSampleRate * 1000000)); // 10E-6 [s]
     return TRUE;  /* Null device */
  }


   // Write to circular buffer with locking 
   lastWriteCount = mCircularBuffer->Fill((const char*)buf, len, true);

	// Start it after putting the first data into the buffer
	// this might cause troubles in case more data are written
	// than space is available
   if(!isRunning){
      PTRACE(1, "Starting " << direction << " device.");
      OSStatus err = AudioOutputUnitStart(mAudioUnit);
      checkStatus(err);
      isRunning = TRUE;
   }

   return (TRUE);
}


BOOL PSoundChannelCoreAudio::PlaySound(const PSound & sound,
					   BOOL wait)
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);

  if (!Write((const BYTE *)sound, sound.GetSize()))
    return FALSE;

  if (wait)
    return WaitForPlayCompletion();

	return TRUE;
}

BOOL PSoundChannelCoreAudio::PlayFile(const PFilePath & file,
					  BOOL wait)
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);

  return TRUE; 
}

BOOL PSoundChannelCoreAudio::HasPlayCompleted()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::WaitForPlayCompletion()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::Read(void *buf,
				  PINDEX len)
{
   PTRACE(1, "Read called with len " << len);

   if (mDeviceID == kAudioDeviceDummy) {
      lastReadCount =  len; 
      bzero(buf, len);
  
      // ugly,  assuming interleaved or mono
      UInt32 nr_samples = len / pwlibASBD.mBytesPerFrame; 
      usleep(UInt32(nr_samples/pwlibASBD.mSampleRate * 1000000)); // 10E-6 [s]
      return TRUE;  /* Null device */
   }

   if(!isRunning){
      PTRACE(1, "Starting " << direction << " device.");
      OSStatus err = AudioOutputUnitStart(mAudioUnit);
      checkStatus(err);
      isRunning = TRUE;
   }

   lastReadCount = mCircularBuffer->Drain((char*)buf, len, true);
   return (TRUE);
}


BOOL PSoundChannelCoreAudio::RecordSound(PSound & sound)
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::RecordFile(const PFilePath & file)
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::StartRecording()
{
   PTRACE(1, __func__ );

	// if(!isOpen)
	//    PTRACE(1, "Open the device first");

   if(!isRunning){
      PTRACE(1, "Starting " << direction << " device.");
      OSStatus err = AudioOutputUnitStart(mAudioUnit);
      checkStatus(err);
      isRunning = TRUE;
   }
	return false;
}

BOOL PSoundChannelCoreAudio::isRecordBufferFull()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::AreAllRecordBuffersFull()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}

BOOL PSoundChannelCoreAudio::WaitForRecordBufferFull()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
  if (os_handle < 0) {
    return FALSE;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}

BOOL PSoundChannelCoreAudio::WaitForAllRecordBuffersFull()
{
   PTRACE(1, __func__ );
  PAssert(0, PUnimplementedFunction);
	return false;
}


// End of file
