/*
 * $Id: sockets.cxx,v 1.59 1997/06/06 10:56:36 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.59  1997/06/06 10:56:36  craigs
 * Added new functions for connectionless UDP writes
 *
 * Revision 1.58  1997/01/04 07:42:18  robertj
 * Fixed GCC Warnings.
 *
 * Revision 1.57  1997/01/04 06:54:38  robertj
 * Added missing canonical name to alias list.
 *
 * Revision 1.56  1996/12/17 11:07:05  robertj
 * Added clear of name cache.
 *
 * Revision 1.55  1996/12/12 09:23:27  robertj
 * Fixed name cache to cache missing names as well.
 * Fixed new connect with specific local port so can be re-used (simultaneous FTP session bug)
 *
 * Revision 1.54  1996/12/05 11:46:39  craigs
 * Fixed problem with Win95 recvfrom not having timeouts
 *
 * Revision 1.53  1996/11/30 12:08:17  robertj
 * Added Connect() variant so can set the local port number on link.
 *
 * Revision 1.52  1996/11/16 10:49:03  robertj
 * Fixed missing const in PIPSocket::Address stream output operator..
 *
 * Revision 1.51  1996/11/16 01:43:49  craigs
 * Fixed problem with ambiguous DNS cache keys
 *
 * Revision 1.50  1996/11/10 21:08:31  robertj
 * Added host name caching.
 *
 * Revision 1.49  1996/11/04 03:40:22  robertj
 * Moved address printer from inline to source.
 *
 * Revision 1.48  1996/10/26 01:41:09  robertj
 * Compensated for Win'95 gethostbyaddr bug.
 *
 * Revision 1.47  1996/09/14 13:09:40  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.46  1996/08/25 09:33:32  robertj
 * Added function to detect "local" host name.
 *
 * Revision 1.45  1996/07/30 12:24:53  robertj
 * Fixed incorrect conditional stopping Select() from working.
 *
 * Revision 1.44  1996/07/27 04:10:35  robertj
 * Changed Select() calls to return error codes.
 *
 * Revision 1.43  1996/06/10 09:58:21  robertj
 * Fixed win95 compatibility with looking up zero address (got a response and shouldn't).
 *
 * Revision 1.42  1996/05/26 03:47:03  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.39  1996/04/29 12:20:01  robertj
 * Fixed GetHostAliases() so doesn't overwrite names with IP numbers.
 *
 * Revision 1.38  1996/04/15 10:59:41  robertj
 * Opened socket on UDP sockets so ReadFrom/WriteTo work when no Connect/Listen.
 *
 * Revision 1.37  1996/03/31 09:06:41  robertj
 * Added socket shutdown function.
 *
 * Revision 1.35  1996/03/18 13:33:18  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
 *
 * Revision 1.34  1996/03/17 05:51:18  robertj
 * Fixed strange bug in accept cant have NULL address.
 *
 * Revision 1.33  1996/03/16 04:52:20  robertj
 * Changed all the get host name and get host address functions to be more consistent.
 *
 * Revision 1.32  1996/03/04 12:21:00  robertj
 * Split file into telnet.cxx
 *
 * Revision 1.31  1996/03/03 07:38:45  robertj
 * Added Reusability clause to the Listen() function on sockets.
 *
 * Revision 1.30  1996/03/02 03:25:13  robertj
 * Added Capability to get and set Berkeley socket options.
 *
 * Revision 1.29  1996/02/25 11:30:08  robertj
 * Changed Listen so can do a listen on a socket that is connected.
 *
 * Revision 1.28  1996/02/25 03:10:55  robertj
 * Moved some socket functions to platform dependent code.
 *
 * Revision 1.27  1996/02/19 13:30:15  robertj
 * Fixed bug in getting port by service name when specifying service by string number.
 * Added SO_LINGER option to socket to stop data loss on close.
 *
 * Revision 1.26  1996/02/15 14:46:44  robertj
 * Added Select() function to PSocket.
 *
 * Revision 1.25  1996/02/13 13:08:09  robertj
 * Fixed usage of sock_addr structure, not being cleared correctly.
 *
 * Revision 1.24  1996/02/08 12:27:22  robertj
 * Added function to get peer port as well as IP number..
 *
 * Revision 1.23  1996/02/03 11:07:37  robertj
 * Fixed buf in assuring error when converting string to IP number and string is empty.
 *
 * Revision 1.22  1996/01/28 14:08:13  robertj
 * Changed service parameter to PString for ease of use in GetPortByService function
 * Fixed up comments.
 * Added default value in string for service name.
 *
 * Revision 1.21  1996/01/23 13:19:13  robertj
 * Moved Accept() function to platform dependent code.
 *
 * Revision 1.20  1995/12/23 03:42:53  robertj
 * Unix portability issues.
 *
 * Revision 1.19  1995/12/10 11:42:23  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.18  1995/10/14 15:11:31  robertj
 * Added internet address to string conversion functionality.
 *
 * Revision 1.17  1995/07/02 01:21:23  robertj
 * Added static functions to get the current host name/address.
 *
 * Revision 1.16  1995/06/17 00:47:01  robertj
 * Changed overloaded Open() calls to 3 separate function names.
 * More logical design of port numbers and service names.
 *
 * Revision 1.15  1995/06/04 12:45:33  robertj
 * Added application layer protocol sockets.
 * Slight redesign of port numbers on sockets.
 *
 * Revision 1.14  1995/04/25 11:12:44  robertj
 * Fixed functions hiding ancestor virtuals.
 *
 * Revision 1.13  1995/04/01 08:31:54  robertj
 * Finally got a working TELNET.
 *
 * Revision 1.12  1995/03/18 06:27:49  robertj
 * Rewrite of telnet socket protocol according to RFC1143.
 *
 * Revision 1.11  1995/03/12  04:46:29  robertj
 * Added more functionality.
 *
 * Revision 1.10  1995/02/21  11:25:29  robertj
 * Further implementation of telnet socket, feature complete now.
 *
 * Revision 1.9  1995/01/27  11:16:16  robertj
 * Fixed missing cast in function, required by some platforms.
 *
 * Revision 1.8  1995/01/15  04:55:47  robertj
 * Moved all Berkley socket functions inside #ifdef.
 *
 * Revision 1.7  1995/01/04  10:57:08  robertj
 * Changed for HPUX and GNU2.6.x
 *
 * Revision 1.6  1995/01/03  09:37:52  robertj
 * Added constructor to open TCP socket.
 *
 * Revision 1.5  1995/01/02  12:28:25  robertj
 * Documentation.
 * Added more socket functions.
 *
 * Revision 1.4  1995/01/01  01:06:58  robertj
 * More implementation.
 *
 * Revision 1.3  1994/11/28  12:38:49  robertj
 * Added DONT and WONT states.
 *
 * Revision 1.2  1994/08/21  23:43:02  robertj
 * Some implementation.
 *
 * Revision 1.1  1994/08/01  03:39:05  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>

#include <ctype.h>

#if defined(_WIN32) || defined(WINDOWS)
static PWinSock dummyForWinSock; // Assure winsock is initialised
#endif

//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::PSocket()
{
  port = 0;
}


BOOL PSocket::Connect(const PString &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::Accept(PSocket &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::SetOption(int option, int value)
{
  return ConvertOSError(::setsockopt(os_handle,
                           SOL_SOCKET, option, (char *)&value, sizeof(value)));
}


BOOL PSocket::SetOption(int option, const void * valuePtr, PINDEX valueSize)
{
  return ConvertOSError(::setsockopt(os_handle,
                             SOL_SOCKET, option, (char *)valuePtr, valueSize));
}


BOOL PSocket::GetOption(int option, int & value)
{
  int valSize = sizeof(value);
  return ConvertOSError(::getsockopt(os_handle,
                                SOL_SOCKET, option, (char *)&value, &valSize));
}


BOOL PSocket::GetOption(int option, void * valuePtr, int valueSize)
{
  return ConvertOSError(::getsockopt(os_handle,
                            SOL_SOCKET, option, (char *)valuePtr, &valueSize));
}


BOOL PSocket::Shutdown(ShutdownValue value)
{
  return ConvertOSError(::shutdown(os_handle, value));
}


WORD PSocket::GetPortByService(const PString & serviceName) const
{
  return GetPortByService(GetProtocolName(), serviceName);
}


WORD PSocket::GetPortByService(const char * protocol, const PString & service)
{
  PINDEX space = service.FindOneOf(" \t\r\n");
  struct servent * serv = ::getservbyname(service(0, space-1), protocol);
  if (serv != NULL)
    return ntohs(serv->s_port);

  if (space != P_MAX_INDEX)
    return (WORD)atoi(service(space+1, P_MAX_INDEX));

  if (isdigit(service[0]))
    return (WORD)atoi(service);

  return 0;
}


PString PSocket::GetServiceByPort(WORD port) const
{
  return GetServiceByPort(GetProtocolName(), port);
}


PString PSocket::GetServiceByPort(const char * protocol, WORD port)
{
  struct servent * serv = ::getservbyport(htons(port), protocol);
  if (serv != NULL)
    return PString(serv->s_name);
  else
    return PString(PString::Unsigned, port);
}


void PSocket::SetPort(WORD newPort)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = newPort;
}


void PSocket::SetPort(const PString & service)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = GetPortByService(service);
}


WORD PSocket::GetPort() const
{
  return port;
}


PString PSocket::GetService() const
{
  return GetServiceByPort(port);
}


int PSocket::Select(PSocket & sock1, PSocket & sock2)
{
  return Select(sock1, sock2, PMaxTimeInterval);
}


int PSocket::Select(PSocket & sock1,
                    PSocket & sock2,
                    const PTimeInterval & timeout)
{
  int h1 = sock1.GetHandle();
  int h2 = sock2.GetHandle();

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(h1, &readfds);
  FD_SET(h2, &readfds);
  fd_set writefds;
  FD_ZERO(&writefds);
  fd_set exceptfds;
  FD_ZERO(&exceptfds);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

  PIntArray allfds(4);
  allfds[0] = h1;
  allfds[1] = 1;
  allfds[2] = h2;
  allfds[3] = 1;
  int rval = os_select(PMAX(h1, h2)+1,
                                readfds, writefds, exceptfds, allfds, timeout);

  Errors lastError;
  int osError;
  if (!ConvertOSError(rval, lastError, osError))
    return lastError;

  rval = 0;
  if (FD_ISSET(h1, &readfds))
    rval -= 1;
  if (FD_ISSET(h2, &readfds))
    rval -= 2;

  return rval;
}


PChannel::Errors PSocket::Select(SelectList & read)
{
  SelectList dummy1, dummy2;
  return Select(read, dummy1, dummy2, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read, const PTimeInterval & timeout)
{
  SelectList dummy1, dummy2;
  return Select(read, dummy1, dummy2, timeout);
}


PChannel::Errors PSocket::Select(SelectList & read, SelectList & write)
{
  SelectList dummy1;
  return Select(read, write, dummy1, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 const PTimeInterval & timeout)
{
  SelectList dummy1;
  return Select(read, write, dummy1, timeout);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except)
{
  return Select(read, write, except, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except,
                                 const PTimeInterval & timeout)
{
  int maxfds = 0;
  PINDEX nextfd = 0;
  PIntArray allfds(2*(read.GetSize()+write.GetSize()+except.GetSize()));

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
  fd_set readfds;
  FD_ZERO(&readfds);
  PINDEX i;
  for (i = 0; i < read.GetSize(); i++) {
    int h = read[i].GetHandle();
    FD_SET(h, &readfds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 1;
  }

  fd_set writefds;
  FD_ZERO(&writefds);
  for (i = 0; i < write.GetSize(); i++) {
    int h = write[i].GetHandle();
    FD_SET(h, &writefds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 2;
  }

  fd_set exceptfds;
  FD_ZERO(&exceptfds);
  for (i = 0; i < except.GetSize(); i++) {
    int h = except[i].GetHandle();
    FD_SET(h, &exceptfds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 4;
  }
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

  int retval = os_select(maxfds+1,readfds,writefds,exceptfds,allfds,timeout);

  Errors lastError;
  int osError;
  if (!ConvertOSError(retval, lastError, osError))
    return lastError;

  if (retval > 0) {
    for (i = 0; i < read.GetSize(); i++)
      if (!FD_ISSET(read[i].GetHandle(), &readfds))
        read.RemoveAt(i--);
    for (i = 0; i < write.GetSize(); i++)
      if (!FD_ISSET(write[i].GetHandle(), &writefds))
        write.RemoveAt(i--);
    for (i = 0; i < except.GetSize(); i++)
      if (!FD_ISSET(except[i].GetHandle(), &exceptfds))
        except.RemoveAt(i--);
  }
  else {
    read.RemoveAll();
    write.RemoveAll();
    except.RemoveAll();
  }

  return NoError;
}



//////////////////////////////////////////////////////////////////////////////
// IP Caching

PDECLARE_CLASS(PIPCacheData, PObject)
  public:
    PIPCacheData(struct hostent * ent, const char * original);
    const PString & GetHostName() const { return hostname; }
    const PIPSocket::Address & GetHostAddress() const { return address; }
    const PStringArray & GetHostAliases() const { return aliases; }
    BOOL HasAged() const;
  private:
    PString            hostname;
    PIPSocket::Address address;
    PStringArray       aliases;
    PTime              birthDate;
};

PIPCacheData::PIPCacheData(struct hostent * host_info, const char * original)
{
  if (host_info == NULL) {
    address.s_addr = 0;
    return;
  }

  hostname = host_info->h_name;
  memcpy(&address, host_info->h_addr, sizeof(address));

  PINDEX count = 0;
  aliases[count++] = host_info->h_name;

  PINDEX i;
  for (i = 0; host_info->h_aliases[i] != NULL; i++)
    aliases[count++] = host_info->h_aliases[i];

  for (i = 0; host_info->h_addr_list[i] != NULL; i++)
    aliases[count++] = inet_ntoa(*(struct in_addr *)host_info->h_addr_list[i]);

  for (i = 0; i < count; i++)
    if (aliases[i] *= original)
      return;

  aliases[i++] = original;
}


BOOL PIPCacheData::HasAged() const
{
  static PTimeInterval retirement(0, 0, 5); // 5 minutes
  PTime now;
  PTimeInterval age = now - birthDate;
  return age > retirement;
}


PDICTIONARY(PHostByName_private, PCaselessString, PIPCacheData);

class PHostByName : PHostByName_private
{
  public:
    BOOL GetHostName(const PString & name, PString & hostname);
    BOOL GetHostAddress(const PString & name, PIPSocket::Address & address);
    BOOL GetHostAliases(const PString & name, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PString & name);
    PSemaphore mutex;
  friend void PIPSocket::ClearNameCache();
} pHostByName;


BOOL PHostByName::GetHostName(const PString & name, PString & hostname)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    hostname = host->GetHostName();

  mutex.Signal();

  return host != NULL;
}


BOOL PHostByName::GetHostAddress(const PString & name, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();

  return host != NULL;
}


BOOL PHostByName::GetHostAliases(const PString & name, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    aliases = host->GetHostAliases();

  mutex.Signal();
  return host != NULL;
}


PIPCacheData * PHostByName::GetHost(const PString & name)
{
  mutex.Wait();

  PCaselessString key = name;
  PIPCacheData * host = GetAt(key);
  if (host == NULL || host->HasAged()) {
    mutex.Signal();
    host = new PIPCacheData(::gethostbyname(name), name);
    mutex.Wait();
    SetAt(key, host);
  }

  return host->GetHostAddress() != 0 ? host : 0;
}


PDECLARE_CLASS(PIPCacheKey, PObject)
  public:
    PIPCacheKey(const PIPSocket::Address & a)
      { addr = a; }

    PObject * Clone() const
      { return new PIPCacheKey(*this); }

    PINDEX HashFunction() const
      { return (addr.Byte2() + addr.Byte3() + addr.Byte4())%41; }

  private:
    PIPSocket::Address addr;
};

PDICTIONARY(PHostByAddr_private, PIPCacheKey, PIPCacheData);

class PHostByAddr : PHostByAddr_private
{
  public:
    BOOL GetHostName(const PIPSocket::Address & addr, PString & hostname);
    BOOL GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address);
    BOOL GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PIPSocket::Address & addr);
    PSemaphore mutex;
  friend void PIPSocket::ClearNameCache();
} pHostByAddr;


BOOL PHostByAddr::GetHostName(const PIPSocket::Address & addr, PString & hostname)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    hostname = host->GetHostName();

  mutex.Signal();
  return host != NULL;
}


BOOL PHostByAddr::GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();
  return host != NULL;
}


BOOL PHostByAddr::GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    aliases = host->GetHostAliases();

  mutex.Signal();
  return host != NULL;
}

PIPCacheData * PHostByAddr::GetHost(const PIPSocket::Address & addr)
{
  mutex.Wait();

  PIPCacheKey key = addr;
  PIPCacheData * host = GetAt(key);
  if (host == NULL || host->HasAged()) {
    mutex.Signal();
    struct hostent * host_info = ::gethostbyaddr((const char *)&addr, sizeof(addr), PF_INET);
#if defined(_WIN32) || defined(WINDOWS)
    if (host_info != NULL && host_info->h_addr_list[0] != NULL)
      host_info->h_addr_list[1] = NULL;
#endif
    host = new PIPCacheData(host_info, inet_ntoa(addr));
    mutex.Wait();
    SetAt(key, host);
  }

  return host->GetHostAddress() != 0 ? host : 0;
}



//////////////////////////////////////////////////////////////////////////////
// PIPSocket

PIPSocket::PIPSocket()
{
}


void PIPSocket::ClearNameCache()
{
  pHostByName.mutex.Wait();
  pHostByName.RemoveAll();
  pHostByName.mutex.Signal();
  pHostByAddr.mutex.Wait();
  pHostByAddr.RemoveAll();
  pHostByAddr.mutex.Signal();
}


PString PIPSocket::GetName() const
{
  PString name;
  sockaddr_in address;
  int size = sizeof(address);
  if (getpeername(os_handle, (struct sockaddr *)&address, &size) == 0)
    name = GetHostName(address.sin_addr) + psprintf(":%u", ntohs(address.sin_port));
  return name;
}


PString PIPSocket::GetHostName()
{
  char name[100];
  if (gethostname(name, sizeof(name)-1) != 0)
    return "localhost";
  name[sizeof(name)-1] = '\0';
  return name;
}


PString PIPSocket::GetHostName(const PString & hostname)
{
  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp = hostname;
  if (temp != 0)
    return GetHostName(temp);

  PString canonicalname;
  if (pHostByName.GetHostName(hostname, canonicalname))
    return canonicalname;

  return hostname;
}


PString PIPSocket::GetHostName(const Address & addr)
{
  if (addr == 0)
    return addr;

  PString hostname;
  if (pHostByAddr.GetHostName(addr, hostname))
    return hostname;

  return addr;
}


BOOL PIPSocket::GetHostAddress(Address & addr)
{
  return pHostByName.GetHostAddress(GetHostName(), addr);
}


BOOL PIPSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  if (hostname.IsEmpty())
    return FALSE;

  // lookup the host address using inet_addr, assuming it is a "." address
  addr = hostname;
  if (addr != 0)
    return TRUE;

  // otherwise lookup the name as a host name
  return pHostByName.GetHostAddress(hostname, addr);
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address addr = hostname;
  if (addr != 0)
    pHostByAddr.GetHostAliases(addr, aliases);
  else
    pHostByName.GetHostAliases(hostname, aliases);

  return aliases;
}


PStringArray PIPSocket::GetHostAliases(const Address & addr)
{
  PStringArray aliases;

  pHostByAddr.GetHostAliases(addr, aliases);

  return aliases;
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

  PStringArray itsAliases = GetHostAliases(hostname);
  if (itsAliases.IsEmpty())
    return FALSE;

  PStringArray myAliases = GetHostAliases(GetHostName());
  if (myAliases.IsEmpty())
    return FALSE;

  for (PINDEX mine = 0; mine < myAliases.GetSize(); mine++) {
    for (PINDEX its = 0; its < itsAliases.GetSize(); its++) {
      if (myAliases[mine] *= itsAliases[its])
        return TRUE;
    }
  }

  return FALSE;
}


BOOL PIPSocket::GetLocalAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetLocalAddress(Address & addr, WORD & portNum)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  portNum = ntohs(address.sin_port);
  return TRUE;
}
 
BOOL PIPSocket::GetPeerAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetPeerAddress(Address & addr, WORD & portNum)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  portNum = ntohs(address.sin_port);
  return TRUE;
}


PString PIPSocket::GetLocalHostName()
{
  PString name;

  sockaddr_in address;
  int size = sizeof(address);
  if (ConvertOSError(::getsockname(os_handle, (struct sockaddr *)&address, &size)))
    name = GetHostName(address.sin_addr);

  return name;
}


PString PIPSocket::GetPeerHostName()
{
  PString name;

  sockaddr_in address;
  int size = sizeof(address);
  if (ConvertOSError(::getpeername(os_handle, (struct sockaddr *)&address, &size)))
    name = GetHostName(address.sin_addr);

  return name;
}


BOOL PIPSocket::Connect(const PString & host)
{
  Address ipnum;
  if (GetHostAddress(host, ipnum))
    return Connect(ipnum);
  return FALSE;
}


BOOL PIPSocket::Connect(const Address & addr)
{
  return Connect(0, addr);
}


BOOL PIPSocket::Connect(WORD localPort, const Address & addr)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  // attempt to create a socket
  if (!OpenSocket())
    return FALSE;

  // attempt to connect
  sockaddr_in sin;
  if (localPort != 0) {
    if (!SetOption(SO_REUSEADDR, 1)) {
      os_close();
      return FALSE;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port        = htons(localPort);       // set the port
    if (!ConvertOSError(::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin)))) {
      os_close();
      return FALSE;
    }
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(port);  // set the port
  sin.sin_addr   = addr;
  if (ConvertOSError(os_connect((struct sockaddr *)&sin, sizeof(sin))))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PIPSocket::Listen(unsigned, WORD newPort, Reusability reuse)
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
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = htonl(INADDR_ANY);
    sin.sin_port        = htons(port);       // set the port

    if (ConvertOSError(::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin)))) {
      int size = sizeof(sin);
      if (ConvertOSError(::getsockname(os_handle, (struct sockaddr*)&sin, &size))) {
        port = ntohs(sin.sin_port);
        return TRUE;
      }
    }
  }

  os_close();
  return FALSE;
}


PIPSocket::Address::Address()
{
  s_addr = inet_addr("127.0.0.1");
}


PIPSocket::Address::Address(const in_addr & addr)
{
  s_addr = addr.s_addr;
}


PIPSocket::Address::Address(const Address & addr)
{
  s_addr = addr.s_addr;
}


PIPSocket::Address::Address(const PString & dotNotation)
{
  s_addr = inet_addr((const char *)dotNotation);
  if (s_addr == INADDR_NONE)
    s_addr = 0;
}


PIPSocket::Address & PIPSocket::Address::operator=(const in_addr & addr)
{
  s_addr = addr.s_addr;
  return *this;
}


PIPSocket::Address & PIPSocket::Address::operator=(const Address & addr)
{
  s_addr = addr.s_addr;
  return *this;
}


PIPSocket::Address::operator PString() const
{
  return inet_ntoa(*this);
}


ostream & operator<<(ostream & s, const PIPSocket::Address & a)
{
  return s << inet_ntoa(a);
}


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

PTCPSocket::PTCPSocket(WORD newPort)
{
  SetPort(newPort);
}


PTCPSocket::PTCPSocket(const PString & service)
{
  SetPort(service);
}


PTCPSocket::PTCPSocket(const PString & address, WORD newPort)
{
  SetPort(newPort);
  Connect(address);
}


PTCPSocket::PTCPSocket(const PString & address, const PString & service)
{
  SetPort(service);
  Connect(address);
}


PTCPSocket::PTCPSocket(PSocket & socket)
{
  Accept(socket);
}


PTCPSocket::PTCPSocket(PTCPSocket & tcpSocket)
{
  Accept(tcpSocket);
}


BOOL PTCPSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_STREAM, 0));
}


const char * PTCPSocket::GetProtocolName() const
{
  return "tcp";
}


BOOL PTCPSocket::Listen(unsigned queueSize, WORD newPort, Reusability reuse)
{
  if (PIPSocket::Listen(queueSize, newPort, reuse) &&
      ConvertOSError(::listen(os_handle, queueSize)))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PTCPSocket::Accept(PSocket & socket)
{
  PAssert(socket.IsDescendant(PIPSocket::Class()), "Invalid listener socket");

  sockaddr_in address;
  address.sin_family = AF_INET;
  int size = sizeof(address);
  if (!ConvertOSError(os_handle = os_accept(socket.GetHandle(),
                                          (struct sockaddr *)&address, &size,
                                           socket.GetReadTimeout())))
    return FALSE;

  port = ((PIPSocket &)socket).GetPort();
  return TRUE;
}


BOOL PTCPSocket::WriteOutOfBand(void const * buf, PINDEX len)
{
  int count = ::send(os_handle, (const char *)buf, len, MSG_OOB);
  if (count < 0) {
    lastWriteCount = 0;
    return ConvertOSError(count);
  }
  else {
    lastWriteCount = count;
    return TRUE;
  }
}


void PTCPSocket::OnOutOfBand(const void *, PINDEX)
{
}


//////////////////////////////////////////////////////////////////////////////
// PIPDatagramSocket

PIPDatagramSocket::PIPDatagramSocket()
{
}


BOOL PIPDatagramSocket::ReadFrom(void * buf, PINDEX len,
                                 Address & addr, WORD & port)
{
  lastReadCount = 0;

  sockaddr_in sockAddr;
  int addrLen = sizeof(sockAddr);
  if (os_recvfrom(buf, len, 0, (struct sockaddr *)&sockAddr, &addrLen)) {
    addr = sockAddr.sin_addr;
    port = ntohs(sockAddr.sin_port);
  }

  return lastReadCount > 0;
}


BOOL PIPDatagramSocket::WriteTo(const void * buf, PINDEX len,
                                const Address & addr, WORD port)
{
  lastWriteCount = 0;

  sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_addr = addr;
  sockAddr.sin_port = htons(port);
  int sendResult = os_sendto(buf, len, 0,
                             (struct sockaddr *)&sockAddr, sizeof(sockAddr));
  if (ConvertOSError(sendResult))
    lastWriteCount = sendResult;

  return lastWriteCount >= len;
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

PUDPSocket::PUDPSocket(WORD newPort)
{
  sendPort = 0;
  SetPort(newPort);
  OpenSocket();
}


PUDPSocket::PUDPSocket(const PString & service)
{
  sendPort = 0;
  SetPort(service);
  OpenSocket();
}


PUDPSocket::PUDPSocket(const PString & address, WORD newPort)
{
  sendPort = 0;
  SetPort(newPort);
  Connect(address);
}


PUDPSocket::PUDPSocket(const PString & address, const PString & service)
{
  sendPort = 0;
  SetPort(service);
  Connect(address);
}


BOOL PUDPSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_DGRAM, 0));
}


const char * PUDPSocket::GetProtocolName() const
{
  return "udp";
}

BOOL PUDPSocket::Connect(const PString & address)
{
  sendPort = 0;
  return PIPDatagramSocket::Connect(address);
}

void PUDPSocket::SetSendAddress(const Address & newAddress, WORD newPort)
{
  sendAddress = newAddress;
  sendPort    = newPort;
}

BOOL PUDPSocket::Write(const void * buf, PINDEX len)
{
  if (sendPort == 0)
    return PIPDatagramSocket::Write(buf, len);
  else
    return PIPDatagramSocket::WriteTo(buf, len, sendAddress, sendPort);
}

// End Of File ///////////////////////////////////////////////////////////////
