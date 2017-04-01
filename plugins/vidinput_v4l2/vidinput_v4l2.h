/*
 * vidinput_v4l2.h
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
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
 * The Original Code is Portable Windows Library (V4L plugin).
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *                 Mark Cooke (mpc@star.sr.bham.ac.uk)
 *                 Nicola Orru' <nigu@itadinanta.it>
 *
 */
#ifndef _PVIDEOIOV4L2
#define _PVIDEOIOV4L2


#include <sys/mman.h>
#include <sys/time.h>

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>
#include <ptclib/delaychan.h>

#include V4L2_HEADER

#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B','A','8','1') /*  8  BGBG.. GRGR.. */
#endif

class PVideoInputDevice_V4L2: public PVideoInputDevice
{

  PCLASSINFO(PVideoInputDevice_V4L2, PVideoInputDevice);
private:
  PVideoInputDevice_V4L2(const PVideoInputDevice_V4L2& ):readyToReadMutex(0,1) {};
  PVideoInputDevice_V4L2& operator=(const PVideoInputDevice_V4L2& ){ return *this; };
public:
  PVideoInputDevice_V4L2();
  virtual ~PVideoInputDevice_V4L2();

  static PStringList GetInputDeviceNames();

  PStringArray GetDeviceNames() const { return GetInputDeviceNames(); }

  PBoolean Open(const PString &deviceName, PBoolean startImmediate);

  PBoolean IsOpen();

  PBoolean Close();

  PBoolean Start();
  PBoolean Stop();

  PBoolean IsCapturing();

  PINDEX GetMaxFrameBytes();

  PBoolean GetFrameData(BYTE*, PINDEX*);
  PBoolean GetFrameDataNoDelay(BYTE*, PINDEX*);

  PBoolean GetFrameSizeLimits(unsigned int&, unsigned int&,
			  unsigned int&, unsigned int&);

  PBoolean SetFrameSize(unsigned int, unsigned int);
  PBoolean SetNearestFrameSize(unsigned int, unsigned int);
  PBoolean SetFrameRate(unsigned int);

  PBoolean SetColourFormat(const PString&);

  bool GetAttributes(Attributes & attrib);
  bool SetAttributes(const Attributes & attrib);

  PBoolean SetVideoChannelFormat(int, PVideoDevice::VideoFormat);
  PBoolean SetVideoFormat(PVideoDevice::VideoFormat);
  int GetNumChannels();
  PBoolean SetChannel(int);

  /**Retrieve a list of Device Capabilities
    */
  virtual PBoolean GetDeviceCapabilities(
    Capabilities * capabilities          ///< List of supported capabilities
  ) const;

  /**Retrieve a list of Device Capabilities for particular device
    */
  static PBoolean GetDeviceCapabilities(
    const PString & deviceName,           ///< Name of device
    Capabilities * capabilities,          ///< List of supported capabilities
    PPluginManager * pluginMgr = NULL     ///< Plug in manager, use default if NULL
  );

private:
  int GetControlCommon(unsigned int control, int *value);
  PBoolean SetControlCommon(unsigned int control, int newValue);
  PBoolean NormalReadProcess(BYTE*, PINDEX*);

  void Reset();
  void ClearMapping();
  PBoolean SetMapping();

  PBoolean VerifyHardwareFrameSize(unsigned int & width, unsigned int & height);
  PBoolean TryFrameSize(unsigned int& width, unsigned int& height);

  PBoolean QueueAllBuffers();

  PBoolean StartStreaming();
  void StopStreaming();

  PBoolean DoIOCTL(unsigned long int r, void * s, int structSize, PBoolean retryOnBusy=false);

  PBoolean EnumFrameFormats(Capabilities & capabilities) const;
  PBoolean EnumControls(Capabilities & capabilities) const;

  struct v4l2_capability videoCapability;
  struct v4l2_streamparm videoStreamParm;
  PBoolean   canRead;
  PBoolean   canStream;
  PBoolean   canSelect;
  PBoolean   canSetFrameRate;
  PBoolean   isMapped;
#define NUM_VIDBUF 4
  BYTE * videoBuffer[NUM_VIDBUF];
  uint   videoBufferCount;
  uint   currentVideoBuffer;

  PSemaphore readyToReadMutex;			/** Allow frame reading only from the time Start() used until Stop() */
  PMutex inCloseMutex;				/** Prevent GetFrameDataNoDelay() to stuck on readyToReadMutex in the middle of device closing operation */
  PBoolean isOpen;				/** Has the Video Input Device successfully been opened? */
  PBoolean areBuffersQueued;
  PBoolean isStreaming;

  int    videoFd;
  int    frameBytes;
  PBoolean   started;
  PAdaptiveDelay m_pacing;
  PString userFriendlyDevName;
};

#endif
