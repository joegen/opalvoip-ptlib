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
 * Revision 1.70  2001/09/10 03:03:36  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.69  2001/08/16 11:58:22  rogerh
 * Add more Mac OS X changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.68  2001/08/12 07:12:40  rogerh
 * More Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.67  2001/08/12 06:34:33  rogerh
 * Add Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
 *
 * Revision 1.66  2001/08/07 02:27:22  robertj
 * Fixed some incorrect error values returned in Read() and Write() functions.
 *
 * Revision 1.65  2001/07/03 04:41:25  yurik
 * Corrections to Jac's submission from 6/28
 *
 * Revision 1.64  2001/06/30 06:59:07  yurik
 * Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
 *
 * Revision 1.63  2001/06/19 12:09:13  rogerh
 * Mac OS X change
 *
 * Revision 1.62  2001/04/16 22:46:22  craigs
 * Fixed problem with os_connect not correctly reporting errors
 *
 * Revision 1.61  2001/03/26 03:31:53  robertj
 * Fixed Solaris compile error.
 *
 * Revision 1.60  2001/03/20 06:44:25  robertj
 * Lots of changes to fix the problems with terminating threads that are I/O
 *   blocked, especially when doing orderly shutdown of service via SIGTERM.
 *
 * Revision 1.59  2001/03/07 23:37:59  robertj
 * Fixed slow down in UDP packet send, thanks Dmitriy Reka
 *
 * Revision 1.58  2001/03/07 06:56:36  yurik
 * Made adjustment for BONE platforms as requested by Jac Goudsmit
 *
 * Revision 1.57  2001/03/06 00:16:59  robertj
 * Fixed BSD compatibility problem.
 *
 * Revision 1.56  2001/03/05 04:28:56  robertj
 * Added net mask to interface info returned by GetInterfaceTable()
 *
 * Revision 1.55  2001/02/02 23:31:30  robertj
 * Fixed enumeration of interfaces, thanks Bertrand Croq.
 *
 * Revision 1.54  2001/01/17 03:48:25  rogerh
 * Fix GetInterfaceTable so it actually works through all interfaces rather
 * than falling over after the first entry.
 *
 * Revision 1.53  2001/01/16 12:56:01  rogerh
 * On BeOS sa_data is 'unsigned char *'. Linux and BSD defines sa_data as 'char *'
 * Add typecast, submitted by Jac Goudsmit <jac_goudsmit@yahoo.com>
 *
 * Revision 1.52  2000/06/21 01:01:22  robertj
 * AIX port, thanks Wolfgang Platzer (wolfgang.platzer@infonova.at).
 *
 * Revision 1.51  2000/04/19 00:13:53  robertj
 * BeOS port changes.
 *
 * Revision 1.50  2000/04/07 05:43:48  rogerh
 * Fix a compilation error in a non-pthreaded function. Found by Kevin Packard
 *
 * Revision 1.49  2000/03/17 03:45:40  craigs
 * Fixed problem with connect call hanging
 *
 * Revision 1.48  2000/02/17 23:47:40  robertj
 * Fixed error in check for SIOCGHWADDR define, thanks Markus Storm.
 *
 * Revision 1.47  2000/01/20 08:20:57  robertj
 * FreeBSD v3 compatibility changes, thanks Roger Hardiman & Motonori Shindo
 *
 * Revision 1.46  1999/11/18 13:45:21  craigs
 * Removed obsolete declaration of iostream semaphore
 *
 * Revision 1.45  1999/10/30 13:43:01  craigs
 * Added correct method of aborting socket operations asynchronously
 *
 * Revision 1.44  1999/09/27 01:04:42  robertj
 * BeOS support changes.
 *
 * Revision 1.43  1999/09/12 07:06:23  craigs
 * Added support for getting Solaris interface info
 *
 * Revision 1.42  1999/09/10 02:31:19  craigs
 * Added interface table routines
 *
 * Revision 1.41  1999/09/03 02:26:25  robertj
 * Changes to aid in breaking I/O locks on thread termination. Still needs more work esp in BSD!
 *
 * Revision 1.40  1999/05/01 03:52:20  robertj
 * Fixed various egcs warnings.
 *
 * Revision 1.39  1999/03/02 05:41:59  robertj
 * More BeOS changes
 *
 * Revision 1.38  1999/02/26 04:10:39  robertj
 * More BeOS port changes
 *
 * Revision 1.37  1999/02/22 13:26:54  robertj
 * BeOS port changes.
 *
 * Revision 1.36  1998/11/30 21:51:58  robertj
 * New directory structure.
 *
 * Revision 1.35  1998/11/24 09:39:22  robertj
 * FreeBSD port.
 *
 * Revision 1.34  1998/11/22 08:11:37  craigs
 * *** empty log message ***
 *
 * Revision 1.33  1998/11/14 10:37:38  robertj
 * Changed semantics of os_sendto to return TRUE if ANY bytes are sent.
 *
 * Revision 1.32  1998/10/16 01:16:55  craigs
 * Added Yield to help with cooperative multithreading.
 *
 * Revision 1.31  1998/10/11 02:23:16  craigs
 * Fixed problem with socket writes not correctly detecting EOF
 *
 * Revision 1.30  1998/09/24 08:21:11  robertj
 * Fixed warning on GNU 6 library.
 *
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
#include <ptlib/sockets.h>


#if defined(SIOCGENADDR)
#define SIO_Get_MAC_Address SIOCGENADDR
#define	ifr_macaddr         ifr_ifru.ifru_enaddr
#elif defined(SIOCGIFHWADDR)
#define SIO_Get_MAC_Address SIOCGIFHWADDR
#define	ifr_macaddr         ifr_hwaddr.sa_data
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD) || defined(P_SOLARIS) || defined(P_MACOSX) || defined(P_MACOS)
#define ifr_netmask ifr_addr
#endif



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

#if defined(__BEOS__) && !defined(BE_THREADS)
  // abort any I/O block using this os_handle
  PProcess::Current().PXAbortIOBlock(os_handle);
#endif

#ifdef BE_BONELESS
  int retval = ::closesocket(os_handle);
  os_handle = -1;
  return retval;
#else
  return PXClose();
#endif
}


static int SetNonBlocking(int fd)
{
#ifdef __BEOS__
  return fd;
#else
  if (fd < 0)
    return -1;

  // Set non-blocking so we can use select calls to break I/O block on close
  int cmd = 1;
  if (::ioctl(fd, FIONBIO, &cmd) == 0 && ::fcntl(fd, F_SETFD, 1) == 0)
    return fd;

  ::close(fd);
  return -1;
#endif // !__BEOS__
}


int PSocket::os_socket(int af, int type, int protocol)
{
  // attempt to create a socket
  return SetNonBlocking(::socket(af, type, protocol));
}


int PSocket::os_connect(struct sockaddr * addr, PINDEX size)
{
  int val = ::connect(os_handle, addr, size);
  if (val == 0)
    return 0;

  if (errno != EINPROGRESS)
    return -1;

  if (!PXSetIOBlock(PXConnectBlock, readTimeout))
    return -1;

  if (val == 0) {
    errno = ECONNREFUSED;
    return -1;
  }

#ifndef __BEOS__
  // A successful select() call does not necessarily mean the socket connected OK.
  int optval = -1;
  socklen_t optlen = sizeof(optval);
  getsockopt(os_handle, SOL_SOCKET, SO_ERROR, (char *)&optval, &optlen);
  if (optval != 0) {
    errno = optval;
    return -1;
  }
#endif //!__BEOS__

  return 0;
}


int PSocket::os_accept(PSocket & listener, struct sockaddr * addr, PINDEX * size)
{
  if (!listener.PXSetIOBlock(PXAcceptBlock, listener.GetReadTimeout())) {
    errno = EINTR;
    return -1;
  }

#if defined(E_PROTO)
  for (;;) {
    int new_fd = ::accept(listener.GetHandle(), addr, (socklen_t *)size);
    if (new_fd >= 0)
      return SetNonBlocking(new_fd);

    if (errno != EPROTO)
      return -1;

    PTRACE(3, "PWLib\tAccept on " << sock << " failed with EPROTO - retrying");
  }
#else
  return SetNonBlocking(::accept(listener.GetHandle(), addr, (socklen_t *)size));
#endif
}


#if !defined(P_PTHREADS) && !defined(P_MAC_MPTHREADS)

int PSocket::os_select(int maxHandle,
                   fd_set & readBits,
                   fd_set & writeBits,
                   fd_set & exceptionBits,
          const PIntArray & osHandles,
      const PTimeInterval & timeout)
{
  struct timeval * tptr = NULL;

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

  return ::select(maxHandle, &readBits, &writeBits, &exceptionBits, tptr);
}
                     
#else

int PSocket::os_select(int width,
                   fd_set & readBits,
                   fd_set & writeBits,
                   fd_set & exceptionBits,
          const PIntArray & ,
      const PTimeInterval & timeout)
{
  struct timeval * tptr = NULL;
  struct timeval   timeout_val;
  if (timeout != PMaxTimeInterval) {
    if (timeout.GetMilliSeconds() < 1000L*60L*60L*24L) {
      timeout_val.tv_usec = (timeout.GetMilliSeconds() % 1000) * 1000;
      timeout_val.tv_sec  = timeout.GetSeconds();
      tptr                = &timeout_val;
    }
  }

  int unblockPipe = PThread::Current()->unblockPipe[0];
  FD_SET(unblockPipe, &readBits);
  width = PMAX(width, unblockPipe+1);

  do {
    int result = ::select(width, &readBits, &writeBits, &exceptionBits, tptr);
    if (result >= 0) {
      if (FD_ISSET(unblockPipe, &readBits)) {
        FD_CLR(unblockPipe, &readBits);
        if (result == 1) {
          BYTE ch;
          ::read(unblockPipe, &ch, 1);
          FD_CLR(unblockPipe, &readBits);
          errno = EINTR;
          return -1;
        }
      }
      return result;
    }
  } while (errno == EINTR);
  return -1;
}

#endif


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

#ifndef __BEOS__
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
#endif //!__BEOS__

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
  if (!PXSetIOBlock(PXReadBlock, readTimeout))
    return FALSE;

#ifndef __BEOS__
  // attempt to read out of band data
  char buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);
#endif // !__BEOS__

  // attempt to read non-out of band data
  int r = ::recv(os_handle, (char *)buf, maxLen, 0);
  if (!ConvertOSError(r, LastReadError))
    return FALSE;

  lastReadCount = r;
  return lastReadCount > 0;
}


BOOL PSocket::os_recvfrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      int    flags,
      sockaddr * addr, // Address from which the datagram was received.
      PINDEX * addrlen)
{
  lastReadCount = 0;

  if (!PXSetIOBlock(PXReadBlock, readTimeout))
    return FALSE;

  // attempt to read non-out of band data
  int r = ::recvfrom(os_handle, (char *)buf, len, flags, (sockaddr *)addr, (socklen_t *)addrlen);
  if (!ConvertOSError(r, LastReadError))
    return FALSE;

  lastReadCount = r;
  return lastReadCount > 0;
}


BOOL PSocket::os_sendto(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      int flags,
      sockaddr * addr, // Address to which the datagram is sent.
      PINDEX addrlen)  
{
  lastWriteCount = 0;

  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF, LastWriteError);

  // attempt to read data
  int result;
  for (;;) {
    if (addr != NULL)
      result = ::sendto(os_handle, (char *)buf, len, flags, (sockaddr *)addr, addrlen);
    else
      result = ::send(os_handle, (char *)buf, len, flags);

    if (result > 0)
      break;

    if (errno != EWOULDBLOCK)
      return ConvertOSError(-1, LastWriteError);

    if (!PXSetIOBlock(PXWriteBlock, writeTimeout))
      return FALSE;
  }

#if !defined(P_PTHREADS) && !defined(P_MAC_MPTHREADS)
  PThread::Yield(); // Starvation prevention
#endif

  lastWriteCount = result;
  return ConvertOSError(0, LastWriteError);
}


BOOL PSocket::Read(void * buf, PINDEX len)
{
  if (os_handle < 0)
    return SetErrorValues(NotOpen, EBADF, LastReadError);

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

PEthSocket::PEthSocket(PINDEX, PINDEX, PINDEX)
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
  else
    return SetErrorValues(NotFound, ENOENT);

#if defined(SIO_Get_MAC_Address) && !defined(__BEOS__)
  PUDPSocket ifsock;
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  strcpy(ifr.ifr_name, interfaceName);
  if (!ConvertOSError(ioctl(ifsock.GetHandle(), SIO_Get_MAC_Address, &ifr)))
    return FALSE;

  memcpy(&macAddress, ifr.ifr_macaddr, sizeof(macAddress));
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
#ifndef __BEOS__
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
#endif //!__BEOS__

  return FALSE;
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

#ifndef __BEOS__
  PUDPSocket ifsock;
  struct ifreq ifr;
  ifr.ifr_addr.sa_family = AF_INET;
  if (idx == 0)
    strcpy(ifr.ifr_name, channelName);
  else
    sprintf(ifr.ifr_name, "%s:%u", (const char *)channelName, (int)(idx-1));
  if (!ConvertOSError(ioctl(os_handle, SIOCGIFADDR, &ifr)))
    return FALSE;

  sockaddr_in *sin = (struct sockaddr_in *)&ifr.ifr_addr;
  addr = sin->sin_addr;

  if (!ConvertOSError(ioctl(os_handle, SIOCGIFNETMASK, &ifr)))
    return FALSE;

  net_mask = sin->sin_addr;
  return TRUE;
#else
  return FALSE;
#endif //!__BEOS__
}


BOOL PEthSocket::GetFilter(unsigned & mask, WORD & type)
{
  if (!IsOpen())
    return FALSE;

#ifndef __BEOS__
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
#else
  return FALSE;
#endif //!__BEOS__
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

#ifndef __BEOS__
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
#else
  return FALSE;
#endif //!__BEOS__
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
  strcpy((char *)to.sa_data, channelName);
  return os_sendto(buf, len, 0, &to, sizeof(to)) && lastWriteCount >= len;
}


///////////////////////////////////////////////////////////////////////////////

BOOL PIPSocket::GetGatewayAddress(Address & addr)
{
  RouteTable table;
  if (GetRouteTable(table)) {
    for (PINDEX i = 0; i < table.GetSize(); i++) {
      if (table[i].GetNetwork() == 0) {
        addr = table[i].GetDestination();
        return TRUE;
      }
    }
  }
  return FALSE;
}



PString PIPSocket::GetGatewayInterface()
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


BOOL PIPSocket::GetRouteTable(RouteTable & table)
{
#if defined(P_LINUX) || defined (P_AIX)

  PTextFile procfile;
  if (!procfile.Open("/proc/net/route", PFile::ReadOnly))
    return FALSE;

  for (;;) {
    // Ignore heading line or remainder of route line
    procfile.ignore(1000, '\n');
    if (procfile.eof())
      return TRUE;

    char iface[20];
    long net_addr, dest_addr, net_mask;
    int flags, refcnt, use, metric;
    procfile >> iface >> ::hex >> net_addr >> dest_addr >> flags 
                      >> ::dec >> refcnt >> use >> metric 
                      >> ::hex >> net_mask;
    if (procfile.bad())
      return FALSE;

    RouteEntry * entry = new RouteEntry(net_addr);
    entry->net_mask = net_mask;
    entry->destination = dest_addr;
    entry->interfaceName = iface;
    entry->metric = metric;
    table.Append(entry);
  }

#else

#if 0 
        // Most of this code came from the source code for the "route" command 
        // so it should work on other platforms too. 
        // However, it is not complete (the "address-for-interface" function doesn't exist) and not tested! 
        
        route_table_req_t reqtable; 
        route_req_t *rrtp; 
        int i,ret; 
        
        ret = get_route_table(&reqtable); 
        if (ret < 0) 
        { 
                return FALSE; 
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
                
        return TRUE; 
#endif // 0  
#warning Platform requires implemetation of GetRouteTable()
  return FALSE;

#endif
}


BOOL PIPSocket::GetInterfaceTable(InterfaceTable & list)
{
  PUDPSocket sock;

#ifndef __BEOS__
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
    ifreq * ifName = ifConf.ifc_req;
    for (num = 0; num < ifNum; num++) {

      struct ifreq ifReq;
      strcpy(ifReq.ifr_name, ifName->ifr_name);

      if (ioctl(sock.GetHandle(), SIOCGIFFLAGS, &ifReq) >= 0) {
        int flags = ifReq.ifr_flags;
        if (flags & IFF_UP) {
          PString name(ifReq.ifr_name);

          PString            macAddr;
          PIPSocket::Address addr;
          PIPSocket::Address mask;

#ifdef SIO_Get_MAC_Address
          if (ioctl(sock.GetHandle(), SIO_Get_MAC_Address, &ifReq) >= 0) {
            BYTE * p = (BYTE *)ifReq.ifr_macaddr;
            PEthSocket::Address a(p);
            macAddr = (PString)a;
          }
#endif

          if (ioctl(sock.GetHandle(), SIOCGIFADDR, &ifReq) >= 0) 
            addr = PIPSocket::Address(((sockaddr_in *)&ifReq.ifr_addr)->sin_addr);

          if (ioctl(sock.GetHandle(), SIOCGIFNETMASK, &ifReq) >= 0) 
            mask = PIPSocket::Address(((sockaddr_in *)&ifReq.ifr_netmask)->sin_addr);

          list.Append(PNEW InterfaceEntry(name, addr, mask, macAddr));
        }
      }

#ifndef MAX
#define MAX(x,y) ( (x) > (y) ? (x) : (y) )
#endif

#if defined(P_FREEBSD) || defined(P_OPENBSD) || defined(P_NETBSD)
      // move the ifName pointer along to the next ifreq entry
      ifName = (struct ifreq *) ((char *)&ifName->ifr_addr
                    + MAX(ifName->ifr_addr.sa_len, sizeof(ifName->ifr_addr)));
#else
      ifName++;
#endif
    }
  }
#endif //!__BEOS__

  return TRUE;
}

#include "../common/pethsock.cxx"


///////////////////////////////////////////////////////////////////////////////

