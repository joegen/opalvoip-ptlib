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

#if P_STUN

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/pstun.h>

#include <ptclib/random.h>
#include <ptclib/cypher.h>
#include <ptclib/pdns.h>


#define new PNEW


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
  : m_pollRetries(DEFAULT_POLL_RETRIES)
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
    return PNatMethod::UnknownNat;
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
      return PNatMethod::BlockedNat;
    }
  }

  PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << m_serverAddress);
      return PNatMethod::UnknownNat; // Protocol error
    }
  }

  mappedAddress->GetIPAndPort(externalAddressAndPort);

  bool notNAT = (socket->GetPort() == externalAddressAndPort.GetPort()) && PIPSocket::IsLocalHost(externalAddressAndPort.GetAddress());

  // can only guess based on a single sample
  if (!canDoChangeRequest) {
    PNatMethod::NatTypes natType = notNAT ? PNatMethod::OpenNat : PNatMethod::SymmetricNat;
    PTRACE(3, "STUN\tSTUN server has only one address - best guess is that NAT is " << PNatMethod::GetNatTypeString(natType));
    return natType;
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
    PNatMethod::NatTypes natType = (testII ? PNatMethod::OpenNat : PNatMethod::PartiallyBlocked);
    // Is not NAT or symmetric firewall
    PTRACE(2, "STUN\tTest I and II indicate nat is " << PNatMethod::GetNatTypeString(natType));
    return natType;
  }

  if (testII)
    return PNatMethod::ConeNat;

  PSTUNAddressAttribute * changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::CHANGED_ADDRESS);
  if (changedAddress == NULL) {
    changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::OTHER_ADDRESS);
    if (changedAddress == NULL) {
      PTRACE(3, "STUN\tTest II response indicates no alternate address in use - testing finished");
      return PNatMethod::UnknownNat; // Protocol error
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
    return PNatMethod::PartiallyBlocked;
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
      return PNatMethod::SymmetricNat;
  }

  socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
  PSTUNMessage requestIII(PSTUNMessage::BindingRequest);
  requestIII.SetAttribute(PSTUNChangeRequest(false, true));
  PSTUNMessage responseIII;

  return responseIII.Poll(*socket, requestIII, m_pollRetries) ? PNatMethod::RestrictedNat : PNatMethod::PortRestrictedNat;
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

  if ((CalcPaddedAttributeLength(attr->length) > (int)GetSize()))
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

#if P_SSL
void PSTUNMessage::CalculateMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi, BYTE * checkHmac)
{
  // calculate hash up to, but not including, MESSAGE_INTEGRITY attribute
  PINDEX lengthWithoutMI = (char *)mi - theArray;

  // calculate message integrity
  PHMAC_SHA1 hmac(credentialsHash, credentialsHashLen);
  PHMAC_SHA1::Result result;
  hmac.Process((BYTE *)theArray, lengthWithoutMI, result);

  // copy the hash to the returned buffer
  memcpy(checkHmac, result.GetPointer(), 20);
}
#else
void PSTUNMessage::CalculateMessageIntegrity(BYTE *, PINDEX, PSTUNMessageIntegrity *, BYTE *)
{
}
#endif

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
PSTUNUDPSocket::PSTUNUDPSocket(PNatMethod::Component component)
  : PNATUDPSocket(component)
  , m_natType(PNatMethod::UnknownNat)
{
}

bool PSTUNUDPSocket::OpenSTUN(PSTUNClient & client)
{
  m_natType = client.GetNatType(false);

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
 
    default :
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


void PSTUNUDPSocket::GetCandidateInfo(PNatCandidate & candidate)
{
  PNATUDPSocket::GetCandidateInfo(candidate);

  switch (m_natType) {
    case PNatMethod::OpenNat:
      candidate.m_type = PNatCandidate::eType_Host;
      break;

    case PNatMethod::ConeNat:
      candidate.m_type = PNatCandidate::eType_ServerReflexive;
      break;

    default :
      break;
  }
}


bool PSTUNUDPSocket::InternalGetLocalAddress(PIPSocketAddressAndPort & addr)
{
  addr = m_serverReflexiveAddress;
  return true;
}


///////////////////////////////////////////////////////////////////////

typedef PSTUNClient PNatMethod_STUN;
PCREATE_NAT_PLUGIN(STUN, "STUN Server");

PSTUNClient::PSTUNClient(unsigned priority)
  : PNatMethod(priority)
  , m_socket(NULL)
  , m_numSocketsForPairing(DEFAULT_NUM_SOCKETS_FOR_PAIRING)
{
}

PSTUNClient::~PSTUNClient()
{
  Close();
}


const char * PSTUNClient::MethodName()
{
  return PPlugin_PNatMethod_STUN::ServiceName();
}


PCaselessString PSTUNClient::GetMethodName() const
{
  return MethodName();
}


bool PSTUNClient::Open(const PIPSocket::Address & binding) 
{ 
  if (!binding.IsAny() && (binding.IsLoopback() || binding.GetVersion() != 4)) {
    PTRACE(1, "STUN\tCannot use interface " << binding << " to find STUN server");
    return false;
  }

  PWaitAndSignal m(m_mutex);

  if (m_interface != binding) {
    Close();
    m_interface = binding;
  }

  if (m_serverAddress.IsValid() || SetServer(m_serverName))
    return GetNatType(true) != UnknownNat;

  PTRACE(1, "STUN\tServer port not set.");
  return false;
}


bool PSTUNClient::IsAvailable(const PIPSocket::Address & binding, PObject * context)
{
  PWaitAndSignal m(m_mutex);
  return PNatMethod::IsAvailable(binding, context) &&
         m_socket != NULL &&
         (binding.IsAny() || binding == m_interface);
}


void PSTUNClient::Close()
{
  PWaitAndSignal m(m_mutex);

  delete m_socket;
  m_socket = NULL;
  m_externalAddress = m_interface = PIPSocket::GetInvalidAddress();
  m_natType = UnknownNat;

  PNatMethod::Close();
}


bool PSTUNClient::SetServer(const PString & server)
{
  if (server.IsEmpty())
    return false;

  m_serverName = server;

#if P_DNS_RESOLVER
  PIPSocketAddressAndPortVector addresses;
  if (PDNS::LookupSRV(server, "_stun._udp.", DefaultPort, addresses) && !addresses.empty()) {
    PTRACE(3, "STUN\tUsing DNS SRV record for server at " << addresses[0]);
    return InternalSetServer(addresses[0]);
  }
#endif

  return InternalSetServer(PIPSocketAddressAndPort(server, DefaultPort));
}


bool PSTUNClient::InternalSetServer(const PIPSocketAddressAndPort & addr)
{
  if (!addr.IsValid())
    return false;

  PWaitAndSignal m(m_mutex);

  if (m_serverAddress != addr) {
    PTRACE(4, "STUN\tServer set to " << addr << " (" << m_serverName << ')');
    m_serverAddress = addr;
    Close();
  }

  return true;
}


PString PSTUNClient::GetServer() const
{
  PWaitAndSignal m(m_mutex);

  if (!m_serverName.IsEmpty())
    return m_serverName;

  if (m_serverAddress.IsValid())
    return m_serverAddress.AsString();

  return PString::Empty();
}


bool PSTUNClient::GetServerAddress(PIPSocketAddressAndPort & serverAddress) const
{
  PWaitAndSignal m(m_mutex);

  if (!m_serverAddress.IsValid())
    return false;

  serverAddress = m_serverAddress;
  return true;
}


bool PSTUNClient::GetInterfaceAddress(PIPSocket::Address & interfaceAddress) const
{
  PWaitAndSignal m(m_mutex);

  interfaceAddress = m_interface;
  return true;
}


PNATUDPSocket * PSTUNClient::InternalCreateSocket(Component component, PObject *)
{
  return new PSTUNUDPSocket(component);
}


void PSTUNClient::InternalUpdate()
{  
  PWaitAndSignal m(m_mutex);

  m_natType = UnknownNat;

  if (!m_interface.IsValid())
    return;

  if (!m_serverAddress.IsValid() && !SetServer(m_serverName)) {
    PTRACE(1, "STUN\tServer not set");
    Close();
    return;
  }

  if (m_socket != NULL) {
    PIPSocketAddressAndPort baseAddress;
    m_natType = DoRFC3489Discovery(m_socket, m_serverAddress, baseAddress, m_externalAddress);
    return;
  }

  delete m_socket;
  m_socket = new PSTUNUDPSocket(eComponent_Unknown);

  // if a specific interface is given, use only that interface
  if (!m_interface.IsAny()) {
    if (!m_singlePortRange.Listen(*m_socket,m_interface)) {
      PTRACE(1, "STUN\tUnable to open a socket on interface " << m_interface << "");
      Close();
      m_natType = UnknownNat;
      return;
    }

    m_socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
    m_socket->SetReadTimeout(replyTimeout);

    PIPSocketAddressAndPort baseAddress;
    m_natType = DoRFC3489Discovery(m_socket, m_serverAddress, baseAddress, m_externalAddress);
    return;
  }

  // get list of interfaces
  PList<PSTUNUDPSocket> sockets;
  PIPSocket::InterfaceTable interfaces;
  if (PIPSocket::GetInterfaceTable(interfaces)) {
    for (PINDEX i =0; i < interfaces.GetSize(); i++) {
      PIPSocket::Address binding = interfaces[i].GetAddress();
      if (!binding.IsLoopback() && (binding.GetVersion() == 4)) {
        PSTUNUDPSocket * socket = new PSTUNUDPSocket(eComponent_Unknown);
        if (m_singlePortRange.Listen(*socket, binding))
          sockets.Append(socket);
        else
          delete socket;
      }
    }
    if (interfaces.IsEmpty()) {
      PTRACE(1, "STUN\tNo interfaces available to find STUN server.");
      return;
    }
  }
  else {
    PSTUNUDPSocket * socket = new PSTUNUDPSocket(eComponent_Unknown);
    sockets.Append(socket);
    if (!m_singlePortRange.Listen(*socket))
      return;
  }

  // send binding request on all interfaces and wait for a reply
  PSTUNMessage requestI(PSTUNMessage::BindingRequest);
  requestI.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI;

  for (PINDEX retry = 0; retry < m_pollRetries; ++retry) {
    PSocket::SelectList selectList;
    for (PList<PSTUNUDPSocket>::iterator socket = sockets.begin(); socket != sockets.end(); ++socket) {
      socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
      if (requestI.Write(*socket))
        selectList += *socket;
    }

    if (selectList.IsEmpty())
      return; // Could not send on any interface!

    // wait for reply
    PChannel::Errors error = PIPSocket::Select(selectList, replyTimeout);
    if (error != PChannel::NoError) {
      PTRACE(1, "STUN\tError in select - " << PChannel::GetErrorText(error));
      return;
    }

    // take the first valid one
    for (PSocket::SelectList::iterator it = selectList.begin(); it != selectList.end(); ++it) {
      PSTUNUDPSocket & udp = dynamic_cast<PSTUNUDPSocket &>(*it);
      if (responseI.Read(udp) && responseI.Validate(requestI)) {
        delete m_socket;
        m_socket = &udp;
        break;
      }
    }

    if (m_socket != NULL) {
      sockets.AllowDeleteObjects(false);
      sockets.Remove(m_socket);
      sockets.AllowDeleteObjects(true);
      break;
    }
  }

  // complete discovery
  m_socket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
  m_socket->SetReadTimeout(replyTimeout);
  PIPSocketAddressAndPort ap;
  m_socket->GetBaseAddress(ap);
  m_interface = ap.GetAddress();
  m_natType = FinishRFC3489Discovery(responseI, m_socket, m_externalAddress);
}


bool PSTUNClient::CreateSocket(PUDPSocket * & udpSocket, const PIPSocket::Address & binding, WORD port, PObject * context, Component component)
{
  PWaitAndSignal m(m_mutex);

  if (!binding.IsAny() && binding != m_interface)
    return false;

  if (!PNatMethod::CreateSocket(udpSocket, binding, port, context, component))
    return false;

  PSTUNUDPSocket * stunSocket = dynamic_cast<PSTUNUDPSocket *>(udpSocket);
  if (stunSocket->OpenSTUN(*this))
    return true;

  delete udpSocket;
  udpSocket = NULL;
  return false;
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
                                   const PIPSocket::Address & binding,
                                   PObject *)
{
  PWaitAndSignal m(m_mutex);

  if (!binding.IsAny() && binding != m_interface)
    return false;

  socket1 = NULL;
  socket2 = NULL;

  switch (GetNatType(false)) {
    case OpenNat :
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (!m_pairedPortRange.IsValid()) {
        PTRACE(1, "STUN\tInvalid local UDP port range " << m_pairedPortRange);
        return false;
      }
      break;

    default :
      PTRACE(1, "STUN\tCannot create socket pair using NAT type " << GetNatTypeName());
      return false;
  }

  PPtrVector<SocketInfo> socketInfo;

  // send binding requests until we get a pair of adjacent sockets
  for (PINDEX socketCount = 0; socketCount < m_numSocketsForPairing; ++socketCount)  {
    // always ensure we have two sockets
    while (socketInfo.size() < 2) {

      // create a socket
      size_t idx = socketInfo.size();
      socketInfo.push_back(new SocketInfo());
      SocketInfo & info = *socketInfo[idx];
      info.m_stunSocket = new PSTUNUDPSocket(eComponent_RTP);
      if (!m_pairedPortRange.Listen(*info.m_stunSocket, m_interface)) {
        PTRACE(1, "STUN\tUnable to open socket to " << *this);
        return false;
      }

      // if necessary, send a binding request
      if (GetNatType(false) == OpenNat) {
        info.m_ready = true;
        socketInfo[idx]->m_stunSocket->GetBaseAddress(socketInfo[idx]->m_stunSocket->m_serverReflexiveAddress);
      }
      else {
        info.m_stunSocket->PUDPSocket::InternalSetSendAddress(m_serverAddress);
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

typedef PTURNClient PNatMethod_TURN;
PCREATE_NAT_PLUGIN(TURN, "TURN Server");

PTURNClient::PTURNClient(unsigned priority)
  : PSTUNClient(priority)
{
}


const char * PTURNClient::MethodName()
{
  return PPlugin_PNatMethod_TURN::ServiceName();
}


PCaselessString PTURNClient::GetMethodName() const
{
  return MethodName();
}


PNATUDPSocket * PTURNClient::InternalCreateSocket(Component component, PObject *)
{
  return new PTURNUDPSocket(component);
}


//////////////////////////////////////////////////////////////////////

PTURNUDPSocket::PTURNUDPSocket(PNatMethod::Component component)
  : PSTUNUDPSocket(component)
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


void PTURNUDPSocket::GetCandidateInfo(PNatCandidate & candidate)
{
  PNATUDPSocket::GetCandidateInfo(candidate);
  candidate.m_type = PNatCandidate::eType_Relay;
}


int PTURNUDPSocket::OpenTURN(PTURNClient & client)
{
  m_usingTURN = false;

  // can only use TURN for RTP
  if ((m_component != PNatMethod::eComponent_RTP) && (m_component != PNatMethod::eComponent_RTCP)) {
    PTRACE(2, "TURN\tUsing STUN for non RTP socket");
    return OpenSTUN(client) ? 0 : -1;
  }

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
  switch (GetNatType(force)) {
    // types that do support RTP 
    case OpenNat:
    case SymmetricNat:
    case ConeNat:
    case RestrictedNat:
    case PortRestrictedNat:
      return RTPSupported;

    // types that do not support RTP
    default:
      return RTPUnknown;
  }
}

struct AllocateSocketFunctor {
  AllocateSocketFunctor(PTURNClient & client, BYTE component, const PIPSocket::Address & iface, PIPSocket::PortRange & portRange)
    : m_client(client)
    , m_component(component)
    , m_interface(iface)
    , m_turnSocket(NULL)
    , m_portRange(portRange)
    , m_status(true)
  { }

  void operator()(PThread & thread);
    
  PTURNClient & m_client;
  BYTE m_component;
  PIPSocket::Address m_interface;
  PTURNUDPSocket * m_turnSocket;
  PIPSocket::PortRange & m_portRange;
  bool m_status;
};

void AllocateSocketFunctor::operator () (PThread &)
{
  int retryCount = 3;
  m_status = true;
  while (retryCount > 0) {

    m_turnSocket = new PTURNUDPSocket(PNatMethod::eComponent_RTP);

    if (!m_portRange.Listen(*m_turnSocket, m_interface)) {
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

bool PTURNClient::CreateSocket(PUDPSocket * & socket, const PIPSocket::Address & binding, WORD port, PObject * context, Component component)
{
  if (component != PNatMethod::eComponent_RTP && component != PNatMethod::eComponent_RTCP)
    return PSTUNClient::CreateSocket(socket, binding, port, context, component);

  if (!binding.IsAny() && binding != m_interface)
    return false;
  
  socket = NULL;

  PIPSocket::PortRange * portRange;
  PIPSocket::PortRange localPortInfo(port);
  if (port != 0)
    portRange = &localPortInfo;
  else
    portRange = &m_singlePortRange;

  AllocateSocketFunctor op(*this, (BYTE)component, m_interface, *portRange);

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
                                   const PIPSocket::Address & binding,
                                   PObject *)
{
  if (!binding.IsAny() && binding != m_interface)
    return false;

  socket1 = NULL;
  socket2 = NULL;

  AllocateSocketFunctor op1(*this, PNatMethod::eComponent_RTP,   binding, m_pairedPortRange);
  AllocateSocketFunctor op2(*this, PNatMethod::eComponent_RTCP,  binding, m_pairedPortRange);
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


#endif // P_STUN

// End of File ////////////////////////////////////////////////////////////////
