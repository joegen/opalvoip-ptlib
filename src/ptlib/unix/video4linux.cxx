/*
 * video4linux.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
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
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *                 Mark Cooke (mpc@star.sr.bham.ac.uk)
 *
 * $Log: video4linux.cxx,v $
 * Revision 1.15  2001/03/20 02:21:57  robertj
 * More enhancements from Mark Cooke
 *
 * Revision 1.14  2001/03/08 23:08:28  robertj
 * Fixed incorrect usage of VIDIOCSYNC, thanks Thorsten Westheider
 *
 * Revision 1.13  2001/03/08 21:46:11  dereks
 * Removed check when setting framesize. Thanks Mark Cooke
 *
 * Revision 1.12  2001/03/08 08:31:34  robertj
 * Numerous enhancements to the video grabbing code including resizing
 *   infrastructure to converters. Thanks a LOT, Mark Cooke.
 *
 * Revision 1.11  2001/03/08 03:59:13  robertj
 * Fixed previous change, needed to allow for -1 as chammelNumber in Open().
 *
 * Revision 1.10  2001/03/08 02:23:17  robertj
 * Added improved defaulting of video formats so Open() does not fail.
 *
 * Revision 1.9  2001/03/07 23:46:18  robertj
 * Double check the v4l device did actually change colour format, thanks Mark Cooke.
 *
 * Revision 1.8  2001/03/07 01:42:59  dereks
 * miscellaneous video fixes. Works on linux now. Add debug statements
 * (at PTRACE level of 1)
 *
 * Revision 1.7  2001/03/07 00:10:05  robertj
 * Improved the device list, uses /proc, thanks Thorsten Westheider.
 *
 * Revision 1.6  2001/03/03 23:25:07  robertj
 * Fixed use of video conversion function, returning bytes in destination frame.
 *
 * Revision 1.5  2001/03/03 06:13:01  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.4  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.3  2000/07/30 03:54:28  robertj
 * Added more colour formats to video device enum.
 *
 * Revision 1.2  2000/07/26 06:13:25  robertj
 * Added missing pragma implementation for GNU headers.
 *
 * Revision 1.1  2000/07/26 02:40:30  robertj
 * Added video I/O devices.
 *
 */

#pragma implementation "videoio.h"

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>
#include <ptlib/vconvert.h>

#include <sys/mman.h>


///////////////////////////////////////////////////////////////////////////////
// Linux Video4Linux Driver Hints Tables.
//
// In an ideal API, we wouldn't need these hints on setup.  There are enough
// wrinkles it seems we have to provide a static list of hints for known
// issues.

#define HINT_CSWIN_ZERO_FLAGS               0x0001
#define HINT_CSPICT_ALWAYS_WORKS            0x0002
#define HINT_CGPICT_DOESNT_SET_PALETTE      0x0004
#define HINT_HAS_PREF_PALETTE               0x0008

static struct {
  char     *name_regexp;        // String used to match the driver name
  char     *name;               // String used for ptrace output
  unsigned hints;               // Hint flags
  int      pref_palette;        // Preferred palette.
} driver_hints[] = {

    /**Philips usb web cameras
    
       Native format is 420.  Use 420P as it's closer to 411P
     */
    
  { "^Philips [0-9]+ webcam$",
    "Philips USB webcam",
    HINT_HAS_PREF_PALETTE,
    VIDEO_PALETTE_YUV420P },
  
  /**Brooktree based capture boards.

     The current bttv driver doesn't fail CSPICT calls with unsupported
     palettes.  It also doesn't return a useful value from CGPICT calls
     to readback the palette.
   */
  { "^BT8(4|7)8",
    "Brooktree BT848 and BT878 based capture boards",
    HINT_CSWIN_ZERO_FLAGS |
    HINT_CSPICT_ALWAYS_WORKS |
    HINT_CGPICT_DOESNT_SET_PALETTE |
    HINT_HAS_PREF_PALETTE,
    VIDEO_PALETTE_YUV411P },

  /** Default device with no special settings
   */
  { "",
    "V4L Supported Device",
    0,
    0 }

};

#define HINT(h) ((driver_hints[hint_index].hints & h) ? TRUE : FALSE)

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

PVideoInputDevice::PVideoInputDevice()
{
  videoFd       = -1;
  canMap        = -1;
  hint_index    = PARRAYSIZE(driver_hints) - 1;
}


static struct {
  const char * colourFormat;
  int code;
} colourFormatTab[] = {
  { "Grey", VIDEO_PALETTE_GREY },  //Entries in this table correspond
  { "RGB32", VIDEO_PALETTE_RGB32 }, //(line by line) to those in the 
  { "RGB24", VIDEO_PALETTE_RGB24 }, // PVideoDevice ColourFormat table.
  { "RGB565", VIDEO_PALETTE_RGB565 },
  { "RGB555", VIDEO_PALETTE_RGB555 },
  { "YUV422", VIDEO_PALETTE_YUV422 },
  { "YUV422P", VIDEO_PALETTE_YUV422P },
  { "YUV411", VIDEO_PALETTE_YUV411 },
  { "YUV411P", VIDEO_PALETTE_YUV411P },
  { "YUV420", VIDEO_PALETTE_YUV420 },
  { "YUV420P", VIDEO_PALETTE_YUV420P },
  { "YUV410P", VIDEO_PALETTE_YUV410P }
};


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  Close();

  deviceName = devName;
  videoFd = ::open((const char *)devName, O_RDWR);
  if (videoFd < 0) {
    PTRACE(1,"PVideoInputDevice::Open failed : "<< ::strerror(errno));
    return FALSE;
  }
  
  // get the device capabilities
  if (::ioctl(videoFd, VIDIOCGCAP, &videoCapability) < 0)  {
    PTRACE(1,"PVideoInputDevice:: get device capablilities failed : "<< ::strerror(errno));
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  }
  
  hint_index = PARRAYSIZE(driver_hints) - 1;
  PString driver_name = videoCapability.name;

  // Scan the hint table, looking for regular expression matches with
  // drivers we hold hints for.
  PINDEX tbl;
  for (tbl = 0; tbl < PARRAYSIZE(driver_hints); tbl ++) {
    PRegularExpression regexp;
    regexp.Compile(driver_hints[tbl].name_regexp, PRegularExpression::Extended);

    if (driver_name.FindRegEx(regexp) != P_MAX_INDEX) {
      PTRACE(1,"PVideoInputDevice::Open: Found driver hints: " << driver_hints[tbl].name);
      hint_index = tbl;
      break;
    }
  }

  // set height and width
  frameHeight = videoCapability.maxheight;
  frameWidth  = videoCapability.maxwidth;
  
  // select the specified input and video format
  if (!SetChannel(channelNumber)) {
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  } 
  
  if (!SetVideoFormat(videoFormat)) {
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  }
  return TRUE;    
}


BOOL PVideoInputDevice::IsOpen() 
{
    return videoFd >= 0;
}


BOOL PVideoInputDevice::Close()
{
  if (!IsOpen())
    return FALSE;

  ClearMapping();
  ::close(videoFd);
  videoFd = -1;
  canMap  = -1;
  
  return TRUE;
}


BOOL PVideoInputDevice::Start()
{
  return TRUE;
}


BOOL PVideoInputDevice::Stop()
{
  return TRUE;
}


BOOL PVideoInputDevice::IsCapturing()
{
  return IsOpen();
}


PStringList PVideoInputDevice::GetInputDeviceNames()
{
  PDirectory   procvideo("/proc/video/dev");
  PString      entry;
  PStringList  devlist;
  
  if (procvideo.Exists()) {
    if (procvideo.Open(PFileInfo::RegularFile)) {
      do {
        entry = procvideo.GetEntryName();
      
        if (entry.Left(5) == "video") {
          PString thisDevice = "/dev/" + entry;
          int videoFd;  
          if ((videoFd = open(thisDevice, O_RDONLY))) {
            struct video_capability  videoCaps;
            if (ioctl(videoFd, VIDIOCGCAP, &videoCaps) >= 0 &&
                   (videoCaps.type & VID_TYPE_CAPTURE) != 0)
              devlist.AppendString(thisDevice);
            close(videoFd);
          }
        }
      } while (procvideo.Next());
    }
  }
  else {
    // Fallback (no proc file system support or whatever)

    devlist.AppendString("/dev/video0");
    devlist.AppendString("/dev/video1");
  }
  
  return devlist;
}


BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  if (!PVideoDevice::SetVideoFormat(newFormat))
    return FALSE;

  // get channel information (to check if channel is valid)
  struct video_channel channel;
  channel.channel = channelNumber;
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice Get Channel info failed : "<< ::strerror(errno));    
    return FALSE;
  }
  
  // set channel information
  static int fmt[4] = { VIDEO_MODE_PAL, VIDEO_MODE_NTSC, 
                          VIDEO_MODE_SECAM, VIDEO_MODE_AUTO };
  channel.norm = fmt[newFormat];

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) >= 0)
    return TRUE;

  PTRACE(1,"VideoInputDevice SetChannel failed : "<< ::strerror(errno));  

  if (newFormat != Auto)
    return FALSE;

  if (SetVideoFormat(PAL))
    return TRUE;
  if (SetVideoFormat(NTSC))
    return TRUE;
  if (SetVideoFormat(SECAM))
    return TRUE;

  return FALSE;
}


int PVideoInputDevice::GetNumChannels() 
{
  return videoCapability.channels;
}


BOOL PVideoInputDevice::SetChannel(int newChannel)
{
  if (!PVideoDevice::SetChannel(newChannel))
    return FALSE;

  // get channel information (to check if channel is valid)
  struct video_channel channel;
  channel.channel = channelNumber;
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice:: Get Channel info failed : "<< ::strerror(errno));    
    return FALSE;
  }
  
  // set channel information
  channel.channel = channelNumber;

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice:: Setchannel info failed : "<< ::strerror(errno));    
    return FALSE;
  }
  
  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(const PString & newFormat)
{
  PINDEX colourFormatIndex = 0;
  while (newFormat != colourFormatTab[colourFormatIndex].colourFormat) {
    colourFormatIndex++;
    if (colourFormatIndex >= PARRAYSIZE(colourFormatTab))
      return FALSE;
  }

  if (!PVideoDevice::SetColourFormat(newFormat))
    return FALSE;

  ClearMapping();

  // get current picture information
  struct video_picture pictureInfo;
  if (::ioctl(videoFd, VIDIOCGPICT, &pictureInfo) < 0) {
    PTRACE(1,"PVideoInputDevice::Get pict info failed : "<< ::strerror(errno));
    return FALSE;
  }
  
  // set colour format
  colourFormatCode = colourFormatTab[colourFormatIndex].code;
  pictureInfo.palette = colourFormatCode;

  // set the information
  if (::ioctl(videoFd, VIDIOCSPICT, &pictureInfo) < 0) {
    PTRACE(1,"PVideoInputDevice::Set pict info failed : "<< ::strerror(errno));
    return FALSE;
  }
  
  // Some drivers always return success for CSPICT, and can't
  // read the current palette back in CGPICT.  We can't do much
  // more than just check to see if there is a preferred palette,
  // and fail if the request isn't the preferred palette.
  
  if (HINT(HINT_CSPICT_ALWAYS_WORKS) &&
      HINT(HINT_CGPICT_DOESNT_SET_PALETTE) &&
      HINT(HINT_HAS_PREF_PALETTE)) {
      if (colourFormatCode != driver_hints[hint_index].pref_palette)
	  return FALSE;
  }

  // Some V4L drivers can't use CGPICT to check for errors.
  if (!HINT(HINT_CGPICT_DOESNT_SET_PALETTE)) {

      if (::ioctl(videoFd, VIDIOCGPICT, &pictureInfo) < 0) {
	  PTRACE(1,"PVideoInputDevice::Get pict info failed : "<< ::strerror(errno));
	  return FALSE;
      }
      
      if (pictureInfo.palette != colourFormatCode)
	  return FALSE;
  }
  
  // set the new information
  return SetFrameSizeConverter(frameWidth, frameHeight, FALSE);
}


BOOL PVideoInputDevice::SetFrameRate(unsigned rate)
{
  if (!PVideoDevice::SetFrameRate(rate))
    return FALSE;

  return TRUE;
}


BOOL PVideoInputDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  if (!IsOpen())
    return FALSE;

  minWidth  = videoCapability.minwidth;
  minHeight = videoCapability.minheight;
  maxWidth  = videoCapability.maxwidth;
  maxHeight = videoCapability.maxheight;
  return TRUE;

}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  if (!PVideoDevice::SetFrameSize(width, height))
    return FALSE;
  
  ClearMapping();
  
  if (!VerifyHardwareFrameSize(width, height))
    return FALSE;

  frameBytes = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  
  return TRUE;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  if (converter != NULL) {
    PINDEX bytes = converter->GetMaxDstFrameBytes();
    if (bytes > frameBytes)
      return bytes;
  }

  return frameBytes;
}



BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  if (canMap < 0) {

    if (::ioctl(videoFd, VIDIOCGMBUF, &frame) < 0)
      cout << "VIDIOCGMBUF failed" << endl;
    else {
      videoBuffer = (BYTE *)::mmap(0, frame.size, PROT_READ|PROT_WRITE, MAP_SHARED, videoFd, 0);

      if (videoBuffer < 0) {
        canMap = 0;

      } else {
        canMap = 1;

        frameBuffer[0].frame  = 0;
        frameBuffer[0].format = colourFormatCode;
        frameBuffer[0].width  = frameWidth;
        frameBuffer[0].height = frameHeight;
	
        frameBuffer[1].frame  = 1;
        frameBuffer[1].format = colourFormatCode;
        frameBuffer[1].width  = frameWidth;
        frameBuffer[1].height = frameHeight;

	if ((::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[0]) != 0) ||
	    (::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[1]) != 0)) {
	    PTRACE(1,"PVideoInputDevice::GetFrameData fallback to read() (A)");
	  canMap = 0;
	  ::munmap(videoBuffer, frame.size);
	  videoBuffer = NULL;
	}
	
        currentFrame = 0;
      }
    }
  }

  // device does not support memory mapping - use normal read
  // take care over signals.
  if (canMap == 0) {

    ssize_t ret;

    ret = -1;
    while (ret < 0) {
      ret = ::read(videoFd, buffer, frameBytes);
      if ((ret < 0) && (errno == EINTR))
	continue;
    
      if (ret < 0) {
	PTRACE(1,"PVideoInputDevice::GetFrameData read() failed");
	return FALSE;
      }
      
    }

    if ((unsigned) ret != frameBytes) {
      PTRACE(1,"PVideoInputDevice::GetFrameData read() returned a short read");
      // Not a completely fatal. Maybe it should return FALSE instead of a partial
      // image though?
      // return FALSE;
    }
    
    if (converter != NULL)
      return converter->ConvertInPlace(buffer, bytesReturned);
    if (bytesReturned != NULL)
      *bytesReturned = frameBytes;
    return TRUE;
  }
  
  // device does support memory mapping, get data

  // wait for the frame to load.  Careful about signals.
  int ret = -1;
  while (ret < 0) {
    ret = ::ioctl(videoFd, VIDIOCSYNC, &currentFrame);
    if ((ret < 0) && (errno == EINTR))
      continue;
    
    if (ret < 0) {
      PTRACE(1,"PVideoInputDevice::GetFrameData fallback to read() (CSYNC failed)");
      canMap = 0;
      ::munmap(videoBuffer, frame.size);
      videoBuffer = NULL;
      return FALSE;
    }
  }
  
  // If converting on the fly do it from frame store to output buffer, otherwise do
  // straight copy.
  if (converter != NULL)
    converter->Convert(videoBuffer + frame.offsets[currentFrame], buffer, bytesReturned);
  else {
    memcpy(buffer, videoBuffer + frame.offsets[currentFrame], frameBytes);
    if (bytesReturned != NULL)
      *bytesReturned = frameBytes;
  }
  
  // trigger capture of next frame in this buffer.
  // fallback to read() on errors.
  if (::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[currentFrame]) != 0) {
    PTRACE(1,"PVideoInputDevice::GetFrameData fallback to read() (B)");
    canMap = 0;
    ::munmap(videoBuffer, frame.size);
    videoBuffer = NULL;
  }
  
  // change buffers
  currentFrame = 1 - currentFrame;

  return TRUE;
}


void PVideoInputDevice::ClearMapping()
{
  if (canMap == 1) {
    if (videoBuffer != NULL)
      ::munmap(videoBuffer, frame.size);
  }
  
  canMap = -1;
  videoBuffer = NULL;
}


BOOL PVideoInputDevice::VerifyHardwareFrameSize(unsigned width,
						unsigned height)
{
    struct video_window vwin;
    
    // Request current hardware frame size
    if (::ioctl(videoFd, VIDIOCGWIN, &vwin) < 0)
	return FALSE;
    
    // Request the width and height
    vwin.width  = width;
    vwin.height = height;
    
    // The only defined flags appear to be as status indicators
    // returned in the CGWIN call.  At least the bttv driver fails
    // when flags isn't zero.  Check the driver hints for clearing
    // the flags.
    if (HINT(HINT_CSWIN_ZERO_FLAGS)) {
	PTRACE(1,"PVideoInputDevice::VerifyHardwareFrameSize: Clearing flags field");
	vwin.flags = 0;
    }
    
    ::ioctl(videoFd, VIDIOCSWIN, &vwin);
    
    // Read back settings to be careful about existing (broken) V4L drivers
    if (::ioctl(videoFd, VIDIOCGWIN, &vwin) < 0)
	return FALSE;
    
    if ((vwin.width != width) || (vwin.height != height))
      return FALSE;

    return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
