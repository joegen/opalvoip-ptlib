/*
 * sound.h
 *
 * Sound class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: sound.h,v $
 * Revision 1.16  2003/03/01 17:05:05  rogerh
 * Mac OS X updates from Shawn Pai-Hsiang Hsiao
 *
 * Revision 1.15  2003/02/24 17:49:02  rogerh
 * Add Mac Core Audio changes from Shawn.
 *
 * Revision 1.14  2003/02/19 10:22:22  rogerh
 * Add ESD fix from Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 * Revision 1.13  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.12  2001/08/11 07:57:30  rogerh
 * Add Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.11  2001/07/09 06:16:15  yurik
 * Jac Goudsmit's BeOS changes of July,6th. Cleaning up media subsystem etc.
 *
 * Revision 1.10  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.9  2001/02/07 03:33:43  craigs
 * Added functions to get sound channel parameters
 *
 * Revision 1.8  2000/07/02 14:15:55  craigs
 * Fixed minor formatting issues
 *
 * Revision 1.7  2000/05/02 08:28:34  craigs
 * Removed "memory leaks" caused by brain-dead GNU linker
 *
 * Revision 1.6  2000/04/19 00:13:52  robertj
 * BeOS port changes.
 *
 * Revision 1.5  1999/07/19 01:34:22  craigs
 * Rewite to compensate for linux OSS sensitivity to ioctl order.
 *
 * Revision 1.4  1999/06/30 13:50:21  craigs
 * Added code to allow full duplex audio
 *
 * Revision 1.3  1998/11/30 22:07:13  robertj
 * New directory structure.
 *
 * Revision 1.2  1998/09/24 04:11:57  robertj
 * Added open software license.
 *
 */

#ifndef _PSOUND

#ifdef USE_ESD
#include <ptclib/delaychan.h>
#endif

#ifdef __BEOS__
#include <media/MediaFormats.h>
 
class BSoundPlayer;
class BMediaRecorder;
 
class P_CircularBuffer;
template <class ISample, class IntSample, class OSample> class BaseResampler;
typedef class BaseResampler<short, long, short> Resampler;
#endif // __BEOS__

#ifdef P_USE_PRAGMA 
#pragma interface
#endif

///////////////////////////////////////////////////////////////////////////////
// declare type for sound handle dictionary

#if defined(P_MAC_MPTHREADS)
class JRingBuffer;
#endif

///////////////////////////////////////////////////////////////////////////////
// PSound

#define _PSOUND_PLATFORM_INCLUDE
#include "../../sound.h"

#endif
#ifdef _PSOUND_PLATFORM_INCLUDE
#undef _PSOUND_PLATFORM_INCLUDE

  public:
    BOOL Close();
    BOOL Write(const void * buf, PINDEX len);
    BOOL Read(void * buf, PINDEX len);
  
#ifdef __BEOS__

  public:
    virtual BOOL IsOpen() const;
    
  private:
  	// Only one of the following pointers can be non-NULL at a time.
	BMediaRecorder		   *mRecorder;
	BSoundPlayer		   *mPlayer;

	// Raw media format specifier used for sound player.
	// It also stores the parameters (number of channels, sample rate etc) so
	// no need to store them separately here.
	// For the recorder, a media_format struct is created temporarily with
	// the data from this raw format spec.
	media_raw_audio_format	mFormat;

	// The class holds a circular buffer whose size is set with SetBuffers.
	// We only need one buffer for BeOS. The number of buffers that was set
	// is only kept for reference.
	friend class P_CircularBuffer;
	P_CircularBuffer	   *mBuffer;			// The internal buffer
	PINDEX					mNumBuffers;		// for reference only!
	
	// Just some helpers so that the Open function doesn't get too big
	BOOL OpenPlayer(void);
	BOOL OpenRecorder(const PString &dev);

	// internal buffer setting function so we can disable the SetBuffers
	// function for debug purposes
	// size is the total size, threshold is the fill/drain threshold on
	// the buffer
	BOOL InternalSetBuffers(PINDEX size, PINDEX threshold);

	// Input resampler
	Resampler			   *mResampler;

#else // !__BEOS__

  protected:
    BOOL  Setup();

    static PMutex dictMutex;

    Directions direction;
    PString device;
    BOOL isInitialised;

#if defined(P_MAC_MPTHREADS)
    JRingBuffer *mpInput;
#endif

    unsigned mNumChannels;
    unsigned mSampleRate;
    unsigned mBitsPerSample;
    unsigned actualSampleRate;

#ifdef USE_ESD
    PAdaptiveDelay writeDelay;
#endif

#ifdef P_MACOSX
    int caDevID;
    unsigned caNumChannels;
    void *caCBData;
    int caBufLen;
    float *caBuf;
    void *caConverterRef;
    pthread_mutex_t caMutex;
    pthread_cond_t caCond;
#endif

#endif

#endif // _PSOUND_PLATFORM_INCLUDE

// End Of File ////////////////////////////////////////////////////////////////
