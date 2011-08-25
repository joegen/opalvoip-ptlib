/*
 * pstun.cxx
 *
 * STUN Client
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "pstun.h"
#endif

#include <ptlib.h>

#include <ptclib/pstun.h>

#if P_STUN

#include <ptclib/random.h>
#include <ptclib/cypher.h>

#define new PNEW

#define ENABLE_TURN_FOR_ALL    1


// Sample server is at larry.gloo.net

#define DEFAULT_REPLY_TIMEOUT 800
#define DEFAULT_POLL_RETRIES  3
#define DEFAULT_NUM_SOCKETS_FOR_PAIRING 4

#define RFC5389_MAGIC_COOKIE  0x2112A442


#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

////////////////////////////////////////////////////////////////////////////////

PSTUN::PSTUN()
  : m_natType(PNatMethod::UnknownNat)
  , m_pollRetries(DEFAULT_POLL_RETRIES)
  , m_timeAddressObtained(0)
  , replyTimeout(DEFAULT_REPLY_TIMEOUT)
{
}

PNatMethod::NatTypes PSTUN::DoRFC3489Discovery(
  PSTUNUDPSocket * socket, 
  const PIPSocketAddressAndPort & serverAddress,
  PIPSocketAddressAndPort & baseAddressAndPort, 
  PIPSocketAddressAndPort & externalAddressAndPort
)
{  
  socket->SetReadTimeout(replyTimeout);

  socket->GetLocalAddress(baseAddressAndPort);
  socket->PUDPSocket::InternalSetSendAddress(serverAddress);

  // RFC3489 discovery

  /* test I - the client sends a STUN Binding Request to a server, without
     any flags set in the CHANGE-REQUEST attribute, and without the
     RESPONSE-ADDRESS attribute. This causes the server to send the response
     back to the address and port that the request came from. */
  PSTUNMessage requestI(PSTUNMessage::BindingRequest);
  //requestI.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI;
  if (!responseI.Poll(*socket, requestI, m_pollRetries)) {
    PTRACE(2, "STUN\tSTUN server " << serverAddress << " did not respond.");
    return m_natType = PNatMethod::UnknownNat;
  }

  return FinishRFC3489Discovery(responseI, socket, externalAddressAndPort);
}

PNatMethod::NatTypes PSTUN::FinishRFC3489Discovery(
  PSTUNMessage & responseI,
  PSTUNUDPSocket * socket, 
  PIPSocketAddressAndPort & externalAddressAndPort
)
{
  // check if server returned "420 Unknown Attribute" - that probably means it cannot do CHANGE_REQUEST even with no changes
  bool canDoChangeRequest = true;

  PSTUNErrorCode * errorAttribute = (PSTUNErrorCode *)responseI.FindAttribute(PSTUNAttribute::ERROR_CODE);
  if (errorAttribute != NULL) {
    bool ok = false;
    if (errorAttribute->GetErrorCode() == 420) {
      // try again without CHANGE request
      PSTUNMessage request(PSTUNMessage::BindingRequest);
      ok = responseI.Poll(*socket, request, m_pollRetries);
      if (ok) { 
        errorAttribute = (PSTUNErrorCode *)responseI.FindAttribute(PSTUNAttribute::ERROR_CODE);
        ok = errorAttribute == NULL;
        canDoChangeRequest = false;
      }
    }
    if (!ok) {
      PTRACE(2, "STUN\tSTUN server " << socket->GetSendAddress() << " returned unexpected error " << errorAttribute->GetErrorCode() << ", reason = '" << errorAttribute->GetReason() << "'");
      return m_natType = PNatMethod::BlockedNat;
    }
  }

  PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << m_serverAddress);
      return m_natType = PNatMethod::UnknownNat; // Protocol error
    }
  }

  mappedAddress->GetIPAndPort(externalAddressAndPort);
  m_timeAddressObtained.SetCurrentTime();

  bool notNAT = (socket->GetPort() == externalAddressAndPort.GetPort()) && PIPSocket::IsLocalHost(externalAddressAndPort.GetAddress());

  // can only guess based on a single sample
  if (!canDoChangeRequest) {
    m_natType = notNAT ? PNatMethod::OpenNat : PNatMethod::SymmetricNat;
    PTRACE(3, "STUN\tSTUN server has only one address - best guess is that NAT is " << PNatMethod::GetNatTypeString(m_natType));
    return m_natType;
  }

  PTRACE(3, "STUN\tTest I response received - sending test II (change port and address)");

  /* Test II - the client sends a Binding Request with both the "change IP"
     and "change port" flags from the CHANGE-REQUEST attribute set. */
  PSTUNMessage requestII(PSTUNMessage::BindingRequest);
  requestII.AddAttribute(PSTUNChangeRequest(true, true));
  PSTUNMessage responseII;
  bool testII = responseII.Poll(*socket, requestII, m_pollRetries);

  PTRACE(3, "STUN\tTest II response " << (testII ? "" : "not ") << "received");

  if (notNAT) {
    m_natType = (testII ? PNatMethod::OpenNat : PNatMethod::SymmetricFirewall);
    // Is not NAT or symmetric firewall
    PTRACE(2, "STUN\tTest I and II indicate nat is " << PNatMethod::GetNatTypeString(m_natType));
    return m_natType;
  }

  if (testII)
    return m_natType = PNatMethod::ConeNat;

  PSTUNAddressAttribute * changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::CHANGED_ADDRESS);
  if (changedAddress == NULL) {
    changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::OTHER_ADDRESS);
    if (changedAddress == NULL) {
      PTRACE(3, "STUN\tTest II response indicates no alternate address in use - testing finished");
      return m_natType = PNatMethod::UnknownNat; // Protocol error
    }
  }

  PTRACE(3, "STUN\tSending test I to alternate server");

  // Send test I to another server, to see if restricted or symmetric
  PIPSocket::Address secondaryServer = changedAddress->GetIP();
  WORD secondaryPort = changedAddress->GetPort();
  socket->PUDPSocket::InternalSetSendAddress(PIPSocketAddressAndPort(secondaryServer, secondaryPort));
  PSTUNMessage requestI2(PSTUNMessage::BindingRequest);
  requestI2.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI2;
  if (!responseI2.Poll(*socket, requestI2, m_pollRetries)) {
    PTRACE(3, "STUN\tPoll of secondary server " << secondaryServer << ':' << secondaryPort
           << " failed, NAT partially blocked by firewall rules.");
    return m_natType = PNatMethod::PartialBlockedNat;
  }

  mappedAddress = (PSTUNAddressAttribute *)responseI2.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)responseI2.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << m_serverAddress);
      return PNatMethod::UnknownNat; // Protocol error
    }
  }

  {
    PIPSocketAddressAndPort ipAndPort;
    mappedAddress->GetIPAndPort(ipAndPort);
    if (ipAndPort != externalAddressAndPort)
      return m_natType = PNatMethod::SymmetricNat;
  }

  socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
  PSTUNMessage requestIII(PSTUNMessage::BindingRequest);
  requestIII.SetAttribute(PSTUNChangeRequest(false, true));
  PSTUNMessage responseIII;

  return m_natType = (responseIII.Poll(*socket, requestIII, m_pollRetries) ? PNatMethod::RestrictedNat : PNatMethod::PortRestrictedNat);
}

void PSTUN::AppendMessageIntegrity(PSTUNMessage & message)
{
  message.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::USERNAME, m_userName));
  message.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::REALM,    m_realm));
  message.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::NONCE,    m_nonce));
  message.InsertMessageIntegrity(m_credentialsHash.GetPointer(), m_credentialsHash.GetSize());    
}

int PSTUN::MakeAuthenticatedRequest(PSTUNUDPSocket * socket, PSTUNMessage & request, PSTUNMessage & response)
{
  socket->SetReadTimeout(replyTimeout);

  PSTUNErrorCode * errorAttribute;

  std::set<std::string> triedServers;
  WORD unauthenticatedLength = ((PSTUNMessageHeader *)request.GetPointer())->msgLength;

  for (;;) {
    // reset message length
    ((PSTUNMessageHeader *)request.GetPointer())->msgLength = unauthenticatedLength;

    // if we have a nonce, apply it
    if (!m_nonce.IsEmpty())
      AppendMessageIntegrity(request);

    // send request, 
    if (!response.Poll(*socket, request, m_pollRetries)) {
      PTRACE(2, "STUN\tServer " << m_serverAddress << " did not respond.");
      return -1;
    }

    // if succeeded, move on
    if (IS_SUCCESS_RESP(response.GetType()))
      break;

    // if not an error, no idea what it is
    if (!IS_ERR_RESP(response.GetType())) {
      PTRACE(2, "STUN\tServer " << m_serverAddress << " responded to allocate request with unexpected response " << hex << response.GetType());
      return -1;
    }

    // get error attribute
    errorAttribute = response.FindAttributeOfType<PSTUNErrorCode>(PSTUNAttribute::ERROR_CODE);
    if (errorAttribute == NULL) {
      PTRACE(2, "STUN\tServer " << m_serverAddress << " refused allocation request without error code");
      return -1;
    }

    // check code
    int code = errorAttribute->GetErrorCode();

    // 300 = try alternate server
    if (code == 300) {
      PSTUNAddressAttribute * alternate = response.FindAttributeOfType<PSTUNAddressAttribute>(PSTUNAttribute::ALTERNATE_SERVER);
      if (alternate == NULL) {
        PTRACE(2, "STUN\tServer " << m_serverAddress << " redirect did not specify address");
        return -1;
      }

      // add old server to list of servers we have tried
      PString str(m_serverAddress.AsString());
      triedServers.insert(str);

      // get new address and check for loop
      alternate->GetIPAndPort(m_serverAddress);
      if (triedServers.find(m_serverAddress.AsString()) != triedServers.end()) {
        PTRACE(2, "STUN\tServer redirect to " << m_serverAddress << " causes loop");
        return -1;
      }

      PTRACE(2, "STUN\tServer redirected to " << m_serverAddress);
      socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
      m_nonce   = PString::Empty();
    }

    // 401 = authentication required
    // 438 = stale nonce
    else if ((code == 401) || (code == 438)) {

      if ((code == 401) && !m_nonce.IsEmpty()) {
        PTRACE(2, "STUN\tServer refused authentication request with error " << code << " - " << errorAttribute->GetReason());
        return code;
      }

      PSTUNStringAttribute * realmAttr      = (PSTUNStringAttribute *)response.FindAttribute(PSTUNAttribute::REALM);
      PSTUNStringAttribute * nonceAttr      = (PSTUNStringAttribute *)response.FindAttribute(PSTUNAttribute::NONCE);
      if ((realmAttr == NULL) || (nonceAttr == NULL)) {
        PTRACE(2, "STUN\tServer refused unauthenticated request with insufficient information - unable to proceed");
        return -1;
      }

      // realms must match
      if (realmAttr->GetString() != m_realm) {
        PTRACE(2, "STUN\tServer returned unknown realm '" << realmAttr->GetString() << "'");
        return -1;
      }

      // save the nonce
      m_nonce = nonceAttr->GetString();
      PTRACE(2, "STUN\tServer requested authentication");
    }

    else {
      PTRACE(2, "STUN\tServer refused request with code " << code << " - " << errorAttribute->GetReason());
      return code;
    }
  }

  // integrity in response must be valid, if we have a username
  if (!m_userName.IsEmpty()) {
    if (response.FindAttribute(PSTUNAttribute::MESSAGE_INTEGRITY) == NULL) {
      PTRACE(2, "STUN\tIgnoring unauthenticated response to authenticated request");
      return -1;
    }
    if (!response.CheckMessageIntegrity(m_credentialsHash.GetPointer(), m_credentialsHash.GetSize())) {
      PTRACE(2, "STUN\tServer response failed message integrity check");
      return -1;
    }
  }

  return 0;
}

void PSTUN::SetCredentials(const PString & username, const PString & password, const PString & realm)
{
  m_userName = username;
  m_realm    = realm;

  if (username.IsEmpty()) 
    m_credentialsHash.SetSize(0);
  else {
    PMessageDigest5::Result credentialsHash;
    PMessageDigest5::Encode(username + ":" + realm + ":" + password, credentialsHash);
    m_credentialsHash.SetSize(credentialsHash.GetSize());
    memcpy(m_credentialsHash.GetPointer(), credentialsHash.GetPointer(), credentialsHash.GetSize());
  }
}


bool PSTUN::GetFromBindingResponse(const PSTUNMessage & response, PIPSocketAddressAndPort & externalAddress)
{
  // check for mapped address attribute
  PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL)
    mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << m_serverAddress);
    return false;
  }

  // set information
  mappedAddress->GetIPAndPort(externalAddress);

  return true;
}

///////////////////////////////////////////////////////////////////////

PSTUNMessage::PSTUNMessage()
{ }
  
PSTUNMessage::PSTUNMessage(MsgType newType, const BYTE * id)
  : PBYTEArray(sizeof(PSTUNMessageHeader))
{
  SetType(newType, id);
}

void PSTUNMessage::SetType(MsgType newType, const BYTE * id)
{
  SetMinSize(sizeof(PSTUNMessageHeader));
  PSTUNMessageHeader * hdr = (PSTUNMessageHeader *)theArray;
  hdr->msgType = (WORD)newType;

  if (id != NULL)
    memcpy(hdr->transactionId, id, sizeof(hdr->transactionId));
  else  {
    // add magic number for RFC 5389 
    PUInt32b * n = (PUInt32b *)&(hdr->transactionId);
    *n = RFC5389_MAGIC_COOKIE;

    for (PINDEX i = 4; i < ((PINDEX)sizeof(hdr->transactionId)); i++)
       hdr->transactionId[i] = id != NULL ? id[i] : (BYTE)PRandom::Number();
  }
}

PSTUNMessage::MsgType PSTUNMessage::GetType() const
{
  PSTUNMessageHeader * hdr = (PSTUNMessageHeader *)theArray;
  return (PSTUNMessage::MsgType)(int)hdr->msgType;
}

const BYTE * PSTUNMessage::GetTransactionID() const
{
  PSTUNMessageHeader * hdr = (PSTUNMessageHeader *)theArray;
  return (const BYTE *)hdr->transactionId;
}

static int CalcPaddedAttributeLength(int len)
{
  return 4 * ((sizeof(PSTUNAttribute) + len + 3) / 4);
}

PSTUNAttribute * PSTUNMessage::GetFirstAttribute() const
{ 
  if ((theArray == NULL) || GetSize() < (int)sizeof(PSTUNMessageHeader))
    return NULL;

  int length = ((PSTUNMessageHeader *)theArray)->msgLength;

  PSTUNAttribute * attr = (PSTUNAttribute *)(theArray+sizeof(PSTUNMessageHeader)); 
  PSTUNAttribute * ptr  = attr;

  if ((CalcPaddedAttributeLength(attr->length) > GetSize()))
    return NULL;

  while (ptr && 
         ((BYTE*)ptr < (BYTE*)(theArray+GetSize())) && 
         (length >= CalcPaddedAttributeLength(ptr->length))) {
    length -= CalcPaddedAttributeLength(ptr->length);
    ptr = ptr->GetNext();
  }

  if (length != 0)
    return NULL;

  return attr; 
}

PSTUNAttribute * PSTUNAttribute::GetNext() const
{ 
  return (PSTUNAttribute *)(((const BYTE *)this)+CalcPaddedAttributeLength(length)); 
}


bool PSTUNMessage::Validate()
{
  PSTUNMessageHeader * header = (PSTUNMessageHeader *)theArray;

  // sanity check the length
  if ((theArray == NULL) || (GetSize() < (int) sizeof(PSTUNMessageHeader)))
    return false;

  int length = header->msgLength;

  if (GetSize() < ((int)sizeof(PSTUNMessageHeader) + length))
    return false;

  // do quick checks for RFC5389: magic cookie and top two bits of type must be 00
  PUInt32b * p = (PUInt32b *)&(header->transactionId);
  m_isRFC5389 = *p == RFC5389_MAGIC_COOKIE;
  if (m_isRFC5389 && ((header->msgType & 0x00c0) != 0x00)) {
    PTRACE(2, "STUN\tPacket received with magic cookie, but type bits are incorrect.");
    return false;
  }

  // check attributes
  PSTUNAttribute * attrib = GetFirstAttribute();
  while (attrib && length > 0) {
    length -= CalcPaddedAttributeLength(attrib->length);
    attrib = attrib->GetNext();
  }

  if (length != 0) {
    PTRACE(2, "STUN\tInvalid packet received, incorrect attribute length.");
    return false;
  }

  return true;
}


bool PSTUNMessage::Validate(const PSTUNMessage & request)
{
  if (!Validate())
    return false;

  if (memcmp(request->transactionId, (*this)->transactionId, sizeof(request->transactionId)) != 0) {
    PTRACE(2, "STUN\tInvalid reply packet received, transaction ID does not match.");
    return false;
  }

  return true;
}

PSTUNAttribute * PSTUNMessage::AddAttribute(const PSTUNAttribute & attribute)
{
  if (theArray == NULL)
    return NULL;

  int length       = sizeof(PSTUNAttribute) + attribute.length;
  int paddedLength = CalcPaddedAttributeLength(attribute.length); 

  int oldLength = ((PSTUNMessageHeader *)theArray)->msgLength;
  int newLength = oldLength + paddedLength;
  ((PSTUNMessageHeader *)theArray)->msgLength = (WORD)newLength;

  // theArray pointer may be invalidated by next statement
  SetMinSize(sizeof(PSTUNMessageHeader) + newLength);

  PSTUNAttribute * newAttr = (PSTUNAttribute *)(theArray + sizeof(PSTUNMessageHeader) + oldLength);
  memcpy(newAttr, &attribute, length);

  return newAttr;
}

PSTUNAttribute * PSTUNMessage::SetAttribute(const PSTUNAttribute & attribute)
{
  if (theArray == NULL)
    return NULL;

  int length = ((PSTUNMessageHeader *)theArray)->msgLength;
  PSTUNAttribute * attrib = GetFirstAttribute();
  while (length > 0) {
    if (attrib->type == attribute.type) {
      if (attrib->length == attribute.length)
        *attrib = attribute;
      else {
        // More here
      }
      return attrib;
    }
    length -= CalcPaddedAttributeLength(attrib->length);
    attrib = attrib->GetNext();
  }

  return AddAttribute(attribute);
}

PSTUNAttribute * PSTUNMessage::FindAttribute(PSTUNAttribute::Types type) const
{
  if (theArray == NULL)
    return NULL;

  int length = ((PSTUNMessageHeader *)theArray)->msgLength;
  PSTUNAttribute * attrib = GetFirstAttribute();
  while (attrib != NULL && length > 0) {
    if (attrib->type == type)
      return attrib;

    length -= CalcPaddedAttributeLength(attrib->length);
    attrib = attrib->GetNext();
  }
  return NULL;
}

bool PSTUNMessage::Read(PUDPSocket & socket)
{
  PUDPSocket::Slice slice(GetPointer(1000), 1000);
  if (!socket.PUDPSocket::InternalReadFrom(&slice, 1, m_sourceAddressAndPort)) {
    PTRACE_IF(2, socket.GetErrorCode(PChannel::LastReadError) != PChannel::Timeout,
              "STUN\tRead error: " << socket.GetErrorText(PChannel::LastReadError));
    return false;
  }

  SetSize(socket.GetLastReadCount());
  return true;
}
  
bool PSTUNMessage::Write(PUDPSocket & socket) const
{
  int len = sizeof(PSTUNMessageHeader) + ((PSTUNMessageHeader *)theArray)->msgLength;
  PUDPSocket::Slice slice(theArray, len);
  PIPSocketAddressAndPort ap;
  socket.PUDPSocket::InternalGetSendAddress(ap);
  if (socket.PUDPSocket::InternalWriteTo(&slice, 1, ap))
    return true;

  PTRACE(2, "STUN\tError writing to " << socket.GetSendAddress()
         << " - " << socket.GetErrorText(PChannel::LastWriteError));
  return false;
}

bool PSTUNMessage::Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries)
{
  for (PINDEX retry = 0; retry < pollRetries; retry++) {
    if (!request.Write(socket))
      return false;

    if (Read(socket)) {
      if (Validate(request))
        return true;
    }
    else {
      if (socket.GetErrorCode(PChannel::LastReadError) != PChannel::Timeout)
        return false;
    }
  }

  PTRACE(4, "STUN\tTimed out on poll with " << pollRetries << " retries.");
  return false;
}

void PSTUNMessage::InsertMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen)
{
  PSTUNMessageIntegrity * mi = FindAttributeOfType<PSTUNMessageIntegrity>(PSTUNAttribute::MESSAGE_INTEGRITY);
  if (mi == NULL)
    mi = (PSTUNMessageIntegrity *)AddAttribute(PSTUNMessageIntegrity());
  return InsertMessageIntegrity(credentialsHash, credentialsHashLen, mi);
}

void PSTUNMessage::InsertMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi)
{
  return CalculateMessageIntegrity(credentialsHash, credentialsHashLen, mi, mi->hmac);
}

bool PSTUNMessage::CheckMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen)
{
  // get message integrity attribute
  PSTUNMessageIntegrity * mi = FindAttributeOfType<PSTUNMessageIntegrity>(PSTUNAttribute::MESSAGE_INTEGRITY);
  if (mi == NULL)
    return true;

  BYTE hmac[20];
  CalculateMessageIntegrity(credentialsHash, credentialsHashLen, mi, hmac);
  return memcmp(hmac, mi->hmac, 20);
}

void PSTUNMessage::CalculateMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi, BYTE * checkHmac)
{
#if P_SSL
  // calculate hash up to, but not including, MESSAGE_INTEGRITY attribute
  int lengthWithoutMI = (BYTE *)mi - (BYTE *)theArray;

  // calculate message integrity
  PHMAC_SHA1 hmac(credentialsHash, credentialsHashLen);
  PHMAC_SHA1::Result result;
  hmac.Process((BYTE *)theArray, lengthWithoutMI, result);

  // copy the hash to the returned buffer
  memcpy(checkHmac, result.GetPointer(), 20);
#endif
}

///////////////////////////////////////////////////////////////////////

void PSTUNErrorCode::Initialise()
{
  type = ERROR_CODE;
  m_zero1     = 0;
  m_zero2     = 0;
  m_hundreds  = 0;
  m_units     = 0;
  m_reason[0] = '\0';
  length      = (WORD)(4 + strlen(m_reason) + 1);
}


void PSTUNErrorCode::SetErrorCode(int code, const PString & reason)
{ 
  m_hundreds = (BYTE)((code / 100) & 7);
  m_units    = (BYTE) (code % 100);
  int len = reason.GetLength();
  if (len > (int)sizeof(m_reason)-1)
    len = sizeof(m_reason)-1;
  memcpy(m_reason, (const char *)reason, len);
  m_reason[len] = '\0';
  length      = (WORD)(4 + len + 1);
}


///////////////////////////////////////////////////////////////////////

void PSTUNChannelNumber::Initialise()
{
  type = CHANNEL_NUMBER;
  m_channelNumber = PSTUN::MinChannelNumber;
  m_rffu          = 0;
  length          = 4;
}

///////////////////////////////////////////////////////////////////////

static bool TypeIsXOR(int type) 
{
  switch (type) {
    case PSTUNAddressAttribute::XOR_MAPPED_ADDRESS:
    case PSTUNAddressAttribute::XOR_PEER_ADDRESS:
    case PSTUNAddressAttribute::XOR_RELAYED_ADDRESS:
      return true;
    default:
      return false;
  }
}

WORD PSTUNAddressAttribute::GetPort() const
{ 
  if (TypeIsXOR(type))
    return port ^ (RFC5389_MAGIC_COOKIE >> 16);
  else
    return port; 
}

PIPSocket::Address PSTUNAddressAttribute::GetIP() const
{ 
  if (TypeIsXOR(type))
    return PIPSocket::Address(
      (BYTE)(ip[0] ^ (RFC5389_MAGIC_COOKIE >> 24)),
      (BYTE)(ip[1] ^ (RFC5389_MAGIC_COOKIE >> 16)),
      (BYTE)(ip[2] ^ (RFC5389_MAGIC_COOKIE >> 8)),
      (BYTE)(ip[3] ^ RFC5389_MAGIC_COOKIE)
    );
  else
    return PIPSocket::Address(4, ip); 
}

void PSTUNAddressAttribute::GetIPAndPort(PIPSocketAddressAndPort & addrAndPort)
{
  addrAndPort.SetAddress(GetIP());
  addrAndPort.SetPort(GetPort());
}


void PSTUNAddressAttribute::SetIPAndPort(const PIPSocketAddressAndPort & addrAndPort)
{
  pad    = 0;
  family = 1;
  if (TypeIsXOR(type)) {
    port   = addrAndPort.GetPort() ^ (RFC5389_MAGIC_COOKIE >> 16);
    PIPSocket::Address addr = addrAndPort.GetAddress();
    ip[0] = (BYTE)(addr.Byte1() ^ (RFC5389_MAGIC_COOKIE >> 24));
    ip[1] = (BYTE)(addr.Byte2() ^ (RFC5389_MAGIC_COOKIE >> 16));
    ip[2] = (BYTE)(addr.Byte3() ^ (RFC5389_MAGIC_COOKIE >> 8));
    ip[3] = (BYTE)(addr.Byte4() ^ RFC5389_MAGIC_COOKIE);
  }
  else {
    port   = addrAndPort.GetPort();
    PIPSocket::Address addr = addrAndPort.GetAddress();
    ip[0] = addr.Byte1();
    ip[1] = addr.Byte2();
    ip[2] = addr.Byte3();
    ip[3] = addr.Byte4();
  }
}

///////////////////////////////////////////////////////////////////////

/**UDP socket that has been created by the STUN client.
  */
PSTUNUDPSocket::PSTUNUDPSocket()
  : m_natType(PNatMethod::UnknownNat)
{
}

bool PSTUNUDPSocket::OpenSTUN(PSTUNClient & client)
{
  m_natType = client.GetNatType(PFalse);

  switch (m_natType) {
    case PNatMethod::OpenNat :
      return true;

    case PNatMethod::ConeNat :
    case PNatMethod::RestrictedNat :
    case PNatMethod::PortRestrictedNat :
      break;

    case PNatMethod::SymmetricNat :
      if (m_component == PNatMethod::eComponent_RTP || m_component == PNatMethod::eComponent_RTCP)
        return false;
      PTRACE(1, "STUN\tAllowing STUN to be used for non-RTP socket on Symmetric Nat");
      break;
 
    default : // UnknownNet, SymmetricFirewall, BlockedNat
      PTRACE(1, "STUN\tCannot create socket using NAT type " << client.GetNatTypeName());
      return false;
  }

  // do a binding request to verify the server is there
  PSTUNMessage request(PSTUNMessage::BindingRequest);
  PSTUNMessage response;
  SetReadTimeout(client.GetTimeout());
  if (!response.Poll(*this, request, client.GetRetries())) {
    PTRACE(1, "STUN\t" << *this << " unexpectedly went offline.");
    return false;
  }

  // populate fields from Binding Response
  if (!client.GetFromBindingResponse(response, m_serverReflexiveAddress))
    return false;

  //SetSendAddress(0, 0);
  SetReadTimeout(PMaxTimeInterval);

  return true;
}

PNatCandidate PSTUNUDPSocket::GetCandidateInfo()
{
  PNatCandidate candidate;
  candidate.m_component = m_component;
  GetBaseAddress(candidate.m_baseAddress);
  GetLocalAddress(candidate.m_transport);

  switch (m_natType) {
    case PNatMethod::OpenNat:
      candidate.m_type = PNatCandidate::eType_Host;
      break;

    case PNatMethod::ConeNat:
      candidate.m_type = PNatCandidate::eType_ServerReflexive;
      break;

    case PNatMethod::RestrictedNat:
    case PNatMethod::PortRestrictedNat:
    case PNatMethod::SymmetricNat:
    case PNatMethod::SymmetricFirewall:
    case PNatMethod::BlockedNat:
    case PNatMethod::PartialBlockedNat:
    case PNatMethod::NumNatTypes:
    case PNatMethod::UnknownNat:
      break;
  }

  return candidate;
}

bool PSTUNUDPSocket::InternalGetLocalAddress(PIPSocketAddressAndPort & addr)
{
  addr = m_serverReflexiveAddress;
  return true;
}

bool PSTUNUDPSocket::InternalGetBaseAddress(PIPSocketAddressAndPort & addr)
{
  return PUDPSocket::InternalGetLocalAddress(addr);
}

///////////////////////////////////////////////////////////////////////

PSTUNClient::PSTUNClient()
  : m_socket(NULL)
  , numSocketsForPairing(DEFAULT_NUM_SOCKETS_FOR_PAIRING)
{
}

PSTUNClient::~PSTUNClient()
{
  Close();
}

bool PSTUNClient::Open(const PIPSocket::Address & binding) 
{ 
  PWaitAndSignal m(m_mutex);

  if (!m_serverAddress.IsValid()) {
    PTRACE(1, "STUN\tServer port not set.");
    return false;
  }

  switch (FindNatType(binding)) {
    case OpenNat :
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    default :
      PTRACE(1, "STUN\tCannot use STUN with " << m_natType << " type.");
      return false;
  }

  return true;
}

bool PSTUNClient::IsAvailable(const PIPSocket::Address & binding)
{
  PWaitAndSignal m(m_mutex);
  return (m_socket != NULL) && (binding == m_interface);
}

void PSTUNClient::Close()
{
  PWaitAndSignal m(m_mutex);

  if (m_socket != NULL) {
    delete m_socket;
    m_socket = NULL;
  }
}


bool PSTUNClient::SetServer(const PString & server)
{
  PWaitAndSignal m(m_mutex);

  m_serverAddress = PIPSocketAddressAndPort(server, DefaultPort);
  if (!m_serverAddress.IsValid())
    return false;

  //InvalidateCache();

  return true;
}


PString PSTUNClient::GetServer() const
{
  PWaitAndSignal m(m_mutex);

  if (!m_serverAddress.IsValid())
    return PString::Empty();

  return m_serverAddress.AsString();
}


PNatMethod::NatTypes PSTUNClient::GetNatType(const PTimeInterval & maxAge)
{  
  PWaitAndSignal m(m_mutex);

  if ((PTime() - m_timeAddressObtained) < maxAge)
    return m_natType;

  return GetNatType(true);
}


PNatMethod::NatTypes PSTUNClient::GetNatType(bool force)
{
  PWaitAndSignal m(m_mutex);

  if (!force && m_externalAddress.IsValid())
    return m_natType;

  if (!m_serverAddress.IsValid()) {
    PTRACE(1, "STUN\tserver not set");
    return m_natType = UnknownNat;
  }

  if (m_socket == NULL)
    return FindNatType(PIPSocket::GetDefaultIpAny());

  PIPSocketAddressAndPort baseAddress;
  return DoRFC3489Discovery(m_socket, m_serverAddress, baseAddress, m_externalAddress);
}


PNatMethod::NatTypes PSTUNClient::FindNatType(const PIPSocket::Address & binding)
{
  PWaitAndSignal m(m_mutex);

  if (!binding.IsAny() && (binding.IsLoopback() || binding.GetVersion() != 4)) {
    PTRACE(1, "STUN\tCannot use interface " << binding << " to find STUN server");
    return m_natType = UnknownNat;
  }

  if (m_socket != NULL) {
    delete m_socket;
    m_socket = NULL;
  }

  m_socket = new PSTUNUDPSocket;

  // if a specific interface is given, use only that interface
  if (!binding.IsAny()) {
    m_interface = binding;
    if (!InternalOpenSocket(eComponent_Unknown, binding, *m_socket, singlePortInfo)) {
      PTRACE(1, "STUN\tUnable to open a socket on interface " << m_interface << "");
      delete m_socket;
      m_socket = NULL;
      return m_natType = UnknownNat;
    }
    m_socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
    m_socket->SetReadTimeout(replyTimeout);

    return GetNatType(true);
  }

  // get list of interfaces
  PList<PSTUNUDPSocket> sockets;
  PIPSocket::InterfaceTable interfaces;
  if (PIPSocket::GetInterfaceTable(interfaces)) {
    for (PINDEX i =0; i < interfaces.GetSize(); i++) {
      PIPSocket::Address binding = interfaces[i].GetAddress();
      if (!binding.IsLoopback() && (binding.GetVersion() == 4)) {
        PSTUNUDPSocket * socket = new PSTUNUDPSocket;
        if (InternalOpenSocket(eComponent_Unknown, binding, *socket, singlePortInfo))
          sockets.Append(socket);
        else
          delete socket;
      }
    }
    if (interfaces.IsEmpty()) {
      PTRACE(1, "STUN\tNo interfaces available to find STUN server.");
      return m_natType = UnknownNat;
    }
  }
  else {
    PSTUNUDPSocket * socket = new PSTUNUDPSocket;
    sockets.Append(socket);
    if (!InternalOpenSocket(eComponent_Unknown, PIPSocket::GetDefaultIpAny(), *socket, singlePortInfo))
      return m_natType = UnknownNat;
  }

  // send binding request on all interfaces and wait for a reply
  PSTUNMessage requestI(PSTUNMessage::BindingRequest);
  requestI.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI;

  for (PINDEX retry = 0; retry < m_pollRetries; ++retry) {
    PSocket::SelectList selectList;
    for (PList<PSTUNUDPSocket>::iterator socket = sockets.begin(); socket != sockets.end(); ++socket) {
      if (requestI.Write(*socket))
        selectList += *socket;
    }

    if (selectList.IsEmpty())
      return m_natType = UnknownNat; // Could not send on any interface!

    // wait for reply
    PChannel::Errors error = PIPSocket::Select(selectList, replyTimeout);
    if (error != PChannel::NoError) {
      PTRACE(1, "STUN\tError in select - " << PChannel::GetErrorText(error));
      return m_natType = UnknownNat;
    }

    // take the first one
    if (!selectList.IsEmpty()) {
      PSTUNUDPSocket & udp = (PSTUNUDPSocket &)selectList.front();
      if (responseI.Read(udp) && responseI.Validate(requestI)) {
        m_socket = &udp;
        sockets.AllowDeleteObjects(false);
        sockets.Remove(m_socket);
        sockets.AllowDeleteObjects(true);
        break;
      }
    }
  }

  // complete discovery
  m_socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
  m_socket->SetReadTimeout(replyTimeout);
  PIPSocketAddressAndPort ap;
  m_socket->GetBaseAddress(ap);
  m_interface = ap.GetAddress();
  return m_natType = FinishRFC3489Discovery(responseI, m_socket, m_externalAddress);
}


/**Get the current server address and port being used.
  */
bool PSTUNClient::GetServerAddress(PIPSocketAddressAndPort & serverAddress) const
{
  PWaitAndSignal m(m_mutex);

  if (!m_serverAddress.IsValid())
    return false;

  serverAddress = m_serverAddress;
  return true;
}


bool PSTUNClient::GetExternalAddress(PIPSocket::Address & externalAddress, const PTimeInterval & maxAge)
{
  PWaitAndSignal m(m_mutex);

  if (GetNatType(maxAge) == UnknownNat)
    return false;

  externalAddress = m_externalAddress.GetAddress();
  return true;
}

bool PSTUNClient::GetInterfaceAddress(PIPSocket::Address & interfaceAddress) const
{
  PWaitAndSignal m(m_mutex);

  interfaceAddress = m_interface;
  return true;
}

//
// this function must be thread safe as it will be called from multple threads
//
bool PSTUNClient::InternalOpenSocket(BYTE component, const PIPSocket::Address & binding, PSTUNUDPSocket & socket, PortInfo & portInfo)
{
  if (!m_serverAddress.IsValid()) {
    PTRACE(1, "STUN\tServer port not set.");
    return false;
  }

  if (portInfo.basePort == 0) {
    if (!socket.Listen(binding, 1)) {
      PTRACE(3, "STUN\tCannot bind port to " << m_interface);
      return false;
    }
  }  
  else {
    WORD startPort = portInfo.currentPort;
    PTRACE(3, "STUN\tUsing ports " << portInfo.basePort << " through " << portInfo.maxPort << " starting at " << startPort);
    for (;;) {
      bool status = socket.Listen(binding, 1, portInfo.currentPort);
      PWaitAndSignal mutex(portInfo.mutex);
      portInfo.currentPort++;
      if (portInfo.currentPort > portInfo.maxPort)
        portInfo.currentPort = portInfo.basePort;
      if (status)
        break;
      if (portInfo.currentPort == startPort) {
        PTRACE(3, "STUN\tListen failed on " << m_interface << ":" << portInfo.currentPort);
        return false;
      }
    } 
  }

  socket.SetComponent(component);
  socket.PUDPSocket::InternalSetSendAddress(m_serverAddress);

  return true;
}


void PSTUNClient::SetCredentials(const PString &, const PString &, const PString &)
{
}

bool PSTUNClient::CreateSocket(BYTE component, PUDPSocket * & udpSocket, const PIPSocket::Address & binding, WORD port)
{
  PWaitAndSignal m(m_mutex);

  if (!binding.IsAny() && binding != m_interface)
    return false;

  PSTUNUDPSocket * stunSocket = new PSTUNUDPSocket;

  bool status = false;
  if (port == 0)
    status = InternalOpenSocket(component, m_interface, *stunSocket, singlePortInfo);
  else {
    PortInfo portInfo(port);
    status = InternalOpenSocket(component, m_interface, *stunSocket, portInfo);
  }

  if (status)
    status = stunSocket->OpenSTUN(*this);

  if (!status) {
    delete stunSocket;
    stunSocket = NULL;
  }

  if (stunSocket != NULL) {
    PIPSocketAddressAndPort ba, la;
    stunSocket->GetBaseAddress(ba);
    stunSocket->GetLocalAddress(la);
    PTRACE(2, "STUN\tsocket created : " << ba << " -> " << la);
  }

  udpSocket = stunSocket;
  return udpSocket != NULL;
}

struct SocketInfo {
  SocketInfo()
    : m_stunSocket(NULL)
    , m_ready(false)
  { }

  ~SocketInfo()
  { delete m_stunSocket; }
   
  PSTUNUDPSocket * m_stunSocket;
  bool m_ready;
  PSTUNMessage    m_request;
  PSTUNMessage    m_response;
};

bool PSTUNClient::CreateSocketPair(PUDPSocket * & socket1,
                                   PUDPSocket * & socket2,
                                   const PIPSocket::Address & binding)
{
  PWaitAndSignal m(m_mutex);

  if (!binding.IsAny() && binding != m_interface)
    return false;

  socket1 = NULL;
  socket2 = NULL;

  switch (GetNatType(PFalse)) {
    case OpenNat :
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (pairedPortInfo.basePort == 0 || pairedPortInfo.basePort > pairedPortInfo.maxPort)
      {
        PTRACE(1, "STUN\tInvalid local UDP port range "
               << pairedPortInfo.currentPort << '-' << pairedPortInfo.maxPort);
        return PFalse;
      }
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      PTRACE(1, "STUN\tCannot create socket pair using NAT type " << GetNatTypeName());
      return PFalse;
  }

  PPtrVector<SocketInfo> socketInfo;

  // send binding requests until we get a pair of adjacent sockets
  for (PINDEX socketCount = 0; socketCount < numSocketsForPairing; ++socketCount)  {
    // always ensure we have two sockets
    while (socketInfo.size() < 2) {

      // create a socket
      size_t idx = socketInfo.size();
      socketInfo.push_back(new SocketInfo());
      SocketInfo & info = *socketInfo[idx];
      info.m_stunSocket = new PSTUNUDPSocket();
      if (!InternalOpenSocket(eComponent_Unknown, m_interface, *info.m_stunSocket, pairedPortInfo)) {
        PTRACE(1, "STUN\tUnable to open socket to " << *this);
        return false;
      }

      // if necessary, send a binding request
      if (GetNatType(false) == OpenNat) {
        info.m_ready = true;
        socketInfo[idx]->m_stunSocket->GetBaseAddress(socketInfo[idx]->m_stunSocket->m_serverReflexiveAddress);
      }
      else {
        info.m_stunSocket->SetReadTimeout(replyTimeout);
        info.m_request = PSTUNMessage(PSTUNMessage::BindingRequest);
        if (!info.m_request.Write(*info.m_stunSocket)) {
          PTRACE(1, "STUN\tsocket write failed");
          return false;
        }
      }
    }

    // wait for something to respond
    if ((GetNatType(false) != OpenNat) && (!socketInfo[0]->m_ready || !socketInfo[1]->m_ready)) {
      int r = PIPSocket::Select(*socketInfo[0]->m_stunSocket, *socketInfo[1]->m_stunSocket, replyTimeout);
      if (r == 0)
        socketInfo.Clear();
      else { 
        if ((r == -1 || r == -3) && socketInfo[0]->m_response.Read(*socketInfo[0]->m_stunSocket) && socketInfo[0]->m_response.Validate(socketInfo[0]->m_request)) {
          GetFromBindingResponse(socketInfo[0]->m_response, socketInfo[0]->m_stunSocket->m_serverReflexiveAddress);
          socketInfo[0]->m_ready = true;
        }
        if ((r == -2 || r == -3) && socketInfo[1]->m_response.Read(*socketInfo[1]->m_stunSocket) && socketInfo[1]->m_response.Validate(socketInfo[1]->m_request)) {
          socketInfo[1]->m_ready = true;
          GetFromBindingResponse(socketInfo[1]->m_response, socketInfo[1]->m_stunSocket->m_serverReflexiveAddress);
        }
      }
    }

    // if both sockets are ready, see if they are adjacent
    if ((socketInfo.size() == 2) && socketInfo[0]->m_ready && socketInfo[1]->m_ready) {
      if ((socketInfo[0]->m_stunSocket->port&1) == 0 && (socketInfo[0]->m_stunSocket->port+1) == socketInfo[1]->m_stunSocket->port) {

        socketInfo[0]->m_stunSocket->PUDPSocket::InternalSetSendAddress(PIPSocket::Address(0, 0));
        socketInfo[0]->m_stunSocket->SetReadTimeout(PMaxTimeInterval);

        socketInfo[1]->m_stunSocket->PUDPSocket::InternalSetSendAddress(PIPSocket::Address(0, 0));
        socketInfo[1]->m_stunSocket->SetReadTimeout(PMaxTimeInterval);

        PIPSocketAddressAndPort ba1, la1, ba2, la2;
        socketInfo[0]->m_stunSocket->GetBaseAddress(ba1);
        socketInfo[0]->m_stunSocket->GetLocalAddress(la1);
        socketInfo[1]->m_stunSocket->GetBaseAddress(ba2);
        socketInfo[1]->m_stunSocket->GetLocalAddress(la2);
        PTRACE(2, "STUN\tsocket pair created : " << ba1 << " -> " << la1 << ", " << ba2 << " -> " << la2);

        socket1 = socketInfo[0]->m_stunSocket;
        socket2 = socketInfo[1]->m_stunSocket;

        socketInfo[0]->m_stunSocket = NULL;
        socketInfo[1]->m_stunSocket = NULL;


        return true;
      }

      // clear the lowest socket
      delete socketInfo[0];
      socketInfo.erase(socketInfo.begin());
    }
  }

  PTRACE(2, "STUN\tCould not get a pair of adjacent port numbers from NAT");
  return false;
}


PSTUNClient::RTPSupportTypes PSTUNClient::GetRTPSupport(bool force)
{
  PWaitAndSignal m(m_mutex);

  switch (GetNatType(force)) {
    // types that do support RTP 
    case OpenNat:
      return RTPSupported;

    // types that support RTP if media sent first
    case ConeNat:
    case SymmetricFirewall:
    case RestrictedNat:
    case PortRestrictedNat:
      return RTPIfSendMedia;

    // types that do not support RTP
    case BlockedNat:
    case SymmetricNat:
      return RTPUnsupported;

    // types that have unknown RTP support
    default:
      return RTPUnknown;
  }
}

//////////////////////////////////////////////////////////////////////
   
void PTURNRequestedTransport::Initialise(BYTE protocol)
{
  m_protocol = protocol;
  type       = REQUESTED_TRANSPORT;
  length     = 4;
  m_rffu1    = 0;
  m_rffu2    = 0;
  m_rffu3    = 0;
}

//////////////////////////////////////////////////////////////////////

PTURNClient::PTURNClient()
  : PSTUNClient()
{
}

bool PTURNClient::Open(const PIPSocket::Address & binding)
{
  if (!m_serverAddress.IsValid()) {
    PTRACE(1, "TURN\tServer not set.");
    return false;
  }

  if (FindNatType(binding) == PNatMethod::UnknownNat || (m_natType == PNatMethod::BlockedNat)) {
    PTRACE(1, "TURN\tUnable to use TURN with unknown or blocked NAT");
    return false;
  }

  return true;
}


//////////////////////////////////////////////////////////////////////

PTURNUDPSocket::PTURNUDPSocket()
  : PSTUNUDPSocket()
  , m_allocationMade(false)
  , m_channelNumber(PTURNClient::MinChannelNumber)
  , m_usingTURN(false)
{
  // first slice for TURN header
  m_txVect.resize(3);
  m_txVect[0].SetBase(&m_txHeader);
  m_txVect[0].SetLength(sizeof(m_txHeader));
  m_txHeader.m_channelNumber = (WORD)m_channelNumber;

  // first slice for TURN header
  m_rxVect.resize(3);
  m_rxVect[0].SetBase(&m_rxHeader);
  m_rxVect[0].SetLength(sizeof(m_rxHeader));
}


PTURNUDPSocket::~PTURNUDPSocket()
{
  Close();
}


PNatCandidate PTURNUDPSocket::GetCandidateInfo()
{
  PNatCandidate candidate(PNatCandidate::eType_Relay, m_component);
  GetBaseAddress(candidate.m_baseAddress);
  GetLocalAddress(candidate.m_transport);
  return candidate;
}


int PTURNUDPSocket::OpenTURN(PTURNClient & client)
{
  m_usingTURN = false;

  // can only use TURN for RTP
  if ((m_component != PNatMethod::eComponent_RTP) && (m_component != PNatMethod::eComponent_RTCP)) {
    PTRACE(2, "TURN\tUsing STUN for non RTP socket");
    return OpenSTUN(client) ? 0 : -1;
  }

  PSTUN::m_natType = client.GetNatType(PFalse);

#ifndef ENABLE_TURN_FOR_ALL
  // for some NAT types, STUN is just fine
  // for others, we gotta use TURN
  switch (PSTUN::m_natType) {
    case PNatMethod::OpenNat :
    case PNatMethod::ConeNat :
      return OpenSTUN(client);

    case PNatMethod::RestrictedNat :
    case PNatMethod::PortRestrictedNat :
    case PNatMethod::SymmetricNat :
    default : // UnknownNet, SymmetricFirewall, BlockedNat
      break;
  }
#endif

  client.GetServerAddress(m_serverAddress);

  bool evenPort = false; //(m_component == PNatMethod::eComponent_RTP);

  SetCredentials(client.m_userName, client.m_password, client.m_realm);

  // create an allocation on the STUN server
  m_protocol = PTURNRequestedTransport::ProtocolUDP;

  PSTUNMessage allocateRequest(PSTUNMessage::Allocate);
  allocateRequest.AddAttribute(PTURNRequestedTransport(m_protocol));
  if (evenPort)
    allocateRequest.AddAttribute(PTURNEvenPort());

  PSTUNMessage allocateResponse;
  int code = MakeAuthenticatedRequest(this, allocateRequest, allocateResponse);
  if (code != 0)
    return code;

  m_allocationMade = true;

  PSTUNAddressAttribute * addrAttr = (PSTUNAddressAttribute *)allocateResponse.FindAttribute(PSTUNAttribute::XOR_RELAYED_ADDRESS);
  if (addrAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain XOR_RELAYED_ADDRESS");
    return -1;
  }
  addrAttr->GetIPAndPort(m_relayedAddress);

  addrAttr = (PSTUNAddressAttribute *)allocateResponse.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (addrAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain XOR_MAPPED_ADDRESS");
    return -1;
  }
  addrAttr->GetIPAndPort(m_serverReflexiveAddress);

  PTURNLifetime * lifetimeAttr = (PTURNLifetime *)allocateResponse.FindAttribute(PSTUNAttribute::LIFETIME);
  if (lifetimeAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain LIFETIME");
    return -1;
  }
  m_lifeTime = lifetimeAttr->GetLifetime();

  m_usingTURN = true;
  PTRACE(2, "TURN\tAddress/port " << m_relayedAddress << " allocated on server with lifetime " << m_lifeTime);

  return 0;
}


bool PTURNUDPSocket::Close()
{
  // delete allocation
  if (m_allocationMade) {
    PSTUNMessage request(PSTUNMessage::Refresh);
    request.AddAttribute(PTURNLifetime(0));

    PSTUNMessage response;
    MakeAuthenticatedRequest(this, request, response);

    m_allocationMade = false;
  }

  return PSTUNUDPSocket::Close();
}


bool PTURNUDPSocket::InternalGetLocalAddress(PIPSocketAddressAndPort & addr)
{
  if (!m_usingTURN)
    return PSTUNUDPSocket::InternalGetLocalAddress(addr);

  addr = m_relayedAddress;
  return true;
}

void PTURNUDPSocket::InternalGetSendAddress(PIPSocketAddressAndPort & addr)
{
  if (!m_usingTURN)
    return PSTUNUDPSocket::InternalGetSendAddress(addr);

  addr = m_peerIpAndPort;
}


void PTURNUDPSocket::InternalSetSendAddress(const PIPSocketAddressAndPort & ipAndPort)
{
  if (!m_usingTURN)
    return PUDPSocket::InternalSetSendAddress(ipAndPort);

  // set permission on TURN server
  if (ipAndPort != m_peerIpAndPort) {

    PTRACE(3, "PTURN\tSending ChannelBind request for channel " << m_channelNumber << " to set peer to " << ipAndPort);
    m_peerIpAndPort = ipAndPort;
    PSTUNMessage permissionRequest(PSTUNMessage::ChannelBind);
    {
      PSTUNChannelNumber attr;
      attr.m_channelNumber = (WORD)m_channelNumber;
      permissionRequest.AddAttribute(attr);
      m_txHeader.m_channelNumber = attr.m_channelNumber;

      if (m_channelNumber < PTURNClient::MaxChannelNumber)
        m_channelNumber++;
      else
        m_channelNumber = PTURNClient::MinChannelNumber;
    }
    {
      PSTUNAddressAttribute attr;
      attr.InitAddrAttr(PSTUNAttribute::XOR_PEER_ADDRESS);
      attr.SetIPAndPort(ipAndPort);
      permissionRequest.AddAttribute(attr);
    }

    PIPSocketAddressAndPort ap;
    PUDPSocket::InternalGetSendAddress(ap);
    PUDPSocket::InternalSetSendAddress(m_serverAddress);

    PSTUNMessage permissionResponse;
    bool stat = MakeAuthenticatedRequest(this, permissionRequest, permissionResponse) == 0;

    PUDPSocket::InternalSetSendAddress(ap);

    if (!stat) {
      PSTUNErrorCode * errorAttribute = (PSTUNErrorCode *)permissionResponse.FindAttribute(PSTUNAttribute::ERROR_CODE);
      if (errorAttribute == NULL)
        PTRACE(2, "PTURN\tChannelBind failed with no useful error");
      else 
        PTRACE(2, "PTURN\tChannelBind failed with error " << errorAttribute->GetErrorCode() << ", reason = '" << errorAttribute->GetReason() << "'");
    }
  }
}



bool PTURNUDPSocket::InternalWriteTo(const Slice * slices, size_t sliceCount, const PIPSocketAddressAndPort & ipAndPort)
{
  if (!m_usingTURN)
    return PUDPSocket::InternalWriteTo(slices, sliceCount, ipAndPort);

  // one slice for the TURN header, and one padding
  m_txVect.resize(1+sliceCount);

  // copy the slices and count the length (needed for padding)
  int len = 0;
  size_t i;
  for (i = 0; i < sliceCount; ++i) {
    m_txVect[i+1] = slices[i];
    len += slices[i].GetLength();
  }

  m_txHeader.m_length = (WORD)len;

  if ((len & 3) != 0) {
    // extra slice for padding
    m_txVect.resize(1+sliceCount+1);
    m_txVect[i].SetBase(m_txPadding);
    m_txVect[i].SetLength(4 - (len & 3));
    ++i;
  }

  bool status = PUDPSocket::InternalWriteTo(&m_txVect[0], i+1, m_serverAddress);

  if (status)
    lastWriteCount -= sizeof(m_txVect[1].GetLength());

  return status;
}


bool PTURNUDPSocket::InternalReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort)
{
  if (!m_usingTURN)
    return PUDPSocket::InternalReadFrom(slices, sliceCount, ipAndPort);

  // one extra slice for the TURN header, and one extra for padding
  m_rxVect.resize(1+sliceCount+1);

  // copy the slices
  size_t i;
  for (i = 0; i < sliceCount; ++i)
    m_rxVect[i+1] = slices[i];
  m_rxVect[i+1].SetBase(m_rxPadding);
  m_rxVect[i+1].SetLength(sizeof(m_rxPadding));

  PIPSocketAddressAndPort ap;
  bool status = PUDPSocket::InternalReadFrom(&m_rxVect[0], sliceCount + 2, ap);
  ipAndPort = m_peerIpAndPort;

  if (status)
    lastReadCount = m_rxHeader.m_length;

  return status;
}

///////////////////////////////////////////////////////////////////////////////////////////

PSTUNClient::RTPSupportTypes PTURNClient::GetRTPSupport(bool force)
{
  PWaitAndSignal m(m_mutex);

  switch (GetNatType(force)) {
    // types that do support RTP 
    case OpenNat:
    case SymmetricNat:
    case ConeNat:
    case SymmetricFirewall:
    case RestrictedNat:
    case PortRestrictedNat:
      return RTPSupported;

    // types that do not support RTP
    case BlockedNat:
    default:
      return RTPUnknown;
  }
}

struct AllocateSocketFunctor {
  AllocateSocketFunctor(PTURNClient & client, BYTE component, const PIPSocket::Address & iface, PNatMethod::PortInfo & portInfo)
    : m_client(client)
    , m_component(component)
    , m_interface(iface)
    , m_turnSocket(NULL)
    , m_portInfo(portInfo)
    , m_status(true)
  { }

  void operator()(PThread & thread);
    
  PTURNClient & m_client;
  BYTE m_component;
  PIPSocket::Address m_interface;
  PTURNUDPSocket * m_turnSocket;
  PNatMethod::PortInfo & m_portInfo;
  bool m_status;
};

void AllocateSocketFunctor::operator () (PThread &)
{
  int retryCount = 3;
  m_status = true;
  while (retryCount > 0) {

    m_turnSocket = new PTURNUDPSocket();

    if (!m_client.InternalOpenSocket(PNatMethod::eComponent_RTP, m_interface, *m_turnSocket, m_portInfo)) {
      PTRACE(2, "TURN\tCould not create socket");
      m_status = false;
      break;
    }

    int code = m_turnSocket->OpenTURN(m_client);
    m_status = (code == 0);

    if (m_status) {
      PIPSocketAddressAndPort ap;
      m_turnSocket->GetLocalAddress(ap);
      if ((m_component != PNatMethod::eComponent_RTP) || ((ap.GetPort() & 1) == 0))
        break;
      PTRACE(2, "TURN\tAllocate returned odd socket for RTP - trying again");
    }
    else if (code == 437) {
      PTRACE(2, "TURN\tAllocate returned 437 Mismatch - trying again");
      retryCount--;
    }
    else {
      PTRACE(2, "TURN\tAllocate returned error " << code << " - cannot create socket");
      break;
    }

    delete m_turnSocket;
    m_turnSocket = NULL;
  }

  if (!m_status) {
    PTRACE(2, "TURN\tCould not create/allocate TURN socket");
    delete m_turnSocket;
    m_turnSocket = NULL; 
  }
  else {
    m_turnSocket->PUDPSocket::InternalSetSendAddress(PIPSocket::Address(0, 0));
    m_turnSocket->SetReadTimeout(PMaxTimeInterval);
  }
}

typedef PThreadFunctor<AllocateSocketFunctor> AllocateSocketThread;

bool PTURNClient::CreateSocket(BYTE component, PUDPSocket * & socket, const PIPSocket::Address & binding, WORD port)
{
  if (component != PNatMethod::eComponent_RTP && component != PNatMethod::eComponent_RTCP)
    return PSTUNClient::CreateSocket(component, socket, binding, port);

  if (!binding.IsAny() && binding != m_interface)
    return false;
  
  socket = NULL;

  PortInfo * portInfo;
  PortInfo localPortInfo(port);
  if (port != 0)
    portInfo = &localPortInfo;
  else
    portInfo = &singlePortInfo;

  AllocateSocketFunctor op(*this, component, m_interface, *portInfo);

  op.operator()(*PThread::Current());

  PTURNUDPSocket * turnSocket = op.m_turnSocket;

  if (op.m_status) {
    PIPSocketAddressAndPort ba, la;
    turnSocket->GetBaseAddress(ba);
    turnSocket->GetLocalAddress(la);
    PTRACE(2, "TURN\tsocket created : " << ba << " -> " << la);
  }

  socket = turnSocket;
  return socket != NULL;
}

bool PTURNClient::CreateSocketPair(PUDPSocket * & socket1,
                                   PUDPSocket * & socket2,
                                   const PIPSocket::Address & binding)
{
  if (!binding.IsAny() && binding != m_interface)
    return false;

#ifndef ENABLE_TURN_FOR_ALL
  switch (GetNatType(PFalse)) {
    case OpenNat :
    //case ConeNat : 
      PTRACE(3, "TURN\tNAT type allows use of STUN for socket pair");
      return PSTUNClient::CreateSocketPair(socket1, socket2, binding);

    case RestrictedNat :
    case PortRestrictedNat :
    case SymmetricNat :
    default : // UnknownNet, SymmetricFirewall, BlockedNat
      break;
  }
#endif

  socket1 = NULL;
  socket2 = NULL;

  AllocateSocketFunctor op1(*this, PNatMethod::eComponent_RTP,   binding, pairedPortInfo);
  AllocateSocketFunctor op2(*this, PNatMethod::eComponent_RTCP,  binding, pairedPortInfo);
  PThread * thread1 = new PThreadFunctor<AllocateSocketFunctor>(op1);
  PThread * thread2 = new PThreadFunctor<AllocateSocketFunctor>(op2);

  PTRACE(3, "TURN\tWaiting for allocations to complete");
  thread1->WaitForTermination();
  delete thread1;

  thread2->WaitForTermination();
  delete thread2;

  if (!op1.m_status || !op2.m_status) {
    delete op1.m_turnSocket;
    delete op2.m_turnSocket;
    return false;
  }

  PTURNUDPSocket * turnSocket1 = op1.m_turnSocket;
  PTURNUDPSocket * turnSocket2 = op2.m_turnSocket;

  PIPSocketAddressAndPort ba1, la1, ba2, la2;
  turnSocket1->GetBaseAddress(ba1);
  turnSocket1->GetLocalAddress(la1);
  turnSocket2->GetBaseAddress(ba2);
  turnSocket2->GetLocalAddress(la2);
  PTRACE(2, "STUN\tsocket pair created : " << ba1 << " -> " << la1 << ", " << ba2 << " -> " << la2);

  socket1 = turnSocket1;
  socket2 = turnSocket2;

  return true;
}

void PTURNClient::SetCredentials(const PString & username, const PString & password, const PString & realm)
{
  m_userName  = username;
  m_password  = password;
  m_realm     = realm;
}

bool PTURNClient::RefreshAllocation(DWORD lifetime)
{
  PSTUNMessage request(PSTUNMessage::Refresh);
  if (lifetime > 0)
    request.AddAttribute(PTURNLifetime(lifetime));

  PSTUNMessage response;
  return MakeAuthenticatedRequest(m_socket, request, response) == 0;
}


#endif P_STUN

// End of File ////////////////////////////////////////////////////////////////
