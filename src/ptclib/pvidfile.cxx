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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "pvidfile.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#include <ptclib/pvidfile.h>
#include <ptlib/videoio.h>


///////////////////////////////////////////////////////////////////////////////

PVideoFile::PVideoFile()
  : unknownFrameSize(TRUE)
  , frameBytes(CalculateFrameBytes())
  , headerOffset(0)
{
}


BOOL PVideoFile::SetFrameSize(unsigned width, unsigned height)   
{ 
  if (!PVideoFrameInfo::SetFrameSize(width, height))
    return FALSE;

  unknownFrameSize = FALSE;
  frameBytes = CalculateFrameBytes();
  return frameBytes > 0;
}


BOOL PVideoFile::Open(const PFilePath & name, PFile::OpenMode mode, int opts)
{
  if (unknownFrameSize)
    ExtractHints(name, *this);
  return file.Open(name, mode, opts);
}


BOOL PVideoFile::WriteFrame(const void * frame)
{
  return file.Write(frame, frameBytes);
}


BOOL PVideoFile::ReadFrame(void * frame)
{
  if (file.Read(frame, frameBytes) && file.GetLastReadCount() == frameBytes)
    return TRUE;

  PTRACE(4, "YUVFILE\tError reading file " << file.GetErrorText(file.GetErrorCode(PFile::LastReadError)));
  return FALSE;
}


off_t PVideoFile::GetLength() const
{
  off_t len = file.GetLength();
  return len < headerOffset ? 0 : ((len - headerOffset)/frameBytes);
}


BOOL PVideoFile::SetLength(off_t len)
{
  return file.SetLength(len*frameBytes + headerOffset);
}


off_t PVideoFile::GetPosition() const
{
  off_t pos = file.GetPosition();
  return pos < headerOffset ? 0 : ((pos - headerOffset)/frameBytes);
}


BOOL PVideoFile::SetPosition(off_t pos, PFile::FilePositionOrigin origin)
{
  pos *= frameBytes;
  if (origin == PFile::Start)
    pos += headerOffset;

  return file.SetPosition(pos, origin);
}


BOOL PVideoFile::ExtractHints(const PFilePath & fn, PVideoFrameInfo & info)
{
  static PRegularExpression  qcif  ("{qcif}|_qcif[^a-z0-9]",                PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression   cif  ("{cif}|_cif[^a-z0-9]",                  PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression sqcif  ("{sqcif}|_sqcif[^a-z0-9]",              PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression   cif4 ("{cif4}|_cif4[^a-z0-9]",                PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression   cif16("{cif16}|_cif16[^a-z0-9]",              PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression   XbyY ("{[0-9]+x[0-9]+}|_[0-9]+x[0-9]+[^a-z]", PRegularExpression::Extended|PRegularExpression::IgnoreCase);
  static PRegularExpression fps    ("_[0-9]+fps[^a-z]",                     PRegularExpression::Extended|PRegularExpression::IgnoreCase);

  PCaselessString str = fn;
  BOOL foundHint = FALSE;
  PINDEX pos;

  if (str.FindRegEx(qcif) != P_MAX_INDEX)
    foundHint = info.SetFrameSize(QCIFWidth, QCIFHeight);
  else if (str.FindRegEx(cif) != P_MAX_INDEX)
    foundHint = info.SetFrameSize(CIFWidth, CIFHeight);
  else if (str.FindRegEx(sqcif) != P_MAX_INDEX)
    foundHint = info.SetFrameSize(SQCIFWidth, SQCIFHeight);
  else if (str.FindRegEx(cif4) != P_MAX_INDEX)
    foundHint = info.SetFrameSize(CIF4Width, CIF4Height);
  else if (str.FindRegEx(cif16) != P_MAX_INDEX)
    foundHint = info.SetFrameSize(CIF16Width, CIF16Height);
  else if ((pos = str.FindRegEx(XbyY)) != P_MAX_INDEX) {
    unsigned width, height;
    if (sscanf(str.Mid(pos+1), "%ux%u", &width, &height) == 2)
      foundHint = info.SetFrameSize(width, height);
  }

  if ((pos = str.FindRegEx(fps)) != P_MAX_INDEX) {
    unsigned rate = str.Mid(pos+1).AsUnsigned();
    foundHint = info.SetFrameRate(rate);
  }

  return foundHint;
}


///////////////////////////////////////////////////////////////////////////////

PINSTANTIATE_FACTORY(PVideoFile, PDefaultPFactoryKey)
static PFactory<PVideoFile>::Worker<PYUVFile> yuvFileFactory("yuv");
static PFactory<PVideoFile>::Worker<PYUVFile> y4mFileFactory("y4m");


PYUVFile::PYUVFile()
  : y4mMode(FALSE)
{
}


BOOL PYUVFile::Open(const PFilePath & name, PFile::OpenMode mode, int opts)
{
  if (!PVideoFile::Open(name, mode, opts))
    return FALSE;

  y4mMode = name.GetType() *= ".y4m";

  if (y4mMode) {
    int ch;
    do {
      if ((ch = file.ReadChar()) < 0)
        return FALSE;
    }
    while (ch != '\n');
    headerOffset = file.GetPosition();
  }

  return TRUE;
}


BOOL PYUVFile::WriteFrame(const void * frame)
{
  if (y4mMode)
    file.WriteChar('\n');

  return file.Write(frame, frameBytes);
}


BOOL PYUVFile::ReadFrame(void * frame)
{
  if (y4mMode) {
    int ch;
    do {
      if ((ch = file.ReadChar()) < 0)
        return FALSE;
    }
    while (ch != '\n');
  }

  return PVideoFile::ReadFrame(frame);
}


#endif  // P_VIDFILE
#endif
