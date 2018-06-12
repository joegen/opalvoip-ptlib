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
 */

#include <ptlib.h>
#include <ptlib/sockets.h>

#include <svcguid.h>
#include <iphlpapi.h>

#include <nspapi.h>
#include <wsipx.h>
#include <netioapi.h>
#include <tchar.h>

#ifdef _MSC_VER
  #include <wsnwlink.h>

  #pragma comment(lib, "ws2_32.lib")

#else

  #define IPX_PTYPE 0x4000

  #ifndef NS_DEFAULT
    #define NS_DEFAULT 0
  #endif

  #ifndef SVCID_NETWARE
  #define SVCID_NETWARE(_SapId) {(0x000B << 16)|(_SapId),0,0,{0xC0,0,0,0,0,0,0,0x46}}
  #endif /* SVCID_NETWARE */

  #define SVCID_FILE_SERVER SVCID_NETWARE(0x4)

#endif

#if P_GQOS
  #include <qossp.h>
#endif

#if P_QWAVE
  #include <qos2.h>
#endif

#ifdef _MSC_VER
  #pragma comment(lib, "iphlpapi.lib")
  #if P_QWAVE
    #pragma comment(lib, "qwave.lib")
  #endif
#endif


#define PTraceModule() "WinSock"


//////////////////////////////////////////////////////////////////////////////
// PWinSock

// Must be one and one only instance of this class, and it must be static!.
struct PWinSock : public PProcessStartup
{
  PFACTORY_GET_SINGLETON(PProcessStartupFactory, PWinSock);

  bool   m_useGQOS;
  HANDLE m_hQoS;

  PWinSock()
    : m_useGQOS(false)
    , m_hQoS(NULL)
  {
    // ensure we support QoS
    WSADATA m_info;
    PAssert(WSAStartup(0x0202, &m_info) == 0, POperatingSystemError);
    PAssert(m_info.wVersion >= 0x0200, POperatingSystemError);

#if P_GQOS
    // Use for NT (5.1) and Win2003 (5.2) but not Win2000 (5.0) or Vista and later (6.x)
    m_useGQOS = PProcess::IsOSVersion(5,1,0) && !PProcess::IsOSVersion(6,0,0);
#endif // P_GQOS

#if P_QWAVE
    // Seems to crash on Server 2003
    if (PProcess::IsOSVersion(6, 0, 0)) {
      // Not always installed so test if there
      HMODULE hTest = LoadLibrary("QWAVE.DLL");
      if (hTest != NULL) {
        QOS_VERSION qosVersion;
        qosVersion.MajorVersion = 1;
        qosVersion.MinorVersion = 0;
        QOSCreateHandle(&qosVersion, &m_hQoS);
        FreeLibrary(hTest);
      }
    }
#endif // P_QWAVE
  }

  ~PWinSock()
  {
#if P_QWAVE
    if (m_hQoS != NULL)
      QOSCloseHandle(m_hQoS);
#endif // P_QWAVE

    WSACleanup();
  }
};

PFACTORY_CREATE_SINGLETON(PProcessStartupFactory, PWinSock);


//////////////////////////////////////////////////////////////////////////////
// P_fd_set

P_fd_set::P_fd_set()
{
  Construct();
  Zero();
}

void P_fd_set::Construct()
{
  max_fd = INT_MAX;
  set = (fd_set *)malloc(sizeof(fd_set));
}


void P_fd_set::Zero()
{
  if (PAssertNULL(set) != NULL)
    FD_ZERO(set);
}

PBoolean P_fd_set::IsPresent(intptr_t fd) const
{
  return FD_ISSET(fd, set);
}


#ifdef _MSC_VER
  #pragma warning(disable:4127)
#endif

P_fd_set::P_fd_set(intptr_t fd)
{
  Construct();
  Zero();
  FD_SET((SOCKET)fd, set);
}


P_fd_set & P_fd_set::operator=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  Zero();
  FD_SET((SOCKET)fd, set);
  return *this;
}


P_fd_set & P_fd_set::operator+=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  FD_SET((SOCKET)fd, set);
  return *this;
}


P_fd_set & P_fd_set::operator-=(intptr_t fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  FD_CLR((SOCKET)fd, set);
  return *this;
}

#ifdef _MSC_VER
  #pragma warning(default:4127)
#endif


//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::~PSocket()
{
  Close();
}


HANDLE PSocket::GetAsyncReadHandle() const
{
  return IsOpen() ? (HANDLE)(P_INT_PTR)os_handle : INVALID_HANDLE_VALUE;
}


HANDLE PSocket::GetAsyncWriteHandle() const
{
  return IsOpen() ? (HANDLE)(P_INT_PTR)os_handle : INVALID_HANDLE_VALUE;
}


PBoolean PSocket::Read(void * buf, PINDEX len)
{
  SetLastReadCount(0);

  if (CheckNotOpen())
    return false;

  if (len == 0)
    return SetErrorValues(BadParameter, EINVAL, LastReadError);

  Slice slice(buf, len);
  os_vread(&slice, 1, 0, NULL, NULL);
  return GetLastReadCount() > 0;
}


PBoolean PSocket::Read(Slice * slices, size_t sliceCount)
{
  SetLastReadCount(0);

  if (CheckNotOpen())
    return false;

  os_vread(slices, sliceCount, 0, NULL, NULL);
  return GetLastReadCount() > 0;
}


PBoolean PSocket::Write(const void * buf, PINDEX len)
{
  SetLastWriteCount(0);

  if (CheckNotOpen())
    return false;

  flush();
  Slice slice(buf, len);
  return os_vwrite(&slice, 1, 0, NULL, 0) && GetLastWriteCount() >= len;
}


PBoolean PSocket::Write(const Slice * slices, size_t sliceCount)
{
  SetLastWriteCount(0);

  if (CheckNotOpen())
    return false;

  flush();
  return os_vwrite(slices, sliceCount, 0, NULL, 0) && GetLastWriteCount() >= 0;
}


PBoolean PSocket::Close()
{
  if (CheckNotOpen())
    return false;

  flush();
  return ConvertOSError(os_close());
}


int PSocket::os_close()
{
  clear();

  SOCKET s = os_handle.exchange(-1);

  int retval;
  while ((retval = closesocket(s)) < 0 && GetLastError() == WSAEWOULDBLOCK)
    PThread::Sleep(10);
  return retval;
}


int PSocket::os_socket(int af, int type, int proto)
{
#if P_GQOS
  if (PWinSock::GetInstance().m_useGQOS) {
    //Try to find a QOS-enabled protocol
    DWORD bufferSize = 0;
    int numProtocols = WSAEnumProtocols(NULL, NULL, &bufferSize);
    if (numProtocols <= 0 && WSAGetLastError() != WSAENOBUFS) 
      return -1;

    PBYTEArray buffer(bufferSize);
    LPWSAPROTOCOL_INFO qosProtocol = (LPWSAPROTOCOL_INFO)buffer.GetPointer();

    numProtocols = WSAEnumProtocols(NULL, qosProtocol, &bufferSize);
    for (int i = 0; i < numProtocols; qosProtocol++, i++) {
      if (  qosProtocol->iSocketType == type &&
            qosProtocol->iAddressFamily == af &&
           (qosProtocol->dwServiceFlags1 & XP1_QOS_SUPPORTED) != 0)
        return (int)WSASocket(af, type, proto, qosProtocol, 0, WSA_FLAG_OVERLAPPED);
    }
  }
#endif // P_GQOS

  return (int)WSASocket(af, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);
}


PBoolean PSocket::os_connect(struct sockaddr * addr, socklen_t size)
{
  if (readTimeout == PMaxTimeInterval)
    return ConvertOSError(::connect(os_handle, addr, size));

  DWORD fionbio = 1;
  if (!ConvertOSError(::ioctlsocket(os_handle, FIONBIO, &fionbio)))
    return false;

  DWORD err = ERROR_SUCCESS;

  if (::connect(os_handle, addr, size) == SOCKET_ERROR && (err = GetLastError()) == WSAEWOULDBLOCK) {
    P_fd_set writefds = os_handle;
    P_fd_set exceptfds = os_handle;
    P_timeval tv;

    /* To avoid some strange behaviour on various windows platforms, do a zero
        timeout select first to pick up errors. Then do real timeout. */
    int selerr = ::select(1, NULL, writefds, exceptfds, tv);
    if (selerr == 0) {
      writefds = os_handle;
      exceptfds = os_handle;
      tv = readTimeout;
      selerr = ::select(1, NULL, writefds, exceptfds, tv);
    }

    switch (selerr) {
      case 1 :
        if (writefds.IsPresent(os_handle)) {
          // The following is to avoid a bug in Win32 sockets. The getpeername() function doesn't
          // work for some period of time after a connect, saying it is not connected yet!
          for (PINDEX failsafe = 0; failsafe < 1000; failsafe++) {
            sockaddr_in address;
            int sz = sizeof(address);
            if (::getpeername(os_handle, (struct sockaddr *)&address, &sz) == 0) {
              if (address.sin_port != 0)
                break;
            }
            ::Sleep(0);
          }

          err = 0;
        }
        else {
          // The following is to avoid a bug in Win32 sockets. The getsockopt() function
          // doesn't work for some period of time after a connect, saying no error!
          for (PINDEX failsafe = 0; failsafe < 1000; failsafe++) {
            int sz = sizeof(err);
            if (::getsockopt(os_handle, SOL_SOCKET, SO_ERROR, (char *)&err, &sz) == 0) {
              if (err != 0)
                break;
            }
            ::Sleep(0);
          }
          if (err == 0)
            err = WSAEFAULT; // Need to have something!
        }
        break;

      case 0 :
        err = WSAETIMEDOUT;
        break;

      default :
        err = GetLastError();
    }
  }

  fionbio = 0;
  if (::ioctlsocket(os_handle, FIONBIO, &fionbio) < 0) {
    PTRACE(1, "Could not reset FIONBIO mode, error=" << GetLastError());
  }

  SetLastError(err);
  return ConvertOSError(err == 0 ? 0 : SOCKET_ERROR);
}


PBoolean PSocket::os_accept(PSocket & listener, struct sockaddr * addr, socklen_t * size)
{
  if (listener.GetReadTimeout() != PMaxTimeInterval) {
    P_fd_set readfds = listener.GetHandle();
    P_timeval tv = listener.GetReadTimeout();
    switch (select(0, readfds, NULL, NULL, tv)) {
      case 1 :
        break;
      case 0 :
        SetLastError(WSAETIMEDOUT);
        // Then return -1
      default :
        return ConvertOSError(-1);
    }
  }
  return ConvertOSError(os_handle = ::accept(listener.GetHandle(), addr, size));
}


bool PSocket::os_vread(Slice * slices, size_t sliceCount,
                       int flags,
                       struct sockaddr * from,
                       socklen_t * fromlen)
{
  SetLastReadCount(0);

  DWORD available;
  if (!ConvertOSError(ioctlsocket(os_handle, FIONREAD, &available), LastReadError))
    return false;

  if (available == 0) {
    P_fd_set readfds = os_handle;
    P_timeval tv = readTimeout;
    int selval = ::select(0, readfds, NULL, NULL, tv);
    if (!ConvertOSError(selval, LastReadError))
      return false;

    if (selval == 0)
      return SetErrorValues(Timeout, ETIMEDOUT, LastReadError);
  }

  DWORD receivedCount;
  DWORD dflags = flags;
  int recvResult = WSARecvFrom(os_handle, slices, sliceCount, &receivedCount, &dflags, from, fromlen, NULL, NULL);

  if (!ConvertOSError(recvResult, LastReadError))
    return false;

  SetLastReadCount(receivedCount);
  return true;
}


bool PSocket::os_vwrite(const Slice * slices,
                        size_t sliceCount,
                        int flags,
                        struct sockaddr * to,
                        socklen_t tolen)
{
  SetLastWriteCount(0);

  if (CheckNotOpen())
    return false;

  P_fd_set writefds = os_handle;
  P_timeval tv = writeTimeout;
  int selval = ::select(0, NULL, writefds, NULL, tv);
  if (!ConvertOSError(selval, LastWriteError))
    return false;

  if (selval == 0)
    return SetErrorValues(Timeout, ETIMEDOUT, LastWriteError);

  DWORD bytesSent = 0;
  PWin32Overlapped overlap;
  int sendResult = ::WSASendTo(os_handle, (LPWSABUF)slices, sliceCount, &bytesSent, flags, to, tolen, &overlap, NULL);
  if (sendResult < 0 && GetLastError() == ERROR_IO_PENDING) {
    DWORD resultFlags = 0;
    sendResult = ::WSAGetOverlappedResult(os_handle, &overlap, &bytesSent, true, &resultFlags) ? 0 : -1;
  }
  if (!ConvertOSError(sendResult, LastWriteError))
    return false;

  if (sendResult != 0)
    return false;

  SetLastWriteCount(bytesSent);
  return true;
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except,
                                 const PTimeInterval & timeout)
{
  PSocket * firstSocket = NULL;

  SelectList::iterator sock;
  P_fd_set readfds;
  for (sock = read.begin(); sock != read.end(); ++sock) {
    if (!sock->IsOpen())
      return NotOpen;
    readfds += sock->GetHandle();
    if (firstSocket == NULL)
      firstSocket = &*sock;
  }

  P_fd_set writefds;
  for (sock = write.begin(); sock != write.end(); ++sock) {
    if (!sock->IsOpen())
      return NotOpen;
    writefds += sock->GetHandle();
    if (firstSocket == NULL)
      firstSocket = &*sock;
  }

  P_fd_set exceptfds;
  for (sock = except.begin(); sock != except.end(); ++sock) {
    if (!sock->IsOpen())
      return NotOpen;
    exceptfds += sock->GetHandle();
    if (firstSocket == NULL)
      firstSocket = &*sock;
  }

  if (firstSocket == NULL)
    return BadParameter;

  P_timeval tval = timeout;
  int retval = select(INT_MAX, readfds, writefds, exceptfds, tval);

  if (!firstSocket->ConvertOSError(retval))
    return firstSocket->GetErrorCode();

  if (retval > 0) {
    sock = read.begin();
    while (sock != read.end()) {
      P_INT_PTR h = sock->GetHandle();
      if (h < 0)
        return Interrupted;
      if (readfds.IsPresent(h))
        ++sock;
      else
        read.erase(sock++);
    }
    sock = write.begin();
    while ( sock != write.end()) {
      P_INT_PTR h = sock->GetHandle();
      if (h < 0)
        return Interrupted;
      if (writefds.IsPresent(h))
        ++sock;
      else
        write.erase(sock++);
    }
    sock = except.begin();
    while ( sock != except.end()) {
      P_INT_PTR h = sock->GetHandle();
      if (h < 0)
        return Interrupted;
      if (exceptfds.IsPresent(h))
        ++sock;
      else
        except.erase(sock++);
    }
  }
  else {
    read.RemoveAll();
    write.RemoveAll();
    except.RemoveAll();
  }

  return NoError;
}


int PSocket::os_errno() const
{
  DWORD err = WSAGetLastError();
  SetLastError(err);

  switch (err) {
    case WSAEBADF:
      return EBADF;
    case WSAEINVAL:
      return EINVAL;
    case WSAEACCES:
      return EACCES;
    case WSAEINTR:
      return EINTR;
    case WSAEINPROGRESS:
      return EINPROGRESS;
    case WSAENOTSOCK:
      return ENOTSOCK;
    case WSAEOPNOTSUPP:
      return EOPNOTSUPP;
    case WSAEAFNOSUPPORT:
      return EAFNOSUPPORT;
    case WSAEADDRINUSE:
      return EADDRINUSE;
    case WSAEADDRNOTAVAIL:
      return EADDRNOTAVAIL;
    case WSAENETDOWN:
      return ENETDOWN;
    case WSAENETUNREACH:
      return ENETUNREACH;
    case WSAENETRESET:
      return ENETRESET;
    case WSAECONNABORTED:
      return ECONNABORTED;
    case WSAECONNRESET:
      return ECONNRESET;
    case WSAENOBUFS:
      return ENOBUFS;
    case WSAEISCONN:
      return EISCONN;
    case WSAENOTCONN:
      return ENOTCONN;
    case WSAECONNREFUSED:
      return ECONNREFUSED;
    case WSAEHOSTUNREACH:
      return EHOSTUNREACH;
    case WSAEMSGSIZE:
      return EMSGSIZE;
    case WSAEWOULDBLOCK:
      return EWOULDBLOCK;
    case WSAETIMEDOUT:
      return ETIMEDOUT;
  }

  return err|PWIN32ErrorFlag;
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket::Address

PIPSocket::Address::Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
  m_version = 4;
  m_v.m_four.S_un.S_un_b.s_b1 = b1;
  m_v.m_four.S_un.S_un_b.s_b2 = b2;
  m_v.m_four.S_un.S_un_b.s_b3 = b3;
  m_v.m_four.S_un.S_un_b.s_b4 = b4;
}


PIPSocket::Address::Address(DWORD dw)
{
  operator=(dw);
}


PIPSocket::Address & PIPSocket::Address::operator=(DWORD dw)
{
  m_version = 4;
  m_v.m_four.S_un.S_addr = dw;
  return *this;
}


PIPSocket::Address::operator DWORD() const
{
  return m_version != 4 ? 0 : m_v.m_four.S_un.S_addr;
}


BYTE PIPSocket::Address::Byte1() const
{
  return m_v.m_four.S_un.S_un_b.s_b1;
}


BYTE PIPSocket::Address::Byte2() const
{
  return m_v.m_four.S_un.S_un_b.s_b2;
}


BYTE PIPSocket::Address::Byte3() const
{
  return m_v.m_four.S_un.S_un_b.s_b3;
}


BYTE PIPSocket::Address::Byte4() const
{
  return m_v.m_four.S_un.S_un_b.s_b4;
}


///////////////////////////////////////////////////////////////////////////////

class PIPRouteTable
{
public:
  PIPRouteTable()
  {
    ULONG size = 0;
    DWORD error = GetIpForwardTable(NULL, &size, TRUE);
    if (error == ERROR_INSUFFICIENT_BUFFER && m_buffer.SetSize(size))
      error = GetIpForwardTable((MIB_IPFORWARDTABLE *)m_buffer.GetPointer(), &size, TRUE);
    if (error != NO_ERROR) {
      m_buffer.SetSize(0);
      m_buffer.SetSize(sizeof(MIB_IPFORWARDTABLE)); // So ->dwNumEntries returns zero
    }
  }

  const MIB_IPFORWARDTABLE * Ptr() const { return (const MIB_IPFORWARDTABLE *)(const BYTE *)m_buffer; }
  const MIB_IPFORWARDTABLE * operator->() const { return  Ptr(); }
  const MIB_IPFORWARDTABLE & operator *() const { return *Ptr(); }
  operator const MIB_IPFORWARDTABLE *  () const { return  Ptr(); }

  bool ValidateAddress(DWORD ifIndex, LPSOCKADDR lpSockAddr)
  {
    int numEntries = Ptr()->dwNumEntries;
    if (numEntries == 0)
      return true;

    if (lpSockAddr == NULL)
      return false;

    const MIB_IPFORWARDROW * row = Ptr()->table;
    for (int i = 0; i < numEntries; i++, row++) {
      if (row->dwForwardIfIndex == ifIndex && (u_long)row->dwForwardDest == ((sockaddr_in *)lpSockAddr)->sin_addr.S_un.S_addr)
              return true;
    }

    return false;
  }

  private:
    PBYTEArray m_buffer;
};


class PIPInterfaceAddressTable
{
public:
  PIPInterfaceAddressTable()
  {
    ULONG size = 0;
    DWORD error = GetIpAddrTable(NULL, &size, FALSE);
    if (error == ERROR_INSUFFICIENT_BUFFER && buffer.SetSize(size))
      error = GetIpAddrTable((MIB_IPADDRTABLE *)buffer.GetPointer(), &size, FALSE);
    if (error != NO_ERROR) {
      buffer.SetSize(0);
      buffer.SetSize(sizeof(MIB_IPADDRTABLE)); // So ->NumAdapters returns zero
    }
  }

  const MIB_IPADDRTABLE * Ptr() const { return (const MIB_IPADDRTABLE *)(const BYTE *)buffer; }
  const MIB_IPADDRTABLE * operator->() const { return  Ptr(); }
  const MIB_IPADDRTABLE & operator *() const { return *Ptr(); }
  operator const MIB_IPADDRTABLE *  () const { return  Ptr(); }

  private:
    PBYTEArray buffer;
};


#if (P_HAS_IPV6 || _WIN32_WINNT_WINXP)
class PIPAdaptersAddressTable
{
public:

  PIPAdaptersAddressTable(DWORD dwFlags = GAA_FLAG_INCLUDE_PREFIX
                                        | GAA_FLAG_SKIP_ANYCAST
                                        | GAA_FLAG_SKIP_DNS_SERVER
                                        | GAA_FLAG_SKIP_MULTICAST) 
  {
    ULONG size = 0;
    DWORD error = GetAdaptersAddresses(AF_UNSPEC, dwFlags, NULL, NULL, &size);
    if (buffer.SetSize(size))
      error = GetAdaptersAddresses(AF_UNSPEC, dwFlags, NULL, (IP_ADAPTER_ADDRESSES *)buffer.GetPointer(), &size);

    if (error != NO_ERROR) {
      buffer.SetSize(0);
      buffer.SetSize(sizeof(IP_ADAPTER_ADDRESSES)); // So ->NumAdapters returns zero
    }
  }

  const IP_ADAPTER_ADDRESSES * Ptr() const { return  (const IP_ADAPTER_ADDRESSES *)(const BYTE *)buffer; }
  const IP_ADAPTER_ADDRESSES * operator->() const { return  Ptr(); }
  const IP_ADAPTER_ADDRESSES & operator *() const { return *Ptr(); }
  operator const IP_ADAPTER_ADDRESSES *  () const { return  Ptr(); }

private:
  PBYTEArray buffer;
};
#endif

#if P_HAS_IPV6
typedef NETIO_STATUS (NETIOAPI_API_ *GETIPFORWARDTABLE2)(ADDRESS_FAMILY, PMIB_IPFORWARD_TABLE2 *);
typedef VOID (NETIOAPI_API_ *FREEMIBTABLE)(PVOID);

class PIPRouteTableIPv6
{
public:
  PIPRouteTableIPv6()
    : m_buffer(sizeof(MIB_IPFORWARD_TABLE2)) // So ->NumEntries returns zero
  {
    HINSTANCE hInst = LoadLibrary(_T("iphlpapi.dll"));
    if (hInst != NULL) {
      GETIPFORWARDTABLE2 pfGetIpForwardTable2 = (GETIPFORWARDTABLE2)GetProcAddress(hInst, _T("GetIpForwardTable2"));
      FREEMIBTABLE pfFreeMibTable = (FREEMIBTABLE)GetProcAddress(hInst, _T("FreeMibTable"));
      if (pfGetIpForwardTable2 != NULL && pfFreeMibTable != NULL) {
        PMIB_IPFORWARD_TABLE2 pt = NULL;
        if (pfGetIpForwardTable2(AF_UNSPEC, &pt) == NO_ERROR) {
          PINDEX sz = pt->NumEntries * sizeof(MIB_IPFORWARD_ROW2);
          memcpy(m_buffer.GetPointer(sz), pt, sz);
          pfFreeMibTable(pt);
        }
      }
      FreeLibrary(hInst);
    }
  }

  const MIB_IPFORWARD_TABLE2 * Ptr() const { return  (const MIB_IPFORWARD_TABLE2 *)(const BYTE *)m_buffer; }
  const MIB_IPFORWARD_TABLE2 * operator->() const { return  Ptr(); }
  const MIB_IPFORWARD_TABLE2 & operator *() const { return *Ptr(); }
  operator const MIB_IPFORWARD_TABLE2 *  () const { return  Ptr(); }


  bool ValidateAddress(DWORD ifIndex, LPSOCKADDR lpSockAddr)
  {
    int numEntries = Ptr()->NumEntries;
    if (numEntries == 0)
      return true;

    if (lpSockAddr == NULL)
      return false;

    const MIB_IPFORWARD_ROW2 * row = Ptr()->Table;
    for (int i = 0; i < numEntries; i++, row++) {
      if (row->InterfaceIndex == ifIndex &&
          row->DestinationPrefix.Prefix.si_family == lpSockAddr->sa_family) {
        switch (lpSockAddr->sa_family) {
          case AF_INET :
            if (row->DestinationPrefix.Prefix.Ipv4.sin_addr.S_un.S_addr == ((sockaddr_in *)lpSockAddr)->sin_addr.S_un.S_addr)
              return true;
            break;

          case AF_INET6 :
            if (memcmp(row->DestinationPrefix.Prefix.Ipv6.sin6_addr.u.Byte,
                ((sockaddr_in6 *)lpSockAddr)->sin6_addr.u.Byte, sizeof(in6_addr)) == 0)
              return true;
        }
      }
    }

    return false;
  }

private:
  PBYTEArray m_buffer;
};

static PIPSocket::Address FromSOCKADDR_INET(const SOCKADDR_INET & sa)
{
  switch (sa.si_family) {
    case AF_INET6 :
      return sa.Ipv6.sin6_addr;

    case AF_INET :
      return sa.Ipv4.sin_addr;
  }

  return PIPSocket::GetInvalidAddress();
}

#endif // P_HAS_IPV6


///////////////////////////////////////////////////////////////////////////////

PIPSocket::PIPSocket()
#if P_QWAVE
  : m_qosFlowId(0)
#endif
{
}


#if P_QWAVE
PBoolean PIPSocket::Close()
{
  if (IsOpen() && PWinSock::GetInstance().m_hQoS != NULL && m_qosFlowId != 0) {
    QOSRemoveSocketFromFlow(PWinSock::GetInstance().m_hQoS, os_handle, m_qosFlowId, 0);
    m_qosFlowId = 0;
  }

  return PSocket::Close();
}
#endif // P_QWAVE


PString PIPSocket::GetInterface(const Address & addr)
{
  PIPInterfaceAddressTable byAddress;
  for (unsigned i = 0; i < byAddress->dwNumEntries; ++i) {
    if (addr == byAddress->table[i].dwAddr) {
      MIB_IFROW info;
      info.dwIndex = byAddress->table[i].dwIndex;
      if (GetIfEntry(&info) == NO_ERROR)
        return PString((const char *)info.bDescr, info.dwDescrLen);
    }
  }

  return PString::Empty();
}


PIPSocket::Address PIPSocket::GetGatewayAddress(unsigned version)
{
  if (version == 6) {
#if P_HAS_IPV6
    PIPRouteTableIPv6 routes;
    int numEntries = routes->NumEntries;
    if (numEntries > 0) {
      const MIB_IPFORWARD_ROW2 * row = routes->Table;
      for (int i = 0; i < numEntries; i++, row++) {
        PIPSocket::Address hop = FromSOCKADDR_INET(row->NextHop);
        if (row->DestinationPrefix.PrefixLength == 0 && hop.GetVersion() == version && !hop.IsAny())
          return hop;
      }
    }
#endif
    return GetInvalidAddress();
  }

  PIPRouteTable routes;
  for (unsigned i = 0; i < routes->dwNumEntries; ++i) {
    if (routes->table[i].dwForwardMask == 0)
      return routes->table[i].dwForwardNextHop;
  }
  return  GetInvalidAddress();
}


PString PIPSocket::GetGatewayInterface(unsigned version)
{
  if (version == 6) {
#if P_HAS_IPV6
    PIPRouteTableIPv6 routes;
    int numEntries = routes->NumEntries;
    if (numEntries > 0) {
      const MIB_IPFORWARD_ROW2 * row = routes->Table;
      for (int i = 0; i < numEntries; i++, row++) {
        if (FromSOCKADDR_INET(row->NextHop).GetVersion() == 6) {
          PIPAdaptersAddressTable adapters;
          for (const IP_ADAPTER_ADDRESSES * adapter = &*adapters; adapter != NULL; adapter = adapter->Next) {
            if (adapter->IfIndex == row->InterfaceIndex)
              return adapter->Description;
          }
        }
      }
    }
#endif
    return PString::Empty();
  }

  PIPRouteTable routes;
  for (unsigned i = 0; i < routes->dwNumEntries; ++i) {
    if (routes->table[i].dwForwardMask == 0) {
      MIB_IFROW info;
      info.dwIndex = routes->table[i].dwForwardIfIndex;
      if (GetIfEntry(&info) == NO_ERROR)
        return PString((const char *)info.bDescr, info.dwDescrLen);
    }
  }
  return PString::Empty();
}


PIPSocket::Address PIPSocket::GetGatewayInterfaceAddress(unsigned version)
{
  if (version == 6) {
#if P_HAS_IPV6
    PIPRouteTableIPv6 routes;
    if (routes->NumEntries > 0) {
      PIPAdaptersAddressTable interfaces;

      for (const IP_ADAPTER_ADDRESSES * adapter = &*interfaces; adapter != NULL; adapter = adapter->Next) {
        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
          if (unicast->Address.lpSockaddr->sa_family != AF_INET6)
            continue;

          PIPSocket::Address ip(((sockaddr_in6 *)unicast->Address.lpSockaddr)->sin6_addr);

          if (routes->NumEntries == 0)
            return ip;

          if (!ip.IsLinkLocal() && routes.ValidateAddress(adapter->IfIndex, unicast->Address.lpSockaddr))
            return ip;
        }
      }
    }
#else
    return GetInvalidAddress();
#endif
  }

  PIPRouteTable routes;
  for (unsigned i = 0; i < routes->dwNumEntries; ++i) {
    if (routes->table[i].dwForwardMask == 0) {
      PIPInterfaceAddressTable interfaces;
      for (unsigned j = 0; j < interfaces->dwNumEntries; ++j) {
        if (interfaces->table[j].dwIndex == routes->table[i].dwForwardIfIndex)
          return interfaces->table[j].dwAddr;
      }
    }
  }

  return GetInvalidAddress();
}


bool PIPSocket::SetQoS(const QoS & qos)
{
  m_qos = qos;

  int new_tos = qos.m_dscp >= 0 || qos.m_dscp < 64 ? (qos.m_dscp<<2) : -1;

#if P_GQOS
  if (PWinSock::GetInstance().m_useGQOS && new_tos < 0 && qos.m_type != BestEffortQoS) {
    QOS qosBuf;
    memset(&qosBuf, 0, sizeof(qosBuf));

    static DWORD const ServiceType[NumQoSType] = {
      SERVICETYPE_BESTEFFORT,     // BackgroundQoS
      SERVICETYPE_BESTEFFORT,     // BestEffortQoS
      SERVICETYPE_CONTROLLEDLOAD, // ExcellentEffortQoS
      SERVICETYPE_GUARANTEED,     // CriticalQoS
      SERVICETYPE_CONTROLLEDLOAD, // VideoQoS
      SERVICETYPE_GUARANTEED,     // VoiceQoS
      SERVICETYPE_GUARANTEED      // ControlQoS
    };
    qosBuf.SendingFlowspec.ServiceType = ServiceType[qos.m_type];
    qosBuf.SendingFlowspec.PeakBandwidth = qos.m_transmit.m_maxBandwidth;
    qosBuf.SendingFlowspec.MaxSduSize = qos.m_transmit.m_maxPacketSize;
    qosBuf.SendingFlowspec.Latency = qos.m_transmit.m_maxLatency;
    qosBuf.SendingFlowspec.DelayVariation = qos.m_transmit.m_maxJitter;
    qosBuf.SendingFlowspec.TokenBucketSize = qos.m_transmit.m_maxPacketSize*11/10 + 1;
    qosBuf.SendingFlowspec.TokenRate = qos.m_transmit.m_maxBandwidth/qosBuf.SendingFlowspec.TokenBucketSize;

    qosBuf.ReceivingFlowspec.ServiceType = ServiceType[qos.m_type];
    qosBuf.ReceivingFlowspec.PeakBandwidth = qos.m_receive.m_maxBandwidth;
    qosBuf.ReceivingFlowspec.MaxSduSize = qos.m_receive.m_maxPacketSize;
    qosBuf.ReceivingFlowspec.Latency = qos.m_receive.m_maxLatency;
    qosBuf.ReceivingFlowspec.DelayVariation = qos.m_receive.m_maxJitter;
    qosBuf.ReceivingFlowspec.TokenBucketSize = qos.m_receive.m_maxPacketSize*11/10 + 1;
    qosBuf.ReceivingFlowspec.TokenRate = qos.m_receive.m_maxBandwidth/qosBuf.ReceivingFlowspec.TokenBucketSize;

    PWin32Overlapped overlap;
    DWORD dummyBuf[1];
    DWORD dummyLen = 0;
    if (ConvertOSError(WSAIoctl(os_handle, SIO_SET_QOS, &qosBuf, sizeof(qosBuf), dummyBuf, sizeof(dummyBuf), &dummyLen, &overlap, NULL))) {
      if (qos.m_dscp < 0)
        return true;
    }
    else {
      PTRACE(2, "Could not SIO_SET_QOS: " << GetErrorText());
    }
  }
#endif // P_GQOS

#if P_QWAVE
  if (PWinSock::GetInstance().m_hQoS != NULL) {
    static QOS_TRAFFIC_TYPE const TrafficType[NumQoSType] = {
      QOSTrafficTypeBackground,      // BackgroundQoS
      QOSTrafficTypeBestEffort,      // BestEffortQoS
      QOSTrafficTypeExcellentEffort, // ExcellentEffortQoS
      QOSTrafficTypeControl,         // CriticalQoS
      QOSTrafficTypeAudioVideo,      // VideoQoS
      QOSTrafficTypeVoice,           // VoiceQoS
      QOSTrafficTypeControl          // ControlQoS
    };

    if (m_qosFlowId == 0) {
      PIPSocketAddressAndPort peer = qos.m_remote;
      if (!peer.IsValid() && !GetPeerAddress(peer)) {
        PTRACE(3, "Cannot set QoS, no peer address yet.");
        return false;
      }

      if (!QOSAddSocketToFlow(PWinSock::GetInstance().m_hQoS, os_handle,
                              sockaddr_wrapper(peer), TrafficType[qos.m_type], QOS_NON_ADAPTIVE_FLOW, &m_qosFlowId)) {
        PTRACE(1, "Could not add socket to QoS flow, error=" << ::GetLastError());
        return false;
      }
    }
    else {
      if (!QOSSetFlow(PWinSock::GetInstance().m_hQoS, m_qosFlowId, QOSSetTrafficType,
                      sizeof(QOS_TRAFFIC_TYPE), (PVOID)&TrafficType[qos.m_type], 0, NULL)) {
        PTRACE(1, "Could not set QoS flow, error=" << ::GetLastError());
        return false;
      }
    }

    if (qos.m_transmit.m_maxBandwidth > 0) {
      QOS_FLOWRATE_OUTGOING outQoS;
	  outQoS.Bandwidth = qos.m_transmit.m_maxBandwidth;
	  outQoS.ShapingBehavior = QOSUseNonConformantMarkings;
	  outQoS.Reason = QOSFlowRateNotApplicable;
      if (!QOSSetFlow(PWinSock::GetInstance().m_hQoS, m_qosFlowId, QOSSetOutgoingRate, sizeof(outQoS), &outQoS, 0, NULL)) {
        PTRACE(2, "Could not set QoS rates, error=" << ::GetLastError());
      }
    }

#if P_QWAVE_DSCP
    if (new_tos < 0)
      return true;

    DWORD dscp = qos.m_dscp;
    if (QOSSetFlow(PWinSock::GetInstance().m_hQoS, m_qosFlowId, QOSSetOutgoingDSCPValue, sizeof(dscp), &dscp, 0, NULL))
      return true;

    PTRACE(2, "Could not set DSCP, error=" << ::GetLastError());
    return false;
#endif // P_QWAVE_DSCP
  }
#endif // P_QWAVE

  // Not explicitly set, so make a value up.
  if (new_tos < 0) {
    static int const DSCP[PIPSocket::NumQoSType] = {
      0,     // BackgroundQoS
      0,     // BestEffortQoS
      8<<2,  // ExcellentEffortQoS
      10<<2, // CriticalQoS
      38<<2, // VideoQoS
      44<<2, // VoiceQoS
      48<<2  // ControlQoS
    };
    new_tos = DSCP[qos.m_type];
  }

  // On Win7 and later IP_TOS succeeds, but does not actually do anything. Useless!
  if (PProcess::IsOSVersion(6, 1, 0)) {
    PTRACE(3, "Setting TOS field of IP header does not work in Windows 7 and later.");
    return false;
  }

  if (!SetOption(IP_TOS, new_tos, IPPROTO_IP)) {
    PTRACE(2, "Could not set TOS field in IP header: " << GetErrorText());
    return false;
  }

  int actual_tos;
  if (!GetOption(IP_TOS, actual_tos, IPPROTO_IP)) {
    PTRACE(1, "Could not get TOS field in IP header: " << GetErrorText());
    return false;
  }

  if (new_tos == actual_tos)
    return true;

  PTRACE(2, "Setting TOS field of IP header appeared successful, but was not really set.");
  return false;
}


PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
#if P_HAS_IPV6
  PIPRouteTableIPv6 routes;
  if (!table.SetSize(routes->NumEntries) || table.IsEmpty())
    return false;

  for (ULONG i = 0; i < routes->NumEntries; ++i) {
    RouteEntry * entry = new RouteEntry(FromSOCKADDR_INET(routes->Table[i].DestinationPrefix.Prefix));
    uint8_t mask[128/8];
    PINDEX offset = routes->Table[i].DestinationPrefix.PrefixLength/8;
    memset(mask, 0xff, offset);
    if (offset < sizeof(mask)) {
      memset(mask+offset, 0, sizeof(mask)-offset);
      mask[offset] = (uint8_t)(0xff << (8 - routes->Table[i].DestinationPrefix.PrefixLength%8));
    }
    entry->net_mask = Address(entry->network.GetSize(), mask);
    entry->destination = FromSOCKADDR_INET(routes->Table[i].NextHop);
    entry->metric = routes->Table[i].Metric;

    MIB_IFROW info;
    info.dwIndex = routes->Table[i].InterfaceIndex;
    if (GetIfEntry(&info) == NO_ERROR)
      entry->interfaceName = PString((const char *)info.bDescr, info.dwDescrLen);
    table.SetAt(i, entry);
  }
#else
  PIPRouteTable routes;

  if (!table.SetSize(routes->dwNumEntries) || table.IsEmpty())
    return false;

  for (unsigned i = 0; i < routes->dwNumEntries; ++i) {
    RouteEntry * entry = new RouteEntry(routes->table[i].dwForwardDest);
    entry->net_mask = routes->table[i].dwForwardMask;
    entry->destination = routes->table[i].dwForwardNextHop;
    entry->metric = routes->table[i].dwForwardMetric1;

    MIB_IFROW info;
    info.dwIndex = routes->table[i].dwForwardIfIndex;
    if (GetIfEntry(&info) == NO_ERROR)
      entry->interfaceName = PString((const char *)info.bDescr, info.dwDescrLen);
    table.SetAt(i, entry);
  }
#endif

  return true;
}


#ifdef _MSC_VER
#pragma optimize("g", off)
#endif

class Win32RouteTableDetector : public PIPSocket::RouteTableDetector
{
    PDynaLink  m_dll;
    BOOL    (WINAPI * m_pCancelIPChangeNotify )(LPOVERLAPPED);
    HANDLE     m_hCancel;

  public:
    Win32RouteTableDetector()
      : m_dll("iphlpapi.dll")
      , m_hCancel(CreateEvent(NULL, TRUE, FALSE, NULL))
    {
      if (!m_dll.GetFunction("CancelIPChangeNotify", (PDynaLink::Function&)m_pCancelIPChangeNotify))
        m_pCancelIPChangeNotify = NULL;
    }

    ~Win32RouteTableDetector()
    {
      if (m_hCancel != NULL)
        CloseHandle(m_hCancel);
    }

    virtual bool Wait(const PTimeInterval & timeout)
    {
      HANDLE hNotify = NULL;
      OVERLAPPED overlap;

      if (m_pCancelIPChangeNotify != NULL) {
        memset(&overlap, 0, sizeof(overlap));
        DWORD error = NotifyAddrChange(&hNotify, &overlap);
        if (error != ERROR_IO_PENDING) {
          PTRACE(1, "Could not get network interface change notification: error=" << error);
          hNotify = NULL;
        }
      }

      if (hNotify == NULL)
        return WaitForSingleObject(m_hCancel, timeout.GetInterval()) == WAIT_TIMEOUT;

      HANDLE handles[2];
      handles[0] = hNotify;
      handles[1] = m_hCancel;
      switch (WaitForMultipleObjects(2, handles, false, INFINITE)) {
        case WAIT_OBJECT_0 :
          return true;

        case WAIT_OBJECT_0+1 :
          if (m_pCancelIPChangeNotify != NULL)
            m_pCancelIPChangeNotify(&overlap);
          // Do next case

        default :
          return false;
      }
    }

    virtual void Cancel()
    {
      SetEvent(m_hCancel);
    }
};

#ifdef _MSC_VER
#pragma optimize("", on)
#endif


PIPSocket::RouteTableDetector * PIPSocket::CreateRouteTableDetector()
{
  return new Win32RouteTableDetector();
}


PIPSocket::Address PIPSocket::GetRouteInterfaceAddress(const PIPSocket::Address & remoteAddress)
{
#if P_HAS_IPV6
  if (remoteAddress.GetVersion() == 6) {
    sockaddr_in6 addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin6_family = AF_INET6;
    addr.sin6_addr = remoteAddress;
    DWORD best;
    if (GetBestInterfaceEx((sockaddr*)&addr, &best) == NO_ERROR) {
      PIPAdaptersAddressTable interfaces;
      PIPRouteTableIPv6 routes;
      for (const IP_ADAPTER_ADDRESSES * adapter = &*interfaces; adapter != NULL; adapter = adapter->Next) {
        if (adapter->IfIndex != best)
          continue;

        for (PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
          if (unicast->Address.lpSockaddr->sa_family == AF_INET6 &&
              routes.ValidateAddress(adapter->IfIndex, unicast->Address.lpSockaddr))
            return ((sockaddr_in6 *)unicast->Address.lpSockaddr)->sin6_addr;
        }
      }
    }
  }
  else
#endif
  {
    // For some variants of Windows GetBestInterface will return 127.0.0.1
    // when we are trying to talk to one of our own interfaces.
    PIPInterfaceAddressTable interfaces;
    for (DWORD i = 0; i < interfaces->dwNumEntries; ++i) {
      if (remoteAddress == interfaces->table[i].dwAddr)
        return remoteAddress;
    }

    DWORD best;
    if (GetBestInterface(remoteAddress, &best) == NO_ERROR) {
      for (DWORD i = 0; i < interfaces->dwNumEntries; ++i) {
        if (interfaces->table[i].dwIndex == best)
          return interfaces->table[i].dwAddr;
      }
    }
  }

  return GetInvalidAddress();
}


PBoolean PIPSocket::GetInterfaceTable(InterfaceTable & table, PBoolean includeDown)
{
  PIPInterfaceAddressTable byAddress;

  if (!table.SetSize(0))
    return false;

  PINDEX count = 0; // address count

#if P_HAS_IPV6 || _WIN32_WINNT_WINXP
  PIPAdaptersAddressTable interfaces;

#if P_HAS_IPV6
  PIPRouteTableIPv6 routes;
#else _WIN32_WINNT_WINXP
  PIPRouteTable routes;
#endif

  for (const IP_ADAPTER_ADDRESSES * adapter = &*interfaces; adapter != NULL; adapter = adapter->Next) {
    if (!includeDown && (adapter->OperStatus != IfOperStatusUp))
      continue;

    for (PIP_ADAPTER_UNICAST_ADDRESS unicast = adapter->FirstUnicastAddress; unicast != NULL; unicast = unicast->Next) {
      if (!routes.ValidateAddress(adapter->IfIndex, unicast->Address.lpSockaddr))
        continue;

      PStringStream macAddr;
      macAddr << ::hex << ::setfill('0');
      for (unsigned b = 0; b < adapter->PhysicalAddressLength; ++b)
        macAddr << setw(2) << (unsigned)adapter->PhysicalAddress[b];

      if (unicast->Address.lpSockaddr->sa_family == AF_INET) {
        PIPSocket::Address addr(((sockaddr_in *)unicast->Address.lpSockaddr)->sin_addr);
        if (addr.IsAny() || addr.IsBroadcast())
          addr = GetInvalidAddress();

        // Find out address index in byAddress table for the mask
        Address mask = Address::GetBroadcast(addr.GetVersion());
        for (unsigned i = 0; i < byAddress->dwNumEntries; ++i) {
          if (adapter->IfIndex == byAddress->table[i].dwIndex) {
            mask = byAddress->table[i].dwMask;
            break; 
          }
        } // find mask for the address

        table.SetAt(count++, new InterfaceEntry(adapter->Description, addr, mask, macAddr));
      } // ipv4
#if P_HAS_IPV6
      else if (unicast->Address.lpSockaddr->sa_family == AF_INET6) {
        sockaddr_in6 * sock6 = (sockaddr_in6 *)unicast->Address.lpSockaddr;
        PIPSocket::Address addr(sock6->sin6_addr, sock6->sin6_scope_id);
        if (addr.IsAny() || addr.IsBroadcast())
          addr = GetInvalidAddress();
        table.SetAt(count++, new InterfaceEntry(adapter->Description, addr, 0L, macAddr));
      }
#endif
    }
  }
#else

  for (unsigned i = 0; i < byAddress->dwNumEntries; ++i) {
    MIB_IFROW info;
    info.dwIndex = byAddress->table[i].dwIndex;
    if (GetIfEntry(&info) == NO_ERROR && (includeDown || info.dwAdminStatus != 0)) {
      PStringStream macAddr;
      macAddr << ::hex << ::setfill('0');
      for (unsigned b = 0; b < info.dwPhysAddrLen; ++b)
        macAddr << setw(2) << (unsigned)info.bPhysAddr[b];

      Address addr = byAddress->table[i].dwAddr;
      if (addr.IsAny() || addr.IsBroadcast())
        addr = GetInvalidAddress();

      table.SetAt(count++, new InterfaceEntry(PString((const char *)info.bDescr, info.dwDescrLen),
                                              addr,
                                              byAddress->table[i].dwMask,
                                              macAddr));
    }
  }

#endif

  return !table.IsEmpty();
}


// End Of File ///////////////////////////////////////////////////////////////
