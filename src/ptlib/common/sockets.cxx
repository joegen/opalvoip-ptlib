/*
 * $Id: sockets.cxx,v 1.6 1995/01/03 09:37:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.6  1995/01/03 09:37:52  robertj
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


//////////////////////////////////////////////////////////////////////////////
// PSocket

BOOL PSocket::Open (const PString &, WORD)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


BOOL PSocket::Accept (const PString &)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

BOOL PIPSocket::GetAddress(Address & addr)
{
  return ConvertOSError(GetAddress(GetName(), addr) ? -1 : 0);
}


BOOL PIPSocket::GetAddress(const PString & hostname, Address & addr)
{
  // lookup the host address using inet_addr, assuming it is a "." address
  long temp;
  if ((temp = inet_addr(hostname)) != -1)
    memcpy(addr, &temp, sizeof(addr));
  else {
    // otherwise lookup the name as a host name
    struct hostent * host_info;
    if ((host_info = gethostbyname(hostname)) != 0)
      memcpy(addr, host_info->h_addr, sizeof(addr));
    else
      return FALSE;
  }

  return TRUE;
}


PStringArray PIPSocket::GetHostAliases() const
{
  return GetHostAliases(GetName());
}

PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;
  struct hostent * host_info;

  // lookup the host address using inet_addr, assuming it is a "." address
  long temp;
  if ((temp = inet_addr(hostname)) != -1)
    host_info = gethostbyaddr((const char *)&temp, 4, PF_INET);
  else
    host_info = gethostbyname(hostname);

  if (host_info != NULL) {
    int i = 0;
    while (host_info->h_aliases[i] != NULL)
      aliases[i] = host_info->h_aliases[i];
  }

  return aliases;
}


#endif


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PTCPSocket::PTCPSocket(WORD newPort)
{
  port = newPort;
}

PTCPSocket::PTCPSocket(const PString & address, WORD port)
{
  Open(address, port);
}


BOOL PTCPSocket::Open(const PString & host, WORD newPort)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  if (newPort != 0)
    port = newPort;
  PAssert(port != 0, "Cannot open socket without setting port");

  // attempt to lookup the host name
  Address ipnum;
  if (!GetAddress(host, ipnum))
    return FALSE;

  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = ::htons(port);  // set the port
  memcpy(&address.sin_addr, ipnum, sizeof(address.sin_addr));

  // attempt to create a socket
  if (!ConvertOSError(os_handle = ::socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // attempt to connect
  if (!ConvertOSError(::connect(os_handle,
                              (struct sockaddr *)&address, sizeof(address)))) {
    Close();
    return FALSE;
  }

  // make the socket non-blocking
#ifndef WIN32
  DWORD cmd = 1;
#ifdef _WINDOWS
  ::ioctlsocket (os_handle, FIONBIO, &cmd);
#else
  ::ioctl (os_handle, FIONBIO, &cmd);
#endif
#endif

  return TRUE;
}


#endif

void PTCPSocket::OnOutOfBand(const void *, PINDEX)
{
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

void PTCPSocket::SetPort(WORD newPort)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = newPort;
}

void PTCPSocket::SetPort(const PString & service)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = GetPort(service);
}


WORD PTCPSocket::GetPort() const
{
  return port;
}


PString PTCPSocket::GetService() const
{
  return GetService(port);
}


WORD PTCPSocket::GetPort(const PString & serviceName) const
{
  struct servent * service = ::getservbyname((const char *)serviceName, "tcp");
  if (service != NULL)
    return service->s_port;
  else
    return 0;
}


PString PTCPSocket::GetService(WORD port) const
{
  struct servent * service = ::getservbyport(port, "tcp");
  if (service != NULL)
    return PString(service->s_name);
  else
    return PString();
}


//////////////////////////////////////////////////////////////////////////////
// PTelnetSocket

PTelnetSocket::PTelnetSocket(WORD newPort)
  : PTCPSocket(newPort)
{
  Construct();
}


PTelnetSocket::PTelnetSocket(const PString & address, WORD port)
  : PTCPSocket(address, port)
{
  Construct();
}


void PTelnetSocket::Construct()
{
  state = StateNormal;
  memset(willOptions, 0, sizeof(willOptions));
  memset(doOptions, 0, sizeof(doOptions));
  debug = TRUE;
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
      switch (state) {
        case StateIAC:
          switch (*src) {
            case IAC:
              state = StateNormal;
              return IAC;

            case DO:
              state = StateDo;
              break;

            case DONT:
              state = StateDont;
              break;

            case WILL:
              state = StateWill;
              break;

            case WONT:
              state = StateWont;
              break;

            case SE :          // subnegotiation end
            case NOP :         // no operation
            case DataMark :    // data stream portion of a Synch
            case Break :       // NVT character break
            case Interrupt :   // The function IP
            case Abort :       // The function AO
            case AreYouThere : // The function AYT
            case EraseChar :   // The function EC
            case EraseLine :   // The function EL
            case GoAhead :     // The function GA
            case SB :          // subnegotiation start
              state = StateNormal;
              break;

            default:
              if (OnUnknownCommand(*src))
                state = StateNormal;
              break;
          }
          break;

        case StateDo:
          OnDo(*src);
          state = StateNormal;
          break;

        case StateDont:
          OnDont(*src);
          state = StateNormal;
          break;

        case StateWill:
          OnWill(*src);
          state = StateNormal;
          break;

        case StateWont:
          OnWont(*src);
          state = StateNormal;
          break;

        case StateNormal:
        default:
          if (*src == IAC)
            state = StateIAC;
          else {
            if (doOptions[TransmitBinary])
              *dst++ = *src;
            else
              *dst++ = (BYTE)(0x7f&*src);
            charsLeft--;
          }
          break;
      }
      lastReadCount--;
      src++;
    }
  }
  lastReadCount = bytesToRead;
  return TRUE;
}

BOOL PTelnetSocket::Write(void const * buffer, PINDEX length)
{
  const char * bufptr = (const char *)buffer;
  int count = 0;

  while (length > 0) {

    // get ptr to first IAC character
    const char * iacptr = (const char *)memchr(bufptr, IAC, length);

    // calculate number of bytes to send with or without the trailing IAC
    PINDEX iaclen = iacptr != NULL ? iacptr - bufptr : length;

    // send the characters
    if (!PTCPSocket::Write(bufptr, iaclen))
      return FALSE;
    count += lastWriteCount;

    // send the IAC (if required)
    if (bufptr != NULL) {
      // Note: cannot use WriteChar(), so send the IAC found again
      if (!PTCPSocket::Write(bufptr, 1))
        return FALSE;
      count += lastWriteCount;
    }

    length -= iaclen;
    bufptr += iaclen;
  }

  lastWriteCount = count;
  return TRUE;
}

#define IMPLEMENT_SEND_CMD(n,c) \
  void PTelnetSocket::Send ##n## (BYTE code) \
  { BYTE cmd[3] = {IAC, c}; cmd[2] = code; PTCPSocket::Write(cmd, 3);  }

IMPLEMENT_SEND_CMD(Will, WILL)
IMPLEMENT_SEND_CMD(Wont, WONT)
IMPLEMENT_SEND_CMD(Do,   DO)
IMPLEMENT_SEND_CMD(Dont, DONT)

static struct {
  BOOL can;
  char * name;
} KnownTELNETOptions[PTelnetSocket::MaxOptions] = 
  { 
    { TRUE,  "TransmitBinary" },
    { TRUE,  "Echo" },
    { FALSE, NULL },
    { TRUE,  "SuppressGoAhead" },
    { FALSE, NULL },
    { TRUE,  "Status" },
    { TRUE,  "TimingMark" }
  };

static PString GetTELNETOptionName(int code)
{
  if (code < PTelnetSocket::MaxOptions && KnownTELNETOptions[code].name != NULL)
    return KnownTELNETOptions[code].name;

  return PString(PString::Unsigned, code);
}


void PTelnetSocket::OnDo(BYTE code)
{
  if (debug)
    PError << "DO " << GetTELNETOptionName(code) << endl;
  if (code < MaxOptions && KnownTELNETOptions[code].can) {
    willOptions[code] = TRUE;
    SendWill(code);
  }
  else
    SendWont(code);
}


void PTelnetSocket::OnDont(BYTE code)
{
  if (debug)
    PError << "DONT " << GetTELNETOptionName(code) << endl;
  if (code < MaxOptions && KnownTELNETOptions[code].can)
    willOptions[code] = FALSE;
}


void PTelnetSocket::OnWill(BYTE code)
{
  if (debug)
    PError << "WILL " << GetTELNETOptionName(code) << endl;
  if (code < MaxOptions && KnownTELNETOptions[code].can) {
    doOptions[code] = TRUE;
    SendDo(code);
  }
  else
    SendDont(code);
}


void PTelnetSocket::OnWont(BYTE code)
{
  if (debug)
    PError << "WONT " << GetTELNETOptionName(code) << endl;
  if (code < MaxOptions && KnownTELNETOptions[code].can)
    doOptions[code] = FALSE;
}


BOOL PTelnetSocket::OnUnknownCommand(BYTE code)
{
  if (debug)
    PError << "unknown telnet command " << (int)code << endl;
  return TRUE;
}

void PTelnetSocket::OnOutOfBand(const void *, PINDEX length)
{
  PError << "Out of band data received of length " << length << endl;
}


void PTelnetSocket::SendDataMark(Command ch)
{
  BYTE dataMark[2] = { DataMark };

  // send the datamark character as the only out of band data byte
  WriteOutOfBand(dataMark, 1);

  // insert a datamark into the outgoing data stream
  dataMark[0] = IAC;
  dataMark[1] = ch;
  PTCPSocket::Write(dataMark, 2);
}


// End Of File ///////////////////////////////////////////////////////////////
