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

#include <ptlib/vconvert.h>
#include <ptclib/pvfiledev.h>

PINSTANTIATE_FACTORY(PVideoInputDevice, YUVFile)

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_YUVFile


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

  file.SetWidth(frameWidth);
  file.SetHeight(frameHeight);

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

  if (!PVideoDevice::SetColourFormat(newFormat))
    return FALSE;

  return SetFrameSize(frameWidth, frameHeight);
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
  minWidth  = 16;
  minHeight = 12;
  maxWidth  = 1024;
  maxHeight =  768;

  return TRUE;
}


BOOL PVideoInputDevice_YUVFile::SetFrameSize(unsigned width, unsigned height)
{
  if (!PVideoDevice::SetFrameSize(width, height))
    return FALSE;

  //file.SetWidth(width);
  //file.SetHeight(height);

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

  if (file.IsOpen()) {
    if (!file.ReadFrame(destFrame))
      file.Close();
  }

  if (!file.IsOpen()) {
    switch (channelNumber) {
      case Channel_PlayAndClose:
      default:
        return FALSE;

      case Channel_PlayAndRepeat:
        if (!file.Open() || !file.ReadFrame(destFrame))
          return FALSE;
        break;

      case Channel_PlayAndKeepLast:
        break;

      case Channel_PlayAndShowBlack:
        FillRect(destFrame, 0, 0, frameWidth, frameHeight, 0, 0, 0);
        break;
    }
  }

  if (converter != NULL) {
    if (!converter->Convert(destFrame, destFrame, bytesReturned))
      return FALSE;
  }

  if (bytesReturned != NULL)
    *bytesReturned = videoFrameSize;

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

#endif // P_VIDFILE
