

#include <ptlib.h>
#include <sockets.h>

#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>

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

  // wait for the connect to occur, or not
  val = PThread::Current()->PXBlockOnIO(os_handle, PXConnectBlock, readTimeout);

  // check the response
  if (val < 0)
    return -1;
  else if (val == 0) {
    errno = ECONNREFUSED;
    return -1;
  }
  return 0;
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size,
                       const PTimeInterval & timeout)
{
  if (!PXSetIOBlock(PXAcceptBlock, sock, timeout)) {
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
  return (DWORD)s_addr;
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

BOOL PSocket::os_recvfrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      int    flags,
      sockaddr * addr, // Address from which the datagram was received.
      int * addrlen)
{
  if (!PXSetIOBlock(PXReadBlock, readTimeout)) {
    lastError     = Timeout;
    lastReadCount = 0;
    return FALSE;
  }

  // attempt to read data
  return ::recvfrom(os_handle, buf, len, flags, (sockaddr *)addr, addrlen);
}


////////////////////////////////////////////////////////////////
//
//  PUDPSocket
//

BOOL PSocket::os_sendto(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      int flags,
      sockaddr * addr, // Address to which the datagram is sent.
      int addrlen)  
{
  if (!PXSetIOBlock(PXWriteBlock, writeTimeout)) {
    lastError     = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

  // attempt to read data
  return ::sendto(os_handle, buf, len, flags, (sockaddr *)addr, addrlen);
}

BOOL PSocket::Read(void * buf, PINDEX len)
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  if (!PXSetIOBlock(PXReadBlock, readTimeout)) 
    return FALSE;

  if (ConvertOSError(lastReadCount = ::recv(os_handle, buf, len, 0)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}
