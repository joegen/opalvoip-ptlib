/*
 * socket.cxx
 *
 * Berkley sockets classes implementation
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
 * $Log: socket.cxx,v $
 * Revision 1.29  1998/09/24 07:55:51  robertj
 * Fixed warning on solaris build.
 *
 * Revision 1.28  1998/09/24 04:13:49  robertj
 * Added open software license.
 *
 * Revision 1.27  1998/09/18 05:46:00  robertj
 * Fixed incorrectly returning success on a connect() error other than a timeout.
 *
 * Revision 1.26  1998/09/08 11:31:51  robertj
 * Fixed ippp bug on very full packets.
 *
 * Revision 1.25  1998/09/08 09:54:31  robertj
 * Fixed ppp and ippp compatibility.
 *
 * Revision 1.24  1998/09/08 05:15:14  robertj
 * Fixed problem in Windows requiring snmpapi.dll for PEthSocket class.
 *
 * Revision 1.23  1998/08/27 01:13:20  robertj
 * Changes to resolve signedness in GNU C library v6
 * Remove Linux EthSocket stuff from Sun build, still needs implementing.
 *
 * Revision 1.22  1998/08/21 05:30:59  robertj
 * Ethernet socket implementation.
 *
 */

#pragma implementation "sockets.h"
#pragma implementation "socket.h"
#pragma implementation "ipsock.h"
#pragma implementation "udpsock.h"
#pragma implementation "tcpsock.h"
#pragma implementation "ipdsock.h"
#pragma implementation "ethsock.h"


#include <ptlib.h>
#include <sockets.h>


extern PSemaphore PX_iostreamMutex;

PSocket::~PSocket()
{
  os_close();
}

int PSocket::os_close()
{
  if (os_handle < 0)
    return -1;

  // send a shutdown to the other end
  ::shutdown(os_handle, 2);

  return PXClose();
}

int PSocket::os_socket(int af, int type, int protocol)
{
  // attempt to create a socket
  int handle;
  if ((handle = ::socket(af, type, protocol)) >= 0) {

    // make the socket non-blocking and close on exec
#ifndef P_PTHREADS
    DWORD cmd = 1;
#endif
    if (
#ifndef P_PTHREADS
        !ConvertOSError(::ioctl(handle, FIONBIO, &cmd)) ||
#endif
        !ConvertOSError(::fcntl(handle, F_SETFD, 1))) {
      ::close(handle);
      return -1;
    }
//PError << "socket " << handle << " created" << endl;
  }
  return handle;
}

int PSocket::os_connect(struct sockaddr * addr, PINDEX size)
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

  if (val == 0) {
    errno = ECONNREFUSED;
    return -1;
  }

  // A successful select() call does not necessarily mean the socket connected OK.
  int optval = -1;
  int optlen = sizeof(optval);
  getsockopt(os_handle, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen);
  if (optval == 0)
    return 0;

  errno = optval;
  return -1;
}


int PSocket::os_accept(int sock, struct sockaddr * addr, PINDEX * size,
                       const PTimeInterval & timeout)
{
  int new_fd;
  if (!PXSetIOBlock(PXAcceptBlock, sock, timeout)) {
    errno = EINTR;
    return -1;
  }

  while (1) {
    new_fd = ::accept(sock, addr, (socklen_t *)size);
    if ((new_fd >= 0) || (errno != EPROTO))
      return new_fd;
    //PError << "accept on " << sock << " failed with EPROTO - retrying" << endl;
  }
}

int PSocket::os_select(int maxHandle,
                   fd_set & readBits,
                   fd_set & writeBits,
                   fd_set & exceptionBits,
#ifndef P_PTHREADS
          const PIntArray & osHandles,
#else
          const PIntArray & ,
#endif

      const PTimeInterval & timeout)
{
  struct timeval * tptr = NULL;

#ifndef P_PTHREADS
  int stat = PThread::Current()->PXBlockOnIO(maxHandle,
                                         readBits,
                                         writeBits,
                                         exceptionBits,
                                         timeout,
					 osHandles);
  if (stat <= 0)
    return stat;

  struct timeval tout = {0, 0};
  tptr = &tout;
#else
  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) {
    if (timeout.GetMilliSeconds() < 1000L*60L*60L*24L) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }
#endif

  return ::select(maxHandle, &readBits, &writeBits, &exceptionBits, tptr);
}
                     

PIPSocket::Address::Address(DWORD dw)
{
  s_addr = dw;
}


PIPSocket::Address & PIPSocket::Address::operator=(DWORD dw)
{
  s_addr = dw;
  return *this;
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
  if (addr == (DWORD)-1)
    return FALSE;

  if (!GetHostAddress(hostname, addr))
    return FALSE;

  PUDPSocket sock;

  // get number of interfaces
  int ifNum;
#ifdef SIOCGIFNUM
  PAssert(::ioctl(sock.GetHandle(), SIOCGIFNUM, &ifNum) >= 0, "could not do ioctl for ifNum");
#else
  ifNum = 100;
#endif

  PBYTEArray buffer;
  struct ifconf ifConf;
  ifConf.ifc_len  = ifNum * sizeof(ifreq);
  ifConf.ifc_req = (struct ifreq *)buffer.GetPointer(ifConf.ifc_len);
  
  if (ioctl(sock.GetHandle(), SIOCGIFCONF, &ifConf) >= 0) {
#ifndef SIOCGIFNUM
    ifNum = ifConf.ifc_len / sizeof(ifreq);
#endif

    int num = 0;
    for (num = 0; num < ifNum; num++) {

      ifreq * ifName = ifConf.ifc_req + num;
      struct ifreq ifReq;
      strcpy(ifReq.ifr_name, ifName->ifr_name);

      if (ioctl(sock.GetHandle(), SIOCGIFFLAGS, &ifReq) >= 0) {
        int flags = ifReq.ifr_flags;
        if (ioctl(sock.GetHandle(), SIOCGIFADDR, &ifReq) >= 0) {
          if ((flags & IFF_UP) && (addr == Address(((sockaddr_in *)&ifReq.ifr_addr)->sin_addr)))
            return TRUE;
        }
      }
    }
  }
  return FALSE;
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
  char buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);

    // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount = ::read(os_handle, (char *)buf, maxLen)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


BOOL PSocket::os_recvfrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      int    flags,
      sockaddr * addr, // Address from which the datagram was received.
      PINDEX * addrlen)
{
  if (!PXSetIOBlock(PXReadBlock, readTimeout)) {
    lastError     = Timeout;
    lastReadCount = 0;
    return FALSE;
  }

  // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount =
        ::recvfrom(os_handle, (char *)buf, len, flags, (sockaddr *)addr, (socklen_t *)addrlen)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


BOOL PSocket::os_sendto(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      int flags,
      sockaddr * addr, // Address to which the datagram is sent.
      PINDEX addrlen)  
{
  if (!IsOpen()) {
    lastError     = NotOpen;
    lastWriteCount = 0;
    return FALSE;
  }

  if (!PXSetIOBlock(PXWriteBlock, writeTimeout)) {
    lastError     = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

  // attempt to read data
  if (ConvertOSError(lastWriteCount =
         ::sendto(os_handle, (char *)buf, len, flags, (sockaddr *)addr, addrlen)))
    return lastWriteCount >= len;

  lastWriteCount = 0;
  return FALSE;
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  if (os_handle < 0) {
    lastError = NotOpen;
    return FALSE;
  }

  if (!PXSetIOBlock(PXReadBlock, readTimeout)) 
    return FALSE;

  if (ConvertOSError(lastReadCount = ::recv(os_handle, (char *)buf, len, 0)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}



//////////////////////////////////////////////////////////////////
//
//  PEthSocket
//

PEthSocket::PEthSocket(PINDEX, PINDEX)
{
  medium = MediumUnknown;
  filterMask = FilterDirected|FilterBroadcast;
  filterType = TypeAll;
  fakeMacHeader = FALSE;
  ipppInterface = FALSE;
}


PEthSocket::~PEthSocket()
{
  Close();
}


BOOL PEthSocket::Connect(const PString & interfaceName)
{
  Close();

  fakeMacHeader = FALSE;
  ipppInterface = FALSE;

  if (strncmp("eth", interfaceName, 3) == 0)
    medium = Medium802_3;
  else if (strncmp("lo", interfaceName, 2) == 0)
    medium = MediumLoop;
  else if (strncmp("sl", interfaceName, 2) == 0) {
    medium = MediumWan;
    fakeMacHeader = TRUE;
  }
  else if (strncmp("ppp", interfaceName, 3) == 0) {
    medium = MediumWan;
    fakeMacHeader = TRUE;
  }
  else if (strncmp("ippp", interfaceName, 4) == 0) {
    medium = MediumWan;
    ipppInterface = TRUE;
  }
  else {
    lastError = NotFound;
    osError = ENOENT;
    return FALSE;
  }

#ifdef SIOCGIFHWADDR
  PUDPSocket ifsock;
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strcpy(ifr.ifr_name, interfaceName);
  if (!ConvertOSError(ioctl(ifsock.GetHandle(), SIOCGIFHWADDR, &ifr)))
    return FALSE;

  memcpy(&macAddress, ifr.ifr_hwaddr.sa_data, sizeof(macAddress));
#endif

  channelName = interfaceName;
  return OpenSocket();
}


BOOL PEthSocket::OpenSocket()
{
#ifdef SOCK_PACKET
  if (!ConvertOSError(os_handle = os_socket(AF_INET, SOCK_PACKET, htons(filterType))))
    return FALSE;

  struct sockaddr addr;
  memset(&addr, 0, sizeof(addr));
  addr.sa_family = AF_INET;
  strcpy(addr.sa_data, channelName);
  if (!ConvertOSError(bind(os_handle, &addr, sizeof(addr)))) {
    os_close();
    os_handle = -1;
    return FALSE;
  }
#endif

  return TRUE;
}


BOOL PEthSocket::Close()
{
  SetFilter(FilterDirected, filterType);  // Turn off promiscuous mode
  return PSocket::Close();
}


BOOL PEthSocket::EnumInterfaces(PINDEX idx, PString & name)
{
  PUDPSocket ifsock;

  ifreq ifreqs[20]; // Maximum of 20 interfaces
  ifconf ifc;
  ifc.ifc_len = sizeof(ifreqs);
  ifc.ifc_buf = (caddr_t)ifreqs;
  if (!ConvertOSError(ioctl(ifsock.GetHandle(), SIOCGIFCONF, &ifc)))
    return FALSE;

  int ifcount = ifc.ifc_len/sizeof(ifreq);
  int ifidx;
  for (ifidx = 0; ifidx < ifcount; ifidx++) {
    if (strchr(ifreqs[ifidx].ifr_name, ':') == NULL) {
      ifreq ifr;
      strcpy(ifr.ifr_name, ifreqs[ifidx].ifr_name);
      if (ioctl(ifsock.GetHandle(), SIOCGIFFLAGS, &ifr) == 0 &&
          (ifr.ifr_flags & IFF_UP) != 0 &&
           idx-- == 0) {
        name = ifreqs[ifidx].ifr_name;
        return TRUE;
      }
    }
  }

  return FALSE;

}


PString PEthSocket::GetGatewayInterface() const
{
  PTextFile procfile;
  if (!procfile.Open("/proc/net/route", PFile::ReadOnly))
    return PString();

  char iface[20];
  long net_addr, dest_addr, net_mask;
  int flags, refcnt, use, metric;

  do {
    // Ignore heading line or remainder of route line
    procfile.ignore(1000, '\n');
    procfile >> iface >> ::hex >> net_addr >> dest_addr >> flags 
                      >> ::dec >> refcnt >> use >> metric 
                      >> ::hex >> net_mask;
    if (procfile.fail())
      return PString();
  } while (net_addr != 0 || net_mask != 0);

  return iface;
}


BOOL PEthSocket::GetAddress(Address & addr)
{
  if (!IsOpen())
    return FALSE;

  addr = macAddress;
  return TRUE;
}


BOOL PEthSocket::EnumIpAddress(PINDEX idx,
                               PIPSocket::Address & addr,
                               PIPSocket::Address & net_mask)
{
  if (!IsOpen())
    return FALSE;

  PUDPSocket ifsock;
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  if (idx == 0)
    strcpy(ifr.ifr_name, channelName);
  else
    sprintf(ifr.ifr_name, "%s:%u", (const char *)channelName, idx-1);
  if (!ConvertOSError(ioctl(os_handle, SIOCGIFADDR, &ifr)))
    return FALSE;

  sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
  addr = sin->sin_addr;

  if (!ConvertOSError(ioctl(os_handle, SIOCGIFNETMASK, &ifr)))
    return FALSE;

  net_mask = sin->sin_addr;
  return TRUE;
}


BOOL PEthSocket::GetFilter(unsigned & mask, WORD & type)
{
  if (!IsOpen())
    return 0;

  ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, channelName);
  if (!ConvertOSError(ioctl(os_handle, SIOCGIFFLAGS, &ifr)))
    return FALSE;

  if ((ifr.ifr_flags&IFF_PROMISC) != 0)
    filterMask |= FilterPromiscuous;
  else
    filterMask &= ~FilterPromiscuous;

  mask = filterMask;
  type = filterType;
  return TRUE;
}


BOOL PEthSocket::SetFilter(unsigned filter, WORD type)
{
  if (!IsOpen())
    return FALSE;

  if (filterType != type) {
    os_close();
    filterType = type;
    if (!OpenSocket())
      return FALSE;
  }

  ifreq ifr;
  memset(&ifr, 0, sizeof(ifr));
  strcpy(ifr.ifr_name, channelName);
  if (!ConvertOSError(ioctl(os_handle, SIOCGIFFLAGS, &ifr)))
    return FALSE;

  if ((filter&FilterPromiscuous) != 0)
    ifr.ifr_flags |= IFF_PROMISC;
  else
    ifr.ifr_flags &= ~IFF_PROMISC;

  if (!ConvertOSError(ioctl(os_handle, SIOCSIFFLAGS, &ifr)))
    return FALSE;

  filterMask = filter;

  return TRUE;
}


PEthSocket::MediumTypes PEthSocket::GetMedium()
{
  return medium;
}


BOOL PEthSocket::ResetAdaptor()
{
  // No implementation
  return TRUE;
}


BOOL PEthSocket::Read(void * buf, PINDEX len)
{
  static const BYTE macHeader[] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0, 0, 0, 0, 0, 0, 8, 0 };

  BYTE * bufptr = (BYTE *)buf;

  if (fakeMacHeader) {
    if (len <= sizeof(macHeader)) {
      memcpy(bufptr, macHeader, len);
      lastReadCount = len;
      return TRUE;
    }

    memcpy(bufptr, macHeader, sizeof(macHeader));
    bufptr += sizeof(macHeader);
    len -= sizeof(macHeader);
  }

  for (;;) {
    sockaddr from;
    PINDEX fromlen = sizeof(from);
    if (!os_recvfrom(bufptr, len, 0, &from, &fromlen))
      return FALSE;

    if (channelName != from.sa_data)
      continue;

    if (ipppInterface) {
      if (lastReadCount <= 10)
        return FALSE;
      if (memcmp(bufptr+6, "\xff\x03\x00\x21", 4) != 0) {
        memmove(bufptr+sizeof(macHeader), bufptr, lastReadCount);
        lastReadCount += sizeof(macHeader);
      }
      else {
        memmove(bufptr+sizeof(macHeader), bufptr+10, lastReadCount);
        lastReadCount += sizeof(macHeader)-10;
      }
      memcpy(bufptr, macHeader, sizeof(macHeader));
      break;
    }

    if (fakeMacHeader) {
      lastReadCount += sizeof(macHeader);
      break;
    }

    if ((filterMask&FilterPromiscuous) != 0)
      break;

    if ((filterMask&FilterDirected) != 0 && macAddress == bufptr)
      break;

    static const Address broadcast;
    if ((filterMask&FilterBroadcast) != 0 && broadcast == bufptr)
      break;
  }

  return lastReadCount > 0;
}


BOOL PEthSocket::Write(const void * buf, PINDEX len)
{
  sockaddr to;
  strcpy(to.sa_data, channelName);
  return os_sendto(buf, len, 0, &to, sizeof(to));
}


#include "ptlib/src/pethsock.cxx"


///////////////////////////////////////////////////////////////////////////////

