//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
// Portions Be, Inc.
//

#ifndef _SOUNDINPUT_H
#define _SOUNDINPUT_H

// We write for PWLib
#include <ptlib.h>

// Media kit bits
#include <media/MediaRoster.h>
#include <media/TimeSource.h>

// Sound Capture example used
#include "SoundConsumer.h"

// Resampler needed
#include "Resampler.h"

PQUEUE(PInputSoundQueue, PSound);

class PSoundInput : public SoundConsumer {

	// Media kit voodoo
	BMediaRoster * m_roster;
	media_node m_audioInputNode;
	media_output m_audioOutput;
	media_input m_recInput;

	// Our client
	PSoundChannel* mpChannel;
	
	// Sounds
	PInputSoundQueue mSounds;
	PSound* mpSound;
	size_t bytesToWrite;
	
	// Input is always 44100(R4.5), output is usually 8000 
	PResampler mResampler;
	
	// Err
	status_t mError;
public:
	PSoundInput(PSoundChannel* pChannel);
	bool StartRecording();
	bool StopRecording();

	~PSoundInput();

	void PSoundInput::Notify(int32 code, ...);
	void PSoundInput::Record(bigtime_t /* time */,
		const void * data, size_t size,
		const media_raw_audio_format & fmt );

	bool GetBuffer( void * buf, PINDEX len, bool fRelease = true );
};

#endif // _SOUNDINPUT_H