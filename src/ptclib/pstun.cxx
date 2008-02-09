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

#define new PNEW


PCREATE_NAT_PLUGIN(STUN);

// Sample server is at larry.gloo.net

#define DEFAULT_REPLY_TIMEOUT 1000
#define DEFAULT_POLL_RETRIES  5
#define DEFAULT_NUM_SOCKETS_FOR_PAIRING 4


///////////////////////////////////////////////////////////////////////

PSTUNClient::PSTUNClient()
  : serverAddress(0),
    serverPort(DefaultPort),
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
  : serverAddress(0),
    serverPort(DefaultPort),
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
  : serverAddress(address),
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


PString PSTUNClient::GetServer() const
{
  PStringStream str;
  str << serverAddress << ':' << serverPort;
  return str;
}


void PSTUNClient::GetServer(PIPSocket::Address & address, WORD & port) const
{
  address = serverAddress;
  port = serverPort;
}


PBoolean PSTUNClient::SetServer(const PString & server)
{
  PINDEX colon = server.Find(':');
  if (colon == P_MAX_INDEX) {
    if (!PIPSocket::GetHostAddress(server, serverAddress))
      return PFalse;
  }
  else {
    if (!PIPSocket::GetHostAddress(server.Left(colon), serverAddress))
      return PFalse;
    serverPort = PIPSocket::GetPortByService("udp", server.Mid(colon+1));
  }

  return serverAddress.IsValid() && serverPort != 0;
}


PBoolean PSTUNClient::SetServer(const PIPSocket::Address & address, WORD port)
{
  serverAddress = address;
  serverPort = port;
  return serverAddress.IsValid() && serverPort != 0;
}

#pragma pack(1)

struct PSTUNAttribute
{
  enum Types {
    MAPPED_ADDRESS = 0x0001,
    RESPONSE_ADDRESS = 0x0002,
    CHANGE_REQUEST = 0x0003,
    SOURCE_ADDRESS = 0x0004,
    CHANGED_ADDRESS = 0x0005,
    USERNAME = 0x0006,
    PASSWORD = 0x0007,
    MESSAGE_INTEGRITY = 0x0008,
    ERROR_CODE = 0x0009,
    UNKNOWN_ATTRIBUTES = 0x000a,
    REFLECTED_FROM = 0x000b,
    MaxValidCode
  };
  
  PUInt16b type;
  PUInt16b length;
  
  PSTUNAttribute * GetNext() const { return (PSTUNAttribute *)(((const BYTE *)this)+length+4); }
};

class PSTUNAddressAttribute : public PSTUNAttribute
{
public:
  BYTE     pad;
  BYTE     family;
  PUInt16b port;
  BYTE     ip[4];

  PIPSocket::Address GetIP() const { return PIPSocket::Address(4, ip); }

protected:
  enum { SizeofAddressAttribute = sizeof(BYTE)+sizeof(BYTE)+sizeof(WORD)+sizeof(PIPSocket::Address) };
  void InitAddrAttr(Types newType)
  {
    type = (WORD)newType;
    length = SizeofAddressAttribute;
    pad = 0;
    family = 1;
  }
  bool IsValidAddrAttr(Types checkType) const
  {
    return type == checkType && length == SizeofAddressAttribute;
  }
};

class PSTUNMappedAddress : public PSTUNAddressAttribute
{
public:
  void Initialise() { InitAddrAttr(MAPPED_ADDRESS); }
  bool IsValid() const { return IsValidAddrAttr(MAPPED_ADDRESS); }
};

class PSTUNChangedAddress : public PSTUNAddressAttribute
{
public:
  void Initialise() { InitAddrAttr(CHANGED_ADDRESS); }
  bool IsValid() const { return IsValidAddrAttr(CHANGED_ADDRESS); }
};

class PSTUNChangeRequest : public PSTUNAttribute
{
public:
  BYTE flags[4];
  
  PSTUNChangeRequest() { }

  PSTUNChangeRequest(bool changeIP, bool changePort)
  {
    Initialise();
    SetChangeIP(changeIP);
    SetChangePort(changePort);
  }

  void Initialise()
  {
    type = CHANGE_REQUEST;
    length = sizeof(flags);
    memset(flags, 0, sizeof(flags));
  }
  bool IsValid() const { return type == CHANGE_REQUEST && length == sizeof(flags); }
  
  bool GetChangeIP() const { return (flags[3]&4) != 0; }
  void SetChangeIP(bool on) { if (on) flags[3] |= 4; else flags[3] &= ~4; }
  
  bool GetChangePort() const { return (flags[3]&2) != 0; }
  void SetChangePort(bool on) { if (on) flags[3] |= 2; else flags[3] &= ~2; }
};

class PSTUNMessageIntegrity : public PSTUNAttribute
{
public:
  BYTE hmac[20];
  
  void Initialise()
  {
    type = MESSAGE_INTEGRITY;
    length = sizeof(hmac);
    memset(hmac, 0, sizeof(hmac));
  }
  bool IsValid() const { return type == MESSAGE_INTEGRITY && length == sizeof(hmac); }
};

struct PSTUNMessageHeader
{
  PUInt16b       msgType;
  PUInt16b       msgLength;
  BYTE           transactionId[16];
};


#pragma pack()


class PSTUNMessage : public PBYTEArray
{
public:
  enum MsgType {
    BindingRequest  = 0x0001,
    BindingResponse = 0x0101,
    BindingError    = 0x0111,
      
    SharedSecretRequest  = 0x0002,
    SharedSecretResponse = 0x0102,
    SharedSecretError    = 0x0112,
  };
  
  PSTUNMessage()
  { }
  
  PSTUNMessage(MsgType newType, const BYTE * id = NULL)
    : PBYTEArray(sizeof(PSTUNMessageHeader))
  {
    SetType(newType, id);
  }

  void SetType(MsgType newType, const BYTE * id = NULL)
  {
    SetMinSize(sizeof(PSTUNMessageHeader));
    PSTUNMessageHeader * hdr = (PSTUNMessageHeader *)theArray;
    hdr->msgType = (WORD)newType;
    for (PINDEX i = 0; i < ((PINDEX)sizeof(hdr->transactionId)); i++)
      hdr->transactionId[i] = id != NULL ? id[i] : (BYTE)PRandom::Number();
  }

  const PSTUNMessageHeader * operator->() const { return (PSTUNMessageHeader *)theArray; }
  
  PSTUNAttribute * GetFirstAttribute() { 

    int length = ((PSTUNMessageHeader *)theArray)->msgLength;
    if (theArray == NULL || length < (int) sizeof(PSTUNMessageHeader))
      return NULL;

    PSTUNAttribute * attr = (PSTUNAttribute *)(theArray+sizeof(PSTUNMessageHeader)); 
    PSTUNAttribute * ptr = attr;

    if (attr->length > GetSize() || attr->type >= PSTUNAttribute::MaxValidCode)
      return NULL;

    while (ptr && (BYTE*) ptr < (BYTE*)(theArray+GetSize()) && length >= (int) ptr->length+4) {

      length -= ptr->length + 4;
      ptr = ptr->GetNext();
    }

    if (length != 0)
      return NULL;

    return attr; 
  }

  bool Validate()
  {
    int length = ((PSTUNMessageHeader *)theArray)->msgLength;
    PSTUNAttribute * attrib = GetFirstAttribute();
    while (attrib && length > 0) {
      length -= attrib->length + 4;
      attrib = attrib->GetNext();
    }

    return length == 0;  // Exactly correct length
  }

  void AddAttribute(const PSTUNAttribute & attribute)
  {
    PSTUNMessageHeader * hdr = (PSTUNMessageHeader *)theArray;
    int oldLength = hdr->msgLength;
    int attrSize = attribute.length + 4;
    int newLength = oldLength + attrSize;
    hdr->msgLength = (WORD)newLength;
    // hdr pointer may be invalidated by next statement
    SetMinSize(newLength+sizeof(PSTUNMessageHeader));
    memcpy(theArray+sizeof(PSTUNMessageHeader)+oldLength, &attribute, attrSize);
  }

  void SetAttribute(const PSTUNAttribute & attribute)
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
        return;
      }

      length -= attrib->length + 4;
      attrib = attrib->GetNext();
    }

    AddAttribute(attribute);
  }

  PSTUNAttribute * FindAttribute(PSTUNAttribute::Types type)
  {
    int length = ((PSTUNMessageHeader *)theArray)->msgLength;
    PSTUNAttribute * attrib = GetFirstAttribute();
    while (length > 0) {
      if (attrib->type == type)
        return attrib;

      length -= attrib->length + 4;
      attrib = attrib->GetNext();
    }
    return NULL;
  }


  bool Read(PUDPSocket & socket)
  {
    if (!socket.Read(GetPointer(1000), 1000))
      return false;
    SetSize(socket.GetLastReadCount());
    return true;
  }
  
  bool Write(PUDPSocket & socket) const
  {
    return socket.Write(theArray, ((PSTUNMessageHeader *)theArray)->msgLength+sizeof(PSTUNMessageHeader)) != PFalse;
  }

  bool Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries)
  {
    for (PINDEX retry = 0; retry < pollRetries; retry++) {
      if (!request.Write(socket))
        break;

      if (Read(socket) && Validate() &&
            memcmp(request->transactionId, (*this)->transactionId, sizeof(request->transactionId)) == 0)
        return true;
    }

    return false;
  }
};


bool PSTUNClient::OpenSocket(PUDPSocket & socket, PortInfo & portInfo, const PIPSocket::Address & binding) const
{
  PWaitAndSignal mutex(portInfo.mutex);

  WORD startPort = portInfo.currentPort;

  do {
    portInfo.currentPort++;
    if (portInfo.currentPort > portInfo.maxPort)
      portInfo.currentPort = portInfo.basePort;

    if (socket.Listen(binding, 1, portInfo.currentPort)) {
      socket.SetSendAddress(serverAddress, serverPort);
      socket.SetReadTimeout(replyTimeout);
      return true;
    }

  } while (portInfo.currentPort != startPort);

  PTRACE(1, "STUN\tFailed to bind to local UDP port in range "
         << portInfo.currentPort << '-' << portInfo.maxPort);
  return false;
}


PSTUNClient::NatTypes PSTUNClient::GetNatType(PBoolean force)
{
  if (!force && natType != UnknownNat)
    return natType;

  PUDPSocket socket;
  if (!OpenSocket(socket, singlePortInfo, PIPSocket::GetDefaultIpAny()))
    return natType = UnknownNat;

  // RFC3489 discovery

  /* test I - the client sends a STUN Binding Request to a server, without
     any flags set in the CHANGE-REQUEST attribute, and without the
     RESPONSE-ADDRESS attribute. This causes the server to send the response
     back to the address and port that the request came from. */
  PSTUNMessage requestI(PSTUNMessage::BindingRequest);
  requestI.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI;
  if (!responseI.Poll(socket, requestI, pollRetries)) {
    if (socket.GetErrorCode(PChannel::LastWriteError) != PChannel::NoError) {
      PTRACE(1, "STUN\tError writing to server " << serverAddress << ':' << serverPort << " - " << socket.GetErrorText(PChannel::LastWriteError));
      return natType = UnknownNat; // No response usually means blocked
    }

    PTRACE(3, "STUN\tNo response to server " << serverAddress << ':' << serverPort << " - " << socket.GetErrorText(PChannel::LastReadError));
    return natType = BlockedNat; // No response usually means blocked
  }

  PSTUNMappedAddress * mappedAddress = (PSTUNMappedAddress *)responseI.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    PTRACE(2, "STUN\tExpected mapped address attribute from server " << serverAddress << ':' << serverPort);
    return natType = UnknownNat; // Protocol error
  }

  PIPSocket::Address mappedAddressI = mappedAddress->GetIP();
  WORD mappedPortI = mappedAddress->port;
  bool notNAT = socket.GetPort() == mappedPortI && PIPSocket::IsLocalHost(mappedAddressI);

  /* Test II - the client sends a Binding Request with both the "change IP"
     and "change port" flags from the CHANGE-REQUEST attribute set. */
  PSTUNMessage requestII(PSTUNMessage::BindingRequest);
  requestII.AddAttribute(PSTUNChangeRequest(true, true));
  PSTUNMessage responseII;
  bool testII = responseII.Poll(socket, requestII, pollRetries);

  if (notNAT) {
    // Is not NAT or symmetric firewall
    return natType = (testII ? OpenNat : SymmetricFirewall);
  }

  if (testII)
    return natType = ConeNat;

  PSTUNChangedAddress * changedAddress = (PSTUNChangedAddress *)responseI.FindAttribute(PSTUNAttribute::CHANGED_ADDRESS);
  if (changedAddress == NULL)
    return natType = UnknownNat; // Protocol error

  // Send test I to another server, to see if restricted or symmetric
  PIPSocket::Address secondaryServer = changedAddress->GetIP();
  WORD secondaryPort = changedAddress->port;
  socket.SetSendAddress(secondaryServer, secondaryPort);
  PSTUNMessage requestI2(PSTUNMessage::BindingRequest);
  requestI2.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage responseI2;
  if (!responseI2.Poll(socket, requestI2, pollRetries)) {
    PTRACE(2, "STUN\tPoll of secondary server " << secondaryServer << ':' << secondaryPort
           << " failed, NAT partially blocked by firwall rules.");
    return natType = PartialBlockedNat;
  }

  mappedAddress = (PSTUNMappedAddress *)responseI2.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
  if (mappedAddress == NULL) {
    PTRACE(2, "STUN\tExpected mapped address attribute from server " << serverAddress << ':' << serverPort);
    return UnknownNat; // Protocol error
  }

  if (mappedAddress->port != mappedPortI || mappedAddress->GetIP() != mappedAddressI)
    return natType = SymmetricNat;

  socket.SetSendAddress(serverAddress, serverPort);
  PSTUNMessage requestIII(PSTUNMessage::BindingRequest);
  requestIII.SetAttribute(PSTUNChangeRequest(false, true));
  PSTUNMessage responseIII;
  return natType = (responseIII.Poll(socket, requestIII, pollRetries) ? RestrictedNat : PortRestrictedNat);
}


PString PSTUNClient::GetNatTypeString(NatTypes type)
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


PSTUNClient::RTPSupportTypes PSTUNClient::IsSupportingRTP(PBoolean force)
{
  switch (GetNatType(force)) {

    // types that do support RTP 
    case OpenNat:
    case ConeNat:
      return RTPOK;

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
    case UnknownNat:
    case PartialBlockedNat:
    default:
      break;
  }

  return RTPUnknown;
}

PBoolean PSTUNClient::GetExternalAddress(PIPSocket::Address & externalAddress,
                                     const PTimeInterval & maxAge)
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
  request.AddAttribute(PSTUNChangeRequest(false, false));
  PSTUNMessage response;
  if (!response.Poll(socket, request, pollRetries))
  {
    PTRACE(1, "STUN\tServer " << serverAddress << ':' << serverPort << " unexpectedly went offline.");
    return false;
  }

  PSTUNMappedAddress * mappedAddress = (PSTUNMappedAddress *)response.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
  if (mappedAddress == NULL)
  {
    PTRACE(2, "STUN\tExpected mapped address attribute from server " << serverAddress << ':' << serverPort);
    return false;
  }

  
  externalAddress = cachedExternalAddress = mappedAddress->GetIP();
  timeAddressObtained = PTime();
  return true;
}


void PSTUNClient::InvalidateExternalAddressCache() {
  cachedExternalAddress = 0;
  natType = UnknownNat;
}


PBoolean PSTUNClient::CreateSocket(PUDPSocket * & socket, const PIPSocket::Address & binding, WORD localPort)
{
  socket = NULL;

  switch (GetNatType(PFalse)) {
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (singlePortInfo.basePort == 0 || singlePortInfo.basePort > singlePortInfo.maxPort)
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

  PSTUNUDPSocket * stunSocket = new PSTUNUDPSocket;

  PBoolean opened;
  if (localPort == 0)
    opened = OpenSocket(*stunSocket, singlePortInfo, binding);
  else {
    PortInfo portInfo = localPort;
    opened = OpenSocket(*stunSocket, portInfo, binding);
  }

  if (opened)
  {
    PSTUNMessage request(PSTUNMessage::BindingRequest);
    request.AddAttribute(PSTUNChangeRequest(false, false));
    PSTUNMessage response;

    if (response.Poll(*stunSocket, request, pollRetries))
    {
      PSTUNMappedAddress * mappedAddress = (PSTUNMappedAddress *)response.FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
      if (mappedAddress != NULL)
      {
        stunSocket->externalIP = mappedAddress->GetIP();
        if (GetNatType(PFalse) != SymmetricNat)
          stunSocket->port = mappedAddress->port;
        stunSocket->SetSendAddress(0, 0);
        stunSocket->SetReadTimeout(PMaxTimeInterval);
        socket = stunSocket;
        return true;
      }

      PTRACE(2, "STUN\tExpected mapped address attribute from server " << serverAddress << ':' << serverPort);
    }
    else
      PTRACE(1, "STUN\tServer " << serverAddress << ':' << serverPort << " unexpectedly went offline.");
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

  PINDEX i;

  PArray<PSTUNUDPSocket> stunSocket;
  PArray<PSTUNMessage> request;
  PArray<PSTUNMessage> response;

  for (i = 0; i < numSocketsForPairing; i++)
  {
    PINDEX idx = stunSocket.Append(new PSTUNUDPSocket);
    if (!OpenSocket(stunSocket[idx], pairedPortInfo, binding)) {
      PTRACE(1, "STUN\tUnable to open socket to server " << serverAddress);
      return false;
    }

    idx = request.Append(new PSTUNMessage(PSTUNMessage::BindingRequest));
    request[idx].AddAttribute(PSTUNChangeRequest(false, false));

    response.Append(new PSTUNMessage);
  }

  for (i = 0; i < numSocketsForPairing; i++)
  {
    if (!response[i].Poll(stunSocket[i], request[i], pollRetries))
    {
      PTRACE(1, "STUN\tServer " << serverAddress << ':' << serverPort << " unexpectedly went offline.");
      return false;
    }
  }

  for (i = 0; i < numSocketsForPairing; i++)
  {
    PSTUNMappedAddress * mappedAddress = (PSTUNMappedAddress *)response[i].FindAttribute(PSTUNAttribute::MAPPED_ADDRESS);
    if (mappedAddress == NULL)
    {
      PTRACE(2, "STUN\tExpected mapped address attribute from server " << serverAddress << ':' << serverPort);
      return false;
    }
    if (GetNatType(PFalse) != SymmetricNat)
      stunSocket[i].port = mappedAddress->port;
    stunSocket[i].externalIP = mappedAddress->GetIP();
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

PBoolean PSTUNClient::IsAvailable() 
{ 

  switch (GetNatType(PFalse)) {
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (pairedPortInfo.basePort == 0 || pairedPortInfo.basePort > pairedPortInfo.maxPort)
         return PFalse;
      
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      return PFalse;
  }

  return PTrue; 
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


// End of File ////////////////////////////////////////////////////////////////
