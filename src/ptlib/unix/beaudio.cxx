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
 * Contributor(s): Yuri Kiryanov (yk@altavista.net)
 *
 * $Log: beaudio.cxx,v $
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

// Storage kit used
#include <storage/File.h>

// Streaming sound player
#include "beaudio/SoundPlayer.h"

// Audio input consumer
#include "beaudio/SoundInput.h"

// Format convertors
#define FORMATFROMBITSPERSAMPLE(bps) (uint32) ( \
		(bps) == 8*sizeof(unsigned char) ? gs_audio_format::B_GS_U8 : \
		(bps) == 8*sizeof(short) ? gs_audio_format::B_GS_S16 : \
		(bps) == 8*sizeof(int) ? gs_audio_format::B_GS_S32 : \
		(bps) == 0 ? gs_audio_format::B_GS_F : gs_audio_format::B_GS_U8 )

#define BITSPERSAMPLEFROMFORMAT(fmt) (unsigned) ( (fmt & 0xf)*8 )
#define DEFAULTSAMPLESIZE BITSPERSAMPLEFROMFORMAT(gs_audio_format::B_GS_U8)
#define MAX_SOUND_FILE_SIZE (3 * 1024 * 1024)

#define GAMESOUNDFORMAT(ps) (*(gs_audio_format*)(ps)->formatInfo.GetPointer())

static const uint32 g_byteOrder = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
static const gs_audio_format g_defaultFmt =  { 44100, 1, 0x11, g_byteOrder, 1024 };

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
	struct formChunk { int32 name ; int32 size; int32 type; } form = { 'MROF', GetSize(), 'FFIA' }; 
	struct aifffmt { uint16 channels; uint32 frames; uint16 bps; float fps; int32 compression; }
		fmt = { numChannels, 0, sampleSize, 1.0 * sampleRate, 0 };
	struct commChunk { int32 comm; aifffmt format; } comm = { 'MMOC', fmt }; 
	struct ssndChunk { int32 name ; int32 size;} ssnd = { 'DNSS', 0 }; 
	const char* aiffFile = "audio/x-aiff"; 

	BYTE aiffHeader[0x35];
	BYTE* pHdr = aiffHeader;
	memset(pHdr, 0, 0x35);
	memcpy(pHdr, &form, sizeof(form) );
	pHdr += sizeof(form);
	memcpy(pHdr, &comm, sizeof(comm));
	pHdr += sizeof(comm);
	memcpy( pHdr, &ssnd, sizeof(ssnd));
	
	status_t err = B_OK;
	if( err == B_OK )
	{
		BFile file(filename, B_CREATE_FILE|B_ERASE_FILE|B_WRITE_ONLY);
		
		err = file.Write( aiffHeader, 0x35 );
		err = file.Write( GetPointer(), GetSize() );
	}
	
	if( err == B_OK )
	{
		BNode node(filename);
		err = node.WriteAttr("BEOS:TYPE", 'MIMS', 0, aiffFile, strlen(aiffFile)+1);
	}
	

  return err == B_OK;
}


///////////////////////////////////////////////////////////////////////////////
class PBufferPlayer : public PSoundPlayer
{
	gs_audio_format fmt;
public:
	PBufferPlayer() : 
         PSoundPlayer( &fmt, true ), 
         	fmt(g_defaultFmt)
	{     	
    }		

	void SetFormat(unsigned numChannels,
                   unsigned sampleRate,
                   unsigned bitsPerSample)
	{
		Abort();
		
		PAssert(!IsPlaying(), PInvalidParameter);
		PAssert(bytesPlayed == 0, PInvalidParameter);
		
		fmt.frame_rate = 1.0 * sampleRate; 
		fmt.channel_count =	numChannels; 
		fmt.format = FORMATFROMBITSPERSAMPLE(bitsPerSample), 
		fmt.byte_order = g_byteOrder;
		fmt.buffer_size = 1024;
		SetParameters(fmt.frame_rate * 0.2, &fmt, 2);
		
		stopDelay = 100;
	}

	void AddBuffer(const void * buf, PINDEX len)
	{
		AddSound(new PSound(fmt.channel_count, 
					fmt.frame_rate, 
						BITSPERSAMPLEFROMFORMAT(fmt.format), 
							(PINDEX)len, (BYTE*) buf ));
		if( !IsPlaying() )
				StartPlaying();
	}
};

// Output
static PBufferPlayer* g_pSoundOutput = NULL;

// Input
static PSoundInput* g_pSoundInput = NULL;

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
	isInitialised = false;
	
	if( !g_pSoundOutput )
		g_pSoundOutput = new PBufferPlayer;

	if( !g_pSoundInput )
		g_pSoundInput = new PSoundInput(this);
}


PSoundChannel::~PSoundChannel()
{
  Close();

   if( g_pSoundInput )
   {
		delete g_pSoundInput;
		g_pSoundInput = NULL;
   }

   if( g_pSoundOutput )
   {
		delete g_pSoundOutput;
		g_pSoundOutput = NULL;
   }
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


BOOL PSoundChannel::Open(const PString & device,
                         Directions dir,
                         unsigned numChannels,
                         unsigned sampleRate,
                         unsigned bitsPerSample)
{
    Close();
	
// 	if( dir == Player )
//	{
		PAssertNULL(g_pSoundOutput);
		
  		SetFormat(numChannels, sampleRate, bitsPerSample);
  		g_pSoundOutput->StartPlaying();
//	}
//	else
//	{
		
// 	}

//  return FALSE;
   return TRUE;
}


BOOL PSoundChannel::SetFormat(unsigned numChannels,
                              unsigned sampleRate,
                              unsigned bitsPerSample)
{
  PError << "PSoundChannel::SetFormat" <<  endl;
  
  Abort();

  PAssert(numChannels >= 1 && numChannels <= 2, PInvalidParameter);
  PAssert(	bitsPerSample == 0 ||
 			bitsPerSample == 8 || 
 			bitsPerSample == 16 ||
 			bitsPerSample == 32, PInvalidParameter);
  if( !bitsPerSample )
  	PError << "\tWarning: sample bits parameter is zero. Float?" << endl;

  PAssertNULL(g_pSoundOutput);
  g_pSoundOutput->SetFormat(numChannels, sampleRate, bitsPerSample);

  return FALSE;
}

BOOL PSoundChannel::Read( void * buf, PINDEX len)
{
  return g_pSoundInput->GetBuffer(buf, len);
}

BOOL PSoundChannel::Write( const void * buf, PINDEX len)
{
  //PError << "PSoundChannel::Write" << endl;
  g_pSoundOutput->AddBuffer(buf, len);
  return g_pSoundOutput->IsPlaying();
}

BOOL PSoundChannel::Close()
{
  Abort();
  
  isInitialised = false;
  return TRUE;
}

BOOL PSoundChannel::SetBuffers(PINDEX size, PINDEX count)
{
  Abort();

  PAssert(size > 0 && count > 0 && count < 65536, PInvalidParameter);

  return FALSE;
}


BOOL PSoundChannel::GetBuffers(PINDEX & size, PINDEX & count)
{
  return TRUE;
}


BOOL PSoundChannel::PlaySound(const PSound & sound, BOOL wait)
{
  Abort();

//  if (!Write((const BYTE *)sound, sound.GetSize()))
//    return FALSE;

//  if (wait)
//  return WaitForPlayCompletion();

	// Set buffer frame count to 20ms.
	PSoundPlayer player((PSound*) &sound);
	
	status_t err = player.StartPlaying();
	while( wait && player.IsPlaying() )
		PXSetIOBlock(PXReadBlock, readTimeout); 

  return err == B_OK;
}


BOOL PSoundChannel::PlayFile(const PFilePath & filename, BOOL wait)
{
  return FALSE;
}


BOOL PSoundChannel::HasPlayCompleted()
{
  return g_pSoundOutput->IsPlaying()? FALSE : TRUE;
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
	PAssertNULL(g_pSoundInput);
	bool isRecording = g_pSoundInput->StartRecording();
	
	if( isRecording )
		PError << "Recording started" << endl;

  return isRecording? TRUE : FALSE;
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
  PAssertNULL(g_pSoundOutput);
  g_pSoundOutput->Abort();
  
  return FALSE;
}


// End of file

