

#pragma implementation "socket.h"
#pragma implementation "ipsock.h"
#pragma implementation "udpsock.h"
#pragma implementation "tcpsock.h"

#include <ptlib.h>
#include <sockets.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netinet/in.h>

#if defined(P_SUN4)
#include <errno.h>
#include <sys/socket.h>

extern "C" int socket(int, int, int);
extern "C" int connect(int, struct sockaddr *, int);
extern "C" int accept(int, struct sockaddr *, int *);
extern "C" int sendto(int, const void *, int, int, const struct sockaddr *, int);
extern "C" int recvfrom(int, void *, int, int, struct sockaddr *, int *);
extern "C" int recv(int, void *, int, int);
extern "C" int shutdown(int, int);
extern "C" int ioctl(int, int, void *);
extern "C" void bzero(void *, int);
#endif

PSocket::~PSocket()
{
  os_close();
}

int PSocket::os_close()
{
  int status;
  if (os_handle >= 0) {
    PProcess::Current()->PXAbortIOBlock(os_handle);
    ::shutdown(os_handle, 2);
    DWORD cmd = 0;
    ::ioctl(os_handle, FIONBIO, &cmd);
    status = ::close(os_handle);
  } else 
    status = 0;

  os_handle = -1;
  return status;
}

int PSocket::os_socket(int af, int type, int protocol)
{
  // attempt to create a socket
  int handle;
  if ((handle = ::socket(af, type, protocol)) >= 0) {

    // make the socket non-blocking
    DWORD cmd = 1;
    if (!ConvertOSError(::ioctl(handle, FIONBIO, &cmd)) ||
        !ConvertOSError(::fcntl(handle, F_SETFD, 1))) {
      ::close(handle);
      return -1;
    }
  }
  return handle;
}

int PSocket::os_connect(struct sockaddr * addr, int size)
{
  int val = ::connect(os_handle, addr, size);

  if (val == 0)
    return 0;

  if (errno != EINPROGRESS)
    return -1;

  // because we have to know whether the except or write bit gets set, we
  // have to a full-blown select
  fd_set rfd, wfd, efd;
  FD_ZERO(&rfd); FD_ZERO(&wfd); FD_ZERO(&efd);
  FD_SET(os_handle, &wfd);
  FD_SET(os_handle, &efd);
  PIntArray osHandles(2);
  osHandles[0] = os_handle;
  osHandles[1] = 2+4;

  val = PThread::Current()->PXBlockOnIO(os_handle+1,
                                            rfd, wfd, efd,
                                            PMaxTimeInterval,
	 		                    osHandles);
  if (val < 0)
    return -1;
  if (val == 0 || FD_ISSET(os_handle, &efd)) {
    errno = ECONNREFUSED;
    return -1;
  }
  return 0;
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size)
{
  if (!PXSetIOBlock(PXAcceptBlock, sock)) {
    errno = EINTR;
    return -1;
  }

  int new_fd = ::accept(sock, addr, size);
  return new_fd;
}

int PSocket::os_select(int maxHandle,
                   fd_set & readBits,
                   fd_set & writeBits,
                   fd_set & exceptionBits,
          const PIntArray & osHandles,
      const PTimeInterval & timeout)
{
  return PThread::Current()->PXBlockOnIO(maxHandle,
                                         readBits,
                                         writeBits,
                                         exceptionBits,
                                         timeout,
					 osHandles);
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
BOOL PTCPSocket::Read(void * buf, PINDEX maxLen)

{
  lastReadCount = 0;

  // wait until select indicates there is data to read, or until
  // a timeout occurs
  if (!PXSetIOBlock(PXReadBlock, readTimeout)) {
    lastError     = Timeout;
    return FALSE;
  }

  // attempt to read out of band data
  BYTE buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);

    // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount = ::read(os_handle, buf, maxLen)))
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
  if (!PXSetIOBlock(PXReadBlock, readTimeout)) {
    lastError     = Timeout;
    lastReadCount = 0;
    return FALSE;
  }

  // attempt to read data
  struct sockaddr_in rec_addr;
  int    addr_len = sizeof(rec_addr);

  if (ConvertOSError(lastReadCount = ::recvfrom(os_handle, buf, len, 0,
                                       (sockaddr *)&rec_addr, &addr_len))) {
    addr = rec_addr.sin_addr;
    port = ntohs(rec_addr.sin_port);
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
  if (!PXSetIOBlock(PXWriteBlock, writeTimeout)) {
    lastError     = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

  // attempt to read data
  struct sockaddr_in rec_addr;
  int    addr_len = sizeof(rec_addr);
  memset(&rec_addr, 0, addr_len);

  rec_addr.sin_family = AF_INET;
  rec_addr.sin_addr   = addr;
  rec_addr.sin_port   = htons(port);

  if (ConvertOSError(lastWriteCount = ::sendto(os_handle, buf, len, 0,
                                       (sockaddr *)&rec_addr, addr_len))) {
    return lastWriteCount > 0;
  }

  lastWriteCount = 0;
  return FALSE;
}
