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
 */

#pragma implementation "channel.h"
#pragma implementation "indchan.h"

#include <ptlib.h>
#include <sys/ioctl.h>

#if defined(P_SOLARIS)
  #include <sys/filio.h>
#endif

#include "../common/pchannel.cxx"


void PChannel::Construct()
{
  os_handle = -1;
  px_lastBlockType = PXReadBlock;
  px_readThread = NULL;
  px_writeThread = NULL;
  px_selectThread[0] = NULL;
  px_selectThread[1] = NULL;
  px_selectThread[2] = NULL;
}


///////////////////////////////////////////////////////////////////////////////
//
// PChannel::PXSetIOBlock
//   This function is used to perform IO blocks.
//   If the return value is false, then the select call either
//   returned an error or a timeout occurred. The member variable lastError
//   can be used to determine which error occurred
//

PBoolean PChannel::PXSetIOBlock(PXBlockType type, const PTimeInterval & timeout)
{
  if (CheckNotOpen())
    return false;

  ErrorGroup group;
  switch (type) {
    case PXReadBlock :
      group = LastReadError;
      break;
    case PXWriteBlock :
      group = LastWriteError;
      break;
    default :
      group = LastGeneralError;
  }

  PThread * blockedThread = PThread::Current();

  {
    PWaitAndSignal mutex(px_threadMutex);
    switch (type) {
      case PXWriteBlock :
        if (px_readThread != NULL && px_lastBlockType != PXReadBlock)
          return SetErrorValues(DeviceInUse, EBUSY, group);

        PTRACE(6, "PTLib\tBlocking on write.");
        px_writeMutex.Wait();
        px_writeThread = blockedThread;
        break;

      case PXReadBlock :
        if (px_readThread != NULL && px_lastBlockType == PXReadBlock)
          PAssertAlways(PSTRSTRM("Attempt to do simultaneous reads from multiple threads:"
                                 " os_handle=" << os_handle << ","
                                 " this-thread=" << *blockedThread << ","
                                 " other-thread=" << *px_readThread));
        // Fall into default case

      default :
        if (px_readThread != NULL)
          return SetErrorValues(DeviceInUse, EBUSY, group);
        px_readThread = blockedThread;
        px_lastBlockType = type;
    }
  }

  int stat = blockedThread->PXBlockOnIO(os_handle, type, timeout);

  if (type != PXWriteBlock) {
    px_threadMutex.Wait();
    px_lastBlockType = PXReadBlock;
    px_readThread = NULL;
    px_threadMutex.Signal();
  }
  else {
    px_writeMutex.Signal(); // Must be outside of px_threadMutex
    px_threadMutex.Wait();
    px_writeThread = NULL;
    px_threadMutex.Signal();
  }

  // if select returned < 0, then convert errno into lastError and return false
  if (stat < 0)
    return ConvertOSError(stat, group);

  // if the select succeeded, then return true
  if (stat > 0) 
    return true;

  // otherwise, a timeout occurred so return false
  return SetErrorValues(Timeout, ETIMEDOUT, group);
}

FILE * PChannel::FDOpen(const char * mode)
{
  FILE * h = fdopen(os_handle, mode);
  if (h != NULL)
    os_handle = -1;
  return h;
}


PBoolean PChannel::Read(void * buf, PINDEX len)
{
  SetLastReadCount(0);

  if (CheckNotOpen())
    return false;

  for (;;) {
    PPROFILE_SYSTEM(
      int result = ::read(os_handle, buf, len);
    );
    if (result >= 0)
      return ConvertOSError(SetLastReadCount(result), LastReadError);

    switch (errno) {
      case EINTR :
        break;

      case EWOULDBLOCK :
        if (readTimeout > 0) {
          if (PXSetIOBlock(PXReadBlock, readTimeout))
            break;
          return false;
        }
        // Next case

      default :
        return ConvertOSError(-1, LastReadError);
    }
  }
}


PBoolean PChannel::Write(const void * buf, PINDEX len)
{
  SetLastWriteCount(0);

  if (CheckNotOpen())
    return false;

  // flush the buffer before doing a write
  flush();

  PINDEX written = 0;
  while (len > 0) {

    int result;
    while ((result = ::write(os_handle, ((char *)buf)+written, len)) < 0) {
      switch (errno) {
        case EINTR :
          break;

        case EWOULDBLOCK :
          if (writeTimeout > 0) {
            if (PXSetIOBlock(PXWriteBlock, writeTimeout))
              break;
            return false;
          }
          // Next case

        default :
          return ConvertOSError(-1, LastWriteError);
      }
    }

    written += result;
    len -= result;
  }

  // Reset all the errors.
  return ConvertOSError(0, LastWriteError) && SetLastWriteCount(written) > 0;
}


#if defined _AIO_H

static void StaticOnIOComplete(union sigval sig)
{
  PChannel::AsyncContext * context = (PChannel::AsyncContext *)sig.sival_ptr;
  context->OnIOComplete(aio_return(context), aio_error(context));
}


void PChannel::AsyncContext::SetOffset(off_t offset)
{
  aio_offset = offset;
}


bool PChannel::AsyncContext::Initialise(PChannel * channel, CompletionFunction onComplete)
{
  if (m_channel != NULL)
    return false;

  m_channel = channel;
  m_onComplete = onComplete;

  aio_fildes = channel->GetHandle();
  aio_buf    = m_buffer;
  aio_nbytes = m_length;
  aio_sigevent.sigev_notify = SIGEV_THREAD;
  aio_sigevent.sigev_notify_function = StaticOnIOComplete;
  aio_sigevent.sigev_value.sival_ptr = this;

  // If doing async, need to be blocking mode, seems but there it is
  int cmd = 0;
  ::ioctl(aio_fildes, FIONBIO, &cmd);
  return true;
}


PBoolean PChannel::ReadAsync(AsyncContext & context)
{
  PTRACE(6, "Async\tStarting ReadAsync");
  return PAssert(context.Initialise(this, &PChannel::OnReadComplete),
                 "Multiple async read with same context!") &&
         ConvertOSError(aio_read(&context), LastReadError);
}


PBoolean PChannel::WriteAsync(AsyncContext & context)
{
  PTRACE(6, "Async\tStarting WriteAsync");
  return PAssert(context.Initialise(this, &PChannel::OnWriteComplete),
                 "Multiple async write with same context!") &&
         ConvertOSError(aio_write(&context), LastWriteError);
}


#else // _AIO_H


void PChannel::AsyncContext::SetOffset(off_t)
{
}


bool PChannel::AsyncContext::Initialise(PChannel *, CompletionFunction)
{
  return false;
}


PBoolean PChannel::ReadAsync(AsyncContext & context)
{
  return false;
}


PBoolean PChannel::WriteAsync(AsyncContext & context)
{
  return false;
}

#endif // _AIO_H


PBoolean PChannel::Close()
{
  if (CheckNotOpen())
    return false;

  return ConvertOSError(PXClose());
}


static void AbortIO(PThread * & thread, PMutex & mutex)
{
  mutex.Wait();
  if (thread != NULL) {
    thread->PXAbortBlock();

    while (thread != NULL) {
      mutex.Signal();
      usleep(1);
      mutex.Wait();
    }
  }

  mutex.Signal();
}

int PChannel::PXClose()
{
  if (os_handle < 0)
    return -1;

  PTRACE(6, "PTLib\tClosing channel, fd=" << os_handle);

  // make sure we don't have any problems
  flush();
  int handle = os_handle.exchange(-1);

  AbortIO(px_readThread, px_threadMutex);
  AbortIO(px_writeThread, px_threadMutex);
  for (PINDEX i = 0; i < 3; ++i)
    AbortIO(px_selectThread[i], px_threadMutex);

  int stat;
  do {
    stat = ::close(handle);
  } while (stat == -1 && errno == EINTR);

  return stat;
}

PString PChannel::GetErrorText(Errors normalisedError, int osError /* =0 */)
{
  if (osError == 0) {
    if (normalisedError == NoError)
      return PString();

    static int const errors[NumNormalisedErrors] = {
      0, ENOENT, EEXIST, ENOSPC, EACCES, EBUSY, EINVAL, ENOMEM, EBADF, EAGAIN, EINTR,
      EMSGSIZE, EIO, 0x1000000
    };
    osError = errors[normalisedError];
  }

  if (osError == 0x1000000)
    return "High level protocol failure";

  const char * err = strerror(osError);
  if (err != NULL)
    return err;

  return psprintf("Unknown error %d", osError);
}


PBoolean PChannel::ConvertOSError(P_INT_PTR libcReturnValue, ErrorGroup group)
{
  int osError = (libcReturnValue >= 0) ? 0 : os_errno();
  Errors lastError;

  switch (osError) {
    case 0 :
      lastError = NoError;
      break;

    case EMSGSIZE:
      lastError = BufferTooSmall;
      break;

    case EBADF:  // will get EBADF if a read/write occurs after closing. This must return Interrupted
    case EINTR:
    case ECANCELED :
      lastError = Interrupted;
      break;

    case EWOULDBLOCK :
    case ETIMEDOUT :
      lastError = Timeout;
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

#ifndef __BEOS__
    case ETXTBSY:
      lastError = DeviceInUse;
      break;
#endif

    case EFAULT:
    case ELOOP:
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

  return SetErrorValues(lastError, osError, group);
}


///////////////////////////////////////////////////////////////////////////////

