

#include <ptlib.h>
#include <sockets.h>

extern PSemaphore PX_iostreamMutex;

PSocket::~PSocket()
{
  os_close();
}

int PSocket::os_close()
{
//PError << "Close requested for socket " << os_handle << endl;
  if (os_handle < 0) {
//PError << "Close requested for closed socket " << os_handle << endl;
    lastError = NotOpen;
    return FALSE;
  }

  // make sure we don't have any problems
  PX_iostreamMutex.Wait();
  int handle = os_handle;
  os_handle = -1;
  flush();
  PX_iostreamMutex.Signal();

  // send a shutdown to the other end
  ::shutdown(os_handle, 2);

#ifndef P_PTHREADS
  // abort any I/O block using this os_handle
  PProcess::Current().PXAbortIOBlock(os_handle);

  DWORD cmd = 0;
  ::ioctl(os_handle, FIONBIO, &cmd);
#endif

  int stat;
  while (1) {
    stat = ::close(handle);
    if (stat != EINTR)
      break;
  }

  return ConvertOSError(stat);
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

int PSocket::os_connect(struct sockaddr * addr, int size)
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
  else if (val == 0) {
    errno = ECONNREFUSED;
    return -1;
  }
  return 0;
}


int PSocket::os_accept(int sock, struct sockaddr * addr, int * size,
                       const PTimeInterval & timeout)
{
  int new_fd;
  if (!PXSetIOBlock(PXAcceptBlock, sock, timeout)) {
    errno = EINTR;
    return -1;
  }

  while (1) {
    new_fd = ::accept(sock, addr, size);
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
  BYTE buffer[32];
  int ooblen;
  while ((ooblen = ::recv(os_handle, buffer, sizeof(buffer), MSG_OOB)) > 0) 
    OnOutOfBand(buffer, ooblen);

    // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount = ::read(os_handle, (char *)buf, maxLen)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}

////////////////////////////////////////////////////////////////
//
//  PUDPSocket
//

BOOL PSocket::os_recvfrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      int    flags,
      sockaddr * addr, // Address from which the datagram was received.
      int * addrlen)
{
  if (!PXSetIOBlock(PXReadBlock, readTimeout)) {
    lastError     = Timeout;
    lastReadCount = 0;
    return FALSE;
  }

  // attempt to read non-out of band data
  if (ConvertOSError(lastReadCount =
        ::recvfrom(os_handle, (char *)buf, len, flags, (sockaddr *)addr, addrlen)))
    return lastReadCount > 0;

  lastReadCount = 0;
  return FALSE;
}


////////////////////////////////////////////////////////////////
//
//  PUDPSocket
//

BOOL PSocket::os_sendto(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      int flags,
      sockaddr * addr, // Address to which the datagram is sent.
      int addrlen)  
{
  if (!PXSetIOBlock(PXWriteBlock, writeTimeout)) {
    lastError     = Timeout;
    lastWriteCount = 0;
    return FALSE;
  }

  // attempt to read data
  return ::sendto(os_handle, (char *)buf, len, flags, (sockaddr *)addr, addrlen);
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
