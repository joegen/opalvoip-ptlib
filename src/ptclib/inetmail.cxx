/*
 * inetmail.cxx
 *
 * Internet Mail classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2002 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): Federico Pinna and Reitek S.p.A. (SASL authentication)
 */

#ifdef __GNUC__
#pragma implementation "inetmail.h"
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>
#include <ptclib/pssl.h>
#include <ptclib/inetmail.h>
#if P_SASL
#include <ptclib/psasl.h>
#endif

static const PConstString CRLF("\r\n");
static const PConstString CRLFdotCRLF("\r\n.\r\n");


#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// PSMTP

static char const * const SMTPCommands[PSMTP::NumCommands] = {
  "HELO", "EHLO", "QUIT", "HELP", "NOOP",
  "TURN", "RSET", "VRFY", "EXPN", "RCPT",
  "MAIL", "SEND", "SAML", "SOML", "DATA",
  "AUTH", "STARTTLS"
};


PSMTP::PSMTP()
  : PInternetProtocol("smtp 25", NumCommands, SMTPCommands)
{
}


bool PSMTP::SendMail(const Parameters & params, PString & error)
{
  if (params.m_to.IsEmpty() || params.m_to.front().IsEmpty()) {
    error = "No \"to\" address provided.";
    return false;
  }

  PString from = params.m_from;
  if (from.IsEmpty())
    from = PSTRSTRM(PProcess::Current().GetUserName() << '@' << PIPSocket::GetHostName());

  PString server = params.m_hostname;
  if (server.IsEmpty())
    server = "127.0.0.1";

  WORD port = params.m_port;
  if (port == 0)
    port = PSocket::GetPortByService("tcp", "smtp 25");

  PTCPSocket socket(port);
  if (!socket.Connect(server)) {
    error = PSTRSTRM("Could not connect to SMTP server " << server << ':' << port << " - " << socket.GetErrorText());
    return false;
  }

  PSMTPClient smtpClient;
  if (!smtpClient.Open(socket)) {
    error = PSTRSTRM("Could not open SMTP to server " << server << ':' << port << " - " << smtpClient.GetErrorText());
    return false;
  }

  if (!params.m_username.IsEmpty() && !smtpClient.LogIn(params.m_username, params.m_password)) {
    error = PSTRSTRM("Could not log into SMTP server " << server << ':' << port << " - " << smtpClient.GetLastResponseInfo());
    return false;
  }

  if (!smtpClient.BeginMessage(from, params.m_to.front(), params.m_eightBitMIME)) {
    error = PSTRSTRM("Could not begin SMTP to server " << server << ':' << port << " - " << smtpClient.GetLastResponseInfo());
    return false;
  }

  PRFC822Channel email(PRFC822Channel::Sending);
  email.SetFromAddress(from);
  email.SetToAddress(PSTRSTRM(setfill(',') << params.m_to));
  email.SetCC(PSTRSTRM(setfill(',') << params.m_cc));
  email.SetBCC(PSTRSTRM(setfill(',') << params.m_bcc));
  email.SetSubject(params.m_subject);

  if (!email.Open(smtpClient)) {
    error = PSTRSTRM("Could not open RFC822 to server " << server << ':' << port);
    return false;
  }

  PString boundary;
  if (!params.m_attachments.IsEmpty())
    boundary = email.MultipartMessage();

  if (!email.WriteString(params.m_bodyText))
    return false;

  for (PStringList::const_iterator it = params.m_attachments.begin(); it != params.m_attachments.end(); ++it) {
    PFilePath filename = *it;
    PFile file(filename, PFile::ReadOnly);
    if (file.IsOpen()) {
      email.NextPart(boundary);
      email.SetContentAttachment(filename.GetFileName());
      PString fileType = filename.GetType();
      PString contentType = PMIMEInfo::GetContentType(fileType);
      if ((fileType *= "txt") || (fileType == "html"))
        email.SetTransferEncoding("7bit", false);
      else
        email.SetTransferEncoding("base64", true);
      BYTE buffer[1024];
      for (;;) {
        if (!file.Read(buffer, sizeof(buffer)))
          break;
        if (!email.Write(buffer, file.GetLastReadCount()))
          return false;
      }
    }
  }

  return true;
}


//////////////////////////////////////////////////////////////////////////////
// PSMTPClient

PSMTPClient::PSMTPClient()
  : m_haveHello(false)
  , m_sendingData(false)
{
}


PSMTPClient::~PSMTPClient()
{
  Close();
}


PBoolean PSMTPClient::OnOpen()
{
  return ReadResponse() && lastResponseCode/100 == 2;
}


PBoolean PSMTPClient::Close()
{
  bool ok = true;

  if (m_sendingData)
    ok = EndMessage();

  if (IsOpen() && m_haveHello) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "")/100 == 2 && ok;
  }
  return PInternetProtocol::Close() && ok;
}


PString PSMTPClient::InternalLocalHostName()
{
  PIPSocket * socket = GetSocket();
  return socket != NULL ? socket->GetLocalHostName() : PIPSocket::GetHostName();
}


bool PSMTPClient::InternalHello()
{
  if (m_haveHello)
    return true;

  if (!InternalExtendedHello())
    return m_haveHello = (ExecuteCommand(HELO, InternalLocalHostName())/100 != 2);

#if P_SSL
  if (m_extensions.Contains("STARTTLS") && ExecuteCommand(STARTTLS) / 100 == 2) {
    PSSLChannel * ssl = new PSSLChannel();
    if (!ssl->Open(Detach(), readAutoDelete) || !ssl->Connect()) {
      delete ssl;
      return false;
    }
    if (!SetReadChannel(ssl) || !SetWriteChannel(ssl))
      return false;
    if (!InternalExtendedHello())
      return false;
  }
#endif

  m_haveHello = true;
  return true;
}


bool PSMTPClient::InternalExtendedHello()
{
  if (ExecuteCommand(EHLO, InternalLocalHostName()) / 100 != 2)
    return false;

  m_extensions.SetAt("EHLO", PString::Empty()); // Make sure something in set.

  PStringArray lines = lastResponseInfo.Lines();
  for (PINDEX i = 1; i < lines.GetSize(); ++i) {
    PString key, data;
    if (lines[i].Split(' ', key, data, PString::SplitDefaultToBefore|PString::SplitTrimBefore|PString::SplitBeforeNonEmpty))
      m_extensions.SetAt(key, data);
  }

  return true;
}


bool PSMTPClient::LogIn(const PString & username, const PString & password)
{
  if (m_haveHello) {
    lastResponseInfo = "Cannot log in when already logged in.";
    return false; // Wrong state
  }

  if (!InternalHello())
    return false;

  PStringArray serverMechs = m_extensions("AUTH").Tokenise(' ', false);
  if (serverMechs.IsEmpty()) {
    lastResponseInfo = "Server did not provide any authentication mechanisms";
    return false;
  }
  
#if P_SASL
  PSASLClient auth("smtp", username, username, password);
  PStringSet ourMechs;

  if (!auth.Init("", ourMechs))
    return false;

  PString mech;

  for (PINDEX i = 0, max = serverMechs.GetSize() ; i < max ; i++)
    if (ourMechs.Contains(serverMechs[i])) {
      mech = serverMechs[i];
      break;
    }

  if (mech.IsEmpty())
    return true;  // No mechanism in common

  PString output;

  // Ok, let's go...
  if (!auth.Start(mech, output))
    return false;

  if (!output.IsEmpty())
    mech = mech + " " + output;

  if (ExecuteCommand(AUTH, mech) <= 0)
    return false;

  PSASLClient::PSASLResult result;
  int response;

  do {
    response = lastResponseCode/100;

    if (response == 2)
      break;
    else if (response != 3)
      return false;

    result = auth.Negotiate(lastResponseInfo, output);
      
    if (result == PSASLClient::Fail)
      return false;

    if (!output.IsEmpty()) {
      WriteLine(output);
      if (!ReadResponse())
        return false;
    }
  } while (result == PSASLClient::Continue);
  auth.End();
  return true;
#else
  PConstString plain("PLAIN");
  if (serverMechs.GetValuesIndex(plain) == P_MAX_INDEX) {
    lastResponseInfo = "SASL not available and PLAIN authentication not allowed.";
    return false;
  }

  PBYTEArray auth((username.GetLength()+1)*2 + password.GetLength());
  memcpy(auth.GetPointer(),                            username.GetPointer(), username.GetLength());
  memcpy(auth.GetPointer()+   username.GetLength()+1,  username.GetPointer(), username.GetLength());
  memcpy(auth.GetPointer()+2*(username.GetLength()+1), password.GetPointer(), password.GetLength());
  return ExecuteCommand(AUTH, plain & PBase64::Encode(auth,""))/100 == 2;
#endif
}


bool PSMTPClient::BeginMessage(const PString & from,
                               const PString & to,
                               bool useEightBitMIME)
{
  m_fromAddress = from;
  m_toNames.RemoveAll();
  m_toNames.AppendString(to);
  return InternalBeginMessage(useEightBitMIME);
}


bool PSMTPClient::BeginMessage(const PString & from,
                               const PStringList & toList,
                               bool useEightBitMIME)
{
  m_fromAddress = from;
  m_toNames = toList;
  return InternalBeginMessage(useEightBitMIME);
}


bool PSMTPClient::InternalBeginMessage(bool useEightBitMIME)
{
  if (!InternalHello())
    return false;

  if (useEightBitMIME && !m_extensions.Contains("8BITMIME")) {
    lastResponseInfo = "Eight bit MIME not supported.";
    return false;
  }

  PString localHost;
  PString peerHost;
  PIPSocket * socket = GetSocket();
  if (socket != NULL) {
    localHost = socket->GetLocalHostName();
    peerHost = socket->GetPeerHostName();
  }

  if (m_fromAddress[0] != '"' && m_fromAddress.Find(' ') != P_MAX_INDEX)
    m_fromAddress = '"' + m_fromAddress + '"';
  if (!localHost.IsEmpty() && m_fromAddress.Find('@') == P_MAX_INDEX)
    m_fromAddress += '@' + localHost;
  if (ExecuteCommand(MAIL, "FROM:<" + m_fromAddress + '>')/100 != 2)
    return false;

  for (PStringList::iterator i = m_toNames.begin(); i != m_toNames.end(); i++) {
    if (!peerHost.IsEmpty() && i->Find('@') == P_MAX_INDEX)
      *i += '@' + peerHost;
    if (ExecuteCommand(RCPT, "TO:<" + *i + '>')/100 != 2)
      return false;
  }

  if (ExecuteCommand(DATA, PString())/100 != 3)
    return false;

  flush();

  stuffingState = StuffIdle;
  m_sendingData = true;
  return true;
}


PBoolean PSMTPClient::EndMessage()
{
  flush();

  stuffingState = DontStuff;
  m_sendingData = false;

  if (!WriteString(CRLFdotCRLF))
    return false;

  return ReadResponse() && lastResponseCode/100 == 2;
}


//////////////////////////////////////////////////////////////////////////////
// PSMTPServer

PSMTPServer::PSMTPServer()
{
  extendedHello = false;
  eightBitMIME = false;
  messageBufferSize = 30000;
  ServerReset();
}


void PSMTPServer::ServerReset()
{
  eightBitMIME = false;
  sendCommand = WasMAIL;
  fromAddress = PString();
  toNames.RemoveAll();
}


PBoolean PSMTPServer::OnOpen()
{
  return WriteResponse(220, PIPSocket::GetHostName() + "ESMTP server ready");
}


PBoolean PSMTPServer::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return false;

  switch (num) {
    case HELO :
      OnHELO(args);
      break;
    case EHLO :
      OnEHLO(args);
      break;
    case QUIT :
      OnQUIT();
      return false;
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

  return true;
}


void PSMTPServer::OnHELO(const PCaselessString & remoteHost)
{
  extendedHello = false;
  ServerReset();

  PCaselessString peerHost;
  PIPSocket * socket = GetSocket();
  if (socket != NULL)
    peerHost = socket->GetPeerHostName();

  PString response = PIPSocket::GetHostName() & "Hello" & (peerHost + ", ");

  if (remoteHost == peerHost)
    response += "pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  WriteResponse(250, response);
}


void PSMTPServer::OnEHLO(const PCaselessString & remoteHost)
{
  extendedHello = true;
  ServerReset();

  PCaselessString peerHost;
  PIPSocket * socket = GetSocket();
  if (socket != NULL)
    peerHost = socket->GetPeerHostName();

  PString response = PIPSocket::GetHostName() & "Hello" & (peerHost + ", ");

  if (remoteHost == peerHost)
    response += ", pleased to meet you.";
  else if (remoteHost.IsEmpty())
    response += "why do you wish to remain anonymous?";
  else
    response += "why do you wish to call yourself \"" + remoteHost + "\"?";

  response += "\nHELP\nVERB\nONEX\nMULT\nEXPN\nTICK\n8BITMIME\n";
  WriteResponse(250, response);
}


void PSMTPServer::OnQUIT()
{
  WriteResponse(221, PIPSocket::GetHostName() + " closing connection, goodbye.");
  Close();
}


void PSMTPServer::OnHELP()
{
  WriteResponse(214, "No help here.");
}


void PSMTPServer::OnNOOP()
{
  WriteResponse(250, "Ok");
}


void PSMTPServer::OnTURN()
{
  WriteResponse(502, "I don't do that yet. Sorry.");
}


void PSMTPServer::OnRSET()
{
  ServerReset();
  WriteResponse(250, "Reset state.");
}


void PSMTPServer::OnVRFY(const PCaselessString & name)
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


void PSMTPServer::OnEXPN(const PCaselessString &)
{
  WriteResponse(502, "I don't do that. Sorry.");
}


static PINDEX ParseMailPath(const PCaselessString & args,
                            const PCaselessString & subCmd,
                            PString & name,
                            PString & domain,
                            PString & forwardList)
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

  PINDEX finishQuote;
  PINDEX startQuote = args.Find('"', leftAngle);
  if (startQuote == P_MAX_INDEX) {
    colon = args.Find(':', leftAngle);
    if (colon == P_MAX_INDEX)
      colon = leftAngle;
    finishQuote = startQuote = colon+1;
  }
  else {
    finishQuote = args.Find('"', startQuote+1);
    if (finishQuote == P_MAX_INDEX)
      finishQuote = startQuote;
    colon = args.Find(':', leftAngle);
    if (colon > startQuote)
      colon = leftAngle;
  }

  PINDEX rightAngle = args.Find('>', finishQuote);
  if (rightAngle == P_MAX_INDEX)
    return 0;

  PINDEX at = args.Find('@', finishQuote);
  if (at > rightAngle)
    at = rightAngle;

  if (startQuote == finishQuote)
    finishQuote = at;

  name = args(startQuote, finishQuote-1);
  domain = args(at+1, rightAngle-1);
  forwardList = args(leftAngle+1, colon-1);

  return rightAngle+1;
}


void PSMTPServer::OnRCPT(const PCaselessString & recipient)
{
  PCaselessString toName;
  PCaselessString toDomain;
  PCaselessString forwardList;
  if (ParseMailPath(recipient, "to", toName, toDomain, forwardList) == 0)
    WriteResponse(501, "Syntax error.");
  else {
    switch (ForwardDomain(toDomain, forwardList)) {
      case CannotForward :
        WriteResponse(550, "Cannot do forwarding.");
        break;

      case WillForward :
        if (!forwardList.IsEmpty())
          forwardList += ":";
        forwardList += toName;
        if (!toDomain.IsEmpty())
          forwardList += "@" + toDomain;
        toNames.AppendString(toName);
        toDomains.AppendString(forwardList);
        break;

      case LocalDomain :
      {
        PString expandedName;
        switch (LookUpName(toName, expandedName)) {
          case ValidUser :
            WriteResponse(250, "Recipient " + toName + " Ok");
            toNames.AppendString(toName);
            toDomains.AppendString("");
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
  }
}


void PSMTPServer::OnMAIL(const PCaselessString & sender)
{
  sendCommand = WasMAIL;
  OnSendMail(sender);
}


void PSMTPServer::OnSEND(const PCaselessString & sender)
{
  sendCommand = WasSEND;
  OnSendMail(sender);
}


void PSMTPServer::OnSAML(const PCaselessString & sender)
{
  sendCommand = WasSAML;
  OnSendMail(sender);
}


void PSMTPServer::OnSOML(const PCaselessString & sender)
{
  sendCommand = WasSOML;
  OnSendMail(sender);
}


void PSMTPServer::OnSendMail(const PCaselessString & sender)
{
  if (!fromAddress.IsEmpty()) {
    WriteResponse(503, "Sender already specified.");
    return;
  }

  PString fromDomain;
  PINDEX extendedArgPos = ParseMailPath(sender, "from", fromAddress, fromDomain, fromPath);
  if (extendedArgPos == 0 || fromAddress.IsEmpty()) {
    WriteResponse(501, "Syntax error.");
    return;
  }
  fromAddress += fromDomain;

  if (extendedHello) {
    PINDEX equalPos = sender.Find('=', extendedArgPos);
    PCaselessString body = sender(extendedArgPos, equalPos).Trim();
    PCaselessString mime = sender.Mid(equalPos+1).Trim();
    eightBitMIME = (body == "BODY" && mime == "8BITMIME");
  }

  PString response = "Sender " + fromAddress;
  if (eightBitMIME)
    response += " and 8BITMIME";
  WriteResponse(250, response + " Ok");
}


void PSMTPServer::OnDATA()
{
  if (fromAddress.IsEmpty()) {
    WriteResponse(503, "Need a valid MAIL command.");
    return;
  }

  if (toNames.GetSize() == 0) {
    WriteResponse(503, "Need a valid RCPT command.");
    return;
  }

  // Ok, everything is ready to start the message.
  if (!WriteResponse(354,
        eightBitMIME ? "Enter 8BITMIME message, terminate with '<CR><LF>.<CR><LF>'."
                     : "Enter mail, terminate with '.' alone on a line."))
    return;

  endMIMEDetectState = eightBitMIME ? StuffIdle : DontStuff;

  PBoolean ok = true;
  PBoolean completed = false;
  PBoolean starting = true;

  while (ok && !completed) {
    PCharArray buffer;
    if (eightBitMIME)
      ok = OnMIMEData(buffer, completed);
    else
      ok = OnTextData(buffer, completed);
    if (ok) {
      ok = HandleMessage(buffer, starting, completed);
      starting = false;
    }
  }

  if (ok)
    WriteResponse(250, "Message received Ok.");
  else
    WriteResponse(554, "Message storage failed.");
}


PBoolean PSMTPServer::OnUnknown(const PCaselessString & command)
{
  WriteResponse(500, "Command \"" + command + "\" unrecognised.");
  return true;
}


PBoolean PSMTPServer::OnTextData(PCharArray & buffer, PBoolean & completed)
{
  PString line;
  while (ReadLine(line)) {
    PINDEX len = line.GetLength();
    if (len == 1 && line[0] == '.') {
      completed = true;
      return true;
    }

    PINDEX start = (len > 1 && line[0] == '.' && line[1] == '.') ? 1 : 0;
    PINDEX size = buffer.GetSize();
    len -= start;
    memcpy(buffer.GetPointer(size + len + 2) + size,
           ((const char *)line)+start, len);
    size += len;
    buffer[size++] = '\r';
    buffer[size++] = '\n';
    if (size > messageBufferSize)
      return true;
  }
  return false;
}


PBoolean PSMTPServer::OnMIMEData(PCharArray & buffer, PBoolean & completed)
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
        if (c == '\n') {
          completed = true;
          return true;
        }
        buffer[count++] = '.';
        buffer[count++] = '\r';
        buffer[count++] = (char)c;
        endMIMEDetectState = StuffIdle;

      default :
        PAssertAlways("Illegal SMTP state");
    }
    if (count > messageBufferSize) {
      buffer.SetSize(messageBufferSize);
      return true;
    }
  }

  return false;
}


PSMTPServer::ForwardResult PSMTPServer::ForwardDomain(PCaselessString & userDomain,
                                                      PCaselessString & forwardDomainList)
{
  return userDomain.IsEmpty() && forwardDomainList.IsEmpty() ? LocalDomain : CannotForward;
}


PSMTPServer::LookUpResult PSMTPServer::LookUpName(const PCaselessString &,
                                                  PString & expandedName)
{
  expandedName = PString();
  return LookUpError;
}


PBoolean PSMTPServer::HandleMessage(PCharArray &, PBoolean, PBoolean)
{
  return false;
}


//////////////////////////////////////////////////////////////////////////////
// PPOP3

static char const * const POP3Commands[PPOP3::NumCommands] = {
  "USER", "PASS", "QUIT", "RSET", "NOOP", "STAT",
  "LIST", "RETR", "DELE", "APOP", "TOP",  "UIDL",
  "AUTH"
};


const PString & PPOP3::okResponse () { static const PConstString s("+OK");  return s; }
const PString & PPOP3::errResponse() { static const PConstString s("-ERR"); return s; }


PPOP3::PPOP3()
  : PInternetProtocol("pop3 110", NumCommands, POP3Commands)
{
}


PINDEX PPOP3::ParseResponse(const PString & line)
{
  lastResponseCode = line[0] == '+';
  PINDEX endCode = line.Find(' ');
  if (endCode != P_MAX_INDEX)
    lastResponseInfo = line.Mid(endCode+1);
  else
    lastResponseInfo = PString();
  return 0;
}


//////////////////////////////////////////////////////////////////////////////
// PPOP3Client

PPOP3Client::PPOP3Client()
{
  loggedIn = false;
}


PPOP3Client::~PPOP3Client()
{
  Close();
}

PBoolean PPOP3Client::OnOpen()
{
  if (!ReadResponse() || lastResponseCode <= 0)
    return false;

  // APOP login command supported?
  PINDEX i = lastResponseInfo.FindRegEx("<.*@.*>");

  if (i != P_MAX_INDEX)
    apopBanner = lastResponseInfo.Mid(i);

  return true;
}


PBoolean PPOP3Client::Close()
{
  PBoolean ok = true;
  if (IsOpen() && loggedIn) {
    SetReadTimeout(60000);
    ok = ExecuteCommand(QUIT, "") > 0;
  }
  return PInternetProtocol::Close() && ok;
}


PBoolean PPOP3Client::LogIn(const PString & username, const PString & password, int options)
{
#if P_SASL
  PString mech;
  PSASLClient auth("pop", username, username, password);

  if ((options & UseSASL) && ExecuteCommand(AUTH, "") > 0) {
    PStringSet serverMechs;
    while (ReadLine(mech) && mech[0] != '.')
      serverMechs.Include(mech);

    mech = PString::Empty();
    PStringSet ourMechs;

    if (auth.Init("", ourMechs)) {

      if (!(options & AllowClearTextSASL)) {
        ourMechs.Exclude("PLAIN");
        ourMechs.Exclude("LOGIN");
      }

      for (PStringSet::iterator it = serverMechs.begin(); it != serverMechs.end(); ++it)
        if (ourMechs.Contains(*it)) {
          mech = *it;
          break;
        }
    }
  }

  PString output;

  if ((options & UseSASL) && !mech.IsEmpty() && auth.Start(mech, output)) {

    if (ExecuteCommand(AUTH, mech) <= 0)
      return false;

    PSASLClient::PSASLResult result;

    do {
      result = auth.Negotiate(lastResponseInfo, output);
      
      if (result == PSASLClient::Fail)
        return false;

      if (!output.IsEmpty()) {
        WriteLine(output);
        if (!ReadResponse() || !lastResponseCode)
          return false;
      }
    } while (result == PSASLClient::Continue);
    auth.End();
  }
  else
#endif
  {
    if (!apopBanner.IsEmpty()) { // let's try with APOP

      PMessageDigest::Result bin_digest;
      PMessageDigest5::Encode(apopBanner + password, bin_digest);
      PString digest;

      const BYTE * data = bin_digest.GetPointer();

      for (PINDEX i = 0, max = bin_digest.GetSize(); i < max ; i++)
        digest.sprintf("%02x", (unsigned)data[i]);

      if (ExecuteCommand(APOP, username + " " + digest) > 0)
        return loggedIn = true;
    }

    // No SASL and APOP didn't work for us
    // If we really have to, we'll go with the plain old USER/PASS scheme

    if (!(options & AllowUserPass))
      return false;

    if (ExecuteCommand(USER, username) <= 0)
      return false;

    if (ExecuteCommand(PASS, password) <= 0)
      return false;
  }

  loggedIn = true;
  return true;
}


int PPOP3Client::GetMessageCount()
{
  if (ExecuteCommand(STATcmd, "") <= 0)
    return -1;

  return (int)lastResponseInfo.AsInteger();
}


PUnsignedArray PPOP3Client::GetMessageSizes()
{
  PUnsignedArray sizes;

  if (ExecuteCommand(LIST, "") > 0) {
    PString msgInfo;
    while (ReadLine(msgInfo) && isdigit(msgInfo[0]))
      sizes.SetAt((PINDEX)msgInfo.AsInteger()-1,
                  (unsigned)msgInfo.Mid(msgInfo.Find(' ')).AsInteger());
  }

  return sizes;
}


PStringArray PPOP3Client::GetMessageHeaders()
{
  PStringArray headers;

  int count = GetMessageCount();
  for (int msgNum = 1; msgNum <= count; msgNum++) {
    if (ExecuteCommand(TOP, PString(PString::Unsigned,msgNum) + " 0") > 0) {
      PString headerLine;
      while (ReadLine(headerLine, true))
        headers[msgNum-1] += headerLine;
    }
  }
  return headers;
}


PBoolean PPOP3Client::BeginMessage(PINDEX messageNumber)
{
  return ExecuteCommand(RETR, PString(PString::Unsigned, messageNumber)) > 0;
}


PBoolean PPOP3Client::DeleteMessage(PINDEX messageNumber)
{
  return ExecuteCommand(DELE, PString(PString::Unsigned, messageNumber)) > 0;
}


//////////////////////////////////////////////////////////////////////////////
// PPOP3Server

PPOP3Server::PPOP3Server()
{
}


PBoolean PPOP3Server::OnOpen()
{
  return WriteResponse(okResponse(), PIPSocket::GetHostName() +
                     " POP3 server ready at " +
                      PTime(PTime::MediumDateTime).AsString());
}


PBoolean PPOP3Server::ProcessCommand()
{
  PString args;
  PINDEX num;
  if (!ReadCommand(num, args))
    return false;

  switch (num) {
    case USER :
      OnUSER(args);
      break;
    case PASS :
      OnPASS(args);
      break;
    case QUIT :
      OnQUIT();
      return false;
    case RSET :
      OnRSET();
      break;
    case NOOP :
      OnNOOP();
      break;
    case STATcmd :
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
        WriteResponse(errResponse(), "Syntax error");
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

  return true;
}


void PPOP3Server::OnUSER(const PString & name)
{
  messageSizes.SetSize(0);
  messageIDs.SetSize(0);
  username = name;
  WriteResponse(okResponse(), "User name accepted.");
}


void PPOP3Server::OnPASS(const PString & password)
{
  if (username.IsEmpty())
    WriteResponse(errResponse(), "No user name specified.");
  else if (HandleOpenMailbox(username, password))
    WriteResponse(okResponse(), username + " mail is available.");
  else
    WriteResponse(errResponse(), "No access to " + username + " mail.");
  messageDeletions.SetSize(messageIDs.GetSize());
}


void PPOP3Server::OnQUIT()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    if (messageDeletions[i])
      HandleDeleteMessage(i+1, messageIDs[i]);

  WriteResponse(okResponse(), PIPSocket::GetHostName() +
                     " POP3 server signing off at " +
                      PTime(PTime::MediumDateTime).AsString());

  Close();
}


void PPOP3Server::OnRSET()
{
  for (PINDEX i = 0; i < messageDeletions.GetSize(); i++)
    messageDeletions[i] = false;
  WriteResponse(okResponse(), "Resetting state.");
}


void PPOP3Server::OnNOOP()
{
  WriteResponse(okResponse(), "Doing nothing.");
}


void PPOP3Server::OnSTAT()
{
  DWORD total = 0;
  for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
    total += messageSizes[i];
  WriteResponse(okResponse(), psprintf("%u %u", messageSizes.GetSize(), total));
}


void PPOP3Server::OnLIST(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse(), psprintf("%u messages.", messageSizes.GetSize()));
    for (PINDEX i = 0; i < messageSizes.GetSize(); i++)
      if (!messageDeletions[i])
        WriteLine(psprintf("%u %u", i+1, messageSizes[i]));
    WriteLine(".");
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse(), "No such message.");
  else
    WriteResponse(okResponse(), psprintf("%u %u", msg, messageSizes[msg-1]));
}


void PPOP3Server::OnRETR(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse(), "No such message.");
  else {
    WriteResponse(okResponse(),
                 PString(PString::Unsigned, messageSizes[msg-1]) + " octets.");
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], P_MAX_INDEX);
    stuffingState = DontStuff;
    WriteString(CRLFdotCRLF);
  }
}


void PPOP3Server::OnDELE(PINDEX msg)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse(), "No such message.");
  else {
    messageDeletions[msg-1] = true;
    WriteResponse(okResponse(), "Message marked for deletion.");
  }
}


void PPOP3Server::OnTOP(PINDEX msg, PINDEX count)
{
  if (msg < 1 || msg > messageDeletions.GetSize())
    WriteResponse(errResponse(), "No such message.");
  else {
    WriteResponse(okResponse(), "Top of message");
    stuffingState = StuffIdle;
    HandleSendMessage(msg, messageIDs[msg-1], count);
    stuffingState = DontStuff;
    WriteString(CRLFdotCRLF);
  }
}


void PPOP3Server::OnUIDL(PINDEX msg)
{
  if (msg == 0) {
    WriteResponse(okResponse(),
              PString(PString::Unsigned, messageIDs.GetSize()) + " messages.");
    for (PINDEX i = 0; i < messageIDs.GetSize(); i++)
      if (!messageDeletions[i])
        WriteLine(PString(PString::Unsigned, i+1) & messageIDs[i]);
    WriteLine(".");
  }
  else if (msg < 1 || msg > messageSizes.GetSize())
    WriteResponse(errResponse(), "No such message.");
  else
    WriteLine(PString(PString::Unsigned, msg) & messageIDs[msg-1]);
}


PBoolean PPOP3Server::OnUnknown(const PCaselessString & command)
{
  WriteResponse(errResponse(), "Command \"" + command + "\" unrecognised.");
  return true;
}


PBoolean PPOP3Server::HandleOpenMailbox(const PString &, const PString &)
{
  return false;
}


void PPOP3Server::HandleSendMessage(PINDEX, const PString &, PINDEX)
{
}


void PPOP3Server::HandleDeleteMessage(PINDEX, const PString &)
{
}


//////////////////////////////////////////////////////////////////////////////
// PRFC822Channel

const PCaselessString & PRFC822Channel::MimeVersionTag() { static const PConstCaselessString s("MIME-version"); return s; }
const PCaselessString & PRFC822Channel::FromTag()        { static const PConstCaselessString s("From");         return s; }
const PCaselessString & PRFC822Channel::ToTag()          { static const PConstCaselessString s("To");           return s; }
const PCaselessString & PRFC822Channel::CCTag()          { static const PConstCaselessString s("cc");           return s; }
const PCaselessString & PRFC822Channel::BCCTag()         { static const PConstCaselessString s("bcc");          return s; }
const PCaselessString & PRFC822Channel::SubjectTag()     { static const PConstCaselessString s("Subject");      return s; }
const PCaselessString & PRFC822Channel::DateTag()        { static const PConstCaselessString s("Date");         return s; }
const PCaselessString & PRFC822Channel::ReturnPathTag()  { static const PConstCaselessString s("Return-Path");  return s; }
const PCaselessString & PRFC822Channel::ReceivedTag()    { static const PConstCaselessString s("Received");     return s; }
const PCaselessString & PRFC822Channel::MessageIDTag()   { static const PConstCaselessString s("Message-ID");   return s; }
const PCaselessString & PRFC822Channel::MailerTag()      { static const PConstCaselessString s("X-Mailer");     return s; }



PRFC822Channel::PRFC822Channel(Direction direction)
{
  writeHeaders = direction == Sending;
  writePartHeaders = false;
  base64 = NULL;
}


PRFC822Channel::~PRFC822Channel()
{
  Close();
  delete base64;
}


PBoolean PRFC822Channel::Close()
{
  flush();
  NextPart(""); // Flush out all the parts
  return PIndirectChannel::Close();
}


PBoolean PRFC822Channel::Write(const void * buf, PINDEX len)
{
  flush();

  if (writeHeaders) {
    if (!headers.Contains(FromTag()) || !headers.Contains(ToTag()))
      return false;

    if (!headers.Contains(MimeVersionTag()))
      headers.SetAt(MimeVersionTag(), "1.0");

    if (!headers.Contains(DateTag()))
      headers.SetAt(DateTag(), PTime().AsString());

    if (writePartHeaders)
      headers.SetAt(ContentTypeTag(), "multipart/mixed; boundary=\""+boundaries.front()+'"');
    else if (!headers.Contains(ContentTypeTag()))
      headers.SetAt(ContentTypeTag(), PMIMEInfo::TextPlain());

    PStringStream hdr;
    hdr << headers;
    if (!PIndirectChannel::Write((const char *)hdr, hdr.GetLength()))
      return false;

    if (base64 != NULL)
      base64->StartEncoding();

    writeHeaders = false;

    flush();
  }

  if (writePartHeaders && (boundaries.GetSize() > 0)) {
    if (!partHeaders.Contains(ContentTypeTag()))
      partHeaders.SetAt(ContentTypeTag(), PMIMEInfo::TextPlain());

    PStringStream hdr;
    hdr << "\n--"  << boundaries.front() << '\n'
        << partHeaders;
    if (!PIndirectChannel::Write((const char *)hdr, hdr.GetLength()))
      return false;

    if (base64 != NULL)
      base64->StartEncoding();

    writePartHeaders = false;

    flush();
  }

  PBoolean ok;
  if (base64 == NULL)
    ok = PIndirectChannel::Write(buf, len);
  else {
    base64->ProcessEncoding(buf, len);
    PString str = base64->GetEncodedString();
    ok = PIndirectChannel::Write((const char *)str, str.GetLength());
  }

  flush();

  // Always return the last write count as the number of bytes expected to be
  // written, not teh actual number which with base64 encoding etc may be
  // significantly more.
  if (ok)
    SetLastWriteCount(len);
  return ok;
}


PBoolean PRFC822Channel::OnOpen()
{
  if (writeHeaders)
    return true;

  istream & this_stream = *this;
  this_stream >> headers;
  return !bad();
}


void PRFC822Channel::NewMessage(Direction direction)
{

  NextPart(""); // Flush out all the parts

  boundaries.RemoveAll();
  headers.RemoveAll();
  partHeaders.RemoveAll();
  writeHeaders = direction == Sending;
  writePartHeaders = false;
}


PString PRFC822Channel::MultipartMessage()
{
  PString boundary;

  do {
    boundary.sprintf("PTLib.%lu.%u", PTime().GetTimeInSeconds(), rand());
  } while (!MultipartMessage(boundary));

  return boundary;
}


PBoolean PRFC822Channel::MultipartMessage(const PString & boundary)
{
  writePartHeaders = true;
  for (PStringList::iterator i = boundaries.begin(); i != boundaries.end(); i++) {
    if (*i == boundary)
      return false;
  }

  if (boundaries.GetSize() > 0) {
    partHeaders.SetAt(ContentTypeTag(), "multipart/mixed; boundary=\""+boundary+'"');
    flush();
    writePartHeaders = true;
  }

  boundaries.Prepend(new PString(boundary));
  return true;
}


void PRFC822Channel::NextPart(const PString & boundary)
{
  if (base64 != NULL) {
    PBase64 * oldBase64 = base64;
    base64 = NULL;
    *this << oldBase64->CompleteEncoding() << '\n';
    delete oldBase64;
    flush();
  }

  while (boundaries.GetSize() > 0) {
    if (boundaries.front() == boundary)
      break;
    *this << "\n--" << boundaries.front() << "--\n";
    boundaries.RemoveHead();
  }

  flush();

  writePartHeaders = boundaries.GetSize() > 0;
  partHeaders.RemoveAll();
}


void PRFC822Channel::SetFromAddress(const PString & fromAddress)
{
  SetHeaderField(FromTag(), fromAddress);
}


void PRFC822Channel::SetToAddress(const PString & toAddress)
{
  SetHeaderField(ToTag(), toAddress);
}


void PRFC822Channel::SetCC(const PString & ccAddress)
{
  SetHeaderField(CCTag(), ccAddress);
}


void PRFC822Channel::SetBCC(const PString & bccAddress)
{
  SetHeaderField(BCCTag(), bccAddress);
}


void PRFC822Channel::SetSubject(const PString & subject)
{
  SetHeaderField(SubjectTag(), subject);
}


void PRFC822Channel::SetContentType(const PString & contentType)
{
  SetHeaderField(ContentTypeTag(), contentType);
}


void PRFC822Channel::SetContentAttachment(const PFilePath & file)
{
  PString name = file.GetFileName();
  SetHeaderField(ContentDispositionTag(), "attachment; filename=\"" + name + '"');
  SetHeaderField(ContentTypeTag(),
                 PMIMEInfo::GetContentType(file.GetType())+"; name=\"" + name + '"');
}


void PRFC822Channel::SetTransferEncoding(const PString & encoding, PBoolean autoTranslate)
{
  SetHeaderField(ContentTransferEncodingTag(), encoding);
  if ((encoding *= "base64") && autoTranslate)
    base64 = new PBase64;
  else {
    delete base64;
    base64 = NULL;
  }
}


void PRFC822Channel::SetHeaderField(const PString & name, const PString & value)
{
  if (writePartHeaders)
    partHeaders.SetAt(name, value);
  else if (writeHeaders)
    headers.SetAt(name, value);
  else
    PAssertAlways(PLogicError);
}


// End Of File ///////////////////////////////////////////////////////////////


