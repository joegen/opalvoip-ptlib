//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
// Portions: 1998-1999, Be Incorporated
//

#ifndef _SOUNDINPUT_H
#define _SOUNDINPUT_H

// Media kit bits
#include <media/MediaDefs.h>
//#include <media/MediaRoster.h>
#include <media/TimeSource.h>

// Sound Capture example used
#include "SoundConsumer.h"

// BlockFIFO example used
#include "BlockFIFO.h"

class PSoundInput : public SoundConsumer {

	// Media kit voodoo
	BMediaRoster * m_roster;
	media_node m_audioInputNode;
	media_output m_audioOutput;
	media_input m_recInput;

	// Err
	status_t mError;

	// Recording flag
	bool mfRecording;

	// FIFO!
	BBlockFIFO mFIFO;
	
	// Resampler stuff
	int memoryL, memoryR, mp, mt;
	
	// Resampler code - donated by Jon Watte
	int Resample(short * in, int inSize);
	void Notify(int32 code, ...);
	void Record(bigtime_t /* time */,
		const void * data, size_t size,
		const media_raw_audio_format & fmt );
public:
	PSoundInput(const char* name, size_t bufSize = 4096);
	bool StartRecording();
	bool StopRecording();
	bool IsRecording() { return mfRecording; }
	status_t InitCheck() { return mError; }

	~PSoundInput();

	bool Read( void * buf, uint32 len );
	
	static PSoundInput* CreateSoundInput(const char* name);
	static void ReleaseSoundInput(PSoundInput* input);
};

#endif // _SOUNDINPUT_H