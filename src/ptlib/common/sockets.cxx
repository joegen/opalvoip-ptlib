/*
 * $Id: sockets.cxx,v 1.40 1996/05/15 10:18:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.40  1996/05/15 10:18:48  robertj
 * Added ICMP protocol socket, getting common ancestor to UDP.
 * Added timeout to accept function.
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
static PTCPSocket dummyForWINSOCK; // Assure winsock is initialised
#endif


//////////////////////////////////////////////////////////////////////////////
// PSocket

BOOL PSocket::Shutdown(ShutdownValue value)
{
  return ConvertOSError(::shutdown(os_handle, value));
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

  if (rval > 0) {
    rval = 0;
    if (FD_ISSET(h1, &readfds))
      rval |= 1;
    if (FD_ISSET(h2, &readfds))
      rval |= 2;
  }

  return rval;
}


BOOL PSocket::Select(SelectList & read)
{
  return Select(read, SelectList(), SelectList(), PMaxTimeInterval);
}


BOOL PSocket::Select(SelectList & read, const PTimeInterval & timeout)
{
  return Select(read, SelectList(), SelectList(), timeout);
}


BOOL PSocket::Select(SelectList & read, SelectList & write)
{
  return Select(read, write, SelectList(), PMaxTimeInterval);
}


BOOL PSocket::Select(SelectList & read,
                     SelectList & write,
                     const PTimeInterval & timeout)
{
  return Select(read, write, SelectList(), timeout);
}


BOOL PSocket::Select(SelectList & read,
                     SelectList & write,
                     SelectList & except)
{
  return Select(read, write, except, PMaxTimeInterval);
}


BOOL PSocket::Select(SelectList & read,
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
  for (PINDEX i = 0; i < read.GetSize(); i++) {
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

  if (retval < 0)
    return FALSE;

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

  return TRUE;
}



//////////////////////////////////////////////////////////////////////////////
// PIPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PIPSocket::PIPSocket(WORD portNum)
{
  port = portNum;
}


PIPSocket::PIPSocket(const char * protocol, const PString & service)
{
  port = GetPortByService(protocol, service);
}


PString PIPSocket::GetName() const
{
  PString name;
  sockaddr_in address;
  int size = sizeof(address);
  if (getpeername(os_handle, (struct sockaddr *)&address, &size) == 0) {
    struct hostent * host_info = gethostbyaddr(
           (const char *)&address.sin_addr, sizeof(address.sin_addr), PF_INET);
    name = host_info != NULL ? host_info->h_name : inet_ntoa(address.sin_addr);
    name += " " + PString(PString::Unsigned, ntohs(address.sin_port));
  }
  return name;
}


PString PIPSocket::GetHostName()
{
  PString name;
  if (gethostname(name.GetPointer(50), 49) != 0)
    return "localhost";

  if (name.Find('.') != P_MAX_INDEX)
    return name;

  return GetHostName(name);
}


PString PIPSocket::GetHostName(const PString & hostname)
{
  struct hostent * host_info;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp = hostname;
  if (temp != 0)
    host_info = ::gethostbyaddr((const char *)&temp, sizeof(temp), PF_INET);
  else
    host_info = ::gethostbyname(hostname);

  if (host_info != NULL)
    return host_info->h_name;
  else if (temp != 0)
    return temp;
  else
    return hostname;
}


PString PIPSocket::GetHostName(const Address & addr)
{
  struct hostent * host_info =
                ::gethostbyaddr((const char *)&addr, sizeof(addr), PF_INET);
  if (host_info != NULL)
    return host_info->h_name;
  else
    return addr;
}


BOOL PIPSocket::GetHostAddress(Address & addr)
{
  return GetHostAddress(GetHostName(), addr);
}


BOOL PIPSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  if (hostname.IsEmpty())
    return FALSE;

  // lookup the host address using inet_addr, assuming it is a "." address
  long temp;
  if ((temp = inet_addr((const char *)hostname)) != -1) {
    memcpy(&addr, &temp, sizeof(addr));
    return TRUE;
  }


  // otherwise lookup the name as a host name
  struct hostent * host_info;
  if ((host_info = ::gethostbyname(hostname)) == NULL)
    return FALSE;

  memcpy(&addr, host_info->h_addr, sizeof(addr));
  return TRUE;
}


static void BuildAliases(struct hostent * host_info, PStringArray & aliases)
{
  if (host_info == NULL)
    return;

  PINDEX count = aliases.GetSize();
  for (PINDEX i = 0; host_info->h_aliases[i] != NULL; i++)
    aliases[count++] = host_info->h_aliases[i];
  for (i = 0; host_info->h_addr_list[i] != NULL; i++) {
    PIPSocket::Address temp;
    memcpy(&temp, host_info->h_addr_list[i], sizeof(temp));
    aliases[count++] = temp;
  }
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp = hostname;
  if (temp != 0)
    BuildAliases(::gethostbyaddr((const char *)&temp, sizeof(temp), PF_INET),
                 aliases);
  else
    BuildAliases(::gethostbyname(hostname), aliases);

  return aliases;
}


PStringArray PIPSocket::GetHostAliases(const Address & addr)
{
  PStringArray aliases;
  BuildAliases(::gethostbyaddr((const char *)&addr, sizeof(addr), PF_INET),
               aliases);

  return aliases;
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
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return PString();

  struct hostent * host_info = gethostbyaddr(
           (const char *)&address.sin_addr, sizeof(address.sin_addr), PF_INET);
  return host_info != NULL ? host_info->h_name : inet_ntoa(address.sin_addr);
}


PString PIPSocket::GetPeerHostName()
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(::getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return PString();

  struct hostent * host_info = gethostbyaddr(
           (const char *)&address.sin_addr, sizeof(address.sin_addr), PF_INET);
  return host_info != NULL ? host_info->h_name : inet_ntoa(address.sin_addr);
}


void PIPSocket::SetPort(WORD newPort)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = newPort;
}


void PIPSocket::SetPort(const PString & service)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = GetPortByService(service);
}


WORD PIPSocket::GetPort() const
{
  return port;
}


PString PIPSocket::GetService() const
{
  return GetServiceByPort(port);
}


WORD PIPSocket::GetPortByService(const char * protocol, const PString & service)
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


PString PIPSocket::GetServiceByPort(const char * protocol, WORD port)
{
  struct servent * serv = ::getservbyport(htons(port), protocol);
  if (serv != NULL)
    return PString(serv->s_name);
  else
    return PString(PString::Unsigned, port);
}


BOOL PIPSocket::_Connect(const PString & host)
{
  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  // attempt to lookup the host name
  Address ipnum;
  if (!GetHostAddress(host, ipnum))
    return FALSE;

  // attempt to connect
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(port);  // set the port
  sin.sin_addr   = ipnum;
  return ConvertOSError(os_connect((struct sockaddr *)&sin, sizeof(sin)));
}


BOOL PIPSocket::_Bind(Reusability reuse)
{
  if (!SetOption(SO_REUSEADDR, reuse == CanReuseAddress ? 1 : 0))
    return FALSE;

  // attempt to listen
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port        = htons(port);       // set the port

  if (!ConvertOSError(bind(os_handle, (struct sockaddr*)&sin, sizeof(sin))))
    return FALSE;

  int size = sizeof(sin);
  if (!ConvertOSError(getsockname(os_handle, (struct sockaddr*)&sin, &size)))
    return FALSE;

  port = ntohs(sin.sin_port);
  return TRUE;
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
  if (s_addr == (u_long)-1L)
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


#endif


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PTCPSocket::PTCPSocket(WORD newPort)
  : PIPSocket(newPort)
{
}


PTCPSocket::PTCPSocket(const PString & service)
  : PIPSocket("tcp", service)
{
}


PTCPSocket::PTCPSocket(const PString & address, WORD newPort)
  : PIPSocket(newPort)
{
  Connect(address);
}


PTCPSocket::PTCPSocket(const PString & address, const PString & service)
  : PIPSocket("tcp", service)
{
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


BOOL PTCPSocket::Connect(const PString & host)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // attempt to create a socket
  if (!ConvertOSError(os_handle = os_socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // attempt to connect
  if (_Connect(host)) 
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PTCPSocket::Listen(unsigned queueSize, WORD newPort, Reusability reuse)
{
  if (IsOpen())
    Close();

  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // attempt to create a socket
  if (!ConvertOSError(os_handle = os_socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // bind it to a port and attempt to listen
  if (_Bind(reuse) && ConvertOSError(::listen(os_handle, queueSize)))
    return TRUE;


  os_close();
  return FALSE;
}


BOOL PTCPSocket::Accept(PSocket & socket)
{
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


WORD PTCPSocket::GetPortByService(const PString & serviceName) const
{
  return PIPSocket::GetPortByService("tcp", serviceName);
}


PString PTCPSocket::GetServiceByPort(WORD port) const
{
  return PIPSocket::GetServiceByPort("tcp", port);
}


#endif


void PTCPSocket::OnOutOfBand(const void *, PINDEX)
{
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PIPDatagramSocket::PIPDatagramSocket(WORD newType, WORD newPort)
  : PIPSocket(newPort), sockType(newType)
{
  protocol = 0;
}

PIPDatagramSocket::PIPDatagramSocket(WORD newType, const char * proto, const PString & service)
  : PIPSocket(proto, service), sockType(newType)
{
  protocol = 0;
}

PIPDatagramSocket::PIPDatagramSocket(WORD newType, const char * protocolName)
  : sockType(newType)
{
  struct protoent * p = ::getprotobyname(protocolName);
  protocol = (p == NULL) ? 0 : p->p_proto;
}

BOOL PIPDatagramSocket::_Socket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, sockType, protocol));
}

BOOL PIPDatagramSocket::Connect(const PString & host)
{
  // close the port if it is already open
  if (!IsOpen() && !_Socket())
    return FALSE;

  // attempt to connect
  if (_Connect(host))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PIPDatagramSocket::Listen(unsigned, WORD newPort, Reusability reuse)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // close the port if it is already open
  if (!IsOpen())
    // attempt to create a socket
    if (!_Socket())
      return FALSE;

  // attempt to listen
  if (_Bind(reuse))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PIPDatagramSocket::Accept(PSocket &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}

PUDPSocket::PUDPSocket(WORD newPort)
  : PIPDatagramSocket(SOCK_DGRAM, newPort)
{
  _Socket();
}


PUDPSocket::PUDPSocket(const PString & service)
  : PIPDatagramSocket(SOCK_DGRAM, "udp", service)
{
  _Socket();
}


PUDPSocket::PUDPSocket(const PString & address, WORD newPort)
  : PIPDatagramSocket(SOCK_DGRAM, newPort)
{
  Connect(address);
}


PUDPSocket::PUDPSocket(const PString & address, const PString & service)
  : PIPDatagramSocket(SOCK_DGRAM, "udp", service)
{
  Connect(address);
}


WORD PUDPSocket::GetPortByService(const PString & serviceName) const
{
  return PIPSocket::GetPortByService("udp", serviceName);
}


PString PUDPSocket::GetServiceByPort(WORD port) const
{
  return PIPSocket::GetServiceByPort("udp", port);
}

#endif


// End Of File ///////////////////////////////////////////////////////////////
