/*
 * video4beos.cxx
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
 * Contributor(s): Jac Goudsmit <jac@be.com>.
 *
 * $Log: video4beos.cxx,v $
 * Revision 1.4  2004/01/02 23:30:18  rjongbloed
 * Removed extraneous static function for getting input device names that has been deprecated during the plug ins addition.
 *
 * Revision 1.3  2002/04/10 08:40:36  rogerh
 * Simplify the SetVideoChannelFormat() code. Use the implementation in the
 * ancestor class.
 *
 * Revision 1.2  2002/04/05 21:54:58  rogerh
 * Add SsetVideoChannelFormat - Reminded by Yuri
 *
 * Revision 1.1  2001/07/09 06:16:15  yurik
 * Jac Goudsmit's BeOS changes of July,6th. Cleaning up media subsystem etc.
 *
 *
 */

//#pragma implementation "videoio.h"

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
  return TRUE;
}


BOOL PVideoInputDevice::Stop()
{
  return TRUE;
}


BOOL PVideoInputDevice::IsCapturing()
{
  return FALSE;
}


BOOL PVideoInputDevice::SetVideoFormat(VideoFormat newFormat)
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


void PVideoInputDevice::ClearMapping()
{
}


BOOL PVideoInputDevice::VerifyHardwareFrameSize(unsigned width,
						unsigned height)
{
	return FALSE;
}
 

// End Of File ///////////////////////////////////////////////////////////////
