//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
// Portions: 1998-1999, Be Incorporated
//

#ifndef _SOUNDINPUT_H
#define _SOUNDINPUT_H

#include "MediaRecorder.h"
#include "BlockFIFO.h"

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

class PSoundInput {

public:
	PSoundInput(const char* name,
		unsigned numChannels,
	    unsigned sampleRate,
	    unsigned bitsPerSample);
    
	~PSoundInput();

	bool StartRecording();
	bool StopRecording();
	bool IsRecording() { return mRecording; }
	status_t InitCheck() { return mError; }

	bool Read( void * buf, uint32 len );
private:
	// Main
	status_t mError;
	bool mRecording;

	media_node * mDevice;
	BMediaRecorder mRecorder;
	BBlockFIFO mFIFO;
	uint32 mFrameSize;

	unsigned mNumChannels;
    unsigned mSampleRate;
    unsigned mBitsPerSample;

	static void RecordHook(void * cookie, void * data, size_t size, const media_header & header);
	bool ConnectIt();
	
	// Resampler code - donated by Jon Watte
	int memoryL, memoryR, mp, mt;
	int Resample(short * in, int inSize);

	// Visualiser
	int mPeakFormat;
	float mCurMaxL;
	float mAvgMaxL;
	float mCurMaxR;
	float mAvgMaxR;
	void SetMax(float maxL, float maxR)
	{
		if (mCurMaxL < maxL) mCurMaxL = maxL;
		if (mCurMaxR < maxR) mCurMaxR = maxR;
	}
	void CalcPeakData(void *data, size_t size);
};

#endif // _SOUNDINPUT_H
