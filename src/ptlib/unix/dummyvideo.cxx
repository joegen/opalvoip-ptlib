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
 * $Id$
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


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  return FALSE;    
}


BOOL PVideoInputDevice::IsOpen() 
{
  return FALSE;    
}


BOOL PVideoInputDevice::Close()
{
  return FALSE;    
}


BOOL PVideoInputDevice::Start()
{
  return FALSE;
}


BOOL PVideoInputDevice::Stop()
{
  return FALSE;
}


BOOL PVideoInputDevice::IsCapturing()
{
  return FALSE;
}


BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
{
  return FALSE;
}


int PVideoInputDevice::GetBrightness()
{
  return -1;
}


BOOL PVideoInputDevice::SetBrightness(unsigned newBrightness)
{
  return FALSE;
}


int PVideoInputDevice::GetHue()
{
  return -1;
}


BOOL PVideoInputDevice::SetHue(unsigned newHue)
{
  return FALSE;
}


int PVideoInputDevice::GetContrast()
{
  return -1;
}


BOOL PVideoInputDevice::SetContrast(unsigned newContrast)
{
  return FALSE;
}


BOOL PVideoInputDevice::GetParameters (int *whiteness, int *brightness,
                                       int *colour, int *contrast, int *hue)
{
  return FALSE;
}


int PVideoInputDevice::GetNumChannels() 
{
  return 0;
}


BOOL PVideoInputDevice::SetChannel(int newChannel)
{
  return FALSE;
}


BOOL PVideoInputDevice::SetColourFormat(const PString & newFormat)
{
  return FALSE;
}


BOOL PVideoInputDevice::SetFrameRate(unsigned rate)
{
  return FALSE;
}


BOOL PVideoInputDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  return FALSE;
}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  return FALSE;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return 0;
}



BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  return FALSE;
}


BOOL PVideoInputDevice::GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned)
{
  return FALSE;
}


void PVideoInputDevice::ClearMapping()
{
}

BOOL PVideoInputDevice::VerifyHardwareFrameSize(unsigned width,
                                                unsigned height)
{
	// Assume the size is valid
	return TRUE;
}

BOOL PVideoInputDevice::TestAllFormats()
{
  return TRUE;
}
    
// End Of File ///////////////////////////////////////////////////////////////
