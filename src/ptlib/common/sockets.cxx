/*
 * $Id: sockets.cxx,v 1.26 1996/02/15 14:46:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
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



//////////////////////////////////////////////////////////////////////////////
// PSocket

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

  int rval = os_select(PMAX(h1, h2)+1, readfds, writefds, exceptfds, timeout);

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
  }

  fd_set writefds;
  FD_ZERO(&writefds);
  for (i = 0; i < write.GetSize(); i++) {
    int h = write[i].GetHandle();
    FD_SET(h, &writefds);
    if (h > maxfds)
      maxfds = h;
  }

  fd_set exceptfds;
  FD_ZERO(&exceptfds);
  for (i = 0; i < except.GetSize(); i++) {
    int h = except[i].GetHandle();
    FD_SET(h, &exceptfds);
    if (h > maxfds)
      maxfds = h;
  }
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

  int retval = os_select(maxfds+1, readfds, writefds, exceptfds, timeout);

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


BOOL PIPSocket::GetAddress(const PString & hostname, Address & addr)
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
  if ((host_info = gethostbyname(hostname)) == NULL)
    return FALSE;

  memcpy(&addr, host_info->h_addr, sizeof(addr));
  return TRUE;
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;
  struct hostent * host_info;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp = hostname;
  if (temp != 0)
    host_info = gethostbyaddr((const char *)&temp, sizeof(temp), PF_INET);
  else
    host_info = gethostbyname(hostname);

  if (host_info != NULL) {
    for (int i = 0; host_info->h_aliases[i] != NULL; i++)
      aliases[i] = host_info->h_aliases[i];
  }

  if (aliases.GetSize() == 0) {
    if (temp != 0)
      aliases[0] = temp;
    else
      aliases[0] = hostname;
  }

  return aliases;
}


BOOL PIPSocket::GetHostAddress(Address & addr)
{
  return GetAddress(GetHostName(), addr);
}


PString PIPSocket::GetHostName()
{
  char buf[50];
  if (gethostname(buf, sizeof(buf)) == 0)
    return buf;
  else
    return PString();
}


BOOL PIPSocket::GetLocalAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetPeerAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetPeerAddress(Address & addr, WORD & portNum)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  portNum = ntohs(address.sin_port);
  return TRUE;
}


PString PIPSocket::GetLocalHostName()
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return PString();

  struct hostent * host_info = gethostbyaddr(
           (const char *)&address.sin_addr, sizeof(address.sin_addr), PF_INET);
  return host_info != NULL ? host_info->h_name : inet_ntoa(address.sin_addr);
}


PString PIPSocket::GetPeerHostName()
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getpeername(os_handle,(struct sockaddr*)&address,&size)))
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

  if (space == P_MAX_INDEX)
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


BOOL PIPSocket::_Socket(int type)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot open socket without setting port");

  // attempt to create a socket
  if (!ConvertOSError(os_handle = ::socket(AF_INET, type, 0)))
    return FALSE;

  // make the socket non-blocking
#if !defined(_WIN32)
  DWORD cmd = 1;
#if defined(_WINDOWS)
  ::ioctlsocket(os_handle, FIONBIO, &cmd);
#else
  ::ioctl(os_handle, FIONBIO, &cmd);
#endif
#endif

  return TRUE;
}


BOOL PIPSocket::_Connect(const PString & host)
{
  // attempt to lookup the host name
  Address ipnum;
  if (!GetAddress(host, ipnum))
    return FALSE;

  // attempt to connect
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(port);  // set the port
  sin.sin_addr   = ipnum;
  return ConvertOSError(::connect(os_handle,
                               (struct sockaddr *)&sin, sizeof(sin)));
}


BOOL PIPSocket::_Bind()
{
  // attempt to listen
  sockaddr_in sin;
  memset(&sin, 0, sizeof(sin));
  sin.sin_family      = AF_INET;
  sin.sin_addr.s_addr = htonl(INADDR_ANY);
  sin.sin_port        = htons(port);       // set the port

  return ConvertOSError(bind(os_handle, (struct sockaddr*)&sin, sizeof(sin)));
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


BOOL PTCPSocket::Connect(const PString & host)
{
  // attempt to create a socket
  if (!_Socket(SOCK_STREAM))
    return FALSE;

  // attempt to connect
  if (_Connect(host))
    return TRUE;

  _Close();
  return FALSE;
}


BOOL PTCPSocket::Listen(unsigned queueSize, WORD newPort)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // attempt to create a socket
  if (!_Socket(SOCK_STREAM))
    return FALSE;

  // attempt to listen
  if (_Bind() && ConvertOSError(::listen(os_handle, queueSize)))
    return TRUE;

  _Close();
  return FALSE;
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

PUDPSocket::PUDPSocket(WORD newPort)
  : PIPSocket(newPort)
{
}


PUDPSocket::PUDPSocket(const PString & service)
  : PIPSocket("udp", service)
{
}


PUDPSocket::PUDPSocket(const PString & address, WORD newPort)
  : PIPSocket(newPort)
{
  Connect(address);
}


PUDPSocket::PUDPSocket(const PString & address, const PString & service)
  : PIPSocket("udp", service)
{
  Connect(address);
}


BOOL PUDPSocket::Connect(const PString & host)
{
  // attempt to create a socket
  if (!_Socket(SOCK_DGRAM))
    return FALSE;

  // attempt to connect
  if (_Connect(host))
    return TRUE;

  Close();
  return FALSE;
}


BOOL PUDPSocket::Listen(unsigned, WORD newPort)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // attempt to create a socket
  if (!_Socket(SOCK_DGRAM))
    return FALSE;

  // attempt to listen
  if (_Bind())
    return TRUE;

  Close();
  return FALSE;
}


BOOL PUDPSocket::Accept(PSocket &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
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


//////////////////////////////////////////////////////////////////////////////
// PTelnetSocket

PTelnetSocket::PTelnetSocket()
  : PTCPSocket("telnet")
{
  Construct();
}


PTelnetSocket::PTelnetSocket(const PString & address)
  : PTCPSocket("telnet")
{
  Construct();
  Connect(address);
}


void PTelnetSocket::Construct()
{
  synchronising = 0;
  terminalType = "UNKNOWN";
  windowWidth = windowHeight = 0;
  state = StateNormal;

  memset(option, 0, sizeof(option));
  SetOurOption(TransmitBinary);
  SetOurOption(SuppressGoAhead);
  SetOurOption(StatusOption);
  SetOurOption(TimingMark);
  SetOurOption(TerminalSpeed);
  SetOurOption(TerminalType);
  SetTheirOption(TransmitBinary);
  SetTheirOption(SuppressGoAhead);
  SetTheirOption(StatusOption);
  SetTheirOption(TimingMark);
  SetTheirOption(EchoOption);

#ifdef _DEBUG
  debug = TRUE;
#endif
}


#define PTelnetError if (debug) PError << "PTelnetSocket: "
#define PDebugError if (debug) PError

BOOL PTelnetSocket::Connect(const PString & host)
{
  PTelnetError << "Connect" << endl;

  if (!PTCPSocket::Connect(host))
    return FALSE;

  SendDo(SuppressGoAhead);
  SendDo(StatusOption);
  SendWill(TerminalSpeed);
  return TRUE;
}


BOOL PTelnetSocket::Accept(PSocket & sock)
{
  if (!PTCPSocket::Accept(sock))
    return FALSE;

  SendDo(SuppressGoAhead);
  SendWill(StatusOption);
  return TRUE;
}


BOOL PTelnetSocket::Write(void const * buffer, PINDEX length)
{
  const BYTE * base = (const BYTE *)buffer;
  const BYTE * next = base;
  int count = 0;

  while (length > 0) {
    if (*next == '\r' &&
            !(length > 1 && next[1] == '\n') && !IsOurOption(TransmitBinary)) {
      // send the characters
      if (!PTCPSocket::Write(base, (next - base) + 1))
        return FALSE;
      count += lastWriteCount;

      char null = '\0';
      if (!PTCPSocket::Write(&null, 1))
        return FALSE;
      count += lastWriteCount;

      base = next+1;
    }

    if (*next == IAC) {
      // send the characters
      if (!PTCPSocket::Write(base, (next - base) + 1))
        return FALSE;
      count += lastWriteCount;
      base = next;
    }

    next++;
    length--;
  }

  if (next > base) {
    if (!PTCPSocket::Write(base, next - base))
      return FALSE;
    count += lastWriteCount;
  }

  lastWriteCount = count;
  return TRUE;
}


BOOL PTelnetSocket::SendCommand(Command cmd, int opt)
{
  BYTE buffer[3];
  buffer[0] = IAC;
  buffer[1] = (BYTE)cmd;

  switch (cmd) {
    case DO :
    case DONT :
    case WILL :
    case WONT :
      buffer[2] = (BYTE)opt;
      return PTCPSocket::Write(buffer, 3);

    case InterruptProcess :
    case Break :
    case AbortProcess :
    case SuspendProcess :
    case AbortOutput :
      if (opt) {
        // Send the command
        if (!PTCPSocket::Write(buffer, 2))
          return FALSE;
        // Send a TimingMark for output flush.
        buffer[1] = TimingMark;
        if (!PTCPSocket::Write(buffer, 2))
          return FALSE;
        // Send a DataMark for synchronisation.
        if (cmd != AbortOutput) {
          buffer[1] = DataMark;
          if (!PTCPSocket::Write(buffer, 2))
            return FALSE;
          // Send the datamark character as the only out of band data byte.
          if (!WriteOutOfBand(&buffer[1], 1))
            return FALSE;
        }
        // Then flush any waiting input data.
        PTimeInterval oldTimeout = readTimeout;
        readTimeout = 0;
        while (PTCPSocket::Read(buffer, sizeof(buffer)))
          ;
        readTimeout = oldTimeout;
      }
      break;

    default :
      return PTCPSocket::Write(buffer, 2);
  }

  return TRUE;
}


static PString GetTELNETOptionName(int code)
{
  static const char * name[] = {
    "TransmitBinary",
    "EchoOption",
    "ReconnectOption",
    "SuppressGoAhead",
    "MessageSizeOption",
    "StatusOption",
    "TimingMark",
    "RCTEOption",
    "OutputLineWidth",
    "OutputPageSize",
    "CRDisposition",
    "HorizontalTabsStops",
    "HorizTabDisposition",
    "FormFeedDisposition",
    "VerticalTabStops",
    "VertTabDisposition",
    "LineFeedDisposition",
    "ExtendedASCII",
    "ForceLogout",
    "ByteMacroOption",
    "DataEntryTerminal",
    "SupDupProtocol",
    "SupDupOutput",
    "SendLocation",
    "TerminalType",
    "EndOfRecordOption",
    "TACACSUID",
    "OutputMark",
    "TerminalLocation",
    "Use3270RegimeOption",
    "UseX3PADOption",
    "WindowSize",
    "TerminalSpeed",
    "FlowControl",
    "LineMode",
    "XDisplayLocation",
    "EnvironmentOption",
    "AuthenticateOption",
    "EncriptionOption"
  };

  if (code < PARRAYSIZE(name))
    return name[code];
  if (code == PTelnetSocket::ExtendedOptionsList)
    return "ExtendedOptionsList";
  return PString(PString::Printf, "Option #%u", code);
}


BOOL PTelnetSocket::StartSend(const char * which, BYTE code)
{
  PTelnetError << which << ' ' << GetTELNETOptionName(code) << ' ';
  if (IsOpen())
    return TRUE;

  PDebugError << "not open yet." << endl;
  osError = EBADF;
  lastError = NotOpen;
  return FALSE;
}


BOOL PTelnetSocket::SendDo(BYTE code)
{
  if (!StartSend("SendDo", code))
    return FALSE;

  OptionInfo & opt = option[code];

  switch (opt.theirState) {
    case OptionInfo::IsNo :
      PDebugError << "initiated.";
      SendCommand(DO, code);
      opt.theirState = OptionInfo::WantYes;
      break;

    case OptionInfo::IsYes :
      PDebugError << "already enabled." << endl;
      return FALSE;

    case OptionInfo::WantNo :
      PDebugError << "queued.";
      opt.theirState = OptionInfo::WantNoQueued;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "already queued." << endl;
      opt.theirState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantYes :
      PDebugError << "already negotiating." << endl;
      opt.theirState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantYesQueued :
      PDebugError << "dequeued.";
      opt.theirState = OptionInfo::WantYes;
      break;
  }

  PDebugError << endl;
  return TRUE;
}


BOOL PTelnetSocket::SendDont(BYTE code)
{
  if (!StartSend("SendDont", code))
    return FALSE;

  OptionInfo & opt = option[code];

  switch (opt.theirState) {
    case OptionInfo::IsNo :
      PDebugError << "already disabled." << endl;
      return FALSE;

    case OptionInfo::IsYes :
      PDebugError << "initiated.";
      SendCommand(DONT, code);
      opt.theirState = OptionInfo::WantNo;
      break;

    case OptionInfo::WantNo :
      PDebugError << "already negotiating." << endl;
      opt.theirState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantNoQueued :
      PDebugError << "dequeued.";
      opt.theirState = OptionInfo::WantNo;
      break;

    case OptionInfo::WantYes :
      PDebugError << "queued.";
      opt.theirState = OptionInfo::WantYesQueued;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "already queued." << endl;
      opt.theirState = OptionInfo::IsYes;
      return FALSE;
  }

  PDebugError << endl;
  return TRUE;
}


BOOL PTelnetSocket::SendWill(BYTE code)
{
  if (!StartSend("SendWill", code))
    return FALSE;

  if (!IsOpen())
    return FALSE;

  OptionInfo & opt = option[code];

  switch (opt.ourState) {
    case OptionInfo::IsNo :
      PDebugError << "initiated.";
      SendCommand(WILL, code);
      opt.ourState = OptionInfo::WantYes;
      break;

    case OptionInfo::IsYes :
      PDebugError << "already enabled." << endl;
      return FALSE;

    case OptionInfo::WantNo :
      PDebugError << "queued.";
      opt.ourState = OptionInfo::WantNoQueued;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "already queued." << endl;
      opt.ourState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantYes :
      PDebugError << "already negotiating." << endl;
      opt.ourState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantYesQueued :
      PDebugError << "dequeued.";
      opt.ourState = OptionInfo::WantYes;
      break;
  }

  PDebugError << endl;
  return TRUE;
}


BOOL PTelnetSocket::SendWont(BYTE code)
{
  if (!StartSend("SendWont", code))
    return FALSE;

  OptionInfo & opt = option[code];

  switch (opt.ourState) {
    case OptionInfo::IsNo :
      PDebugError << "already disabled." << endl;
      return FALSE;

    case OptionInfo::IsYes :
      PDebugError << "initiated.";
      SendCommand(WONT, code);
      opt.ourState = OptionInfo::WantNo;
      break;

    case OptionInfo::WantNo :
      PDebugError << "already negotiating." << endl;
      opt.ourState = OptionInfo::IsNo;
      return FALSE;

    case OptionInfo::WantNoQueued :
      PDebugError << "dequeued.";
      opt.ourState = OptionInfo::WantNo;
      break;

    case OptionInfo::WantYes :
      PDebugError << "queued.";
      opt.ourState = OptionInfo::WantYesQueued;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "already queued." << endl;
      opt.ourState = OptionInfo::IsYes;
      return FALSE;
  }

  PDebugError << endl;
  return TRUE;
}


BOOL PTelnetSocket::SendSubOption(BYTE code,
                                    const BYTE * info, PINDEX len, int subCode)
{
  if (!StartSend("SendSubOption", code))
    return FALSE;

  PDebugError << "with " << len << " bytes." << endl;

  PBYTEArray buffer(len + 6);
  buffer[0] = IAC;
  buffer[1] = SB;
  buffer[2] = code;
  PINDEX i = 3;
  if (subCode >= 0)
    buffer[i++] = (BYTE)subCode;
  while (len-- > 0) {
    if (*info == IAC)
      buffer[i++] = IAC;
    buffer[i++] = *info++;
  }
  buffer[i++] = IAC;
  buffer[i++] = SE;

  return PTCPSocket::Write((const BYTE *)buffer, i);
}


void PTelnetSocket::SetTerminalType(const PString & newType)
{
  terminalType = newType;
}


void PTelnetSocket::SetWindowSize(WORD width, WORD height)
{
  windowWidth = width;
  windowHeight = height;
  if (IsOurOption(WindowSize)) {
    BYTE buffer[4];
    buffer[0] = (BYTE)(width >> 8);
    buffer[1] = (BYTE)width;
    buffer[2] = (BYTE)(height >> 8);
    buffer[3] = (BYTE)height;
    SendSubOption(WindowSize, buffer, 4);
  }
  else {
    SetOurOption(WindowSize);
    SendWill(WindowSize);
  }
}


void PTelnetSocket::GetWindowSize(WORD & width, WORD & height) const
{
  width = windowWidth;
  height = windowHeight;
}


BOOL PTelnetSocket::Read(void * data, PINDEX bytesToRead)
{
  PBYTEArray buffer(bytesToRead);
  PINDEX charsLeft = bytesToRead;
  BYTE * dst = (BYTE *)data;

  while (charsLeft > 0) {
    BYTE * src = buffer.GetPointer(charsLeft);
    if (!PTCPSocket::Read(src, charsLeft)) {
      lastReadCount = bytesToRead - charsLeft;
      return lastReadCount > 0;
    }

    while (lastReadCount > 0) {
      BYTE currentByte = *src++;
      lastReadCount--;
      switch (state) {
        case StateCarriageReturn :
          state = StateNormal;
          if (currentByte == '\0')
            break; // Ignore \0 after CR
          // Else, fall through for normal processing

        case StateNormal :
          if (currentByte == IAC)
            state = StateIAC;
          else {
            if (currentByte == '\r' && !IsTheirOption(TransmitBinary))
              state = StateCarriageReturn;
            *dst++ = currentByte;
            charsLeft--;
          }
          break;

        case StateIAC :
          switch (currentByte) {
            case IAC :
              state = StateNormal;
              *dst++ = IAC;
              charsLeft--;
              break;

            case DO :
              state = StateDo;
              break;

            case DONT :
              state = StateDont;
              break;

            case WILL :
              state = StateWill;
              break;

            case WONT :
              state = StateWont;
              break;

            case DataMark :    // data stream portion of a Synch
              /* We may have missed an urgent notification, so make sure we
                 flush whatever is in the buffer currently.
               */
              PTelnetError << "received DataMark" << endl;
              if (synchronising > 0)
                synchronising--;
              break;

            case SB :          // subnegotiation start
              state = StateSubNegotiations;
              subOption.SetSize(0);
              break;

            default:
              if (OnCommand(currentByte))
                state = StateNormal;
              break;
          }
          break;

        case StateDo :
          OnDo(currentByte);
          state = StateNormal;
          break;

        case StateDont :
          OnDont(currentByte);
          state = StateNormal;
          break;

        case StateWill :
          OnWill(currentByte);
          state = StateNormal;
          break;

        case StateWont :
          OnWont(currentByte);
          state = StateNormal;
          break;

        case StateSubNegotiations :
          if (currentByte == IAC)
            state = StateEndNegotiations;
          else
            subOption[subOption.GetSize()] = currentByte;
          break;

        case StateEndNegotiations :
          if (currentByte == SE)
            state = StateNormal;
          else if (currentByte != IAC) {
            /* This is an error.  We only expect to get "IAC IAC" or "IAC SE".
               Several things may have happend.  An IAC was not doubled, the
               IAC SE was left off, or another option got inserted into the
               suboption are all possibilities. If we assume that the IAC was
               not doubled, and really the IAC SE was left off, we could get
               into an infinate loop here.  So, instead, we terminate the
               suboption, and process the partial suboption if we can.
             */
            state = StateIAC;
            src--;  // Go back to character for IAC ccommand
          }
          else {
            subOption[subOption.GetSize()] = currentByte;
            state = StateSubNegotiations;
            break;  // Was IAC IAC, subnegotiation not over yet.
          }
          if (subOption.GetSize() > 1 && IsOurOption(subOption[0]))
            OnSubOption(subOption[0],
                            ((const BYTE*)subOption)+1, subOption.GetSize()-1);
          break;

        default :
          PTelnetError << "illegal state: " << (int)state << endl;
          state = StateNormal;
      }
      if (synchronising > 0) {
        charsLeft = bytesToRead;    // Flush data being received.
        dst = (BYTE *)data;
      }
    }
  }
  lastReadCount = bytesToRead;
  return TRUE;
}


void PTelnetSocket::OnDo(BYTE code)
{
  PTelnetError << "OnDo " << GetTELNETOptionName(code) << ' ';

  OptionInfo & opt = option[code];

  switch (opt.ourState) {
    case OptionInfo::IsNo :
      if (opt.weCan) {
        PDebugError << "WILL.";
        SendCommand(WILL, code);
        opt.ourState = OptionInfo::IsYes;
      }
      else {
        PDebugError << "WONT.";
        SendCommand(WONT, code);
      }
      break;

    case OptionInfo::IsYes :
      PDebugError << "ignored.";
      break;

    case OptionInfo::WantNo :
      PDebugError << "is answer to WONT.";
      opt.ourState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "impossible answer.";
      opt.ourState = OptionInfo::IsYes;
      break;

    case OptionInfo::WantYes :
      PDebugError << "accepted.";
      opt.ourState = OptionInfo::IsYes;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "refused.";
      opt.ourState = OptionInfo::WantNo;
      SendCommand(WONT, code);
      break;
  }

  PDebugError << endl;

  if (IsOurOption(code)) {
    switch (code) {
      case TerminalSpeed : {
          static BYTE defSpeed[] = "38400,38400";
          SendSubOption(TerminalSpeed,defSpeed,sizeof(defSpeed)-1,SubOptionIs);
        }
        break;

      case TerminalType :
        SendSubOption(TerminalType,
                          terminalType, terminalType.GetLength(), SubOptionIs);
        break;

      case WindowSize :
        SetWindowSize(windowWidth, windowHeight);
        break;
    }
  }
}


void PTelnetSocket::OnDont(BYTE code)
{
  PTelnetError << "OnDont " << GetTELNETOptionName(code) << ' ';

  OptionInfo & opt = option[code];

  switch (opt.ourState) {
    case OptionInfo::IsNo :
      PDebugError << "ignored.";
      break;

    case OptionInfo::IsYes :
      PDebugError << "WONT.";
      opt.ourState = OptionInfo::IsNo;
      SendCommand(WONT, code);
      break;

    case OptionInfo::WantNo :
      PDebugError << "disabled.";
      opt.ourState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "accepting.";
      opt.ourState = OptionInfo::WantYes;
      SendCommand(DO, code);
      break;

    case OptionInfo::WantYes :
      PDebugError << "queued disable.";
      opt.ourState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "refused.";
      opt.ourState = OptionInfo::IsNo;
      break;
  }

  PDebugError << endl;
}


void PTelnetSocket::OnWill(BYTE code)
{
  PTelnetError << "OnWill " << GetTELNETOptionName(code) << ' ';

  OptionInfo & opt = option[code];

  switch (opt.theirState) {
    case OptionInfo::IsNo :
      if (opt.theyShould) {
        PDebugError << "DO.";
        SendCommand(DO, code);
        opt.theirState = OptionInfo::IsYes;
      }
      else {
        PDebugError << "DONT.";
        SendCommand(DONT, code);
      }
      break;

    case OptionInfo::IsYes :
      PDebugError << "ignored.";
      break;

    case OptionInfo::WantNo :
      PDebugError << "is answer to DONT.";
      opt.theirState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "impossible answer.";
      opt.theirState = OptionInfo::IsYes;
      break;

    case OptionInfo::WantYes :
      PDebugError << "accepted.";
      opt.theirState = OptionInfo::IsYes;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "refused.";
      opt.theirState = OptionInfo::WantNo;
      SendCommand(DONT, code);
      break;
  }

  PDebugError << endl;
}


void PTelnetSocket::OnWont(BYTE code)
{
  PTelnetError << "OnWont " << GetTELNETOptionName(code) << ' ';

  OptionInfo & opt = option[code];

  switch (opt.theirState) {
    case OptionInfo::IsNo :
      PDebugError << "ignored.";
      break;

    case OptionInfo::IsYes :
      PDebugError << "DONT.";
      opt.theirState = OptionInfo::IsNo;
      SendCommand(DONT, code);
      break;

    case OptionInfo::WantNo :
      PDebugError << "disabled.";
      opt.theirState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantNoQueued :
      PDebugError << "accepting.";
      opt.theirState = OptionInfo::WantYes;
      SendCommand(DO, code);
      break;

    case OptionInfo::WantYes :
      PDebugError << "refused.";
      opt.theirState = OptionInfo::IsNo;
      break;

    case OptionInfo::WantYesQueued :
      PDebugError << "queued refusal.";
      opt.theirState = OptionInfo::IsNo;
      break;
  }

  PDebugError << endl;
}


void PTelnetSocket::OnSubOption(BYTE code, const BYTE * info, PINDEX len)
{
  PTelnetError << "OnSubOption " << GetTELNETOptionName(code)
               << " of " << len << " bytes." << endl;
  switch (code) {
    case TerminalType :
      if (*info == SubOptionSend)
        SendSubOption(TerminalType,
                          terminalType, terminalType.GetLength(), SubOptionIs);
      break;

    case TerminalSpeed :
      if (*info == SubOptionSend) {
        static BYTE defSpeed[] = "38400,38400";
        SendSubOption(TerminalSpeed,defSpeed,sizeof(defSpeed)-1,SubOptionIs);
      }
      break;
  }
}


BOOL PTelnetSocket::OnCommand(BYTE code)
{
  PTelnetError << "unknown command " << (int)code << endl;
  return TRUE;
}

void PTelnetSocket::OnOutOfBand(const void *, PINDEX length)
{
  PTelnetError << "out of band data received of length " << length << endl;
  synchronising++;
}


// End Of File ///////////////////////////////////////////////////////////////
