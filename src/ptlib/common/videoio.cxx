/*
 * videoio.cxx
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
 * $Log: videoio.cxx,v $
 * Revision 1.6  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.5  2000/11/09 00:20:58  robertj
 * Added qcif size constants
 *
 * Revision 1.4  2000/07/26 03:50:50  robertj
 * Added last error variable to video device.
 *
 * Revision 1.3  2000/07/26 02:13:48  robertj
 * Added some more "common" bounds checking to video device.
 *
 * Revision 1.2  2000/07/25 13:38:26  robertj
 * Added frame rate parameter to video frame grabber.
 *
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>
#include <ptlib/vconvert.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice(VideoFormat videofmt,
                           int channel,
                           ColourFormat colourFmt)
{
  lastError = 0;
  videoFormat = videofmt;
  channelNumber = channel;
  colourFormat = colourFmt;
  frameRate = 15;
  frameWidth = CIFWidth;
  frameHeight = CIFHeight;
}


BOOL PVideoDevice::SetVideoFormat(VideoFormat videoFmt)
{
  videoFormat = videoFmt;
  return IsOpen();
}


PVideoDevice::VideoFormat PVideoDevice::GetVideoFormat() const
{
  return videoFormat;
}

PStringList PVideoDevice::GetDeviceNames() const
{
  PStringList list;

  list.AppendString("Video Device raw.");

  return list;  
}


int PVideoDevice::GetNumChannels()
{
  return 1;
}


BOOL PVideoDevice::SetChannel(int channelNum)
{
  if (channelNum >= (int)GetNumChannels())
    return FALSE;

  channelNumber = channelNum;
  return IsOpen();
}


int PVideoDevice::GetChannel() const
{
  return channelNumber;
}


BOOL PVideoDevice::SetColourFormat(ColourFormat colourFmt)
{
  colourFormat = colourFmt;
  return IsOpen();
}


PVideoDevice::ColourFormat PVideoDevice::GetColourFormat() const
{
  return colourFormat;
}


BOOL PVideoDevice::SetFrameRate(unsigned rate)
{
  frameRate = rate;
  return IsOpen();
}


unsigned PVideoDevice::GetFrameRate() const
{
  return frameRate;
}


BOOL PVideoDevice::GetFrameSizeLimits(unsigned & minWidth,
                                      unsigned & minHeight,
                                      unsigned & maxWidth,
                                      unsigned & maxHeight) 
{
  minWidth = minHeight = 1;
  maxWidth = maxHeight = UINT_MAX;
  return FALSE;
}


BOOL PVideoDevice::SetFrameSize(unsigned width, unsigned height)
{
  unsigned minWidth, minHeight, maxWidth, maxHeight;
  GetFrameSizeLimits(minWidth, minHeight, maxWidth, maxHeight);

  if (width < minWidth)
    frameWidth = minWidth;
  else if (width > maxWidth)
    frameWidth = maxWidth;
  else
    frameWidth = width;

  if (height < minHeight)
    frameHeight = minHeight;
  else if (height > maxHeight)
    frameHeight = maxHeight;
  else
    frameHeight = height;

  return IsOpen();
}


BOOL PVideoDevice::GetFrameSize(unsigned & width, unsigned & height) 
{
  width = frameWidth;
  height = frameHeight;
  return IsOpen();
}

  ///Colour format bit per pixel table.
static struct {
    int intColourFormat;
    int bitsPerPixel;
   } colourFormatBPPTab[PVideoDevice::NumColourFormats] = {
       {  /*Grey   */  0,   8 },  
       {  /*RGB32  */  1,  32 }, 
       {  /*RGB24  */  2,  24 }, 
       {  /*RGB565 */  3,  16 },
       {  /*RGB555 */  4,  16 },
       {  /*YUV422 */  5,  16 },
       {  /*YUV422P*/  6,  16 },
       {  /*YUV411 */  7,  12 },
       {  /*YUV411P*/  8,  12 },
       {  /*YUV420 */  9,  12 },
       {  /*YUV420P*/ 10,  12 },
       {  /*YUV410 */ 11,   0 },
       {  /*YUV410P*/ 12,  10 }
    };


unsigned PVideoDevice::CalcFrameSize(unsigned width, unsigned height, int colourFormat)
{
   return width * height * colourFormatBPPTab[colourFormat].bitsPerPixel/8;
}
 

BOOL PVideoDevice::Close()
{
  return TRUE;  
}

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

BOOL PVideoInputDevice::GetFrame(PBYTEArray & frame)
{
  PINDEX returned;
  if (!GetFrameData(frame.GetPointer(GetMaxFrameBytes()), &returned))
    return FALSE;

  frame.SetSize(returned);
  return TRUE;
}

// End Of File ///////////////////////////////////////////////////////////////
