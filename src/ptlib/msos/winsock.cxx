/*
 * $Id: winsock.cxx,v 1.24 1996/09/14 13:09:47 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: winsock.cxx,v $
 * Revision 1.24  1996/09/14 13:09:47  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.23  1996/08/08 10:06:07  robertj
 * Fixed incorrect value in write, causes incorrect output if send is split.
 *
 * Revision 1.22  1996/07/27 04:03:29  robertj
 * Created static version of ConvertOSError().
 *
 * Revision 1.21  1996/06/01 04:19:34  robertj
 * Added flush to PSocket destructor as needs to use Write() at that level.
 *
 * Revision 1.20  1996/05/15 10:23:08  robertj
 * Changed millisecond access functions to get 64 bit integer.
 * Added timeout to accept function.
 * Added ICMP protocol socket, getting common ancestor to UDP.
 *
 * Revision 1.19  1996/04/29 12:22:26  robertj
 * Fixed detection of infinite timeout.
 *
 * Revision 1.18  1996/04/17 12:09:52  robertj
 * Fixed bug in detecting infinte timeout.
 *
 * Revision 1.17  1996/04/12 09:45:06  robertj
 * Rewrite of PSocket::Read() to avoid "Connection Reset" errors caused by SO_RCVTIMEO
 *
 * Revision 1.17  1996/04/10 12:15:11  robertj
 * Rewrite of PSocket::Read() to avoid "Connection Reset" errors caused by SO_RCVTIMEO.
 *
 * Revision 1.16  1996/04/05 01:42:28  robertj
 * Assured PSocket::Write always writes the number of bytes specified.
 *
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
// PWinSock

PWinSock::PWinSock()
{
  WSADATA winsock;
  PAssert(WSAStartup(0x101, &winsock) == 0, POperatingSystemError);
  PAssert(LOBYTE(winsock.wVersion) == 1 &&
          HIBYTE(winsock.wVersion) == 1, POperatingSystemError);
}


PWinSock::~PWinSock()
{
  WSACleanup();
}


BOOL PWinSock::OpenSocket()
{
  return FALSE;
}


const char * PWinSock::GetProtocolName() const
{
  return NULL;
}


//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::~PSocket()
{
  flush();
  Close();
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  flush();
  lastReadCount = 0;

  if (len == 0) {
    lastError = BadParameter;
    osError = EINVAL;
    return FALSE;
  }

  if (readTimeout != PMaxTimeInterval) {
    DWORD available;
    if (!ConvertOSError(ioctlsocket(os_handle, FIONREAD, &available)))
      return FALSE;

    if (available == 0) {
      fd_set readfds;
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
      FD_ZERO(&readfds);
      FD_SET(os_handle, &readfds);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
      struct timeval tv;
      tv.tv_usec = (long)(readTimeout.GetMilliSeconds()%1000)*1000;
      tv.tv_sec = readTimeout.GetSeconds();
      int selval = select(0, &readfds, NULL, NULL, &tv);
      if (!ConvertOSError(selval))
        return FALSE;

      if (selval == 0) {
        lastError = Timeout;
        osError = EAGAIN;
        return FALSE;
      }

      if (!ConvertOSError(ioctlsocket(os_handle, FIONREAD, &available)))
        return FALSE;
    }

    if (available > 0 && len > (PINDEX)available)
      len = available;
  }

  int recvResult = ::recv(os_handle, (char *)buf, len, 0);
  if (ConvertOSError(recvResult))
    lastReadCount = recvResult;

  return lastReadCount > 0;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  flush();
  lastWriteCount = 0;

  int timeout;
  if (writeTimeout == PMaxTimeInterval)
    timeout = 0;
  else if (writeTimeout == 0)
    timeout = 1;
  else
    timeout = writeTimeout.GetInterval();
  if (!SetOption(SO_SNDTIMEO, timeout))
    return FALSE;

  while (len > 0) {
    int sendResult = ::send(os_handle, ((char *)buf)+lastWriteCount, len, 0);
    if (!ConvertOSError(sendResult))
      return FALSE;
    lastWriteCount += sendResult;
    len -= sendResult;
  }

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


int PSocket::os_socket(int af, int type, int proto)
{
  return ::socket(af, type, proto);
}


int PSocket::os_connect(struct sockaddr * addr, int size)
{
  return ::connect(os_handle, addr, size);
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size,
                       const PTimeInterval & timeout)
{
  if (timeout != PMaxTimeInterval) {
    struct timeval tv;
    tv.tv_usec = (long)(timeout.GetMilliSeconds()%1000)*1000;
    tv.tv_sec = timeout.GetSeconds();
    fd_set readfds;
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
    switch (select(0, &readfds, NULL, NULL, &tv)) {
      case 1 :
        break;
      case 0 :
        SetLastError(WSAETIMEDOUT);
        // Then return -1
      default :
        return -1;
    }
  }
  return ::accept(sock, addr, size);
}

BOOL PSocket::os_recvfrom(void * buf,
                          PINDEX len,
                          int flags,
                          struct sockaddr * from,
                          int * fromlen)
{
  int timeout;
  if (readTimeout == PMaxTimeInterval)
    timeout = 0;
  else if (readTimeout == 0)
    timeout = 1;
  else
    timeout = readTimeout.GetInterval();
  if (!SetOption(SO_RCVTIMEO, timeout))
    return FALSE;

  return ::recvfrom(os_handle, (char *)buf, len, flags, from, fromlen);
}


BOOL PSocket::os_sendto(const void * buf,
                        PINDEX len,
                        int flags,
                        struct sockaddr * to,
                        int tolen)
{
  int timeout;
  if (writeTimeout == PMaxTimeInterval)
    timeout = 0;
  else if (writeTimeout == 0)
    timeout = 1;
  else
    timeout = writeTimeout.GetInterval();
  if (!SetOption(SO_SNDTIMEO, timeout))
    return FALSE;

  return ::sendto(os_handle, (const char *)buf, len, flags, to, tolen);
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
    tv->tv_usec = (long)(timeout.GetMilliSeconds()%1000)*1000;
    tv->tv_sec = timeout.GetSeconds();
  }
  return select(maxfds, &readfds, &writefds, &exceptfds, tv);
}


BOOL PSocket::ConvertOSError(int error)
{
  return ConvertOSError(error, lastError, osError);
}


BOOL PSocket::ConvertOSError(int error, Errors & lastError, int & osError)
{
  if (error >= 0) {
    lastError = NoError;
    osError = 0;
    return TRUE;
  }

#ifdef _WIN32
  SetLastError(WSAGetLastError());
  return PChannel::ConvertOSError(-2, lastError, osError);
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
// PIPXSocket

PIPXSocket::Address::Address()
{
  memset(this, 0, sizeof(*this));
}


PIPXSocket::Address::Address(const Address & addr)
{
  memcpy(this, &addr, sizeof(*this));
}


PIPXSocket::Address::Address(const PString & str)
{
  memset(this, 0, sizeof(*this));

  PINDEX colon = str.Find(':');
  if (colon == P_MAX_INDEX)
    colon = 0;
  else
    network.dw = str.Left(colon++).AsInteger();

  PINDEX byte = 11;
  PINDEX pos = str.GetLength();
  do {
    pos--;
    int c = str[pos];
    if (isdigit(c))
      node[byte/2] = (BYTE)((node[byte/2] << 4) + c - '0');
    else if (isxdigit(c))
      node[byte/2] = (BYTE)((node[byte/2] << 4) + toupper(c) - 'A' + 10);
    byte--;
  } while (pos > colon);
}


PIPXSocket::Address::Address(DWORD netNum, const char * nodeNum)
{
  network.dw = netNum;
  memcpy(node, nodeNum, sizeof(node));
}


PIPXSocket::Address & PIPXSocket::Address::operator=(const Address & addr)
{
  memcpy(this, &addr, sizeof(*this));
  return *this;
}


PIPXSocket::Address::operator PString() const
{
  return psprintf("%lu:%02X%02X%02X%02X%02X%02X",
                  network.dw,
                  node[0], node[1], node[2], node[3], node[4], node[5]);
}


PIPXSocket::PIPXSocket(WORD newPort)
{
  SetPort(newPort);
}


PString PIPXSocket::GetName() const
{
  Address addr;
  if (((PIPXSocket*)this)->GetPeerAddress(addr))
    return addr;
  else
    return PString();
}


BOOL PIPXSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_IPX, SOCK_DGRAM, NSPROTO_IPX));
}


const char * PIPXSocket::GetProtocolName() const
{
  return "ipx";
}


PString PIPXSocket::GetHostName(const Address & addr)
{
  return addr;
}


BOOL PIPXSocket::GetHostAddress(Address & addr)
{
  return FALSE;
}


BOOL PIPXSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  return FALSE;
}


static void AssignAddress(sockaddr_ipx & sip, const PIPXSocket::Address & addr)
{
  memcpy(sip.sa_netnum, &addr.network, sizeof(sip.sa_netnum));
  memcpy(sip.sa_nodenum, addr.node, sizeof(sip.sa_nodenum));
}


static void AssignAddress(PIPXSocket::Address & addr, const sockaddr_ipx & sip)
{
  memcpy(&addr.network, sip.sa_netnum, sizeof(addr.network));
  memcpy(addr.node, sip.sa_nodenum, sizeof(addr.node));
}


BOOL PIPXSocket::GetLocalAddress(Address & addr)
{
  sockaddr_ipx sip;
  int size = sizeof(sip);
  if (!ConvertOSError(::getsockname(os_handle, (struct sockaddr *)&sip, &size)))
    return FALSE;

  AssignAddress(addr, sip);
  return TRUE;
}


BOOL PIPXSocket::GetLocalAddress(Address & addr, WORD & portNum)
{
  sockaddr_ipx sip;
  int size = sizeof(sip);
  if (!ConvertOSError(::getsockname(os_handle, (struct sockaddr *)&sip, &size)))
    return FALSE;

  AssignAddress(addr, sip);
  portNum = sip.sa_socket;
  return TRUE;
}


BOOL PIPXSocket::GetPeerAddress(Address & addr)
{
  sockaddr_ipx sip;
  int size = sizeof(sip);
  if (!ConvertOSError(::getpeername(os_handle, (struct sockaddr *)&sip, &size)))
    return FALSE;

  AssignAddress(addr, sip);
  return TRUE;
}


BOOL PIPXSocket::GetPeerAddress(Address & addr, WORD & portNum)
{
  sockaddr_ipx sip;
  int size = sizeof(sip);
  if (!ConvertOSError(::getpeername(os_handle, (struct sockaddr *)&sip, &size)))
    return FALSE;

  AssignAddress(addr, sip);
  portNum = sip.sa_socket;
  return TRUE;
}


BOOL PIPXSocket::Connect(const PString & host)
{
  Address addr;
  if (GetHostAddress(host, addr))
    return Connect(addr);
  return FALSE;
}


BOOL PIPXSocket::Connect(const Address & addr)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  // attempt to create a socket
  if (!OpenSocket())
    return FALSE;

  // attempt to lookup the host name
  sockaddr_ipx sip;
  memset(&sip, 0, sizeof(sip));
  sip.sa_family = AF_IPX;
  AssignAddress(sip, addr);
  sip.sa_socket  = port;  // set the port
  if (ConvertOSError(os_connect((struct sockaddr *)&sip, sizeof(sip))))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PIPXSocket::Listen(unsigned, WORD newPort, Reusability reuse)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // close the port if it is already open
  if (!IsOpen()) {
    // attempt to create a socket
    if (!OpenSocket())
      return FALSE;
  }

  // attempt to listen
  if (SetOption(SO_REUSEADDR, reuse == CanReuseAddress ? 1 : 0)) {
    // attempt to listen
    sockaddr_ipx sip;
    memset(&sip, 0, sizeof(sip));
    sip.sa_family = AF_IPX;
    sip.sa_socket = port;       // set the port

    if (ConvertOSError(::bind(os_handle, (struct sockaddr*)&sip, sizeof(sip)))) {
      int size = sizeof(sip);
      if (ConvertOSError(::getsockname(os_handle, (struct sockaddr*)&sip, &size))) {
        port = sip.sa_socket;
        return TRUE;
      }
    }
  }

  os_close();
  return FALSE;
}


BOOL PIPXSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  lastReadCount = 0;

  sockaddr_ipx sip;
  int addrLen = sizeof(sip);
  int recvResult = os_recvfrom(buf, len, 0, (struct sockaddr *)&sip, &addrLen);
  if (ConvertOSError(recvResult)) {
    AssignAddress(addr, sip);
    port = sip.sa_socket;

    lastReadCount = recvResult;
  }

  return lastReadCount > 0;
}


BOOL PIPXSocket::WriteTo(const void * buf, PINDEX len, const Address & addr, WORD port)
{
  lastWriteCount = 0;

  sockaddr_ipx sip;
  sip.sa_family = AF_IPX;
  AssignAddress(sip, addr);
  sip.sa_socket = port;
  int sendResult = os_sendto(buf, len, 0, (struct sockaddr *)&sip, sizeof(sip));
  if (ConvertOSError(sendResult))
    lastWriteCount = sendResult;

  return lastWriteCount >= len;
}


//////////////////////////////////////////////////////////////////////////////
// PSPXSocket

PSPXSocket::PSPXSocket(WORD port)
  : PIPXSocket(port)
{
}


BOOL PSPXSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_IPX, SOCK_STREAM, NSPROTO_SPX));
}


const char * PSPXSocket::GetProtocolName() const
{
  return "spx";
}


BOOL PSPXSocket::Listen(unsigned queueSize, WORD newPort, Reusability reuse)
{
  if (PIPXSocket::Listen(queueSize, newPort, reuse) &&
      ConvertOSError(::listen(os_handle, queueSize)))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PSPXSocket::Accept(PSocket & socket)
{
  PAssert(socket.IsDescendant(PIPXSocket::Class()), "Invalid listener socket");

  sockaddr_ipx sip;
  sip.sa_family = AF_IPX;
  int size = sizeof(sip);
  if (!ConvertOSError(os_handle = os_accept(socket.GetHandle(),
                                          (struct sockaddr *)&sip, &size,
                                           socket.GetReadTimeout())))
    return FALSE;

  port = ((PIPXSocket &)socket).GetPort();
  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
