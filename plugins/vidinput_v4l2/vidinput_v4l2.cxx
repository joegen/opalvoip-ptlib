/*
 * vidinput_v4l2.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1998-2000 Equivalence Pty. Ltd.
 * Copyright (c) 2003 March Networks
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
 * First V4L2 capture code written by March Networks 
 * (http://www.marchnetworks.com) 
 *
 * This code is based on the Video4Linux 1 code.
 *
 * Contributor(s): Guilhem Tardy (gtardy@salyens.com)
 *  Nicola Orru' <nigu@itadinanta.it>
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#pragma implementation "vidinput_v4l2.h"

#include "vidinput_v4l2.h"
#include <sys/ioctl.h>
#include <sys/utsname.h>

PCREATE_VIDINPUT_PLUGIN(V4L2);
#define CLEAR(x) memset (&(x), 0, sizeof (x))

#include "vidinput_names.h" 

#ifdef HAS_LIBV4L
#include <libv4l2.h>
#else
#define v4l2_fd_open(fd, flags) (fd)
#define v4l2_open open
#define v4l2_close close
#define v4l2_ioctl ioctl
#define v4l2_read read
#define v4l2_mmap mmap
#define v4l2_munmap munmap
#endif  

class V4L2Names : public V4LXNames
{

  PCLASSINFO(V4L2Names, V4LXNames);

public:
  
  V4L2Names() { kernelVersion=KUNKNOWN; };

  virtual void Update ();
  
protected:
  
  virtual PString BuildUserFriendly(PString devname);

  enum KernelVersionEnum {
    K2_4, 
    K2_6,
    KUNKNOWN,
  } kernelVersion;

};


PMutex creationMutex;
static 
V4L2Names & GetNames()
{
  PWaitAndSignal m(creationMutex);
  static V4L2Names names;
  names.Update();
  return names;
}

// this is used to get more userfriendly names:

void
V4L2Names::Update()
{
  PTRACE(1,"PV4L2Plugin\tDetecting V4L2 devices");
  PWaitAndSignal m(mutex);
  inputDeviceNames.RemoveAll (); // flush the previous run
#if defined(P_FREEBSD)
  for (int i = 0; i < 10; i++) {
    PString thisDevice = PString("/dev/video") + PString(i);
    int videoFd=::v4l2_open((const char *)thisDevice, O_RDONLY | O_NONBLOCK);
    if ((videoFd > 0) || (errno == EBUSY)) {
      PBoolean valid = false;
      struct v4l2_capability videoCaps;
      CLEAR(videoCaps);
      if ((errno == EBUSY) ||
          (v4l2_ioctl(videoFd, VIDIOC_QUERYCAP, &videoCaps) >= 0 &&
          (videoCaps.capabilities & V4L2_CAP_VIDEO_CAPTURE))) {
        PTRACE(1,"PV4L2Plugin\tdetected capture device " << videoCaps.card);
        valid = true;
      }
      else {
        PTRACE(1,"PV4L2Plugin\t" << thisDevice << "is not deemed valid");
      }
      if (videoFd>0)
        ::v4l2_close(videoFd);
      if(valid)
        inputDeviceNames += thisDevice;
    }
  }
#else
  PDirectory   procvideo2_4("/proc/video/dev");
  PDirectory   procvideo2_6("/sys/class/video4linux");
  PDirectory * procvideo;
  PString      entry;
  PStringList  devlist;
  PString      oldDevName;
  // Try and guess kernel version
  if (procvideo2_6.Exists()) {
    kernelVersion = K2_6;
    procvideo=&procvideo2_6;
  }
  else if (procvideo2_4.Exists()) {
    kernelVersion=K2_4;
    procvideo=&procvideo2_4;
  }
  else {
    kernelVersion=KUNKNOWN;
    procvideo=0;
  }
  if (procvideo) {
    PTRACE(2,"PV4L2Plugin\tdetected device metadata at "<<*procvideo);
    if (((kernelVersion==K2_6 && procvideo->Open(PFileInfo::SubDirectory|PFileInfo::SymbolicLink)) ||
        (procvideo->Open(PFileInfo::RegularFile)))) {
      do {
        entry = procvideo->GetEntryName();
        if ((entry.Left(5) == "video")) {
          PString thisDevice = "/dev/" + entry;
          int videoFd=::v4l2_open((const char *)thisDevice, O_RDONLY | O_NONBLOCK);
          if ((videoFd > 0) || (errno == EBUSY)) {
            PBoolean valid = false;
            struct v4l2_capability videoCaps;
            CLEAR(videoCaps);
            if ((errno == EBUSY) ||
                (v4l2_ioctl(videoFd, VIDIOC_QUERYCAP, &videoCaps) >= 0 &&
                (videoCaps.capabilities & V4L2_CAP_VIDEO_CAPTURE))) {
              PTRACE(1,"PV4L2Plugin\tdetected capture device " << videoCaps.card);
              valid = true;
            }
            else {
              PTRACE(1,"PV4L2Plugin\t" << thisDevice << "is not deemed valid");
            }
            if (videoFd>0)
              ::v4l2_close(videoFd);
            if(valid)
              inputDeviceNames += thisDevice;
          }
          else {
            PTRACE(1,"PV4L2Plugin\tcould not open " << thisDevice);
          }
        }
      } while (procvideo->Next());
    }
  }
  else {
    PTRACE(1,"Unable to detect v4l2 directory");
  }
#endif
  if (inputDeviceNames.GetSize() == 0) {
    POrdinalToString vid;
    ReadDeviceDirectory("/dev/", vid);

    for (POrdinalToString::iterator it = vid.begin(); it != vid.end(); ++it) {
      PINDEX cardnum = it->first;
      int fd = ::v4l2_open(vid[cardnum], O_RDONLY | O_NONBLOCK);
      if ((fd >= 0) || (errno == EBUSY)) {
        if (fd >= 0)
          ::v4l2_close(fd);
        inputDeviceNames += vid[cardnum];
      }
    }
  }
  PopulateDictionary();
}

PString V4L2Names::BuildUserFriendly(PString devname)
{
  PString Result;

  int fd = ::v4l2_open((const char *)devname, O_RDONLY);
  if(fd < 0) {
    return devname;
  }

  struct v4l2_capability videocap;
  CLEAR(videocap);
  if (v4l2_ioctl(fd, VIDIOC_QUERYCAP, &videocap) < 0)  {
    ::v4l2_close(fd);
    return devname;
  }

  ::v4l2_close(fd);
  PString ufname((const char*)videocap.card);

  return ufname;
}

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_V4L2

PVideoInputDevice_V4L2::PVideoInputDevice_V4L2():
readyToReadMutex(0,1)		// Initially creating mutex blocked. Will unlock it in a Start() function.
{
  Reset();
  areBuffersQueued = false;
  videoBufferCount = 0;
  currentVideoBuffer = 0;
  frameBytes = 0;
}

PVideoInputDevice_V4L2::~PVideoInputDevice_V4L2()
{
  Close();
}


#ifndef V4L2_PIX_FMT_H263
#define V4L2_PIX_FMT_H263       v4l2_fourcc('H','2','6','3')
#endif


static struct {
  const char * colourFormat;
#ifdef SOLARIS
  uint32_t code;
#else
  __u32 code;
#endif
} colourFormatTab[] = {
    { "Grey", V4L2_PIX_FMT_GREY },   //Entries in this table correspond
    { "RGB32", V4L2_PIX_FMT_RGB32 }, //(line by line) to those in the 
    { "BGR32", V4L2_PIX_FMT_BGR32 }, //PVideoDevice ColourFormat table.
    { "RGB24", V4L2_PIX_FMT_RGB24 }, 
    { "BGR24", V4L2_PIX_FMT_BGR24 },
    { "RGB565", V4L2_PIX_FMT_RGB565 },
    { "RGB555", V4L2_PIX_FMT_RGB555 },
    { "YUV411", V4L2_PIX_FMT_Y41P },
    { "YUV411P", V4L2_PIX_FMT_YUV411P },
    { "YUV420", V4L2_PIX_FMT_NV21 },
    { "YUV420P", V4L2_PIX_FMT_YUV420 },
    { "YUV422", V4L2_PIX_FMT_YUYV },   /* Note: YUV422 is for compatibility */
    { "YUV422P", V4L2_PIX_FMT_YUV422P },
    { "YUY2", V4L2_PIX_FMT_YUYV },
    { "JPEG", V4L2_PIX_FMT_JPEG },
    { "H263", V4L2_PIX_FMT_H263 },
    { "SBGGR8", V4L2_PIX_FMT_SBGGR8 },
    { "MJPEG", V4L2_PIX_FMT_MJPEG},
    { "UYVY422", V4L2_PIX_FMT_UYVY}
};


PBoolean PVideoInputDevice_V4L2::Open(const PString & devName, PBoolean /* startImmediate */)
{
  if (isOpen) {
    PTRACE(1,"V4L2\tClosing " << m_deviceName << " already open on this instance, fd:" << videoFd);
    Close();
  }

  m_deviceName=GetNames().GetDeviceName(devName);
  userFriendlyDevName=devName;
  
  PTRACE(5,"V4L2\tOpen()\tdevName:" << m_deviceName << "  videoFd:" << videoFd);
  videoFd = ::v4l2_open((const char *)m_deviceName, O_RDWR);
  if (videoFd < 0) {
    PTRACE(1,"V4L2\topen failed : " << ::strerror(errno));
    return isOpen;
  }

  isOpen = true;

  PTRACE(5,"V4L2\tNew handle for " << m_deviceName << ": fd=" << videoFd);

  // Don't share the camera device with subprocesses - they could cause
  // EBUSY errors on VIDIOC_STREAMON if the parent tries to close and reopen
  // the camera while the child is still running.
  ::fcntl(videoFd, F_SETFD, FD_CLOEXEC);

  /* Note the v4l2_xxx functions are designed so that if they get passed an
     unknown fd, the will behave exactly as their regular xxx counterparts, so
     if v4l2_fd_open fails, we continue as normal (missing the libv4l2 custom
     cam format to normal formats conversion). Chances are big we will still
     fail then though, as normally v4l2_fd_open only fails if the device is not
     a v4l2 device. */
  int libv4l2_fd = v4l2_fd_open(videoFd, 0);
  if (libv4l2_fd != -1)
    videoFd = libv4l2_fd;

  // get the device capabilities
  if (v4l2_ioctl(videoFd, VIDIOC_QUERYCAP, &videoCapability) < 0) {
    PTRACE(1,"V4L2\tQUERYCAP failed : " << ::strerror(errno));
    Close();
    return isOpen;
  }

  canRead = videoCapability.capabilities & V4L2_CAP_READWRITE;
  canStream = videoCapability.capabilities & V4L2_CAP_STREAMING;
  canSelect = videoCapability.capabilities & V4L2_CAP_ASYNCIO;

  // set height and width
  m_frameHeight = QCIFHeight;
  m_frameWidth  = QCIFWidth;


  // get the capture parameters
  videoStreamParm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(videoFd, VIDIOC_G_PARM, &videoStreamParm) < 0)  {

    PTRACE(3,"V4L2\tG_PARM failed : " << ::strerror(errno));
    canSetFrameRate = false;

  } else {

    canSetFrameRate = videoStreamParm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME;
    if (canSetFrameRate) {
      if (videoStreamParm.parm.capture.timeperframe.numerator == 0) {
        PTRACE(1,"V4L2\tDriver/webcam bug: numerator is zero and denominator is " << videoStreamParm.parm.capture.timeperframe.denominator << ", I assume it cannot set frame rate");
        canSetFrameRate = PFalse;
      } else {
        unsigned rate = videoStreamParm.parm.capture.timeperframe.denominator / videoStreamParm.parm.capture.timeperframe.numerator;
        if(rate <= 0 || rate > 1000)
          rate = 1;
        PVideoDevice::SetFrameRate (rate);
      }
    }
  }

  SetVideoFormat(m_videoFormat);
  SetColourFormat(m_colourFormat);

  return true;
}


PBoolean PVideoInputDevice_V4L2::IsOpen()
{
  return isOpen;
}


PBoolean PVideoInputDevice_V4L2::Close()
{
  PWaitAndSignal m(inCloseMutex);
  PTRACE(1,"V4L2\tClose()\tvideoFd:" << videoFd << "  started:" << started << "  isOpen:" << isOpen);
  if (IsOpen()){
    if(IsCapturing())
      Stop();

    if (v4l2_close(videoFd) < 0) {
      PTRACE(2, "V4L2\tERROR errno = " << ::strerror(errno) << "(" << errno << ")");
    }

    Reset();
  }

  return true;
}


PBoolean PVideoInputDevice_V4L2::Start()
{
  PTRACE(8, "V4L2\tStarting " << m_deviceName);

  if (started == true) {
    PTRACE(2, "V4L2\tVideo Input Device already started");
    return started;
  }

  if(canStream){
    // automatically set mapping
    if (!SetMapping()) {
      ClearMapping();
      canStream = false; // don't try again
      return started;
    }

    /* Queue all buffers */
    if (!QueueAllBuffers()) {
      PTRACE(2, "V4L2\tCould not QueueBuffers for Video Input Device!");
      return started;
    }

    /* Start streaming */
    if (!StartStreaming()) {
      PTRACE(2, "V4L2\tCould not StartStreaming for Video Input Device!");
      return started;
    }
  }

  started = true;
  readyToReadMutex.Signal();

  return started;
}


PBoolean PVideoInputDevice_V4L2::Stop()
{
  if (started) {
    readyToReadMutex.Wait();
    StopStreaming();
    ClearMapping();

    areBuffersQueued = false;  // Looks like at kernel 3.5.3 queued buffers vanish after ClearMapping() SetMapping() sequence
    started = false;

    // no need to dequeue filled buffers, as this is handled by V4L2 at the next VIDIOC_STREAMON
  }

  return true;
}


PBoolean PVideoInputDevice_V4L2::IsCapturing()
{
  return started;
}

PStringList PVideoInputDevice_V4L2::GetInputDeviceNames()
{
  return GetNames().GetInputDeviceNames();
}


PBoolean PVideoInputDevice_V4L2::SetVideoFormat(VideoFormat newFormat)
{
  PTRACE(8,"V4L2\tSet video format " << newFormat);

  if (newFormat == Auto) {
    PBoolean videoStandardSetCorrectly = false;
    if (true == (videoStandardSetCorrectly = SetVideoFormat(PAL))) {
      return videoStandardSetCorrectly;
    }
    if (true == (videoStandardSetCorrectly = SetVideoFormat(NTSC))) {
      return videoStandardSetCorrectly;
    }
    if (true == (videoStandardSetCorrectly = SetVideoFormat(SECAM))) {
      return videoStandardSetCorrectly;
    }
    return videoStandardSetCorrectly;
  }

  struct {
#ifdef SOLARIS
    uint32_t code;
#else
    __u32 code;
#endif
    const char * name;
  } static const fmt[3] = { {V4L2_STD_PAL, "PAL"},
      {V4L2_STD_NTSC, "NTSC"},
      {V4L2_STD_SECAM, "SECAM"} };

#ifdef SOLARIS
    uint32_t carg;
#else
    __u32 carg;
#endif
  carg = V4L2_STD_UNKNOWN;

  if (v4l2_ioctl(videoFd, VIDIOC_G_STD, &carg) < 0) {
    PTRACE(3, "V4L2\tG_STD failed for fd=" << videoFd << " with error: " << ::strerror(errno));
    // Assume that if G_STD is not available, that the device still works correctly.
    return true;
  } else {
    PTRACE(5, "V4L2\tG_STD succeeded for " << newFormat << ", carg = " << carg);
  }

  carg = fmt[newFormat].code;

  if (v4l2_ioctl(videoFd, VIDIOC_S_STD, &carg) < 0) {
    PTRACE(2, "V4L2\tS_STD failed for " << newFormat << " with error: " << ::strerror(errno));
    return false;
  } else {
    PTRACE(5, "V4L2\tS_STD succeeded for " << newFormat << ", carg = " << carg);
  }

  if (!PVideoDevice::SetVideoFormat(newFormat)) {
    PTRACE(1,"PVideoDevice::SetVideoFormat failed for format " << newFormat);
    return false;
  }

  return true;
}


int PVideoInputDevice_V4L2::GetNumChannels()
{
  PTRACE(8,"V4L2\tGet number of channels");
  // if opened, return the capability value, else 1 as in videoio.cxx
  if (IsOpen ()) {

    struct v4l2_input videoEnumInput;
    videoEnumInput.index = 0;
    while (v4l2_ioctl(videoFd, VIDIOC_ENUMINPUT, &videoEnumInput) >= 0)
      videoEnumInput.index++;

    return videoEnumInput.index;
  }
  else
    return 1;
}


PBoolean PVideoInputDevice_V4L2::SetChannel(int newChannel)
{
  PTRACE(8,"V4L2\tSet channel #" << newChannel);

  if (!PVideoDevice::SetChannel(newChannel)) {
    PTRACE(1,"PVideoDevice::SetChannel failed for channel " << newChannel);
    return false;
  }

  // set the channel
  if (v4l2_ioctl(videoFd, VIDIOC_S_INPUT, &m_channelNumber) < 0) {
    PTRACE(1,"VideoInputDevice\tS_INPUT failed : " << ::strerror(errno));
    return false;
  }

  PTRACE(6,"V4L2\tset channel " << newChannel << ", fd=" << videoFd);

  return true;
}


PBoolean PVideoInputDevice_V4L2::SetVideoChannelFormat (int newChannel, VideoFormat videoFormat) 
{
  PTRACE(8,"V4L2\tSet channel #" << newChannel << " format \"" << videoFormat << "\"");

  if (!SetChannel(newChannel) ||
      !SetVideoFormat(videoFormat))
    return false;

  return true;
}


PBoolean PVideoInputDevice_V4L2::SetColourFormat(const PString & newFormat)
{
  PTRACE(8,"V4L2\tSet colour format \"" << newFormat << "\"");

  PINDEX currentColourFormatIndex, colourFormatIndex = 0;
  while (newFormat != colourFormatTab[colourFormatIndex].colourFormat) {
    PTRACE(9,"V4L2\tColourformat did not match " << colourFormatTab[colourFormatIndex].colourFormat);
    colourFormatIndex++;
    if (colourFormatIndex >= PARRAYSIZE(colourFormatTab))
      return false;
  }

  if (!PVideoDevice::SetColourFormat(newFormat)) {
    PTRACE(3,"V4L2\tSetColourFormat failed for colour format " << newFormat);
    return false;
  }

  struct v4l2_format videoFormat;
  CLEAR(videoFormat);
  videoFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // get the frame rate so we can preserve it throughout the S_FMT call
  struct v4l2_streamparm streamParm;
  CLEAR(streamParm);
  unsigned int fi_n = 0, fi_d = 0;
  streamParm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(videoFd, VIDIOC_G_PARM, &streamParm) == 0 &&
        (streamParm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
    fi_n = streamParm.parm.capture.timeperframe.numerator;
    fi_d = streamParm.parm.capture.timeperframe.denominator;
    PTRACE(8,"V4L2\tG_PARM succeeded (preserving frame rate at " << fi_n << "/" << fi_d << ")");
  } else {
    PTRACE(1,"V4L2\tG_PARM failed (preserving frame rate may not work) : " << ::strerror(errno));
  }

  // get the colour format
  if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &videoFormat) < 0) {
    PTRACE(1,"V4L2\tG_FMT failed : " << ::strerror(errno));
    return false;
  } else {
    PTRACE(8,"V4L2\tG_FMT succeeded");
  }

  // update colourFormat to current value so in case of VIDIOC_S_FMT failure will have corect one
  for (currentColourFormatIndex = 0; currentColourFormatIndex < PARRAYSIZE(colourFormatTab); currentColourFormatIndex++) {
    if (videoFormat.fmt.pix.pixelformat == colourFormatTab[currentColourFormatIndex].code)
    {
      m_colourFormat = colourFormatTab[currentColourFormatIndex].colourFormat;
      break;
    }
  }

  if(videoFormat.fmt.pix.pixelformat == colourFormatTab[colourFormatIndex].code){
    frameBytes = videoFormat.fmt.pix.sizeimage;
    PTRACE(3,"V4L2\tcolour format already set.");
    return true;
  }

  videoFormat.fmt.pix.pixelformat = colourFormatTab[colourFormatIndex].code;

  {
    PBoolean resume = started;
    if (started == true) {
      Stop();
    }

    // set the colour format
    if (v4l2_ioctl(videoFd, VIDIOC_S_FMT, &videoFormat) < 0) {
      PTRACE(1,"V4L2\tS_FMT failed : " << ::strerror(errno));
      PTRACE(1,"V4L2\tused code of " << videoFormat.fmt.pix.pixelformat << " for palette: " << colourFormatTab[colourFormatIndex].colourFormat);
      return false;
    } else {
      PTRACE(8,"V4L2\tS_FMT succeeded for palette: " << colourFormatTab[colourFormatIndex].colourFormat);
    }

    // get the colour format again to be careful about broken drivers
    if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &videoFormat) < 0) {
      PTRACE(1,"V4L2\tG_FMT failed : " << ::strerror(errno));
      return false;
    } else {
      PTRACE(8,"V4L2\tG_FMT succeeded");
    }

    if (videoFormat.fmt.pix.pixelformat != colourFormatTab[colourFormatIndex].code) {
      PTRACE(3,"V4L2\tcolour format mismatch.");
      return false;
    } else {
      m_colourFormat = newFormat;
      PTRACE(8,"V4L2\tcolour format matches.");
    }

    // reset the frame rate because it may have been overridden by the call to S_FMT
    if (fi_n == 0 || fi_d == 0 || v4l2_ioctl(videoFd, VIDIOC_S_PARM, &streamParm) < 0) {
      PTRACE(3,"V4L2\tunable to reset frame rate.");
    } else if (streamParm.parm.capture.timeperframe.numerator != fi_n ||
               streamParm.parm.capture.timeperframe.denominator  != fi_d) {
      PTRACE(3, "V4L2\tnew frame interval (" << streamParm.parm.capture.timeperframe.numerator
                << "/" << streamParm.parm.capture.timeperframe.denominator
                << ") differs from what was requested (" << fi_n << "/" << fi_d << ").");
    } else {
      PTRACE(8,"V4L2\tS_PARM succeeded (preserving frame rate at " << fi_n << "/" << fi_d << ")");
    }

    frameBytes = videoFormat.fmt.pix.sizeimage;

    PTRACE(4,"V4L2\tset colour format \"" << newFormat << "\" set for " << m_deviceName
           << ", size=" << frameBytes << ", fd=" << videoFd);

    if (resume) {
      if (false == Start()) {
        return false;
      }
    }
  }

  return true;
}


PBoolean PVideoInputDevice_V4L2::SetFrameRate(unsigned rate)
{
  unsigned originalFrameRate = m_frameRate;
  if (!PVideoDevice::SetFrameRate(rate)) {
    PTRACE(3,"V4L2\tSetFrameRate failed for rate " << rate);
    return false;
  }

  PTRACE(8,"V4L2\tSetFrameRate()\tvideoFd:" << videoFd << "  started:" << started);

  if (!canSetFrameRate)
    return true;

  videoStreamParm.parm.capture.timeperframe.numerator = 1;
  videoStreamParm.parm.capture.timeperframe.denominator = rate;

  bool wasStarted = started;
  if (wasStarted)
    Stop();

  // set the stream parameters
  if (!DoIOCTL(VIDIOC_S_PARM, &videoStreamParm, sizeof(videoStreamParm), true))  {
    PTRACE(1,"V4L2\tS_PARM failed : "<< ::strerror(errno));
    m_frameRate = originalFrameRate;
    return false;
  }

  // The rate is 1/x * s^-1 where timeperframe is x * s, so we have to inverse the
  // values.
  v4l2_ioctl(videoFd, VIDIOC_G_PARM, &videoStreamParm);
  rate = videoStreamParm.parm.capture.timeperframe.denominator /
          videoStreamParm.parm.capture.timeperframe.numerator;
  PTRACE_IF(2, rate != m_frameRate, "V4L2\tFrame rate mismatch, "
            "wanted=" << m_frameRate << "fps, got=" << rate << "fps, fd=" << videoFd);

  return !wasStarted || Start();
}


PBoolean PVideoInputDevice_V4L2::GetFrameSizeLimits(unsigned & minWidth,
                                                unsigned & minHeight,
                                                unsigned & maxWidth,
                                                unsigned & maxHeight) 
{
  minWidth=0;
  maxWidth=65535;
  minHeight=0;
  maxHeight=65535;

  // Before 2.6.19 there is no official way to enumerate frame sizes
  // in V4L2, but we can use VIDIOC_TRY_FMT to find the largest supported
  // size. This is roughly what the kernel V4L1 compatibility layer does.

  struct v4l2_format fmt;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &fmt) < 0) {
    return false;
  }

  fmt.fmt.pix.width = fmt.fmt.pix.height = 10000;
  if (v4l2_ioctl(videoFd, VIDIOC_TRY_FMT, &fmt) < 0) {
    return false;
  }
  maxWidth = fmt.fmt.pix.width;
  maxHeight = fmt.fmt.pix.height;

  PTRACE(8,"V4L2\tFrame size limits: [" << minWidth << "," << maxWidth << "]" << "x"
                                      << "[" << minWidth << "," << maxWidth << "]");

  return true;
}

PBoolean PVideoInputDevice_V4L2::SetFrameSize(unsigned width, unsigned height) {
  unsigned requestedWidth = width;
  unsigned requestedHeight = height;

  // Try first. If it succeeds, we may skip close and reopen the device.
  if(TryFrameSize(requestedWidth, requestedHeight)){
    if ((requestedWidth != width) || (requestedHeight != height))
    {
      PTRACE(4, "V4L2\t" << width << "x" << height << " requested but "
                              << requestedWidth << "x" << requestedHeight << " returned");
      return false;
    }
  }

  if (!VerifyHardwareFrameSize(requestedWidth, requestedHeight)) {
    PTRACE(5, "V4L2\tVerifyHardwareFrameSize failed for size " << width << "x" << height);
    PTRACE(4, "V4L2\tCurrent resolution " << requestedWidth << "x" << requestedHeight);
    return false;
  }

  if ((requestedWidth != width) || (requestedHeight != height)){
    PTRACE(4, "V4L2\t" << width << "x" << height << " requested but "
                            << requestedWidth << "x" << requestedHeight << " returned");
    return false;
  } else {
    PTRACE(5, "V4L2\tVerifyHardwareFrameSize succeeded for size " << width << "x" << height);
    PTRACE(4, "V4L2\tCurrent resolution " << requestedWidth << "x" << requestedHeight);
  }

  if(!PVideoDevice::SetFrameSize(requestedWidth, requestedHeight)){
    return false;
  }

  return true;
}

PBoolean PVideoInputDevice_V4L2::SetNearestFrameSize(unsigned width, unsigned height) {
  unsigned requestedWidth = width;
  unsigned requestedHeight = height;

  if (!VerifyHardwareFrameSize(requestedWidth, requestedHeight)) {
    PTRACE(5, "V4L2\tVerifyHardwareFrameSize failed for size " << width << "x" << height);
    PTRACE(4, "V4L2\tCurrent resolution " << requestedWidth << "x" << requestedHeight);
    return false;
  }

  if ((requestedWidth != width) || (requestedHeight != height)){
    PTRACE(4, "V4L2\t" << width << "x" << height << " requested but "
                            << requestedWidth << "x" << requestedHeight << " returned");
  }

  if(!PVideoDevice::SetFrameSize(requestedWidth, requestedHeight)){
    return false;
  }

  return true;
}


PINDEX PVideoInputDevice_V4L2::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(frameBytes);
}


PBoolean PVideoInputDevice_V4L2::SetMapping()
{
  if (isMapped) {
    PTRACE(2, "V4L2\tVideo buffers already mapped! Do ClearMapping() first!");
    ClearMapping();
    if(isMapped)
      return false;
  }

  if (!canStream)
    return isMapped;

  struct v4l2_requestbuffers reqbuf;
  CLEAR(reqbuf);
  reqbuf.count = NUM_VIDBUF;
  reqbuf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  reqbuf.memory = V4L2_MEMORY_MMAP;

  if (v4l2_ioctl(videoFd, VIDIOC_REQBUFS, &reqbuf) < 0) {
    PTRACE(3,"V4L2\tREQBUFS failed : " << ::strerror(errno));
    return isMapped;
  }
  if (reqbuf.count < 1) {
    PTRACE(3,"V4L2\tNot enough video buffer available. (got " << reqbuf.count << ")");
    return isMapped;
  }
  if (reqbuf.count > NUM_VIDBUF) {
    PTRACE(3,"V4L2\tToo much video buffer allocated. (got " << reqbuf.count << ")");
    return isMapped;
  }

  struct v4l2_buffer buf;
  videoBufferCount = reqbuf.count;

  for (uint i = 0; i < videoBufferCount; i++) {
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;
    if (v4l2_ioctl(videoFd, VIDIOC_QUERYBUF, &buf) < 0) {
      PTRACE(3,"V4L2\tQUERYBUF failed : " << ::strerror(errno));
      return isMapped;
    }

    if ((videoBuffer[buf.index] = (BYTE *)v4l2_mmap(0, buf.length, PROT_READ|PROT_WRITE, MAP_SHARED, videoFd, buf.m.offset)) == MAP_FAILED) {
      PTRACE(3,"V4L2\tmmap failed for buffer " << buf.index << " with error " << ::strerror(errno) << "(" << errno << ")");
      return isMapped;
    }
  }

  isMapped = true;

  PTRACE(7,"V4L2\tset mapping for " << videoBufferCount << " buffers, fd=" << videoFd);


  return isMapped;
}


void PVideoInputDevice_V4L2::ClearMapping()
{
  if (!canStream) // 'isMapped' wouldn't handle partial mappings
    return;

  struct v4l2_buffer buf;
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;

  for (buf.index = 0; ; buf.index++) {
    if (v4l2_ioctl(videoFd, VIDIOC_QUERYBUF, &buf) < 0)
      break;

#ifdef SOLARIS
    ::v4l2_munmap((char*)videoBuffer[buf.index], buf.length);
#else
    ::v4l2_munmap(videoBuffer[buf.index], buf.length);
#endif
  }

  isMapped = false;

  PTRACE(5,"V4L2\tVideo Input Device \"" << m_deviceName << "\" cleared mapping, fd=" << videoFd);
}


PBoolean PVideoInputDevice_V4L2::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  PTRACE(8,"V4L2\tGetFrameData()");

  m_pacing.Delay(1000/GetFrameRate());
  return GetFrameDataNoDelay(buffer, bytesReturned);
}


PBoolean PVideoInputDevice_V4L2::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  PTRACE(8,"V4L2\tGetFrameDataNoDelay()\tstarted:" << started << "  canSelect:" << canSelect);
  {
    PWaitAndSignal m(inCloseMutex);
    if (!isOpen)
      return PFalse;
  }

  PWaitAndSignal m(readyToReadMutex);
  if (!started)
    return PFalse;

  if (!canStream)
    return NormalReadProcess(buffer, bytesReturned);

  // Using streaming here. Return false, if streaming wasn't started, yet
  if(!isStreaming)
    return PFalse;

  // use select() here, because VIDIOC_DQBUF seems to block with some drivers
  // and does never return.
  fd_set rfds;

  // Time interval is half the frame rate, so we want to wait max. two frames.
  struct timeval tv; tv.tv_sec = 0; tv.tv_usec = (2 * 1000 * 1000)/GetFrameRate();

  FD_ZERO(&rfds);
  FD_SET(videoFd, &rfds);

  int ret = select(videoFd+1, &rfds, NULL, NULL, &tv);

  if(ret == -1){
    PTRACE(1,"V4L2\tselect() failed : " << ::strerror(errno));
    return PFalse;
  } else if(ret == 0){
    PTRACE(4,"V4L2\tNo data in outgoing queue. Skip frame (@" << GetFrameRate() << "fps)");
    return PTrue;
  }

  struct v4l2_buffer buf;
  CLEAR(buf);
  buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  buf.index = currentVideoBuffer;

  if (v4l2_ioctl(videoFd, VIDIOC_DQBUF, &buf) < 0) {
    // strace resistance
    if (errno == EINTR) {
      if (v4l2_ioctl(videoFd, VIDIOC_DQBUF, &buf) < 0) {
        PTRACE(1,"V4L2\tDQBUF failed : " << ::strerror(errno));
        return false;
      }
    }
  }

  currentVideoBuffer = (currentVideoBuffer+1) % NUM_VIDBUF;

  // If the dequeued buffer returns zero bytes, do not copy it as
  // it is possibly corrupt.
  if(buf.bytesused){
    // If converting on the fly do it from frame store to output buffer,
    // otherwise do straight copy.
    if (m_converter != NULL) {
      m_converter->SetSrcFrameBytes(buf.bytesused);
      m_converter->Convert(videoBuffer[buf.index], buffer, bytesReturned);
    }
    else {
      memcpy(buffer, videoBuffer[buf.index], buf.bytesused);
      if (bytesReturned != NULL)
        *bytesReturned = buf.bytesused;
    }

    PTRACE(8,"V4L2\tget frame data of " << buf.bytesused << "bytes, fd=" << videoFd);
  }

  // requeue the buffer
  if (v4l2_ioctl(videoFd, VIDIOC_QBUF, &buf) < 0) {
    PTRACE(1,"V4L2\tQBUF failed : " << ::strerror(errno));
  }

  return true;
}


// This video device does not support memory mapping - so use
// normal read process to extract a frame of video data.
PBoolean PVideoInputDevice_V4L2::NormalReadProcess(BYTE * buffer, PINDEX * bytesReturned)
{ 
  if (!canRead)
    return false;

  ssize_t bytesRead;

  do
    bytesRead = v4l2_read(videoFd, buffer, frameBytes);
  while (bytesRead < 0 && errno == EINTR && IsOpen());

  if (bytesRead < 0) {
    
    PTRACE(1,"V4L2\tread failed (read = "<<bytesRead<< " expected " << frameBytes <<")");
    bytesRead = frameBytes;
  }

  if ((PINDEX)bytesRead != frameBytes) {
    PTRACE(1,"V4L2\tread returned fewer bytes than expected");
    // May result from a compressed format, otherwise indicates an error.
  }

  if (m_converter != NULL)
    return m_converter->ConvertInPlace(buffer, bytesReturned);

  if (bytesReturned != NULL)
    *bytesReturned = (PINDEX)bytesRead;

  return true;
}

PBoolean PVideoInputDevice_V4L2::TryFrameSize(unsigned& width, unsigned& height){

  struct v4l2_format videoFormat;
  CLEAR(videoFormat);
  videoFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  // get the frame size
  if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &videoFormat) < 0) {
    PTRACE(1,"V4L2\tG_FMT failed : " << ::strerror(errno));
    return false;
  }

  videoFormat.fmt.pix.width = width;
  videoFormat.fmt.pix.height = height;

  if(v4l2_ioctl(videoFd, VIDIOC_TRY_FMT, &videoFormat) < 0){
    PTRACE(3,"V4L2\tTRY_FMT failed : " << ::strerror(errno));
    return false;
  }

  width = videoFormat.fmt.pix.width;
  height = videoFormat.fmt.pix.height;

  return true;

}

PBoolean PVideoInputDevice_V4L2::VerifyHardwareFrameSize(unsigned & width, unsigned & height)
{
  struct v4l2_format videoFormat;
  CLEAR(videoFormat);
  videoFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  struct v4l2_streamparm streamParm;
  CLEAR(streamParm);
  unsigned int fi_n = 0, fi_d = 0;
  streamParm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  // get the frame size
  if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &videoFormat) < 0) {
    PTRACE(1,"V4L2\tG_FMT failed : " << ::strerror(errno));
    return false;
  }

  // get the frame rate so we can preserve it throughout the S_FMT call
  // Sidenote: V4L2 gives us the frame interval, i.e. 1/fps.
  if (v4l2_ioctl(videoFd, VIDIOC_G_PARM, &streamParm) == 0 &&
        (streamParm.parm.capture.capability & V4L2_CAP_TIMEPERFRAME)) {
    fi_n = streamParm.parm.capture.timeperframe.numerator;
    fi_d = streamParm.parm.capture.timeperframe.denominator;
  } else {
    PTRACE(1,"V4L2\tG_PARM failed (preserving frame rate may not work) : " << ::strerror(errno));
  }

  videoFormat.fmt.pix.width = width;
  videoFormat.fmt.pix.height = height;

  {
    PBoolean resume = started;

    if (started == true) {
      Stop();
    }

    PTRACE(4, "V4L2\tTry setting resolution: " << videoFormat.fmt.pix.width << "x" << videoFormat.fmt.pix.height);
    if(!DoIOCTL(VIDIOC_S_FMT, &videoFormat, sizeof(videoFormat), true)){
      PTRACE(1,"V4L2\tS_FMT failed: " << ::strerror(errno));
      return false;
    }

    //
    //	Double checking that the resolution change was successful (in case of a broken driver)
    //
    CLEAR(videoFormat);
    videoFormat.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

    if (v4l2_ioctl(videoFd, VIDIOC_G_FMT, &videoFormat) < 0) {
      PTRACE(1,"V4L2\tG_FMT failed : " << ::strerror(errno));
      return false;
    } else {
      frameBytes = videoFormat.fmt.pix.sizeimage;
      PTRACE(5, "V4L2\tG_FMT returned resolution: "<< videoFormat.fmt.pix.width << "x" <<
              videoFormat.fmt.pix.height << ", size=" << frameBytes);
      width = videoFormat.fmt.pix.width;
      height = videoFormat.fmt.pix.height;
    }

    // reset the frame rate because it may have been overridden by the call to S_FMT
    if (fi_n == 0 || fi_d == 0 || v4l2_ioctl(videoFd, VIDIOC_S_PARM, &streamParm) < 0) {
      PTRACE(3,"V4L2\tunable to reset frame rate.");
    } else if (streamParm.parm.capture.timeperframe.numerator != fi_n ||
               streamParm.parm.capture.timeperframe.denominator  != fi_d) {
      PTRACE(3, "V4L2\tnew frame interval (" << streamParm.parm.capture.timeperframe.numerator
                << "/" << streamParm.parm.capture.timeperframe.denominator
                << ") differs from what was requested (" << fi_n << "/" << fi_d << ").");
    }

    if (resume) {
      if (false == Start()) {
        return false;
      }
    }
  }

  return true;
}

PBoolean PVideoInputDevice_V4L2::DoIOCTL(unsigned long int r, void * s, int structSize, PBoolean retryOnBusy)
{
  PBoolean retval = false;
  void *structCopy = NULL;

  if(!(structCopy = malloc(structSize)))
    goto end;
  memcpy(structCopy, s, structSize);
  
  if ((retval = (v4l2_ioctl(videoFd, r, s) >= 0)))
    goto end;

  if (errno != EBUSY || !retryOnBusy)
    goto end;

  PTRACE(3,"V4L2\tReopening device and retrying IOCTL ("<< r << ')');
  Close();
  Open(userFriendlyDevName, true);

  memcpy(s, structCopy, structSize);
  retval = (v4l2_ioctl(videoFd, r, s) >= 0);

end:
  free(structCopy);
  return retval;
}


/**
 * Query the current control setting
 * @param control is v4l2 control id (V4L2_CID_BRIGHTNESS, V4L2_CID_WHITENESS, ...)
 * @return  -1 control is unknown, or an error occured
 *         >=0 current value in a range [0-65535]
 */
int PVideoInputDevice_V4L2::GetControlCommon(unsigned int control, int *value) 
{
  struct v4l2_queryctrl q;
  CLEAR(q);
  q.id = control;
  if (v4l2_ioctl(videoFd, VIDIOC_QUERYCTRL, &q) < 0)
    return -1;

  struct v4l2_control c;
  CLEAR(c);
  c.id = control;
  if (v4l2_ioctl(videoFd, VIDIOC_G_CTRL, &c) < 0)
    return -1;

  *value = (int)((float)(c.value - q.minimum) / (q.maximum-q.minimum) * (1<<16));

  return *value;
}

bool PVideoInputDevice_V4L2::GetAttributes(Attributes & attrib)
{
  if (!IsOpen())
    return false;

  GetControlCommon(V4L2_CID_BRIGHTNESS, &attrib.m_brightness);
  GetControlCommon(V4L2_CID_SATURATION, &attrib.m_saturation);
  GetControlCommon(V4L2_CID_CONTRAST, &attrib.m_contrast);
  GetControlCommon(V4L2_CID_HUE, &attrib.m_hue);
  GetControlCommon(V4L2_CID_GAMMA, &attrib.m_gamma);
  GetControlCommon(V4L2_CID_EXPOSURE, &attrib.m_exposure);

  return true;
}

/**
 * Set a control to a new value
 *
 * @param control: V4L2_CID_BRIGHTNESS, V4L2_CID_WHITENESS, ...
 * @param newValue: 0-65535 Set this control to this range
 *                  -1 Set the default value
 * @return false, if an error occur or the control is not supported
 */
PBoolean PVideoInputDevice_V4L2::SetControlCommon(unsigned int control, int newValue)
{
  if (!IsOpen())
    return false;

  struct v4l2_queryctrl q;
  CLEAR(q);
  q.id = control;
  if (v4l2_ioctl(videoFd, VIDIOC_QUERYCTRL, &q) < 0)
    return false;

  struct v4l2_control c;
  CLEAR(c);
  c.id = control;
  if (newValue < 0)
    c.value = q.default_value;
  else
    c.value = (float)(q.minimum + ((q.maximum-q.minimum) * ((float)newValue) / (1<<16)));

  if (v4l2_ioctl(videoFd, VIDIOC_S_CTRL, &c) < 0)
    return false;

  return true;
}

bool PVideoInputDevice_V4L2::SetAttributes(const Attributes & attrib)
{ 
  return SetControlCommon(V4L2_CID_BRIGHTNESS, attrib.m_brightness) &&
         SetControlCommon(V4L2_CID_SATURATION, attrib.m_saturation) &&
         SetControlCommon(V4L2_CID_CONTRAST, attrib.m_contrast) &&
         SetControlCommon(V4L2_CID_HUE, attrib.m_hue) &&
         SetControlCommon(V4L2_CID_GAMMA, attrib.m_gamma) &&
         SetControlCommon(V4L2_CID_EXPOSURE, attrib.m_exposure);
}

PBoolean PVideoInputDevice_V4L2::QueueAllBuffers()
{
  if (true == areBuffersQueued) {
    PTRACE(3, "V4L2\tVideo buffers already queued!");
    return areBuffersQueued;
  }

  if (false == isMapped) {
    PTRACE(3, "Buffers are not mapped yet! Do SetMapping() first!");
    return areBuffersQueued;
  }

  /* Queue all buffers */
  currentVideoBuffer = 0;

  for (unsigned int i = 0; i < videoBufferCount; i++) {
    struct v4l2_buffer buf;
    CLEAR(buf);

    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index = i;

    if (v4l2_ioctl(videoFd, VIDIOC_QBUF, &buf) < 0) {
      PTRACE(3, "V4L2\tVIDIOC_QBUF failed for buffer " << i << ": " << ::strerror(errno));
      return areBuffersQueued;
    }
    PTRACE(6, "V4L2\tBuffer " << i << " queued");
  }

  areBuffersQueued = true;
  PTRACE(8, "V4L2\t" << videoBufferCount << " buffers successfully queued.");
  return areBuffersQueued;
}


PBoolean PVideoInputDevice_V4L2::StartStreaming()
{
  PTRACE(8, "V4L2\tStart streaming for \"" << m_deviceName << "\" with fd=" << videoFd);

  if (true == isStreaming) {
    PTRACE(4, "V4L2\tVideo buffers already streaming! Nothing to do.");
    return isStreaming;
  }

  if (false == areBuffersQueued) {
    PTRACE(2, "Buffers are not queued yet! Do QueueBuffers() first!");
    return isStreaming;
  }

  //
  // Now start streaming
  //
  enum v4l2_buf_type type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if (v4l2_ioctl(videoFd, VIDIOC_STREAMON, &type) < 0) {
    PTRACE(2, "V4L2\tSTREAMON failed with error " << ::strerror(errno));
    return isStreaming;
  }

  isStreaming = true;
  PTRACE(5, "V4L2\tVideo Input Device \"" << m_deviceName << "\" successfully started streaming.");
  return isStreaming;
}


void PVideoInputDevice_V4L2::StopStreaming(){
  if (false == isStreaming) {
    PTRACE(2, "V4L2\tVideo buffers already not streaming! Do StartStreaming() first.");
    return;
  }

  int type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (v4l2_ioctl(videoFd, VIDIOC_STREAMOFF, &type) < 0) {
    PTRACE(2, "V4L2\tSTREAMOFF failed : " << ::strerror(errno));
    return;
  }

  isStreaming = false;
  PTRACE(5, "V4L2\tVideo Input Device \"" << m_deviceName << "\" successfully stopped streaming.");
}

void PVideoInputDevice_V4L2::Reset(){
  videoFd = -1;
  canRead = false;
  canStream = false;
  canSelect = false;
  canSetFrameRate = false;
  isOpen = false;
  isMapped = false;
  isStreaming = false;
  started = false;
  areBuffersQueued = false;
  videoBufferCount = 0;
  currentVideoBuffer = 0;
  frameBytes = 0;
  CLEAR(videoCapability);
  CLEAR(videoStreamParm);
  CLEAR(videoBuffer);
}

PBoolean PVideoInputDevice_V4L2::EnumControls(Capabilities & capabilities) const {
  //TODO: get control capabilities
  return true;
}

PBoolean PVideoInputDevice_V4L2::EnumFrameFormats(Capabilities & capabilities) const {
  int retFmt, retSize, retFps;
  struct v4l2_fmtdesc fmt;
  struct v4l2_frmsizeenum fsize;
  struct v4l2_frmivalenum fival;

  CLEAR(fmt);
  fmt.index = 0;
  fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  PVideoFrameInfo frameInfo;

  PTRACE(4, "V4L2\tEnumerate all frame format information");

  while ((retFmt = ioctl(videoFd, VIDIOC_ENUM_FMT, &fmt)) == 0) {
    // Set the frame format
    PINDEX colourFormatIndex = 0;
    while (fmt.pixelformat != colourFormatTab[colourFormatIndex].code) {
      colourFormatIndex++;
      if (colourFormatIndex >= PARRAYSIZE(colourFormatTab))
        break;
    }
    if(colourFormatIndex < PARRAYSIZE(colourFormatTab)){
      frameInfo.SetColourFormat(colourFormatTab[colourFormatIndex].colourFormat);

      CLEAR(fsize);
      fsize.index = 0;
      fsize.pixel_format = fmt.pixelformat;
      while ((retSize = ioctl(videoFd, VIDIOC_ENUM_FRAMESIZES, &fsize)) == 0) {
        if (fsize.type == V4L2_FRMSIZE_TYPE_DISCRETE) {

          frameInfo.SetFrameSize(fsize.discrete.width,fsize.discrete.height);
          CLEAR(fival);
          fival.index = 0;
          fival.pixel_format = fsize.pixel_format;
          fival.width = fsize.discrete.width;
          fival.height = fsize.discrete.height;
          while ((retFps = ioctl(videoFd, VIDIOC_ENUM_FRAMEINTERVALS, &fival)) == 0) {
            if (fival.type == V4L2_FRMIVAL_TYPE_DISCRETE) {

              frameInfo.SetFrameRate(fival.discrete.denominator/fival.discrete.numerator);
              capabilities.m_frameSizes.push_back(frameInfo);

              PTRACE(7, "V4L2\tAdded frame size capability: " << frameInfo);

            } else if(fival.type == V4L2_FRMIVAL_TYPE_STEPWISE ||
                      fival.type == V4L2_FRMIVAL_TYPE_CONTINUOUS) {

              frameInfo.SetFrameRate(fival.stepwise.min.denominator/fival.stepwise.min.numerator);
              capabilities.m_frameSizes.push_back(frameInfo);

              PTRACE(7, "V4L2\tAdded frame size capability: " << frameInfo);

              frameInfo.SetFrameRate(fival.stepwise.max.denominator/fival.stepwise.max.numerator);
              capabilities.m_frameSizes.push_back(frameInfo);

              PTRACE(7, "V4L2\tAdded frame size capability: " << frameInfo);

              break;
            }
            fival.index++;
          }
          if (retFps != 0 && errno != EINVAL) {
            PTRACE(3, "V4L2\tError enumerating frame intervals");
            return false;
          }
        }
        fsize.index++;
      }
      if (retSize != 0 && errno != EINVAL) {
        PTRACE(3, "V4L2\tError enumerating frame sizes");
        return false;
      }
    }
    fmt.index++;
  }
  if (errno != EINVAL) {
    PTRACE(3, "V4L2\tError enumerating frame formats");
    return false;
  }

  return true;
}

PBoolean PVideoInputDevice_V4L2::GetDeviceCapabilities(
    Capabilities * caps
    ) const
{
  //static Capabilities capabilities;

  PTRACE(4, "V4L2\tGet device capabilities for " << m_deviceName);

  if(!EnumFrameFormats(*caps) || !EnumControls(*caps))
    return false;

  //caps = &capabilities;

  return true;
}

PBoolean PVideoInputDevice_V4L2::GetDeviceCapabilities(
    const PString & m_deviceName,
    Capabilities * capabilities,
    PPluginManager * pluginMgr)
{
  PVideoInputDevice_V4L2 device;

  device.Open(m_deviceName, false);

  return device.GetDeviceCapabilities(capabilities);
}

// End Of File ///////////////////////////////////////////////////////////////
