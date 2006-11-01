/*
 * pvfiledev.cxx
 *
 * Video file declaration
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is
 * Craig Southeren <craigs@postincrement.com>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pvfiledev.cxx,v $
 * Revision 1.10  2006/11/01 00:46:01  csoutheren
 * Fix problem in video output file device
 *
 * Revision 1.9  2006/10/31 04:10:40  csoutheren
 * Make sure PVidFileDev class is loaded, and make it work with OPAL
 *
 * Revision 1.8  2006/06/21 03:28:44  csoutheren
 * Various cleanups thanks for Frederic Heem
 *
 * Revision 1.7  2006/04/19 04:09:04  csoutheren
 * Allow frame size conversions
 *
 * Revision 1.6  2006/03/17 06:55:33  csoutheren
 * Removed unused member variable
 *
 * Revision 1.5  2006/03/06 06:04:13  csoutheren
 * Added YUVFile video output device
 *
 * Revision 1.4  2006/02/24 04:51:26  csoutheren
 * Fixed problem with using CIF from video files
 * Added support for video files in y4m format
 *
 * Revision 1.3  2006/02/20 09:31:58  csoutheren
 * Fixed link problem on Linux
 *
 * Revision 1.2  2006/02/20 06:49:45  csoutheren
 * Added video file and video file input device code
 *
 * Revision 1.1  2006/02/20 06:17:28  csoutheren
 * Added ability to read video from a file
 *
 */

#ifdef __GNUC__
#pragma implementation "pvfiledev.h"
#endif

#define P_FORCE_STATIC_PLUGIN

#include <ptlib.h>

#if P_VIDFILE

namespace PWLibStupidHacks {
  int loadVideoFileStuff;
};

#include <ptlib/vconvert.h>
#include <ptclib/pvfiledev.h>
#include <ptlib/pfactory.h>
#include <ptlib/pluginmgr.h>
#include <ptlib/videoio.h>

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_YUVFile

PINSTANTIATE_FACTORY(PVideoInputDevice, YUVFile)
PCREATE_VIDINPUT_PLUGIN(YUVFile);

PVideoInputDevice_YUVFile::PVideoInputDevice_YUVFile()
{
  SetColourFormat("YUV420P");
  channelNumber = 0; 
  grabCount = 0;
  SetFrameRate(10);
}


BOOL PVideoInputDevice_YUVFile::Open(const PString & devName, BOOL /*startImmediate*/)
{
  PFilePath fn(devName);

  if (!file.Open(fn, PFile::ReadOnly, PFile::MustExist))
    return FALSE;

  deviceName = fn.GetTitle();

  return TRUE;    
}


BOOL PVideoInputDevice_YUVFile::IsOpen() 
{
  return file.IsOpen();
}


BOOL PVideoInputDevice_YUVFile::Close()
{
  return file.Close();
}


BOOL PVideoInputDevice_YUVFile::Start()
{
  return TRUE;
}


BOOL PVideoInputDevice_YUVFile::Stop()
{
  return TRUE;
}


BOOL PVideoInputDevice_YUVFile::IsCapturing()
{
  return IsOpen();
}


PStringList PVideoInputDevice_YUVFile::GetInputDeviceNames()
{
  PStringList list;
  list.AppendString("yuvfile");
  return list;
}


BOOL PVideoInputDevice_YUVFile::SetVideoFormat(VideoFormat newFormat)
{
  return PVideoDevice::SetVideoFormat(newFormat);
}


int PVideoInputDevice_YUVFile::GetNumChannels() 
{
  return ChannelCount;  
}


BOOL PVideoInputDevice_YUVFile::SetChannel(int newChannel)
{
  return PVideoDevice::SetChannel(newChannel);
}

BOOL PVideoInputDevice_YUVFile::SetColourFormat(const PString & newFormat)
{
  if (!(newFormat *= "YUV420P"))
    return FALSE;

  return PVideoDevice::SetColourFormat(newFormat);
}


BOOL PVideoInputDevice_YUVFile::SetFrameRate(unsigned rate)
{
  if (rate < 1)
    rate = 1;
  else if (rate > 50)
    rate = 50;

  return PVideoDevice::SetFrameRate(rate);
}


BOOL PVideoInputDevice_YUVFile::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  if (file.GetWidth() != 0 && file.GetHeight() != 0) {
    minWidth  = maxWidth  = file.GetWidth();
    minHeight = maxHeight = file.GetHeight();
  }
  else
  {
    minWidth  = 16;
    minHeight = 12;
    maxWidth  = 1024;
    maxHeight =  768;
  }

  return TRUE;
}

BOOL PVideoInputDevice_YUVFile::SetFrameSizeConverter(
      unsigned width,        ///< New width of frame
      unsigned height,       ///< New height of frame
      BOOL     bScaleNotCrop ///< Scale or crop/pad preference
)
{
  // if the file does not know what size it is, then set it
  if (file.GetWidth() == 0 && file.GetHeight() == 0) {
    file.SetWidth(width);
    file.SetHeight(height);
  }

  return PVideoInputDevice::SetFrameSizeConverter(width, height, bScaleNotCrop);
}

BOOL PVideoInputDevice_YUVFile::SetFrameSize(unsigned width, unsigned height)
{
  // if the file does not know what size it is, then set it
  if (file.GetWidth() == 0 && file.GetHeight() == 0) {
    file.SetWidth(width);
    file.SetHeight(height);
  }

  if (width != (unsigned)file.GetWidth() || height != (unsigned)file.GetHeight())
    return FALSE;

  frameWidth = width;
  frameHeight = height;

  videoFrameSize = CalculateFrameBytes(frameWidth, frameHeight, colourFormat);
  scanLineWidth = videoFrameSize/frameHeight;
  return videoFrameSize > 0;
}


PINDEX PVideoInputDevice_YUVFile::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(videoFrameSize);
}


BOOL PVideoInputDevice_YUVFile::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{    
  frameTimeError += msBetweenFrames;

  PTime now;
  PTimeInterval delay = now - previousFrameTime;
  frameTimeError -= (int)delay.GetMilliSeconds();
  previousFrameTime = now;

  if (frameTimeError > 0) {
    PTRACE(6, "YUVFile\t Sleep for " << frameTimeError << " milli seconds");
    PThread::Sleep(frameTimeError);
  }

  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
BOOL PVideoInputDevice_YUVFile::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  grabCount++;

  BYTE * readBuffer = destFrame;

  if (converter != NULL)
    readBuffer = frameStore.GetPointer(videoFrameSize);

  if (file.IsOpen()) {
    if (!file.ReadFrame(readBuffer))
      file.Close();
  }

  if (!file.IsOpen()) {
    switch (channelNumber) {
      case Channel_PlayAndClose:
      default:
        return FALSE;

      case Channel_PlayAndRepeat:
        if (!file.Open() || !file.ReadFrame(readBuffer))
          return FALSE;
        break;

      case Channel_PlayAndKeepLast:
        break;

      case Channel_PlayAndShowBlack:
        FillRect(readBuffer, 0, 0, frameWidth, frameHeight, 0, 0, 0);
        break;
    }
  }

  if (converter == NULL) {
    if (bytesReturned != NULL)
      *bytesReturned = videoFrameSize;
  } else {
    converter->SetSrcFrameSize(frameWidth, frameHeight);
    if (!converter->Convert(readBuffer, destFrame, bytesReturned))
      return FALSE;
    if (bytesReturned != NULL)
      *bytesReturned = converter->GetMaxDstFrameBytes();
  }

  return TRUE;
}


void PVideoInputDevice_YUVFile::GrabBlankImage(BYTE *resFrame)
{
  // Change colour every second, cycle is:
  // black, red, green, yellow, blue, magenta, cyan, white
  int mask = grabCount/frameRate;
  FillRect(resFrame,
           0, 0, frameWidth, frameHeight, //Fill the whole frame with the colour.
           (mask&1) ? 255 : 0, // red
           (mask&2) ? 255 : 0, // green
           (mask&4) ? 255 : 0);//blue
}

void PVideoInputDevice_YUVFile::FillRect(BYTE * frame,
                   int xPos, int initialYPos,
                   int rectWidth, int rectHeight,
                   int r, int g,  int b)
{
// PTRACE(0,"x,y is"<<xPos<<" "<<yPos<<" and size is "<<rectWidth<<" "<<rectHeight);

  //This routine fills a region of the video image with data. It is used as the central
  //point because one only has to add other image formats here.

  int yPos = initialYPos;

  int offset       = ( yPos * frameWidth ) + xPos;
  int colourOffset = ( (yPos * frameWidth) >> 2) + (xPos >> 1);

  int Y  =  ( 257 * r + 504 * g +  98 * b)/1000 + 16;
  int Cb =  (-148 * r - 291 * g + 439 * b)/1000 + 128;
  int Cr =  ( 439 * r - 368 * g -  71 * b)/1000 + 128;

  unsigned char * Yptr  = frame + offset;
  unsigned char * CbPtr = frame + (frameWidth * frameHeight) + colourOffset;
  unsigned char * CrPtr = frame + (frameWidth * frameHeight) + (frameWidth * frameHeight/4)  + colourOffset;

  int rr ;
  int halfRectWidth = rectWidth >> 1;
  int halfWidth     = frameWidth >> 1;
  
  for (rr = 0; rr < rectHeight;rr+=2) {
    memset(Yptr, Y, rectWidth);
    Yptr += frameWidth;
    memset(Yptr, Y, rectWidth);
    Yptr += frameWidth;

    memset(CbPtr, Cb, halfRectWidth);
    memset(CrPtr, Cr, halfRectWidth);

    CbPtr += halfWidth;
    CrPtr += halfWidth;
  }
}

///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice_YUVFile

PINSTANTIATE_FACTORY(PVideoOutputDevice, YUVFile)
PCREATE_VIDOUTPUT_PLUGIN(YUVFile);

PVideoOutputDevice_YUVFile::PVideoOutputDevice_YUVFile()
{
}


BOOL PVideoOutputDevice_YUVFile::Open(const PString & _deviceName, BOOL /*startImmediate*/)
{
  deviceName = _deviceName;
  if (!file.Open(deviceName, PFile::WriteOnly, PFile::Create)) {
    PTRACE(1, "Cannot create file " << deviceName << " as video output device");
    return FALSE;
  }

  return TRUE;
}

BOOL PVideoOutputDevice_YUVFile::Close()
{
  return file.Close();
}

BOOL PVideoOutputDevice_YUVFile::Start()
{
  file.SetHeight(frameHeight);
  file.SetWidth(frameWidth);
  return TRUE;
}

BOOL PVideoOutputDevice_YUVFile::Stop()
{
  return TRUE;
}

BOOL PVideoOutputDevice_YUVFile::IsOpen()
{
  return file.IsOpen();
}


PStringList PVideoOutputDevice_YUVFile::GetOutputDeviceNames()
{
  PStringList list;
  list.AppendString("yuvfile");
  return list;
}


PINDEX PVideoOutputDevice_YUVFile::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(CalculateFrameBytes(frameWidth, frameHeight, colourFormat));
}


BOOL PVideoOutputDevice_YUVFile::SetFrameData(unsigned x, unsigned y,
                                              unsigned width, unsigned height,
                                              const BYTE * data,
                                              BOOL /*endFrame*/)
{
  if (x != 0 || y != 0 || width != frameWidth || height != frameHeight) {
    PTRACE(1, "YUVFile output device only supports full frame writes");
    return FALSE;
  }

  if ((file.GetWidth() == 0) && (file.GetHeight() == 0)) {
    file.SetWidth(width);
    file.SetHeight(height);
  }
  else if (((unsigned)file.GetWidth() != width) || ((unsigned)file.GetHeight() != height))
    return FALSE;

  return file.WriteFrame(data);
}


BOOL PVideoOutputDevice_YUVFile::EndFrame()
{
  return TRUE;
}



#endif // P_VIDFILE
