/*
 * $Id: sockets.cxx,v 1.10 1995/02/21 11:25:29 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: sockets.cxx,v $
 * Revision 1.10  1995/02/21 11:25:29  robertj
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
  if ((temp = inet_addr((const char *)hostname)) != -1)
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
  if ((temp = inet_addr((const char *)hostname)) != -1)
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
  address.sin_port = htons(port);  // set the port
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

PTelnetSocket::PTelnetSocket(WORD newPort)
  : PTCPSocket(newPort)
{
  Construct();
}


PTelnetSocket::PTelnetSocket(const PString & address, WORD port)
  : PTCPSocket(port)
{
  Construct();
  Open(address);
}


void PTelnetSocket::Construct()
{
  synchronising = 0;
  terminalType = "UNKNOWN";
  state = StateNormal;
  memset(option, 0, sizeof(option));

  if (port == 0)
    port = GetPort("telnet");

#ifdef _DEBUG
  debug = TRUE;
#endif
}


#define PTelnetError if (debug) PError << "PTelnetSocket: "

BOOL PTelnetSocket::Open(const PString & host, WORD newPort)
{
  if (!PTCPSocket::Open(host, newPort))
    return FALSE;

  PTelnetError << "open" << endl;

  SendDo(SuppressGoAhead, TRUE);
  SendWill(TerminalType, TRUE);
  SendWill(WindowSize, TRUE);
  SendWill(TerminalSpeed, TRUE);
  SendDo(StatusOption, TRUE);
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


void PTelnetSocket::SendCommand(Command cmd, int opt)
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
      PTCPSocket::Write(buffer, 3);
      break;

    case InterruptProcess :
    case Break :
    case AbortProcess :
    case SuspendProcess :
    case AbortOutput :
      if (opt) {
        // Send the command
        PTCPSocket::Write(buffer, 2);
        // Send a TimingMark for output flush.
        buffer[1] = TimingMark;
        PTCPSocket::Write(buffer, 2);
        // Send a DataMark for synchronisation.
        if (cmd != AbortOutput) {
          buffer[1] = DataMark;
          PTCPSocket::Write(buffer, 2);
          // Send the datamark character as the only out of band data byte.
          WriteOutOfBand(&buffer[1], 1);
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
      PTCPSocket::Write(buffer, 2);
  }
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


void PTelnetSocket::SendDo(BYTE code, BOOL initiating)
{
  OptionInfo & opt = option[code];

  if (initiating) {
    if (opt.wantDo || (opt.isDo && opt.respondDoDont == 0))
      return;
    opt.wantDo = TRUE;
    opt.respondDoDont++;
  }

  SendCommand(DO, code);

  PTelnetError << "tx DO " << GetTELNETOptionName(code) << endl;
}


void PTelnetSocket::SendDont(BYTE code, BOOL initiating)
{
  OptionInfo & opt = option[code];

  if (initiating) {
    if (!opt.wantDo || (!opt.isDo && opt.respondDoDont == 0))
      return;
    opt.wantDo = FALSE;
    opt.respondDoDont++;
  }

  SendCommand(DONT, code);

  PTelnetError << "tx DONT " << GetTELNETOptionName(code) << endl;
}


void PTelnetSocket::SendWill(BYTE code, BOOL initiating)
{
  OptionInfo & opt = option[code];

  if (initiating) {
    if (opt.wantWill || (opt.isWill && opt.respondWillWont == 0))
      return;
    opt.wantWill = TRUE;
    opt.respondWillWont++;
  }

  SendCommand(WILL, code);

  PTelnetError << "tx WILL " << GetTELNETOptionName(code) << endl;
}


void PTelnetSocket::SendWont(BYTE code, BOOL initiating)
{
  OptionInfo & opt = option[code];

  if (initiating) {
    if (!opt.wantWill || (!opt.isWill && opt.respondWillWont == 0))
      return;
    opt.wantWill = FALSE;
    opt.respondWillWont++;
  }

  SendCommand(WONT, code);

  PTelnetError << "tx WONT " << GetTELNETOptionName(code) << endl;
}


void PTelnetSocket::SendSubOption(BYTE code, const BYTE * info, PINDEX len)
{
  PBYTEArray buffer(len + 6);
  buffer[0] = IAC;
  buffer[1] = SB;
  buffer[2] = code;
  buffer[3] = SubOptionIs;
  PINDEX i = 4;
  while (len-- > 0) {
    if (*info == IAC)
      buffer[i++] = IAC;
    buffer[i++] = *info++;
  }
  buffer[i++] = IAC;
  buffer[i] = SE;

  PTCPSocket::Write(buffer, i);
}


void PTelnetSocket::SetTerminalType(const PString & newType)
{
  terminalType = newType;
  if (option[TerminalType].isWill)
    SendSubOption(TerminalType, terminalType, terminalType.GetLength());
}


void PTelnetSocket::SetWindowSize(WORD width, WORD height)
{
  windowWidth = width;
  windowHeight = height;
  if (option[TerminalType].isWill) {
    BYTE buffer[4];
    buffer[0] = (BYTE)(width >> 8);
    buffer[1] = (BYTE)width;
    buffer[2] = (BYTE)(height >> 8);
    buffer[3] = (BYTE)height;
    SendSubOption(TerminalType, buffer, 4);
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
            break;  // Ignore \0 after CR
          if (currentByte == '\n' && !option[EchoOption].wantDo) {
            *dst++ = currentByte;
            charsLeft--;
            break;
          }
          // Else, fall through

        case StateNormal :
          if (currentByte == IAC)
            state = StateIAC;
          else if (currentByte == '\r' && !option[TransmitBinary].wantDo) {
            /* The 'crmod' hack (see following) is needed since we can't set
               CRMOD on output only. Machines like MULTICS like to send \r
               without \n; since we must turn off CRMOD to get proper input,
               the mapping is done here (sigh).

               PS: This is stolen directly from the standard source...
             */
            if (lastReadCount > 0) {
              currentByte = *src;
              if (currentByte == '\0') { // a "true" CR
                src++;
                lastReadCount--;
                *dst++ = '\r';
                charsLeft--;
              }
              else if (currentByte == '\n' && !option[EchoOption].wantDo) {
                src++;
                lastReadCount--;
                *dst++ = '\n';
                charsLeft--;
              }
              else {
                *dst++ = '\r';
                charsLeft--;
              }
            }
            else {
              state = StateCarriageReturn;
              *dst++ = '\r';
              charsLeft--;
            }
          } else {
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
              return IAC;

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
          ProcessDo(currentByte);
          state = StateNormal;
          break;

        case StateDont :
          ProcessDont(currentByte);
          state = StateNormal;
          break;

        case StateWill :
          ProcessWill(currentByte);
          state = StateNormal;
          break;

        case StateWont :
          ProcessWont(currentByte);
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
          if (subOption.GetSize() > 1 && option[subOption[0]].wantWill)
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


void PTelnetSocket::ProcessDo(BYTE code)
{
  PTelnetError << "rx DO " << GetTELNETOptionName(code) << endl;

  OptionInfo & opt = option[code];
  if (opt.respondWillWont != 0) {
    opt.respondWillWont--;
    if (opt.respondWillWont != 0 && opt.isWill)
      opt.respondWillWont--;
  }

  if (opt.respondWillWont == 0 && !opt.wantWill) {
    switch (OnDo(code)) {
      case WillDo :
        opt.wantWill = TRUE;
        SendWill(code, FALSE);
        break;

      case WontDont :
        opt.respondWillWont++;
        SendWont(code, FALSE);
        break;

      case IgnoreOption :
        opt.isWill = FALSE;
        return;
    }
  }
  opt.isWill = TRUE;
}


PTelnetSocket::OptionAction PTelnetSocket::OnDo(BYTE code)
{
  switch (code) {
    case TimingMark :
      // Special case for TM.  We send a WILL, but pretend we sent WONT.
      SendWill(code, FALSE);
      return IgnoreOption;

    case WindowSize :
      SetWindowSize(windowWidth, windowHeight);
      // Do next case

    case TransmitBinary :
    case TerminalSpeed :
    case TerminalType :
    case SuppressGoAhead :
      return WillDo;
  }

  return WontDont;
}


void PTelnetSocket::ProcessDont(BYTE code)
{
  PTelnetError << "rx DONT " << GetTELNETOptionName(code) << endl;

  OptionInfo & opt = option[code];
  if (opt.respondWillWont != 0) {
    opt.respondWillWont--;
    if (opt.respondWillWont != 0 && opt.isWill)
      opt.respondWillWont--;
  }

  if (opt.respondWillWont == 0 && opt.wantWill) {
    OnDont(code);

    // we always accept a DONT
    opt.wantWill = FALSE;
    if (opt.isWill)
      SendWont(code, FALSE);
  }

  option[code].isWill = FALSE;
}


void PTelnetSocket::OnDont(BYTE)
{
  // Do nothing
}


void PTelnetSocket::ProcessWill(BYTE code)
{
  PTelnetError << "rx WILL " << GetTELNETOptionName(code) << endl;

  OptionInfo & opt = option[code];
  if (opt.respondDoDont != 0) {
    opt.respondDoDont--;
    if (opt.respondDoDont != 0 && opt.isDo)
      opt.respondDoDont--;
  }

  if (opt.respondDoDont == 0 && !opt.wantDo) {
    switch (OnWill(code)) {
      case WillDo :
        opt.wantDo = TRUE;
        SendDo(code, FALSE);
        break;

      case WontDont :
        opt.respondDoDont++;
        SendDont(code, FALSE);
        break;

      case IgnoreOption :
        opt.isDo = FALSE;
        return;
    }
  }

  opt.isDo = TRUE;
}


PTelnetSocket::OptionAction PTelnetSocket::OnWill(BYTE code)
{
  switch (code) {
    case TimingMark :
      /* Special case for TM.  If we get back a WILL, pretend we got back a
         WONT. Never reply to TM will's/wont's.
       */
      return IgnoreOption;

    case EchoOption :
    case TransmitBinary :
    case SuppressGoAhead :
      // settimer(modenegotiated);
      // Do next case

    case StatusOption :
      return WillDo;
  }

  return WontDont;
}


void PTelnetSocket::ProcessWont(BYTE code)
{
  PTelnetError << "rx WONT " << GetTELNETOptionName(code) << endl;

  OptionInfo & opt = option[code];
  if (opt.respondDoDont != 0) {
    opt.respondDoDont--;
    if (opt.respondDoDont != 0 && !opt.isDo)
      opt.respondDoDont--;
  }

  if (code == TimingMark) {
    opt.wantDo = FALSE;
    opt.isDo = FALSE; // Never reply to TM will's/wont's
  }
  else if (opt.respondDoDont == 0 && opt.wantDo) {
    OnWont(code);

    opt.wantDo = FALSE;
    if (opt.isDo)
      SendDont(code, FALSE);
  }

  opt.isDo = FALSE;
}


void PTelnetSocket::OnWont(BYTE code)
{
  switch (code) {
    case EchoOption :
      // settimer(modenegotiated);
      break;
  }
}


void PTelnetSocket::OnSubOption(BYTE code, const BYTE * info, PINDEX)
{
  switch (code) {
    case TerminalType :
      if (*info == SubOptionSend)
        SendSubOption(TerminalType, terminalType, terminalType.GetLength());
      break;

    case TerminalSpeed :
      if (*info == SubOptionSend) {
        static BYTE defSpeed[] = "38400,38400";
        SendSubOption(TerminalType, defSpeed, sizeof(defSpeed)-1);
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
