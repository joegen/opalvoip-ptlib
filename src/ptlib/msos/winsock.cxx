/*
 * $Id: winsock.cxx,v 1.15 1996/03/31 09:11:06 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: winsock.cxx,v $
 * Revision 1.15  1996/03/31 09:11:06  robertj
 * Fixed major performance problem in timeout read/write to sockets.
 *
 * Revision 1.14  1996/03/10 13:16:25  robertj
 * Fixed ioctl of closed socket.
 *
 * Revision 1.13  1996/03/04 12:41:02  robertj
 * Fixed bug in leaving socket in non-blocking mode.
 * Changed _Close to os_close to be consistent.
 *
 * Revision 1.12  1996/02/25 11:23:40  robertj
 * Fixed bug in Read for when a timeout occurs on select, not returning error code.
 *
 * Revision 1.11  1996/02/25 03:13:12  robertj
 * Moved some socket functions to platform dependent code.
 *
 * Revision 1.10  1996/02/19 13:52:39  robertj
 * Added SO_LINGER option to socket to stop data loss on close.
 * Fixed error reporting for winsock classes.
 *
 * Revision 1.9  1996/02/15 14:53:36  robertj
 * Added Select() function to PSocket.
 *
 * Revision 1.8  1996/01/23 13:25:48  robertj
 * Moved Accept from platform independent code.
 *
 * Revision 1.7  1996/01/02 12:57:17  robertj
 * Unix compatibility.
 *
 * Revision 1.6  1995/12/10 12:06:00  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.5  1995/06/17 00:59:49  robertj
 * Fixed bug with stream being flushed on read/write.
 *
 * Revision 1.4  1995/06/04 12:49:51  robertj
 * Fixed bugs in socket read and write function return status.
 * Fixed bug in socket close setting object state to "closed".
 *
 * Revision 1.3  1995/03/12 05:00:10  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.2  1995/01/03  09:43:27  robertj
 * Moved out of band stuff to common.
 *
 * Revision 1.1  1994/10/30  12:06:56  robertj
 * Initial revision
 */

#include <ptlib.h>
#include <sockets.h>


//////////////////////////////////////////////////////////////////////////////
// PSocket

BOOL PSocket::WinSockStarted = FALSE;

static void SocketCleanup()
{
  WSACleanup();
}


PSocket::PSocket()
{
  if (!WinSockStarted) {
    WSADATA winsock;
    PAssert(WSAStartup(0x101, &winsock) == 0, POperatingSystemError);
    PAssert(LOBYTE(winsock.wVersion) == 1 &&
            HIBYTE(winsock.wVersion) == 1, POperatingSystemError);
    atexit(SocketCleanup);
    WinSockStarted = TRUE;
  }
}


PSocket::~PSocket()
{
  Close();
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  flush();
  lastReadCount = 0;

  int timeout = readTimeout.GetMilliseconds();
  if (timeout == 0)
    timeout = 1;
  else if (timeout == -1)
    timeout = 0;
  if (!SetOption(SO_RCVTIMEO, timeout))
    return FALSE;

  int recvResult = ::recv(os_handle, (char *)buf, len, 0);
  if (ConvertOSError(recvResult))
    lastReadCount = recvResult;

  return lastReadCount > 0;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  flush();
  lastWriteCount = 0;

  int timeout = writeTimeout.GetMilliseconds();
  if (timeout == 0)
    timeout = 1;
  else if (timeout == -1)
    timeout = 0;
  if (!SetOption(SO_SNDTIMEO, timeout))
    return FALSE;

  int sendResult = ::send(os_handle, (const char *)buf, len, 0);
  if (ConvertOSError(sendResult))
    lastWriteCount = sendResult;

  return lastWriteCount >= len;
}


BOOL PSocket::Close()
{
  if (!IsOpen())
    return FALSE;
  return ConvertOSError(os_close());
}


int PSocket::os_close()
{
  int err = closesocket(os_handle);
  os_handle = -1;
  return err;
}


int PSocket::os_socket(int af, int type, int protocol)
{
  return ::socket(af, type, protocol);
}


int PSocket::os_connect(struct sockaddr * addr, int size)
{
  return ::connect(os_handle, addr, size);
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size)
{
  return ::accept(sock, addr, size);
}

int PSocket::os_select(int maxfds,
                       fd_set & readfds,
                       fd_set & writefds,
                       fd_set & exceptfds,
                       const PIntArray &,
                       const PTimeInterval & timeout)
{
  struct timeval tv_buf;
  struct timeval * tv = NULL;
  if (timeout != PMaxTimeInterval) {
    tv = &tv_buf;
    tv->tv_usec = timeout.GetMilliseconds()%1000*1000;
    tv->tv_sec = timeout.GetSeconds();
  }
  return select(maxfds, &readfds, &writefds, &exceptfds, tv);
}


BOOL PSocket::ConvertOSError(int error)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

#ifdef _WIN32
  SetLastError(WSAGetLastError());
  return PChannel::ConvertOSError(-2);
#else
  osError = WSAGetLastError();
  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    case WSAEWOULDBLOCK :
      lastError = Timeout;
      break;
    default :
      osError |= 0x40000000;
      lastError = Miscellaneous;
  }
  return FALSE;
#endif
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

PIPSocket::Address::Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
  S_un.S_un_b.s_b1 = b1;
  S_un.S_un_b.s_b2 = b2;
  S_un.S_un_b.s_b3 = b3;
  S_un.S_un_b.s_b4 = b4;
}


PIPSocket::Address::operator DWORD() const
{
  return PSocket::Net2Host(S_un.S_addr);
}


BYTE PIPSocket::Address::Byte1() const
{
  return S_un.S_un_b.s_b1;
}


BYTE PIPSocket::Address::Byte2() const
{
  return S_un.S_un_b.s_b2;
}


BYTE PIPSocket::Address::Byte3() const
{
  return S_un.S_un_b.s_b3;
}


BYTE PIPSocket::Address::Byte4() const
{
  return S_un.S_un_b.s_b4;
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

BOOL PUDPSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  lastReadCount = 0;

  int timeout = readTimeout.GetMilliseconds();
  if (timeout == 0)
    timeout = 1;
  else if (timeout == -1)
    timeout = 0;
  if (!SetOption(SO_RCVTIMEO, timeout))
    return FALSE;

  sockaddr_in sockAddr;
  int addrLen = sizeof(sockAddr);
  int recvResult = recvfrom(os_handle,
                  (char *)buf, len, 0, (struct sockaddr *)&sockAddr, &addrLen);
  if (ConvertOSError(recvResult)) {
    addr = sockAddr.sin_addr;
    port = ntohs(sockAddr.sin_port);

    lastReadCount = recvResult;
  }

  return lastReadCount > 0;
}


BOOL PUDPSocket::WriteTo(const void * buf, PINDEX len,
                                               const Address & addr, WORD port)
{
  lastWriteCount = 0;

  int timeout = writeTimeout.GetMilliseconds();
  if (timeout == 0)
    timeout = 1;
  else if (timeout == -1)
    timeout = 0;
  if (!SetOption(SO_SNDTIMEO, timeout))
    return FALSE;

  sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_addr = addr;
  sockAddr.sin_port = htons(port);
  int sendResult = ::sendto(os_handle, (const char *)buf, len, 0,
                               (struct sockaddr *)&sockAddr, sizeof(sockAddr));
  if (ConvertOSError(sendResult))
    lastWriteCount = sendResult;

  return lastWriteCount >= len;
}


// End Of File ///////////////////////////////////////////////////////////////
