/*
 * video4bsd.cxx
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
 * $Log: video4bsd.cxx,v $
 * Revision 1.3  2001/01/05 14:52:36  rogerh
 * More work on the FreeBSD video capture code
 *
 * Revision 1.2  2001/01/04 18:02:16  rogerh
 * remove some old parts refering to linux
 *
 * Revision 1.1  2001/01/04 18:00:43  rogerh
 * Start to add support for video capture using on FreeBSD/NetBSD and OpenBSD
 * using the Meteor API (used by the Matrox Meteor and the bktr driver for
 * Bt848/Bt878 TV Tuner Cards). This is incomplete but it does compile.
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
}


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  if( channelNumber < 0 )
    return FALSE;

  Close();

  deviceName = devName;
  videoFd = ::open((const char *)devName, O_RDWR);
  if (videoFd < 0)
    return FALSE;
 
  // fill in a device capabilities structure
  videoCapability.minheight = 32;
  videoCapability.minwidth  = 32;
  videoCapability.maxheight = 768;
  videoCapability.maxwidth  = 576;
  videoCapability.channels  = 4;

  // set height and width
  frameHeight = videoCapability.maxheight;
  frameWidth  = videoCapability.maxwidth;
  
  // select the specified input
  if (!SetChannel(channelNumber)) {
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  } 
  
  // select the video format (eg PAL, NTSC)
  if (!SetVideoFormat(videoFormat)) {
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  }
 
  // select the colpur format (eg YUV420, or RGB)
  if (!SetColourFormat(colourFormat)) {
    ::close (videoFd);
    videoFd = -1;
    return FALSE;
  }

  // select the image size
  if (!SetFrameSize(frameWidth, frameHeight)) {
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

  // If the mmap buffer is not set, then set it
  if (canMap == -1) {
    mmap_size = videoFrameSize;
    videoBuffer = (BYTE *)::mmap(0, mmap_size, PROT_READ, MAP_SHARED, videoFd, 0);
    if (videoBuffer < 0)
      return FALSE;
    else
      canMap = 1;
  }


  // Start continuous capture mode
  int mode = METEOR_CAP_CONTINOUS;
  if (::ioctl(videoFd, METEORCAPTUR, &mode) < 0)
    return FALSE;

  return TRUE;
}


BOOL PVideoInputDevice::Stop()
{
  // Stop capturing

  int mode = METEOR_CAP_STOP_CONT;
  if (::ioctl(videoFd, METEORCAPTUR, &mode) < 0)
    return FALSE;

  return TRUE;
}


BOOL PVideoInputDevice::IsCapturing()
{
  return IsOpen();
}


PStringList PVideoInputDevice::GetDeviceNames() const
{
  PStringList list;

  list.AppendString("/dev/bktr0");
  list.AppendString("/dev/bktr1");
  list.AppendString("/dev/meteor0");
  list.AppendString("/dev/meteor1");

  return list;
}


BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  if (!PVideoDevice::SetVideoFormat(newFormat))
    return FALSE;

  // set channel information
  static int fmt[4] = { METEOR_FMT_PAL, METEOR_FMT_NTSC,
                        METEOR_FMT_SECAM, METEOR_FMT_AUTOMODE };
  int format = fmt[newFormat];

  // set the information
  if (::ioctl(videoFd, METEORSFMT, &format) < 0)
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

  // set channel information
  static int chnl[4] = { METEOR_INPUT_DEV0, METEOR_INPUT_DEV1,
                         METEOR_INPUT_DEV2, METEOR_INPUT_DEV3 };
  int channel = chnl[newChannel];

  // set the information
  if (::ioctl(videoFd, METEORSINPUT, &channel) < 0)
    return FALSE;

  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(ColourFormat newFormat)
{
  if (!PVideoDevice::SetColourFormat(newFormat))
    return FALSE;

  ClearMapping();

  // set the information
  struct meteor_geomet geo;

  // get the current geometry
  if (ioctl(videoFd, METEORGETGEO, &geo) < 0) {
    return FALSE;
  }

  // Set the colour format
  geo.oformat = METEOR_GEO_YUV_422 | METEOR_GEO_YUV_12;

  // set the new geometry
  if (ioctl(videoFd, METEORSETGEO, &geo) < 0) {
    return FALSE;
  }

  videoFrameSize = CalcFrameSize ( frameWidth, frameHeight, (int)colourFormat);

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

  // set the information
  struct meteor_geomet geo;

  // get the current geometry
  if (ioctl(videoFd, METEORGETGEO, &geo) < 0) {
    return FALSE;
  }

  geo.rows = frameHeight;
  geo.columns = frameWidth;
  geo.frames = 1;

  // set the new geometry
  if (ioctl(videoFd, METEORSETGEO, &geo) < 0) {
    return FALSE;
  }

  videoFrameSize = CalcFrameSize ( frameWidth, frameHeight, (int)colourFormat);
  
  return TRUE;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return videoFrameSize;
}



BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  // simply memcpy the frame to our storage
  memcpy(buffer, videoBuffer, videoFrameSize);

  return TRUE;
}


void PVideoInputDevice::ClearMapping()
{
  if (canMap == 1) {
    if (videoBuffer != NULL)
      ::munmap(videoBuffer, mmap_size);

    canMap = -1;
    videoBuffer = NULL;
  }
}


    
// End Of File ///////////////////////////////////////////////////////////////
