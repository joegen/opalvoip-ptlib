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
 * $Log: dummyvideo.cxx,v $
 * Revision 1.1  2001/06/27 17:27:23  rogerh
 * Dummy video allows code dependent on PVideoInputDevice to compile and link.
 * It has no functionality.
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


BOOL PVideoInputDevice::Open(const PString & devName, BOOL startImmediate)
{
  return TRUE;    
}


BOOL PVideoInputDevice::IsOpen() 
{
  return TRUE;    
}


BOOL PVideoInputDevice::Close()
{
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
  return TRUE;
}


PStringList PVideoInputDevice::GetInputDeviceNames()
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
  return TRUE;
}


int PVideoInputDevice::GetNumChannels() 
{
  return TRUE;
}


BOOL PVideoInputDevice::SetChannel(int newChannel)
{
  return TRUE;
}


BOOL PVideoInputDevice::SetColourFormat(const PString & newFormat)
{
  return TRUE;
}


BOOL PVideoInputDevice::SetFrameRate(unsigned rate)
{
  return TRUE;
}


BOOL PVideoInputDevice::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  return TRUE;
}


BOOL PVideoInputDevice::SetFrameSize(unsigned width, unsigned height)
{
  return TRUE;
}


PINDEX PVideoInputDevice::GetMaxFrameBytes()
{
  return 0;
}



BOOL PVideoInputDevice::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{
  return TRUE;
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
    
// End Of File ///////////////////////////////////////////////////////////////
