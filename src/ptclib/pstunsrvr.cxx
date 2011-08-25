/*
 * Implementation of RFC 5389 - Session Traversal Utilities for NAT (STUN)
 */

#ifdef __GNUC__
#pragma implementation "pstunsrvr.h"
#endif

#include <ptlib.h>

#include <ptclib/pstunsrvr.h>

#if P_STUNSRVR

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
    PTRACE(2, "PSTUNSRVR\tGetInterfaceTable failed");
    return false;
  }

  int i;
  std::vector<PIPSocket::Address> interfaceAddresses;

  // find interfaces
  for (i = 0; i < interfaces.GetSize(); i++) {
    if (!interfaces[i].GetAddress().IsLoopback()
#if P_HAS_IPV6
        && !interfaces[i].GetAddress().IsLinkLocal()
#endif
#ifndef _DEBUG
        && !interfaces[i].GetAddress().IsRFC1918()
#endif
        ) {
      interfaceAddresses.push_back(interfaces[i].GetAddress());
    }
  }

  if (interfaceAddresses.size() == 0) {
    PTRACE(2, "PSTUNSRVR\tno suitable interfaces found");
    return false;
  }

  // open sockets
  size_t j;
  for (j = 0; j < interfaceAddresses.size(); ++j) {
    PIPSocketAddressAndPort addr(interfaceAddresses[j], port);
    if (!CreateAndAddSocket(interfaceAddresses[j], port)) {
      PTRACE(2, "PSTUNSRVR\tCannot open socket on " << addr);
      Close();
      return false;
    }
    PTRACE(2, "PSTUNSRVR\tListening on " << addr);
  }

  if (m_sockets.GetSize() == 0) {
    PTRACE(2, "PSTUNSRVR\tUnable to open any ports" << addr);
    return false;
  }

  // open and populate alternates
  if (m_sockets.GetSize() > 1) {
    SocketToSocketInfoMap::iterator r = m_socketToSocketInfoMap.begin();

    // primary socket
    PUDPSocket * primarySocket        = r->second.m_socket;
    PIPSocket::Address primaryAddress = r->second.m_socketAddress.GetAddress();
    WORD primaryPort                  = r->second.m_socketAddress.GetPort();
    WORD alternatePort                = primaryPort + 1;
    ++r;
    PUDPSocket * secondarySocket        = r->second.m_socket;
    PIPSocket::Address secondaryAddress =  r->second.m_socketAddress.GetAddress();

    PUDPSocket * primaryAlternateSocket;
    {
      // primary socket, alternate port
      SocketInfo * info = CreateAndAddSocket(primaryAddress, alternatePort);
      if (info == NULL) {
        PTRACE(2, "PSTUNSRVR\tCannot open primary alternate port socket on " << primaryAddress << ":" << alternatePort);
        return false;
      }
      PTRACE(2, "PSTUNSRVR\tListening on " << info->m_socketAddress);
      primaryAlternateSocket = info->m_socket;
    }

    PUDPSocket * secondaryAlternateSocket;
    {
      // primary socket, alternate port
      SocketInfo * info = CreateAndAddSocket(secondaryAddress, alternatePort);
      if (info == NULL) {
        PTRACE(2, "PSTUNSRVR\tCannot open secondary alternate port socket on " << secondaryAddress << ":" << alternatePort);
        return false;
      }
      PTRACE(2, "PSTUNSRVR\tListening on " << info->m_socketAddress);
      secondaryAlternateSocket = info->m_socket;
    }

    PopulateInfo(primarySocket,            secondaryAddress, alternatePort, primaryAlternateSocket,   secondarySocket,          secondaryAlternateSocket);
    PopulateInfo(primaryAlternateSocket,   secondaryAddress, primaryPort,   primarySocket,            secondaryAlternateSocket, secondarySocket);

    PopulateInfo(secondarySocket,          primaryAddress,   alternatePort, secondaryAlternateSocket, primarySocket,            primaryAlternateSocket);
    PopulateInfo(secondaryAlternateSocket, primaryAddress,   primaryPort,   secondarySocket,          primaryAlternateSocket,   primarySocket);
  }

  m_selectList.DisallowDeleteObjects();

  return true;
}

void PSTUNServer::PopulateInfo(PUDPSocket * socket, 
                               const PIPSocket::Address & alternateAddress, WORD alternatePort, 
                               PUDPSocket * alternatePortSocket, PUDPSocket * alternateAddressSocket, PUDPSocket * alternateAddressAndPortSocket)
{
  SocketToSocketInfoMap::iterator r = m_socketToSocketInfoMap.find(socket);
  if (r == m_socketToSocketInfoMap.end()) {
    PTRACE(2, "PSTUNSRVR\tCould not find socket info for socket ");
    return;
  }
  PSTUNServer::SocketInfo & info = r->second;

  info.m_alternateAddressAndPort       = PIPSocketAddressAndPort(alternateAddress, alternatePort);

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

  if (!sock->IsOpen()) {
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
  m_sockets.AllowDeleteObjects(m_autoDelete);
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
    for (int i = 0; i < m_sockets.GetSize(); ++i)
      m_selectList += m_sockets[i];

    int r = PIPSocket::Select(m_selectList);
    if (r == PChannel::Timeout)
      return true;
    if (r != PChannel::NoError)
      return false;
    if (m_selectList.GetSize() == 0)
      return true;
  }

  PSocket::SelectList::iterator r = m_selectList.begin();
  PUDPSocket * socket = (PUDPSocket *)&(*r);
  m_selectList.erase(r);

  if (!message.Read(*socket)) {
    // ignore read errors - they are likely to be connection
    // refused ICMP messages from symmetric NATs
    //PTRACE(2, "STUNSRVR\tRead failed");
    message.SetSize(0);
    return true;
  }  

  {
    SocketToSocketInfoMap::iterator r = m_socketToSocketInfoMap.find(socket);
    if (r == m_socketToSocketInfoMap.end()) {
      PTRACE(2, "STUNSRVR\tUnable to find interface for received request - ignoring");
      return false;
    }
    socketInfo = r->second;
  }

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

bool PSTUNServer::OnUnknownRequest(const PSTUNMessage & PTRACE_PARAM(request), PSTUNServer::SocketInfo & /*socketInfo*/)
{
  PTRACE(2, "STUNSRVR\tReceived unknown request " << hex << request.GetType() << " from " << request.GetSourceAddressAndPort());
  return false;
}


bool PSTUNServer::OnBindingRequest(const PSTUNMessage & request, PSTUNServer::SocketInfo & socketInfo)
{
  PSTUNMessage response;
  PUDPSocket * replySocket = socketInfo.m_socket;

  PTRACE(2, "STUNSRVR\tReceived " << (request.IsRFC5389() ? "RFC5389 " : "") << "BINDING request from " << request.GetSourceAddressAndPort() << " on " << socketInfo.m_socketAddress);

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

      // set OTHER-ADDRESS, if we can
      if (socketInfo.m_alternateAddressSocket != 0) {
        PSTUNAddressAttribute attr;
        attr.InitAddrAttr(PSTUNAttribute::OTHER_ADDRESS);
        attr.SetIPAndPort(socketInfo.m_alternateAddressAndPort);
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

      // set CHANGED-ADDRESS, if we can
      if (socketInfo.m_alternateAddressSocket != 0) {
        PSTUNAddressAttribute attr;
        attr.InitAddrAttr(PSTUNAttribute::CHANGED_ADDRESS);
        attr.SetIPAndPort(socketInfo.m_alternateAddressAndPort);
        response.AddAttribute(attr);
      }
    }

    // fulfill CHANGE-REQUEST, if any
    if (changeRequest != NULL) {
      if (changeRequest->GetChangeIP() && changeRequest->GetChangePort()) {
        PTRACE(3, "STUNSRVR\tChanged source to alternate address and port " << socketInfo.m_alternateAddressAndPort);
        replySocket = socketInfo.m_alternateAddressAndPortSocket;
      }
      else if (changeRequest->GetChangeIP()) {
        PTRACE(3, "STUNSRVR\tChanged source to alternate address " << socketInfo.m_alternateAddressAndPort.GetAddress());
        replySocket = socketInfo.m_alternateAddressSocket;
      }
      else if (changeRequest->GetChangePort())  {
        PTRACE(3, "STUNSRVR\tChanged source to alternate port " << socketInfo.m_alternateAddressAndPort.GetPort());
        replySocket = socketInfo.m_alternatePortSocket;
      }
    }

    PTRACE(3, "STUNSRVR\tSending BindingResponse to " << request.GetSourceAddressAndPort());
  }

  WriteTo(response, *replySocket, request.GetSourceAddressAndPort());

  return true;
}


#endif // P_STUNSRVR
