/*
 * $Id: winsock.cxx,v 1.8 1996/01/23 13:25:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: winsock.cxx,v $
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


BOOL PSocket::_WaitForData(BOOL reading)
{
  flush();

  PTimeInterval & timeout = reading ? readTimeout : writeTimeout;

  u_long state = timeout == PMaxTimeInterval ? 0 : 1;
  if (!ConvertOSError(::ioctlsocket(os_handle, FIONBIO, &state)))
    return FALSE;

  if (timeout == PMaxTimeInterval)
    return TRUE;

  fd_set fds;
  FD_ZERO(&fds);
#pragma warning(disable:4127)
  FD_SET(os_handle, &fds);
#pragma warning(default:4127)
  struct timeval tval;
  tval.tv_sec = timeout.GetSeconds();
  tval.tv_usec = (timeout.GetMilliseconds()%1000)*1000;
  int selectResult = ::select(os_handle+1,
                              reading ? &fds : NULL,
                              reading ? NULL : &fds,
                              NULL,
                              &tval);
  return ConvertOSError(selectResult);
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  flush();
  lastReadCount = 0;

  if (!_WaitForData(TRUE))
    return FALSE;

  int recvResult = ::recv(os_handle, (char *)buf, len, 0);
  if (!ConvertOSError(recvResult))
    return FALSE;

  lastReadCount = recvResult;
  return lastReadCount > 0;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  flush();
  lastWriteCount = 0;

  if (!_WaitForData(FALSE))
    return FALSE;

  int sendResult = ::send(os_handle, (const char *)buf, len, 0);
  if (!ConvertOSError(sendResult))
    return FALSE;

  lastWriteCount = sendResult;
  return lastWriteCount >= len;
}


BOOL PSocket::Close()
{
  if (!IsOpen())
    return FALSE;
  return ConvertOSError(_Close());
}


int PSocket::_Close()
{
  int err = closesocket(os_handle);
  os_handle = -1;
  return err;
}


BOOL PSocket::ConvertOSError(int error)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

  osError = WSAGetLastError();
#ifdef _WIN32
  SetLastError(osError);
#endif
  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    case WSAEWOULDBLOCK :
      lastError = Timeout;
      break;
    default :
      lastError = Miscellaneous;
  }
  return FALSE;
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
// PTCPSocket

BOOL PTCPSocket::Accept(PSocket & socket)
{
  // attempt to create a socket
  sockaddr_in address;
  address.sin_family = AF_INET;
  int size = sizeof(address);
  if (!ConvertOSError(os_handle = ::accept(socket.GetHandle(),
                                          (struct sockaddr *)&address, &size)))
    return FALSE;

  port = ntohs(address.sin_port);
  return TRUE;
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

BOOL PUDPSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  lastReadCount = 0;

  if (!_WaitForData(TRUE))
    return FALSE;

  sockaddr_in sockAddr;
  int addrLen = sizeof(sockAddr);
  int recvResult = recvfrom(os_handle,
                  (char *)buf, len, 0, (struct sockaddr *)&sockAddr, &addrLen);
  if (!ConvertOSError(recvResult))
    return FALSE;

  addr = sockAddr.sin_addr;
  port = ntohs(sockAddr.sin_port);

  lastReadCount = recvResult;
  return lastReadCount > 0;
}


BOOL PUDPSocket::WriteTo(const void * buf, PINDEX len,
                                               const Address & addr, WORD port)
{
  lastWriteCount = 0;

  if (!_WaitForData(FALSE))
    return FALSE;

  sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_addr = addr;
  sockAddr.sin_port = htons(port);
  int sendResult = ::sendto(os_handle, (const char *)buf, len, 0,
                               (struct sockaddr *)&sockAddr, sizeof(sockAddr));
  if (!ConvertOSError(sendResult))
    return FALSE;

  lastWriteCount = sendResult;
  return lastWriteCount >= len;
}


// End Of File ///////////////////////////////////////////////////////////////
