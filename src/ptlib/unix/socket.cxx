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
 * $Revision$
 * $Author$
 * $Date$
 */

#pragma implementation "sockets.h"
#pragma implementation "socket.h"
#pragma implementation "ipsock.h"
#pragma implementation "udpsock.h"
#pragma implementation "tcpsock.h"
#pragma implementation "ipdsock.h"
#pragma implementation "ethsock.h"
#pragma implementation "qos.h"

#include <ptlib.h>

#include <ptbuildopts.h>
#include <ptlib/sockets.h>

#include <map>
#include <ptlib/pstring.h>

#if defined(SIOCGENADDR)
#define SIO_Get_MAC_Address SIOCGENADDR
#define  ifr_macaddr         ifr_ifru.ifru_enaddr
#elif defined(SIOCGIFHWADDR)
#define SIO_Get_MAC_Address SIOCGIFHWADDR
#define  ifr_macaddr         ifr_hwaddr.sa_data
#endif

#if defined(P_LINUX) && P_HAS_RECVMSG && P_HAS_RECVMSG_MSG_ERRQUEUE
  #include "asm/types.h"
  #include "linux/errqueue.h"
#endif

#if defined(P_LINUX)
#include <sys/types.h>
#include <sys/socket.h>
#include <ifaddrs.h>
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_SOLARIS) || defined(P_MACOSX) || defined(P_MACOS) || defined(P_IRIX) || defined(P_VXWORKS) || defined(P_RTEMS) || defined(P_QNX)
#define ifr_netmask ifr_addr

#include <net/if_dl.h>

#if !defined(P_IPHONEOS)
#include <net/if_types.h>
#include <net/route.h>
#endif

#include <netinet/in.h>
#if !defined(P_QNX)  && !defined(P_IPHONEOS)
#include <netinet/if_ether.h>
#endif

#if defined(P_FREEBSD) || defined(P_NETBSD) || defined(P_OPENBSD) || defined(P_MACOSX) || defined(P_SOLARIS)
#include <ifaddrs.h>
#endif

#define ROUNDUP(a) \
        ((a) > 0 ? (1 + (((a) - 1) | (sizeof(long) - 1))) : sizeof(long))

#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_MACOSX) || defined(P_MACOS) || defined(P_QNX)
#include <sys/sysctl.h>
#endif

#ifdef P_RTEMS
#include <bsp.h>
#endif

#ifdef P_BEOS
#include <posix/sys/ioctl.h> // for FIONBIO
#include <be/bone/net/if.h> // for ifconf
#include <be/bone/sys/sockio.h> // for SIOCGI*
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_MACOSX) || defined(P_VXWORKS) || defined(P_RTEMS) || defined(P_QNX)
// Define _SIZEOF_IFREQ for platforms (eg OpenBSD) which do not have it.
#ifndef _SIZEOF_ADDR_IFREQ
#define _SIZEOF_ADDR_IFREQ(ifr) \
  ((ifr).ifr_addr.sa_len > sizeof(struct sockaddr) ? \
  (sizeof(struct ifreq) - sizeof(struct sockaddr) + \
  (ifr).ifr_addr.sa_len) : sizeof(struct ifreq))
#endif
#endif

int PX_NewHandle(const char *, int);

#ifdef P_VXWORKS
// VxWorks variant of inet_ntoa() allocates INET_ADDR_LEN bytes via malloc
// BUT DOES NOT FREE IT !!!  Use inet_ntoa_b() instead.
#define INET_ADDR_LEN      18
extern "C" void inet_ntoa_b(struct in_addr inetAddress, char *pString);
#endif // P_VXWORKS

//////////////////////////////////////////////////////////////////////////////
// P_fd_set

void P_fd_set::Construct()
{
  max_fd = PProcess::Current().GetMaxHandles();
  set = (fd_set *)malloc((max_fd+7)>>3);
}


void P_fd_set::Zero()
{
  if (PAssertNULL(set) != NULL)
    memset(set, 0, (max_fd+7)>>3);
}


//////////////////////////////////////////////////////////////////////////////

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


static int SetNonBlocking(int fd)
{
  if (fd < 0)
    return -1;

  // Set non-blocking so we can use select calls to break I/O block on close
  int cmd = 1;
#if defined(P_VXWORKS)
  if (::ioctl(fd, FIONBIO, &cmd) == 0)
#else
  if (::ioctl(fd, FIONBIO, &cmd) == 0 && ::fcntl(fd, F_SETFD, 1) == 0)
#endif
    return fd;

  ::close(fd);
  return -1;
}


int PSocket::os_socket(int af, int type, int protocol)
{
  int fd = ::socket(af, type, protocol);

#if P_HAS_RECVMSG_IP_RECVERR
  if ((fd != -1) && (type == SOCK_DGRAM)) {
    int v = 1;
    setsockopt(fd, IPPROTO_IP, IP_RECVERR, &v, sizeof(v));
  }
#endif

  // attempt to create a socket
  return SetNonBlocking(PX_NewHandle(GetClass(), fd));
}


PBoolean PSocket::os_connect(struct sockaddr * addr, socklen_t size)
{
  int val;
  do {
    val = ::connect(os_handle, addr, size);
  } while (val != 0 && errno == EINTR);
  if (val == 0 || errno != EINPROGRESS)
    return ConvertOSError(val);

  if (!PXSetIOBlock(PXConnectBlock, readTimeout))
    return false;

  // A successful select() call does not necessarily mean the socket connected OK.
  int optval = -1;
  socklen_t optlen = sizeof(optval);
  getsockopt(os_handle, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen);
  if (optval != 0) {
    errno = optval;
    return ConvertOSError(-1);
  }

  return true;
}


PBoolean PSocket::os_accept(PSocket & listener, struct sockaddr * addr, socklen_t * size)
{
  int new_fd;
  while ((new_fd = ::accept(listener.GetHandle(), addr, (socklen_t *)size)) < 0) {
    switch (errno) {
      case EINTR :
        break;

#if defined(E_PROTO)
      case EPROTO :
        PTRACE(3, "PTLib\tAccept on " << listener << " failed with EPROTO - retrying");
        break;
#endif

      case EWOULDBLOCK :
        if (listener.GetReadTimeout() > 0) {
          if (listener.PXSetIOBlock(PXAcceptBlock, listener.GetReadTimeout()))
            break;
          return SetErrorValues(listener.GetErrorCode(), listener.GetErrorNumber());
        }
        // Next case

      default :
        return ConvertOSError(-1, LastReadError);
    }
  }

  return ConvertOSError(os_handle = SetNonBlocking(new_fd));
}


#if !defined(P_PTHREADS) && !defined(P_MAC_MPTHREADS) && !defined(P_BEOS)

PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except,
      const PTimeInterval & timeout)
{
  PINDEX i, j;
  PINDEX nextfd = 0;
  int maxfds = 0;
  Errors lastError = NoError;
  PThread * unblockThread = PThread::Current();
  
  P_fd_set fds[3];
  SelectList * list[3] = { &read, &write, &except };

  for (i = 0; i < 3; i++) {
    for (j = 0; j < list[i]->GetSize(); j++) {
      PSocket & socket = (*list[i])[j];
      if (!socket.IsOpen())
        lastError = NotOpen;
      else {
        int h = socket.GetHandle();
        fds[i] += h;
        if (h > maxfds)
          maxfds = h;
      }
      socket.px_selectMutex[i].Wait();
      socket.px_selectThread[i] = unblockThread;
    }
  }

  if (lastError == NoError) {
    P_timeval tval = timeout;
    int result = ::select(maxfds+1, 
                          (fd_set *)fds[0], 
                          (fd_set *)fds[1], 
                          (fd_set *)fds[2], 
                          tval);

    int osError;
    (void)ConvertOSError(result, lastError, osError);
  }

  for (i = 0; i < 3; i++) {
    for (j = 0; j < list[i]->GetSize(); j++) {
      PSocket & socket = (*list[i])[j];
      socket.px_selectThread[i] = NULL;
      socket.px_selectMutex[i].Signal();
      if (lastError == NoError) {
        int h = socket.GetHandle();
        if (h < 0)
          lastError = Interrupted;
        else if (!fds[i].IsPresent(h))
          list[i]->RemoveAt(j--);
      }
    }
  }

  return lastError;
}
                     
#else

PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except,
                                 const PTimeInterval & timeout)
{
  PINDEX i, j;
  int maxfds = 0;
  Errors lastError = NoError;
  PThread * unblockThread = PThread::Current();
  int unblockPipe = unblockThread->unblockPipe[0];

  P_fd_set fds[3];
  SelectList * list[3] = { &read, &write, &except };

  for (i = 0; i < 3; i++) {
    for (j = 0; j < list[i]->GetSize(); j++) {
      PSocket & socket = (*list[i])[j];
      if (!socket.IsOpen())
        lastError = NotOpen;
      else {
        int h = socket.GetHandle();
        fds[i] += h;
        if (h > maxfds)
          maxfds = h;
      }
      socket.px_selectMutex[i].Wait();
      socket.px_selectThread[i] = unblockThread;
    }
  }

  int result = -1;
  if (lastError == NoError) {
    fds[0] += unblockPipe;
    if (unblockPipe > maxfds)
      maxfds = unblockPipe;

    P_timeval tval = timeout;
    do {
      result = ::select(maxfds+1, (fd_set *)fds[0], (fd_set *)fds[1], (fd_set *)fds[2], tval);
    } while (result < 0 && errno == EINTR);

    int osError;
    if (ConvertOSError(result, lastError, osError)) {
      if (fds[0].IsPresent(unblockPipe)) {
        PTRACE2(6, NULL, "PTLib\tSelect unblocked fd=" << unblockPipe);
        BYTE ch;
        if (ConvertOSError(::read(unblockPipe, &ch, 1), lastError, osError))
          lastError = Interrupted;
      }
    }
  }

  for (i = 0; i < 3; i++) {
    SelectList::iterator it = list[i]->begin();
    while (it != list[i]->end()) {
      PSocket & socket = *it;
      socket.px_selectThread[i] = NULL;
      socket.px_selectMutex[i].Signal();
      if (lastError != NoError)
        ++it;
      else {
        int h = socket.GetHandle();
        if (h < 0) {
          lastError = Interrupted;
          ++it;
        }
        else if (fds[i].IsPresent(h))
          ++it;
        else
          list[i]->erase(it++);
      }
    }
  }

  return lastError;
}

#endif


#if P_HAS_RECVMSG

#if P_HAS_RECVMSG_MSG_ERRQUEUE
  #include "linux/errqueue.h"
  static void SetErrorFromQueue(SOCKET handle, PChannel::Errors & errorCode, int & errorNumber)
  {
    msghdr errorData;
    memset(&errorData, 0, sizeof(errorData));

    char control_data[50];
    errorData.msg_control    = control_data;
    errorData.msg_controllen = sizeof(control_data);

    if (::recvmsg(handle, &errorData, MSG_ERRQUEUE) < 0)
      return;

    for (struct cmsghdr *cmsg = CMSG_FIRSTHDR(&errorData); cmsg != NULL; cmsg = CMSG_NXTHDR(&errorData, cmsg)) {
      if (cmsg->cmsg_level == SOL_IP && cmsg->cmsg_type == IP_RECVERR) {
        struct sock_extended_err * sock_error = (struct sock_extended_err *)CMSG_DATA(cmsg);
        PTRACE_IF(4, sock_error->ee_origin == SO_EE_ORIGIN_ICMP,
                  "PTLib\tICMP error from " << PIPSocketAddressAndPort(SO_EE_OFFENDER(sock_error), sizeof(sockaddr)));
        errno = sock_error->ee_errno;
        PChannel::ConvertOSError(-1, errorCode, errorNumber);
      }
    }
  }
#else
  #define SetErrorFromQueue(handle, errorCode, errorNumber)
#endif

bool PSocket::os_vread(Slice * slices, size_t sliceCount, int flags, struct sockaddr * addr, socklen_t * addrlen)
{
  lastReadCount = 0;

  do {
    msghdr readData;
    memset(&readData, 0, sizeof(readData));

    readData.msg_name       = addr;
    readData.msg_namelen    = *addrlen;

    readData.msg_iov        = slices;
    readData.msg_iovlen     = sliceCount;

    // read a packet 
    int result = ::recvmsg(os_handle, &readData, flags);
    if (ConvertOSError(result, LastReadError)) {
      lastReadCount = result;
      if ((readData.msg_flags&MSG_TRUNC) == 0)
        return lastReadCount > 0;

      PTRACE(4, "PTlib\tTruncated packet read, returning EMSGSIZE");
      SetErrorValues(BufferTooSmall, EMSGSIZE, LastReadError);
      return false;
    }

    if (lastErrorNumber[LastReadError] != EWOULDBLOCK) {
      SetErrorFromQueue(os_handle, lastErrorCode[LastReadError], lastErrorNumber[LastReadError]);
      return false;
    }
  } while (PXSetIOBlock(PXReadBlock, readTimeout));

  return false;
}


bool PSocket::os_vwrite(const Slice * slices, size_t sliceCount, int flags, struct sockaddr * addr, socklen_t addrLen)
{
  lastWriteCount = 0;

  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF, LastWriteError);

  do {
    msghdr writeData;
    memset(&writeData, 0, sizeof(writeData));

    writeData.msg_name    = addr;
    writeData.msg_namelen = addrLen;

    writeData.msg_iov     = const_cast<Slice *>(slices);
    writeData.msg_iovlen  = sliceCount;

    // write the packet 
    int result = ::sendmsg(os_handle, &writeData, flags);
    if (ConvertOSError(result, LastWriteError)) {
      lastWriteCount = result;
      return true;
    }

    if (lastErrorNumber[LastWriteError] != EWOULDBLOCK) {
      SetErrorFromQueue(os_handle, lastErrorCode[LastWriteError], lastErrorNumber[LastWriteError]);
      return false;
    }
  } while (PXSetIOBlock(PXWriteBlock, writeTimeout));

  return false;
}

#else // P_RECVMSG

bool PSocket::os_vread(Slice * slices, size_t sliceCount, int flags, struct sockaddr * addr, socklen_t * addrlen)
{
  #error Implementation of PSocket::os_vread not available
  return false;
}

bool PSocket::os_vwrite(const Slice * slices, size_t sliceCount, int flags, struct sockaddr * addr, socklen_t addrLen)
{
  #error Implementation of PSocket::os_vwrite not available
  return false;
}

#endif // P_RECVMSG

PIPSocket::Address::Address(DWORD dw)
{
  operator=(dw);
}


PIPSocket::Address & PIPSocket::Address::operator=(DWORD dw)
{
  m_version = 4;
  m_v.m_four.s_addr = dw;
  return *this;
}


PIPSocket::Address::operator DWORD() const
{
  return m_version != 4 ? 0 : (DWORD)m_v.m_four.s_addr;
}

BYTE PIPSocket::Address::Byte1() const
{
  return *(((BYTE *)&m_v.m_four.s_addr)+0);
}

BYTE PIPSocket::Address::Byte2() const
{
  return *(((BYTE *)&m_v.m_four.s_addr)+1);
}

BYTE PIPSocket::Address::Byte3() const
{
  return *(((BYTE *)&m_v.m_four.s_addr)+2);
}

BYTE PIPSocket::Address::Byte4() const
{
  return *(((BYTE *)&m_v.m_four.s_addr)+3);
}

PIPSocket::Address::Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4)
{
  m_version = 4;
  BYTE * p = (BYTE *)&m_v.m_four.s_addr;
  p[0] = b1;
  p[1] = b2;
  p[2] = b3;
  p[3] = b4;
}

PBoolean PIPSocket::IsLocalHost(const PString & hostname)
{
  if (hostname.IsEmpty())
    return true;

  if (hostname *= "localhost")
    return true;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address addr = hostname;
  if (addr.IsLoopback())  // Is 127.0.0.1
    return true;
  if (!addr.IsValid())
    return false;

  if (!GetHostAddress(hostname, addr))
    return false;

#if P_HAS_IPV6
  {
    FILE * file;
    int dummy;
    int addr6[16];
    char ifaceName[255];
    PBoolean found = false;
    if ((file = fopen("/proc/net/if_inet6", "r")) != NULL) {
      while (!found && (fscanf(file, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %x %x %x %x %255s\n",
              &addr6[0],  &addr6[1],  &addr6[2],  &addr6[3], 
              &addr6[4],  &addr6[5],  &addr6[6],  &addr6[7], 
              &addr6[8],  &addr6[9],  &addr6[10], &addr6[11], 
              &addr6[12], &addr6[13], &addr6[14], &addr6[15], 
             &dummy, &dummy, &dummy, &dummy, ifaceName) != EOF)) {
        Address ip6addr(
          psprintf("%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
              addr6[0],  addr6[1],  addr6[2],  addr6[3], 
              addr6[4],  addr6[5],  addr6[6],  addr6[7], 
              addr6[8],  addr6[9],  addr6[10], addr6[11], 
              addr6[12], addr6[13], addr6[14], addr6[15]
          )
        );
        found = (ip6addr *= addr);
      }
      fclose(file);
    }
    if (found)
      return true;
  }
#endif

  // check IPV4 addresses
  PUDPSocket sock;
  
  PBYTEArray buffer;
  struct ifconf ifConf;

#if defined(P_NETBSD)
  struct ifaddrs *ifap, *ifa;

  PAssert(getifaddrs(&ifap) == 0, "getifaddrs failed");
  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
#else
#ifdef SIOCGIFNUM
  int ifNum;
  PAssert(::ioctl(sock.GetHandle(), SIOCGIFNUM, &ifNum) >= 0, "could not do ioctl for ifNum");
  ifConf.ifc_len = ifNum * sizeof(ifreq);
#else
  ifConf.ifc_len = 100 * sizeof(ifreq); // That's a LOT of interfaces!
#endif

  ifConf.ifc_req = (struct ifreq *)buffer.GetPointer(ifConf.ifc_len);
  
  if (ioctl(sock.GetHandle(), SIOCGIFCONF, &ifConf) >= 0) {
    void * ifEndList = (char *)ifConf.ifc_req + ifConf.ifc_len;
    ifreq * ifName = ifConf.ifc_req;

    while (ifName < ifEndList) {
#endif
      struct ifreq ifReq;
#if !defined(P_NETBSD)
      memcpy(&ifReq, ifName, sizeof(ifreq));
#else
      memset(&ifReq, 0, sizeof(ifReq));
      strncpy(ifReq.ifr_name, ifa->ifa_name, sizeof(ifReq.ifr_name) - 1);
#endif
      
      if (ioctl(sock.GetHandle(), SIOCGIFFLAGS, &ifReq) >= 0) {
        int flags = ifReq.ifr_flags;
        if ((flags & IFF_UP) && ioctl(sock.GetHandle(), SIOCGIFADDR, &ifReq) >= 0) {
          sockaddr_in * sin = (sockaddr_in *)&ifReq.ifr_addr;
          PIPSocket::Address address = sin->sin_addr;
          if (addr *= address)
            return true;
        }
      }
      
#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_MACOSX) || defined(P_VXWORKS) || defined(P_RTEMS) || defined(P_QNX)
      // move the ifName pointer along to the next ifreq entry
      ifName = (struct ifreq *)((char *)ifName + _SIZEOF_ADDR_IFREQ(*ifName));
#elif !defined(P_NETBSD)
      ifName++;
#endif
    }
#if !defined(P_NETBSD)
  }
#endif
  
  return false;
}


////////////////////////////////////////////////////////////////
//
//  PTCPSocket
//
PBoolean PTCPSocket::Read(void * buf, PINDEX maxLen)

{
  lastReadCount = 0;

  // wait until select indicates there is data to read, or until
  // a timeout occurs
  if (!PXSetIOBlock(PXReadBlock, readTimeout))
    return false;

  // attempt to read out of band data
  char buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);

  // attempt to read non-out of band data
  int r = ::recv(os_handle, (char *)buf, maxLen, 0);
  if (!ConvertOSError(r, LastReadError))
    return false;

  lastReadCount = r;
  return lastReadCount > 0;
}


PBoolean PSocket::Read(void * buf, PINDEX len)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF, LastReadError);

  if (!PXSetIOBlock(PXReadBlock, readTimeout)) 
    return false;

  int lastReadCount = ::recv(os_handle, (char *)buf, len, 0);
    return lastReadCount > 0;
  if (ConvertOSError(lastReadCount))
    return lastReadCount > 0;

  lastReadCount = 0;
  return false;
}

bool PSocket::Write(const void * buf, PINDEX len)
{
  return PChannel::Write(buf, len);
}

PBoolean PSocket::Read(Slice * slices, size_t sliceCount)
{
  flush();
  lastReadCount = 0;

  if (sliceCount == 0)
    return SetErrorValues(BadParameter, EINVAL, LastReadError);

  os_vread(slices, sliceCount, 0, NULL, NULL);
  return lastReadCount > 0;
}


PBoolean PSocket::Write(const Slice * slices, size_t sliceCount)
{
  flush();
  return os_vwrite(slices, sliceCount, 0, NULL, 0) && lastWriteCount >= 0;
}


///////////////////////////////////////////////////////////////////////////////

PBoolean PIPSocket::GetGatewayAddress(Address & addr, int version)
{
  RouteTable table;
  if (GetRouteTable(table)) {
    for (PINDEX i = 0; i < table.GetSize(); i++) {
      if ((table[i].GetNetwork() == 0)
          && (table[i].GetDestination().GetVersion() == (unsigned)version)) {
        addr = table[i].GetDestination();
        return true;
      }
    }
  }
  return false;
}



PString PIPSocket::GetGatewayInterface(int version)
{
  RouteTable table;
  if (GetRouteTable(table)) {
    for (PINDEX i = 0; i < table.GetSize(); i++) {
      if (table[i].GetNetwork() == 0)
        return table[i].GetInterface();
    }
  }
  return PString();
}


bool PIPSocket::SetQoS(const QoS & qos)
{
  m_qos = qos;

  static int const DSCP[NumQoSType] = {
    0,     // BackgroundQoS
    0,     // BestEffortQoS
    8<<2,  // ExcellentEffortQoS
    10<<2, // CriticalQoS
    38<<2, // VideoQoS
    44<<2, // VoiceQoS
    46<<2  // ControlQoS
  };
  if (SetOption(IP_TOS, qos.m_dscp < 0 || qos.m_dscp > 63 ? DSCP[qos.m_type] : qos.m_dscp, IPPROTO_IP))
    reutrn true;

  PTRACE(1, "Socket\tCould not set TOS field in IP header: " << GetErrorText());
  return false;
}


// bit setting inspired by Tim Ring on StackOverflow
const unsigned char QuickByteMask[8] = { 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x02, 0x01 };
void ResetBit(unsigned bit, BYTE *bitmap)
{
    unsigned x = bit / 8;                // Index to byte.
    unsigned n = bit % 8;                // Specific bit in byte.
    bitmap[x] &= (~QuickByteMask[n]);  // Reset bit.
}

PIPSocket::Address NetmaskV6WithPrefix(unsigned prefixbits, unsigned masklen = 0, BYTE * mask = NULL)
{
  BYTE fullmask[16] = { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff };

  if (mask) {
    memset(&fullmask, 0, sizeof(fullmask));
    memcpy(&fullmask, mask, std::min((size_t)masklen, sizeof(fullmask)));
  }
  for(unsigned i=128; i >= prefixbits; --i) {
    ResetBit(i, fullmask);
  }
  return PIPSocket::Address(16, (BYTE*)&fullmask);
}

#if defined(P_LINUX) || defined (P_AIX)

PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
  table.RemoveAll();

  PString strLine;
  PTextFile procfile;

  if (procfile.Open("/proc/net/route", PFile::ReadOnly) && procfile.ReadLine(strLine)) {
    // Ignore heading line above
    while (procfile.ReadLine(strLine)) {
      char iface[20];
      uint32_t net_addr, dest_addr, net_mask;
      int flags, refcnt, use, metric;
      if (sscanf(strLine, "%s%x%x%x%u%u%u%x",
                 iface, &net_addr, &dest_addr, &flags, &refcnt, &use, &metric, &net_mask) == 8) {
        RouteEntry * entry = new RouteEntry(net_addr);
        entry->net_mask = net_mask;
        entry->destination = dest_addr;
        entry->interfaceName = iface;
        entry->metric = metric;
        table.Append(entry);
      }
    }
  }

#if P_HAS_IPV6
  if (procfile.Open("/proc/net/ipv6_route", PFile::ReadOnly)) {
    while (procfile.ReadLine(strLine)) {
      PStringArray tokens = strLine.Tokenise(" \t", false);
      if (tokens.GetSize() == 10) {
        // 0 = dest_addr
        // 1 = net_mask (cidr in hex)
        // 2 = src_addr
        // 4 = next_hop
        // 5 = metric
        // 6 = refcnt
        // 7 = use
        // 8 = flags
        // 9 = device name

        BYTE net_addr[16];
        for (size_t i = 0; i < sizeof(net_addr); ++i)
          net_addr[i] = tokens[0].Mid(i*2, 2).AsUnsigned(16);

        BYTE dest_addr[16];
        for (size_t i = 0; i < sizeof(dest_addr); ++i)
          dest_addr[i] = tokens[4].Mid(i*2, 2).AsUnsigned(16);

        RouteEntry * entry = new RouteEntry(Address(sizeof(net_addr), net_addr));
        entry->destination = Address(sizeof(dest_addr), dest_addr);
        entry->interfaceName = tokens[9];
        entry->metric = tokens[5].AsUnsigned(16);
		BYTE net_mask[16];
		bzero(net_mask, sizeof(net_mask));
		for(size_t i = 0; i < tokens[1].AsUnsigned(16) / 4; ++i)
			net_mask[i/2] = (i % 2 == 0) ? 0xf0 : 0xff;
        entry->net_mask = Address(sizeof(net_mask), net_mask);
        table.Append(entry);
      }
    }
  }
#endif

  return !table.IsEmpty();
}

#elif (defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_MACOSX) || defined(P_QNX)) && !defined(P_IPHONEOS)

PBoolean process_rtentry(struct rt_msghdr *rtm, char *ptr, PIPSocket::Address & net_addr,
                     PIPSocket::Address & net_mask, PIPSocket::Address & dest_addr, int & metric);
PBoolean get_ifname(int index, char *name);

PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
  int mib[6];
  size_t space_needed;
  char *limit, *buf, *ptr;
  struct rt_msghdr *rtm;

  InterfaceTable if_table;

  // Read the Routing Table
  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = 0;
  mib[4] = NET_RT_DUMP;
  mib[5] = 0;

  if (sysctl(mib, 6, NULL, &space_needed, NULL, 0) < 0) {
    printf("sysctl: net.route.0.0.dump estimate");
    return false;
  }

  if ((buf = (char *)malloc(space_needed)) == NULL) {
    printf("malloc(%lu)", (unsigned long)space_needed);
    return false;
  }

  // read the routing table data
  if (sysctl(mib, 6, buf, &space_needed, NULL, 0) < 0) {
    printf("sysctl: net.route.0.0.dump");
    free(buf);
    return false;
  }

  // Read the interface table
  if (!GetInterfaceTable(if_table)) {
    printf("Interface Table Invalid\n");
    return false;
  }

  // Process the Routing Table data
  limit = buf + space_needed;
  for (ptr = buf; ptr < limit; ptr += rtm->rtm_msglen) {

    PIPSocket::Address net_addr, dest_addr, net_mask;
    int metric;
    char name[16];

    rtm = (struct rt_msghdr *)ptr;

    if ( process_rtentry(rtm, ptr, net_addr, net_mask, dest_addr, metric) ){

      RouteEntry * entry = new RouteEntry(net_addr);
      entry->net_mask = net_mask;
      entry->destination = dest_addr;
      if ( get_ifname(rtm->rtm_index,name) )
        entry->interfaceName = name;
      entry->metric = metric;
      table.Append(entry);

    } // end if

  } // end for loop

  free(buf);
  return true;
}

PBoolean process_rtentry(struct rt_msghdr *rtm, char *ptr, PIPSocket::Address & net_addr,
                     PIPSocket::Address & net_mask, PIPSocket::Address & dest_addr, int & metric) {

  struct sockaddr_in *sa_in = (struct sockaddr_in *)(rtm + 1);

  // Check for zero length entry
  if (rtm->rtm_msglen == 0) {
    printf("zero length message\n");
    return false;
  }

  if ((~rtm->rtm_flags & RTF_LLINFO)
#if defined(P_NETBSD) || defined(P_QNX)
        && (~rtm->rtm_flags&RTF_CLONED)     // Net BSD has flag one way
#elif !defined(P_OPENBSD) && !defined(P_FREEBSD)
        && (~rtm->rtm_flags&RTF_WASCLONED)  // MAC has it another
#else
                                            // Open/Free BSD does not have it at all!
#endif
     ) {

	metric=0;

    // NET_ADDR
    if(rtm->rtm_addrs & RTA_DST) {
      if(sa_in->sin_family == AF_INET)
        net_addr = PIPSocket::Address(AF_INET, sizeof(sockaddr_in), (struct sockaddr *)sa_in);
      if(sa_in->sin_family == AF_INET6)
        net_addr = PIPSocket::Address(AF_INET6, sizeof(sockaddr_in6), (struct sockaddr *)sa_in);

      sa_in = (struct sockaddr_in *)((char *)sa_in + ROUNDUP(sa_in->sin_len));
    }

    // DEST_ADDR
    if(rtm->rtm_addrs & RTA_GATEWAY) {
      if(sa_in->sin_family == AF_INET)
        dest_addr = PIPSocket::Address(AF_INET, sizeof(sockaddr_in), (struct sockaddr *)sa_in);
      if(sa_in->sin_family == AF_INET6)
        dest_addr = PIPSocket::Address(AF_INET6, sizeof(sockaddr_in6), (struct sockaddr *)sa_in);

      sa_in = (struct sockaddr_in *)((char *)sa_in + ROUNDUP(sa_in->sin_len));
    }

    // NETMASK
    if(rtm->rtm_addrs & RTA_NETMASK) {
      unsigned char *ptr = (unsigned char *)&((sockaddr*)sa_in)->sa_data[2];
      if (sa_in->sin_len == 0) {
        net_mask = (net_addr.GetVersion() == 4) ? "0.0.0.0" : "::";
      } else if (sa_in->sin_len == 5) {
        net_mask =  PString(PString::Printf, "%d.0.0.0", *ptr);
      } else if (sa_in->sin_len == 6) {
        net_mask = PString(PString::Printf, "%d.%d.0.0", *ptr, *(ptr+1));
      } else if (sa_in->sin_len == 7) {
        net_mask = PString(PString::Printf, "%d.%d.%d.0", *ptr, *(ptr+1), *(ptr+2));
      } else if (sa_in->sin_len == 8) {
        net_mask = PString(PString::Printf, "%d.%d.%d.%d", *ptr, *(ptr+1), *(ptr+2), *(ptr+3));
      } else if (sa_in->sin_len > 8) {
        net_mask = NetmaskV6WithPrefix((sa_in->sin_len - 8) * 8, (sa_in->sin_len - 8), ptr+4);
      }

      sa_in = (struct sockaddr_in *)((char *)sa_in + ROUNDUP(sa_in->sin_len));
    }

    if(rtm->rtm_addrs & RTA_IFP) {
      //const char *ptr = (const char *)&((sockaddr_dl*)sa_in)->sdl_data[0];
      //PTRACE(5, "RTA_IFP addr=" << net_addr << " name=" << PString(ptr));
      sa_in = (struct sockaddr_in *)((char *)sa_in + ROUNDUP(sa_in->sin_len));
    }

    if(rtm->rtm_addrs & RTA_IFA) {
      if (dest_addr.IsLoopback()) {
        if(sa_in->sin_family == AF_INET)
          dest_addr = PIPSocket::Address(AF_INET, sizeof(sockaddr_in), (struct sockaddr *)sa_in);
        if(sa_in->sin_family == AF_INET6)
          dest_addr = PIPSocket::Address(AF_INET6, sizeof(sockaddr_in6), (struct sockaddr *)sa_in);
      }

      sa_in = (struct sockaddr_in *)((char *)sa_in + ROUNDUP(sa_in->sin_len));
    }

    if(rtm->rtm_flags & RTF_HOST) {
      if(net_addr.GetVersion() == 4)
        net_mask = 0xffffffff;
      if(net_addr.GetVersion() == 6)
        net_mask = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff";
    }

    return true;

  } else {
    return false;
  }
}

PBoolean get_ifname(int index, char *name) {
  int mib[6];
  size_t needed;
  char *lim, *buf, *next;
  struct if_msghdr *ifm;
  struct  sockaddr_dl *sdl;

  mib[0] = CTL_NET;
  mib[1] = PF_ROUTE;
  mib[2] = 0;
  mib[3] = AF_INET;
  mib[4] = NET_RT_IFLIST;
  mib[5] = index;

  if (sysctl(mib, 6, NULL, &needed, NULL, 0) < 0) {
    printf("ERR route-sysctl-estimate");
    return false;
  }

  if ((buf = (char *)malloc(needed)) == NULL) {
    printf("ERR malloc");
    return false;
  }

  if (sysctl(mib, 6, buf, &needed, NULL, 0) < 0) {
    printf("ERR actual retrieval of routing table");
    free(buf);
    return false;
  }

  lim = buf + needed;

  next = buf;
  if (next < lim) {

    ifm = (struct if_msghdr *)next;

    if (ifm->ifm_type == RTM_IFINFO) {
      sdl = (struct sockaddr_dl *)(ifm + 1);
    } else {
      printf("out of sync parsing NET_RT_IFLIST\n");
      return false;
    }
    next += ifm->ifm_msglen;

    strncpy(name, sdl->sdl_data, sdl->sdl_nlen);
    name[sdl->sdl_nlen] = '\0';

    free(buf);
    return true;

  } else {
    free(buf);
    return false;
  }

}


#elif defined(P_SOLARIS)

/* jpd@louisiana.edu - influenced by Merit.edu's Gated 3.6 routine: krt_rtread_sunos5.c */

#include <sys/stream.h>
#include <stropts.h>
#include <sys/tihdr.h>
#include <sys/tiuser.h>
#include <inet/common.h>
#include <inet/mib2.h>
#include <inet/ip.h>

#ifndef T_CURRENT
#define T_CURRENT       MI_T_CURRENT
#endif

PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
#define task_pagesize 512
    char buf[task_pagesize];  /* = task_block_malloc(task_pagesize);*/
    int flags;
    int j = 0;
    int  sd, i, rc;
    struct strbuf strbuf;
    struct T_optmgmt_req *tor = (struct T_optmgmt_req *) buf;
    struct T_optmgmt_ack *toa = (struct T_optmgmt_ack *) buf;
    struct T_error_ack  *tea = (struct T_error_ack *) buf;
    struct opthdr *req;

    sd = open("/dev/ip", O_RDWR);
    if (sd < 0) {
#ifdef SOL_COMPLAIN
      perror("can't open mib stream");
#endif
      goto Return;
    }

    strbuf.buf = buf;

    tor->PRIM_type = T_OPTMGMT_REQ;
    tor->OPT_offset = sizeof(struct T_optmgmt_req);
    tor->OPT_length = sizeof(struct opthdr);
    tor->MGMT_flags = T_CURRENT;
    req = (struct opthdr *) (tor + 1);
    req->level = MIB2_IP;    /* any MIB2_xxx value ok here */
    req->name = 0;
    req->len = 0;

    strbuf.len = tor->OPT_length + tor->OPT_offset;
    flags = 0;
    rc = putmsg(sd, &strbuf, (struct strbuf *) 0, flags);
    if (rc == -1) {
#ifdef SOL_COMPLAIN
      perror("putmsg(ctl)");
#endif
      goto Return;
    }
    /*
     * each reply consists of a ctl part for one fixed structure
     * or table, as defined in mib2.h.  The format is a T_OPTMGMT_ACK,
     * containing an opthdr structure.  level/name identify the entry,
     * len is the size of the data part of the message.
     */
    req = (struct opthdr *) (toa + 1);
    strbuf.maxlen = task_pagesize;
    while (++j) {
  flags = 0;
  rc = getmsg(sd, &strbuf, (struct strbuf *) 0, &flags);
  if (rc == -1) {
#ifdef SOL_COMPLAIN
    perror("getmsg(ctl)");
#endif
    goto Return;
  }
  if (rc == 0
      && strbuf.len >= (int)sizeof(struct T_optmgmt_ack)
      && toa->PRIM_type == T_OPTMGMT_ACK
      && toa->MGMT_flags == T_SUCCESS
      && req->len == 0) {
    errno = 0;    /* just to be darned sure it's 0 */
    goto Return;    /* this is EOD msg */
  }

  if (strbuf.len >= (int)sizeof(struct T_error_ack)
      && tea->PRIM_type == T_ERROR_ACK) {
      errno = (tea->TLI_error == TSYSERR) ? tea->UNIX_error : EPROTO;
#ifdef SOL_COMPLAIN
      perror("T_ERROR_ACK in mibget");
#endif
      goto Return;
  }
      
  if (rc != MOREDATA
      || strbuf.len < (int)sizeof(struct T_optmgmt_ack)
      || toa->PRIM_type != T_OPTMGMT_ACK
      || toa->MGMT_flags != T_SUCCESS) {
      errno = ENOMSG;
      goto Return;
  }

  if ( (req->level != MIB2_IP
#if P_SOLARIS > 7
        || req->name != MIB2_IP_ROUTE)
      && (req->level != MIB2_IP6 || req->name != MIB2_IP6_ROUTE)
#endif
           ) {  /* == 21 */
      /* If this is not the routing table, skip it */
      strbuf.maxlen = task_pagesize;
      do {
        rc = getmsg(sd, (struct strbuf *) 0, &strbuf, &flags);
      } while (rc == MOREDATA) ;
      continue;
  }

  strbuf.maxlen = (task_pagesize / sizeof (mib2_ipRouteEntry_t)) * sizeof (mib2_ipRouteEntry_t);
  strbuf.len = 0;
  flags = 0;
  do {
      rc = getmsg(sd, (struct strbuf *) 0, &strbuf, &flags);
      
      switch (rc) {
      case -1:
#ifdef SOL_COMPLAIN
        perror("mibget getmsg(data) failed.");
#endif
        goto Return;

      default:
#ifdef SOL_COMPLAIN
        fprintf(stderr,"mibget getmsg(data) returned %d, strbuf.maxlen = %d, strbuf.len = %d",
            rc,
            strbuf.maxlen,
            strbuf.len);
#endif
        goto Return;

      case MOREDATA:
      case 0:
        {
          mib2_ipRouteEntry_t *rp = (mib2_ipRouteEntry_t *) strbuf.buf;
          mib2_ipRouteEntry_t *lp = (mib2_ipRouteEntry_t *) (strbuf.buf + strbuf.len);

          do {
            char name[256];
            name[0] = '\0';
            if (req->level == 0) {
              if (rp->ipRouteInfo.re_ire_type & (IRE_BROADCAST|IRE_CACHE|IRE_LOCAL)) {
                ++rp;
                continue;
              }
              RouteEntry * entry = new RouteEntry(rp->ipRouteDest);
              entry->net_mask = rp->ipRouteMask;
              entry->destination = rp->ipRouteNextHop;
              unsigned len = rp->ipRouteIfIndex.o_length;
              if (len >= sizeof(name))
                len = sizeof(name)-1;
              strncpy(name, rp->ipRouteIfIndex.o_bytes, len);
              name[len] = '\0';
              entry->interfaceName = name;
              entry->metric = rp->ipRouteMetric1;
              table.Append(entry);
              ++rp;
            } else {
              mib2_ipv6RouteEntry_t *rp6 = (mib2_ipv6RouteEntry_t *) rp;
              if (rp6->ipv6RouteInfo.re_ire_type & (IRE_BROADCAST|IRE_CACHE|IRE_LOCAL)) {
                rp = (mib2_ipRouteEntry_t *) ((BYTE*)rp + sizeof(mib2_ipv6RouteEntry_t));
                continue;
              }
              RouteEntry * entry = new RouteEntry(Address(16, (BYTE*)&rp6->ipv6RouteDest));
              entry->net_mask = NetmaskV6WithPrefix(rp6->ipv6RoutePfxLength);
              entry->destination = Address(16, (BYTE*)&rp6->ipv6RouteNextHop);
              unsigned len = rp6->ipv6RouteIfIndex.o_length;
              if (len >= sizeof(name))
                len = sizeof(name)-1;
              strncpy(name, rp6->ipv6RouteIfIndex.o_bytes, len);
              name[len] = '\0';
              entry->interfaceName = name;
              entry->metric = rp6->ipv6RouteMetric;
              table.Append(entry);
              rp = (mib2_ipRouteEntry_t *) ((BYTE*)rp + sizeof(mib2_ipv6RouteEntry_t));
            }
          } while (rp < lp) ;

          }
          break;
        }
      } while (rc == MOREDATA) ;
    }

 Return:
    i = errno;
    (void) close(sd);
    errno = i;
    /*task_block_reclaim(task_pagesize, buf);*/
    if (errno)
      return (false);
    else
      return (true);
}


#elif defined(P_VXWORKS)

PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
  PAssertAlways("PIPSocket::GetRouteTable()");
  for(;;){
    char iface[20];
    unsigned long net_addr, dest_addr, net_mask;
    int  metric;
    RouteEntry * entry = new RouteEntry(net_addr);
    entry->net_mask = net_mask;
    entry->destination = dest_addr;
    entry->interfaceName = iface;
    entry->metric = metric;
    table.Append(entry);
    return true;
  }
}

#else // unsupported platform

#if 0 
PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
        // Most of this code came from the source code for the "route" command 
        // so it should work on other platforms too. 
        // However, it is not complete (the "address-for-interface" function doesn't exist) and not tested! 
        
        route_table_req_t reqtable; 
        route_req_t *rrtp; 
        int i,ret; 
        
        ret = get_route_table(&reqtable); 
        if (ret < 0) 
        { 
                return false; 
        } 
        
        for (i=reqtable.cnt, rrtp = reqtable.rrtp;i>0;i--, rrtp++) 
        { 
                //the datalink doesn't save addresses/masks for host and default 
                //routes, so the route_req_t may not be filled out completely 
                if (rrtp->flags & RTF_DEFAULT) { 
                        //the IP default route is 0/0 
                        ((struct sockaddr_in *)&rrtp->dst)->sin_addr.s_addr = 0; 
                        ((struct sockaddr_in *)&rrtp->mask)->sin_addr.s_addr = 0; 
        
                } else if (rrtp->flags & RTF_HOST) { 
                        //host routes are addr/32 
                        ((struct sockaddr_in *)&rrtp->mask)->sin_addr.s_addr = 0xffffffff; 
                } 
        
            RouteEntry * entry = new RouteEntry(/* address_for_interface(rrtp->iface) */); 
            entry->net_mask = rrtp->mask; 
            entry->destination = rrtp->dst; 
            entry->interfaceName = rrtp->iface; 
            entry->metric = rrtp->refcnt; 
            table.Append(entry); 
        } 
        
        free(reqtable.rrtp); 
                
        return true; 
#endif // 0

PBoolean PIPSocket::GetRouteTable(RouteTable & table)
{
#warning Platform requires implemetation of GetRouteTable()
  return false;
}
#endif



#ifdef P_HAS_NETLINK

#include <asm/types.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <linux/genetlink.h>

#include <memory.h>
#include <errno.h>

class NetLinkRouteTableDetector : public PIPSocket::RouteTableDetector
{
  public:
    int m_fdLink;
    int m_fdCancel[2];

    NetLinkRouteTableDetector()
    {
      m_fdLink = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);

      if (m_fdLink != -1) {
        struct sockaddr_nl sanl;
        memset(&sanl, 0, sizeof(sanl));
        sanl.nl_family = AF_NETLINK;
        sanl.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;

        bind(m_fdLink, (struct sockaddr *)&sanl, sizeof(sanl));
      }

      if (pipe(m_fdCancel) == -1)
        m_fdCancel[0] = m_fdCancel[1] = -1;

      PTRACE(3, "PTLIB\tOpened NetLink socket");
    }

    ~NetLinkRouteTableDetector()
    {
      if (m_fdLink != -1)
        close(m_fdLink);
      if (m_fdCancel[0] != -1)
        close(m_fdCancel[0]);
      if (m_fdCancel[1] != -1)
        close(m_fdCancel[1]);
    }

    bool Wait(const PTimeInterval & timeout)
    {
      if (m_fdCancel[0] == -1)
        return false;

      bool ok = true;
      while (ok) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(m_fdCancel[0], &fds);

        struct timeval tval;
        struct timeval * ptval = NULL;
        if (m_fdLink != -1) {
          tval.tv_sec  = timeout.GetMilliSeconds() / 1000;
          tval.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
          ptval = &tval;

          FD_SET(m_fdLink, &fds);
        }

        int result = select(std::max(m_fdLink, m_fdCancel[0])+1, &fds, NULL, NULL, ptval);
        if (result < 0)
          return false;
        if (result == 0)
          return true;

        if (FD_ISSET(m_fdCancel[0], &fds))
          return false;

        struct sockaddr_nl snl;
        char buf[4096];
        struct iovec iov = { buf, sizeof buf };
        struct msghdr msg = { (void*)&snl, sizeof snl, &iov, 1, NULL, 0, 0};

        int status = recvmsg(m_fdLink, &msg, 0);
        if (status < 0)
          return false;

        for (struct nlmsghdr * nlmsg = (struct nlmsghdr *)buf;
             NLMSG_OK(nlmsg, (unsigned)status);
             nlmsg = NLMSG_NEXT(nlmsg, status)) {
          if (nlmsg->nlmsg_len < sizeof(struct nlmsghdr))
            break;

          switch (nlmsg->nlmsg_type) {
            case RTM_NEWADDR :
            case RTM_DELADDR :
              PTRACE(3, "PTLIB\tInterface table change detected via NetLink");
              return true;
          }
        }
      }
      return false;
    }

    void Cancel()
    {
      PAssert(write(m_fdCancel[1], "", 1) == 1, POperatingSystemError);
    }
};

PIPSocket::RouteTableDetector * PIPSocket::CreateRouteTableDetector()
{
  return new NetLinkRouteTableDetector();
}

#elif defined(P_IPHONEOS)

#include <netdb.h>
#include <sys/time.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <SystemConfiguration/SystemConfiguration.h>
#include <SystemConfiguration/SCNetworkReachability.h>

#define kSCNetworkReachabilityOptionNodeName	CFSTR("nodename")

/*!
	@constant kSCNetworkReachabilityOptionServName
	@discussion A CFString that will be passed to getaddrinfo(3).  An acceptable
		value is either a decimal port number or a service name listed in
		services(5).
 */
#define kSCNetworkReachabilityOptionServName	CFSTR("servname")

/*!
	@constant kSCNetworkReachabilityOptionHints
	@discussion A CFData wrapping a "struct addrinfo" that will be passed to
		getaddrinfo(3).  The caller can supply any of the ai_family,
		ai_socktype, ai_protocol, and ai_flags structure elements.  All
		other elements must be 0 or the null pointer.
 */
#define kSCNetworkReachabilityOptionHints	CFSTR("hints")

class ReachabilityRouteTableDetector : public PIPSocket::RouteTableDetector
{
	SCNetworkReachabilityRef	target_async;

   public:
	SCNetworkReachabilityRef _setupReachability(SCNetworkReachabilityContext *context)
	{
		struct sockaddr_in		sin;
		struct sockaddr_in6		sin6;
		SCNetworkReachabilityRef	target	= NULL;

		bzero(&sin, sizeof(sin));
		sin.sin_len    = sizeof(sin);
		sin.sin_family = AF_INET;

		bzero(&sin6, sizeof(sin6));
		sin6.sin6_len    = sizeof(sin6);
		sin6.sin6_family = AF_INET6;

		const char *anchor = "apple.com";

		if (inet_aton(anchor, &sin.sin_addr) == 1) {

			target = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&sin);
			
		} else if (inet_pton(AF_INET6, anchor, &sin6.sin6_addr) == 1) {
			char	*p;

			p = strchr(anchor, '%');
			if (p != NULL) {
				sin6.sin6_scope_id = if_nametoindex(p + 1);
			}

			target = SCNetworkReachabilityCreateWithAddress(NULL, (struct sockaddr *)&sin6);
			
		} else {
			
		target = SCNetworkReachabilityCreateWithName(NULL, anchor);
				
#if	!TARGET_OS_IPHONE
		if (CFDictionaryGetCount(options) > 0) {

			target = SCNetworkReachabilityCreateWithOptions(NULL, options);

		} else {

			SCPrint(TRUE, stderr, CFSTR("Must specify nodename or servname\n"));
			return NULL;
		}
		CFRelease(options);
#endif			
		}

		return target;
	}
	  
	static void callout(SCNetworkReachabilityRef target, SCNetworkReachabilityFlags flags, void *info)
	{
		struct tm	tm_now;
		struct timeval	tv_now;

		(void)gettimeofday(&tv_now, NULL);
		(void)localtime_r(&tv_now.tv_sec, &tm_now);

		PTRACE(1, psprintf("Reachability changed at: %2d:%02d:%02d.%03d, now it is %sreachable",
			tm_now.tm_hour,
			tm_now.tm_min,
			tm_now.tm_sec,
			tv_now.tv_usec / 1000,
			flags & kSCNetworkReachabilityFlagsReachable? "" : "not ")
			);

		ReachabilityRouteTableDetector* d = (ReachabilityRouteTableDetector*) info;	
		d->Cancel();
	}

	ReachabilityRouteTableDetector()
		: RouteTableDetector(),
		target_async(NULL)
	{
		SCNetworkReachabilityContext	context	= { 0, NULL, NULL, NULL, NULL };
		
		target_async = _setupReachability(&context);
		if (target_async == NULL) {
			PTRACE(1, psprintf("  Could not determine status: %s\n", SCErrorString(SCError())));
			return;
		}
		
		context.info = (void*) this;
		
		if (!SCNetworkReachabilitySetCallback(target_async, ReachabilityRouteTableDetector::callout, &context)) {
			PTRACE(1, psprintf("SCNetworkReachabilitySetCallback() failed: %s\n", SCErrorString(SCError())));
			return;
		}

		if (!SCNetworkReachabilityScheduleWithRunLoop(target_async, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode)) {
			PTRACE(1, psprintf("SCNetworkReachabilityScheduleWithRunLoop() failed: %s\n", SCErrorString(SCError()) ) );
			return;
		}
	}
   
	~ReachabilityRouteTableDetector()
	{
		if(target_async != NULL)
			CFRelease(target_async);
	}

    bool Wait(const PTimeInterval & timeout)
    {
      return !m_cancel.Wait(timeout);
    }

    void Cancel()
    {
      m_cancel.Signal();
    }

  private:
    PSyncPoint m_cancel;
	PBoolean m_continue;
};

PIPSocket::RouteTableDetector * PIPSocket::CreateRouteTableDetector()
{
	return new ReachabilityRouteTableDetector();
}

#else // P_HAS_NETLINK, elif defined(P_IPHONEOS)

class DummyRouteTableDetector : public PIPSocket::RouteTableDetector
{
  public:
    bool Wait(const PTimeInterval & timeout)
    {
      return !m_cancel.Wait(timeout);
    }

    void Cancel()
    {
      m_cancel.Signal();
    }

  private:
    PSyncPoint m_cancel;
};


PIPSocket::RouteTableDetector * PIPSocket::CreateRouteTableDetector()
{
  return new DummyRouteTableDetector();
}

#endif // P_HAS_NETLINK, elif defined(P_IPHONEOS)


PBoolean PIPSocket::GetInterfaceTable(InterfaceTable & list, PBoolean includeDown)
{
#if defined(P_LINUX) || defined(P_FREEBSD) || defined (P_NETBSD) || defined(P_OPENBSD) || defined(P_MACOSX) || defined(P_SOLARIS)
  // tested on Linux 2.6.x, FreeBSD 8.2, NetBSD 5.1, OpenBSD 5.0, MacOS X 10.5.6 and Solaris 11
  struct ifaddrs *interfaces, *ifa;

  if (getifaddrs(&interfaces) == 0) {
    for (ifa = interfaces; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == NULL) continue;
      if ((ifa->ifa_flags & IFF_UP) == 0) continue;
      PString macAddr;
#if defined(SIO_Get_MAC_Address) 
      PUDPSocket ifsock;
      struct ifreq ifReq;
      memset(&ifReq, 0, sizeof(ifReq));
      ifReq.ifr_addr.sa_family = ifa->ifa_addr->sa_family;
      strncpy(ifReq.ifr_name, ifa->ifa_name, sizeof(ifReq.ifr_name) - 1);
      if (ioctl(ifsock.GetHandle(), SIO_Get_MAC_Address, &ifReq) == 0) {
        macAddr = PEthSocket::Address((BYTE *)ifReq.ifr_macaddr);
      }
#endif
      if (ifa->ifa_addr->sa_family == AF_INET)
        list.Append(PNEW InterfaceEntry(ifa->ifa_name,
                                        Address(AF_INET, sizeof(sockaddr_in), ifa->ifa_addr),
                                        Address(AF_INET, sizeof(sockaddr_in), ifa->ifa_netmask),
                                        macAddr));
#ifdef P_HAS_IPV6
      else if (ifa->ifa_addr->sa_family == AF_INET6)
        list.Append(PNEW InterfaceEntry(ifa->ifa_name,
                                        Address(AF_INET6, sizeof(sockaddr_in6), ifa->ifa_addr),
                                        Address(AF_INET6, sizeof(sockaddr_in6), ifa->ifa_netmask),
                                        macAddr));
#endif
    }
    freeifaddrs(interfaces);
  }
#else	// not P_FREEBSD...
  PUDPSocket sock;

  PBYTEArray buffer;
  struct ifconf ifConf;
  
#if defined(P_NETBSD)
  struct ifaddrs *ifap, *ifa;

  PAssert(getifaddrs(&ifap) == 0, "getifaddrs failed");

  for (ifa = ifap; ifa; ifa = ifa->ifa_next) {
#else
  // HERE
#if defined(SIOCGIFNUM)
  int ifNum;
  PAssert(::ioctl(sock.GetHandle(), SIOCGIFNUM, &ifNum) >= 0, "could not do ioctl for ifNum");
  ifConf.ifc_len = ifNum * sizeof(ifreq);
#else
  ifConf.ifc_len = 100 * sizeof(ifreq); // That's a LOT of interfaces!
#endif

  ifConf.ifc_req = (struct ifreq *)buffer.GetPointer(ifConf.ifc_len);

  if (ioctl(sock.GetHandle(), SIOCGIFCONF, &ifConf) >= 0) {
    void * ifEndList = (char *)ifConf.ifc_req + ifConf.ifc_len;
    ifreq * ifName = ifConf.ifc_req;
    while (ifName < ifEndList) {
#endif
      struct ifreq ifReq;
#if !defined(P_NETBSD)
          memcpy(&ifReq, ifName, sizeof(ifreq));
#else
          memset(&ifReq, 0, sizeof(ifReq));
          strncpy(ifReq.ifr_name, ifa->ifa_name, sizeof(ifReq.ifr_name) - 1);
#endif

      if (ioctl(sock.GetHandle(), SIOCGIFFLAGS, &ifReq) >= 0) {
        int flags = ifReq.ifr_flags;
        if (includeDown || (flags & IFF_UP) != 0) {
          PString name(ifReq.ifr_name);

          PString macAddr;
#if defined(SIO_Get_MAC_Address)
          memcpy(&ifReq, ifName, sizeof(ifreq));
          if (ioctl(sock.GetHandle(), SIO_Get_MAC_Address, &ifReq) >= 0)
            macAddr = PEthSocket::Address((BYTE *)ifReq.ifr_macaddr);
#endif

#if !defined(P_NETBSD)
          memcpy(&ifReq, ifName, sizeof(ifreq));
#else
          memset(&ifReq, 0, sizeof(ifReq));
          strncpy(ifReq.ifr_name, ifa->ifa_name, sizeof(ifReq.ifr_name) - 1);
#endif

          if (ioctl(sock.GetHandle(), SIOCGIFADDR, &ifReq) >= 0) {

            sockaddr_in * sin = (sockaddr_in *)&ifReq.ifr_addr;
            PIPSocket::Address addr = sin->sin_addr;

#if !defined(P_NETBSD)
            memcpy(&ifReq, ifName, sizeof(ifreq));
#else
            memset(&ifReq, 0, sizeof(ifReq));
            strncpy(ifReq.ifr_name, ifa->ifa_name, sizeof(ifReq.ifr_name) - 1);
#endif

            if (ioctl(sock.GetHandle(), SIOCGIFNETMASK, &ifReq) >= 0) {
              PIPSocket::Address mask = 
#ifndef P_BEOS
    ((sockaddr_in *)&ifReq.ifr_netmask)->sin_addr;
#else
    ((sockaddr_in *)&ifReq.ifr_mask)->sin_addr;
#endif // !P_BEOS
              PINDEX i;
              for (i = 0; i < list.GetSize(); i++) {
#ifdef P_TORNADO
                if (list[i].GetName() == name &&
                    list[i].GetAddress() == addr)
                    if(list[i].GetNetMask() == mask)
#else
                if (list[i].GetName() == name &&
                    list[i].GetAddress() == addr &&
                    list[i].GetNetMask() == mask)
#endif
                  break;
              }
              if (i >= list.GetSize())
                list.Append(PNEW InterfaceEntry(name, addr, mask, macAddr));
            }
          }
        }
      }

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_MACOSX) || defined(P_VXWORKS) || defined(P_RTEMS) || defined(P_QNX)
      // move the ifName pointer along to the next ifreq entry
      ifName = (struct ifreq *)((char *)ifName + _SIZEOF_ADDR_IFREQ(*ifName));
#elif !defined(P_NETBSD)
      ifName++;
#endif

    }
#if !defined(P_NETBSD)
  }
#endif

#if P_HAS_IPV6
  // build a table of IPV6 interface addresses
  // fe800000000000000202e3fffe1ee330 02 40 20 80     eth0
  // 00000000000000000000000000000001 01 80 10 80       lo
  FILE * file;
  int dummy;
  int addr[16];
  int scope;
  char ifaceName[255];
  if ((file = fopen("/proc/net/if_inet6", "r")) != NULL) {
    while (fscanf(file, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x %x %x %x %x %255s\n",
            &addr[0],  &addr[1],  &addr[2],  &addr[3], 
            &addr[4],  &addr[5],  &addr[6],  &addr[7], 
            &addr[8],  &addr[9],  &addr[10], &addr[11], 
            &addr[12], &addr[13], &addr[14], &addr[15], 
           &dummy, &dummy, &scope, &dummy, ifaceName) != EOF) {
      BYTE bytes[16];
      for (PINDEX i = 0; i < 16; i++)
        bytes[i] = addr[i];

      PString macAddr;

#if defined(SIO_Get_MAC_Address)
      struct ifreq ifReq;
      memset(&ifReq, 0, sizeof(ifReq));
      strncpy(ifReq.ifr_name, ifaceName, sizeof(ifReq.ifr_name) - 1);
      if (ioctl(sock.GetHandle(), SIO_Get_MAC_Address, &ifReq) >= 0)
        macAddr = PEthSocket::Address((BYTE *)ifReq.ifr_macaddr);
#endif

      // the scope value in /proc/net/if_inet6 does not give the scope value for the interface
      // so, we need to use the hack of assuming that if_nametoindex returns the correct scope
      // for link-local interfaces. if anyone knows how to do this better, contact craigs@postincrement.com
      scope = if_nametoindex(ifaceName);
      list.Append(PNEW InterfaceEntry(ifaceName, Address(16, bytes, scope), Address::GetAny(6), macAddr));
    }
    fclose(file);
  }
#endif
#endif

  return true;
}

#ifdef P_VXWORKS

int h_errno;

struct hostent * Vx_gethostbyname(char *name, struct hostent *hp)
{
  u_long addr;
  static char staticgethostname[100];

  hp->h_aliases = NULL;
  hp->h_addr_list[1] = NULL;
  if ((int)(addr = inet_addr(name)) != ERROR) {
    memcpy(staticgethostname, &addr, sizeof(addr));
    hp->h_addr_list[0] = staticgethostname;
    h_errno = SUCCESS;
    return hp;
  }
  memcpy(staticgethostname, &addr, sizeof (addr));
  hp->h_addr_list[0] = staticgethostname;
  h_errno = SUCCESS;
  return hp;
}

struct hostent * Vx_gethostbyaddr(char *name, struct hostent *hp)
{
  u_long addr;
  static char staticgethostaddr[100];

  hp->h_aliases = NULL;
  hp->h_addr_list = NULL;

  if ((int)(addr = inet_addr(name)) != ERROR) {
    char ipStorage[INET_ADDR_LEN];
    inet_ntoa_b(*(struct in_addr*)&addr, ipStorage);
    sprintf(staticgethostaddr,"%s",ipStorage);
    hp->h_name = staticgethostaddr;
    h_errno = SUCCESS;
  }
  else
  {
    printf ("_gethostbyaddr: not able to get %s\n",name);
    h_errno = NOTFOUND;
  }
  return hp;
}

#endif // P_VXWORKS


#include "../common/pethsock.cxx"

//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

#if P_QOS

void PUDPSocket::EnableGQoS()
{
}

PBoolean PUDPSocket::SupportQoS(const PIPSocket::Address & )
{
  return false;
}

#endif

///////////////////////////////////////////////////////////////////////////////

