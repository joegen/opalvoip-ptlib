//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
#ifndef _SOUNDPLAYER_H
#define _SOUNDPLAYER_H

// We write for PWLib
#include <ptlib.h>

// Media kit used
#include <media/MediaDefs.h>
#include <media/MediaFile.h>
#include <media/MediaTrack.h>
#include <media/MediaRoster.h>
#include <media/TimeSource.h>

// Game kit used also 
// (Software resampler in BGameSound!)
#include <game/GameSoundDefs.h>
#include <game/StreamingGameSound.h>

PQUEUE(POutputSoundQueue, PSound);

class PSoundPlayer : public BStreamingGameSound
{
protected:
	// Sounds of music
	PSound* mpSound;
	POutputSoundQueue mSounds;			
	size_t bytesPlayed;

	int stopDelay;
	int timeToStop;
	
	bool mfDisposeBuffers;
public:
	PSoundPlayer(gs_audio_format* format, bool fDisposeBuffers = false);
	PSoundPlayer(PSound* pSound, bool fStartNow = true);
	
	// Sounds
	void AddSound(PSound* pSound);

	void Abort();
		
	virtual void FillBuffer(void * inBuffer, size_t inByteCount);
};
#endif // _SOUNDPLAYER_H

