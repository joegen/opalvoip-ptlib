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
 * Revision 1.2  2000/07/25 13:38:26  robertj
 * Added frame rate parameter to video frame grabber.
 *
 * Revision 1.1  2000/07/15 09:47:35  robertj
 * Added video I/O device classes.
 *
 */

#include <ptlib.h>
#include <ptlib/videoio.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoDevice

PVideoDevice::PVideoDevice(VideoFormat videofmt,
                           unsigned channel,
                           ColourFormat colourFmt)
{
  videoFormat = videofmt;
  channelNumber = channel;
  colourFormat = colourFmt;
  frameRate = 15;
  frameWidth = CIF_WIDTH;
  frameHeight = CIF_HEIGHT;
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


unsigned PVideoDevice::GetNumChannels() const
{
  return 1;
}


BOOL PVideoDevice::SetChannel(unsigned channelNum)
{
  channelNumber = channelNum;
  return IsOpen();
}


unsigned PVideoDevice::GetChannel() const
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


BOOL PVideoDevice::GetMaxFrameSize(unsigned & width, unsigned & height) const
{
  width = height = UINT_MAX;
  return FALSE;
}


BOOL PVideoDevice::GetMinFrameSize(unsigned & width, unsigned & height) const
{
  width = height = 1;
  return FALSE;
}


BOOL PVideoDevice::SetFrameSize(unsigned width, unsigned height)
{
  frameWidth = width;
  frameHeight = height;
  return IsOpen();
}


BOOL PVideoDevice::GetFrameSize(unsigned & width, unsigned & height) const
{
  width = frameWidth;
  height = frameHeight;
  return IsOpen();
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
