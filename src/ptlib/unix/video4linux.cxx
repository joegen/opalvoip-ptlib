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
 * Contributor(s): ______________________________________.
 *
 * $Log: video4linux.cxx,v $
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
// PVideoInputDevice

PVideoInputDevice::PVideoInputDevice()
{
  videoFd       = -1;
  canMap        = -1;
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

  // get picture information
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
  
  // Some V4L drivers are lax about returning errors on VIDIOCSPICT
  if (::ioctl(videoFd, VIDIOCGPICT, &pictureInfo) < 0)
    return FALSE;
  
  if (pictureInfo.palette != colourFormatCode)
    return FALSE;

  // set the new information
  return SetFrameSize(frameWidth, frameHeight);
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
        ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[0]);

        frameBuffer[1].frame  = 1;
        frameBuffer[1].format = colourFormatCode;
        frameBuffer[1].width  = frameWidth;
        frameBuffer[1].height = frameHeight;
        ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[1]);

        currentFrame = 0;
      }
    }
  }

  // device does not support memory mapping - use normal read
  if (canMap == 0) {
    if ((PINDEX)::read(videoFd, buffer, frameBytes) != frameBytes)
      return FALSE;
    if (converter != NULL)
      return converter->ConvertInPlace(buffer, bytesReturned);
    if (bytesReturned != NULL)
      *bytesReturned = frameBytes;
    return TRUE;
  }

  // device does support memory mapping, get data

  // wait for the frame to load
  ::ioctl(videoFd, VIDIOCSYNC, currentFrame);

  // If converting on the fly do it from frame store to output buffer, otherwise do
  // straight copy.
  if (converter != NULL)
    converter->Convert(videoBuffer + frame.offsets[currentFrame], buffer, bytesReturned);
  else {
    memcpy(buffer, videoBuffer + frame.offsets[currentFrame], frameBytes);
    if (bytesReturned != NULL)
      *bytesReturned = frameBytes;
  }


  // trigger capture of next frame in this buffer
  ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[currentFrame]);

  // change buffers
  currentFrame = 1 - currentFrame;

  return TRUE;
}


void PVideoInputDevice::ClearMapping()
{
  if (canMap == 1) {
    if (videoBuffer != NULL)
      ::munmap(videoBuffer, frame.size);

    canMap = -1;
    videoBuffer = NULL;
  }
}


    
// End Of File ///////////////////////////////////////////////////////////////
