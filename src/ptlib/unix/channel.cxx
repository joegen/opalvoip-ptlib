/*
 * $Id: channel.cxx,v 1.5 1995/10/15 12:56:54 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: channel.cxx,v $
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

#include <ptlib.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>

///////////////////////////////////////////////////////////////////////////////
//
// PChannel
//

BOOL PChannel::SetIOBlock (BOOL isRead)
{
  PTimeInterval timeout = isRead ? readTimeout : writeTimeout;
  if (timeout != PMaxTimeInterval) 
    return PThread::Current()->PXBlockOnIO(os_handle, isRead, timeout);
  else
    return PThread::Current()->PXBlockOnIO(os_handle, isRead);
}


BOOL PChannel::Read(void * buf, PINDEX len)
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  if (!SetIOBlock(TRUE)) 
    lastError = Timeout;
  else if (ConvertOSError(lastReadCount = ::read(os_handle, buf, len)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


BOOL PChannel::Write(const void * buf, PINDEX len)
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  if (!SetIOBlock(FALSE))
    lastError = Timeout;
  else if (ConvertOSError(lastWriteCount = ::write(os_handle, buf, len)))
    return lastWriteCount >= len;

  lastWriteCount = 0;
  return FALSE;
}

BOOL PChannel::Close()
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  } else {
    os_handle = -1;
    return ConvertOSError(::close(os_handle));
  }
}

PString PChannel::GetErrorText() const
{
  return strerror(osError);
#if 0
#ifdef P_HPUX9
  if (osError > 0 && osError < sys_nerr)
    return sys_errlist[osError];
#else
  if (osError > 0 && osError < _sys_nerr)
    return _sys_errlist[osError];
#endif
  else
    return PString();
#endif
}

BOOL PChannel::ConvertOSError(int err)

{
  osError = (err >= 0) ? 0 : errno;

  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;

    case EEXIST:
      lastError = FileExists;
      break;
    case EISDIR:
    case EROFS:
      lastError = AccessDenied;
      break;
    case ETXTBSY:
      lastError = DeviceInUse;
      break;
    case EFAULT:
    case ELOOP:
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
