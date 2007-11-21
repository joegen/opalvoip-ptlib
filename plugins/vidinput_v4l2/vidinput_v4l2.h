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
 * $Id$
 */
#ifndef _PVIDEOIOV4L2
#define _PVIDEOIOV4L2


#include <sys/mman.h>
#include <sys/time.h>

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>
#include <ptclib/delaychan.h>

#ifdef SOLARIS
#include <sys/videodev2.h>
#else
#include <linux/videodev.h>
#endif

#ifndef V4L2_PIX_FMT_SBGGR8
#define V4L2_PIX_FMT_SBGGR8  v4l2_fourcc('B','A','8','1') /*  8  BGBG.. GRGR.. */
#endif

class PVideoInputDevice_V4L2: public PVideoInputDevice
{

  PCLASSINFO(PVideoInputDevice_V4L2, PVideoInputDevice);
      

public:
  PVideoInputDevice_V4L2();
  ~PVideoInputDevice_V4L2();
  
  void ReadDeviceDirectory (PDirectory, POrdinalToString &);

  static PStringList GetInputDeviceNames();

  PStringList GetDeviceNames() const
  { return GetInputDeviceNames(); }

  BOOL Open(const PString &deviceName, BOOL startImmediate);

  BOOL IsOpen();

  BOOL Close();

  BOOL Start();
  BOOL Stop();

  BOOL IsCapturing();

  PINDEX GetMaxFrameBytes();

  BOOL GetFrameData(BYTE*, PINDEX*);
  BOOL GetFrameDataNoDelay(BYTE*, PINDEX*);

  BOOL GetFrameSizeLimits(unsigned int&, unsigned int&,
			  unsigned int&, unsigned int&);

  BOOL TestAllFormats();

  BOOL SetFrameSize(unsigned int, unsigned int);
  BOOL SetFrameRate(unsigned int);
  BOOL VerifyHardwareFrameSize(unsigned int, unsigned int);

  BOOL GetParameters(int*, int*, int*, int*, int*);

  BOOL SetColourFormat(const PString&);

  int GetControlCommon(unsigned int control, int *value);
  BOOL SetControlCommon(unsigned int control, int newValue);

  int GetContrast();
  BOOL SetContrast(unsigned int);
  int GetBrightness();
  BOOL SetBrightness(unsigned int);
  int GetWhiteness();
  BOOL SetWhiteness(unsigned int);
  int GetColour();
  BOOL SetColour(unsigned int);
  int GetHue();
  BOOL SetHue(unsigned int);

  BOOL SetVideoChannelFormat(int, PVideoDevice::VideoFormat);
  BOOL SetVideoFormat(PVideoDevice::VideoFormat);
  int GetNumChannels();
  BOOL SetChannel(int);

  BOOL NormalReadProcess(BYTE*, PINDEX*);

  void ClearMapping();

  BOOL SetMapping();

  struct v4l2_capability videoCapability;
  struct v4l2_streamparm videoStreamParm;
  BOOL   canRead;
  BOOL   canStream;
  BOOL   canSelect;
  BOOL   canSetFrameRate;
  BOOL   isMapped;
#define NUM_VIDBUF 4
  BYTE * videoBuffer[NUM_VIDBUF];
  uint   videoBufferCount;
  uint   currentvideoBuffer;

  int    videoFd;
  int    frameBytes;
  BOOL   started;
  PAdaptiveDelay m_pacing;
};

#endif
