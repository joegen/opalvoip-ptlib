#ifndef _PVIDEOIOV4L2
#define _PVIDEOIOV4L2


#include <sys/mman.h>
#include <sys/time.h>

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vconvert.h>

#include <linux/videodev.h>


class PVideoInputV4l2Device: public PVideoInputDevice
{

  PCLASSINFO(PVideoInputV4l2Device, PVideoInputDevice);
      

public:
  PVideoInputV4l2Device();
  ~PVideoInputV4l2Device();
  
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

  BOOL GetFrame(PBYTEArray & frame);
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
  uint    videoBufferCount;

  int    videoFd;
  int    frameBytes;
  BOOL   started;
};

#endif
