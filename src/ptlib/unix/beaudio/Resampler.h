//
// (c) Yuri Kiryanov, home.att.net/~bevox
// for www.Openh323.org
//
#ifndef _RESAMPLER_H
#define _RESAMPLER_H

// Some stuff from sox
typedef struct effect *eff_t;

// We write for PWLib
#include <ptlib.h>

class PResampler : public PObject {

	// Resampler structures
	eff_t mEffp;
	int mbufSamples;
	
	PSound mInSound, mOutSound;

	unsigned mnChannels;
	unsigned mnSampleSize;
	long* mLongInPtr;
	long* mLongOutPtr;
	
public:
	PResampler();
	~PResampler();

	void Create(long from, long to, 
		unsigned channels = 2, 
		unsigned samplesize = 16, 
		int samples = 4096);
	bool Resample(PSound& sound);
};

#endif // _RESAMPLER_H
