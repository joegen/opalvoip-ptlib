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
 */

#pragma implementation "pipechan.h"

#include <ptlib.h>
#include <ptlib/pipechan.h>
#include <ptlib/pprocess.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>

#if defined(P_LINUX) || defined(P_SOLARIS)
#include <termio.h>
#endif

#if defined(P_SOLARIS)
  #include <sys/filio.h>
#endif

#include "../common/pipechan.cxx"

#if defined(P_MACOSX) && !defined(P_IOS)
#  include <crt_externs.h>
#  define environ (*_NSGetEnviron())
#else
extern char ** environ;
#endif


#define PTraceModule() "PipeChannel"


int PX_NewHandle(const char *, int);


////////////////////////////////////////////////////////////////
//
//  PPipeChannel
//

PPipeChannel::PPipeChannel()
{
  m_childPID = -1;
  m_toChildPipe[0] = m_toChildPipe[1] = -1;
  m_fromChildPipe[0] = m_fromChildPipe[1] = -1;
  m_stderrChildPipe[0] = m_stderrChildPipe[1] = -1;
  m_returnCode = -1;
}


PBoolean PPipeChannel::PlatformOpen(const PString & subProgram,
                                const PStringArray & argumentList,
                                OpenMode mode,
                                PBoolean searchPath,
                                PBoolean stderrSeparate,
                                const PStringToString * environment)
{
#if defined(P_VXWORKS) || defined(P_RTEMS)
  PAssertAlways("PPipeChannel::PlatformOpen");
#else
  subProgName = subProgram;

  // setup the pipe to the child
  if (mode == ReadOnly)
    m_toChildPipe[0] = m_toChildPipe[1] = -1;
  else {
    PAssert(pipe(m_toChildPipe) == 0, POperatingSystemError);
    PX_NewHandle("PPipeChannel m_toChildPipe", PMAX(m_toChildPipe[0], m_toChildPipe[1]));
  }
 
  // setup the pipe from the child
  if (mode == WriteOnly || mode == ReadWriteStd)
    m_fromChildPipe[0] = m_fromChildPipe[1] = -1;
  else {
    PAssert(pipe(m_fromChildPipe) == 0, POperatingSystemError);
    PX_NewHandle("PPipeChannel m_fromChildPipe", PMAX(m_fromChildPipe[0], m_fromChildPipe[1]));
  }

  if (stderrSeparate)
    PAssert(pipe(m_stderrChildPipe) == 0, POperatingSystemError);
  else {
    m_stderrChildPipe[0] = m_stderrChildPipe[1] = -1;
    PX_NewHandle("PPipeChannel m_stderrChildPipe", PMAX(m_stderrChildPipe[0], m_stderrChildPipe[1]));
  }

  // fork to allow us to execute the child
#if defined(__BEOS__) || defined(P_IRIX)
  m_childPID = fork();
#else
  m_childPID = vfork();
#endif

  if (m_childPID < 0) {
    PTRACE(1, "Could not fork process: errno=" << errno);
    return false;
  }

  if (m_childPID > 0) {
#if PTRACING
    static const int TraceLevel = 5;
    if (PTrace::CanTrace(TraceLevel)) {
      ostream & log = PTRACE_BEGIN(TraceLevel);
      log << "Forked child process (pid=" << m_childPID << ") \"" << subProgram << '"';
      for (PINDEX i = 0; i < argumentList.GetSize(); ++i)
        log << " \"" << argumentList[i] << '"';
      log << PTrace::End;
    }
#endif

    // setup the pipe to the child
    if (m_toChildPipe[0] != -1) {
      ::close(m_toChildPipe[0]);
      m_toChildPipe[0] = -1;
    }

    if (m_fromChildPipe[1] != -1) {
      ::close(m_fromChildPipe[1]);
      m_fromChildPipe[1] = -1;
    }
 
    if (m_stderrChildPipe[1] != -1) {
      ::close(m_stderrChildPipe[1]);
      m_stderrChildPipe[1] = -1;
    }

    os_handle = 0;
    m_returnCode = -2; // Indicate are running
    return true;
  }

  // the following code is in the child process

  // if we need to write to the child, make sure the child's stdin
  // is redirected
  if (m_toChildPipe[0] != -1) {
    ::close(STDIN_FILENO);
    if (::dup(m_toChildPipe[0]) == -1)
      return false;
    ::close(m_toChildPipe[0]);
    ::close(m_toChildPipe[1]);  
  }
  else {
    int fd = open("/dev/null", O_RDONLY);
    PAssertOS(fd >= 0);
    ::close(STDIN_FILENO);
    if (::dup(fd) == -1)
      return false;
    ::close(fd);
  }

  // if we need to read from the child, make sure the child's stdout
  // and stderr is redirected
  if (m_fromChildPipe[1] != -1) {
    ::close(STDOUT_FILENO);
    if (::dup(m_fromChildPipe[1]) == -1)
      return false;
    ::close(STDERR_FILENO);
    if (!stderrSeparate)
      if (::dup(m_fromChildPipe[1]) == -1)
        return false;
    ::close(m_fromChildPipe[1]);
    ::close(m_fromChildPipe[0]); 
  }
  else if (mode != ReadWriteStd) {
    int fd = ::open("/dev/null", O_WRONLY);
    PAssertOS(fd >= 0);
    ::close(STDOUT_FILENO);
    if (::dup(fd) == -1)
      return false;
    ::close(STDERR_FILENO);
    if (!stderrSeparate)
      if (::dup(fd) == -1)
        return false;
    ::close(fd);
  }

  if (stderrSeparate) {
    if (::dup(m_stderrChildPipe[1]) == -1)
      return false;
    ::close(m_stderrChildPipe[1]);
    ::close(m_stderrChildPipe[0]); 
  }

  // Restore signal handlers so the child process doesn't
  // inherit them from the parent
  PProcess::Current().RemoveRunTimeSignalHandlers();

  // and set ourselves as out own process group so we don't get signals
  // from our parent's terminal (hopefully!)
  PSETPGRP();

  // setup the arguments and environment, note as we are about to exec or
  // exit, we don't care about memory leaks, they are not real!
  char ** argv;
  if (argumentList[0] == subProgram)
    argv = argumentList.ToCharArray();
  else {
    PStringArray withProgram(argumentList.GetSize()+1);
    withProgram[0] = subProgram;
    for (PINDEX i = 0; i < argumentList.GetSize(); ++i)
      withProgram[i+1] = argumentList[i];
    argv = withProgram.ToCharArray();
  }

  // run the program, does not return
  if (environment == NULL) {
    if (searchPath)
      execvp(subProgram, argv);
    else
      execv(subProgram, argv);
  }
  else {
    char ** envp = environment != NULL ? environment->ToCharArray(true) : environ;
    if (!searchPath)
      execve(subProgram, argv, envp);
    else {
      #if __GLIBC__ >3 || __GLIBC__ == 2 && __GLIBC_MINOR__ >= 11
        execvpe(subProgram, argv, envp);
      #else
        // Need to search path manually
        PString path(getenv("PATH"));
        if (path.IsEmpty())
          path = ".:/bin:/usr/bin";
        PStringArray dir = path.Tokenise(':', false);
        for (PINDEX i = 0; i < dir.GetSize(); ++i) {
          PString progPath = dir[i] + '/' + subProgram;
          if (PFile::Exists(progPath)) {
            execve(progPath, argv, envp);
            break;
          }
        }
        execve(subProgram, argv, envp);
      #endif
    }
  }

  // Returned! Error!
  _exit(errno != 0 ? errno : 1);
#endif // P_VXWORKS || P_RTEMS

  return false;
}


PBoolean PPipeChannel::Close()
{
  bool wasRunning = false;

  // close pipe from child
  if (m_fromChildPipe[0] != -1) {
    ::close(m_fromChildPipe[0]);
    m_fromChildPipe[0] = -1;
  }

  if (m_fromChildPipe[1] != -1) {
    ::close(m_fromChildPipe[1]);
    m_fromChildPipe[1] = -1;
  }

  // close pipe to child
  if (m_toChildPipe[0] != -1) {
    ::close(m_toChildPipe[0]);
    m_toChildPipe[0] = -1;
  }

  if (m_toChildPipe[1] != -1) {
    ::close(m_toChildPipe[1]);
    m_toChildPipe[1] = -1;
  }

  // close pipe to child
  if (m_stderrChildPipe[0] != -1) {
    ::close(m_stderrChildPipe[0]);
    m_stderrChildPipe[0] = -1;
  }

  if (m_stderrChildPipe[1] != -1) {
    ::close(m_stderrChildPipe[1]);
    m_stderrChildPipe[1] = -1;
  }

  // kill the child process
  if (IsRunning()) {
    wasRunning = true;
    PTRACE(4, "Child being sent SIGKILL");
    kill(m_childPID, SIGKILL);
    WaitForTermination();
  }

  // ensure this channel looks like it is closed
  os_handle = m_childPID = -1;
  if (m_returnCode == -2)
    m_returnCode = -1;

  return wasRunning;
}


PBoolean PPipeChannel::Read(void * buffer, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  if (!PAssert(m_fromChildPipe[0] != -1, "Attempt to read from write-only pipe"))
    return false;

  os_handle = m_fromChildPipe[0];
  return PChannel::Read(buffer, len);
}


PBoolean PPipeChannel::Write(const void * buffer, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  if (!PAssert(m_toChildPipe[1] != -1, "Attempt to write to read-only pipe"))
    return false;

  os_handle = m_toChildPipe[1];
  return PChannel::Write(buffer, len);
}


PBoolean PPipeChannel::Execute()
{
  flush();
  clear();

  if (m_toChildPipe[1] != -1) {
    ::close(m_toChildPipe[1]);
    PTRACE(5, "Closed pipe to child: fd=" << m_toChildPipe[1]);
    m_toChildPipe[1] = -1;
  }

  return IsRunning();
}


PPipeChannel::~PPipeChannel()
{
  Close();
}

int PPipeChannel::GetReturnCode() const
{
  return m_returnCode;
}


PBoolean PPipeChannel::IsRunning() const
{
  return const_cast<PPipeChannel *>(this)->WaitForTermination(0) < -1;
}


int PPipeChannel::WaitForTermination()
{
  return WaitForTermination(PMaxTimeInterval);
}


int PPipeChannel::WaitForTermination(const PTimeInterval & timeout)
{
  if (m_childPID < 0)
    return m_returnCode;

#if defined(P_PTHREADS)
  PAssert(timeout == 0 || timeout == PMaxTimeInterval, PUnimplementedFunction);
  int result, status;
  while ((result = waitpid(m_childPID, &status, timeout == 0 ? WNOHANG : 0)) != m_childPID) {
    if (result == 0)
      return -2; // Still running

    if (errno != EINTR) {
      ConvertOSError(-1);
      return -1;
    }
  }

  m_childPID = -1;
  if (WIFEXITED(status)) {
    m_returnCode = WEXITSTATUS(status);
    PTRACE(3, "Child exited with code " << m_returnCode);
  }
  else if (WIFSIGNALED(status)) {
    PTRACE(3, "Child was terminated with signal " << WTERMSIG(status));
    m_returnCode = WTERMSIG(status) + 256;
  }
  else {
    PTRACE(3, "Child was stopped with unknown status" << status);
    m_returnCode = 256;
  }

#else
  if (ConvertOSError(kill(m_childPID, 0)))
    m_returnCode = PThread::Current()->PXBlockOnChildTerminate(m_childPID, timeout);
#endif

  return m_returnCode;
}

PBoolean PPipeChannel::Kill(int killType)
{
  PTRACE(4, "Child being sent signal " << killType);
  return ConvertOSError(kill(m_childPID, killType));
}

PBoolean PPipeChannel::CanReadAndWrite()
{
  return true;
}


PBoolean PPipeChannel::ReadStandardError(PString & errors, PBoolean wait)
{
  if (CheckNotOpen())
    return false;

  if (!PAssert(m_stderrChildPipe[0] != -1, "Attempt to read from write-only pipe"))
    return false;

  os_handle = m_stderrChildPipe[0];
  
  PBoolean status = false;
#ifndef BE_BONELESS
  int available;
  if (ConvertOSError(ioctl(m_stderrChildPipe[0], FIONREAD, &available))) {
    if (available != 0)
      status = PChannel::Read(errors.GetPointerAndSetLength(available+1), available);
    else if (wait) {
      char firstByte;
      status = PChannel::Read(&firstByte, 1);
      if (status) {
        errors = firstByte;
        if (ConvertOSError(ioctl(m_stderrChildPipe[0], FIONREAD, &available))) {
          if (available != 0)
            status = PChannel::Read(errors.GetPointerAndSetLength(available+2)+1, available);
        }
      }
    }
  }
#endif

  return status;
}


int PPipeChannel::Run(const PString & command, PString & output, bool includeStderr, const PTimeInterval & timeout)
{
  output.MakeEmpty();

  PString cmdWithStderr = command;
  if (includeStderr)
    cmdWithStderr += " 2>&1";
  FILE * pipe = popen(cmdWithStderr, "r");
  if (pipe == NULL) {
    PTRACE2(2, NULL, "Could not execute command [" << command << "] - errno=" << errno);
    return -1;
  }

  int c;
  while ((c = fgetc(pipe)) != EOF)
    output += (char)c;

  int status = pclose(pipe);

#if PTRACING
  ostream & trace = PTRACE_BEGIN(4, NULL, PTraceModule());
  trace << "Sub-process [" << command << "] executed: status=" << WEXITSTATUS(status);
  if (WIFSIGNALED(status))
      trace << ", signal=" << WTERMSIG(status);
  if (WCOREDUMP(status))
      trace << ", core dumped";
  trace << PTrace::End;
#endif

  if (WCOREDUMP(status))
    return INT_MIN;

  if (WIFSIGNALED(status))
    return - WTERMSIG(status);

  return WEXITSTATUS(status);
}


