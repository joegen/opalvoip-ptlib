

#pragma implementation "socket.h"
#pragma implementation "ipsock.h"
#pragma implementation "udpsock.h"
#pragma implementation "tcpsock.h"
#pragma implementation "telnet.h"
#pragma implementation "appsock.h"

#include <ptlib.h>
#include <sockets.h>

#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termio.h>
#include <signal.h>

////////////////////////////////////////////////////////////////
//
//  PTCPSocket
//
BOOL PTCPSocket::Read(void * buf, PINDEX len)

{
  if (!SetIOBlock(TRUE)) {
    lastError      = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

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
