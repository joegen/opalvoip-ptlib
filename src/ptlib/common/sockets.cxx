/*
 * $Id: sockets.cxx,v 1.15 1995/06/04 12:45:33 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
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


//////////////////////////////////////////////////////////////////////////////
// PSocket

BOOL PSocket::Open(const PString &)
{
  PAssertAlways(PLogicError);
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PString PIPSocket::GetName() const
{
  PString name;
  sockaddr_in address;
  int size = sizeof(address);
  if (getpeername(os_handle,(struct sockaddr*)&address,&size) == 0){
    struct hostent * host_info = gethostbyaddr(
           (const char *)&address.sin_addr, sizeof(address.sin_addr), PF_INET);
    name = host_info != NULL ? host_info->h_name : inet_ntoa(address.sin_addr);
    name += " " + PString(PString::Unsigned, ntohs(address.sin_port));
  }
  return name;
}


BOOL PIPSocket::GetAddress(const PString & hostname, Address & addr)
{
  // lookup the host address using inet_addr, assuming it is a "." address
  long temp;
  if ((temp = inet_addr((const char *)hostname)) != -1)
    memcpy(&addr, &temp, sizeof(addr));
  else {
    // otherwise lookup the name as a host name
    struct hostent * host_info;
    if ((host_info = gethostbyname(hostname)) != 0)
      memcpy(&addr, host_info->h_addr, sizeof(addr));
    else
      return FALSE;
  }

  return TRUE;
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;
  struct hostent * host_info;

  // lookup the host address using inet_addr, assuming it is a "." address
  long temp;
  if ((temp = inet_addr((const char *)hostname)) != -1)
    host_info = gethostbyaddr((const char *)&temp, sizeof(temp), PF_INET);
  else
    host_info = gethostbyname(hostname);

  if (host_info != NULL) {
    int i = 0;
    while (host_info->h_aliases[i] != NULL)
      aliases[i] = host_info->h_aliases[i];
  }

  return aliases;
}


BOOL PIPSocket::GetLocalAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  memcpy(&addr, &address.sin_addr, sizeof(address.sin_addr));
  return TRUE;
}


BOOL PIPSocket::GetPeerAddress(Address & addr)
{
  sockaddr_in address;
  int size = sizeof(address);
  if (!ConvertOSError(getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  memcpy(&addr, &address.sin_addr, sizeof(address.sin_addr));
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


#endif


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#ifdef P_HAS_BERKELEY_SOCKETS

PTCPSocket::PTCPSocket(WORD newPort)
{
  port = newPort;
}


PTCPSocket::PTCPSocket(const PString & service)
{
  SetPort(service);
}


PTCPSocket::PTCPSocket(const PString & address, WORD port)
{
  SetPort(port);
  Open(address);
}


PTCPSocket::PTCPSocket(const PString & address, const PString & service)
{
  SetPort(service);
  Open(address);
}


PTCPSocket::PTCPSocket(PSocket & socket)
{
  Open(socket);
}


BOOL PTCPSocket::Open(const PString & host)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot open socket without setting port");

  // attempt to lookup the host name
  Address ipnum;
  if (!GetAddress(host, ipnum))
    return FALSE;

  // attempt to create a socket
  if (!ConvertOSError(os_handle = ::socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // attempt to connect
  sockaddr_in address;
  address.sin_family = AF_INET;
  address.sin_port = htons(port);  // set the port
  address.sin_addr = ipnum;
  if (!ConvertOSError(::connect(os_handle,
                              (struct sockaddr *)&address, sizeof(address)))) {
    Close();
    return FALSE;
  }

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


BOOL PTCPSocket::Open(WORD newPort)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  if (newPort != 0)
    port = newPort;
  PAssert(port != 0, "Cannot open socket without setting port");

  // attempt to create a socket
  if (!ConvertOSError(os_handle = ::socket(AF_INET, SOCK_STREAM, 0)))
    return FALSE;

  // attempt to listen
  sockaddr_in sin;
  sin.sin_family = AF_INET;
  sin.sin_port = htons(port);  // set the port
  memset(&sin.sin_addr, 0, sizeof(sin.sin_addr));

  if (ConvertOSError(::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin))) &&
      ConvertOSError(::listen(os_handle, 5)))
    return TRUE;

  Close();
  return FALSE;
}


BOOL PTCPSocket::Open(PSocket & socket)
{
  // attempt to create a socket
  return ConvertOSError(os_handle = ::accept(socket.GetHandle(), NULL, 0));
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


WORD PTCPSocket::GetPort(const PString & serviceName) const
{
  struct servent * service = ::getservbyname((const char *)serviceName, "tcp");
  if (service != NULL)
    return ntohs(service->s_port);
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


#endif


void PTCPSocket::OnOutOfBand(const void *, PINDEX)
{
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
  if (port == 0)
    port = (WORD)service.AsInteger();
}


WORD PTCPSocket::GetPort() const
{
  return port;
}


PString PTCPSocket::GetService() const
{
  return GetService(port);
}


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
  Open(address);
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

BOOL PTelnetSocket::Open(const PString & host)
{
  PTelnetError << "open" << endl;

  if (!PTCPSocket::Open(host))
    return FALSE;

  SendDo(SuppressGoAhead);
  SendDo(StatusOption);
  SendWill(TerminalSpeed);
  return TRUE;
}


BOOL PTelnetSocket::Open(WORD newPort)
{
  return PTCPSocket::Open(newPort);
}


BOOL PTelnetSocket::Open(PSocket & sock)
{
  if (!PTCPSocket::Open(sock))
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


//////////////////////////////////////////////////////////////////////////////
// PApplicationSocket

PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       WORD port)
  : PTCPSocket(port), commandNames(cmdCount, cmdNames)
{
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & service)
  : PTCPSocket(service), commandNames(cmdCount, cmdNames)
{
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       WORD port)
  : PTCPSocket(address, port), commandNames(cmdCount, cmdNames)
{
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       const PString & service)
  : PTCPSocket(address, service), commandNames(cmdCount, cmdNames)
{
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       PSocket & socket)
  : PTCPSocket(socket), commandNames(cmdCount, cmdNames)
{
}


BOOL PApplicationSocket::WriteLine(const PString & line)
{
  if (line.FindOneOf("\r\n") == P_MAX_INDEX)
    return WriteString(line + "\r\n");

  PStringArray lines = line.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); i++)
    if (!WriteString(lines[i] + "\r\n"))
      return FALSE;

  return TRUE;
}


BOOL PApplicationSocket::ReadLine(PString & str)
{
  PCharArray line(100);
  PINDEX count = 0;
  int c;
  while ((c = ReadChar()) >= 0) {
    switch (c) {
      case '\b' :
      case '\177' :
        if (count > 0)
          count--;
        break;

      case '\r' :
      case '\n' :
        if (count > 0) {
          str = PString(line, count);
          return TRUE;
        }
        break;

      default :
        if (count >= line.GetSize())
          line.SetSize(count + 100);
        line[count++] = (char)c;
    }
  }

  if (count > 0)
    str = PString(line, count);
  else
    str = PString();

  return FALSE;
}


BOOL PApplicationSocket::WriteCommand(PINDEX cmdNumber,  const PString & param)
{
  if (cmdNumber >= commandNames.GetSize())
    return FALSE;
  return WriteLine(commandNames[cmdNumber] + " " + param);
}


PINDEX PApplicationSocket::ReadCommand(PString & args)
{
  if (!ReadLine(args))
    return P_MAX_INDEX;

  PINDEX endCommand = args.Find(' ');
  if (endCommand == P_MAX_INDEX)
    endCommand = args.GetLength();
  PCaselessString cmd = args.Left(endCommand);

  PINDEX num = commandNames.GetValuesIndex(cmd);
  if (num != P_MAX_INDEX)
    args = args.Mid(endCommand);

  return num;
}


BOOL PApplicationSocket::WriteResponse(unsigned code, const PString & info)
{
  return WriteResponse(psprintf("%03u", code), info);
}


BOOL PApplicationSocket::WriteResponse(const PString & code,
                                       const PString & info)
{
  if (info.FindOneOf("\r\n") == P_MAX_INDEX)
    return WriteString(code + ' ' + info + "\r\n");

  PStringArray lines = info.Lines();
  for (PINDEX i = 0; i < lines.GetSize()-1; i++)
    if (!WriteString(code + '-' + lines[i] + "\r\n"))
      return FALSE;

  return WriteString(code + ' ' + lines[i] + "\r\n");
}


BOOL PApplicationSocket::ReadResponse()
{
  return ReadResponse(lastResponseCode, lastResponseInfo);
}


BOOL PApplicationSocket::ReadResponse(PString & code, PString & info)
{
  PString line;
  if (!ReadLine(line))
    return FALSE;

  PINDEX endCode = line.FindOneOf(" -", 1);
  if (endCode == P_MAX_INDEX) {
    code = line;
    info = PString();
    return TRUE;
  }

  code = line.Left(endCode);
  info = line.Mid(endCode+1);

  while (line[endCode] == '-') {
    info += '\n';
    if (!ReadLine(line))
      return FALSE;
    info += line.Mid(endCode+1);
  }

  return TRUE;
}


char PApplicationSocket::ExecuteCommand(PINDEX cmdNumber,
                                        const PString & param)
{
  if (!WriteCommand(cmdNumber, param))
    return '\0';

  if (!ReadResponse())
    return '\0';

  return lastResponseCode[0];
}



//////////////////////////////////////////////////////////////////////////////
// PSMTPSocket

static char const * SMTPCommands[PSMTPSocket::NumCommands] = {
  "HELO", "EHLO", "QUIT", "HELP", "NOOP",
  "TURN", "RSET", "VRFY", "EXPN", "RCPT",
  "MAIL", "SEND", "SAML", "SOML", "DATA"
};


PSMTPSocket::PSMTPSocket()
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp")
{
  Reset();
}


PSMTPSocket::PSMTPSocket(const PString & address)
  : PApplicationSocket(NumCommands, SMTPCommands, address, "smtp")
{
  Reset();
}


PSMTPSocket::PSMTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, SMTPCommands, socket)
{
  Reset();
}


void PSMTPSocket::Reset()
{
  haveHello = FALSE;
  extendedHello = FALSE;
  sendingData = FALSE;
  eightBitMIME = FALSE;
  sendCommand = WasMAIL;
  fromName = PString();
  toNames.RemoveAll();
  messageBufferSize = 30000;
}


BOOL PSMTPSocket::Write(const void * buf, PINDEX len)
{
  if (!sendingData)
    return PApplicationSocket::Write(buf, len);

  const char * base = (const char *)buf;
  const char * current = base;
  while (len-- > 0) {
    switch (stuffingState) {
      case GotNothing :
        switch (*current) {
          case '\r' :
            stuffingState = GotCR;
            break;
          case '\n' :
            if (!eightBitMIME) {
              if (current > base)
                if (!PApplicationSocket::Write(base, current - base))
                  return FALSE;
              if (!PApplicationSocket::Write("\r", 1))
                return FALSE;
              base = current;
            }
        }
        break;

      case GotCR :
        stuffingState = *current != '\n' ? GotNothing : GotCRLF;
        break;

      case GotCRLF :
        if (*current == '.') {
          if (current > base)
            if (!PApplicationSocket::Write(base, current - base))
              return FALSE;
          if (!PApplicationSocket::Write(".", 1))
            return FALSE;
          base = current;
        }
        // Then do default state

      default :
        stuffingState = GotNothing;
        break;
    }
    current++;
  }
  return FALSE;
}


BOOL PSMTPSocket::BeginMessage(const PString & from,
                               const PString & to,
                               BOOL useEightBitMIME)
{
  fromName = from;
  toNames.RemoveAll();
  toNames.AppendString(to);
  eightBitMIME = useEightBitMIME;
  return _BeginMessage();
}


BOOL PSMTPSocket::BeginMessage(const PString & from,
                               const PStringList & toList,
                               BOOL useEightBitMIME)
{
  fromName = from;
  toNames = toList;
  eightBitMIME = useEightBitMIME;
  return _BeginMessage();
}


BOOL PSMTPSocket::_BeginMessage()
{
  if (!haveHello) {
    if (ExecuteCommand(EHLO, GetLocalHostName()) == '2')
      haveHello = extendedHello = TRUE;
  }

  if (!haveHello) {
    extendedHello = FALSE;
    if (eightBitMIME)
      return FALSE;
    if (ExecuteCommand(HELO, GetLocalHostName()) != '2')
      return FALSE;
    haveHello = TRUE;
  }

  for (PINDEX i = 0; i < toNames.GetSize(); i++) {
    if (toNames[i].Find('@') == P_MAX_INDEX)
      toNames[i] += GetPeerHostName();
    if (ExecuteCommand(RCPT, toNames[i]) != '2')
      return FALSE;
  }

  if (fromName.Find('@') == P_MAX_INDEX)
    fromName += GetLocalHostName();
  if (ExecuteCommand(MAIL, fromName) != '2')
    return FALSE;

  if (ExecuteCommand(DATA, PString()) != '3')
    return FALSE;

  sendingData = TRUE;
  stuffingState = GotNothing;
  return TRUE;
}


BOOL PSMTPSocket::EndMessage()
{
  sendingData = FALSE;
  return PApplicationSocket::Write("\r\n.\r\n", 5);
}


BOOL PSMTPSocket::ProcessCommand()
{
  PString args;
  switch (ReadCommand(args)) {
    case HELO :
      OnHELO(args);
      break;
    case EHLO :
      OnEHLO(args);
      break;
    case QUIT :
      OnQUIT();
      return FALSE;
    case NOOP :
      OnNOOP();
      break;
    case TURN :
      OnTURN();
      break;
    case RSET :
      OnRSET();
      break;
    case VRFY :
      OnVRFY(args);
      break;
    case EXPN :
      OnEXPN(args);
      break;
    case RCPT :
      OnRCPT(args);
      break;
    case MAIL :
      OnMAIL(args);
      break;
    case SEND :
      OnSEND(args);
      break;
    case SAML :
      OnSAML(args);
      break;
    case SOML :
      OnSOML(args);
      break;
    case DATA :
      OnDATA();
      break;
    default :
      return OnUnknown(args);
  }

  return TRUE;
}


void PSMTPSocket::OnHELO(const PCaselessString & remoteHost)
{
  extendedHello = FALSE;
  Reset();

  PCaselessString peer = GetPeerHostName();

  PString response = GetLocalHostName() + " Hello " + peer + ", ";

  if (remoteHost == peer)
    response += ", pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response);
}


void PSMTPSocket::OnEHLO(const PCaselessString & remoteHost)
{
  extendedHello = TRUE;
  Reset();

  PCaselessString peer = GetPeerHostName();

  PString response = GetLocalHostName() + " Hello " + peer + ", ";

  if (remoteHost == peer)
    response += ", pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response +
               "\nHELP\nVERB\nONEX\nMULT\nEXPN\nTICK\n8BITMIME\n");
}


void PSMTPSocket::OnQUIT()
{
  WriteResponse(221, GetLocalHostName() + " closing connection, goodbye.");
  Close();
}


void PSMTPSocket::OnHELP()
{
  WriteResponse(214, "No help here.");
}


void PSMTPSocket::OnNOOP()
{
  WriteResponse(250, "Ok");
}


void PSMTPSocket::OnTURN()
{
  WriteResponse(502, "I don't do that yet. Sorry.");
}


void PSMTPSocket::OnRSET()
{
  Reset();
  WriteResponse(250, "Reset state.");
}


void PSMTPSocket::OnVRFY(const PCaselessString & name)
{
  PString expandedName;
  switch (LookUpName(name, expandedName)) {
    case AmbiguousUser :
      WriteResponse(553, "User ambiguous.");
      break;

    case ValidUser :
      WriteResponse(250, expandedName);
      break;

    case UnknownUser :
      WriteResponse(550, "String does not match anything.");
      break;

    default :
      WriteResponse(550, "Error verifying user.");
  }
}


void PSMTPSocket::OnEXPN(const PCaselessString &)
{
  WriteResponse(502, "I don't do that. Sorry.");
}


static PINDEX ExtractName(const PCaselessString & args,
                          const PCaselessString & subCmd,
                          PString & name)
{
  PINDEX colon = args.Find(':');
  if (colon == P_MAX_INDEX)
    return 0;

  PCaselessString word = args.Left(colon).Trim();
  if (subCmd != word)
    return 0;

  PINDEX leftAngle = args.Find('<', colon);
  if (leftAngle == P_MAX_INDEX)
    return 0;

  PINDEX rightAngle = args.Find('>', leftAngle);
  if (rightAngle == P_MAX_INDEX)
    return 0;

  name = args(leftAngle+1, rightAngle-1);
  return rightAngle+1;
}


void PSMTPSocket::OnRCPT(const PCaselessString & recipient)
{
  PCaselessString toName;
  if (ExtractName(recipient, "to", toName) == 0)
    WriteResponse(501, "Syntax error.");
  else if (toName.Find(':') != P_MAX_INDEX)
    WriteResponse(550, "Cannot do forwarding.");
  else {
    PString expandedName;
    switch (LookUpName(toName, expandedName)) {
      case ValidUser :
        WriteResponse(553, "Recipient " + toName + " Ok");
        toNames.AppendString(toName);
        break;

      case AmbiguousUser :
        WriteResponse(553, "User ambiguous.");
        break;

      case UnknownUser :
        WriteResponse(550, "User unknown.");
        break;

      default :
        WriteResponse(550, "Error verifying user.");
    }
  }
}


void PSMTPSocket::OnMAIL(const PCaselessString & sender)
{
  sendCommand = WasMAIL;
  OnSendMail(sender);
}


void PSMTPSocket::OnSEND(const PCaselessString & sender)
{
  sendCommand = WasSEND;
  OnSendMail(sender);
}


void PSMTPSocket::OnSAML(const PCaselessString & sender)
{
  sendCommand = WasSAML;
  OnSendMail(sender);
}


void PSMTPSocket::OnSOML(const PCaselessString & sender)
{
  sendCommand = WasSOML;
  OnSendMail(sender);
}


void PSMTPSocket::OnSendMail(const PCaselessString & sender)
{
  if (!fromName.IsEmpty()) {
    WriteResponse(503, "Sender already specified.");
    return;
  }

  PINDEX extendedArgPos = ExtractName(sender, "from", fromName);
  if (extendedArgPos == 0) {
    WriteResponse(501, "Syntax error.");
    return;
  }

  if (extendedHello) {
    PINDEX equalPos = sender.Find('=', extendedArgPos);
    PCaselessString body = sender(extendedArgPos, equalPos).Trim();
    PCaselessString mime = sender.Mid(equalPos+1).Trim();
    eightBitMIME = (body == "BODY" && mime == "8BITMIME");
  }

  PString response = "Sender " + fromName;
  if (eightBitMIME)
    response += " and 8BITMIME";
  WriteResponse(250, response + " Ok");
}


void PSMTPSocket::OnDATA()
{
  if (fromName.IsEmpty()) {
    WriteResponse(503, "Need a valid MAIL command.");
    return;
  }

  if (toNames.GetSize() == 0) {
    WriteResponse(503, "Need a valid RCPT command.");
    return;
  }

  // Ok, everything is ready to start the message.
  stuffingState = GotNothing;
  BOOL ok = TRUE;
  PCharArray buffer;
  if (eightBitMIME) {
    WriteResponse(354,
                "Enter 8BITMIME message, terminate with '<CR><LF>.<CR><LF>'.");
    while (ok && OnMimeData(buffer))
      ok = HandleMessage(buffer, FALSE);
  }
  else {
    WriteResponse(354, "Enter mail, terminate with '.' alone on a line.");
    while (ok && OnTextData(buffer))
      ok = HandleMessage(buffer, FALSE);
  }

  if (ok && HandleMessage(buffer, TRUE))
    WriteResponse(250, "Message received Ok.");
  else
    WriteResponse(554, "Message storage failed.");
}


BOOL PSMTPSocket::OnUnknown(const PCaselessString & command)
{
  WriteResponse(500, "Command \"" + command + "\" unrecognised.");
  return TRUE;
}


PSMTPSocket::LookUpResult PSMTPSocket::LookUpName(
                               const PCaselessString &, PString & expandedName)
{
  expandedName = PString();
  return LookUpError;
}


BOOL PSMTPSocket::OnTextData(PCharArray & buffer)
{
  PString line;
  while (line != ".") {
    if (!ReadLine(line))
      return FALSE;
    if (line.GetLength() > 1 && line[0] == '.' && line[1] == '.')
      line.Delete(0, 1);
    line += '\n';
    PINDEX size = buffer.GetSize();
    PINDEX len = line.GetLength();
    memcpy(buffer.GetPointer(size + len) + size, (const char *)line, len);
    if (size + len > messageBufferSize)
      return TRUE;
  }
  return FALSE;
}


BOOL PSMTPSocket::OnMimeData(PCharArray & buffer)
{
  int count = 0;
  int c;
  while ((c = ReadChar()) >= 0) {
    if (count >= buffer.GetSize())
      buffer.SetSize(count + 100);
    switch (stuffingState) {
      case GotNothing :
        buffer[count++] = (char)c;
        break;

      case GotCR :
        stuffingState = c != '\n' ? GotNothing : GotCRLF;
        buffer[count++] = (char)c;
        break;

      case GotCRLF :
        if (c == '.')
          stuffingState = GotCRLFdot;
        else {
          stuffingState = GotNothing;
          buffer[count++] = (char)c;
        }
        break;

      case GotCRLFdot :
        switch (c) {
          case '\r' :
            stuffingState = GotCRLFdotCR;
            break;

          case '.' :
            stuffingState = GotNothing;
            buffer[count++] = (char)c;
            break;

          default :
            stuffingState = GotNothing;
            buffer[count++] = '.';
            buffer[count++] = (char)c;
        }
        break;

      case GotCRLFdotCR :
        if (c == '\n')
          return FALSE;
        buffer[count++] = '.';
        buffer[count++] = '\r';
        buffer[count++] = (char)c;
        stuffingState = GotNothing;
    }
    if (count > messageBufferSize) {
      buffer.SetSize(messageBufferSize);
      return TRUE;
    }
  }

  return FALSE;
}


BOOL PSMTPSocket::HandleMessage(PCharArray &, BOOL)
{
  return FALSE;
}


// End Of File ///////////////////////////////////////////////////////////////
