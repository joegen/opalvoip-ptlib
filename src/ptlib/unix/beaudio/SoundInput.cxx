//
// (c) Yuri Kiryanov, openh323@kiryanov.com
// for www.Openh323.org by Equivalence
//
// Portions: 1998-1999, Be Incorporated
//
#include <stdio.h>
#include <MediaRoster.h>

#include "MediaRecorder.h"
#include "BlockFIFO.h"

#include "SoundInput.h"
#include "SoundPlayer.h"

PSoundInput::PSoundInput(const char* name,
	unsigned numChannels,
    unsigned sampleRate,
    unsigned bitsPerSample) :
	mDevice(0),
	mRecorder(name),
	mFIFO(4096, 1, 3)
{ 
	mRecording = false;
	mFrameSize = 0;
	mPeakFormat = 0;
	mNumChannels = numChannels;
    mSampleRate = sampleRate;
    mBitsPerSample = bitsPerSample;
	mRecorder.SetHook(this, RecordHook);
}

PSoundInput::~PSoundInput()
{
	if (mRecording) {
		StopRecording();
	}

	if (mDevice != 0) {
		BMediaRoster::CurrentRoster()->ReleaseNode(*mDevice);
		delete mDevice;
	}
}

bool PSoundInput::ConnectIt()
{
	media_format fmt;
	fprintf(stderr, "ConnectIt(): %s\n", mRecorder.IsConnected() ? "already connected" : "not yet connected");

	if (!mRecorder.IsConnected()) {
		fmt.type = B_MEDIA_RAW_AUDIO;
		char str[200];
		string_for_format(fmt, str, 200);
		fprintf(stderr, "connect format %s\n", str);

		if (mDevice != 0) {
			fprintf(stderr, "mDevice is non-NULL, connecting to id %ld\n", mDevice->node);
			mRecorder.Connect(*mDevice, 0, &fmt);
		}
		else {
			fprintf(stderr, "connecting to default for format\n");
			mError = mRecorder.Connect(fmt);
		}

		if (mError < B_OK) {
			fprintf(stderr, "cannot connect recorder to input\n");
			return false;
		}

		fmt = mRecorder.Format();
		mFrameSize = (fmt.u.raw_audio.format & 0xf)*fmt.u.raw_audio.channel_count;
		fprintf(stderr, "frame size: %ld\n", mFrameSize);
		fprintf(stderr, "data rate: %.3f MB/s\n", fmt.u.raw_audio.frame_rate*mFrameSize/(1024.0*1024.0));
		mPeakFormat = fmt.u.raw_audio.format;
		mRecorder.Start();
	}
	return true;
}

bool PSoundInput::StartRecording()
{
	if (mRecording) {
		return true;
	}
	// connect thing if not connected
	if (!ConnectIt()) {
		return false;
	}
	mFIFO.Reset();

	// set mRecording to true, and start recorder
	mRecording = true;

	media_format fmtRec(mRecorder.Format());
	media_format fmtOut(fmtRec);
	fmtOut.u.raw_audio.format = FORMATFROMBITSPERSAMPLE(mBitsPerSample);	
	fmtOut.u.raw_audio.channel_count = mNumChannels;
	fmtOut.u.raw_audio.frame_rate = 1.0 * mSampleRate;
	
	char strRecFmt[200];
	char strOutFmt[200];
	string_for_format(fmtRec, strRecFmt, 200);
	string_for_format(fmtOut, strOutFmt, 200);
	fprintf(stderr, "PSoundInput::StartRecording, Recorder format %s, Required format: %s.\n", 
			strRecFmt, strOutFmt);
	
	// Reset resampler
	memoryL = 0; memoryR = 0; mp = 0; mt = (int32) mSampleRate;
	
	return true;
}

bool PSoundInput::StopRecording()
{
	mRecording = false;
	mError = mRecorder.Disconnect();
	if (mError < B_OK) {
		return false;
	}
	status_t status;
	mFIFO.CopyNextBufferIn(&status, 0, B_INFINITE_TIMEOUT, true);
	
	return true;
}

//
//	Resampler
//
int PSoundInput::Resample(short * in, int inSize) 
{
	int c = 0;
	short * out = in;
	inSize /= (2 * mNumChannels); // (2/4 for mono/stereo, 16 bit in and out)

	while (inSize > 0) {
		while (mp < mt) {
			memoryL = ( memoryL +  *(in++) * 7 ) >> 3;
			if(mNumChannels == 2)
				memoryR = ( memoryR +  *(in++) * 7 ) >> 3;
			
			mp += 8000;
			inSize--;
			if (inSize < 1) goto done;
		}

		*out++ = memoryL;
		if(mNumChannels == 2)
			*out++ = memoryR;

		mp -= mt;
		c++;
	}
done:
	return c* 2 * mNumChannels; // (2/4 mono/stereo, 16-bit)
}

void PSoundInput::RecordHook(void *cookie, void *data, size_t size, const media_header &header)
{
	PSoundInput * pThis = (PSoundInput *)cookie;
	if (pThis->mRecording) {
		int x = pThis->mNumChannels == 2 ? 2 : 1;

		size  /= (x * sizeof(short));
		short* temp = new short[size];
		if( !temp )
			return;

		size_t n = 0;	 
		for( n = 0; n < size; n++ )
			temp[n] = ((short*)data)[n*x];

		size = pThis->Resample((short*)temp, n * sizeof(short));

		int32 result = pThis->mFIFO.CopyNextBufferIn(temp, size, B_INFINITE_TIMEOUT, false);
		delete[] temp;

		if (result < (int32) size) {
			fprintf(stderr, "CopyNextBufferIn(%p, %ld, B_INFINITE_TIMEOUT, false) failed with %ld.\n",
				data, size, result);
			fprintf(stderr, "error while recording: %s; bailing\n", strerror(result));
			pThis->mRecording = false;
		}
	}

	pThis->CalcPeakData(data, size);
}

//
//	Interface
//
bool PSoundInput::Read( void * buf, uint32 len )
{
	if( !mRecording )
		return false;
		
	while( 0 == mFIFO.SizeAvailableToGet() );
		
	status_t result = mFIFO.CopyNextBlockOut(buf, len, B_INFINITE_TIMEOUT);

	if (result < (int32) len ) {
		fprintf(stderr, "PSoundInput::Read: CopyNextBlockOut(%p, %ld, B_INFINITE_TIMEOUT, false) failed with %ld.\n",
			buf, len, result);
		return false;
	}
	
	return true;
}

void PSoundInput::CalcPeakData(void *data, size_t size)
{
//	do peak data
	if (mPeakFormat == 0x2) {

		int16 m = 0;
		int16 n = 0;
		int16 * ptr = (int16 *)data;
		int cnt = size/4;
		while (cnt-- > 0) {
			//	we don't handle -32768 correctly
			if (*ptr > m)
				m = *ptr;
			else if (-*ptr > m)
				m = -*ptr;
			ptr++;
			if (*ptr > n)
				n = *ptr;
			else if (-*ptr > n)
				n = -*ptr;
			ptr++;
		}
		SetMax(m/32767.0, n/32767.0);
	}
	else if (mPeakFormat == 0x4) {
		int32 m = 0;
		int32 n = 0;
		int32 * ptr = (int32 *)data;
		int cnt = size/8;
		while (cnt-- > 0) {
			//	we don't handle -32768 correctly
			if (*ptr > m)
				m = *ptr;
			else if (-*ptr > m)
				m = -*ptr;
			ptr++;
			if (*ptr > n)
				n = *ptr;
			else if (-*ptr > n)
				n = -*ptr;
			ptr++;
		}
		SetMax(m/(32767.0*65536.0), n/(32767.0*65536.0));
	}
	else if (mPeakFormat == 0x24) {
		float m = 0;
		float n = 0;
		float * ptr = (float *)data;
		int cnt = size/8;
		while (cnt-- > 0) {
			if (*ptr > m)
				m = *ptr;
			else if (-*ptr > m)
				m = -*ptr;
			ptr++;
			if (*ptr > n)
				n = *ptr;
			else if (-*ptr > n)
				n = -*ptr;
			ptr++;
		}
		SetMax(m, n);
	}
}
