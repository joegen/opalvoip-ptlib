//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
#include "SoundPlayer.h"
#define MAX_SOUND_FILE_SIZE (3 * 1024 * 1024)

// PSoundPlayer
PSoundPlayer::PSoundPlayer(gs_audio_format* format, bool fDisposeBuffers = false) :
	BStreamingGameSound( 
		format->frame_rate * 0.2, // 20 ms
		format ),
	mpSound(NULL), 
	bytesPlayed(0), 
	stopDelay( 0 ), 
	timeToStop(0),
	mfDisposeBuffers(fDisposeBuffers)
{ };

PSoundPlayer::PSoundPlayer(PSound* pSound, bool fStartNow = true) :
	BStreamingGameSound( 
		((gs_audio_format*) pSound->GetFormatInfoData())->frame_rate * 0.2, // 20 ms
		((gs_audio_format*) pSound->GetFormatInfoData()) ),
	mpSound(NULL), bytesPlayed(0), stopDelay( 0 ), timeToStop(0)
	{ 
		AddSound(pSound);
		if( fStartNow ) StartPlaying(); 
	};

// Sounds
void PSoundPlayer::AddSound(PSound* pSound)
{
	Lock();
	mSounds.Enqueue(pSound);
	
	if ( (pSound->GetSize() * mSounds.GetSize()) > MAX_SOUND_FILE_SIZE )
	{
		delete mSounds.Dequeue();
#if EXT_DEBUG
		PError << "Output buffer overflow!" << endl;
#endif
	}
	Unlock();
}

void PSoundPlayer::Abort()
{
	if ( IsPlaying() )
		StopPlaying();	

	Lock();
	while ( mSounds.GetSize() )
		mpSound = mSounds.Dequeue();

	mpSound = NULL;
	Unlock();
		
	bytesPlayed = 0;
	timeToStop = stopDelay;
}
	
void PSoundPlayer::FillBuffer(void * inBuffer, size_t inByteCount)
{
	size_t size = 0;
	BYTE* data = NULL ;
	
	Lock();
	if( mpSound ) 
	{
		size = mpSound->GetSize();
		data = mpSound->GetPointer() ;
	}
	Unlock();
	
	if( bytesPlayed >= size )
	{
		bytesPlayed = 0;
		if( mfDisposeBuffers )
			delete mpSound;

		mpSound = NULL;
		
		if( !mSounds.GetSize() )
			mpSound = NULL; // Finita la comedia
		else
		{
			Lock();
			mpSound = mSounds.Dequeue();

			// New sound to play
			if( mpSound )
			{
				size = mpSound->GetSize();
				data = mpSound->GetPointer();
			}
			Unlock();
		}
		
		if( !mpSound )
		{ 
			if( !timeToStop || !stopDelay )
			{
				StopPlaying();
				return ;
			}
			else
			{
				PError << "Time to stop: " << timeToStop << endl;
				timeToStop--;
			}
		}
		else
		{
			timeToStop = stopDelay;
		}

	}

	if( (int) inByteCount <= 0 )
		return;

	if( size && data )
	{
		Lock();
		memcpy( inBuffer, data + bytesPlayed, inByteCount);
		bytesPlayed += inByteCount;
		if( bytesPlayed > size )
			;// TODO: fill rest of buffer
		Unlock();
	}
}

