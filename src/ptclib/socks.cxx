/*
 * main.cxx
 *
 * PWLib application source file for GetCyberPatrol
 *
 * Main program entry point.
 *
 * Copyright 98 Equivalence
 *
 * $Log: socks.cxx,v $
 * Revision 1.1  1998/12/22 10:30:24  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <ptclib/socks.h>

#define new PNEW

#define SOCKS_VERSION ((BYTE)5)

#define SOCKS_CMD_CONNECT ((BYTE)1)
#define SOCKS_CMD_BIND    ((BYTE)2)
#define SOCKS_CMD_UDP     ((BYTE)3)

#define SOCKS_AUTH_NONE      ((BYTE)0)
#define SOCKS_AUTH_USER_PASS ((BYTE)2)
#define SOCKS_AUTH_FAILED    ((BYTE)0xff)

#define SOCKS_ADDR_IPV4       ((BYTE)1)
#define SOCKS_ADDR_DOMAINNAME ((BYTE)3)
#define SOCKS_ADDR_IPV6       ((BYTE)4)


#pragma pack(1)
struct SOCKS5_PDU {
  BYTE version;
  BYTE code;
  BYTE reserved;
  BYTE addrType;
  PIPSocket::Address address;
  WORD port;
};
#pragma pack()


///////////////////////////////////////////////////////////////////////////////

PSocksSocket::PSocksSocket(WORD port)
  : PTCPSocket(port)
{
  serverHost = "proxy";
  serverPort = DefaultServerPort;
  remotePort = port;
  localPort = 0;

  // get proxy information
  PConfig config(PConfig::System, "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\");

  // get the proxy configuration string
  PString str = config.GetString("Internet Settings", "ProxyServer", "");
  if (str.Find('=') == P_MAX_INDEX)
    SetServer("socks");
  else {
    PStringArray tokens = str.Tokenise(";");
    PINDEX i;
    for (i = 0; i < tokens.GetSize(); i++) {
      str = tokens[i];
      PINDEX equalPos = str.Find('=');
      if (equalPos != P_MAX_INDEX && (str.Left(equalPos) *= "socks")) {
        SetServer(str.Mid(equalPos+1));
        break;
      }
    }
  }
}


BOOL PSocksSocket::Connect(const PString & address)
{
  return ConnectServer() && SendSocksCommand(SOCKS_CMD_CONNECT, address, 0);
}


BOOL PSocksSocket::Connect(const Address & addr)
{
  return ConnectServer() && SendSocksCommand(SOCKS_CMD_CONNECT, NULL, addr);
}


BOOL PSocksSocket::Connect(WORD, const Address &)
{
  PAssertAlways(PUnsupportedFeature);
  return FALSE;
}


BOOL PSocksSocket::Listen(unsigned, WORD newPort, Reusability reuse)
{
  PAssert(newPort != 0 || port != 0, PUnsupportedFeature);
  PAssert(reuse, PUnsupportedFeature);

  if (!ConnectServer())
    return FALSE;

  if (!SendSocksCommand(SOCKS_CMD_BIND, NULL, 0))
    return FALSE;

  port = localPort;
  return TRUE;
}


BOOL PSocksSocket::Accept()
{
  if (!IsOpen())
    return FALSE;

  return ReceiveSocksResponse(remoteAddress, remotePort);
}


BOOL PSocksSocket::Accept(PSocket & socket)
{
  // If is right class, transfer the SOCKS socket to class to receive the accept
  // The "listener" socket is implicitly closed as there is really only one
  // handle in a SOCKS BIND operation.
  PAssert(socket.IsDescendant(PSocksSocket::Class()), PUnsupportedFeature);
  os_handle = ((PSocksSocket &)socket).TransferHandle(*this);
  return Accept();
}


int PSocksSocket::TransferHandle(PSocksSocket & destination)
{
  // This "transfers" the socket from one onstance to another.

  int the_handle = os_handle;
  destination.SetReadTimeout(readTimeout);
  destination.SetWriteTimeout(writeTimeout);

  // Close the instance of the socket but don't actually close handle.
  os_handle = -1;

  return the_handle;
}


BOOL PSocksSocket::GetLocalAddress(Address & addr)
{
  if (!IsOpen())
    return FALSE;

  addr = localAddress;
  return TRUE;
}


BOOL PSocksSocket::GetLocalAddress(Address & addr, WORD & port)
{
  if (!IsOpen())
    return FALSE;

  addr = localAddress;
  port = localPort;
  return TRUE;
}


BOOL PSocksSocket::GetPeerAddress(Address & addr)
{
  if (!IsOpen())
    return FALSE;

  addr = remoteAddress;
  return TRUE;
}


BOOL PSocksSocket::GetPeerAddress(Address & addr, WORD & port)
{
  if (!IsOpen())
    return FALSE;

  addr = remoteAddress;
  port = remotePort;
  return TRUE;
}


BOOL PSocksSocket::SetServer(const PString & hostname, const char * service)
{
  return SetServer(hostname, GetPortByService(service));
}


BOOL PSocksSocket::SetServer(const PString & hostname, WORD port)
{
  if (IsOpen())
    return FALSE;

  PINDEX colon = hostname.Find(':');
  if (colon == P_MAX_INDEX)
    serverHost = hostname;
  else {
    unsigned portnum = hostname.Mid(colon+1).AsUnsigned();
    if (portnum == 0)
      serverHost = hostname;
    else {
      serverHost = hostname.Left(colon);
      port = (WORD)portnum;
    }
  }

  if (port == 0)
    port = DefaultServerPort;

  serverPort = port;

  return TRUE;
}


void PSocksSocket::SetAuthentication(const PString & username, const PString & password)
{
  PAssert(authenticationUsername.GetLength() < 256, PInvalidParameter);
  authenticationUsername = username;
  PAssert(authenticationPassword.GetLength() < 256, PInvalidParameter);
  authenticationPassword = password;
}


BOOL PSocksSocket::ConnectServer()
{
  Address ipnum;
  if (!GetHostAddress(serverHost, ipnum))
    return FALSE;

  remotePort = port;
  port = serverPort;
  BOOL ok = PTCPSocket::Connect(0, ipnum);
  port = remotePort;
  return ok;
}


///////////////////////////////////////////////////////////////////////////////

PSocks5Socket::PSocks5Socket(WORD port)
  : PSocksSocket(port)
{
}


PSocks5Socket::PSocks5Socket(const PString & host, WORD port)
  : PSocksSocket(port)
{
  Connect(host);
}


PObject * PSocks5Socket::Clone() const
{
  return new PSocks5Socket(remotePort);
}


BOOL PSocks5Socket::ConnectServer()
{
  if (!PSocksSocket::ConnectServer())
    return FALSE;

  *this << SOCKS_VERSION
    << (authenticationUsername.IsEmpty() ? '\001' : '\002') // length
    << SOCKS_AUTH_NONE;
  if (!authenticationUsername)
    *this << SOCKS_AUTH_USER_PASS;  // Simple cleartext username/password
  flush();

  BYTE auth_pdu[2];
  if (!ReadBlock(auth_pdu, sizeof(auth_pdu)))  // Should get 2 byte reply
    return FALSE;

  if (auth_pdu[0] != SOCKS_VERSION || auth_pdu[1] == SOCKS_AUTH_FAILED) {
    Close();
    lastError = AccessDenied;
    osError = EACCES;
    return FALSE;
  }

  if (auth_pdu[1] == SOCKS_AUTH_USER_PASS) {
    *this << SOCKS_VERSION
          << (char)authenticationUsername.GetLength()  // Username length as single byte
          << authenticationUsername
          << (char)authenticationPassword.GetLength()  // Password length as single byte
          << authenticationPassword
          << ::flush;
    if (!ReadBlock(auth_pdu, sizeof(auth_pdu)))  // Should get 2 byte reply
      return FALSE;
    if (auth_pdu[0] != SOCKS_VERSION || auth_pdu[1] != 0) {
      Close();
      lastError = AccessDenied;
      osError = EACCES;
      return FALSE;
    }
  }

  return TRUE;
}


BOOL PSocks5Socket::SendSocksCommand(BYTE command, const char * hostname, Address addr)
{
  if (!IsOpen())
    return FALSE;

  *this << SOCKS_VERSION
        << command
        << '\000'; // Reserved
  if (hostname != NULL)
    *this << SOCKS_ADDR_DOMAINNAME << (BYTE)strlen(hostname) << hostname;
  else
    *this << SOCKS_ADDR_IPV4
          << addr.Byte1() << addr.Byte2() << addr.Byte3() << addr.Byte4();
  *this << (BYTE)(remotePort >> 8) << (BYTE)remotePort
        << ::flush;

  return ReceiveSocksResponse(localAddress, localPort);
}


BOOL PSocks5Socket::ReceiveSocksResponse(Address & addr, WORD & port)
{
  int reply;
  if ((reply = ReadChar()) < 0)
    return FALSE;

  if (reply != SOCKS_VERSION) {
    lastError = Miscellaneous;
    osError = EINVAL;
    return FALSE;
  }

  if ((reply = ReadChar()) < 0)
    return FALSE;

  switch (reply) {
    case 0 :  // No error
      break;

    case 2 :  // Refused permission
      lastError = AccessDenied;
      osError = EACCES;
      return FALSE;

    case 3 : // Network unreachable
      lastError = NotFound;
      osError = ENETUNREACH;
      return FALSE;

    case 4 : // Host unreachable
      lastError = NotFound;
      osError = EHOSTUNREACH;
      return FALSE;

    case 5 : // Connection refused
      lastError = NotFound;
      osError = EHOSTUNREACH;
      return FALSE;

    default :
      lastError = Miscellaneous;
      osError = EINVAL;
      return FALSE;
  }

  // Ignore next byte (reserved)
  if ((reply = ReadChar()) < 0)
    return FALSE;

  // Get type byte for bound address
  if ((reply = ReadChar()) < 0)
    return FALSE;

  switch (reply) {
    case SOCKS_ADDR_DOMAINNAME :
      // Get length
      if ((reply = ReadChar()) < 0)
        return FALSE;

      if (!GetHostAddress(ReadString(reply), addr))
        return FALSE;
      break;

    case SOCKS_ADDR_IPV4 :
      if (!ReadBlock(&addr, sizeof(addr)))
        return FALSE;
      break;

    default :
      lastError = Miscellaneous;
      osError = EINVAL;
      return FALSE;
  }

  WORD rxPort;
  if (!ReadBlock(&rxPort, sizeof(rxPort)))
    return FALSE;

  port = Net2Host(rxPort);
  return TRUE;
}


///////////////////////////////////////////////////////////////////////////////

PSocksUDPSocket::PSocksUDPSocket(WORD port)
{
}


PSocksUDPSocket::PSocksUDPSocket(const PString & host, WORD port)
{
}


BOOL PSocksUDPSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  PBYTEArray newbuf(len+262);
  Address rx_addr;
  WORD rx_port;
  if (!PUDPSocket::ReadFrom(newbuf.GetPointer(), newbuf.GetSize(), rx_addr, rx_port))
    return FALSE;

  if (rx_addr != serverAddress || rx_port != serverPort)
    return FALSE;

  return TRUE;
}


BOOL PSocksUDPSocket::WriteTo(const void * buf, PINDEX len, const Address & addr, WORD port)
{
  PBYTEArray newbuf(len+262);

  return PUDPSocket::WriteTo(newbuf, newbuf.GetSize(), serverAddress, serverPort);
}


// End of File ///////////////////////////////////////////////////////////////
