/*
 * beaudio.cxx
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
 * Contributor(s): Yuri Kiryanov (openh323@kiryanov.com)
 *
 * $Log: beaudio.cxx,v $
 * Revision 1.6  2000/12/16 13:08:56  rogerh
 * BeOS changes from Yuri Kiryanov <openh323@kiryanov.com>
 *
 * Revision 1.5  2000/04/19 00:13:52  robertj
 * BeOS port changes.
 *
 * Revision 1.4  1999/09/21 00:56:29  robertj
 * Added more sound support for BeOS (thanks again Yuri!)
 *
 * Revision 1.3  1999/06/28 09:28:02  robertj
 * Portability issues, especially n BeOS (thanks Yuri!)
 *
 * Revision 1.2  1999/03/05 07:03:27  robertj
 * Some more BeOS port changes.
 *
 * Revision 1.1  1999/03/02 05:41:59  robertj
 * More BeOS changes
 *
 */

#pragma implementation "sound.h"

#include <ptlib.h>

// Storage kit bits
#include <storage/Directory.h>
#include <storage/Entry.h>
#include <storage/File.h>

// Medis kit bits
#include <media/MediaDefs.h>
#include <media/MediaFile.h>
#include <media/MediaTrack.h>

// Game kit bits
#include <game/GameSoundDefs.h>

// Homegrown bits
#include "beaudio/SoundInput.h"
#include "beaudio/SoundPlayer.h"

// PSound
PSound::PSound(unsigned channels,
               unsigned samplesPerSecond,
               unsigned bitsPerSample,
               PINDEX   bufferSize,
               const BYTE * buffer)
{
	// Copy format
	encoding = B_MEDIA_RAW_AUDIO; // raw
	SetFormat(channels, samplesPerSecond, DEFAULTSAMPLESIZE);
    
    // Copy data passed
    SetSize(bufferSize); 
    if( buffer )
    	memcpy(GetPointer(), buffer, bufferSize);
}


PSound::PSound(const PFilePath & filename)
{
	// Copy format
	encoding = B_MEDIA_RAW_AUDIO; // raw
	SetFormat(1, 8000, DEFAULTSAMPLESIZE);
	
	// Load the sound
    SetSize(0);
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
	formatInfo.SetSize( sizeof(gs_audio_format) ); // media format inside

	// Build our format definition
	sampleSize = bitsPerSample;
	sampleRate = samplesPerSecond;
	GAMESOUNDFORMAT(this).format = FORMATFROMBITSPERSAMPLE(bitsPerSample);
	GAMESOUNDFORMAT(this).channel_count = numChannels = channels;
	GAMESOUNDFORMAT(this).frame_rate = 1.0 * sampleRate;
	GAMESOUNDFORMAT(this).byte_order = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
	GAMESOUNDFORMAT(this).buffer_size = 2048; // 1K default buffer size
}

BOOL PSound::Load(const PFilePath & filename)
{
	entry_ref ref; 
	BEntry entry(filename, true); 
	
	PError << "PSound::Load from filename" << endl;
	status_t err = entry.GetRef(&ref);
	if ( err == B_OK)
	{
		media_format fmt;
		BMediaFile mediaFile(&ref);
		BMediaTrack * trk = mediaFile.TrackAt(0); // only first track
		err = trk->DecodedFormat(&fmt);

		// Frames
		int64 numFrames = trk->CountFrames();
		int32 frameSize = (fmt.u.raw_audio.format&15) * fmt.u.raw_audio.channel_count;
		if ( (frameSize * numFrames) > MAX_SOUND_FILE_SIZE )
			err = -1;
		else
		{	
			int64 framesRead = 0;
 
	 		// Format
			SetFormat(fmt.u.raw_audio.channel_count, 
				fmt.u.raw_audio.frame_rate, 
				BITSPERSAMPLEFROMFORMAT(fmt.u.raw_audio.format));
			
			// Read
			SetSize(frameSize * numFrames);
			
			PMutex readLock;
			readLock.Wait();

			char* dest = (char*) GetPointer();
			int64 frameCount = numFrames;
			while ( frameCount )
			{
				err = trk->ReadFrames(dest, &framesRead);
				dest += framesRead * frameSize;
				frameCount -= framesRead;
			}
			readLock.Signal();
		}
	}  
	return err == B_OK;
}


BOOL PSound::Save(const PFilePath & filename)
{
    PTRACE(1, "PSound::Save(" << filename << ") " <<  numChannels << "," << sampleRate << "," << sampleSize);
	
	PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
	PAssert( sampleSize == 0 ||
		sampleSize == 8 || 
		sampleSize == 16 ||
		sampleSize == 32, PInvalidParameter);

	if( !sampleSize )
		PError << "\tWarning: sample bits parameter is zero. Float?" << endl;

	status_t err = B_OK;
	entry_ref ref;
	media_file_format ff;
	int32 cookie = 0;
	while ((err = get_next_file_format(&cookie, &ff)) == B_OK) {
		if (!strcasecmp(ff.file_extension, "wav")) {
			PTRACE(1, "chose WAV file format" << endl );
			break;
		}
	}

	BDirectory dir((const char*)filename.GetDirectory());
	BEntry ent;
	BFile file;
	if ((err = dir.CreateFile((const char*)filename.GetFileName(), &file)) < B_OK) {
		PTRACE(1, "cannot create output file" << endl );
		return FALSE;
	}
	if (ff.mime_type[0] != 0) {
		file.RemoveAttr("BEOS:TYPE");
		file.WriteAttr("BEOS:TYPE", 'MIMS', 0, ff.mime_type, strlen(ff.mime_type));
	}
	if ((err = dir.FindEntry((const char*)filename.GetFileName(), &ent)) < B_OK) {
		PTRACE(1, "cannot get entry for output file" << endl);
		return FALSE; 
	}
	ent.GetRef(&ref);

	if (err != B_OK) {
		PTRACE(1, "no file format found\n" << endl);
		return FALSE;
	}
	BMediaFile mFile(&ref, &ff);
	if ((err = mFile.InitCheck()) < B_OK) {
		PTRACE(1, "could not create BMediaFile" << endl);
		return FALSE;
	}

	media_format fmt;
	fmt.type = B_MEDIA_RAW_AUDIO;
	fmt.u.raw_audio.format = FORMATFROMBITSPERSAMPLE(sampleSize);
	fmt.u.raw_audio.channel_count = numChannels;
	fmt.u.raw_audio.frame_rate = 1.0 * sampleRate;
	fmt.u.raw_audio.byte_order = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
	fmt.u.raw_audio.buffer_size = 2048; 

	char str[200];
	string_for_format(fmt, str, 200);
	fprintf(stderr, "format: %s (%g;0x%lx;%ld;%ld;%ld)\n", str, fmt.u.raw_audio.frame_rate,
			fmt.u.raw_audio.format, fmt.u.raw_audio.channel_count, fmt.u.raw_audio.byte_order,
			fmt.u.raw_audio.buffer_size);

	BMediaTrack* pmTrack = NULL;
	if (0 == (pmTrack = mFile.CreateTrack(&fmt))) {
		PTRACE(1, "could not add track to file" << endl);
		return FALSE;
	}
	
	err = mFile.CommitHeader();
	if (err < B_OK) {
		PTRACE(1, "CommitHeader() failed" << endl);
		pmTrack = 0;
		return FALSE;
	}

	if (0 > pmTrack->WriteFrames(GetPointer(), GetSize(), B_MEDIA_KEY_FRAME)) {
		PTRACE(1, "buffer_flusher couldn't write data; bailing" << endl);
	}

	// flush file
	err = mFile.CloseFile();
	if (err < B_OK) {
		PTRACE(1, "CloseFile() failed" << endl);
	}

	return B_OK;
}


PSoundChannel::PSoundChannel() :
	mpInput(NULL), mpOutput(NULL), 
	mNumChannels(1), mSampleRate(8000), mBitsPerSample(16)
{
  Construct();
}


PSoundChannel::PSoundChannel(const PString & device,
                             Directions dir,
                             unsigned numChannels,
                             unsigned sampleRate,
                             unsigned bitsPerSample) :
	mpInput(NULL), mpOutput(NULL), 
	mNumChannels(numChannels), mSampleRate(sampleRate), mBitsPerSample(bitsPerSample)
{
  Construct();
  
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}


void PSoundChannel::Construct()
{
}


PSoundChannel::~PSoundChannel()
{
	Close();
  
	if( direction == Recorder && mpInput )
		delete mpInput;
}


PStringArray PSoundChannel::GetDeviceNames(Directions /*dir*/)
{
  PStringArray array;

  return array;
}


PString PSoundChannel::GetDefaultDevice(Directions /*dir*/)
{
  return "/dev/audio";
}


BOOL PSoundChannel::Open(const PString & dev,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
    Close();
	
	device = dev;
	direction = dir;
	
  	SetFormat(numChannels, sampleRate, bitsPerSample);

 	if( direction == Player )
	{
		mpOutput = new PSoundPlayer(device);
		if( !mpOutput )
			return FALSE;		
  		
  		mpOutput->SetFormat(mNumChannels, mSampleRate, mBitsPerSample);
   
   		bool fStarted = mpOutput->StartPlayer();
  		return fStarted ? TRUE : FALSE;
	}
	else
 	if( direction == Recorder )
	{
		mpInput = new PSoundInput(device, mNumChannels, mSampleRate, mBitsPerSample);

  		// Can't yet set format for input. Resampler?
		// Recorder initialised
    	if( !mpInput )
			return FALSE;		
    	
  		bool fStarted = mpInput->StartRecording(); 
  		return fStarted ? TRUE : FALSE;
 	}
   		
	// No more channel types 
   	return FALSE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
    PTRACE(1, "PSoundChannel::SetFormat: " <<  numChannels << "," << sampleRate << "," << bitsPerSample);
	
	PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
	PAssert(	bitsPerSample == 0 ||
		bitsPerSample == 8 || 
		bitsPerSample == 16 ||
		bitsPerSample == 32, PInvalidParameter);

	if( !bitsPerSample )
		PError << "\tWarning: sample bits parameter is zero. Float?" << endl;
	
	mNumChannels = numChannels;
	mSampleRate = sampleRate;
	mBitsPerSample = bitsPerSample;
	
  	if( direction == Player )
	{
		if( !mpOutput )
			return FALSE;		

		bool fFormatSet = mpOutput->SetFormat(mNumChannels, mSampleRate, mBitsPerSample);
		return fFormatSet ? TRUE : FALSE;
	}
	else
	if( direction == Recorder )
	{
		if( !mpInput )
			return FALSE;

		return TRUE;
	}
	
	return FALSE;
}

BOOL PSoundChannel::Read( void * buf, PINDEX len)
{
  	if( direction == Player )
	{
		return FALSE;
	}
	else
	if( direction == Recorder )
	{
		if( !mpInput )
			return FALSE;
		
		bool fRead = mpInput->Read(buf, len);
		return fRead? TRUE : FALSE;
	}

	return FALSE;
}

BOOL PSoundChannel::Write( const void * buf, PINDEX len)
{
  	if( direction == Player )
	{
		PAssertNULL(mpOutput);
		
		mpOutput->Play(buf, len);
		return mpOutput->IsPlaying()? TRUE : FALSE;
	}
	else
	if( direction == Recorder )
	{
		return FALSE;
	}

	return FALSE;
}

BOOL PSoundChannel::Close()
{
  	if( direction == Player )
	{
		if ( mpOutput )
		 	mpOutput->StopPlayer();
		
	 	isInitialised = false;
		return TRUE;
		
	}
	else
	if( direction == Recorder )
	{
		if ( mpInput )
		 	mpInput->StopRecording();
		
	 	isInitialised = false;
		return TRUE;
	}

	return FALSE;
}

BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  return TRUE;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  return TRUE;
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
	// Set buffer frame count to 20ms.
	PSoundPlayer NewSoundPlayer("TempOutput");
	NewSoundPlayer.SetFormat(sound.GetChannels(), sound.GetSampleRate(), sound.GetSampleSize(), sound.GetSize());
	
	status_t err = NewSoundPlayer.StartPlayer();
	while( wait && NewSoundPlayer.IsPlaying() )
		PXSetIOBlock(PXReadBlock, readTimeout); 

  return err == B_OK;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  return FALSE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
	PAssertNULL(mpOutput);
			
	return mpOutput->IsPlaying()? FALSE : TRUE;
}


BOOL PSoundChannel::WaitForPlayCompletion()
{
  return FALSE;
}


BOOL PSoundChannel::RecordSound(PSound & sound)
{
  return FALSE;
}


BOOL PSoundChannel::RecordFile(const PFilePath & filename)
{
  return FALSE;
}


BOOL PSoundChannel::StartRecording()
{
  	if( direction == Player )
	{
		return FALSE;
	}
	else
	if( direction == Recorder )
	{
		PAssertNULL(mpInput);

		bool isRecording = mpInput->StartRecording();
		
		if( isRecording )
			PError << "Recording started" << endl;
		return isRecording? TRUE : FALSE;
	}

	return FALSE;
}


BOOL PSoundChannel::IsRecordBufferFull()
{
  return FALSE;
}


BOOL PSoundChannel::AreAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannel::WaitForRecordBufferFull()
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  return PXSetIOBlock(PXReadBlock, readTimeout);
}


BOOL PSoundChannel::WaitForAllRecordBuffersFull()
{
  return FALSE;
}


BOOL PSoundChannel::Abort()
{
  	if( direction == Player )
	{
		if( !mpOutput )
			return FALSE;
			
		return mpOutput->StopPlayer()? TRUE : FALSE;
	}
  	else
  	if( direction == Recorder )
	{
		if(!mpInput)
			return FALSE;
			
		return mpInput->StopRecording()? TRUE : FALSE;
	}
	
	return FALSE;
}


// End of file

