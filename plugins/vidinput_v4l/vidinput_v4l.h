#pragma interface

#include <sys/mman.h>
#include <sys/time.h>

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>
#include <ptlib/vconvert.h>
#if !P_USE_INLINES
#include <ptlib/contain.inl>
#endif

// FIXME! For some reason I had to copy-paste it from linux/videodev.h...
struct video_capability
{
        char name[32];
        int type;
        int channels;   /* Num channels */
        int audios;     /* Num audio devices */
        int maxwidth;   /* Supported width */
        int maxheight;  /* And height */
        int minwidth;   /* Supported width */
        int minheight;  /* And height */
};


class PVideoInputV4lDevice: public PVideoInputDevice
{

public:
  PVideoInputV4lDevice();
  ~PVideoInputV4lDevice();

  static PStringList GetInputDeviceNames();

  BOOL Close();

  BOOL Open(const PString &deviceName, BOOL startImmediate);

  BOOL IsOpen();
  BOOL IsCapturing();

  BOOL Start();
  BOOL Stop();

  PINDEX GetMaxFrameBytes();

  BOOL GetFrameData(BYTE*, PINDEX*);
  BOOL GetFrameDataNoDelay(BYTE*, PINDEX*);

  BOOL GetFrameSizeLimits(unsigned int&, unsigned int&,
			  unsigned int&, unsigned int&);
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
  BOOL TestAllFormats();

  void ClearMapping();

  int    videoFd;
  struct video_capability videoCapability;
  int    canMap;  // -1 = don't know, 0 = no, 1 = yes
  int    colourFormatCode;
  PINDEX hint_index;
  BYTE *videoBuffer;
  PINDEX frameBytes;
  
  BOOL   pendingSync[2];
  
  int    currentFrame;
  struct video_mbuf frame;
  struct video_mmap frameBuffer[2];
  
};
