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
 * Revision 1.38  2003/03/17 07:52:52  robertj
 * Removed canCaptureVideo variable as this is really a virtual function to
 *   distinguish PVideoOutputDevice from PVideoInputDevice, it is not dynamic.
 *
 * Revision 1.37  2003/03/06 02:43:43  dereks
 * Make error messages slightly more descriptive.
 *
 * Revision 1.36  2002/09/09 22:16:54  dereks
 * update hints for spca50x camera.
 *
 * Revision 1.35  2002/09/08 22:01:56  dereks
 * Add support for Intel PC Pro Camera, spca50x driver, Thanks Dennis Gilmore!
 *
 * Revision 1.34  2002/06/05 12:29:16  craigs
 * Changes for gcc 3.1
 *
 * Revision 1.33  2002/04/21 22:02:35  dereks
 * Tidy up test for existance of video devices. Thanks Guilhem Tardy.
 *
 * Revision 1.32  2002/04/17 21:54:06  dereks
 * Improve searching of proc file system for video device names. Thanks Guilhem Tardy.
 *
 * Revision 1.31  2002/04/13 07:54:38  rogerh
 * Add CPiA camera hint to work around driver bug.
 * From Damien Sandras and Keith Packard.
 *
 * Revision 1.30  2002/04/08 21:18:24  rogerh
 * Emulate original behaviour of pwlib when Open and then SetVideoFormat
 * are called. Tested by Mark Cooke.
 *
 * Revision 1.29  2002/04/05 06:41:54  rogerh
 * Apply video changes from Damien Sandras <dsandras@seconix.com>.
 * The Video Channel and Format are no longer set in Open(). Instead
 * call the new SetVideoChannelFormat() method. This makes video capture
 * and GnomeMeeting more stable with certain Linux video capture devices.
 *
 * Revision 1.28  2002/01/26 23:58:15  craigs
 * Changed for GCC 3.0 compatibility, thanks to manty@manty.net
 *
 * Revision 1.27  2002/01/16 03:43:01  dereks
 * Match every VIDIOCMCAPTURE with a VIDIOCSYNC.
 *
 * Revision 1.26  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
 * Revision 1.25  2001/12/10 22:22:48  dereks
 * Add hint so Logitech USB Camera will only grab in large video size.
 *
 * Revision 1.24  2001/12/08 00:59:44  robertj
 * Added hint for BT879 chips, thanks Damian Sandras.
 *
 * Revision 1.23  2001/12/06 22:15:09  dereks
 * Additional debugging lines
 *
 * Revision 1.22  2001/11/30 00:14:46  dereks
 * Fix frame rate limitation.
 *
 * Revision 1.21  2001/11/28 05:10:19  robertj
 * Added enable of TV card sound when grabbing, thanks Santiago Garcia Mantinan
 *
 * Revision 1.20  2001/11/28 00:07:32  dereks
 * Locking added to PVideoChannel, allowing reader/writer to be changed mid call
 * Enabled adjustment of the video frame rate
 * New fictitous image, a blank grey area
 *
 * Revision 1.19  2001/11/05 01:03:20  dereks
 * Fix error in collection of video data. Frame rate is now double of that
 * obtained previously.
 *
 * Revision 1.18  2001/08/22 02:04:43  robertj
 * Resolved confusion with YUV411P and YUV420P video formats, thanks Mark Cooke.
 *
 * Revision 1.17  2001/08/20 07:01:26  robertj
 * Fixed wierd problems with YUV411P and YUV420P formats, thanks Mark Cooke.
 *
 * Revision 1.16  2001/08/03 04:21:51  dereks
 * Add colour/size conversion for YUV422->YUV411P
 * Add Get/Set Brightness,Contrast,Hue,Colour for PVideoDevice,  and
 * Linux PVideoInputDevice.
 * Add lots of PTRACE statement for debugging colour conversion.
 * Add support for Sony Vaio laptop under linux. Requires 2.4.7 kernel.
 *
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
#include <sys/time.h>


///////////////////////////////////////////////////////////////////////////////
// Linux Video4Linux Driver Hints Tables.
//
// In an ideal API, we wouldn't need these hints on setup.  There are enough
// wrinkles it seems we have to provide a static list of hints for known
// issues.

#define HINT_CSWIN_ZERO_FLAGS               0x0001
#define HINT_CSPICT_ALWAYS_WORKS            0x0002  /// ioctl return value indicates pict was set ok.
#define HINT_CGPICT_DOESNT_SET_PALETTE      0x0004
#define HINT_HAS_PREF_PALETTE               0x0008  /// use this palette with this camera.
#define HINT_ALWAYS_WORKS_320_240           0x0010  /// Camera always  opens OK at this size.
#define HINT_ALWAYS_WORKS_640_480           0x0020  /// Camera always  opens OK at this size.
#define HINT_ONLY_WORKS_PREF_PALETTE        0x0040  /// Camera always (and only) opens at pref palette.
#define HINT_CGWIN_FAILS                    0x0080  /// ioctl VIDIOCGWIN always fails.
#define HINT_FORCE_LARGE_SIZE               0x0100  /// driver does not work in small video size.
#define HINT_FORCE_DEPTH_16                 0x0200  /// CPiA cameras return a wrong value for the depth, and if you try to use that wrong value, it fails.

static struct {
  char     *name_regexp;        // String used to match the driver name
  char     *name;               // String used for ptrace output
  unsigned hints;               // Hint flags
  int      pref_palette;        // Preferred palette.
} driver_hints[] = {

    /**Philips usb web cameras
       Native format is 420(P) so use it.
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
  { "^BT8(4|7)(8|9)",
    "Brooktree BT848 and BT878 based capture boards",
    HINT_CSWIN_ZERO_FLAGS |
    HINT_CSPICT_ALWAYS_WORKS |
    HINT_CGPICT_DOESNT_SET_PALETTE |
    HINT_HAS_PREF_PALETTE,
    VIDEO_PALETTE_YUV420P },

  /** Sony Vaio Motion Eye camera
      Linux kernel 2.4.7 has meye.c driver module.
   */
  { "meye",
    "Sony Vaio Motion Eye Camera",
    HINT_CGPICT_DOESNT_SET_PALETTE |
    HINT_CSPICT_ALWAYS_WORKS       |
    HINT_ALWAYS_WORKS_320_240      |
    HINT_ALWAYS_WORKS_640_480      |
    HINT_CGWIN_FAILS               |
    HINT_ONLY_WORKS_PREF_PALETTE   |
    HINT_HAS_PREF_PALETTE,
    VIDEO_PALETTE_YUV422 },

  /** USB camera, which only works in large size.
   */
  { "Logitech USB Webcam",
    "Logitech USB Webcam which works in large size only",
    HINT_FORCE_LARGE_SIZE,
    VIDEO_PALETTE_YUV420P 
  },

  /** Creative VideoBlaster Webcam II USB
   */
  {"CPiA Camera",
   "CPIA which works with cpia and cpia_usb driver modules",
   HINT_FORCE_DEPTH_16 |
   HINT_ONLY_WORKS_PREF_PALETTE   |
   HINT_HAS_PREF_PALETTE,
   VIDEO_PALETTE_YUV422
  },

 /** Intel PC Pro Camera
 
 */
  { "SPCA50X USB Camera",
    "Intel PC Pro Camera uses the spca50x driver",
    HINT_ONLY_WORKS_PREF_PALETTE        |
    HINT_HAS_PREF_PALETTE,
    VIDEO_PALETTE_RGB24
  },


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
  hint_index    = PARRAYSIZE(driver_hints) - 1;

  canMap           = -1;
  for (int i=0; i<2; i++)
    pendingSync[i] = FALSE;
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
  
  if ((videoCapability.type & VID_TYPE_CAPTURE) != 0) {
    PTRACE(1,"PVideoInputDevice:: device capablilities reports cannot capture");
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  }

  hint_index = PARRAYSIZE(driver_hints) - 1;
  PString driver_name(videoCapability.name);  

  // Scan the hint table, looking for regular expression matches with
  // drivers we hold hints for.
  PINDEX tbl;
  for (tbl = 0; tbl < PARRAYSIZE(driver_hints); tbl ++) {
    PRegularExpression regexp;
    regexp.Compile(driver_hints[tbl].name_regexp, PRegularExpression::Extended);

    if (driver_name.FindRegEx(regexp) != P_MAX_INDEX) {
      PTRACE(1,"PVideoInputDevice::Open: Found driver hints: " << driver_hints[tbl].name);
      PTRACE(1,"PVideoInputDevice::Open: format: " << driver_hints[tbl].pref_palette);
      hint_index = tbl;
      break;
    }
  }

  // set height and width
  frameHeight = videoCapability.maxheight;
  frameWidth  = videoCapability.maxwidth;
  

  // Init audio
  struct video_audio videoAudio;
  if (::ioctl(videoFd, VIDIOCGAUDIO, &videoAudio) >= 0 &&
                      (videoAudio.flags & VIDEO_AUDIO_MUTABLE) != 0) {
    videoAudio.flags &= ~VIDEO_AUDIO_MUTE;
    videoAudio.mode = VIDEO_SOUND_MONO;
    ::ioctl(videoFd, VIDIOCSAUDIO, &videoAudio);
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


  // Mute audio
  struct video_audio videoAudio;
  if (::ioctl(videoFd, VIDIOCGAUDIO, &videoAudio) >= 0 &&
                      (videoAudio.flags & VIDEO_AUDIO_MUTABLE) != 0) {
    videoAudio.flags |= VIDEO_AUDIO_MUTE;
    ::ioctl(videoFd, VIDIOCSAUDIO, &videoAudio);
  }

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

	if ((entry.Left(5) == "video") || (entry.Left(7) == "capture")) {
	  PString thisDevice = "/dev/video" + entry.Right(1);
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
  } else {
    // Fallback (no proc file system support or whatever)

    devlist.AppendString("/dev/video0");
    devlist.AppendString("/dev/video1");
  }
  
  return devlist;
}

BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  if (!PVideoDevice::SetVideoFormat(newFormat)) {
    PTRACE(1,"PVideoDevice::SetVideoFormat\t failed for format "<<newFormat);
    return FALSE;
  }

  // The channel and format are both set at the same time with one ioctl().
  // Get the channel information (to check if channel is valid)
  // Note: If the channel is -1, we need to search for the first valid channel
  if (channelNumber == -1) {
    if (!SetChannel(channelNumber)){
      PTRACE(1,"PVideoDevice::Cannot set default channel in SetVideoFormat");
      return FALSE;
    }
  }

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
  /* If Opened, return the capability value, else 1 as in videoio.cxx */
  if (IsOpen ())
    return videoCapability.channels;
  else
    return 1;
}


BOOL PVideoInputDevice::SetChannel(int newChannel)
{
  if (!PVideoDevice::SetChannel(newChannel))
    return FALSE;

  // get channel information (to check if channel is valid)
  struct video_channel channel;
  channel.channel = channelNumber;
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice:: Get info on channel " << channelNumber << " failed : "<< ::strerror(errno));    
    return FALSE;
  }
  
  // set channel information
  channel.channel = channelNumber;

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice:: Set info on channel " << channelNumber << " failed : "<< ::strerror(errno));    
    return FALSE;
  }
  
  return TRUE;
}


BOOL PVideoInputDevice::SetVideoChannelFormat (int newNumber, VideoFormat videoFormat) 
{
  if (!PVideoDevice::SetChannel(newNumber))
    return FALSE;

  if (!PVideoDevice::SetVideoFormat(videoFormat)) {
    PTRACE(1,"PVideoDevice::SetVideoFormat\t failed for format "<<videoFormat);
    return FALSE;
  }

  static int fmt[4] = { VIDEO_MODE_PAL, VIDEO_MODE_NTSC, 
                          VIDEO_MODE_SECAM, VIDEO_MODE_AUTO };

  // select the specified input and video format
  // get channel information (to check if channel is valid)
  struct video_channel channel;
  channel.channel = channelNumber;
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice Get Channel info failed : "<< ::strerror(errno));    

    return FALSE;
  }
  
  // set channel information
  channel.norm = fmt[videoFormat];
  channel.channel = channelNumber;

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0) {
    PTRACE(1,"VideoInputDevice SetChannel failed : "<< ::strerror(errno));  

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
  if (HINT (HINT_FORCE_DEPTH_16))
    pictureInfo.depth = 16;

  // set the information
  if (::ioctl(videoFd, VIDIOCSPICT, &pictureInfo) < 0) {
    PTRACE(1,"PVideoInputDevice::Set pict info failed : "<< ::strerror(errno));
    PTRACE(1,"PVideoInputDevice:: used code of "<<colourFormatCode);
    PTRACE(1,"PVideoInputDevice:: palette: "<<colourFormatTab[colourFormatIndex].colourFormat);
    return FALSE;
  }
  

  // Driver only (and always) manages to set the colour format  with call to VIDIOCSPICT.
  if( (HINT(HINT_ONLY_WORKS_PREF_PALETTE) ) &&                   
      ( colourFormatCode == driver_hints[hint_index].pref_palette) ) {
    PTRACE(3,"PVideoInputDevice:: SetColourFormat succeeded with "<<newFormat);
    return TRUE;
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

  if(HINT(HINT_FORCE_LARGE_SIZE)) {
    videoCapability.maxheight = 288;
    videoCapability.maxwidth  = 352;
    videoCapability.minheight = 288;
    videoCapability.minwidth  = 352;
  }

  maxHeight = videoCapability.maxheight;
  maxWidth  = videoCapability.maxwidth;
  minHeight = videoCapability.minheight;
  minWidth  = videoCapability.minwidth;
    
  PTRACE(3,"PVideoInputDevice:\t GetFrameSizeLimits. "<<minWidth<<"x"<<minHeight<<" -- "<<maxWidth<<"x"<<maxHeight);
  
  return TRUE;
}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  PTRACE(5, "PVideoInputDevice\t SetFrameSize " << width <<"x"<<height << " Initiated.");
  if (!PVideoDevice::SetFrameSize(width, height)) {
    PTRACE(3,"PVideoInputDevice\t SetFrameSize "<<width<<"x"<<height<<" FAILED");
    return FALSE;
  }

  ClearMapping();
  
  if (!VerifyHardwareFrameSize(width, height)) {
    PTRACE(3,"PVideoInputDevice\t SetFrameSize failed for "<<width<<"x"<<height);
    PTRACE(3,"VerifyHardwareFrameSize failed.");
    return FALSE;
  }

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
  if(frameRate>0) {
    frameTimeError += msBetweenFrames;
   
    do {
      if ( !GetFrameDataNoDelay(buffer, bytesReturned))
	return FALSE;      
      PTime now;
      PTimeInterval delay = now - previousFrameTime;
      frameTimeError -= (int)delay.GetMilliSeconds();
      previousFrameTime = now;
    }  while(frameTimeError > 0) ;

    return TRUE;
  }
  return GetFrameDataNoDelay(buffer,bytesReturned);
}


BOOL PVideoInputDevice::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  if (canMap < 0) {
    //When canMap is < 0, it is the first use of GetFrameData. Check for memory mapping.
    if (::ioctl(videoFd, VIDIOCGMBUF, &frame) < 0) {
      canMap=0;
      PTRACE(3, "VideoGrabber " << deviceName << " cannot do memory mapping - GMBUF failed.");
      //This video device cannot do memory mapping.
    } else {
      videoBuffer = (BYTE *)::mmap(0, frame.size, PROT_READ|PROT_WRITE, MAP_SHARED, videoFd, 0);
      
      if (videoBuffer < 0) {
        canMap = 0;
	PTRACE(3, "VideoGrabber " << deviceName << " cannot do memory mapping - ::mmap failed.");
	//This video device cannot do memory mapping.
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

        currentFrame = 0;
	int ret;
        ret = ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[currentFrame]);
	if (ret < 0) {
	  PTRACE(1,"PVideoInputDevice::GetFrameData mcapture1 failed : " << ::strerror(errno));
	  ClearMapping();	  
	  canMap = 0;
	  //This video device cannot do memory mapping.
	}
	pendingSync[currentFrame] = TRUE;
      }
    }
  }

  if (canMap == 0) 
    return NormalReadProcess(buffer, bytesReturned);

  /*****************************
   * The xawtv package from http://bytesex.org/xawtv/index.html
   * contains a programming-FAQ by Gerd Knorr.
   * For streaming video with video4linux at the full frame rate 
   * (25 hz PAL, 30 hz NTSC) you need to, 
   *
   *   videoiomcapture frame 0                         (setup)
   *
   * loop:
   *   videoiomcapture frame 1   (returns immediately)
   *   videoiocsync    frame 0   (waits on the data)
   *  goto loop:
   *
   * the loop body could also have been:
   *   videoiomcapture frame 0   (returns immediately)
   *   videoiocsync    frame 1   (waits on the data)
   *  
   * The driver requires each mcapture has a corresponding sync. 
   * Thus, you use the pendingSync array.
   *
   * After the loop is finished, you need a videoiocsync 0.
   */

  // trigger capture of next frame in this buffer.
  // fallback to read() on errors.
  int ret = -1;
  
  ret = ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[ 1 - currentFrame ]);
  if ( ret < 0 ) {
    PTRACE(1,"PVideoInputDevice::GetFrameData mcapture2 failed : " << ::strerror(errno));
    ClearMapping();
    canMap = 0;
    
    return NormalReadProcess(buffer, bytesReturned);
  }
  pendingSync[ 1 - currentFrame ] = TRUE;
  
  // device does support memory mapping, get data

  // wait for the frame to load. 
  ret = ::ioctl(videoFd, VIDIOCSYNC, &currentFrame);
  pendingSync[currentFrame] = FALSE;    
  if (ret < 0) {
    PTRACE(1,"PVideoInputDevice::GetFrameData csync failed : " << ::strerror(errno));
    ClearMapping();
    canMap = 0;
 
    return NormalReadProcess(buffer, bytesReturned);
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
  
  // change buffers
  currentFrame = 1 - currentFrame;

  return TRUE;
}

//This video device does not support memory mapping - so 
// use normal read process to extract a frame of video data.
BOOL PVideoInputDevice::NormalReadProcess(BYTE *resultBuffer, PINDEX *bytesReturned)
{ 
   ssize_t ret;
   ret = -1;
   while (ret < 0) {

     ret = ::read(videoFd, resultBuffer, frameBytes);
     if ((ret < 0) && (errno == EINTR))
	continue;
    
      if (ret < 0) {
	PTRACE(1,"PVideoInputDevice::NormalReadProcess() failed");
	return FALSE;
      }      
    }

    if ((PINDEX)ret != frameBytes) {
      PTRACE(1,"PVideoInputDevice::NormalReadProcess() returned a short read");
      // Not a completely fatal. Maybe it should return FALSE instead of a partial
      // image though?
      // return FALSE;
    }
    
    if (converter != NULL)
      return converter->ConvertInPlace(resultBuffer, bytesReturned);

    if (bytesReturned != NULL)
      *bytesReturned = frameBytes;

    return TRUE;
}

void PVideoInputDevice::ClearMapping()
{
  if ((canMap == 1) && (videoBuffer != NULL)) {
    for (int i=0; i<2; i++)
      if (pendingSync[i]) {
	int res = ::ioctl(videoFd, VIDIOCSYNC, &i);
	if (res < 0) 
	  PTRACE(1,"PVideoInputDevice::GetFrameData csync failed : " << ::strerror(errno));
	pendingSync[i] = FALSE;    
      }
    ::munmap(videoBuffer, frame.size);
  }
  
  canMap = -1;   
  videoBuffer = NULL;
}



BOOL PVideoInputDevice::VerifyHardwareFrameSize(unsigned width,
						unsigned height)
{
  struct video_window vwin;

  if (HINT(HINT_FORCE_LARGE_SIZE))
    if(  (width==352) && (height==288) ) {
      PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize USB OK  352x288 ");
      return TRUE;
    } else {
      PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize USB FAIL "<<width<<"x"<<height);
      return FALSE;
    }
    
  if (HINT(HINT_ALWAYS_WORKS_320_240) &&  (width==320) && (height==240) ) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize OK  for  320x240 ");
    return TRUE;
  }
    
  if (HINT(HINT_ALWAYS_WORKS_640_480) &&  (width==640) && (height==480) ) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize OK for 640x480 ");
    return TRUE;
  }
     
  if (HINT(HINT_CGWIN_FAILS)) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize fails for size "
            << width << "x" << height);
    return FALSE;
  }
  
  // Request current hardware frame size
  if (::ioctl(videoFd, VIDIOCGWIN, &vwin) < 0) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize VIDIOCGWIN error::" << ::strerror(errno));
    return FALSE;
  }

  // Request the width and height
  vwin.width  = width;
  vwin.height = height;
  
  // The only defined flags appear to be as status indicators
  // returned in the CGWIN call.  At least the bttv driver fails
  // when flags isn't zero.  Check the driver hints for clearing
  // the flags.
  if (HINT(HINT_CSWIN_ZERO_FLAGS)) {
    PTRACE(1,"PVideoInputDevice\t VerifyHardwareFrameSize: Clearing flags field");
    vwin.flags = 0;
  }
  
  ::ioctl(videoFd, VIDIOCSWIN, &vwin);
  
  // Read back settings to be careful about existing (broken) V4L drivers
  if (::ioctl(videoFd, VIDIOCGWIN, &vwin) < 0) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize VIDIOCGWIN error::" << ::strerror(errno));
    return FALSE;
  }
  
  if ((vwin.width != width) || (vwin.height != height)) {
    PTRACE(3,"PVideoInputDevice\t VerifyHardwareFrameSize Size mismatch.");
    return FALSE;
  }

  return TRUE;
}

int PVideoInputDevice::GetBrightness() 
{ 
  if (!IsOpen())
    return -1;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return -1;
  frameBrightness = vp.brightness;

  return frameBrightness; 
}


int PVideoInputDevice::GetWhiteness() 
{ 
  if (!IsOpen())
    return -1;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return -1;
  frameWhiteness = vp.whiteness;

  return frameWhiteness;
}

int PVideoInputDevice::GetColour() 
{ 
  if (!IsOpen())
    return -1;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return -1;
  frameColour = vp.colour;

  return frameColour; 
}



int PVideoInputDevice::GetContrast() 
{
  if (!IsOpen())
    return -1;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return -1;
  frameContrast = vp.contrast;

 return frameContrast; 
}

int PVideoInputDevice::GetHue() 
{
  if (!IsOpen())
    return -1;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return -1;
  frameHue = vp.hue;

  return frameHue; 
}

BOOL PVideoInputDevice::SetBrightness(unsigned newBrightness) 
{ 
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  vp.brightness = newBrightness;
  if (::ioctl(videoFd, VIDIOCSPICT, &vp) < 0)
    return FALSE;

  frameBrightness=newBrightness;
  return TRUE;
}
BOOL PVideoInputDevice::SetWhiteness(unsigned newWhiteness) 
{ 
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  vp.whiteness = newWhiteness;
  if (::ioctl(videoFd, VIDIOCSPICT, &vp) < 0)
    return FALSE;

  frameWhiteness = newWhiteness;
  return TRUE;
}

BOOL PVideoInputDevice::SetColour(unsigned newColour) 
{ 
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  vp.colour = newColour;
  if (::ioctl(videoFd, VIDIOCSPICT, &vp) < 0)
    return FALSE;

  frameColour = newColour;
  return TRUE;
}
BOOL PVideoInputDevice::SetContrast(unsigned newContrast) 
{ 
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  vp.contrast = newContrast;
  if (::ioctl(videoFd, VIDIOCSPICT, &vp) < 0)
    return FALSE;

  frameContrast = newContrast;
  return TRUE;
}

BOOL PVideoInputDevice::SetHue(unsigned newHue) 
{
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  vp.hue = newHue;
  if (::ioctl(videoFd, VIDIOCSPICT, &vp) < 0)
    return FALSE;

   frameHue=newHue; 
  return TRUE;
}

BOOL PVideoInputDevice::GetParameters (int *whiteness, int *brightness, 
                                      int *colour, int *contrast, int *hue)
{
  if (!IsOpen())
    return FALSE;

  struct video_picture vp;

  if (::ioctl(videoFd, VIDIOCGPICT, &vp) < 0)
    return FALSE;

  *brightness = vp.brightness;
  *colour     = vp.colour;
  *contrast   = vp.contrast;
  *hue        = vp.hue;
  *whiteness  = vp.whiteness;

  frameBrightness = *brightness;
  frameColour     = *colour;
  frameContrast   = *contrast;
  frameHue        = *hue;
  frameWhiteness  = *whiteness;
  
  return TRUE;
}

BOOL PVideoInputDevice::TestAllFormats()
{
  return TRUE;
}
// End Of File ///////////////////////////////////////////////////////////////
