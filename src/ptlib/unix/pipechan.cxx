
#pragma implementation "pipechan.h"

#if defined (P_SUN4)
extern "C" int vfork();
#endif

#include <ptlib.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>

////////////////////////////////////////////////////////////////
//
//  SIGPIPE signal handler
//

static void PipeSignalHandler(int parm)
{
#if 1
  PError << "SIGPIPE" << endl;
#endif
  signal(SIGPIPE, PipeSignalHandler);
}

static void (*oldPipeSignalHandler)(int) = NULL;

////////////////////////////////////////////////////////////////
//
//  SIGCLD signal handler
//

static void ChildSignalHandler(int parm)
{
#if 0
    PError << "SIGCLD" << endl;
#endif
}

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
  else {
    if (oldPipeSignalHandler == NULL) 
      oldPipeSignalHandler = signal(SIGPIPE, PipeSignalHandler);
    PAssert(pipe(toChildPipe) == 0, POperatingSystemError);
  }
 
  // setup the pipe from the child
  if (mode == WriteOnly)
    fromChildPipe[0] = fromChildPipe[1] = -1;
  else
    PAssert(pipe(fromChildPipe) == 0, POperatingSystemError);

  // setup the arguments
  const char * cmd;
  char * const * args;
  char ** newArgs = NULL;
  PStringArray array;

  if (arguments != NULL) {
    cmd  = subProgram;
    args = (char * const *)arguments;
  } else {
    array = subProgram.Tokenise(" ", FALSE);
    int l = array.GetSize();
    newArgs = new char *[l+1];
    for (PINDEX i = 0; i < l; i++) 
      newArgs[i] = array[i];
    newArgs[i] = NULL;
    args = (char * const *)newArgs;
    cmd = args[0];
  }

  // fork to allow us to execute the child
  signal(SIGCLD, ChildSignalHandler);
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

    // execute the child as required
    if (searchPath)
      execvp(cmd, args);
    else
      execv(cmd, args);

    PError << "fatal error: child process failed to exec" << endl;
  }

  // setup the pipe to the child
  if (toChildPipe[0] != -1) 
    ::close(toChildPipe[0]);

  if (fromChildPipe[1] != -1)
    ::close(fromChildPipe[1]);
 
  if (newArgs != NULL)
    delete newArgs;

  PAssert(childPid >= 0, POperatingSystemError);

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

  // if the child is still alive, then kill it
  if (kill(childPid, 0) == 0)
    kill (childPid, SIGKILL);

  // wait for the return status so we don't get a zombie
  int retVal;
  waitpid(childPid, &retVal, WNOHANG);

  // ensure this channel looks like it is closed
  os_handle = -1;

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


