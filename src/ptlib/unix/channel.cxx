/*
 * $Id: channel.cxx,v 1.3 1995/02/15 20:28:14 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: channel.cxx,v $
 * Revision 1.3  1995/02/15 20:28:14  craigs
 * Removed sleep after pipe channel open
 *
// Revision 1.2  1995/01/23  22:58:01  craigs
// Changes for HPUX and Sun 4
//
 */

#pragma implementation "channel.h"
#pragma implementation "serchan.h"
#pragma implementation "pipechan.h"
#pragma implementation "modem.h"
#pragma implementation "socket.h"
#pragma implementation "ipsock.h"
#pragma implementation "udpsock.h"
#pragma implementation "tcpsock.h"
#pragma implementation "telnet.h"

#include "ptlib.h"
#include "sockets.h"

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>


#if defined (P_SUN4)
extern "C" int vfork();
#endif

#define BINARY_LOCK	1
#define	LOCK_PREFIX	"/var/spool/uucp/LCK.."
#define	DEV_PREFIX	"/dev/"

#define	PORT_PREFIX	"ttyS"
#define	PORT_PREFIX_LEN	4
#define	PORT_START	0
#define	PORT_COUNT	4


///////////////////////////////////////////////////////////////////////////////
//
// PChannel
//

BOOL PChannel::SetIOBlock (BOOL isRead)
{
  PTimeInterval timeout = isRead ? readTimeout : writeTimeout;
  if (timeout != PMaxTimeInterval) 
    PThread::Current()->PXBlockOnIO(os_handle, isRead, timeout);
  else
    PThread::Current()->PXBlockOnIO(os_handle, isRead);

  return FALSE;
}


BOOL PChannel::Read(void * buf, PINDEX len)
{
  SetIOBlock(TRUE);
  if (ConvertOSError(lastReadCount = ::read(os_handle, buf, len)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


BOOL PChannel::Write(const void * buf, PINDEX len)
{
  SetIOBlock(FALSE);
  if (ConvertOSError(lastWriteCount = ::write(os_handle, buf, len)))
    return lastWriteCount >= len;

  lastWriteCount = 0;
  return FALSE;
}

BOOL PChannel::Close()
{
  if (os_handle >= 0) 
    return ConvertOSError(::close(os_handle));
  else
    return FALSE;
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

////////////////////////////////////////////////////////////////
//
//  PSerialChannel
//

BOOL PSerialChannel::Close()
{
  // delete the lockfile
  if (os_handle >= 0) {
    PFile::Remove(PString(LOCK_PREFIX) + channelName);
    delete (Termio);
  }

  return PChannel::Close();
}


BOOL PSerialChannel::Open(const PString & port, 
                                    DWORD speed,
                                     BYTE data,
                                   Parity parity,
                                     BYTE stop,
                              FlowControl inputFlow,
                              FlowControl outputFlow)
{
  // if the port is already open, close it
  if (IsOpen())
    Close();

  // check prefix of name
  if (port.Left(PORT_PREFIX_LEN) != PORT_PREFIX) {
    lastError = BadParameter;
    return FALSE;
  }

  // check suffix
  int portnum = (port.Right(port.GetLength()-PORT_PREFIX_LEN)).AsInteger();
  if ((portnum < PORT_START) || (portnum >= (PORT_START + PORT_COUNT))) {
    lastError = BadParameter;
    return FALSE;
  }

  // save the port name
  channelName = port;

  // construct lock filename 
  PString lockfilename = PString(LOCK_PREFIX) + port;

  // if the file exists, probe the process to see if it is still running
  if (PFile::Exists(lockfilename)) {
    PFile lockfile(lockfilename, PFile::ReadOnly);

    int lock_pid;
#ifdef BINARY_LOCK
    lockfile.Read(&lock_pid, sizeof(lock_pid));
#else
    char lock_pid_str[20];
    lockfile.Read(&lock_pid_str, 20);
    lock_pid = atoi(lock_pid_str);
#endif
    
    // if kill returns 0, then the port is in use
    if (kill(lock_pid, 0) == 0) {
      lastError = DeviceInUse;
      return FALSE;
    }

    // remove the lock file
    lockfile.Remove();
  }

  // create new lockfile with our PID
  PFile lockfile(lockfilename, PFile::WriteOnly, PFile::Create);
  int pid = getpid();
#ifdef BINARY_LOCK
  lockfile.Write(&pid, sizeof(pid));
#else
  lockfile << pid;
#endif
  lockfile.Close();

  // attempt to open the device
  PString device_name = PString(DEV_PREFIX) + port;
  if ((os_handle = open((const char *)device_name, O_RDWR)) < 0) {
    lastError = AccessDenied;
    Close();
    return FALSE;
  }

  // put the port into non-blocking mode
  {
    long cmd = 1;
    ::ioctl (os_handle, FIONBIO, &cmd);
  }

  // save the channel name
  channelName = port;

  // get the current mode and then set to desired default mode
  Termio = new(termio);
  ::ioctl(os_handle, TCGETA, Termio);

  // set input mode: ignore breaks, ignore parity errors, do not strip chars,
  // no CR/NL conversion, no case conversion, no XON/XOFF control,
  // no start/stop
  Termio->c_iflag = IGNBRK | IGNPAR;

  // set output mode: no post process output, 
  Termio->c_oflag = 0;

  // set control modes: 9600, N, 8, 1, local line
  Termio->c_cflag = B9600 | CS8 | CSTOPB | CREAD | CLOCAL;
  baudRate   = 9600;
  dataBits   = 8;
  parityBits = NoParity;
  stopBits   = 1;

  // set line discipline
  Termio->c_lflag = 0;

  // set the device into the correct mode
  ::ioctl(os_handle, TCSETAW, Termio);

  // now use the inputted parameters
  SetSpeed(speed);
  SetDataBits(data);
  SetParity(parity);
  SetStopBits(stop);
  SetInputFlowControl(inputFlow);
  SetOutputFlowControl(outputFlow);

  return TRUE;
}


void PSerialChannel::CopyContents(const PSerialChannel & chan)
{
  baudRate    = chan.baudRate;
  dataBits    = chan.dataBits;
  parityBits  = chan.parityBits;
  stopBits    = chan.stopBits;
  channelName = chan.channelName;
  os_handle   = chan.os_handle;
  if (chan.Termio == NULL)
    Termio = NULL;
  else {
    Termio  = new(termio);
    *Termio = *chan.Termio;
  }
}


BOOL PSerialChannel::SetSpeed(DWORD speed)
{
  int baud;
  int mask = 0;

#ifdef CBAUD
  mask |= CBAUD;
#endif

  switch(speed) {
#ifdef CBAUD
#ifdef B50
    case 50:
      baud = B50;
      break;
#endif
#ifdef B75
    case 75:
      baud = B75;
      break;
#endif
#ifdef B110
    case 110:
      baud = B110;
      break;
#endif
#ifdef B134
    case 134:
      baud = B134;
      break;
#endif
#ifdef B150
    case 150:
      baud = B150;
      break;
#endif
#ifdef B200
    case 200:
      baud = B200;
      break;
#endif
#ifdef B300
    case 300:
      baud = B300;
      break;
#endif
#ifdef B600
    case 600:
      baud = B600;
      break;
#endif
#ifdef B1200
    case 1200:
      baud = B1200;
      break;
#endif
#ifdef B1800
    case 1800:
      baud = B1800;
      break;
#endif
#ifdef B2400
    case 2400:
      baud = B2400;
      break;
#endif
#ifdef B4800
    case 4800:
      baud = B4800;
      break;
#endif
#ifdef B9600
    case 9600:
      baud = B9600;
      break;
#endif
#ifdef B19200
    case 19200:
      baud = B19200;	
      break;
#endif
#ifdef B38400
    case 38400:
      baud = B38400;	
      break;
#endif
#endif
    default:
      baud = 0;
  };
 
  if (baud == 0) {
    lastError = BadParameter;
    return FALSE;
  }

  // set new baud rate
  baudRate       = speed;
  Termio->c_cflag &= ~mask;
  Termio->c_cflag |= baud;
  ::ioctl(os_handle, TCSETAW, Termio);

  return TRUE;
}


BOOL PSerialChannel::SetDataBits(BYTE data)
{
  int flags;
  int mask = 0;

#ifdef CSIZE
  mask |= CSIZE;
#endif

  switch (data) {
#ifdef CSIZE
#ifdef CS5
    case 5:
      flags = CS5;
      break;
#endif
#ifdef CS6
    case 6:
      flags = CS6;
      break;
#endif
#ifdef CS7
    case 7:
      flags = CS7;
      break;
#endif
#ifdef CS8
    case 8:
      flags = CS8;
      break;
#endif
#endif
    default:
      flags = 0;
      break;
  }

  if (flags == 0) {
    lastError = BadParameter;
    return FALSE;
  }

  // set the new number of data bits
  dataBits       = data;
  Termio->c_cflag &= ~mask;
  Termio->c_cflag |= flags;
  ::ioctl(os_handle, TCSETAW, Termio);

  return TRUE;
}

BOOL PSerialChannel::SetParity(Parity parity)
{
  int flags;
  int mask = 0;

#ifdef PARENB
  mask |= PARENB;
#ifdef  PARODD
  mask |= PARODD;
#endif
#endif

  switch (parity) {
#ifdef PARENB
#ifdef PARODD
    case OddParity:
    case DefaultParity:
      flags = PARODD | PARENB;
      break;
#endif
    case EvenParity:
      flags = PARENB;
#endif
    case NoParity:
      flags = 0;
      break;

    case MarkParity:
    case SpaceParity:
    default:
      flags = 0;
  }

  if (flags == 0) {
    lastError = BadParameter;
    return FALSE;
  }

  // set the new parity
  parityBits = parity;
  Termio->c_cflag &= ~mask;
  Termio->c_cflag |= flags;
  ::ioctl(os_handle, TCSETAW, Termio);

  return TRUE;
}

BOOL PSerialChannel::SetStopBits(BYTE stop)
{
  int flags;
  int mask = 0;

#ifdef CSTOPB
  mask |= CSTOPB;
#endif

  switch (stop) {
#ifdef CSTOPB
    case 1:
      flags = 0;
      break;
    case 2:
      flags = CSTOPB;
      break;
#endif
    default:
      flags = 0;
  }

  if (flags == 0) {
    lastError = BadParameter;
    return FALSE;
  }

  // set the new number of stop bits
  dataBits = stop;
  Termio->c_cflag &= ~mask;
  Termio->c_cflag |= flags;
  ::ioctl(os_handle, TCSETAW, Termio);

  return TRUE;
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl flowControl)
{
  return TRUE;
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  return NoFlowControl;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl flowControl)
{
  return TRUE;
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  return NoFlowControl;
}


void PSerialChannel::SetDTR(BOOL state)
{
  return;
}


void PSerialChannel::SetRTS(BOOL state)
{
  return;
}


void PSerialChannel::SetBreak(BOOL state)
{
  return;
}


BOOL PSerialChannel::GetCTS()
{
  return FALSE;
}


BOOL PSerialChannel::GetDSR()
{
  return FALSE;
}


BOOL PSerialChannel::GetDCD()
{
  return FALSE;
}


BOOL PSerialChannel::GetRing()
{
  return FALSE;
}


PStringList PSerialChannel::GetPortNames()
{
  PStringList ports;

  for (int i = 0; i < PORT_COUNT; i++) 
    ports.AppendString(PString(PORT_PREFIX) + (char)('0' + PORT_START + i));

  return ports;
}

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
  pid_t pid;

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


void PPipeChannel::DestroyContents()
{
  Close();
  PChannel::DestroyContents();
}

void PPipeChannel::CopyContents(const PPipeChannel & c)
{
  toChildPipe[0]   = c.toChildPipe[0];
  toChildPipe[1]   = c.toChildPipe[1];
  fromChildPipe[0] = c.fromChildPipe[0];
  fromChildPipe[1] = c.fromChildPipe[1];
  childPid         = c.childPid;
}

void PPipeChannel::CloneContents(const PPipeChannel * c)
{
  PAssertAlways("Cannot clone PPipeChannel");
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


////////////////////////////////////////////////////////////////
//
//  PTCPSocket
//
BOOL PTCPSocket::Read(void * buf, PINDEX len)

{
  SetIOBlock(TRUE);

  // attempt to read out of band data
  BYTE buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);

  // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount = ::read(os_handle, buf, len)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}
