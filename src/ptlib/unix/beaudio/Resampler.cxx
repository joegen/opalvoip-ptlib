//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
// Resampling routines
//

// Don't forget to change buffer to 2048 in resample.c!
// When updating the code.

extern "C" {
#include "st.h"
#undef BYTE
#undef LONG
#undef WORD
#undef DWORD
};

#include "Resampler.h"

extern "C" {
typedef struct effect *eff_t;
void report(char * fmt, ...) { 
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args );
	va_end(args);
	
	PError << "<Resampler> " << buf << endl; 
};

void fail(char *fmt, ...) { 
	char buf[1024];
	va_list args;
	va_start(args, fmt);
	vsprintf(buf, fmt, args );
	va_end(args);
	
	PError << "<Resampler failed> " << buf << endl; 
};

void resample_getopts(eff_t effp, int n, char ** argv);
void resample_start(eff_t effp);
void resample_stop(eff_t effp);
void resample_flow(eff_t effp, long* ibuf, long* obuf, int* isamp, int* osamp);
};

PResampler::PResampler() :
	mEffp(new effect), 
	mnChannels(2), mnSampleSize(16),
	mLongInPtr(NULL),
	mLongOutPtr(NULL)
{
	// Resampler
	PAssertNULL( mEffp );
	memset(mEffp, 0, sizeof(eff_t));
}

void PResampler::Create(long from, long to, 
	unsigned channels, unsigned samplesize, int samples)
{
	PAssertNULL( mEffp );
	
	//TODO: Fix params
	mEffp->ininfo.rate = from;
	mEffp->ininfo.size = 2;
	mEffp->ininfo.style = 2;
	mEffp->ininfo.channels = 2;
	mEffp->outinfo.rate = to; 
	mEffp->outinfo.size = 2;
	mEffp->outinfo.style = 2;
	mEffp->outinfo.channels = 2;

	mnChannels = channels;
	mnSampleSize = samplesize;

	// resample_start allocates some buffers. We need their buf sizes
	mbufSamples = 4096; //TODO fix this

	mInSound.SetFormat( 
		mnChannels, 
		from, 	
		mnSampleSize );
	mInSound.SetSize( mbufSamples * sizeof(long) );
	mLongInPtr = (long*) mInSound.GetPointer(); 

	mOutSound.SetFormat( 
		mnChannels, 
		to, 	
		mnSampleSize );
	mOutSound.SetSize( mbufSamples * sizeof(long) );
	mLongOutPtr = (long*) mOutSound.GetPointer(); 

	// Resample start
	resample_getopts(mEffp, 0, NULL);
	resample_start(mEffp);
}

PResampler::~PResampler()
{
	PAssertNULL( mEffp );

	// Stop resampler
	resample_stop(mEffp);
	delete mEffp;
}

bool PResampler::Resample(PSound& sound)
{
	int nSize;
	int iOutputSamples;
	int iInputSamples;
	int iNewSamples;
	int16* pwSound;
	int16* pwNewSound;
	PSound newSound;
	
	PAssertNULL( mEffp );
	PAssert( mEffp->ininfo.rate == (long) sound.GetSampleRate(), 
		PInvalidParameter );
	PAssert( mnChannels == sound.GetChannels(), 
		PInvalidParameter );
	PAssert( mnSampleSize == sound.GetSampleSize(), 
		PInvalidParameter );

	nSize = sound.GetSize();
	PAssert( nSize > 0, 
		PInvalidParameter );

	pwSound = (int16*) sound.GetPointer();
	PAssertNULL( pwSound );

	// Prepare new sound
	newSound.SetFormat( 
		mnChannels, 
		mEffp->outinfo.rate, 	
		mnSampleSize );

	// What space is needed to allocate new sound?
	double Factor = 
		(double)mEffp->outinfo.rate / (double)mEffp->ininfo.rate;

	newSound.SetSize( (nSize / Factor) + 512 ); // reserve a bit more space
	pwNewSound = (int16*) newSound.GetPointer();
	PAssertNULL( pwNewSound );

	iNewSamples = 0;
	if( nSize >= mbufSamples )
	{
		nSize -= (nSize % mbufSamples); // <-TODO: Remove this
		PAssert( (nSize % mbufSamples) == 0, PInvalidParameter );
		while( nSize > 0 )
		{ 
			// Init counters
			iOutputSamples = iInputSamples = mbufSamples;

			// Copy old sound to input sound, converting to long
			for ( int i = 0, j = 0; i < iInputSamples; i++, j++ )
				mLongInPtr[i] = (((long)pwSound[j++]) << 16); // only one channel
			
			// Do it
			resample_flow(mEffp, mLongInPtr, mLongOutPtr, 
				&iInputSamples, &iOutputSamples);
		
			for ( int i = 0; i < iOutputSamples; i++ )
				pwNewSound[i] = ( (mLongOutPtr[i] >> 16) & 0x0000FFFF );

			nSize -= mbufSamples;
			pwSound += mbufSamples;
			pwNewSound += iOutputSamples;
			iNewSamples += iOutputSamples;
		}
		
		// Set new format
		sound.SetFormat( 
			mnChannels, 
			mEffp->outinfo.rate, 	
			mnSampleSize );

		// Adjust size
		sound.SetSize( iNewSamples * sizeof(int16) );

		// Copy data bytes
		sound = newSound;
	}
	return true;
}
