/*
 * shmvideo.h
 *
 * This file contains the class hierarchy for both shared memory video
 * input and output devices.
 *
 * Copyright (c) 2003 Pai-Hsiang Hsiao
 * Copyright (c) 1998-2003 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * $Id$
 */

#ifndef __SHMVIDEO_H__
#define __SHMVIDEO_H__

#define P_FORCE_STATIC_PLUGIN

#ifndef _PTLIB_H
#include <ptlib.h>
#endif

#include <ptclib/delaychan.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <semaphore.h>

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHMVIDEO_MAX_WIDTH  512
#define SHMVIDEO_MAX_HEIGHT 512
/*
 * maximum frame size is computed using RGB32 storage requirement and
 * the dimension.
 */
#define SHMVIDEO_FRAMESIZE  (SHMVIDEO_MAX_WIDTH*SHMVIDEO_MAX_HEIGHT*4)
/*
 * the shared memory buffer started with a 3-byte header of
 * (width, height, bytes per pixel)
 */
#define SHMVIDEO_BUFSIZE    (sizeof(long)*3+SHMVIDEO_FRAMESIZE)


#define SEM_NAME_OF_OUTPUT_DEVICE "PVideoOutputDevice_Shm"

class PVideoOutputDevice_Shm : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDevice_Shm, PVideoOutputDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice_Shm();

    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name (filename base) to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine if the device is currently open.
      */
    virtual BOOL IsOpen();

    /**Close the device.
      */
    virtual BOOL Close();

    /**Get a list of all of the drivers available.
      */
    virtual PStringList GetDeviceNames() const;
	
	virtual BOOL SetColourFormat(const PString & colourFormat);
	
	virtual BOOL SetFrameSize(unsigned width,
							  unsigned height);
	
	virtual PINDEX GetMaxFrameBytes();
	
	virtual BOOL SetFrameData(unsigned x,
							  unsigned y,
							  unsigned width,
							  unsigned height,
							  const BYTE * data,
							  BOOL endFrame = TRUE);
	
	/**Indicate frame may be displayed.
		*/
	virtual BOOL EndFrame();

  protected:
    BOOL shmInit();

    PINDEX bytesPerPixel;
    sem_t *semLock;
    int    shmId;
    key_t  shmKey;
    void * shmPtr;
};

#define SEM_NAME_OF_INPUT_DEVICE "PVideoInputDevice_Shm"

class PVideoInputDevice_Shm : public PVideoInputDevice
{
  PCLASSINFO(PVideoInputDevice_Shm, PVideoInputDevice);

  public:
    PVideoInputDevice_Shm();
	
    BOOL Start();
	
    BOOL Stop();

    BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
      );
    BOOL IsOpen();
    BOOL Close();

    /**Get a list of all of the drivers available.
     */
    static PStringList GetInputDeviceNames();
	
	virtual PStringList GetDeviceNames() const
	{ return GetInputDeviceNames(); }
	
	virtual BOOL GetFrame(PBYTEArray & frame);

    virtual BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// Optional bytes returned.
      );

    virtual BOOL GetFrameDataNoDelay (BYTE *, PINDEX *);
	

    /**Get the minimum & maximum size of a frame on the device.

    Default behaviour returns the value 1 to UINT_MAX for both and returns
    FALSE.
    */
    virtual BOOL GetFrameSizeLimits(
      unsigned & minWidth,   /// Variable to receive minimum width
      unsigned & minHeight,  /// Variable to receive minimum height
      unsigned & maxWidth,   /// Variable to receive maximum width
      unsigned & maxHeight   /// Variable to receive maximum height
      ) ;
	
	virtual BOOL TestAllFormats();
	
	virtual BOOL IsCapturing();
	
	virtual PINDEX GetMaxFrameBytes();


  protected:
    BOOL shmInit();

    PAdaptiveDelay m_pacing;

    PINDEX videoFrameSize;
    int grabCount;
    sem_t *semLock;
    int shmId;
    key_t shmKey;
    void *shmPtr;
};


#endif /* __SHMVIDEO_H__ */
