/*
 * $Id: inetprot.cxx,v 1.1 1995/06/17 00:50:37 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: inetprot.cxx,v $
 * Revision 1.1  1995/06/17 00:50:37  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>


//////////////////////////////////////////////////////////////////////////////
// PApplicationSocket

PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       WORD port)
  : PTCPSocket(port), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & service)
  : PTCPSocket(service), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       WORD port)
  : PTCPSocket(address, port), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       const PString & address,
                                       const PString & service)
  : PTCPSocket(address, service), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


PApplicationSocket::PApplicationSocket(PINDEX cmdCount,
                                       char const * const * cmdNames,
                                       PSocket & socket)
  : PTCPSocket(socket), commandNames(cmdCount, cmdNames, TRUE)
{
  Construct();
}


void PApplicationSocket::Construct()
{
  stuffingState = unstuffingState = DontStuff;
  newLineToCRLF = TRUE;
}


BOOL PApplicationSocket::Write(const void * buf, PINDEX len)
{
  if (len == 0 || stuffingState == DontStuff)
    return PTCPSocket::Write(buf, len);

  PINDEX totalWritten = 0;
  const char * base = (const char *)buf;
  const char * current = base;
  while (len-- > 0) {
    switch (stuffingState) {
      case StuffIdle :
        switch (*current) {
          case '\r' :
            stuffingState = StuffCR;
            break;

          case '\n' :
            if (newLineToCRLF) {
              if (current > base) {
                if (!PTCPSocket::Write(base, current - base))
                  return FALSE;
                totalWritten += lastWriteCount;
              }
              if (!PTCPSocket::Write("\r", 1))
                return FALSE;
              totalWritten += lastWriteCount;
              base = current;
            }
        }
        break;

      case StuffCR :
        stuffingState = *current != '\n' ? StuffIdle : StuffCRLF;
        break;

      case StuffCRLF :
        if (*current == '.') {
          if (current > base) {
            if (!PTCPSocket::Write(base, current - base))
              return FALSE;
            totalWritten += lastWriteCount;
          }
          if (!PTCPSocket::Write(".", 1))
            return FALSE;
          totalWritten += lastWriteCount;
          base = current;
        }
        // Then do default state

      default :
        stuffingState = StuffIdle;
        break;
    }
    current++;
  }

  if (current > base)
    if (!PTCPSocket::Write(base, current - base))
      return FALSE;

  lastWriteCount += totalWritten;
  return lastWriteCount > 0;
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


BOOL PApplicationSocket::ReadLine(PString & str, BOOL unstuffLine)
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
          if (unstuffLine && count > 1 && line[0] == '.' && line[1] == '.')
            str = PString(((const char *)line) + 1, count-1);
          else
            str = PString(line, count);
          return !(unstuffLine && count == 1 && line[0] == '.');
        }
        break;

      default :
        if (count >= line.GetSize())
          line.SetSize(count + 100);
        line[count++] = (char)c;
    }
  }

  if (count == 0)
    str = PString();
  else
    str = PString(line, count);

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
    args = args.Mid(endCommand+1);

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
  Construct();
}


PSMTPSocket::PSMTPSocket(const PString & address)
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp")
{
  Construct();
  Connect(address);
}


PSMTPSocket::PSMTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, SMTPCommands)
{
  Construct();
  Accept(socket);
}


PSMTPSocket::~PSMTPSocket()
{
  Close();
}


void PSMTPSocket::Construct()
{
  haveHello = FALSE;
  extendedHello = FALSE;
  messageBufferSize = 30000;
  ServerReset();
}


void PSMTPSocket::ServerReset()
{
  eightBitMIME = FALSE;
  sendCommand = WasMAIL;
  fromName = PString();
  toNames.RemoveAll();
}


BOOL PSMTPSocket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (ReadResponse() && lastResponseCode[0] == '2')
    return TRUE;

  Close();
  return FALSE;
}


BOOL PSMTPSocket::Accept(PSocket & socket)
{
  if (!PApplicationSocket::Accept(socket))
    return FALSE;

  return WriteResponse(220, GetLocalHostName() +
                     " Sendmail v1.61/WinSMTPSrv ready at " +
                      PTime(PTime::MediumDateTime).AsString());
}


BOOL PSMTPSocket::Close()
{
  BOOL ok = TRUE;
  if (IsOpen() && haveHello) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "") == '2';
  }
  return PApplicationSocket::Close() && ok;
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

  if (fromName[0] != '"' && fromName.Find(' ') != P_MAX_INDEX)
    fromName = '"' + fromName + '"';
  if (fromName.Find('@') == P_MAX_INDEX)
    fromName += '@' + GetLocalHostName();
  if (ExecuteCommand(MAIL, "FROM:<" + fromName + '>') != '2')
    return FALSE;

  for (PINDEX i = 0; i < toNames.GetSize(); i++) {
    if (toNames[i].Find('@') == P_MAX_INDEX)
      toNames[i] += '@' + GetPeerHostName();
    if (ExecuteCommand(RCPT, "TO:<" + toNames[i] + '>') != '2')
      return FALSE;
  }

  if (ExecuteCommand(DATA, PString()) != '3')
    return FALSE;

  stuffingState = StuffIdle;
  return TRUE;
}


BOOL PSMTPSocket::EndMessage()
{
  flush();
  stuffingState = DontStuff;
  if (!Write("\r\n.\r\n", 5))
    return FALSE;
  return ReadResponse() && lastResponseCode[0] == '2';
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
  ServerReset();

  PCaselessString peer = GetPeerHostName();

  PString response = GetLocalHostName() + " Hello " + peer + ", ";

  if (remoteHost == peer)
    response += "pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response);
}


void PSMTPSocket::OnEHLO(const PCaselessString & remoteHost)
{
  extendedHello = TRUE;
  ServerReset();

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
  ServerReset();
  WriteResponse(250, "Reset state.");
}


void PSMTPSocket::OnVRFY(const PCaselessString & name)
{
  PString expandedName;
  switch (LookUpName(name, expandedName)) {
    case AmbiguousUser :
      WriteResponse(553, "User \"" + name + "\" ambiguous.");
      break;

    case ValidUser :
      WriteResponse(250, expandedName);
      break;

    case UnknownUser :
      WriteResponse(550, "Name \"" + name + "\" does not match anything.");
      break;

    default :
      WriteResponse(550, "Error verifying user \"" + name + "\".");
  }
}


void PSMTPSocket::OnEXPN(const PCaselessString &)
{
  WriteResponse(502, "I don't do that. Sorry.");
}


static PINDEX ExtractName(const PCaselessString & args,
                          const PCaselessString & subCmd,
                          PString & name, PString & host)
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

  PINDEX finishQuote = P_MAX_INDEX;
  PINDEX startQuote = args.Find('"', leftAngle);
  if (startQuote == P_MAX_INDEX)
    finishQuote = startQuote = leftAngle+1;
  else {
    finishQuote = args.Find('"', startQuote+1);
    if (finishQuote == P_MAX_INDEX)
      finishQuote = startQuote;
  }

  PINDEX at = args.Find('@', finishQuote);
  if (at == P_MAX_INDEX)
    at = finishQuote;

  PINDEX rightAngle = args.Find('>', at);
  if (rightAngle == P_MAX_INDEX)
    return 0;

  if (at == finishQuote)
    at = rightAngle-1;

  if (startQuote == finishQuote)
    finishQuote = rightAngle;

  name = args(startQuote, finishQuote-1);
  host = args(at, rightAngle-1);

  return rightAngle+1;
}


void PSMTPSocket::OnRCPT(const PCaselessString & recipient)
{
  PCaselessString toName;
  PCaselessString toHost;
  if (ExtractName(recipient, "to", toName, toHost) == 0)
    WriteResponse(501, "Syntax error.");
  else if (toName.Find(':') != P_MAX_INDEX)
    WriteResponse(550, "Cannot do forwarding.");
  else {
    PString expandedName;
    switch (LookUpName(toName, expandedName)) {
      case ValidUser :
        WriteResponse(250, "Recipient " + toName + " Ok");
        toNames.AppendString(toName);
        toHosts.AppendString(toHost);
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

  PINDEX extendedArgPos = ExtractName(sender, "from", fromName, fromHost);
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
  unstuffingState = StuffIdle;
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


BOOL PSMTPSocket::OnTextData(PCharArray & buffer)
{
  PString line;
  while (ReadLine(line, TRUE)) {
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
    switch (unstuffingState) {
      case StuffIdle :
        buffer[count++] = (char)c;
        break;

      case StuffCR :
        unstuffingState = c != '\n' ? StuffIdle : StuffCRLF;
        buffer[count++] = (char)c;
        break;

      case StuffCRLF :
        if (c == '.')
          unstuffingState = StuffCRLFdot;
        else {
          unstuffingState = StuffIdle;
          buffer[count++] = (char)c;
        }
        break;

      case StuffCRLFdot :
        switch (c) {
          case '\r' :
            unstuffingState = StuffCRLFdotCR;
            break;

          case '.' :
            unstuffingState = StuffIdle;
            buffer[count++] = (char)c;
            break;

          default :
            unstuffingState = StuffIdle;
            buffer[count++] = '.';
            buffer[count++] = (char)c;
        }
        break;

      case StuffCRLFdotCR :
        if (c == '\n')
          return FALSE;
        buffer[count++] = '.';
        buffer[count++] = '\r';
        buffer[count++] = (char)c;
        unstuffingState = StuffIdle;
    }
    if (count > messageBufferSize) {
      buffer.SetSize(messageBufferSize);
      return TRUE;
    }
  }

  return FALSE;
}


PSMTPSocket::LookUpResult PSMTPSocket::LookUpName(
                               const PCaselessString &, PString & expandedName)
{
  expandedName = PString();
  return LookUpError;
}


BOOL PSMTPSocket::HandleMessage(PCharArray &, BOOL)
{
  return FALSE;
}


//////////////////////////////////////////////////////////////////////////////
// PPOP3Socket

static char const * POP3Commands[PPOP3Socket::NumCommands] = {
  "USER", "PASS", "QUIT", "RSET", "NOOP", "STAT",
  "LIST", "RETR", "DELE", "APOP", "TOP",  "UIDL"
};


PString PPOP3Socket::okResponse = "+OK";
PString PPOP3Socket::errResponse = "-ERR";


PPOP3Socket::PPOP3Socket()
  : PApplicationSocket(NumCommands, POP3Commands, "pop3")
{
  Construct();
}


PPOP3Socket::PPOP3Socket(const PString & address)
  : PApplicationSocket(NumCommands, POP3Commands, "pop3")
{
  Construct();
  Connect(address);
}


PPOP3Socket::PPOP3Socket(PSocket & socket)
  : PApplicationSocket(NumCommands, POP3Commands)
{
  Construct();
  Accept(socket);
}


PPOP3Socket::~PPOP3Socket()
{
  Close();
}

void PPOP3Socket::Construct()
{
  loggedIn = FALSE;
}


BOOL PPOP3Socket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (ReadResponse() && lastResponseCode[0] == '+')
    return TRUE;

  Close();
  return FALSE;
}


BOOL PPOP3Socket::Accept(PSocket & socket)
{
  if (!PApplicationSocket::Accept(socket))
    return FALSE;

  return WriteResponse(okResponse, GetLocalHostName() +
                     " POP3 server ready at " +
                      PTime(PTime::MediumDateTime).AsString());
}


BOOL PPOP3Socket::Close()
{
  BOOL ok = TRUE;
  if (IsOpen() && loggedIn) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "") == '+';
  }
  return PApplicationSocket::Close() && ok;
}


BOOL PPOP3Socket::LogIn(const PString & username, const PString & password)
{
  if (ExecuteCommand(USER, username) != '+')
    return FALSE;

  if (ExecuteCommand(PASS, password) != '+')
    return FALSE;

  loggedIn = TRUE;
  return TRUE;
}


int PPOP3Socket::GetMessageCount()
{
  if (ExecuteCommand(STAT, "") != '+')
    return -1;

  return (int)lastResponseInfo.AsInteger();
}


PUnsignedArray PPOP3Socket::GetMessageSizes()
{
  PUnsignedArray sizes;

  if (ExecuteCommand(LIST, "") == '+') {
    PString msgInfo;
    while (ReadLine(msgInfo, TRUE))
      sizes.SetAt((PINDEX)msgInfo.AsInteger()-1,
                  (unsigned)msgInfo.Mid(msgInfo.Find(' ')).AsInteger());
  }

  return sizes;
}


PStringArray PPOP3Socket::GetMessageHeaders()
{
  PStringArray headers;

  int count = GetMessageCount();
  for (int msgNum = 1; msgNum <= count; msgNum++) {
    if (ExecuteCommand(TOP, PString(PString::Unsigned, msgNum) + " 0") == '+'){
      PString headerLine;
      while (ReadLine(headerLine, TRUE))
        headers[msgNum-1] += headerLine;
    }
  }
  return headers;
}


BOOL PPOP3Socket::BeginMessage(PINDEX messageNumber)
{
  return ExecuteCommand(RETR, PString(PString::Unsigned,messageNumber)) == '+';
}


BOOL PPOP3Socket::DeleteMessage(PINDEX messageNumber)
{
  return ExecuteCommand(DELE, PString(PString::Unsigned,messageNumber)) == '+';
}


BOOL PPOP3Socket::ProcessCommand()
{
  PString args;
  switch (ReadCommand(args)) {
    case USER :
      OnUSER(args);
      break;
    case PASS :
      OnPASS(args);
      break;
    case QUIT :
      OnQUIT();
      return FALSE;
    case RSET :
      OnRSET();
      break;
    case NOOP :
      OnNOOP();
      break;
    case STAT :
      OnSTAT();
      break;
    case LIST :
      OnLIST((PINDEX)args.AsInteger());
      break;
    case RETR :
      OnRETR((PINDEX)args.AsInteger());
      break;
    case DELE :
      OnDELE((PINDEX)args.AsInteger());
      break;
    case TOP :
      if (args.Find(' ') == P_MAX_INDEX)
        WriteResponse(errResponse, "Syntax error");
      else
        OnTOP((PINDEX)args.AsInteger(),
              (PINDEX)args.Mid(args.Find(' ')).AsInteger());
      break;
    case UIDL :
      OnUIDL((PINDEX)args.AsInteger());
      break;
    default :
      return OnUnknown(args);
  }

  return TRUE;
}


void PPOP3Socket::OnUSER(const PString & name)
{
  messageSizes.SetSize(0);
  messageIDs.SetSize(0);
  username = name;
  WriteResponse(okResponse, "User name accepted.");
}


void PPOP3Socket::OnPASS(const PString & password)
{
  if (username.IsEmpty())
    WriteResponse(errResponse, "No user name specified.");
  else if (HandleOpenMailbox(username, password))
    WriteResponse(okResponse, username + " mail is available.");
  else
    WriteResponse(errResponse, "No access to " + username + " mail.");
  messageDeletions.SetSize(messageIDs.GetSize());
}


void PPOP3Socket::OnQUIT()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    if (messageDeletions[i])
      HandleDeleteMessage(i+1, messageIDs[i]);

  WriteResponse(okResponse, GetLocalHostName() +
                     " POP3 server signing off at " +
                      PTime(PTime::MediumDateTime).AsString());

  Close();
}


void PPOP3Socket::OnRSET()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    messageDeletions[i] = FALSE;
  WriteResponse(okResponse, "Resetting state.");
}


void PPOP3Socket::OnNOOP()
{
  WriteResponse(okResponse, "Doing nothing.");
}


void PPOP3Socket::OnSTAT()
{
  DWORD total = 0;
  for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
    total += messageSizes[i];
  WriteResponse(okResponse, psprintf("%u %u", messageSizes.GetSize(), total));
}


void PPOP3Socket::OnLIST(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse,
            PString(PString::Unsigned, messageSizes.GetSize()) + " messages.");
    for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
      WriteLine(psprintf("%u %u", i, messageSizes[i-1]));
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    WriteResponse(okResponse, psprintf("%u %u", msg, messageSizes[msg-1]));
}


void PPOP3Socket::OnRETR(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else {
    WriteResponse(okResponse,
                 PString(PString::Unsigned, messageSizes[msg-1]) + " octets.");
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], P_MAX_INDEX);
    stuffingState = DontStuff;
    Write("\r\n.\r\n", 5);
  }
}


void PPOP3Socket::OnDELE(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    messageDeletions[msg-1] = TRUE;
}


void PPOP3Socket::OnTOP(PINDEX msg, PINDEX count)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse, "No such message.");
  else {
    WriteResponse(okResponse, PString());
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], count);
    stuffingState = DontStuff;
    Write("\r\n.\r\n", 5);
  }
}


void PPOP3Socket::OnUIDL(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse,
              PString(PString::Unsigned, messageIDs.GetSize()) + " messages.");
    for (PINDEX i = 1; i <= messageIDs.GetSize(); i++)
      WriteLine(PString(PString::Unsigned, i) + ' ' + messageIDs[i-1]);
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    WriteLine(PString(PString::Unsigned, msg) + ' ' + messageIDs[msg-1]);
}


BOOL PPOP3Socket::OnUnknown(const PCaselessString & command)
{
  WriteResponse(errResponse, "Command \"" + command + "\" unrecognised.");
  return TRUE;
}


BOOL PPOP3Socket::HandleOpenMailbox(const PString &, const PString &)
{
  return FALSE;
}


void PPOP3Socket::HandleSendMessage(PINDEX, const PString &, PINDEX)
{
}


void PPOP3Socket::HandleDeleteMessage(PINDEX, const PString &)
{
}


// End Of File ///////////////////////////////////////////////////////////////
