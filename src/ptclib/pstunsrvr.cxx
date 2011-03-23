/*
 * Implementation of RFC 5389 - Session Traversal Utilities for NAT (STUN)
 */

#ifdef __GNUC__
#pragma implementation "pstunsrvr.h"
#endif

#include <ptlib.h>
#include <ptclib/pstun.h>
#include <ptclib/random.h>
#include <ptclib/pstunsrvr.h>

#define new PNEW


#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

//////////////////////////////////////////////////

PSTUNServer::SocketInfo::SocketInfo()
  : m_socket(NULL)
  , m_alternatePortSocket(NULL)
  , m_alternateAddressSocket(NULL)
  , m_alternateAddressAndPortSocket(NULL)
{
}

//////////////////////////////////////////////////

PSTUNServer::PSTUNServer()
  : m_autoDelete(true)
{
}

bool PSTUNServer::Open(WORD port)
{
  Close();

  PIPSocket::Address addr;

  // get interfaces to bind to
  PIPSocket::InterfaceTable interfaces;
  if (!PIPSocket::GetInterfaceTable(interfaces)) {
    PTRACE(2, "PSTUNSRVR\nGetInterfaceTable failed");
    return false;
  }

  int i;
  std::vector<PIPSocket::Address> interfaceAddresses;

  // find interfaces
  for (i = 0; i < interfaces.GetSize(); i++) {
    if (!interfaces[i].GetAddress().IsLinkLocal()
        && !interfaces[i].GetAddress().IsLoopback()
#ifndef _DEBUG
        && !interfaces[i].GetAddress().IsRFC1918()
#endif
        ) {
      interfaceAddresses.push_back(interfaces[i].GetAddress());
    }
  }

  if (interfaceAddresses.size() == 0) {
    PTRACE(2, "PSTUNSRVR\nno suitable interfaces found");
    return false;
  }

  // open sockets
  size_t j;
  for (j = 0; j < interfaceAddresses.size(); ++j) {
    PIPSocketAddressAndPort addr(interfaceAddresses[j], port);
    if (!CreateAndAddSocket(interfaceAddresses[j], port)) {
      PTRACE(2, "PSTUNSRVR\nCannot open socket on " << addr);
      Close();
      return false;
    }
    PTRACE(2, "PSTUNSRVR\nListening on " << addr);
  }

  if (m_sockets.GetSize() == 0) {
    PTRACE(2, "PSTUNSRVR\nUnable to open any ports" << addr);
    return false;
  }

  // open and populate alternates
  if (m_sockets.GetSize() > 1) {
    SocketToSocketInfoMap::iterator r = m_socketToSocketInfoMap.begin();

    // primary socket
    SocketInfo & primary = r->second;
    ++r;

    // sedcondary socket
    SocketInfo & secondary = r->second;

    // primary socket, alternate port
    SocketInfo * primaryAlternate = CreateAndAddSocket(primary.m_socketAddress.GetAddress(), primary.m_socketAddress.GetPort()+1);
    if (primaryAlternate == NULL) {
      PTRACE(2, "PSTUNSRVR\nCannot open primary alternate port socket on " << addr);
      return false;
    }
    PTRACE(2, "PSTUNSRVR\nListening on " << primaryAlternate->m_socketAddress);

    // secondary socket, alternate port
    SocketInfo * secondaryAlternate = CreateAndAddSocket(secondary.m_socketAddress.GetAddress(), secondary.m_socketAddress.GetPort()+1);
    if (secondaryAlternate == NULL) {
      PTRACE(2, "PSTUNSRVR\nCannot open secondary secondary port socket on " << addr);
      return false;
    }
    PTRACE(2, "PSTUNSRVR\nListening on " << secondaryAlternate->m_socketAddress);

    PopulateInfo(primary,           secondary.m_socketAddress.GetAddress(), port+1, primaryAlternate->m_socket, secondary.m_socket,           secondaryAlternate->m_socket);
    PopulateInfo(*primaryAlternate, secondary.m_socketAddress.GetAddress(), port,   primary.m_socket,           secondaryAlternate->m_socket, secondary.m_socket);

    PopulateInfo(secondary,           primary.m_socketAddress.GetAddress(), port+1, secondaryAlternate->m_socket, primary.m_socket,           primaryAlternate->m_socket);
    PopulateInfo(*secondaryAlternate, primary.m_socketAddress.GetAddress(), port,   secondary.m_socket,           primaryAlternate->m_socket, primary.m_socket);
  }

  m_sockets.AllowDeleteObjects(m_autoDelete);
  return true;
}

void PSTUNServer::PopulateInfo(PSTUNServer::SocketInfo & info, const PIPSocket::Address & alternateAddress, WORD alternatePort, 
             PUDPSocket * alternatePortSocket, PUDPSocket * alternateAddressSocket, PUDPSocket * alternateAddressAndPortSocket)
{
  info.m_alternateAddressAndPort = PIPSocketAddressAndPort(alternateAddress, alternatePort);

  info.m_alternatePortSocket           = alternatePortSocket;
  info.m_alternateAddressSocket        = alternateAddressSocket;
  info.m_alternateAddressAndPortSocket = alternateAddressAndPortSocket;
}

PSTUNServer::SocketInfo * PSTUNServer::CreateAndAddSocket(const PIPSocket::Address & address, WORD port)
{
  PUDPSocket * sock = new PUDPSocket();
  if (!sock->Listen(address, 5, port)) {
    delete sock;
    return NULL;
  }

  m_sockets.Append(sock);
  SocketInfo info;
  info.m_socket        = sock;
  info.m_socketAddress = PIPSocketAddressAndPort(address, port);
  return &m_socketToSocketInfoMap.insert(SocketToSocketInfoMap::value_type(sock, info)).first->second;
}

bool PSTUNServer::IsOpen() const 
{ 
  return m_sockets.GetSize() > 0; 
}

bool PSTUNServer::Close()
{
  m_sockets.SetSize(0);
  m_selectList.SetSize(0);
  m_socketToSocketInfoMap.clear();

  return true;
}

bool PSTUNServer::Read(PSTUNMessage & message, PSTUNServer::SocketInfo & socketInfo)
{
  message.SetSize(0);

  if (!IsOpen())
    return false;

  if (m_selectList.GetSize() == 0) {
    m_selectList = m_sockets;
    //m_selectList.MakeUnique();
    int r = PIPSocket::Select(m_selectList);
    if (r == PChannel::Timeout)
      return true;
    if (r != PChannel::NoError)
      return false;
    if (m_selectList.GetSize() == 0)
      return true;
  }

  PUDPSocket * socket = (PUDPSocket *)&m_selectList[0];
  if (!message.Read(*socket)) {
    PTRACE(2, "STUNSRVR\tRead failed");
    return false;
  }  

  SocketToSocketInfoMap::iterator r = m_socketToSocketInfoMap.find(socket);
  if (r == m_socketToSocketInfoMap.end()) {
    PTRACE(2, "STUNSRVR\tUnable to find interface for received request - ignoring");
    return false;
  }
  socketInfo = r->second;
  return true;
}

bool PSTUNServer::Process(const PSTUNMessage & message, PSTUNServer::SocketInfo & socketInfo)
{
  int type = message.GetType();

  // decode requests
  if (IS_REQUEST(type)) {
    if (type == PSTUNMessage::BindingRequest)
      return OnBindingRequest(message, socketInfo);
    else
      return OnUnknownRequest(message, socketInfo);
  }

  return false;
}

bool PSTUNServer::WriteTo(const PSTUNMessage & message, PUDPSocket & socket, const PIPSocketAddressAndPort & dest)
{
  socket.SetSendAddress(dest);
  return message.Write(socket);
}

bool PSTUNServer::OnUnknownRequest(const PSTUNMessage & request, PSTUNServer::SocketInfo & /*socketInfo*/)
{
  PTRACE(2, "STUNSRVR\tReceived unknown request " << hex << request.GetType() << " from " << request.GetSourceAddressAndPort());
  return false;
}


bool PSTUNServer::OnBindingRequest(const PSTUNMessage & request, PSTUNServer::SocketInfo & socketInfo)
{
  PSTUNMessage response;
  PUDPSocket * replySocket = socketInfo.m_socket;

  PTRACE(2, "STUNSRVR\tReceived BINDING request from " << request.GetSourceAddressAndPort());

  // if CHANGE-REQUEST was specified, and we have no alternate address, then refuse the request
  const PSTUNChangeRequest * changeRequest = (PSTUNChangeRequest *)request.FindAttribute(PSTUNAttribute::CHANGE_REQUEST);
  if ((changeRequest != NULL) && 
      (        
         (
          (changeRequest->GetChangeIP() && changeRequest->GetChangePort() && (socketInfo.m_alternateAddressAndPortSocket == NULL))
         ) || (
          (changeRequest->GetChangeIP() && (socketInfo.m_alternateAddressSocket == NULL))
         ) || (
          (changeRequest->GetChangePort() && (socketInfo.m_alternatePortSocket == NULL))
          )
      )
      ) {
    PTRACE(2, "STUNSRVR\tUnable to fulfill CHANGE-REQUEST from " << request.GetSourceAddressAndPort());

    // initialise the response as per RFC 5780, para 6.1
    response.SetType(PSTUNMessage::BindingError, request.GetTransactionID());

    PSTUNErrorCode attr;
    attr.Initialise();
    attr.SetErrorCode(420, "");
    response.AddAttribute(attr);
  }
  else {

    // initialise the response
    response.SetType(PSTUNMessage::BindingResponse, request.GetTransactionID());

    // set the MAPPED_ADDRESS attribute
    {
      PSTUNAddressAttribute attr;
      attr.InitAddrAttr(PSTUNAttribute::MAPPED_ADDRESS);
      attr.SetIPAndPort(request.GetSourceAddressAndPort());
      response.AddAttribute(attr);
    }

    // if RFC 5389, set XOR-MAPPED_ADDRESS, RESPONSE_ORIGIN
    if (request.IsRFC5389()) {

      // set XOR-MAPPED_ADDRESS
      {
        PSTUNAddressAttribute attr;
        attr.InitAddrAttr(PSTUNAttribute::XOR_MAPPED_ADDRESS);
        attr.SetIPAndPort(request.GetSourceAddressAndPort());
        response.AddAttribute(attr);
      }

      // set RESPONSE-ORIGIN
      {
        PSTUNAddressAttribute attr;
        attr.InitAddrAttr(PSTUNAttribute::RESPONSE_ORIGIN);
        attr.SetIPAndPort(socketInfo.m_socketAddress);
        response.AddAttribute(attr);
      }
    }

    // if not RFC 5389, set the SOURCE attribute 
    else {
      // replies always contain SOURCE-ADDRESS 
      PSTUNAddressAttribute attr;
      attr.InitAddrAttr(PSTUNAttribute::SOURCE_ADDRESS);
      attr.SetIPAndPort(socketInfo.m_socketAddress);
      response.AddAttribute(attr);
    }

    // set OTHER-ADDRESS, if we can
    if (socketInfo.m_alternateAddressSocket != 0) {
      PSTUNAddressAttribute attr;
      attr.InitAddrAttr(PSTUNAttribute::OTHER_ADDRESS);
      attr.SetIPAndPort(socketInfo.m_alternateAddressAndPort);
      response.AddAttribute(attr);
    }

    // fulfill CHANGE-REQUEST, if any
    if (changeRequest != NULL) {
      if (changeRequest->GetChangeIP() && changeRequest->GetChangePort()) 
        replySocket = socketInfo.m_alternateAddressAndPortSocket;
      else if (changeRequest->GetChangeIP())
        replySocket = socketInfo.m_alternateAddressSocket;
      else if (changeRequest->GetChangePort()) 
        replySocket = socketInfo.m_alternatePortSocket;
    }

    PTRACE(3, "STUNSRVR\tSending BindingResponse to " << request.GetSourceAddressAndPort());
  }

  WriteTo(response, *replySocket, request.GetSourceAddressAndPort());

  return true;
}


