

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
    lastError     = Timeout;
    lastReadCount = 0;
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

////////////////////////////////////////////////////////////////
//
//  PUDPSocket
//

BOOL PUDPSocket::ReadFrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      Address & addr, // Address from which the datagram was received.
      WORD & port)     // Port from which the datagram was received.
{
  if (!SetIOBlock(TRUE)) {
    lastError     = Timeout;
    lastReadCount = 0;
    return FALSE;
  }

  // attempt to read data
  struct sockaddr_in rec_addr;
  int    addr_len;
  if (ConvertOSError(lastReadCount = ::recvfrom(os_handle, buf, len, 0,
                                       (sockaddr *)&rec_addr, &addr_len))) {
    addr = rec_addr.sin_addr;
    port = rec_addr.sin_port;
    return lastReadCount > 0;
  }

  lastReadCount = 0;
  return FALSE;
}


////////////////////////////////////////////////////////////////
//
//  PUDPSocket
//

virtual BOOL PUDPSocket::WriteTo(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      const Address & addr, // Address to which the datagram is sent.
      WORD port)          // Port to which the datagram is sent.
{
  if (!SetIOBlock(FALSE)) {
    lastError     = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

  // attempt to read data
  struct sockaddr_in rec_addr;
  int    addr_len;

  rec_addr.sin_addr = addr;
  rec_addr.sin_port = port;

  if (ConvertOSError(lastWriteCount = ::sendto(os_handle, buf, len, 0,
                                       (sockaddr *)&rec_addr, &addr_len))) {
    return lastWriteCount > 0;
  }

  lastWriteCount = 0;
  return FALSE;
}



