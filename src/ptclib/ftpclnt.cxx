/*
 * $Id: ftpclnt.cxx,v 1.2 1997/03/28 13:06:58 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: ftpclnt.cxx,v $
 * Revision 1.2  1997/03/28 13:06:58  robertj
 * made STAT command more robust for getting file info from weird FTP servers.
 *
 * Revision 1.1  1996/09/14 13:02:18  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>
#include <ftp.h>


/////////////////////////////////////////////////////////
//  FTP Client

PFTPClient::PFTPClient()
{
}


PFTPClient::~PFTPClient()
{
  Close();
}


BOOL PFTPClient::Close()
{
  if (!IsOpen())
    return FALSE;
  BOOL ok = ExecuteCommand(QUIT)/100 == 2;
  return PFTP::Close() && ok;
}

BOOL PFTPClient::OnOpen()
{
  if (!ReadResponse() || lastResponseCode != 220)
    return FALSE;

  // the default data port for a server is the adjacent port
  PIPSocket::Address remoteHost;
  PIPSocket * socket = GetSocket();
  if (socket == NULL)
    return FALSE;

  socket->GetPeerAddress(remoteHost, remotePort);
  remotePort--;
  return TRUE;
}


BOOL PFTPClient::LogIn(const PString & username, const PString & password)
{
  if (ExecuteCommand(USER, username)/100 != 3)
    return FALSE;
  return ExecuteCommand(PASS, password)/100 == 2;
}


PString PFTPClient::GetSystemType()
{
  if (ExecuteCommand(SYST)/100 != 2)
    return PString();

  return lastResponseInfo.Left(lastResponseInfo.Find(' '));
}


BOOL PFTPClient::SetType(RepresentationType type)
{
  static const char * const typeCode[] = { "A", "E", "I" };
  PAssert(type < PARRAYSIZE(typeCode), PInvalidParameter);
  return ExecuteCommand(TYPE, typeCode[type])/100 == 2;
}


BOOL PFTPClient::ChangeDirectory(const PString & dirPath)
{
  return ExecuteCommand(CWD, dirPath)/100 == 2;
}


PString PFTPClient::GetCurrentDirectory()
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


PStringArray PFTPClient::GetDirectoryNames(NameTypes type,
                                           DataChannelType ctype)
{
  return GetDirectoryNames(PString(), type, ctype);
}


PStringArray PFTPClient::GetDirectoryNames(const PString & path,
                                          NameTypes type,
                                          DataChannelType ctype)
{
  SetType(PFTP::ASCII);

  Commands lcmd = type == DetailedNames ? LIST : NLST;
  PTCPSocket * socket = ctype != Passive ? NormalClientTransfer(lcmd, path)
                                         : PassiveClientTransfer(lcmd, path);
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


PString PFTPClient::GetFileStatus(const PString & path, DataChannelType ctype)
{
  if (ExecuteCommand(STAT, path)/100 == 2 && lastResponseInfo.Find(path) != P_MAX_INDEX) {
    PINDEX start = lastResponseInfo.Find('\n');
    if (start != P_MAX_INDEX) {
      PINDEX end = lastResponseInfo.Find('\n', ++start);
      if (end != P_MAX_INDEX)
        return lastResponseInfo(start, end-1);
    }
  }

  PTCPSocket * socket = ctype != Passive ? NormalClientTransfer(LIST, path)
                                         : PassiveClientTransfer(LIST, path);
  if (socket == NULL)
    return PString();

  PString str;
  socket->Read(str.GetPointer(200), 199);
  str[socket->GetLastReadCount()] = '\0';
  delete socket;
  ReadResponse();

  PINDEX end = str.FindOneOf("\r\n");
  if (end != P_MAX_INDEX)
    str[end] = '\0';
  return str;
}


PTCPSocket * PFTPClient::NormalClientTransfer(Commands cmd,
                                              const PString & args)
{
  // setup a socket so we can tell the host where to connect to
  PTCPSocket listenSocket;
  listenSocket.Listen();

  // get host address and port to send to other end
  WORD localPort = listenSocket.GetPort();
  PIPSocket::Address localAddr;
  PIPSocket * socket = GetSocket();
  if (socket != NULL)
    socket->GetLocalAddress(localAddr);

  // send PORT command to host
  if (!SendPORT(localAddr, localPort))
    return NULL;

  if (ExecuteCommand(cmd, args)/100 != 1)
    return NULL;

  PTCPSocket * dataSocket = new PTCPSocket(listenSocket);
  if (dataSocket->IsOpen())
    return dataSocket;

  delete dataSocket;
  return NULL;
}


PTCPSocket * PFTPClient::PassiveClientTransfer(Commands cmd,
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


PTCPSocket * PFTPClient::GetFile(const PString & filename,
                                 DataChannelType channel)
{
  return channel != Passive ? NormalClientTransfer(RETR, filename)
                            : PassiveClientTransfer(RETR, filename);
}


PTCPSocket * PFTPClient::PutFile(const PString & filename,
                                 DataChannelType channel)
{
  return channel != Passive ? NormalClientTransfer(STOR, filename)
                            : PassiveClientTransfer(STOR, filename);
}



// End of File ///////////////////////////////////////////////////////////////
