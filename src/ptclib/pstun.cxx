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
#include <ptclib/random.h>
#include <ptclib/cypher.h>

#define new PNEW


// Sample server is at larry.gloo.net

#define DEFAULT_REPLY_TIMEOUT 800
#define DEFAULT_POLL_RETRIES  3
#define DEFAULT_NUM_SOCKETS_FOR_PAIRING 4

#define RFC5389_MAGIC_COOKIE  0x2112A442


typedef PSTUNClient PNatMethod_STUN;
PCREATE_NAT_PLUGIN(STUN);


#define IS_REQUEST(msg_type)       (((msg_type) & 0x0110) == 0x0000)
#define IS_INDICATION(msg_type)    (((msg_type) & 0x0110) == 0x0010)
#define IS_SUCCESS_RESP(msg_type)  (((msg_type) & 0x0110) == 0x0100)
#define IS_ERR_RESP(msg_type)      (((msg_type) & 0x0110) == 0x0110)

///////////////////////////////////////////////////////////////////////

PString PSTUN::GetNatTypeString(NatTypes type)
{
  static const char * const Names[NumNatTypes] = {
    "Unknown NAT",
    "Open NAT",
    "Cone NAT",
    "Restricted NAT",
    "Port Restricted NAT",
    "Symmetric NAT",
    "Symmetric Firewall",
    "Blocked",
    "Partially Blocked"
  };

  if (type < NumNatTypes)
    return Names[type];
  
  return psprintf("<NATType %u>", type);
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
    *(PUInt32b *)(&(hdr->transactionId)) = RFC5389_MAGIC_COOKIE;

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
  m_isRFC5389 = *(PUInt32b *)(&(header->transactionId)) == RFC5389_MAGIC_COOKIE;
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
  int length = ((PSTUNMessageHeader *)theArray)->msgLength;
  PSTUNAttribute * attrib = GetFirstAttribute();
  while (length > 0) {
    if (attrib->type == type)
      return attrib;

    length -= CalcPaddedAttributeLength(attrib->length);
    attrib = attrib->GetNext();
  }
  return NULL;
}

bool PSTUNMessage::Read(PUDPSocket & socket)
{
  if (!socket.Read(GetPointer(1000), 1000)) {
    PTRACE(3, "STUNSRVR\tRead error " << socket.GetErrorCode(PChannel::LastReadError) << " - " << socket.GetErrorText(PChannel::LastReadError));
    return false;
  }

  socket.GetLastReceiveAddress(m_sourceAddressAndPort);

  SetSize(socket.GetLastReadCount());
  return true;
}
  
bool PSTUNMessage::Write(PUDPSocket & socket) const
{
  int len = sizeof(PSTUNMessageHeader) + ((PSTUNMessageHeader *)theArray)->msgLength;
  return socket.Write(theArray, len);
}

bool PSTUNMessage::Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries)
{
  for (PINDEX retry = 0; retry < pollRetries; retry++) {
    if (!request.Write(socket))
      break;

    if (Read(socket) && Validate(request))
      return true;
  }

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
  // calculate hash up to, but not including, MESSAGE_INTEGRITY attribute
  int lengthWithoutMI = (BYTE *)mi - (BYTE *)theArray;

  // calculate message integrity
  PHMAC_SHA1 hmac(credentialsHash, credentialsHashLen);
  PHMAC_SHA1::Result result;
  hmac.Process((BYTE *)theArray, lengthWithoutMI, result);

  // copy the hash to the returned buffer
  memcpy(checkHmac, result.GetPointer(), 20);
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
  if (type == XOR_MAPPED_ADDRESS) {
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

PSTUNClient::PSTUNClient()
  : serverPort(DefaultPort),
    replyTimeout(DEFAULT_REPLY_TIMEOUT),
    pollRetries(DEFAULT_POLL_RETRIES),
    numSocketsForPairing(DEFAULT_NUM_SOCKETS_FOR_PAIRING),
    natType(UnknownNat),
    cachedExternalAddress(0),
    timeAddressObtained(0)
{
}

PSTUNClient::PSTUNClient(const PString & server,
                         WORD portBase, WORD portMax,
                         WORD portPairBase, WORD portPairMax)
  : serverPort(DefaultPort),
    replyTimeout(DEFAULT_REPLY_TIMEOUT),
    pollRetries(DEFAULT_POLL_RETRIES),
    numSocketsForPairing(DEFAULT_NUM_SOCKETS_FOR_PAIRING),
    natType(UnknownNat),
    cachedExternalAddress(0),
    timeAddressObtained(0)
{
  SetServer(server);
  SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


PSTUNClient::PSTUNClient(const PIPSocket::Address & address, WORD port,
                         WORD portBase, WORD portMax,
                         WORD portPairBase, WORD portPairMax)
  : serverHost(address.AsString()),
    serverPort(port),
    replyTimeout(DEFAULT_REPLY_TIMEOUT),
    pollRetries(DEFAULT_POLL_RETRIES),
    numSocketsForPairing(DEFAULT_NUM_SOCKETS_FOR_PAIRING),
    natType(UnknownNat),
    cachedExternalAddress(0),
    timeAddressObtained(0)
{
  SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


void PSTUNClient::Initialise(const PString & server,
                             WORD portBase, WORD portMax,
                             WORD portPairBase, WORD portPairMax)
{
  SetServer(server);
  SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


bool PSTUNClient::GetServerAddress(PIPSocket::Address & address, WORD & port) const
{
  if (serverPort == 0)
    return false;

  port = serverPort;

  if (cachedServerAddress.IsValid()) {
    address = cachedServerAddress;
    return true;
  }

  return PIPSocket::GetHostAddress(serverHost, address);
}


PBoolean PSTUNClient::SetServer(const PString & server)
{
  PString host;
  WORD port = serverPort;

  PINDEX colon = server.Find(':');
  if (colon == P_MAX_INDEX)
    host = server;
  else {
    host = server.Left(colon);
    PString service = server.Mid(colon+1);
    if ((port = PIPSocket::GetPortByService("udp", service)) == 0) {
      PTRACE(2, "STUN\tCould not find service \"" << service << "\".");
      return false;
    }
  }

  if (host.IsEmpty() || port == 0)
    return false;

  if (serverHost == host && serverPort == port)
    return true;

  serverHost = host;
  serverPort = port;
  InvalidateCache();
  return true;
}


PBoolean PSTUNClient::SetServer(const PIPSocket::Address & address, WORD port)
{
  if (!address.IsValid() || port == 0)
    return false;

  serverHost = address.AsString();
  cachedServerAddress = address;
  serverPort = port;
  return true;
}

bool PSTUNClient::OpenSocket(PUDPSocket & socket, PortInfo & portInfo, const PIPSocket::Address & binding)
{
  if (serverPort == 0) {
    PTRACE(1, "STUN\tServer port not set.");
    return false;
  }

  if (!PIPSocket::GetHostAddress(serverHost, cachedServerAddress) || !cachedServerAddress.IsValid()) {
    PTRACE(2, "STUN\tCould not find host \"" << serverHost << "\".");
    return false;
  }

  PWaitAndSignal mutex(portInfo.mutex);

  PTRACE(3, "STUN\tUsing ports " << portInfo.basePort << " through " << portInfo.maxPort);

  WORD startPort = portInfo.currentPort;

  do {
    portInfo.currentPort++;
    if (portInfo.currentPort > portInfo.maxPort)
      portInfo.currentPort = portInfo.basePort;

    if (socket.Listen(binding, 1, portInfo.currentPort)) {
      socket.SetSendAddress(cachedServerAddress, serverPort);
      socket.SetReadTimeout(replyTimeout);
      PTRACE(3, "STUN\tListen succeeded on " << binding << ":" << portInfo.currentPort);
      return true;
    }

    PTRACE(3, "STUN\tListen failed on " << binding << ":" << portInfo.currentPort);

  } while (portInfo.currentPort != startPort);

  PTRACE(1, "STUN\tFailed to bind to local UDP port in range "
         << portInfo.currentPort << '-' << portInfo.maxPort);
  return false;
}


PSTUNClient::NatTypes PSTUNClient::GetNatType(PBoolean force)
{
  if (!force && natType != UnknownNat)
    return natType;

  PList<PUDPSocket> sockets;

  PIPSocket::InterfaceTable interfaces;
  if (PIPSocket::GetInterfaceTable(interfaces)) {
    for (PINDEX i =0; i < interfaces.GetSize(); i++) {
      PIPSocket::Address binding = interfaces[i].GetAddress();
      if (!binding.IsLoopback() && (binding.GetVersion() == 4)) {
        PUDPSocket * socket = new PUDPSocket;
        if (OpenSocket(*socket, singlePortInfo, binding))
          sockets.Append(socket);
        else
          delete socket;
      }
    }
    if (interfaces.IsEmpty()) {
      PTRACE(1, "STUN\tNo interfaces available to find STUN server.");
      return natType = UnknownNat;
    }
  }
  else {
    PUDPSocket * socket = new PUDPSocket;
    sockets.Append(socket);
    if (!OpenSocket(*socket, singlePortInfo, PIPSocket::GetDefaultIpAny()))
      return natType = UnknownNat;
  }

  // RFC3489 discovery

  /* test I - the client sends a STUN Binding Request to a server, without
     any flags set in the CHANGE-REQUEST attribute, and without the
     RESPONSE-ADDRESS attribute. This causes the server to send the response
     back to the address and port that the request came from. */
  PSTUNMessage requestI(PSTUNMessage::BindingRequest);
  requestI.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI;

  PUDPSocket * replySocket = NULL;

  for (PINDEX retry = 0; retry < pollRetries; ++retry) {
    PSocket::SelectList selectList;
    for (PList<PUDPSocket>::iterator socket = sockets.begin(); socket != sockets.end(); ++socket) {
      if (requestI.Write(*socket))
        selectList += *socket;
      else {
        PTRACE(1, "STUN\tError writing to " << *this << " - " << socket->GetErrorText(PChannel::LastWriteError));
      }
    }

    if (selectList.IsEmpty())
      return natType = UnknownNat; // Could not send on any interface!

    PChannel::Errors error = PIPSocket::Select(selectList, replyTimeout);
    if (error != PChannel::NoError) {
      PTRACE(1, "STUN\tError in select - " << PChannel::GetErrorText(error));
      return natType = UnknownNat;
    }

    if (!selectList.IsEmpty()) {
      PUDPSocket & udp = (PUDPSocket &)selectList.front();
      if (responseI.Read(udp) && responseI.Validate(requestI)) {
        replySocket = &udp;
        break;
      }
    }
  }

  if (replySocket == NULL) {
    PTRACE(3, "STUN\tNo response to test I " << *this);
    return natType = BlockedNat; // No response usually means blocked
  }

  replySocket->GetLocalAddress(interfaceAddress);

  // check if server returned "420 Unknown Attribute" - that probably means it cannot do CHANGE_REQUEST even with no changes
  bool canDoChangeRequest = true;

  PSTUNErrorCode * errorAttribute = (PSTUNErrorCode *)responseI.FindAttribute(PSTUNAttribute::ERROR_CODE);
  if (errorAttribute != NULL) {
    bool ok = false;
    if (errorAttribute->GetErrorCode() == 420) {
      // try again without CHANGE request
      PSTUNMessage request(PSTUNMessage::BindingRequest);
      ok = responseI.Poll(*replySocket, request, pollRetries);
      if (ok) { 
        errorAttribute = (PSTUNErrorCode *)responseI.FindAttribute(PSTUNAttribute::ERROR_CODE);
        ok = errorAttribute == NULL;
        canDoChangeRequest = false;
      }
    }
    if (!ok) {
      PTRACE(2, "STUN\tSTUN server returned unexpected error " << errorAttribute->GetErrorCode() << ", reason = '" << errorAttribute->GetReason() << "'");
      return natType = BlockedNat;
    }
  }

  PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << *this);
      return natType = UnknownNat; // Protocol error
    }
  }

  PIPSocket::Address mappedAddressI = mappedAddress->GetIP();
  WORD mappedPortI = mappedAddress->GetPort();
  cachedExternalAddress = mappedAddress->GetIP();

  bool notNAT = (replySocket->GetPort() == mappedPortI) && PIPSocket::IsLocalHost(mappedAddressI);

  // can only guess based on a single sample
  if (!canDoChangeRequest) {
    natType = notNAT ? OpenNat : SymmetricNat;
    PTRACE(2, "STUN\tSTUN server has only one address - best guess is that NAT is " << GetNatTypeString(natType));
    return natType;
  }

  PTRACE(2, "STUN\tTest I response received - sending test II (change port and address)");

  /* Test II - the client sends a Binding Request with both the "change IP"
     and "change port" flags from the CHANGE-REQUEST attribute set. */
  PSTUNMessage requestII(PSTUNMessage::BindingRequest);
  requestII.AddAttribute(PSTUNChangeRequest(true, true));
  PSTUNMessage responseII;
  bool testII = responseII.Poll(*replySocket, requestII, pollRetries);

  PTRACE(2, "STUN\tTest II response " << (testII ? "" : "not ") << "received");

  if (notNAT) {
    natType = (testII ? OpenNat : SymmetricFirewall);
    // Is not NAT or symmetric firewall
    PTRACE(2, "STUN\tTest I and II indicate nat is " << GetNatTypeString(natType));
    return natType;
  }

  if (testII)
    return natType = ConeNat;

  PSTUNAddressAttribute * changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::CHANGED_ADDRESS);
  if (changedAddress == NULL) {
    changedAddress = (PSTUNAddressAttribute *)responseI.FindAttribute(PSTUNAttribute::OTHER_ADDRESS);
    if (changedAddress == NULL) {
      PTRACE(2, "STUN\tTest II response indicates no alternate address in use - testing finished");
      return natType = UnknownNat; // Protocol error
    }
  }

  PTRACE(2, "STUN\tSending test I to alternate server");

  // Send test I to another server, to see if restricted or symmetric
  PIPSocket::Address secondaryServer = changedAddress->GetIP();
  WORD secondaryPort = changedAddress->GetPort();
  replySocket->SetSendAddress(secondaryServer, secondaryPort);
  PSTUNMessage requestI2(PSTUNMessage::BindingRequest);
  requestI2.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI2;
  if (!responseI2.Poll(*replySocket, requestI2, pollRetries)) {
    PTRACE(2, "STUN\tPoll of secondary server " << secondaryServer << ':' << secondaryPort
           << " failed, NAT partially blocked by firewall rules.");
    return natType = PartialBlockedNat;
  }

  mappedAddress = (PSTUNAddressAttribute *)responseI2.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)responseI2.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << *this);
      return UnknownNat; // Protocol error
    }
  }

  if (mappedAddress->GetPort() != mappedPortI || mappedAddress->GetIP() != mappedAddressI)
    return natType = SymmetricNat;

  replySocket->SetSendAddress(cachedServerAddress, serverPort);
  PSTUNMessage requestIII(PSTUNMessage::BindingRequest);
  requestIII.SetAttribute(PSTUNChangeRequest(false, true));
  PSTUNMessage responseIII;

  return natType = (responseIII.Poll(*replySocket, requestIII, pollRetries) ? RestrictedNat : PortRestrictedNat);
}


PSTUNClient::RTPSupportTypes PSTUNClient::GetRTPSupport(PBoolean force)
{
  switch (GetNatType(force)) {
    // types that do support RTP 
    case OpenNat:
    case ConeNat:
      return RTPSupported;

    // types that support RTP if media sent first
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

PBoolean PSTUNClient::GetExternalAddress(PIPSocket::Address & externalAddress, const PTimeInterval & maxAge)
{
  if (cachedExternalAddress.IsValid() && (PTime() - timeAddressObtained < maxAge)) {
    externalAddress = cachedExternalAddress;
    return PTrue;
  }

  externalAddress = 0; // Set to invalid address

  PUDPSocket socket;
  if (!OpenSocket(socket, singlePortInfo, PIPSocket::GetDefaultIpAny()))
    return false;

  PSTUNMessage request(PSTUNMessage::BindingRequest);
  //request.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage response;
  if (!response.Poll(socket, request, pollRetries))
  {
    PTRACE(1, "STUN\t" << *this << " unexpectedly went offline.");
    return false;
  }

  PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL) {
      PTRACE(2, "STUN\tExpected mapped address attribute from " << *this);
      return false;
    }
  }
  
  externalAddress = cachedExternalAddress = mappedAddress->GetIP();
  timeAddressObtained = PTime();
  return true;
}


bool PSTUNClient::GetInterfaceAddress(PIPSocket::Address & internalAddress) const
{
  if (!interfaceAddress.IsValid())
    return false;

  internalAddress = interfaceAddress;
  return true;
}


void PSTUNClient::InvalidateCache()
{
  cachedServerAddress = 0;
  cachedExternalAddress = 0;
  interfaceAddress = 0;
  natType = UnknownNat;
}


PBoolean PSTUNClient::CreateSocket(PUDPSocket * & socket, const PIPSocket::Address & binding, WORD localPort)
{
  socket = NULL;
  GetNatType(PFalse);

  switch (GetNatType(PFalse)) {
    case OpenNat :
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (localPort == 0 && (singlePortInfo.basePort == 0 || singlePortInfo.basePort > singlePortInfo.maxPort))
      {
        PTRACE(1, "STUN\tInvalid local UDP port range "
               << singlePortInfo.currentPort << '-' << singlePortInfo.maxPort);
        return PFalse;
      }
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      PTRACE(1, "STUN\tCannot create socket using NAT type " << GetNatTypeName());
      return PFalse;
  }

  if (!IsAvailable(binding)) {
    PTRACE(1, "STUN\tCannot create socket using binding " << binding);
    return PFalse;
  }

  PSTUNUDPSocket * stunSocket = new PSTUNUDPSocket;

  PBoolean opened;
  if (localPort == 0)
    opened = OpenSocket(*stunSocket, singlePortInfo, interfaceAddress);
  else {
    PortInfo portInfo = localPort;
    opened = OpenSocket(*stunSocket, portInfo, interfaceAddress);
  }

  if (opened)
  {
    if (GetNatType(false) == OpenNat) {
      socket = stunSocket;
      return true;
    }

    PSTUNMessage request(PSTUNMessage::BindingRequest);
    //request.AddAttribute(PSTUNChangeRequest(false, false));
    PSTUNMessage response;

    if (response.Poll(*stunSocket, request, pollRetries))
    {
      PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
      if (mappedAddress == NULL)
        mappedAddress = (PSTUNAddressAttribute *)response.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
      if (mappedAddress != NULL) {
        stunSocket->externalIP = mappedAddress->GetIP();
        if (GetNatType(PFalse) != SymmetricNat)
          stunSocket->port = mappedAddress->GetPort();
        stunSocket->SetSendAddress(0, 0);
        stunSocket->SetReadTimeout(PMaxTimeInterval);
        socket = stunSocket;
        return true;
      }

      PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << *this);
    }
    else
      PTRACE(1, "STUN\t" << *this << " unexpectedly went offline.");
  }

  delete stunSocket;
  return false;
}


PBoolean PSTUNClient::CreateSocketPair(PUDPSocket * & socket1,
                                   PUDPSocket * & socket2,
                                   const PIPSocket::Address & binding)
{
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

  if (!IsAvailable(binding)) {
    PTRACE(1, "STUN\tCannot create socket using binding " << binding);
    return PFalse;
  }

  PINDEX i;

  PArray<PSTUNUDPSocket> stunSocket;
  PArray<PSTUNMessage> request;
  PArray<PSTUNMessage> response;

  for (i = 0; i < numSocketsForPairing; i++)
  {
    PINDEX idx = stunSocket.Append(new PSTUNUDPSocket);
    if (!OpenSocket(stunSocket[idx], pairedPortInfo, interfaceAddress)) {
      PTRACE(1, "STUN\tUnable to open socket to " << *this);
      return false;
    }

    if (GetNatType(false) != OpenNat) {
      idx = request.Append(new PSTUNMessage(PSTUNMessage::BindingRequest));
      response.Append(new PSTUNMessage);
    }
  }

  if (GetNatType(false) != OpenNat) {
    for (i = 0; i < numSocketsForPairing; i++) {
      if (!response[i].Poll(stunSocket[i], request[i], pollRetries))
      {
        PTRACE(1, "STUN\t" << *this << " unexpectedly went offline.");
        return false;
      }
    }
  }

  for (i = 0; i < numSocketsForPairing; i++) {
    if (GetNatType(false) == OpenNat)
      stunSocket[i].GetLocalAddress(stunSocket[i].externalIP, stunSocket[i].port);
    else {
      PSTUNAddressAttribute * mappedAddress = (PSTUNAddressAttribute *)response[i].FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
      if (mappedAddress == NULL) {
        mappedAddress = (PSTUNAddressAttribute *)response[i].FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
        if (mappedAddress == NULL) {
          PTRACE(2, "STUN\tExpected (XOR)mapped address attribute from " << *this);
          return false;
        }
      }
      if (GetNatType(PFalse) != SymmetricNat)
        stunSocket[i].port = mappedAddress->GetPort();
      stunSocket[i].externalIP = mappedAddress->GetIP();
    }
  }

  for (i = 0; i < numSocketsForPairing; i++)
  {
    for (PINDEX j = 0; j < numSocketsForPairing; j++)
    {
      if ((stunSocket[i].port&1) == 0 && (stunSocket[i].port+1) == stunSocket[j].port) {
        stunSocket[i].SetSendAddress(0, 0);
        stunSocket[i].SetReadTimeout(PMaxTimeInterval);
        stunSocket[j].SetSendAddress(0, 0);
        stunSocket[j].SetReadTimeout(PMaxTimeInterval);
        socket1 = &stunSocket[i];
        socket2 = &stunSocket[j];
        stunSocket.DisallowDeleteObjects();
        stunSocket.Remove(socket1);
        stunSocket.Remove(socket2);
        stunSocket.AllowDeleteObjects();
        return true;
      }
    }
  }

  PTRACE(2, "STUN\tCould not get a pair of adjacent port numbers from NAT");
  return false;
}

bool PSTUNClient::IsAvailable(const PIPSocket::Address & binding) 
{ 
  switch (GetNatType(PFalse)) {
    case OpenNat :
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (pairedPortInfo.basePort == 0 || pairedPortInfo.basePort > pairedPortInfo.maxPort)
        return false;
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      return false;
  }

  return binding.IsAny() || binding == interfaceAddress || binding == cachedExternalAddress;
}

void PSTUNClient::SetCredentials(const PString & username, const PString & password, const PString & realm)
{
  PMessageDigest5::Result credentialsHash;
  PMessageDigest5::Encode(username + ":" + realm + ":" + password, credentialsHash);
  SetCredentialsHash(credentialsHash.GetPointer(), credentialsHash.GetSize());
}

void PSTUNClient::SetCredentialsHash(const BYTE * ptr, PINDEX len)
{
  m_credentialsHash.SetSize(len);
  memcpy(m_credentialsHash.GetPointer(), ptr, len);
}


////////////////////////////////////////////////////////////////

PSTUNUDPSocket::PSTUNUDPSocket()
  : externalIP(0)
{
}


PBoolean PSTUNUDPSocket::GetLocalAddress(Address & addr)
{
  if (!externalIP.IsValid())
    return PUDPSocket::GetLocalAddress(addr);

  addr = externalIP;
  return true;
}


PBoolean PSTUNUDPSocket::GetLocalAddress(Address & addr, WORD & port)
{
  if (!externalIP.IsValid())
    return PUDPSocket::GetLocalAddress(addr, port);

  addr = externalIP;
  port = GetPort();
  return true;
}

////////////////////////////////////////////////////////////////
   
void PTURNRequestedTransport::Initialise(BYTE protocol)
{
  m_protocol = protocol;
  type       = REQUESTED_TRANSPORT;
  length     = 4;
  m_rffu1    = 0;
  m_rffu2    = 0;
  m_rffu3    = 0;
}

////////////////////////////////////////////////////////////////

PTURNClient::PTURNClient(const PString & server)
  : PSTUNClient(server)
  , m_socket(NULL)
  , m_allocationNumber(0)
{
}


PTURNClient::PTURNClient(const PIPSocket::Address & serverAddress, WORD serverPort)
  : PSTUNClient(serverAddress, serverPort)
  , m_socket(NULL)
  , m_allocationNumber(0)
{
}

bool PTURNClient::Open(const PString & username, const PString & password, const PString & realm)
{
  // always close the socket first
  Close();

  // set new credentials
  SetCredentials(username, password, realm);

  // reset allocation number
  m_allocationNumber = 0;

  // resolve address
  if (!PIPSocket::GetHostAddress(serverHost, cachedServerAddress) || !cachedServerAddress.IsValid()) {
    PTRACE(3, "TURN\tCannot resolve TURN server address '" << serverHost << "'");
    return true;
  }

  // open a new socket
  m_socket = new PUDPSocket();
  if (!m_socket->Listen()) {
    PTRACE(3, "TURN\tListen failed");
    return false;
  }

  m_socket->SetSendAddress(cachedServerAddress, serverPort);

  // save parameters
  m_username = username;
  m_password = password;
  m_realm    = realm;

  // do a binding request, just to check if the TURN server is there
  PSTUNMessage request(PSTUNMessage::BindingRequest);
  PSTUNMessage response;
  if (!response.Poll(*m_socket, request, pollRetries)) {
    PTRACE(2, "TURN\tServer did not respond");
    return false;
  }

  PSTUNErrorCode * errorAttribute = (PSTUNErrorCode *)response.FindAttribute(PSTUNAttribute::ERROR_CODE);

  if (response.GetType() != PSTUNMessage::BindingResponse) {
    if (errorAttribute == NULL)
      PTRACE(2, "TURN\tServer refused authenticated response with no error code");
    else
      PTRACE(2, "TURN\tServer refused authenticated response with error code " << errorAttribute->GetErrorCode() << " - " << errorAttribute->GetReason());
    return false;
  }

  return true;
}

bool PTURNClient::IsOpen() const
{
  return (m_socket != NULL) && m_socket->IsOpen();
}


int PTURNClient::CreateAllocation(bool evenPort)
{
  BYTE protocol = PTURNRequestedTransport::ProtocolUDP;

  if (!IsOpen()) {
    PTRACE(2, "TURN\tCannot do allocate request on unopened TURN client");
    return -1;
  }

  PSTUNMessage allocateRequest(PSTUNMessage::Allocate);
  allocateRequest.AddAttribute(PTURNRequestedTransport(PTURNRequestedTransport::ProtocolUDP));
  if (evenPort)
    allocateRequest.AddAttribute(PTURNEvenPort());

  PSTUNMessage allocateResponse;
  if (!MakeAuthenticatedRequest(allocateRequest, allocateResponse))
    return -1;

  Allocation allocation;
  allocation.m_protocol = protocol;

  PSTUNAddressAttribute * addrAttr = (PSTUNAddressAttribute *)allocateResponse.FindAttribute(PSTUNAttribute::XOR_RELAYED_ADDRESS);
  if (addrAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain XOR_RELAYED_ADDRESS");
    return -1;
  }
  addrAttr->GetIPAndPort(allocation.m_relayedTransportAddress);

  addrAttr = (PSTUNAddressAttribute *)allocateResponse.FindAttribute(PSTUNAttribute::XOR_MAPPED_ADDRESS);
  if (addrAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain XOR_MAPPED_ADDRESS");
    return -1;
  }
  addrAttr->GetIPAndPort(allocation.m_clientAddress);

  PTURNLifetime * lifetimeAttr = (PTURNLifetime *)allocateResponse.FindAttribute(PSTUNAttribute::LIFETIME);
  if (lifetimeAttr == NULL) {
    PTRACE(2, "TURN\tAllocate response did not contain LIFETIME");
    return -1;
  }
  allocation.m_lifeTime = lifetimeAttr->GetLifetime();

  PWaitAndSignal m(m_allocationMutex);
  int allocationNumber = m_allocationNumber++;
  m_allocations.insert(AllocationsMap::value_type(allocationNumber, allocation));

  PTRACE(2, "TURN\tAddress/port " << allocation.m_relayedTransportAddress << " allocated on server with lifetime " << allocation.m_lifeTime);

  return allocationNumber;
}


bool PTURNClient::DeleteAllocation(int allocationNumber)
{
  if (!IsOpen()) {
    PTRACE(2, "TURN\tCannot do delete request on unopened TURN client");
    return false;
  }

  PWaitAndSignal m(m_allocationMutex);
  AllocationsMap::iterator r = m_allocations.find(allocationNumber);
  if (r == m_allocations.end()) {
    PTRACE(2, "TURN\tCannot find allocation number " << allocationNumber);
    return false;
  }
  m_allocations.erase(r);

  PSTUNMessage request(PSTUNMessage::Refresh);
  request.AddAttribute(PTURNLifetime(0));

  PSTUNMessage response;
  MakeAuthenticatedRequest(request, response);
  return true;
}


bool PTURNClient::RefreshAllocation(int allocationNumber, DWORD lifetime)
{
  if (!IsOpen()) {
    PTRACE(2, "TURN\tCannot do refresh request on unopened TURN client");
    return false;
  }

  PWaitAndSignal m(m_allocationMutex);
  AllocationsMap::iterator r = m_allocations.find(allocationNumber);
  if (r == m_allocations.end()) {
    PTRACE(2, "TURN\tCannot find allocation number " << allocationNumber);
    return false;
  }

  PSTUNMessage request(PSTUNMessage::Refresh);
  if (lifetime > 0)
    request.AddAttribute(PTURNLifetime(lifetime));

  PSTUNMessage response;
  return MakeAuthenticatedRequest(request, response);
}


bool PTURNClient::MakeAuthenticatedRequest(PSTUNMessage & request, PSTUNMessage & response)
{
  // send request, which we expect to fail due to authorisation issues
  if (!response.Poll(*m_socket, request, pollRetries)) {
    PTRACE(2, "TURN\tServer did not respond");
    return false;
  }

  if (!IS_ERR_RESP(response.GetType())) {
    PTRACE(2, "TURN\tServer responded to unauthenticated request with unexpected response " << hex << response.GetType());
    return false;
  }

  PSTUNStringAttribute * realmAttr      = (PSTUNStringAttribute *)response.FindAttribute(PSTUNAttribute::REALM);
  PSTUNErrorCode       * errorAttribute = (PSTUNErrorCode *)response.FindAttribute(PSTUNAttribute::ERROR_CODE);
  PSTUNStringAttribute * nonceAttr      = (PSTUNStringAttribute *)response.FindAttribute(PSTUNAttribute::NONCE);

  if ((errorAttribute == NULL) || (errorAttribute->GetErrorCode() != 401) || (nonceAttr == NULL) || (realmAttr == NULL)) {
    PTRACE(2, "TURN\tServer refused unauthenticated request with insufficient information - unable to proceed");
    return false;
  }

  // realms must match
  if (realmAttr->GetString() != m_realm) {
    PTRACE(2, "TURN\tServer returned unknown realm '" << realmAttr->GetString() << "'");
    return false;
  }

  // save the nonce
  m_nonce = nonceAttr->GetString();

  PTRACE(2, "TURN\tServer requested authentication for correct realm");

  // now send real allocate request
  request.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::USERNAME, m_username));
  request.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::REALM,    m_realm));
  request.AddAttribute(PSTUNStringAttribute(PSTUNAttribute::NONCE,    m_nonce));
  request.InsertMessageIntegrity(m_credentialsHash.GetPointer(), m_credentialsHash.GetSize());

  // send request
  if (!response.Poll(*m_socket, request, pollRetries)) {
    PTRACE(2, "TURN\tServer failed to respond to authenticated request");
    return false;
  }

  // check response
  if (!IS_SUCCESS_RESP(response.GetType())) {
    PSTUNErrorCode * errorAttribute = (PSTUNErrorCode *)response.FindAttribute(PSTUNAttribute::ERROR_CODE);
    if (errorAttribute == NULL)
      PTRACE(2, "TURN\tServer refused authenticated response with no error code");
    else
      PTRACE(2, "TURN\tServer refused authenticated response with error code " << errorAttribute->GetErrorCode() << " - " << errorAttribute->GetReason());
    return false;
  }

  // integrity must match
  if (!response.CheckMessageIntegrity(m_credentialsHash.GetPointer(), m_credentialsHash.GetSize())) {
    PTRACE(2, "TURN\tServer response failed message integrity check");
    return false;
  }

  return true;
}


bool PTURNClient::Close()
{
  m_allocations.clear();
  if (m_socket != NULL) {
    delete m_socket;
    m_socket = NULL;
  }
  return true;
}


// End of File ////////////////////////////////////////////////////////////////
