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

PVideoInputDevice::PVideoInputDevice(VideoFormat videoFmt,
                                     int channel,
                                     ColourFormat colourFmt)
  : PVideoDevice(videoFmt, channel, colourFmt)
{
  videoFd     = -1;
  canMap      = -1;
  
  conversion = NULL;
}


static struct {
  int palette;
  int bitsPerPixel;
} colourFormatTab[PVideoInputDevice::NumColourFormats] = {
  { VIDEO_PALETTE_GREY,    8 },  //Entries in this table correspond
  { VIDEO_PALETTE_RGB32,   32 }, //(line by line) to those in the 
  { VIDEO_PALETTE_RGB24,   24 }, // PVideoDevice ColourFormat table.
  { VIDEO_PALETTE_RGB565,  16 },
  { VIDEO_PALETTE_RGB555,  16 },
  { VIDEO_PALETTE_YUV422,  16 },
  { VIDEO_PALETTE_YUV422P, 16 },
  { VIDEO_PALETTE_YUV411,  12 },
  { VIDEO_PALETTE_YUV411P, 12 },
  { VIDEO_PALETTE_YUV420,  12 },
  { VIDEO_PALETTE_YUV420P, 12 },
  { 0,  0 },
  { VIDEO_PALETTE_YUV410P, 10 }
};


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  if( channelNumber < 0 )
    return FALSE;

  Close();

  deviceName = devName;
  videoFd = ::open((const char *)devName, O_RDWR);
  if (videoFd < 0)
    return FALSE;
  
  // get the device capabilities
  if (::ioctl(videoFd, VIDIOCGCAP, &videoCapability) >= 0) {

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
  
	  if ( !SetColourFormat(colourFormat)) {
	    //OK Try some others.
	    if ( !SetColourFormat(YUV422)) {
   	    ::close (videoFd);
	      videoFd = -1;
	      return FALSE;
	    }
	  conversion = new PVideoConvert(YUV422,colourFormat,frameWidth,frameHeight);
	  }
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


PStringList PVideoInputDevice::GetDeviceNames() const
{
  PStringList list;

  list.AppendString("/dev/video0");
  list.AppendString("/dev/video1");

  return list;
}


BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  if (!PVideoDevice::SetVideoFormat(newFormat))
    return FALSE;

  // get channel information (to check if channel is valid)
  struct video_channel channel;
  channel.channel = channelNumber;
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0)
    return FALSE;

  // set channel information
  static int fmt[4] = { VIDEO_MODE_PAL, VIDEO_MODE_NTSC, 
                          VIDEO_MODE_SECAM, VIDEO_MODE_AUTO };
  channel.norm = fmt[newFormat];

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0)
    return FALSE;

  return TRUE;  
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
  if (::ioctl(videoFd, VIDIOCGCHAN, &channel) < 0)
    return FALSE;

  // set channel information
  channel.channel = channelNumber;

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0)
    return FALSE;

  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(ColourFormat newFormat)
{
  if (!PVideoDevice::SetColourFormat(newFormat))
    return FALSE;

  ClearMapping();

  // get picture information
  struct video_picture pictureInfo;
  if (::ioctl(videoFd, VIDIOCGPICT, &pictureInfo) < 0)
    return FALSE;

  // set colour format
  pictureInfo.palette = colourFormatTab[newFormat].palette;

  // set the information
  if (::ioctl(videoFd, VIDIOCSPICT, &pictureInfo) < 0)
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

  videoFrameSize = CalcFrameSize ( frameWidth, frameHeight, (int)colourFormat);
  
  return TRUE;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return videoFrameSize;
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
        frameBuffer[0].format = colourFormatTab[colourFormat].palette;
        frameBuffer[0].width  = frameWidth;
        frameBuffer[0].height = frameHeight;
        ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[0]);

        frameBuffer[1].frame  = 1;
        frameBuffer[1].format = colourFormatTab[colourFormat].palette;
        frameBuffer[1].width  = frameWidth;
        frameBuffer[1].height = frameHeight;
        ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[1]);

        currentFrame = 0;
      }
    }
  }

  if (bytesReturned != NULL)
    *bytesReturned = videoFrameSize;

  // device does not support memory mapping - use normal read
  if (canMap == 0)
    return (PINDEX)::read(videoFd, buffer, videoFrameSize) == videoFrameSize;

  // device does support memory mapping, get data

  // wait for the frame to load
  ::ioctl(videoFd, VIDIOCSYNC, currentFrame);

  // copy the frame to our storage
  memcpy(buffer, videoBuffer + frame.offsets[currentFrame], videoFrameSize);

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
