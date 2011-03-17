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

// from RFC 5389
#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

PSTUNServer::PSTUNServer()
  : m_stunSocket1(NULL)
  , m_stunSocket2(NULL)
  , m_autoDelete(false)
{
}

bool PSTUNServer::Open(WORD port)
{
  return Open(port, port+1);
}


bool PSTUNServer::Open(WORD port1, WORD port2)
{
  Close();

  PIPSocket::Address addr1;
  PIPSocket::Address addr2;

  // get interfaces to bind to
  PIPSocket::InterfaceTable interfaces;
  if (!PIPSocket::GetInterfaceTable(interfaces)) {
    PTRACE(2, "PSTUNSRVR\nGetInterfaceTable failed");
  }
  else {
    PINDEX i;

    // find first interface
    for (i = 0; i < interfaces.GetSize(); i++) {
      if (!interfaces[i].GetAddress().IsLinkLocal()
          && !interfaces[i].GetAddress().IsLoopback()
#ifndef _DEBUG
          && !interfaces[i].GetAddress().IsRFC1918()
#endif
          ) {
        addr1 = interfaces[i].GetAddress();
        PTRACE(2, "PSTUNSRVR\nFound " << addr1 << " as STUN address 1");
        break;
      }
    }

    // find second interface
    for (++i;i < interfaces.GetSize(); i++) {
      if (!interfaces[i].GetAddress().IsLinkLocal()
          && !interfaces[i].GetAddress().IsLoopback()
#ifndef _DEBUG
          && !interfaces[i].GetAddress().IsRFC1918()
#endif
          ) {
        addr2 = interfaces[i].GetAddress();
        PTRACE(2, "PSTUNSRVR\nFound " << addr2 << " as STUN address 2");
        break;
      }
    }
  }

  m_interfaceAddress1 = PIPSocketAddressAndPort(addr1, port1);
  m_interfaceAddress2 = PIPSocketAddressAndPort(addr2, port2);

  // if could not determine interfaces, use INADDR_ANY
  if (!addr1.IsValid()) {
    PTRACE(2, "PSTUNSRVR\nUsing one socket with INADDR_ANY");
    m_stunSocket1 = new PUDPSocket(port1);
    if (!m_stunSocket1->IsOpen() || !m_stunSocket1->Listen()) {
      PTRACE(2, "PSTUNSRVR\nCannot open socket on INADDR_ANY on port " << (int)port1);
      delete m_stunSocket1;
      m_stunSocket1 = NULL;
      return false;
    }
    m_stunSocket2 = NULL;
  }

  // open designated interfaces
  else {
    m_stunSocket1 = new PUDPSocket();
    if (!m_stunSocket1->Listen(addr1, 5, port1)) {
      PTRACE(2, "PSTUNSRVR\nCannot open socket on " << addr1 << " on port " << (int)port1);
      delete m_stunSocket1;
      m_stunSocket1 = NULL;
      return false;
    }
    if (addr2.IsValid()) {
      m_stunSocket2 = new PUDPSocket();
      if (!m_stunSocket2->Listen(addr2, 5, port2)) {
        PTRACE(2, "PSTUNSRVR\nCannot open socket on " << addr2 << " on port " << (int)port2);
        delete m_stunSocket2;
        m_stunSocket2 = NULL;
        return false;
      }
    }
  }

  m_autoDelete = true;
  return true;
}

bool PSTUNServer::Open(PUDPSocket * socket, bool autoDelete)
{
  Close();

  m_stunSocket1 = socket;
  m_stunSocket2 = NULL;

  m_autoDelete = autoDelete;

  return false;
}

bool PSTUNServer::IsOpen() const 
{ 
  return ((m_stunSocket1 != NULL) 
          && m_stunSocket1->IsOpen())
          && ((m_stunSocket2 == NULL) || m_stunSocket2->IsOpen()); }


bool PSTUNServer::Close()
{
  if (m_autoDelete) {
    delete m_stunSocket1;
    delete m_stunSocket2;
  }

  m_stunSocket1 = NULL;
  m_stunSocket2 = NULL;

  return true;
}

bool PSTUNServer::Read(PSTUNMessage & message, PIPSocketAddressAndPort & receivedInterface)
{
  if (!IsOpen())
    return false;

  if (m_stunSocket2 == NULL)
    return message.Read(*m_stunSocket1);

  int r = PIPSocket::Select(*m_stunSocket1, *m_stunSocket2);
  if (r == -1) {
    receivedInterface = m_interfaceAddress1;
    return message.Read(*m_stunSocket1);
  }
  else if (r == -2) {
    receivedInterface = m_interfaceAddress2; 
    return message.Read(*m_stunSocket2);
  }
  else
    return false;
}

bool PSTUNServer::Write(const PSTUNMessage & message, const PIPSocketAddressAndPort & destination, const PIPSocketAddressAndPort & iface)
{
  if ((m_stunSocket2 == NULL) || (m_interfaceAddress1 == iface)) {
    m_stunSocket1->SetSendAddress(destination);
    return message.Write(*m_stunSocket1);
  }
  m_stunSocket2->SetSendAddress(destination);
  return message.Write(*m_stunSocket2);
}

bool PSTUNServer::Process(PSTUNMessage & response, const PSTUNMessage & message, const PIPSocketAddressAndPort & receivedInterface, const PIPSocketAddressAndPort * alternateInterface)
{
  int type = message.GetType();

  // decode requests
  if (IS_REQUEST(type)) {
    if (type == PSTUNMessage::BindingRequest)
      return OnBindingRequest(response, message, receivedInterface, alternateInterface);
    else
      return OnUnknownRequest(response, message, receivedInterface);
  }

  return false;
}

bool PSTUNServer::OnUnknownRequest(PSTUNMessage & /*response*/, const PSTUNMessage & request, const PIPSocketAddressAndPort & /*receivedInterface*/)
{
  PTRACE(2, "STUNSRVR\tReceived unknown request " << hex << request.GetType() << " from " << request.GetSourceAddressAndPort());
  return false;
}


bool PSTUNServer::OnBindingRequest(PSTUNMessage & response, const PSTUNMessage & request, const PIPSocketAddressAndPort & receivedInterface, const PIPSocketAddressAndPort * /*alternateInterface*/)
{
  PTRACE(2, "STUNSRVR\tReceived BINDING request from " << request.GetSourceAddressAndPort());

  if (request.IsRFC5389()) {
    // RFC5389 messages MUST contain a MESSAGE-INTEGRITY attribute (RFC 5389, 10.2.2)
    const PSTUNAttribute * messageIntegrity = request.FindAttribute(PSTUNAttribute::MESSAGE_INTEGRITY);
    if (!messageIntegrity) {
      PTRACE(2, "STUNSRVR\tReceived RFC 5389 BINDING request without MESSAGE_INTEGRITY");
      // TODO - insert correct response here
      return false;
    }

    // set the MAPPED_ADDRESS attribute
    //{
    //  PSTUNMappedAddress attr;
    //  attr.Initialise();
    //  attr.SetIPAndPort(request.m_sourceAddressAndPort);
    //  response.AddAttribute(attr);
    //}
    return false;
  }

  // if CHANGE_REQUEST was specified, and we have no alternate address, then refuse the request
  const PSTUNChangeRequest * changeRequest = (PSTUNChangeRequest *)request.FindAttribute(PSTUNAttribute::CHANGE_REQUEST);
  if (changeRequest != NULL && (changeRequest->GetChangeIP() || changeRequest->GetChangePort())) {
    // initialise the response
    response.SetType(PSTUNMessage::BindingError, request.GetTransactionID());

    PSTUNErrorCode attr;
    attr.Initialise();
    attr.SetErrorCode(600, "Only one interface available");
    response.AddAttribute(attr);

    return true;
  }

  // initialise the response
  response.SetType(PSTUNMessage::BindingResponse, request.GetTransactionID());

  // set the MAPPED_ADDRESS attribute
  {
    PSTUNMappedAddress attr;
    attr.Initialise();
    attr.SetIPAndPort(request.GetSourceAddressAndPort());
    response.AddAttribute(attr);
  }

  // replies always contain SOURCE-ADDRESS 
  PSTUNSourceAddress attr;
  attr.Initialise();
  attr.SetIPAndPort(receivedInterface);
  response.AddAttribute(attr);

  PTRACE(3, "STUNSRVR\tSending BindingResponse to " << request.GetSourceAddressAndPort());

  return true;
}


