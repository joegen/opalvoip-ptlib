/*
 * pvidfile.cxx
 *
 * Video file implementation
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
 * $Log: pvidfile.cxx,v $
 * Revision 1.2  2006/02/20 06:49:45  csoutheren
 * Added video file and video file input device code
 *
 * Revision 1.1  2006/02/20 06:17:28  csoutheren
 * Added ability to read video from a file
 *
 */

#ifdef __GNUC__
#pragma implementation "pvidfile.h"
#endif

#include <ptlib.h>

#if P_VIDFILE

#include <ptclib/pvidfile.h>

///////////////////////////////////////////////////////////////////////////////

PVideoFile::PVideoFile()
{
  yuvSize = yuvWidth = yuvHeight = 0;
}

PVideoFile::PVideoFile(PINDEX width,
                       PINDEX height,
                     OpenMode mode,
                          int opts)
  : PFile(mode, opts), yuvWidth(width), yuvHeight(height)
{
  yuvSize = yuvWidth * yuvHeight * 3 / 2;
}

PVideoFile::PVideoFile(PINDEX width, 
                       PINDEX height, 
            const PFilePath & name,
                     OpenMode mode,
                          int opts)
  : PFile(name, mode, opts), yuvWidth(width), yuvHeight(height)
{
  yuvSize = yuvWidth * yuvHeight * 3 / 2;
}

void PVideoFile::SetWidth(PINDEX v)    
{ 
  yuvWidth = v; 
  yuvSize = yuvWidth * yuvHeight * 3 / 2;
}

void PVideoFile::SetHeight(PINDEX v)   
{ 
  yuvHeight = v; 
  yuvSize = yuvWidth * yuvHeight * 3 / 2;
}

///////////////////////////////////////////////////////////////////////////////

PYUVFile::PYUVFile()
  : PVideoFile()
{
}

PYUVFile::PYUVFile(PINDEX width,
                   PINDEX height,
                 OpenMode mode,
                      int opts)
  : PVideoFile(width, height, mode, opts)
{
}

PYUVFile::PYUVFile(PINDEX width, 
                   PINDEX height, 
        const PFilePath & name,
                 OpenMode mode,
                      int opts)
  : PVideoFile(width, height, name, mode, opts)
{
}

BOOL PYUVFile::WriteFrame(const void * frame)
{
  return PFile::Write(frame, yuvSize);
}

BOOL PYUVFile::ReadFrame(void * frame)
{
  if (!PFile::Read(frame, yuvSize)) {
    PTRACE(4, "YUVFILE\tError reading file " << GetErrorText(GetErrorCode(LastReadError)));
    return FALSE;
  }

  if (GetLastReadCount() != yuvSize)
    return FALSE;

  return TRUE;
}

#endif  // P_VIDFILE

