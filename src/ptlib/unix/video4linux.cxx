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

#include <sys/mman.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoInputDevice::PVideoInputDevice(VideoFormat videoFmt,
                                     unsigned channel,
                                     ColourFormat colourFmt)
  : PVideoDevice(videoFmt, channel, colourFmt)
{
  videoFd = -1;
  canMap  = -1;
}


static int colourFormatTab[PVideoInputDevice::NumColourFormats][2] = {
  { VIDEO_PALETTE_GREY,   1 },
  { VIDEO_PALETTE_RGB24,  3 },
  { VIDEO_PALETTE_RGB32,  4 },
  { VIDEO_PALETTE_YUV422, 2 },
  { VIDEO_PALETTE_RGB565, 2 }
};


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  Close();

  deviceName = devName;

  // open the device
  videoFd = ::open((const char *)devName, O_RDWR);
  if (videoFd < 0)
    return FALSE;

  // get the device capabilities
  if (::ioctl(videoFd, VIDIOCGCAP, &videoCapability) >= 0) {

    // set height and width
    frameHeight = videoCapability.maxheight;
    frameWidth  = videoCapability.maxwidth;

    // select the specified input and video format
    if (SetChannel(channelNumber) && SetVideoFormat(videoFormat)) {

      // get picture information
      struct video_picture pictureInfo;
      if (::ioctl(videoFd, VIDIOCGPICT, &pictureInfo) >= 0) {

        // set colour format
        PINDEX i;
        for (i = 0; i < NumVideoFormats; i++)
          if (colourFormatTab[i][0] == pictureInfo.palette)
            break;
        if (i < NumVideoFormats)
          return SetColourFormat((ColourFormat)i);
        else
          return SetColourFormat(RGB24);
      }
    }
  }

  ::close(videoFd);
  videoFd = -1;
  return FALSE;
}


BOOL PVideoInputDevice::IsOpen() const
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
  static int fmt[4] = { VIDEO_MODE_PAL, VIDEO_MODE_NTSC, VIDEO_MODE_SECAM, VIDEO_MODE_AUTO };
  channel.norm = fmt[newFormat];

  // set the information
  if (::ioctl(videoFd, VIDIOCSCHAN, &channel) < 0)
    return FALSE;

  return TRUE;
}


unsigned PVideoInputDevice::GetNumChannels() const
{
  return videoCapability.channels;
}


BOOL PVideoInputDevice::SetChannel(unsigned newChannel)
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
  pictureInfo.palette = colourFormatTab[newFormat][0];

  // set the information
  if (::ioctl(videoFd, VIDIOCSPICT, &pictureInfo) < 0)
    return FALSE;

  // set the new information
  pixelSize      = colourFormatTab[newFormat][1];
  videoFrameSize = frameWidth * frameHeight * pixelSize;

  return TRUE;
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
                                           unsigned & maxHeight) const
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

  videoFrameSize = frameWidth * frameHeight * pixelSize;

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
        frameBuffer[0].format = colourFormatTab[colourFormat][0];
        frameBuffer[0].width  = frameWidth;
        frameBuffer[0].height = frameHeight;
        ::ioctl(videoFd, VIDIOCMCAPTURE, &frameBuffer[0]);

        frameBuffer[1].frame  = 1;
        frameBuffer[1].format = colourFormatTab[colourFormat][0];
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
      ::munmap(videoBuffer, videoFrameSize*2);

    canMap = -1;
    videoBuffer = NULL;
  }
}


    
// End Of File ///////////////////////////////////////////////////////////////
