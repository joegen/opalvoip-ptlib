/*
 * $Id: winsock.cxx,v 1.6 1995/12/10 12:06:00 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: winsock.cxx,v $
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
  BOOL ret = ConvertOSError(closesocket(os_handle));
  os_handle = -1;
  return ret;
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
