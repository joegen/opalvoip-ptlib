/*
 * $Id: inetmail.cxx,v 1.5 1996/06/28 13:22:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: inetmail.cxx,v $
 * Revision 1.5  1996/06/28 13:22:09  robertj
 * Changed SMTP incoming message handler so can tell when started, processing or ended message.
 *
 * Revision 1.4  1996/05/26 03:46:51  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.3  1996/03/18 13:33:16  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
 *
 * Revision 1.2  1996/03/16 04:51:28  robertj
 * Changed lastResponseCode to an integer.
 * Added ParseReponse() for splitting reponse line into code and info.
 *
 * Revision 1.1  1996/03/04 12:12:51  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <mailsock.h>


static const PString CRLF = "\r\n";
static const PString CRLFdotCRLF = "\r\n.\r\n";


//////////////////////////////////////////////////////////////////////////////
// PSMTPSocket

static char const * SMTPCommands[PSMTPSocket::NumCommands] = {
  "HELO", "EHLO", "QUIT", "HELP", "NOOP",
  "TURN", "RSET", "VRFY", "EXPN", "RCPT",
  "MAIL", "SEND", "SAML", "SOML", "DATA"
};


PSMTPSocket::PSMTPSocket()
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp 25")
{
  Construct();
}


PSMTPSocket::PSMTPSocket(const PString & address)
  : PApplicationSocket(NumCommands, SMTPCommands, "smtp 25")
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

  if (ReadResponse() && lastResponseCode/100 == 2)
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
  if (!WriteString(CRLFdotCRLF))
    return FALSE;
  return ReadResponse() && lastResponseCode/100 == 2;
}


BOOL PSMTPSocket::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return FALSE;

  switch (num) {
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
  BOOL ok = TRUE;
  PCharArray buffer;
  MessagePosition position = MessageStart;
  if (eightBitMIME) {
    WriteResponse(354,
                "Enter 8BITMIME message, terminate with '<CR><LF>.<CR><LF>'.");
    endMIMEDetectState = StuffIdle;
    while (ok && OnMIMEData(buffer)) {
      ok = HandleMessage(buffer, position);
      position = MessagePart;
    }
  }
  else {
    WriteResponse(354, "Enter mail, terminate with '.' alone on a line.");
    while (ok && OnTextData(buffer)) {
      ok = HandleMessage(buffer, position);
      position = MessagePart;
    }
  }

  if (ok && HandleMessage(buffer, MessageEnd))
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
  while (ReadLine(line)) {
    line += '\n';
    PINDEX size = buffer.GetSize();
    PINDEX len = line.GetLength();
    memcpy(buffer.GetPointer(size + len) + size, (const char *)line, len);
    if (size + len > messageBufferSize)
      return TRUE;
  }
  return FALSE;
}


BOOL PSMTPSocket::OnMIMEData(PCharArray & buffer)
{
  PINDEX count = 0;
  int c;
  while ((c = ReadChar()) >= 0) {
    if (count >= buffer.GetSize())
      buffer.SetSize(count + 100);
    switch (endMIMEDetectState) {
      case StuffIdle :
        buffer[count++] = (char)c;
        break;

      case StuffCR :
        endMIMEDetectState = c != '\n' ? StuffIdle : StuffCRLF;
        buffer[count++] = (char)c;
        break;

      case StuffCRLF :
        if (c == '.')
          endMIMEDetectState = StuffCRLFdot;
        else {
          endMIMEDetectState = StuffIdle;
          buffer[count++] = (char)c;
        }
        break;

      case StuffCRLFdot :
        switch (c) {
          case '\r' :
            endMIMEDetectState = StuffCRLFdotCR;
            break;

          case '.' :
            endMIMEDetectState = StuffIdle;
            buffer[count++] = (char)c;
            break;

          default :
            endMIMEDetectState = StuffIdle;
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
        endMIMEDetectState = StuffIdle;

      default :
        PAssertAlways("Illegal SMTP state");
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


BOOL PSMTPSocket::HandleMessage(PCharArray &, MessagePosition)
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
  : PApplicationSocket(NumCommands, POP3Commands, "pop3 110")
{
  Construct();
}


PPOP3Socket::PPOP3Socket(const PString & address)
  : PApplicationSocket(NumCommands, POP3Commands, "pop3 110")
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

  if (ReadResponse() && lastResponseCode > 0)
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
    ok = ExecuteCommand(QUIT, "") > 0;
  }
  return PApplicationSocket::Close() && ok;
}


BOOL PPOP3Socket::LogIn(const PString & username, const PString & password)
{
  if (ExecuteCommand(USER, username) <= 0)
    return FALSE;

  if (ExecuteCommand(PASS, password) <= 0)
    return FALSE;

  loggedIn = TRUE;
  return TRUE;
}


int PPOP3Socket::GetMessageCount()
{
  if (ExecuteCommand(STAT, "") <= 0)
    return -1;

  return (int)lastResponseInfo.AsInteger();
}


PUnsignedArray PPOP3Socket::GetMessageSizes()
{
  PUnsignedArray sizes;

  if (ExecuteCommand(LIST, "") > 0) {
    PString msgInfo;
    while (ReadLine(msgInfo))
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
    if (ExecuteCommand(TOP, PString(PString::Unsigned,msgNum) + " 0") > 0) {
      PString headerLine;
      while (ReadLine(headerLine, TRUE))
        headers[msgNum-1] += headerLine;
    }
  }
  return headers;
}


BOOL PPOP3Socket::BeginMessage(PINDEX messageNumber)
{
  return ExecuteCommand(RETR, PString(PString::Unsigned, messageNumber)) > 0;
}


BOOL PPOP3Socket::DeleteMessage(PINDEX messageNumber)
{
  return ExecuteCommand(DELE, PString(PString::Unsigned, messageNumber)) > 0;
}


PINDEX PPOP3Socket::ParseResponse(const PString & line)
{
  lastResponseCode = line[0] == '+';
  PINDEX endCode = line.Find(' ');
  if (endCode != P_MAX_INDEX)
    lastResponseInfo = line.Mid(endCode+1);
  else
    lastResponseInfo = PString();
  return 0;
}


BOOL PPOP3Socket::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return FALSE;

  switch (num) {
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
    WriteString(CRLFdotCRLF);
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
    WriteString(CRLFdotCRLF);
  }
}


void PPOP3Socket::OnUIDL(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse,
              PString(PString::Unsigned, messageIDs.GetSize()) + " messages.");
    for (PINDEX i = 1; i <= messageIDs.GetSize(); i++)
      WriteLine(PString(PString::Unsigned, i) & messageIDs[i-1]);
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse, "No such message.");
  else
    WriteLine(PString(PString::Unsigned, msg) & messageIDs[msg-1]);
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
