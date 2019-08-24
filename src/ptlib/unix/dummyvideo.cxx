/*
 * dummyvideo.cxx
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2001 Equivalence Pty. Ltd.
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
 * Contributor(s): Roger Hardiman <roger@freebsd.org>
 *
 */

#pragma implementation "videoio.h"

#include <ptlib.h>
#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>
#include <ptlib/vconvert.h>

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice

PVideoInputDevice::PVideoInputDevice()
{
}


PBoolean PVideoInputDevice::Open(const PString & devName, PBoolean startImmediate)
{
  return false;    
}


PBoolean PVideoInputDevice::IsOpen() 
{
  return false;    
}


PBoolean PVideoInputDevice::Close()
{
  return false;    
}


PBoolean PVideoInputDevice::Start()
{
  return false;
}


PBoolean PVideoInputDevice::Stop()
{
  return false;
}


PBoolean PVideoInputDevice::IsCapturing()
{
  return false;
}


PBoolean PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  return false;
}


int PVideoInputDevice::GetBrightness()
{
  return -1;
}


PBoolean PVideoInputDevice::SetBrightness(unsigned newBrightness)
{
  return false;
}


int PVideoInputDevice::GetHue()
{
  return -1;
}


PBoolean PVideoInputDevice::SetHue(unsigned newHue)
{
  return false;
}


int PVideoInputDevice::GetContrast()
{
  return -1;
}


PBoolean PVideoInputDevice::SetContrast(unsigned newContrast)
{
  return false;
}


PBoolean PVideoInputDevice::GetParameters (int *whiteness, int *brightness,
                                       int *colour, int *contrast, int *hue)
{
  return false;
}


int PVideoInputDevice::GetNumChannels() 
{
  return 0;
}


PBoolean PVideoInputDevice::SetChannel(int newChannel)
{
  return false;
}


PBoolean PVideoInputDevice::SetColourFormat(const PString & newFormat)
{
  return false;
}


PBoolean PVideoInputDevice::SetFrameRate(unsigned rate)
{
  return false;
}


PBoolean PVideoInputDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  return false;
}


PBoolean PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  return false;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return 0;
}



bool PVideoInputDevice::InternalGetFrameData(BYTE * buffer, PINDEX & bytesReturned, bool & keyFrame, bool wait)
{
  return false;
}


void PVideoInputDevice::ClearMapping()
{
}

PBoolean PVideoInputDevice::VerifyHardwareFrameSize(unsigned width,
                                                unsigned height)
{
	// Assume the size is valid
	return true;
}

PBoolean PVideoInputDevice::TestAllFormats()
{
  return true;
}
    
// End Of File ///////////////////////////////////////////////////////////////
