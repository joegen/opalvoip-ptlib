/*
 * $Id: winsock.cxx,v 1.1 1994/10/30 12:06:56 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: winsock.cxx,v $
 * Revision 1.1  1994/10/30 12:06:56  robertj
 * Initial revision
 *
// Revision 1.1  1994/10/23  05:42:49  robertj
// Initial revision
//
// Revision 1.1  1994/08/22  00:18:02  robertj
// Initial revision
//
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


BOOL PSocket::Read(void * buf, PINDEX len)
{
  lastReadCount = 0;

  u_long state = readTimeout == PMaxTimeInterval ? 0 : 1;
  if (!ConvertOSError(::ioctlsocket(os_handle, FIONBIO, &state)))
    return FALSE;

  if (readTimeout != PMaxTimeInterval) {
    fd_set read_fds;
    FD_ZERO(&read_fds);
#pragma warning(disable:4127)
    FD_SET(os_handle, &read_fds);
#pragma warning(default:4127)
    struct timeval tval;
    tval.tv_sec = readTimeout.GetSeconds();
    tval.tv_usec = (readTimeout.GetMilliseconds()%1000)*1000;
    int selectResult = ::select(os_handle+1, &read_fds, NULL, NULL, &tval);
    if (!ConvertOSError(selectResult) || selectResult == 0)
      return FALSE;
  }

  int recvResult = ::recv(os_handle, (char *)buf, len, 0);
  if (!ConvertOSError(recvResult))
    return FALSE;

  lastReadCount = recvResult;
  return TRUE;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  lastWriteCount = 0;

  u_long state = writeTimeout == PMaxTimeInterval ? 0 : 1;
  if (!ConvertOSError(::ioctlsocket(os_handle, FIONBIO, &state)))
    return FALSE;

  if (writeTimeout != PMaxTimeInterval) {
    fd_set write_fds;
    FD_ZERO(&write_fds);
#pragma warning(disable:4127)
    FD_SET(os_handle, &write_fds);
#pragma warning(default:4127)
    struct timeval tval;
    tval.tv_sec = writeTimeout.GetSeconds();
    tval.tv_usec = (writeTimeout.GetMilliseconds()%1000)*1000;
    int selectResult = ::select(os_handle+1, NULL, &write_fds, NULL, &tval);
    if (!ConvertOSError(selectResult) || selectResult == 0)
      return FALSE;
  }

  int sendResult = ::send(os_handle, (const char *)buf, len, 0);
  if (!ConvertOSError(sendResult))
    return FALSE;

  lastWriteCount = sendResult;
  return TRUE;
}


BOOL PSocket::Close()
{
  if (IsOpen())
    return FALSE;
  return ConvertOSError(closesocket(os_handle));
}


BOOL PSocket::ConvertOSError(int error)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

  osError = WSAGetLastError();
  switch (osError) {
    case 0 :
      lastError = NoError;
      return TRUE;
    default :
      lastError = Miscellaneous;
      osError |= 0x20000000;
  }
  return FALSE;
}


BOOL PTCPSocket::ReadOutOfBand(void *, PINDEX)
{
  return FALSE;
}


BOOL PTCPSocket::WriteOutOfBand(const void *, PINDEX)
{
  return FALSE;
}


// End Of File ///////////////////////////////////////////////////////////////
