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
 * Revision 1.7  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
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
#include <ptlib/vconvert.h>



///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice()
  : colourFormat("RGB24")
{
  lastError = 0;

  videoFormat = Auto;
  channelNumber = 0;
  frameRate = 15;
  frameWidth = CIFWidth;
  frameHeight = CIFHeight;

  converter = NULL;
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


BOOL PVideoDevice::SetColourFormatConverter(const PString & colourFmt)
{
  if (SetColourFormat(colourFmt))
    return TRUE; // Can do it native

  /************************
    Eventually, need something more sophisticated than this, but for the
    moment pick the most common raw colour format that the device is very
    likely to support and then look for a conversion routine from that to
    the destaintion format.

    What we really want is some sort of better heuristic that looks at
    computational requirements of each converter and picks a pair of formats
    that the hardware supports and uses the least CPU.
  */

  if (!SetColourFormat("RGB24"))
    return FALSE;

  converter = PColourConverter::Create("RGB24", colourFmt, frameWidth, frameHeight);
  return converter != NULL;
}


BOOL PVideoDevice::SetColourFormat(const PString & colourFmt)
{
  colourFormat = colourFmt;
  return IsOpen();
}


const PString & PVideoDevice::GetColourFormat() const
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
  const char * colourFormat;
  unsigned     bitsPerPixel;
} colourFormatBPPTab[] = {
  { "Grey",     8 },  
  { "RGB32",   32 }, 
  { "RGB24",   24 }, 
  { "RGB565",  16 },
  { "RGB555",  16 },
  { "YUV422",  16 },
  { "YUV422P", 16 },
  { "YUV411",  12 },
  { "YUV411P", 12 },
  { "YUV420",  12 },
  { "YUV420P", 12 },
  { "YUV410",  0  },
  { "YUV410P", 10 }
};


unsigned PVideoDevice::CalculateFrameBytes(unsigned width, unsigned height,
                                           const PString & colourFormat)
{
  for (PINDEX i = 0; i < PARRAYSIZE(colourFormatBPPTab); i++) {
    if (colourFormat == colourFormatBPPTab[i].colourFormat)
      return width * height * colourFormatBPPTab[i].bitsPerPixel/8;
  }
  return 0;
}
 

BOOL PVideoDevice::Close()
{
  return TRUE;  
}


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice

PVideoOutputDevice::PVideoOutputDevice()
{
  now = 0;
  suppress = FALSE;
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
