/*
 * Implementation of RFC 5389 - Session Traversal Utilities for NAT (STUN)
 */

#ifdef __GNUC__
#pragma implementation "pstunsrvr.h"
#endif

#include <ptlib.h>

#if P_STUNSRVR

#define P_FORCE_STATIC_PLUGIN
#include <ptlib/pluginmgr.h>

#include <ptclib/pstunsrvr.h>

#define new PNEW
#define PTraceModule() "STUNSrvr"


//////////////////////////////////////////////////

PSTUNServer::SocketInfo::SocketInfo(PUDPSocket * socket)
  : m_socket(socket)
  , m_alternatePortSocket(NULL)
  , m_alternateAddressSocket(NULL)
  , m_alternateAddressAndPortSocket(NULL)
{
  if (socket != NULL)
    socket->GetLocalAddress(m_socketAddress);
}

//////////////////////////////////////////////////

PSTUNServer::PSTUNServer()
  : m_autoDelete(true)
{
}

bool PSTUNServer::Open(WORD port)
{
  Close();

  PIPSocket::Address ip;

  // get interfaces to bind to
  PIPSocket::InterfaceTable interfaces;
  if (!PIPSocket::GetInterfaceTable(interfaces)) {
    PTRACE(2, "GetInterfaceTable failed");
    return false;
  }

  PINDEX i;
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
    PTRACE(2, "no suitable interfaces found");
    return false;
  }

  // open sockets
  size_t j;
  for (j = 0; j < interfaceAddresses.size(); ++j) {
    PIPSocketAddressAndPort ap(interfaceAddresses[j], port);
    if (!CreateAndAddSocket(interfaceAddresses[j], port)) {
      PTRACE(2, "Cannot open socket on " << ap);
      Close();
      return false;
    }
    PTRACE(2, "Listening on " << ap);
  }

  if (m_sockets.GetSize() == 0) {
    PTRACE(2, "Unable to open any ports on " << ip);
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
        PTRACE(2, "Cannot open primary alternate port socket on " << primaryAddress << ":" << alternatePort);
        return false;
      }
      PTRACE(2, "Listening on " << info->m_socketAddress);
      primaryAlternateSocket = info->m_socket;
    }

    PUDPSocket * secondaryAlternateSocket;
    {
      // primary socket, alternate port
      SocketInfo * info = CreateAndAddSocket(secondaryAddress, alternatePort);
      if (info == NULL) {
        PTRACE(2, "Cannot open secondary alternate port socket on " << secondaryAddress << ":" << alternatePort);
        return false;
      }
      PTRACE(2, "Listening on " << info->m_socketAddress);
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

bool PSTUNServer::Open(PUDPSocket * socket1, PUDPSocket * socket2)
{
  if (socket1 != NULL) {
    m_sockets.Append(socket1);
    PopulateInfo(socket1, PIPSocket::GetInvalidAddress(), 0, NULL, NULL, NULL);
  }
  if (socket2 != NULL) {
    m_sockets.Append(socket1);
    PopulateInfo(socket2, PIPSocket::GetInvalidAddress(), 0, NULL, NULL, NULL);
  }

  return !m_sockets.IsEmpty();
}

void PSTUNServer::PopulateInfo(PUDPSocket * socket, 
                               const PIPSocket::Address & alternateAddress, WORD alternatePort, 
                               PUDPSocket * alternatePortSocket, PUDPSocket * alternateAddressSocket, PUDPSocket * alternateAddressAndPortSocket)
{
  SocketToSocketInfoMap::iterator it = m_socketToSocketInfoMap.find(socket);
  if (it == m_socketToSocketInfoMap.end())
    it = m_socketToSocketInfoMap.insert(SocketToSocketInfoMap::value_type(socket, socket)).first;
  PSTUNServer::SocketInfo & info = it->second;

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
  return &m_socketToSocketInfoMap.insert(SocketToSocketInfoMap::value_type(sock, SocketInfo(sock))).first->second;
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
    for (PINDEX i = 0; i < m_sockets.GetSize(); ++i)
      m_selectList += m_sockets[i];

    switch (PIPSocket::Select(m_selectList)) {
      default:
        return false;
      case PChannel::Timeout:
        return true;
      case PChannel::NoError:
        if (m_selectList.GetSize() == 0)
          return true;
    }
  }

  PSocket::SelectList::iterator selection = m_selectList.begin();
  PUDPSocket * socket = (PUDPSocket *)&(*selection);
  m_selectList.erase(selection);

  if (!message.Read(*socket)) {
    // ignore read errors - they are likely to be connection
    // refused ICMP messages from symmetric NATs
    //PTRACE(2, "Read failed");
    message.SetSize(0);
    return true;
  }  

  SocketToSocketInfoMap::iterator it = m_socketToSocketInfoMap.find(socket);
  if (it == m_socketToSocketInfoMap.end()) {
    PTRACE(2, "Unable to find interface for received request - ignoring");
    return false;
  }
  socketInfo = it->second;

  return true;
}

bool PSTUNServer::Process()
{
  PSTUNMessage message;
  SocketInfo socketInfo;
  return Read(message, socketInfo) && OnReceiveMessage(message, socketInfo);
}

bool PSTUNServer::OnReceiveMessage(const PSTUNMessage & message, const PSTUNServer::SocketInfo & socketInfo)
{
  switch (message.GetType()) {
    case PSTUNMessage::BindingRequest :
      return OnBindingRequest(message, socketInfo);

    default :
      if (message.IsRequest())
        return OnUnknownRequest(message, socketInfo);

      PTRACE(2, "Unexpected response message: " << message);
  }

  return false;
}


bool PSTUNServer::OnUnknownRequest(const PSTUNMessage & PTRACE_PARAM(request), const PSTUNServer::SocketInfo & /*socketInfo*/)
{
  PTRACE(2, "Received unknown request " << hex << request.GetType() << " from " << request.GetSourceAddressAndPort());
  return false;
}


bool PSTUNServer::OnBindingRequest(const PSTUNMessage & request, const PSTUNServer::SocketInfo & socketInfo)
{
  PSTUNMessage response;
  PUDPSocket * replySocket = socketInfo.m_socket;
  const PSTUNChangeRequest * changeRequest;

  if (!m_password.IsEmpty()) {
    PSTUNStringAttribute * userAttr = request.FindAttributeAs<PSTUNStringAttribute>(PSTUNAttribute::USERNAME);
    if (userAttr == NULL) {
      PTRACE(2, "No USERNAME attribute in " << request << " on interface " << socketInfo.m_socketAddress);
      response.SetErrorType(400, request.GetTransactionID());
      goto sendResponse;
    }

    if (userAttr->GetString() != m_userName) {
      PTRACE(2, "Incorrect USERNAME attribute in " << request << " on interface " << socketInfo.m_socketAddress
             << ", got \"" << userAttr->GetString() << "\", expected \"" << m_userName << '"');
      response.SetErrorType(401, request.GetTransactionID());
      goto sendResponse;
    }

    if (!request.CheckMessageIntegrity(m_password)) {
      PTRACE(2, "Integrity check failed for " << request << " on interface " << socketInfo.m_socketAddress);
      response.SetErrorType(401, request.GetTransactionID());
      goto sendResponse;
    }
  }

  PTRACE(m_throttleReceivedPacket, "Received " << request << " on " << socketInfo.m_socketAddress << m_throttleReceivedPacket);

  // if CHANGE-REQUEST was specified, and we have no alternate address, then refuse the request
  changeRequest = (PSTUNChangeRequest *)request.FindAttribute(PSTUNAttribute::CHANGE_REQUEST);
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
    PTRACE(2, "Unable to fulfill CHANGE-REQUEST from " << request.GetSourceAddressAndPort());

    response.SetErrorType(420, request.GetTransactionID());
    goto sendResponse;
  }

  // initialise the response
  response.SetType(PSTUNMessage::BindingResponse, request.GetTransactionID());

  // if RFC 5389, set XOR-MAPPED_ADDRESS, RESPONSE_ORIGIN
  if (request.IsRFC5389()) {
    response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS, request.GetSourceAddressAndPort()));
    //response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::RESPONSE_ORIGIN, socketInfo.m_socketAddress));

    // set OTHER-ADDRESS, if we can
    if (socketInfo.m_alternateAddressSocket != 0)
      response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::OTHER_ADDRESS, socketInfo.m_alternateAddressAndPort));
  }

  // if not RFC 5389, set the SOURCE attribute 
  else {
    // replies always contain SOURCE-ADDRESS 
    response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::SOURCE_ADDRESS, socketInfo.m_socketAddress));

    // set the MAPPED_ADDRESS attribute
    response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::MAPPED_ADDRESS, request.GetSourceAddressAndPort()));

    // set CHANGED-ADDRESS, if we can
    if (socketInfo.m_alternateAddressSocket != 0)
      response.AddAttribute(PSTUNAddressAttribute(PSTUNAttribute::CHANGED_ADDRESS, socketInfo.m_alternateAddressAndPort));
  }

  // fulfill CHANGE-REQUEST, if any
  if (changeRequest != NULL) {
    if (changeRequest->GetChangeIP() && changeRequest->GetChangePort()) {
      PTRACE(3, "Changed source to alternate address and port " << socketInfo.m_alternateAddressAndPort);
      replySocket = socketInfo.m_alternateAddressAndPortSocket;
    }
    else if (changeRequest->GetChangeIP()) {
      PTRACE(3, "Changed source to alternate address " << socketInfo.m_alternateAddressAndPort.GetAddress());
      replySocket = socketInfo.m_alternateAddressSocket;
    }
    else if (changeRequest->GetChangePort())  {
      PTRACE(3, "Changed source to alternate port " << socketInfo.m_alternateAddressAndPort.GetPort());
      replySocket = socketInfo.m_alternatePortSocket;
    }
  }

  OnBindingResponse(request, response);

sendResponse:
  response.AddMessageIntegrity(m_password); // Must be last things before sending
  response.AddFingerprint();
  response.Write(*replySocket, request.GetSourceAddressAndPort());

  return true;
}


void PSTUNServer::OnBindingResponse(const PSTUNMessage &, PSTUNMessage &)
{
}


#endif // P_STUNSRVR
