/*
 * $Id: ftp.cxx,v 1.1 1996/03/04 12:12:51 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: ftp.cxx,v $
 * Revision 1.1  1996/03/04 12:12:51  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>
#include <appsock.h>
#include <ftp.h>


#define	READY_STRING  "PWLib FTP Server v1.0 ready"
#define	GOOBYE_STRING "Goodbye"


static const char * FTPCommands[PFTPSocket::NumCommands] = 
{
  "USER", "PASS", "ACCT", "CWD", "CDUP", "SMNT", "QUIT", "REIN", "PORT", "PASV",
  "TYPE", "STRU", "MODE", "RETR", "STOR", "STOU", "APPE", "ALLO", "REST", "RNFR",
  "RNTO", "ABOR", "DELE", "RMD", "MKD", "PWD", "LIST", "NLST", "SITE", "SYST",
  "STAT", "HELP", "NOOP"
};

static const int RequiresLogin[PFTPSocket::NumCommands] = {
  1, // USER
  1, // PASS
  0, // ACCT
  0, // CWD
  0, // CDUP
  0, // SMNT
  1, // QUIT
  0, // REIN
  1, // PORT
  0, // PASV
  1, // TYPE
  1, // STRU
  1, // MODE
  0, // RETR
  0, // STOR
  0, // STOU
  0, // APPE
  0, // ALLO
  0, // REST
  0, // RNFR
  0, // RNTO
  1, // ABOR
  0, // DELE
  0, // RMD
  0, // MKD
  0, // PWD
  0, // LIST
  0, // NLST
  1, // SITE
  1, // SYST
  1, // STAT
  1, // HELP
  1, // NOOP
};


/////////////////////////////////////////////////////////
//
//  FTP Socket
//

PFTPSocket::PFTPSocket(WORD port)
  : PApplicationSocket(NumCommands, FTPCommands, port)
{
  Construct();
}


PFTPSocket::PFTPSocket(const PString & address, WORD port)
  : PApplicationSocket(NumCommands, FTPCommands, address, port)
{
  Construct();
  if (!Connect(address))
    return;
  // do not put any implementation here - put into Connect instead
}


PFTPSocket::PFTPSocket(PSocket & socket, const PString & readyString)
  : PApplicationSocket(NumCommands, FTPCommands, socket)
{
  ConstructServerSocket(readyString);
}


PFTPSocket::PFTPSocket(PSocket & socket)
  : PApplicationSocket(NumCommands, FTPCommands, socket)
{
  ConstructServerSocket(PIPSocket::GetHostName() & READY_STRING);
}


void PFTPSocket::ConstructServerSocket(const PString & readyString)
{
  Construct();

  state = NeedUser;
  WriteResponse(220, readyString);

  // the default data port for a client is the same port
  GetPeerAddress(remoteHost, remotePort);
}


PFTPSocket::~PFTPSocket()
{
  delete passiveSocket;
  Close();
}


void PFTPSocket::Construct()
{
  illegalPasswordCount = 0;
  state     = NotConnected;
  type      = 'A';
  structure = 'F';
  mode      = 'S';
  passiveSocket = NULL;
}


BOOL PFTPSocket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (!ReadResponse() || lastResponseCode != "220") {
    Close();
    return FALSE;
  }

  // the default data port for a server is the adjacent port
  GetPeerAddress(remoteHost, remotePort);
  remotePort--;

  return TRUE;
}


PString PFTPSocket::GetHelloString(const PString & user) const
{
  return PString("User") & user & "logged in.";
}


PString PFTPSocket::GetGoodbyeString(const PString &) const
{
  return PString(GOOBYE_STRING);
}


PString PFTPSocket::GetSystemTypeString() const
{
  return PProcess::GetOSClass() + " " + PProcess::GetOSName() + " " + PProcess::GetOSVersion();
}


BOOL PFTPSocket::AuthoriseUser(const PString &, const PString &, BOOL &)
{
  return TRUE;
}


BOOL PFTPSocket::ProcessCommand()
{
  PString args;
  PINDEX code;
  if (!ReadCommand(code, args))
    return FALSE;

  if (code == P_MAX_INDEX)
    return OnUnknown(args);

  //  handle commands that require login
  if (state == Connected || !CheckLoginRequired(code)) 
    return DispatchCommand(code, args);
  
  // otherwise enforce login
  WriteResponse(530, "Please login with USER and PASS.");
  return TRUE;
}


BOOL PFTPSocket::DispatchCommand(PINDEX code, const PString & args)
{
  switch (code) {

    // mandatory commands
    case USER:
      return OnUSER(args);
    case PASS:
      return OnPASS(args);
    case QUIT:
      return OnQUIT(args);
    case PORT:
      return OnPORT(args);
    case STRU:
      return OnSTRU(args);
    case MODE:
      return OnMODE(args);
    case NOOP:
      return OnNOOP(args);
    case TYPE:
      return OnTYPE(args);
    case RETR:
      return OnRETR(args);
    case STOR:
      return OnSTOR(args);
    case SYST:
      return OnSYST(args);
    case STAT:
      return OnSTAT(args);
    case ACCT:
      return OnACCT(args);
    case CWD:
      return OnCWD(args);
    case CDUP:
      return OnCDUP(args);
    case PASV:
      return OnPASV(args);
    case APPE:
      return OnAPPE(args);
    case RNFR:
      return OnRNFR(args);
    case RNTO:
      return OnRNTO(args);
    case DELE:
      return OnDELE(args);
    case RMD:
      return OnRMD(args);
    case MKD:
      return OnMKD(args);
    case PWD:
      return OnPWD(args);
    case LIST:
      return OnLIST(args);
    case NLST:
      return OnNLST(args);

    // optional commands
    case HELP:
      return OnHELP(args);
    case SITE:
      return OnSITE(args);
    case ABOR:
      return OnABOR(args);
    case SMNT:
      return OnSMNT(args);
    case REIN:
      return OnREIN(args);
    case STOU:
      return OnSTOU(args);
    case ALLO:
      return OnALLO(args);
    case REST:
      return OnREST(args);
    default:
      PAssertAlways("Registered FTP command not handled");
      return FALSE;
  }
  return TRUE;
}


BOOL PFTPSocket::CheckLoginRequired(PINDEX cmd)
{
  if (cmd < NumCommands)
    return RequiresLogin[cmd] == 0;
  else
    return TRUE;
}


BOOL PFTPSocket::OnUnknown(const PCaselessString & command)
{
  WriteResponse(500, "\"" + command + "\" command unrecognised.");
  return TRUE;
}


void PFTPSocket::OnNotImplemented(PINDEX cmdNum)
{
  if (cmdNum < commandNames.GetSize())
    WriteResponse(502, "Command \"" + commandNames[cmdNum] + "\" not implemented");
}


void PFTPSocket::OnSyntaxError(PINDEX cmdNum)
{
  if (cmdNum < commandNames.GetSize())
    WriteResponse(501, "\"" + commandNames[cmdNum] + "\" : syntax error in parameters or arguments.");
}


void PFTPSocket::OnCommandSuccessful(PINDEX cmdNum)
{
  if (cmdNum < commandNames.GetSize())
    WriteResponse(200, "\"" + commandNames[cmdNum] + "\" command successful.");
}


// mandatory commands that can be performed without loggin in

BOOL PFTPSocket::OnUSER(const PCaselessString & args)
{
  userName = args;
  state    = NeedPassword;
  WriteResponse(331, "Password required for " + args + ".");
  return TRUE;
}


BOOL PFTPSocket::OnPASS(const PCaselessString & args)
{
  BOOL replied = FALSE;
  if (state != NeedPassword) 
    WriteResponse(503, "Login with USER first.");
  else if (!AuthoriseUser(userName, args, replied)) {
    if (!replied)
      WriteResponse(530, "Login incorrect.");
    if (illegalPasswordCount++ == MaxIllegalPasswords)
      return FALSE;
  } else {
    if (!replied)
      WriteResponse(230, GetHelloString(userName));
    illegalPasswordCount = 0;
    state = Connected;
  }
  return TRUE;
}


BOOL PFTPSocket::OnQUIT(const PCaselessString & userName)
{
  WriteResponse(221, GetGoodbyeString(userName));
  return FALSE;
}


BOOL PFTPSocket::OnPORT(const PCaselessString & args)
{
  PStringArray tokens = args.Tokenise(",");

  long values[6];
  int len = PMIN(args.GetSize(), 6);

  for (PINDEX i = 0; i < len; i++) {
    values[i] = tokens[i].AsInteger();
    if (values[i] < 0 || values[i] > 255)
      break;
  }
  if (i < 6) 
    OnSyntaxError(PORT);
  else {
    remoteHost = PIPSocket::Address((BYTE)values[0],
                            (BYTE)values[1], (BYTE)values[2], (BYTE)values[3]);
    remotePort = (WORD)(values[4]*256 + values[5]);
    OnCommandSuccessful(PORT);
  }
  return TRUE;
}


BOOL PFTPSocket::OnPASV(const PCaselessString &)
{
  if (passiveSocket != NULL)
    delete passiveSocket;

  passiveSocket = PNEW PTCPSocket;
  passiveSocket->Listen();

  WORD portNo = passiveSocket->GetPort();
  PIPSocket::Address ourAddr;
  GetHostAddress(ourAddr);
  PString str = psprintf("Entering Passive Mode (%i,%i,%i,%i,%i,%i)",
                          ourAddr.Byte1(),
                          ourAddr.Byte2(),
                          ourAddr.Byte3(),
                          ourAddr.Byte4(),
                          portNo/256, portNo%256);

  return WriteResponse(227, str);
}


BOOL PFTPSocket::OnTYPE(const PCaselessString & args)
{
  if (args.IsEmpty())
    OnSyntaxError(TYPE);
  else {
    switch (toupper(args[0])) {
      case 'A':
        type = 'A';
        break;
      case 'I':
        type = 'I';
        break;
      case 'E':
      case 'L':
        WriteResponse(504, PString("TYPE not implemented for parameter ") + args);
        return TRUE;

      default:
        OnSyntaxError(TYPE);
        return TRUE;
    }
  }
  OnCommandSuccessful(TYPE);
  return TRUE;
}


BOOL PFTPSocket::OnMODE(const PCaselessString & args)
{
  if (args.IsEmpty())
    OnSyntaxError(MODE);
  else {
    switch (toupper(args[0])) {
      case 'S':
        structure = 'S';
        break;
      case 'B':
      case 'C':
        WriteResponse(504, PString("MODE not implemented for parameter ") + args);
        return TRUE;
      default:
        OnSyntaxError(MODE);
        return TRUE;
    }
  }
  OnCommandSuccessful(MODE);
  return TRUE;
}


BOOL PFTPSocket::OnSTRU(const PCaselessString & args)
{
  if (args.IsEmpty())
    OnSyntaxError(STRU);
  else {
    switch (toupper(args[0])) {
      case 'F':
        structure = 'F';
        break;
      case 'R':
      case 'P':
        WriteResponse(504, PString("STRU not implemented for parameter ") + args);
        return TRUE;
      default:
        OnSyntaxError(STRU);
        return TRUE;
    }
  }
  OnCommandSuccessful(STRU);
  return TRUE;
}


BOOL PFTPSocket::OnNOOP(const PCaselessString &)
{
  OnCommandSuccessful(NOOP);
  return TRUE;
}


// mandatory commands that cannot be performed without logging in

BOOL PFTPSocket::OnRETR(const PCaselessString &)
{
  OnNotImplemented(RETR);
  return TRUE;
}


BOOL PFTPSocket::OnSTOR(const PCaselessString &)
{
  OnNotImplemented(STOR);
  return TRUE;
}


BOOL PFTPSocket::OnACCT(const PCaselessString &)
{
  WriteResponse(532, "Need account for storing files");
  return TRUE;
}


BOOL PFTPSocket::OnCWD(const PCaselessString &)
{
  OnNotImplemented(CWD);
  return TRUE;
}


BOOL PFTPSocket::OnCDUP(const PCaselessString &)
{
  OnNotImplemented(CDUP);
  return TRUE;
}


BOOL PFTPSocket::OnSMNT(const PCaselessString &)
{
  OnNotImplemented(SMNT);
  return TRUE;
}


BOOL PFTPSocket::OnREIN(const PCaselessString &)
{
  OnNotImplemented(REIN);
  return TRUE;
}


BOOL PFTPSocket::OnSTOU(const PCaselessString &)
{
  OnNotImplemented(STOU);
  return TRUE;
}


BOOL PFTPSocket::OnAPPE(const PCaselessString &)
{
  OnNotImplemented(APPE);
  return TRUE;
}


BOOL PFTPSocket::OnALLO(const PCaselessString &)
{
  OnNotImplemented(ALLO);
  return TRUE;
}


BOOL PFTPSocket::OnREST(const PCaselessString &)
{
  OnNotImplemented(REST);
  return TRUE;
}


BOOL PFTPSocket::OnRNFR(const PCaselessString &)
{
  OnNotImplemented(RNFR);
  return TRUE;
}


BOOL PFTPSocket::OnRNTO(const PCaselessString &)
{
  OnNotImplemented(RNTO);
  return TRUE;
}


BOOL PFTPSocket::OnABOR(const PCaselessString &)
{
  OnNotImplemented(ABOR);
  return TRUE;
}


BOOL PFTPSocket::OnDELE(const PCaselessString &)
{
  OnNotImplemented(DELE);
  return TRUE;
}


BOOL PFTPSocket::OnRMD(const PCaselessString &)
{
  OnNotImplemented(RMD);
  return TRUE;
}


BOOL PFTPSocket::OnMKD(const PCaselessString &)
{
  OnNotImplemented(MKD);
  return TRUE;
}


BOOL PFTPSocket::OnPWD(const PCaselessString &)
{
  OnNotImplemented(PWD);
  return TRUE;
}


BOOL PFTPSocket::OnLIST(const PCaselessString &)
{
  OnNotImplemented(LIST);
  return TRUE;
}


BOOL PFTPSocket::OnNLST(const PCaselessString &)
{
  OnNotImplemented(NLST);
  return TRUE;
}


BOOL PFTPSocket::OnSITE(const PCaselessString &)
{
  OnNotImplemented(SITE);
  return TRUE;
}


BOOL PFTPSocket::OnSYST(const PCaselessString &)
{
  WriteResponse(215, GetSystemTypeString());
  return TRUE;
}


BOOL PFTPSocket::OnSTAT(const PCaselessString &)
{
  OnNotImplemented(STAT);
  return TRUE;
}


BOOL PFTPSocket::OnHELP(const PCaselessString &)
{
  OnNotImplemented(HELP);
  return TRUE;
}


void PFTPSocket::SendToClient(const PFilePath & filename)
{
  if (!PFile::Exists(filename)) 
    WriteResponse(450, filename + ": file not found");
  else {
    PTCPSocket * dataSocket;
    if (passiveSocket != NULL) {
      dataSocket = PNEW PTCPSocket(*passiveSocket);
      delete passiveSocket;
      passiveSocket = NULL;
    } else
      dataSocket = PNEW PTCPSocket(remoteHost, remotePort);
    if (!dataSocket->IsOpen())
      WriteResponse(425, "Cannot open data connection");
    else {
      if (type == 'A') {
        PTextFile file(filename, PFile::ReadOnly);
        if (!file.IsOpen())
          WriteResponse(450, filename + ": cannot open file");
        else {
          PString fileSize(PString::Unsigned, file.GetLength());
          WriteResponse(150, PString("Opening ASCII data connection for " + filename.GetFileName() + "(" + fileSize + " bytes)"));
          PString line;
          BOOL ok = TRUE;
          while (ok && file.ReadLine(line)) {
            if (!dataSocket->Write((const char *)line, line.GetLength())) {
              WriteResponse(426, "Connection closed - transfer aborted");
              ok = FALSE;
            }
          }
          file.Close();
        }
      } else {
        PFile file(filename, PFile::ReadOnly);
        if (!file.IsOpen())
          WriteResponse(450, filename + ": cannot open file");
        else {
          PString fileSize(PString::Unsigned, file.GetLength());
          WriteResponse(150, PString("Opening BINARY data connection for " + filename.GetFileName() + "(" + fileSize + " bytes)"));
          BYTE buffer[2048];
          BOOL ok = TRUE;
          while (ok && file.Read(buffer, 2048)) {
            if (!dataSocket->Write(buffer, file.GetLastReadCount())) {
              WriteResponse(426, "Connection closed - transfer aborted");
              ok = FALSE;
            }
          }
          file.Close();
        }
      }
      delete dataSocket;
      WriteResponse(226, "Transfer complete");
    }
  }
}


BOOL PFTPSocket::SendPORT(const PIPSocket::Address & addr, WORD port)
{
  PString info;
  PINDEX code;
  return SendPORT(addr, port, code, info);
}


BOOL PFTPSocket::SendPORT(const PIPSocket::Address & addr, WORD port,
                          PINDEX & code, PString & info)
{
  PString str(psprintf("%i,%i,%i,%i,%i,%i",
              addr.Byte1(),
              addr.Byte2(),
              addr.Byte3(),
              addr.Byte4(),
              port/256,
              port%256));

  if (!WriteCommand(PORT, str))
    return FALSE;

  PString codeStr;
  BOOL resp = ReadResponse(codeStr, info);
  code = codeStr.AsInteger();
  return resp;
}


// End of File ///////////////////////////////////////////////////////////////
