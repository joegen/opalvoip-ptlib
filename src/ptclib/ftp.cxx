/*
 * $Id: ftp.cxx,v 1.5 1996/03/31 09:01:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: ftp.cxx,v $
 * Revision 1.5  1996/03/31 09:01:20  robertj
 * More FTP client implementation.
 *
 * Revision 1.4  1996/03/26 00:50:30  robertj
 * FTP Client Implementation.
 *
 * Revision 1.3  1996/03/18 13:33:15  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
 *
 * Revision 1.2  1996/03/16 04:51:12  robertj
 * Changed lastResponseCode to an integer.
 *
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
  : PApplicationSocket(NumCommands, FTPCommands, port)
{
  Construct();
  Connect(address);
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


BOOL PFTPSocket::Close()
{
  if (state == ClientConnect)
    ExecuteCommand(QUIT);
  else
    lastResponseCode = 200;
  state = NotConnected;
  return PApplicationSocket::Close() && lastResponseCode/100 == 2;
}


BOOL PFTPSocket::Connect(const PString & address)
{
  if (!PApplicationSocket::Connect(address))
    return FALSE;

  if (!ReadResponse() || lastResponseCode != 220) {
    Close();
    return FALSE;
  }

  // the default data port for a server is the adjacent port
  GetPeerAddress(remoteHost, remotePort);
  remotePort--;
  state = ClientConnect;

  return TRUE;
}


BOOL PFTPSocket::LogIn(const PString & username, const PString & password)
{
  if (ExecuteCommand(USER, username)/100 != 3)
    return FALSE;
  return ExecuteCommand(PASS, password)/100 == 2;
}


PString PFTPSocket::GetSystemType()
{
  if (ExecuteCommand(SYST)/100 != 2)
    return PString();

  return lastResponseInfo.Left(lastResponseInfo.Find(' '));
}


BOOL PFTPSocket::SetType(RepresentationType type)
{
  static const char * const typeCode[] = { "A", "E", "I" };
  PAssert(type < PARRAYSIZE(typeCode), PInvalidParameter);
  return ExecuteCommand(TYPE, typeCode[type])/100 == 2;
}


BOOL PFTPSocket::ChangeDirectory(const PString & dirPath)
{
  return ExecuteCommand(CWD, dirPath)/100 == 2;
}


PString PFTPSocket::GetCurrentDirectory()
{
  if (ExecuteCommand(PWD) != 257)
    return PString();

  PINDEX quote1 = lastResponseInfo.Find('"');
  if (quote1 == P_MAX_INDEX)
    return PString();

  PINDEX quote2 = quote1 + 1;
  do {
    quote2 = lastResponseInfo.Find('"', quote2);
    if (quote2 == P_MAX_INDEX)
      return PString();

    while (lastResponseInfo[quote2]=='"' && lastResponseInfo[quote2+1]=='"')
      quote2 += 2;

  } while (lastResponseInfo[quote2] != '"');

  return lastResponseInfo(quote1+1, quote2-1);
}


PStringArray PFTPSocket::GetDirectoryNames(NameTypes type)
{
  return GetDirectoryNames(PString(), type);
}


PStringArray PFTPSocket::GetDirectoryNames(const PString & path,
                                          NameTypes type)
{
  SetType(PFTPSocket::ASCII);

  PTCPSocket * socket =
              PassiveClientTransfer(type == DetailedNames ? LIST : NLST, path);
  if (socket == NULL)
    return PStringArray();

  PString response = lastResponseInfo;
  PString str;
  int count = 0;
  while(socket->Read(str.GetPointer(count+1000)+count, 1000))
    count += socket->GetLastReadCount();
  str.SetSize(count+1);

  delete socket;
  ReadResponse();
  lastResponseInfo = response + '\n' + lastResponseInfo;
  return str.Lines();
}


PString PFTPSocket::GetFileStatus(const PString & path)
{
  if (ExecuteCommand(STAT, path)/100 != 2)
    return PString();

  PINDEX start = lastResponseInfo.Find('\n');
  if (start == P_MAX_INDEX)
    return PString();

  PINDEX end = lastResponseInfo.Find('\n', ++start);
  if (end == P_MAX_INDEX)
    return PString();

  return lastResponseInfo(start, end-1);
}


PTCPSocket * PFTPSocket::NormalClientTransfer(Commands cmd,
                                              const PString & args)
{
  // setup a socket so we can tell the host where to connect to
  PTCPSocket listenSocket;
  listenSocket.Listen();

  // get host address and port to send to other end
  WORD localPort = listenSocket.GetPort();
  PIPSocket::Address localAddr;
  PIPSocket::GetHostAddress(localAddr);

  // send PORT command to host
  if (!SendPORT(localAddr, localPort))
    return NULL;

  if (ExecuteCommand(cmd, args)/100 != 1)
    return NULL;

  PTCPSocket * socket = new PTCPSocket(listenSocket);
  if (socket->IsOpen())
    return socket;

  delete socket;
  return NULL;
}


PTCPSocket * PFTPSocket::PassiveClientTransfer(Commands cmd,
                                               const PString & args)
{
  PIPSocket::Address passiveAddress;
  WORD passivePort;

  if (ExecuteCommand(PASV) != 227)
    return NULL;

  PINDEX start = lastResponseInfo.FindOneOf("0123456789");
  if (start == P_MAX_INDEX)
    return NULL;

  PStringArray bytes = lastResponseInfo(start, P_MAX_INDEX).Tokenise(',');
  if (bytes.GetSize() != 6)
    return NULL;

  passiveAddress = PIPSocket::Address((BYTE)bytes[0].AsInteger(),
                                      (BYTE)bytes[1].AsInteger(),
                                      (BYTE)bytes[2].AsInteger(),
                                      (BYTE)bytes[3].AsInteger());
  passivePort = (WORD)(bytes[4].AsInteger()*256 + bytes[5].AsInteger());

  PTCPSocket * socket = new PTCPSocket(passiveAddress, passivePort);
  if (socket->IsOpen())
    if (ExecuteCommand(cmd, args)/100 == 1)
      return socket;

  delete socket;
  return NULL;
}


PTCPSocket * PFTPSocket::GetFile(const PString & filename,
                                 DataChannelType channel)
{
  return channel != Passive ? NormalClientTransfer(RETR, filename)
                            : PassiveClientTransfer(RETR, filename);
}


PTCPSocket * PFTPSocket::PutFile(const PString & filename,
                                 DataChannelType channel)
{
  return channel != Passive ? NormalClientTransfer(STOR, filename)
                            : PassiveClientTransfer(STOR, filename);
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
  PString str(PString::Printf,
              "Entering Passive Mode (%i,%i,%i,%i,%i,%i)",
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
  int code;
  return SendPORT(addr, port, code, info);
}


BOOL PFTPSocket::SendPORT(const PIPSocket::Address & addr, WORD port,
                          int & code, PString & info)
{
  PString str(PString::Printf,
              "%i,%i,%i,%i,%i,%i",
              addr.Byte1(),
              addr.Byte2(),
              addr.Byte3(),
              addr.Byte4(),
              port/256,
              port%256);

  if (!WriteCommand(PORT, str))
    return FALSE;

  return ReadResponse(code, info) && lastResponseCode/100 == 2;
}


// End of File ///////////////////////////////////////////////////////////////
