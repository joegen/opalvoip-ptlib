//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
#include "SoundPlayer.h"
#include <stdio.h>
#include <string.h>

//#define EXT_DEBUG

// PRawBuffer
const uint32 PSoundPlayer::g_byteOrder = (B_HOST_IS_BENDIAN) ? B_MEDIA_BIG_ENDIAN : B_MEDIA_LITTLE_ENDIAN;
const gs_audio_format PSoundPlayer::g_defaultFmt =  { 44100, 2, gs_audio_format::B_GS_S16, g_byteOrder, 1024 };

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

bool PSoundPlayer::Play(const void * buf, size_t size)
{
	status_t err = mFIFO.CopyNextBufferIn(buf, size, B_INFINITE_TIMEOUT, false);
	if (err < (int32)size) {
		fprintf(stderr, "Error while PSoundPlayer::Play: %s; bailing\n", strerror(err));

		fprintf(stderr, "\tCopyNextBufferIn(%p, %ld, B_INFINITE_TIMEOUT, false) failed with %ld.\n",
			buf, size, err);
		return false;
	}

	return true;
}

void PSoundPlayer::FillBuffer(void * inBuffer, size_t inByteCount)
{
	status_t err = mFIFO.CopyNextBlockOut(inBuffer, inByteCount, B_INFINITE_TIMEOUT);

	if (err < (int32) inByteCount ) {
		fprintf(stderr, "Error while PSoundPlayer::FillBuffer: %s; bailing\n", strerror(err));

		fprintf(stderr, "\tCopyNextBlockOut(%p, %ld, B_INFINITE_TIMEOUT) failed with %ld.\n",
			inBuffer, inByteCount, err);
	}
}

void PSoundPlayer::SetFormat(unsigned numChannels,
                  unsigned sampleRate,
                  unsigned bitsPerSample,
				  unsigned bufSize )
{
	bool fWasPlaying = IsPlaying();
	StopPlaying();

	mFmt.frame_rate = 1.0 * sampleRate; 
	mFmt.channel_count = numChannels; 
	mFmt.format = FORMATFROMBITSPERSAMPLE(bitsPerSample), 
	mFmt.byte_order = g_byteOrder;
	
	mFmt.buffer_size = bufSize;
	status_t err = SetParameters(mFmt.frame_rate * 0.2, &mFmt, 2);
	if (err != B_OK ) {
		fprintf(stderr, "Error while PSoundPlayer::SetFormat: %s. Channels: %d, rate: %d, bits: %d, bufsize: %d.\n", 
			strerror(err), numChannels, sampleRate, bitsPerSample, bufSize );
	}

	if( fWasPlaying )
		StartPlaying();
}

