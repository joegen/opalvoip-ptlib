

#pragma implementation "serchan.h"
#pragma implementation "modem.h"

#include <ptlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#if 0
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <termio.h>
#include <signal.h>
#endif

#if defined(P_SUN4)
#include <sys/termio.h>
extern "C" int ioctl(int, int, void *);
#endif

//#define BINARY_LOCK	1
#define	LOCK_PREFIX	"/var/spool/uucp/LCK.."
#define	DEV_PREFIX	"/dev/"

#define	PORTLISTENV	"PWLIB_SERIALPORTS"
#define	DEV_PREFIX	"/dev/"

#include "../../common/src/serial.cxx"

////////////////////////////////////////////////////////////////
//
//  PSerialChannel
//

void PSerialChannel::Construct()
{
  // set control modes: 9600, N, 8, 1, local line
  baudRate   = 9600;
  dataBits   = 8;
  parityBits = NoParity;
  stopBits   = 1;
  Termio.c_cflag = B9600 | CS8 | CSTOPB | CREAD | CLOCAL;

  // set input mode: ignore breaks, ignore parity errors, do not strip chars,
  // no CR/NL conversion, no case conversion, no XON/XOFF control,
  // no start/stop
  Termio.c_iflag = IGNBRK | IGNPAR;

  // set output mode: no post process output, 
  Termio.c_oflag = 0;

  // set line discipline
  Termio.c_lflag = 0;
}

BOOL PSerialChannel::Close()
{
  if (os_handle >= 0) {

    // delete the lockfile
    PFile::Remove(PString(LOCK_PREFIX) + channelName);

    // restore the original terminal settings
    ::ioctl(os_handle, TCSETAW, &oldTermio);
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

//  // check prefix of name
//  if (port.Left(PORT_PREFIX_LEN) != PORT_PREFIX) {
//    lastError = BadParameter;
//    return FALSE;
//  }

//  // check suffix
//  int portnum = (port.Right(port.GetLength()-PORT_PREFIX_LEN)).AsInteger();
//  if ((portnum < PORT_START) || (portnum >= (PORT_START + PORT_COUNT))) {
//    lastError = BadParameter;
//    return FALSE;
//  }

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
  if ((os_handle = ::open((const char *)device_name, O_RDWR|O_NONBLOCK)) < 0) {
    lastError = AccessDenied;
    Close();
    return FALSE;
  }

  // save the current port setup
  ::ioctl(os_handle, TCGETA, &oldTermio);

  // save the channel name
  channelName = port;

  // now set the mode that was passed in
  SetSpeed(speed);
  SetDataBits(data);
  SetParity(parity);
  SetStopBits(stop);
  SetInputFlowControl(inputFlow);
  SetOutputFlowControl(outputFlow);

  ::fcntl(os_handle, F_SETFD, 1);

  return TRUE;
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
  Termio.c_cflag &= ~mask;
  Termio.c_cflag |= baud;

  if (os_handle >= 0)
    ::ioctl(os_handle, TCSETAW, &Termio);

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
  Termio.c_cflag &= ~mask;
  Termio.c_cflag |= flags;

  if (os_handle >= 0)
    ::ioctl(os_handle, TCSETAW, &Termio);

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
  Termio.c_cflag &= ~mask;
  Termio.c_cflag |= flags;

  if (os_handle >= 0)
    ::ioctl(os_handle, TCSETAW, &Termio);

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
  Termio.c_cflag &= ~mask;
  Termio.c_cflag |= flags;

  if (os_handle >= 0)
    ::ioctl(os_handle, TCSETAW, &Termio);

  return TRUE;
}


BOOL PSerialChannel::SetInputFlowControl(FlowControl)
{
  return TRUE;
}


PSerialChannel::FlowControl PSerialChannel::GetInputFlowControl() const
{
  return NoFlowControl;
}


BOOL PSerialChannel::SetOutputFlowControl(FlowControl)
{
  return TRUE;
}


PSerialChannel::FlowControl PSerialChannel::GetOutputFlowControl() const
{
  return NoFlowControl;
}


void PSerialChannel::SetDTR(BOOL)
{
}


void PSerialChannel::SetRTS(BOOL)
{
}


void PSerialChannel::SetBreak(BOOL)
{
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

  char * env = getenv(PORTLISTENV);
  if (env != NULL) {
    PString str(env);
    PStringArray tokens = str.Tokenise(" ,\t", FALSE);
    PINDEX i;
    for (i = 0; i < tokens.GetSize(); i++) 
      ports.AppendString(tokens[i]);
  } else {
    ports.AppendString(PString("ttyS0"));
    ports.AppendString(PString("ttyS1"));
    ports.AppendString(PString("ttyS2"));
    ports.AppendString(PString("ttyS3"));
  }

  return ports;
}
