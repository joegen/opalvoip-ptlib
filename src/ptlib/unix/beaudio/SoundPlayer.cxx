//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
#include "SoundPlayer.h"
#include <string.h>

#include "stdio.h"

//#define EXT_DEBUG

// PRawBuffer
const uint32 PSoundPlayer::g_byteOrder = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
const gs_audio_format PSoundPlayer::g_defaultFmt =  { 8000, 2, gs_audio_format::B_GS_S16, g_byteOrder, 2048 };

// PSoundPlayer
PSoundPlayer::PSoundPlayer(const char* name, const gs_audio_format* format, int32 bufSize) :
	BStreamingGameSound( 
		format->frame_rate * 0.2, // 20 ms
		format ),
	mFmt(*format),
	mFIFO( mFmt.buffer_size, 1, 3, B_ANY_ADDRESS, 0, name ) // 3 buffers
{ 
	SetParameters(mFmt.frame_rate * 0.2, &mFmt, 2);
}

PSoundPlayer::~PSoundPlayer() 
{ 
	status_t status;
	mFIFO.CopyNextBufferIn(&status, 0, B_INFINITE_TIMEOUT, true);
}

bool PSoundPlayer::StartPlayer()
{
	mFIFO.Reset();
	return (BStreamingGameSound::StartPlaying() == B_OK);
}

bool PSoundPlayer::StopPlayer()
{
	status_t status = BStreamingGameSound::StopPlaying();

	mFIFO.Reset();
	return (status == B_OK);
}

bool PSoundPlayer::Play(const void * buf, size_t len)
{
	if (mFIFO.SizeAvailableToPut() == 0 )
	{
#ifdef EXT_DEBUG
		fprintf(stderr, "PSoundPlayer::Play: FIFO is empty: %ld\n", mFIFO.SizeAvailableToPut());
#endif
		return false;
	}

	status_t result = mFIFO.CopyNextBufferIn(buf, len, B_INFINITE_TIMEOUT, false);
	if (result < (int32)len) 
	{
#ifdef EXT_DEBUG
		fprintf(stderr, "PSoundPlayer::Play: CopyNextBufferIn(%p, %ld, 50*1000*1000, false) failed with %ld.\n",
			buf, len, result);
#endif
		return false;
	}

	return true;
}

void PSoundPlayer::FillBuffer(void * buf, size_t len)
{
	status_t result = mFIFO.CopyNextBlockOut(buf, len, B_INFINITE_TIMEOUT);

	if (result < (int32) len ) {

		fprintf(stderr, "PSoundPlayer::FillBuffer: CopyNextBlockOut(%p, %ld, B_INFINITE_TIMEOUT, false) failed with %ld.\n",
			buf, len, result);

		return;
	}

	return;
}

bool PSoundPlayer::SetFormat(unsigned numChannels,
                  unsigned sampleRate,
                  unsigned bitsPerSample,
				  unsigned bufSize )
{
	bool fPlaying = IsPlaying();
	StopPlaying();

	mFmt.frame_rate = 1.0 * sampleRate; 
	mFmt.channel_count = numChannels; 
	mFmt.format = FORMATFROMBITSPERSAMPLE(bitsPerSample), 
	mFmt.byte_order = g_byteOrder;
	
	mFmt.buffer_size = bufSize;

	fprintf(stderr, "PSoundPlayer::SetFormat: Channels: %d, rate: %d, bits: %d, bufsize: %d.\n", 
		 numChannels, sampleRate, bitsPerSample, bufSize );

	status_t err = SetParameters(mFmt.frame_rate * 0.2, &mFmt, 2);
	if (err != B_OK ) {
		fprintf(stderr, "Error while set format: %s.\n", 
			strerror(err));
	}

	if( fPlaying )
		return StartPlaying();
	return true;
}

