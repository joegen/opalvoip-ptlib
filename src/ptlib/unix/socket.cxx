

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

PSocket::~PSocket()
{
  _Close();
}

int PSocket::_Close()
{
  int status;
  if (os_handle >= 0)
    status = close(os_handle);
  else 
    status = 0;
  os_handle = -1;
  return status;
}

PIPSocket::Address::operator DWORD() const
{
  return PSocket::Net2Host((DWORD)s_addr);
}

BYTE PIPSocket::Address::Byte1() const
{
  return *(((BYTE *)&s_addr)+0);
}

BYTE PIPSocket::Address::Byte2() const
{
  return *(((BYTE *)&s_addr)+1);
}

BYTE PIPSocket::Address::Byte3() const
{
  return *(((BYTE *)&s_addr)+2);
}

BYTE PIPSocket::Address::Byte4() const
{
  return *(((BYTE *)&s_addr)+3);
}

PIPSocket::Address::Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
  BYTE * p = (BYTE *)&s_addr;
  p[0] = b1;
  p[1] = b2;
  p[2] = b3;
  p[3] = b4;
}

////////////////////////////////////////////////////////////////
//
//  PTCPSocket
//
BOOL PTCPSocket::Read(void * buf, PINDEX len)

{
  if (!PXSetIOBlock(PXReadBlock)) {
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
  if (!PXSetIOBlock(PXReadBlock)) {
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
  if (!PXSetIOBlock(PXWriteBlock)) {
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
                                       (sockaddr *)&rec_addr, addr_len))) {
    return lastWriteCount > 0;
  }

  lastWriteCount = 0;
  return FALSE;
}


BOOL PTCPSocket::Accept(PSocket & socket)
{
  // ensure the socket we are accpeting on is open
  PAssert(socket.GetHandle() >= 0, "Accept on closed socket");

  // attempt to create a socket
  sockaddr_in address;
  address.sin_family = AF_INET;
  int size = sizeof(address);

  // set up a blocked I/O call
  if (!PXSetIOBlock(PXAcceptBlock, socket.GetHandle())) {
    lastError     = Timeout;
    return FALSE;
  }

  if (!ConvertOSError(os_handle = ::accept(socket.GetHandle(),
                                          (struct sockaddr *)&address, &size))) 
    return FALSE;

  port = ::ntohs(address.sin_port);
  return TRUE;
}
