//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
#ifndef _SOUNDPLAYER_H
#define _SOUNDPLAYER_H

// Media kit used
#include <media/MediaDefs.h>
#include <media/MediaFile.h>
#include <media/MediaTrack.h>
#include <media/TimeSource.h>
#include <media/Buffer.h>
#include <media/BufferGroup.h>

// Game kit used also 
// (Software resampler in BGameSound!)
#include <game/GameSoundDefs.h>
#include <game/StreamingGameSound.h>

// FIFO!
#include "BlockFIFO.h"

// Format convertors
#define FORMATFROMBITSPERSAMPLE(bps) (uint32) ( \
		(bps) == 8*sizeof(unsigned char) ? gs_audio_format::B_GS_U8 : \
		(bps) == 8*sizeof(short) ? gs_audio_format::B_GS_S16 : \
		(bps) == 8*sizeof(int) ? gs_audio_format::B_GS_S32 : \
		(bps) == 0 ? gs_audio_format::B_GS_F : gs_audio_format::B_GS_U8 )

#define BITSPERSAMPLEFROMFORMAT(fmt) (unsigned) ( (fmt & 0xf)*8 )
#define DEFAULTSAMPLESIZE BITSPERSAMPLEFROMFORMAT(gs_audio_format::B_GS_U8)

class PSoundPlayer : public BStreamingGameSound
{
protected:
	gs_audio_format mFmt;

	BBlockFIFO mFIFO;
	
	virtual void FillBuffer(void * inBuffer, size_t inByteCount);

public:
	static const uint32 g_byteOrder;
	static const gs_audio_format g_defaultFmt;

	PSoundPlayer(const char* name, const gs_audio_format* format = &g_defaultFmt, 
		int32 bufSize = g_defaultFmt.buffer_size * (g_defaultFmt.format & 0xf) );

	~PSoundPlayer();
	
	bool StartPlayer();
	bool StopPlayer();

	void SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample, 
			unsigned bufSize = g_defaultFmt.buffer_size * (g_defaultFmt.format & 0xf) );
			
	bool Play(const void * buf, size_t size);
};

#endif // _SOUNDPLAYER_H

