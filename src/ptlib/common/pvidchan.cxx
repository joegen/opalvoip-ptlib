/*
 * pvidchan.cxx
 *
 * Video Channel implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: pvidchan.cxx,v $
 * Revision 1.4  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.3  2001/06/19 00:51:57  dereks
 * The ::Write method now returns the result of mpOutput->Redraw(), rather than
 *   always true.
 *
 * Revision 1.2  2001/03/23 20:24:23  yurik
 * Got rid of "unknown pragma" for WinCE port
 *
 * Revision 1.1  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 *
 *
 */

#ifndef _WIN32_WCE
#pragma implementation "video.h"
#endif

#include <ptlib.h>


PVideoChannel::PVideoChannel() 
{
  mpInput = NULL;
  mpOutput = NULL;
}


PVideoChannel::PVideoChannel(const PString & device,
                             Directions dir)
{
  mpInput = NULL;
  mpOutput = NULL;
  Open(device, dir);
}

PVideoChannel::~PVideoChannel()
{
	Close();
}


PStringList PVideoChannel::GetDeviceNames(Directions /*dir*/)
{
  PStringList list;

  list.AppendString("Video Channel Base");

  return list;
}


PString PVideoChannel::GetDefaultDevice(Directions /*dir*/)
{
#if defined(P_FREEBSD) || defined(P_OPENBSD)
  return "/dev/bktr0";
#endif

#ifndef DEFAULT_VIDEO
     return "/dev/video0";
#else
  return DEFAULT_VIDEO;
#endif
}


BOOL PVideoChannel::Open(const PString & dev,
                         Directions dir   )
{
    Close();
	
	deviceName = dev;
	direction = dir;
	
   	return TRUE;
}



BOOL PVideoChannel::Read( void * buf, PINDEX  len)
{
  if( mpInput == NULL)  
    return FALSE;

  BYTE * dataBuf;
  PINDEX dataLen;
  dataBuf = (BYTE *)buf;
  dataLen = len;
  mpInput->GetFrameData(dataBuf, &dataLen );

  return TRUE;
}

BOOL PVideoChannel::Write(const void * buf,  //image data to be rendered
                          PINDEX      /* len */)
{
   if( mpOutput == NULL)
      return FALSE;
   
   return mpOutput->Redraw (buf);
}

BOOL PVideoChannel::Close()
{
  if (mpInput) 
    delete mpInput;
  
  if (mpOutput)
    delete mpOutput;
  
  mpInput = NULL;
  mpOutput = NULL;

  return TRUE;
}

/*returns true if either input or output is open */
BOOL PVideoChannel::IsOpen() const
{
  return ((mpInput != NULL) || (mpOutput != NULL) );
}


PString PVideoChannel::GetName() const
{
  return deviceName;
}

void PVideoChannel::AttachVideoPlayer(PVideoOutputDevice * device)
{
  if (mpOutput)
    PAssertAlways("Error: Attempt to add video player while one is already defined");
  else 
    mpOutput = device;
}

void PVideoChannel::AttachVideoReader(PVideoInputDevice * device)
{
  if (mpInput)
    PAssertAlways("Error: Attempt to add video reader while one is already defined");
  else 
    mpInput = device;
}

PINDEX  PVideoChannel::GetGrabHeight() 
{
  if (mpInput)
    return mpInput->GetFrameHeight() ;
  else
    return 0;
}


PINDEX  PVideoChannel::GetGrabWidth()
{
  if (mpInput)
    return  mpInput->GetFrameWidth() ;
  else
    return 0;
}

BOOL PVideoChannel::IsGrabberOpen()
{
  if (mpInput)
    return mpInput->IsOpen();
  else
    return FALSE; 
}

BOOL PVideoChannel::IsRenderOpen()      
{
  if (mpOutput)
    return mpOutput->IsOpen();
  else
    return FALSE; 
}

///////////////////////////////////////////////////////////////////////////
// End of file

