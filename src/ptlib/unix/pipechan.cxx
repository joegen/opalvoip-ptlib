
#pragma implementation "pipechan.h"

#include <ptlib.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>

#include "../../common/src/pipechan.cxx"

////////////////////////////////////////////////////////////////
//
//  PPipeChannel
//

void PPipeChannel::Construct(const PString & subProgram,
               const char * const * arguments,
                                OpenMode mode,
                                    BOOL searchPath)

{
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

  // setup the arguments
  const char * cmd;
  char ** args;
  PINDEX i, l;
  PStringArray array = subProgram.Tokenise(" ", FALSE);

  if (arguments != NULL) {
    for (i = 0; arguments[i] != NULL; i++)
      ;
    l = i;
    args = new char *[l+2];
    for (i = 0; i < l; i++) 
      args[i+1] = (char *)arguments[i];
    cmd = subProgram;
  } else {
    array = subProgram.Tokenise(" ", FALSE);
    l = array.GetSize();
    args = new char *[l+2];
    for (i = 0; i < l; i++) 
      args[i+1] = array[i].GetPointer();
    cmd     = array[0];
  }
  PString arg0Str = PFilePath(cmd).GetTitle();
  args[0]   = arg0Str.GetPointer();
  args[l+1] = NULL;

  

  // fork to allow us to execute the child
  if ((childPid = vfork()) == 0) {

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
      ::dup(fromChildPipe[1]);
      ::close(fromChildPipe[1]);
      ::close(fromChildPipe[0]); 
    } else {
      int fd = ::open("/dev/null", O_WRONLY);
      PAssertOS(fd >= 0);
      ::close(STDOUT_FILENO);
      ::dup(fd);
      ::close(STDERR_FILENO);
      ::dup(fd);
      ::close(fd);
    }

    // set the SIGINT and SIGQUIT to ignore so the child process doesn't
    // inherit them from the parent
    signal(SIGINT,  SIG_IGN);
    signal(SIGQUIT, SIG_IGN);

    // and set ourselves as out own process group so we don't get signals
    // from our parent's terminal (hopefully!)
    PSETPGRP();

    // execute the child as required
    if (searchPath)
      execvp(cmd, args);
    else
      execv(cmd, args);

    PError << "fatal error: child process failed to exec" << endl;
  }

  PAssert(childPid >= 0, POperatingSystemError);

  // setup the pipe to the child
  if (toChildPipe[0] != -1) 
    ::close(toChildPipe[0]);

  if (fromChildPipe[1] != -1)
    ::close(fromChildPipe[1]);
 
  if (args != NULL)
    delete args;

  os_handle = 0;
}

BOOL PPipeChannel::Close()
{
  if (!IsOpen())
    return TRUE;

  // close pipe from child
  if (fromChildPipe[0] != -1) {
    if (!ConvertOSError(::close(fromChildPipe[0])))
      return FALSE;
    fromChildPipe[0] = -1;
  }

  // close pipe to child
  if (toChildPipe[1] != -1) {
    if (!ConvertOSError(::close(toChildPipe[1])))
      return FALSE;
    toChildPipe[1] = -1;
  }

  // kill the child process
  if (IsRunning()) {
    kill (childPid, SIGKILL);
    PXWaitForTerminate();
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

void PPipeChannel::PXWaitForTerminate()
{
  PAssert(childPid > 0, "waiting on closed PPipeChannel");
  if (kill (childPid, 0) == 0)
    retVal = PThread::Current()->PXBlockOnChildTerminate(childPid);
}

void PPipeChannel::PXKill(int killType)
{
  if (childPid > 0)
    kill (childPid, killType);
}

BOOL PPipeChannel::CanReadAndWrite()
  { return TRUE; }
