/*
 * winsock.cxx
 *
 * WINSOCK implementation of Berkley sockets.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: winsock.cxx,v $
 * Revision 1.39  1998/09/24 03:31:02  robertj
 * Added open software license.
 *
 * Revision 1.38  1998/08/28 14:09:45  robertj
 * Fixed bug in Write() that caused endlesss loops, introduced in previous version.
 *
 * Revision 1.37  1998/08/21 05:27:31  robertj
 * Fixed bug where write streams out to non-stream socket.
 *
 * Revision 1.36  1998/08/06 00:55:21  robertj
 * Fixed conversion of text to IPX address, was swapping nibbles.
 *
 * Revision 1.35  1998/05/08 11:52:03  robertj
 * Added workaround for winsock bug where getpeername() doesn't work immediately after connect().
 *
 * Revision 1.34  1998/05/07 05:21:04  robertj
 * Fixed DNS lookup so only works around bug in old Win95 and not OSR2
 *
 * Revision 1.33  1998/01/26 01:00:06  robertj
 * Added timeout to os_connect().
 * Fixed problems with NT version of IsLocalHost().
 *
 * Revision 1.32  1997/12/18 05:05:27  robertj
 * Moved IsLocalHost() to platform dependent code.
 *
 * Revision 1.31  1997/12/11 10:41:55  robertj
 * Added DWORD operator for IP addresses.
 *
 * Revision 1.30  1997/01/03 04:37:11  robertj
 * Fixed '95 problem with send timeouts.
 *
 * Revision 1.29  1996/12/05 11:51:50  craigs
 * Fixed Win95 recvfrom timeout problem
 *
 * Revision 1.28  1996/11/10 21:04:56  robertj
 * Fixed bug in not flushing stream on close of socket.
 *
 * Revision 1.27  1996/10/31 12:39:30  robertj
 * Fixed bug in byte order of port numbers in IPX protocol.
 *
 * Revision 1.26  1996/10/26 01:43:18  robertj
 * Removed translation of IP address to host order DWORD. Is ALWAYS net order.
 *
 * Revision 1.25  1996/10/08 13:03:09  robertj
 * More IPX support.
 *
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

#include <nspapi.h>
#include <svcguid.h>


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

  os_recvfrom((char *)buf, len, 0, NULL, NULL);
  return lastReadCount > 0;
}


BOOL PSocket::Write(const void * buf, PINDEX len)
{
  flush();
  return os_sendto(((char *)buf)+lastWriteCount, len, 0, NULL, 0);
}


BOOL PSocket::Close()
{
  if (!IsOpen())
    return FALSE;
  flush();
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


class fd_set_class : public fd_set {
  public:
    fd_set_class(SOCKET fd)
      {
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
        FD_ZERO(this);
        FD_SET(fd, this);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif
      }
    BOOL IsPresent(int h) const
      { return FD_ISSET(h, this); }
};

class timeval_class : public timeval {
  public:
    timeval_class(const PTimeInterval & time)
      {
        tv_usec = (long)(time.GetMilliSeconds()%1000)*1000;
        tv_sec = time.GetSeconds();
      }
};


int PSocket::os_connect(struct sockaddr * addr, int size)
{
  if (readTimeout == PMaxTimeInterval)
    return ::connect(os_handle, addr, size);

  DWORD fionbio = 1;
  if (::ioctlsocket(os_handle, FIONBIO, &fionbio) == SOCKET_ERROR)
    return SOCKET_ERROR;
  fionbio = 0;

  if (::connect(os_handle, addr, size) != SOCKET_ERROR)
    return ::ioctlsocket(os_handle, FIONBIO, &fionbio);

  DWORD err = GetLastError();
  if (err != WSAEWOULDBLOCK) {
    ::ioctlsocket(os_handle, FIONBIO, &fionbio);
    SetLastError(err);
    return SOCKET_ERROR;
  }

  fd_set_class writefds = os_handle;
  timeval_class tv = readTimeout;
  switch (select(0, NULL, &writefds, NULL, &tv)) {
    case 1 :
      err = 0;
      break;
    case 0 :
      err = WSAETIMEDOUT;
      break;
    default :
      err = GetLastError();
  }

  if (::ioctlsocket(os_handle, FIONBIO, &fionbio) == SOCKET_ERROR) {
    if (err == 0)
      err = GetLastError();
  }

  // The following is to avoid a bug in Win32 sockets. The getpeername() function doesn't
  // work for some period of time after a connect, saying it is not connected yet!
  for (PINDEX failsafe = 0; failsafe < 1000; failsafe++) {
    sockaddr_in address;
    int sz = sizeof(address);
    if (::getpeername(os_handle, (struct sockaddr *)&address, &sz) == 0)
      break;
    ::Sleep(0);
  }

  SetLastError(err);
  return err == 0 ? 0 : SOCKET_ERROR;
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size,
                       const PTimeInterval & timeout)
{
  if (timeout != PMaxTimeInterval) {
    fd_set_class readfds = sock;
    timeval_class tv = timeout;
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
  lastReadCount = 0;

  if (readTimeout != PMaxTimeInterval) {
    DWORD available;
    if (!ConvertOSError(ioctlsocket(os_handle, FIONREAD, &available)))
      return FALSE;

    if (available == 0) {
      fd_set_class readfds = os_handle;
      timeval_class tv = readTimeout;
      int selval = ::select(0, &readfds, NULL, NULL, &tv);
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

  int recvResult = ::recvfrom(os_handle, (char *)buf, len, flags, from, fromlen);
  if (!ConvertOSError(recvResult))
    return FALSE;

  lastReadCount = recvResult;
  return TRUE;
}


BOOL PSocket::os_sendto(const void * buf,
                        PINDEX len,
                        int flags,
                        struct sockaddr * to,
                        int tolen)
{
  lastWriteCount = 0;

  if (writeTimeout != PMaxTimeInterval) {
    fd_set_class writefds = os_handle;
    timeval_class tv = writeTimeout;
    int selval = ::select(0, NULL, &writefds, NULL, &tv);
    if (selval < 0)
      return FALSE;

    if (selval == 0) {
      errno = EAGAIN;
      return FALSE;
    }
  }

  int sendResult = ::sendto(os_handle, (const char *)buf, len, flags, to, tolen);
  if (!ConvertOSError(sendResult))
    return FALSE;

  lastWriteCount = sendResult;
  return TRUE;
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


PIPSocket::Address::Address(DWORD dw)
{
  S_un.S_addr = dw;
}


PIPSocket::Address & PIPSocket::Address::operator=(DWORD dw)
{
  S_un.S_addr = dw;
  return *this;
}


PIPSocket::Address::operator DWORD() const
{
  return S_un.S_addr;
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


BOOL P_IsOldWin95()
{
  static int state = -1;
  if (state < 0) {
    state = 1;
    OSVERSIONINFO info;
    info.dwOSVersionInfoSize = sizeof(info);
    if (GetVersionEx(&info)) {
      state = 0;
      if (info.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS && info.dwBuildNumber < 1000)
        state = 1;
    }
  }
  return state != 0;
}


BOOL PIPSocket::IsLocalHost(const PString & hostname)
{
  if (hostname.IsEmpty())
    return TRUE;

  if (hostname *= "localhost")
    return TRUE;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address addr = hostname;
  if (addr == 16777343)  // Is 127.0.0.1
    return TRUE;

  if (addr == 0) {
    if (!GetHostAddress(hostname, addr))
      return FALSE;
  }

  struct hostent * host_info = ::gethostbyname(GetHostName());

  if (P_IsOldWin95())
    return addr == Address(*(struct in_addr *)host_info->h_addr_list[0]);

  for (PINDEX i = 0; host_info->h_addr_list[i] != NULL; i++) {
    if (addr == Address(*(struct in_addr *)host_info->h_addr_list[i]))
      return TRUE;
  }

  return FALSE;
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
  PINDEX colon = str.Find(':');
  if (colon == P_MAX_INDEX)
    colon = 0;
  else {
    DWORD netnum = 0;
    for (PINDEX i = 0; i < colon; i++) {
      int c = str[i];
      if (isdigit(c))
        netnum = (netnum << 4) + c - '0';
      else if (isxdigit(c))
        netnum = (netnum << 4) + toupper(c) - 'A' + 10;
      else {
        memset(this, 0, sizeof(*this));
        return;
      }
    }
    network.dw = ntohl(netnum);
  }

  memset(node, 0, sizeof(node));

  int shift = 0;
  PINDEX byte = 5;
  PINDEX pos = str.GetLength();
  while (--pos > colon) {
    int c = str[pos];
    if (c != '-') {
      if (isdigit(c))
        node[byte] |= (c - '0') << shift;
      else if (isxdigit(c))
        node[byte] |= (toupper(c) - 'A' + 10) << shift;
      else {
        memset(this, 0, sizeof(*this));
        return;
      }
      if (shift == 0)
        shift = 4;
      else {
        shift = 0;
        byte--;
      }
    }
  }
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
  return psprintf("%02X%02X%02X%02X:%02X%02X%02X%02X%02X%02X",
                  network.b.b1, network.b.b2, network.b.b3, network.b.b4,
                  node[0], node[1], node[2], node[3], node[4], node[5]);
}


BOOL PIPXSocket::Address::IsValid() const
{
  static Address empty;
  return memcmp(this, &empty, sizeof(empty)) != 0;
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


BOOL PIPXSocket::SetPacketType(int type)
{
  return ConvertOSError(::setsockopt(os_handle,
                           NSPROTO_IPX, IPX_PTYPE, (char *)&type, sizeof(type)));
}


int PIPXSocket::GetPacketType()
{
  int value;
  int valSize = sizeof(value);
  if (ConvertOSError(::getsockopt(os_handle,
                                NSPROTO_IPX, IPX_PTYPE, (char *)&value, &valSize)))
    return value;
  return -1;
}


PString PIPXSocket::GetHostName(const Address & addr)
{
  return addr;
}


BOOL PIPXSocket::GetHostAddress(Address &)
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


BOOL PIPXSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  addr = hostname;
  if (addr.IsValid())
    return TRUE;

  static GUID netware_file_server = SVCID_FILE_SERVER;
  CSADDR_INFO addr_info[10];
  DWORD buffer_length = sizeof(addr_info);
	int num = GetAddressByName(NS_DEFAULT,
                             &netware_file_server,
                             (LPTSTR)(const char *)hostname,
                             NULL,
                             0,
                             NULL,
                             addr_info,
                             &buffer_length,
                             NULL,
                             NULL
                            );
  if (num <= 0)
    return FALSE;

  AssignAddress(addr, *(sockaddr_ipx *)addr_info[0].RemoteAddr.lpSockaddr);
  return TRUE;
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
  portNum = Net2Host(sip.sa_socket);
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
  portNum = Net2Host(sip.sa_socket);
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
  sip.sa_socket  = Host2Net(port);  // set the port
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
    sip.sa_socket = Host2Net(port);       // set the port

    if (ConvertOSError(::bind(os_handle, (struct sockaddr*)&sip, sizeof(sip)))) {
      int size = sizeof(sip);
      if (ConvertOSError(::getsockname(os_handle, (struct sockaddr*)&sip, &size))) {
        port = Net2Host(sip.sa_socket);
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
  if (os_recvfrom(buf, len, 0, (struct sockaddr *)&sip, &addrLen)) {
    AssignAddress(addr, sip);
    port = Net2Host(sip.sa_socket);
  }

  return lastReadCount > 0;
}


BOOL PIPXSocket::WriteTo(const void * buf, PINDEX len, const Address & addr, WORD port)
{
  lastWriteCount = 0;

  sockaddr_ipx sip;
  sip.sa_family = AF_IPX;
  AssignAddress(sip, addr);
  sip.sa_socket = Host2Net(port);
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
