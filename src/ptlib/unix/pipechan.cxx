/*
 * pipechan.cxx
 *
 * Sub-process commuicating with pip I/O channel implementation
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
 * $Log: pipechan.cxx,v $
 * Revision 1.15  1998/11/02 10:30:40  robertj
 * GNU v6 compatibility.
 *
 * Revision 1.14  1998/11/02 10:07:34  robertj
 * Added ReadStandardError implementation
 *
 * Revision 1.13  1998/10/30 13:02:50  robertj
 * New pipe channel enhancements.
 *
 * Revision 1.12  1998/10/26 11:09:56  robertj
 * added separation of stdout and stderr.
 *
 * Revision 1.11  1998/09/24 04:12:14  robertj
 * Added open software license.
 *
 */

#pragma implementation "pipechan.h"

#include <ptlib.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>

#include "../../common/ptlib/src/pipechan.cxx"

////////////////////////////////////////////////////////////////
//
//  PPipeChannel
//

PPipeChannel::PPipeChannel()
{
  toChildPipe[0] = toChildPipe[1] = -1;
  fromChildPipe[0] = fromChildPipe[1] = -1;
  stderrChildPipe[0] = stderrChildPipe[1] = -1;
}


BOOL PPipeChannel::PlatformOpen(const PString & subProgram,
                                const PStringArray & argumentList,
                                OpenMode mode,
                                BOOL searchPath,
                                BOOL stderrSeparate,
                                const PStringToString * environment)
{
  subProgName = subProgram;

  // setup the pipe to the child
  if (mode == ReadOnly)
    toChildPipe[0] = toChildPipe[1] = -1;
  else 
    PAssert(pipe(toChildPipe) == 0, POperatingSystemError);
 
  // setup the pipe from the child
  if (mode == WriteOnly)
    fromChildPipe[0] = fromChildPipe[1] = -1;
  else
    PAssert(pipe(fromChildPipe) == 0, POperatingSystemError);

  if (stderrSeparate)
    PAssert(pipe(stderrChildPipe) == 0, POperatingSystemError);
  else
    stderrChildPipe[0] = stderrChildPipe[1] = -1;

  // fork to allow us to execute the child
  if ((childPid = vfork()) != 0) {
    // setup the pipe to the child
    if (toChildPipe[0] != -1) 
      ::close(toChildPipe[0]);

    if (fromChildPipe[1] != -1)
      ::close(fromChildPipe[1]);
 
    if (stderrChildPipe[1] != -1)
      ::close(stderrChildPipe[1]);
 
    if (childPid < 0)
      return FALSE;

    os_handle = 0;
    return TRUE;
  }

  // the following code is in the child process

  // if we need to write to the child, make sure the child's stdin
  // is redirected
  if (toChildPipe[0] != -1) {
    ::close(STDIN_FILENO);
    ::dup(toChildPipe[0]);
    ::close(toChildPipe[0]);
    ::close(toChildPipe[1]);  
  } else {
    int fd = open("/dev/null", O_RDONLY);
    PAssertOS(fd >= 0);
    ::close(STDIN_FILENO);
    ::dup(fd);
    ::close(fd);
  }

  // if we need to read from the child, make sure the child's stdout
  // and stderr is redirected
  if (fromChildPipe[1] != -1) {
    ::close(STDOUT_FILENO);
    ::dup(fromChildPipe[1]);
    ::close(STDERR_FILENO);
    if (!stderrSeparate)
      ::dup(fromChildPipe[1]);
    ::close(fromChildPipe[1]);
    ::close(fromChildPipe[0]); 
  } else {
    int fd = ::open("/dev/null", O_WRONLY);
    PAssertOS(fd >= 0);
    ::close(STDOUT_FILENO);
    ::dup(fd);
    ::close(STDERR_FILENO);
    if (!stderrSeparate)
      ::dup(fd);
    ::close(fd);
  }

  if (stderrSeparate) {
    ::dup(stderrChildPipe[1]);
    ::close(stderrChildPipe[1]);
    ::close(stderrChildPipe[0]); 
  }

  // set the SIGINT and SIGQUIT to ignore so the child process doesn't
  // inherit them from the parent
  signal(SIGINT,  SIG_IGN);
  signal(SIGQUIT, SIG_IGN);

  // and set ourselves as out own process group so we don't get signals
  // from our parent's terminal (hopefully!)
  PSETPGRP();

  // setup the arguments, not as we are about to execl or exit, we don't
  // care about memory leaks, they are not real!
  char ** args = (char **)calloc(argumentList.GetSize()+2, sizeof(char *));
  args[0] = strdup(subProgName.GetTitle());
  PINDEX i;
  for (i = 0; i < argumentList.GetSize(); i++) 
    args[i+1] = argumentList[i].GetPointer();

  // Set up new environment if one specified.
  if (environment != NULL) {
    __environ = (char **)calloc(environment->GetSize()+1, sizeof(char*));
    for (i = 0; i < environment->GetSize(); i++) {
      PString str = environment->GetKeyAt(i) + '=' + environment->GetDataAt(i);
      __environ[i] = strdup(str);
    }
  }

  // execute the child as required
  if (searchPath)
    execvp(subProgram, args);
  else
    execv(subProgram, args);

  exit(2);
  return FALSE;
}


BOOL PPipeChannel::Close()
{
  if (!IsOpen())
    return TRUE;

  // close pipe from child
  if (fromChildPipe[0] != -1) {
    ::close(fromChildPipe[0]);
    fromChildPipe[0] = -1;
  }

  // close pipe to child
  if (toChildPipe[1] != -1) {
    ::close(toChildPipe[1]);
    toChildPipe[1] = -1;
  }

  // close pipe to child
  if (stderrChildPipe[0] != -1) {
    ::close(stderrChildPipe[0]);
    stderrChildPipe[0] = -1;
  }

  // kill the child process
  if (IsRunning()) {
    kill (childPid, SIGKILL);
    WaitForTermination();
  }

  // ensure this channel looks like it is closed
  os_handle = -1;
  childPid  = 0;

  return TRUE;
}

BOOL PPipeChannel::Read(void * buffer, PINDEX len)
{
  PAssert(IsOpen(), "Attempt to read from closed pipe");
  PAssert(fromChildPipe[0] != -1, "Attempt to read from write-only pipe");

  os_handle = fromChildPipe[0];
  BOOL status = PChannel::Read(buffer, len);
  os_handle = 0;
  return status;
}

BOOL PPipeChannel::Write(const void * buffer, PINDEX len)
{
  PAssert(IsOpen(), "Attempt to write to closed pipe");
  PAssert(toChildPipe[1] != -1, "Attempt to write to read-only pipe");

  os_handle = toChildPipe[1];
  BOOL status = PChannel::Write(buffer, len);
  os_handle = 0;
  return status;
}

BOOL PPipeChannel::Execute()
{
  flush();
  clear();
  if (toChildPipe[1] != -1) {
    ::close(toChildPipe[1]);
    toChildPipe[1] = -1;
  }
  return TRUE;
}


PPipeChannel::~PPipeChannel()
{
  Close();
}

int PPipeChannel::GetReturnCode() const
{
  return retVal;
}

BOOL PPipeChannel::IsRunning() const
{
  PAssert(childPid > 0, "IsRunning called for closed PPipeChannel");
  return kill(childPid, 0) == 0;
}

int PPipeChannel::WaitForTermination()
{
#ifdef P_PTHREADS
  if (kill (childPid, 0) == 0) {
    while (wait3(NULL, WUNTRACED, NULL) != childPid)
      ;
  }
#else
  if (kill (childPid, 0) == 0)
    return retVal = PThread::Current()->PXBlockOnChildTerminate(childPid, PMaxTimeInterval);
#endif

  ConvertOSError(-1);
  return -1;
}

int PPipeChannel::WaitForTermination(const PTimeInterval & timeout)
{
#ifdef P_PTHREADS
  if (kill (childPid, 0) == 0) {
    while (wait3(NULL, WUNTRACED, NULL) != childPid)
      ;
  }
#else
  if (kill (childPid, 0) == 0)
    return retVal = PThread::Current()->PXBlockOnChildTerminate(childPid, timeout);
#endif

  ConvertOSError(-1);
  return -1;
}

BOOL PPipeChannel::Kill(int killType)
{
  return ConvertOSError(kill (childPid, killType));
}

BOOL PPipeChannel::CanReadAndWrite()
{
  return TRUE;
}


BOOL PPipeChannel::ReadStandardError(PString & errors, BOOL wait)
{
  PAssert(IsOpen(), "Attempt to read from closed pipe");
  PAssert(stderrChildPipe[0] != -1, "Attempt to read from write-only pipe");

  os_handle = stderrChildPipe[0];
  
  BOOL status = FALSE;
  int available;
  if (ConvertOSError(ioctl(stderrChildPipe[0], FIONREAD, &available))) {
    if (available != 0)
      status = PChannel::Read(errors.GetPointer(available+1), available);
    else if (wait) {
      char firstByte;
      status = PChannel::Read(&firstByte, 1);
      if (status) {
        errors = firstByte;
        if (ConvertOSError(ioctl(stderrChildPipe[0], FIONREAD, &available))) {
          if (available != 0)
            status = PChannel::Read(errors.GetPointer(available+2)+1, available);
        }
      }
    }
  }

  os_handle = 0;
  return status;
}


