/*
 * channel.cxx
 *
 * I/O channel classes implementation
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
 * Contributor(s): ______________________________________.
 *
 * $Log: channel.cxx,v $
 * Revision 1.20  1998/10/11 02:23:16  craigs
 * Fixed problem with socket writes not correctly detecting EOF
 *
 * Revision 1.19  1998/09/24 04:12:09  robertj
 * Added open software license.
 *
 * Revision 1.18  1998/08/27 01:06:30  robertj
 * Fixed very strange link error with GNU C library v6.
 *
 * Revision 1.17  1998/05/25 10:03:26  robertj
 * Fixed problem with socket/channel close and blocked threads.
 *
 * Revision 1.16  1998/03/26 05:01:12  robertj
 * Added PMutex and PSyncPoint classes.
 *
 * Revision 1.15  1998/01/03 22:35:04  craigs
 * Added PThread support
 *
 * Revision 1.14  1997/02/14 09:18:36  craigs
 * Changed for PProcess::Current being a reference rather that a ptr
 *
 * Revision 1.13  1996/11/03 04:35:32  craigs
 * Added PSocket::Read to fix recv/read problem
 *
 * Revision 1.12  1996/09/21 05:38:28  craigs
 * Added indchan pragma
 *
 * Revision 1.11  1996/08/03 12:04:28  craigs
 * Fixed problem with PChannel::Write terminating early
 * Changed for new PChannel error reporting functions
 *
 * Revision 1.10  1996/05/25 06:06:33  craigs
 * Sun4 fixes and updated for gcc 2.7.2
 *
 * Revision 1.9  1996/05/03 13:11:35  craigs
 * More Sun4 fixes
 *
 * Revision 1.8  1996/05/02 12:01:23  craigs
 * More Sun4 fixes
 *
 * Revision 1.7  1996/04/15 10:49:11  craigs
 * Last build prior to release of MibMaster v1.0
 *
 * Revision 1.6  1996/01/26 11:09:42  craigs
 * Fixed problem with blocking accepts and incorrect socket errors
 *
 * Revision 1.5  1995/10/15 12:56:54  craigs
 * Multiple updates - split channel implementation into multiple files
 *
 * Revision 1.4  1995/07/09 00:35:43  craigs
 * Latest and greatest omnibus change
 *
 * Revision 1.3  1995/02/15 20:28:14  craigs
 * Removed sleep after pipe channel open
 *
// Revision 1.2  1995/01/23  22:58:01  craigs
// Changes for HPUX and Sun 4
//
 */

#pragma implementation "channel.h"
#pragma implementation "indchan.h"

#include <ptlib.h>
#include <sys/ioctl.h>

PMutex PX_iostreamMutex;

ios::ios(const ios &)
{
  PAssertAlways("Cannot copy ios");
}

///////////////////////////////////////////////////////////////////////////////
//
// PChannel::PXSetIOBlock
//   These functions are used to perform IO blocks.
//   If the return value is FALSE, then the select call either
//   returned an error or a timeout occurred. The member variable lastError
//   can be used to determine which error occurred
//

BOOL PChannel::PXSetIOBlock (int type, const PTimeInterval & timeout)
{
  return PXSetIOBlock(type, os_handle, timeout);
}

BOOL PChannel::PXSetIOBlock (int type, int blockHandle, const PTimeInterval & timeout)
{
  if (blockHandle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  int stat = PThread::Current()->PXBlockOnIO(blockHandle, type, timeout);

  // if select returned < 0, then covert errno into lastError and return FALSE
  if (stat < 0)
    return ConvertOSError(stat);

  // if the select succeeded, then return TRUE
  if (stat > 0) 
    return TRUE;

  // otherwise, a timeout occurred so return FALSE
  lastError = Timeout;
  return FALSE;
}


BOOL PChannel::Read(void * buf, PINDEX len)
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  if (!PXSetIOBlock(PXReadBlock, readTimeout)) 
    return FALSE;

  if (ConvertOSError(lastReadCount = ::read(os_handle, buf, len)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


BOOL PChannel::Write(const void * buf, PINDEX len)
{
  // if the os_handle isn't open, no can do
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  // flush the buffer before doing a write
  PX_iostreamMutex.Wait();
  flush();
  PX_iostreamMutex.Signal();

  lastWriteCount = 0;
  
  while (len > 0) {

    int sendResult = ::write(os_handle,
                  ((const char *)buf)+lastWriteCount, len);
    if (sendResult <= 0) {
      if (errno != EWOULDBLOCK)
        return ConvertOSError(sendResult);

      if (!PXSetIOBlock(PXWriteBlock, writeTimeout))
        return FALSE;

      sendResult = ::write(os_handle,
                    ((const char *)buf)+lastWriteCount, len);

    }

    if (!ConvertOSError(sendResult))
      return FALSE;

    lastWriteCount += sendResult;
    len -= sendResult;
  }

  return TRUE;
}

BOOL PChannel::Close()
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }
  
  return ConvertOSError(PXClose());
}

BOOL PChannel::PXClose()
{
  if (os_handle < 0)
    return -1;

  // make sure we don't have any problems
  PX_iostreamMutex.Wait();
  flush();
  int handle = os_handle;
  os_handle = -1;
  PX_iostreamMutex.Signal();

#ifndef P_PTHREADS
  // abort any I/O block using this os_handle
  PProcess::Current().PXAbortIOBlock(handle);

  DWORD cmd = 0;
  ::ioctl(handle, FIONBIO, &cmd);
#endif

  int stat;
  do {
    stat = ::close(handle);
  } while (stat == -1 && errno == EINTR);

  return stat;
}

PString PChannel::GetErrorText(Errors, int osError = 0)
{
  return strerror(osError);
}

PString PChannel::GetErrorText() const
{
  return GetErrorText(lastError, osError);
}

BOOL PChannel::ConvertOSError(int err)
{
  return ConvertOSError(err, lastError, osError);
}

BOOL PChannel::ConvertOSError(int err, Errors & lastError, int & osError)

{
  osError = (err >= 0) ? 0 : errno;

  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;

    case EMSGSIZE:
      lastError = BufferTooSmall;
      break;

    case EINTR:
      lastError = Interrupted;
      break;

    case EEXIST:
      lastError = FileExists;
      break;

    case EISDIR:
    case EROFS:
    case EACCES:
    case EPERM:
      lastError = AccessDenied;
      break;

    case ETXTBSY:
      lastError = DeviceInUse;
      break;

    case EFAULT:
    case ELOOP:
    case EBADF:
    case EINVAL:
      lastError = BadParameter;
      break;

    case ENOENT :
    case ENAMETOOLONG:
    case ENOTDIR:
      lastError = NotFound;
      break;

    case EMFILE:
    case ENFILE:
    case ENOMEM :
      lastError = NoMemory;
      break;

    case ENOSPC:
      lastError = DiskFull;
      break;

    default :
      lastError = Miscellaneous;
      break;
  }
  return FALSE;
}
