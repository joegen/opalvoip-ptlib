/*
 * memfile.cxx
 *
 * memory file I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * Equivalence Pty Ltd
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: memfile.cxx,v $
 * Revision 1.1  2002/06/26 09:03:16  craigs
 * Initial version
 *
 */

#include <ptlib.h>
#include <ptclib/memfile.h>

#ifdef __GNUC__
#pragma implementation "memfile.h"
#endif

PMemoryFile::PMemoryFile()
{
  position = 0;
}

PMemoryFile::PMemoryFile(const PBYTEArray & ndata)
{
  data = ndata;
  position = 0;
}


BOOL PMemoryFile::Read(void * buf, PINDEX len)
{
  if ((position + len) > data.GetSize())
    len = data.GetSize() - position;

  lastReadCount = len;

  if (len != 0) {
    ::memcpy(buf, position + (const BYTE * )data, len);
    position += len;
    lastReadCount = len;
  }

  return lastReadCount != 0;
}

BOOL PMemoryFile::Write(const void * buf, PINDEX len)
{
  memcpy(data.GetPointer(position+len) + position, buf, len);
  position += len;
  return TRUE;
}

off_t PMemoryFile::GetLength() const
{
  return data.GetSize();
}
      
BOOL PMemoryFile::SetLength(off_t len)
{
  return data.SetSize(len);
}

BOOL PMemoryFile::SetPosition(off_t pos, FilePositionOrigin origin)
{
  switch (origin) {
    case Start:
      if (pos > data.GetSize())
        return FALSE;
      position = pos;
      break;

    case Current:
      if (pos < -position || pos > (data.GetSize() - position))
        return FALSE;
      position += pos;
      break;

    case End:
      if (pos < -data.GetSize())
        return FALSE;
      position = data.GetSize() - pos;
      break;
  }
  return TRUE;
}

off_t PMemoryFile::GetPosition() const
{
  return position;
}


