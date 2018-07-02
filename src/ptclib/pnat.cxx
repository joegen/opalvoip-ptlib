
/*
 * pnat.cxx
 *
 * NAT Strategy support for Portable Windows Library.
 *
 *
 * Copyright (c) 2004 ISVO (Asia) Pte Ltd. All Rights Reserved.
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
 *
 * The Original Code is derived from and used in conjunction with the 
 * OpenH323 Project (www.openh323.org/)
 *
 * The Initial Developer of the Original Code is ISVO (Asia) Pte Ltd.
 *
 *
 * Contributor(s): ______________________________________.
 */

#include <ptlib.h>

#if P_NAT

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/pnat.h>

#include <ptclib/random.h>


#define PTraceModule() "NAT"


PNatMethods::PNatMethods(bool loadFromFactory, PPluginManager * pluginMgr)
{
  if (loadFromFactory)
    LoadAll(pluginMgr);
}


void PNatMethods::LoadAll(PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  PStringArray methods = pluginMgr->GetPluginsProviding(PPlugin_PNatMethod::ServiceType(), false);
  for (PINDEX i = 0; i < methods.GetSize(); ++i)
    Append(pluginMgr->CreatePlugin(methods[i], PPlugin_PNatMethod::ServiceType()));
}


PNatMethod * PNatMethods::GetMethod(const PIPSocket::Address & binding, PObject * userData)
{
  for (PNatMethods::iterator it = begin(); it != end(); ++it) {
    if (it->IsAvailable(binding, userData) && it->GetNatType() != PNatMethod::BlockedNat) {
      PTRACE(5, "Found method " << it->GetFriendlyName() << " on " << binding);
      return &*it;
    }
  }

  PTRACE(3, "No available methods on " << binding);
  return NULL;
}


PNatMethod * PNatMethods::GetMethodByName(const PString & name)
{
  for (iterator it = begin(); it != end(); ++it) {
    if (it->GetMethodName() == name)
      return &*it;
  }

  return NULL;
}


bool PNatMethods::RemoveMethod(const PString & meth)
{
  for (iterator it = begin(); it != end(); ++it) {
    if (it->GetMethodName() == meth) {
      erase(it);
      return true;
    }
  }

  return false;
}


bool PNatMethods::SetMethodPriority(const PString & name, unsigned priority)
{
  for (iterator it = begin(); it != end(); ++it) {
    if (it->GetMethodName() == name) {
      PNatMethod * natMethod = &*it;
      if (natMethod->m_priority == priority)
        return true;

      DisallowDeleteObjects();
      erase(it);
      AllowDeleteObjects();

      natMethod->m_priority = priority;
      Append(natMethod);
      return true;
    }
  }

  return false;
}


bool PNatMethods::IsLocalAddress(const PIPSocket::Address & ip) const
{
  /* Check if the remote address is a private IP, broadcast, or us */
  return ip.IsAny() || ip.IsBroadcast() || ip.IsRFC1918() || PIPSocket::IsLocalHost(ip);
}


void PNatMethods::SetPortRanges(WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax)
{
  for (iterator it = begin(); it != end(); ++it)
    it->SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


///////////////////////////////////////////////////////////////////////

const PTimeInterval & PNatMethod::GetDefaultMaxAge() { static const PTimeInterval age(0, 0, 1); return age; }

PNatMethod::PNatMethod(unsigned priority)
  : m_active(true)
  , m_natType(UnknownNat)
  , m_externalAddress(PIPSocket::GetInvalidAddress())
  , m_updateTime(0)
  , m_priority(priority)
{
}


PNatMethod::~PNatMethod()
{
}


PString PNatMethod::GetNatTypeString(NatTypes type)
{
  static const char * const Names[NumNatTypes] = {
    "Unknown NAT",
    "Open NAT",
    "Cone NAT",
    "Restricted NAT",
    "Port Restricted NAT",
    "Symmetric NAT",
    "Partially Blocked",
    "Blocked",
  };

  if (type < NumNatTypes)
    return Names[type];
  
  return psprintf("<NATType %u>", type);
}


PNatMethod * PNatMethod::Create(const PString & name, PPluginManager * pluginMgr)
{
  return PPluginManager::CreatePluginAs<PNatMethod>(pluginMgr, name, PPlugin_PNatMethod::ServiceType());
}


PString PNatMethod::GetFriendlyName() const
{
  PPluginServiceDescriptor * descriptor = PPluginFactory::CreateInstance("PNatMethod" + GetMethodName());
  return PAssertNULL(descriptor)->GetFriendlyName();
}


void PNatMethod::Activate(bool active)
{
  m_active = active;
}


bool PNatMethod::GetServerAddress(PIPSocket::Address & address, WORD & port) const
{
  PIPSocketAddressAndPort ap;
  if (!GetServerAddress(ap))
    return false;

  address = ap.GetAddress();
  port = ap.GetPort();
  return true;
}


bool PNatMethod::GetServerAddress(PIPSocketAddressAndPort & ap) const
{
  return ap.Parse(GetServer());
}


bool PNatMethod::SetServer(const PString &)
{
  return true;
}


void PNatMethod::SetCredentials(const PString &, const PString &, const PString &)
{
}


PNatMethod::NatTypes PNatMethod::GetNatType(const PTimeInterval & maxAge)
{
  PWaitAndSignal mutex(m_mutex);

  // Make sure we update server address as DNS pooling may have it change
  PIPAddressAndPort oldAP, newAP;
  GetServerAddress(oldAP); // Don't check return result here as may be first time

  if (!SetServer(GetServer()) || !GetServerAddress(newAP))
    return m_natType = UnknownNat;

  if (newAP == oldAP && m_updateTime.GetElapsed() < maxAge)
    return m_natType;

  InternalUpdate();
  m_updateTime.SetCurrentTime();
  return m_natType;
}


bool PNatMethod::GetExternalAddress(PIPSocket::Address & externalAddress, const PTimeInterval & maxAge)
{
  PWaitAndSignal mutex(m_mutex);

  if (GetNatType(maxAge) == UnknownNat)
    return false;

  return static_cast<const PNatMethod *>(this)->GetExternalAddress(externalAddress);
}


bool PNatMethod::GetExternalAddress(PIPSocket::Address & externalAddress) const
{
  PWaitAndSignal mutex(m_mutex);

  if (!m_externalAddress.IsValid())
    return false;

  externalAddress = m_externalAddress.GetAddress();
  return true;
}


bool PNatMethod::GetInterfaceAddress(PIPSocket::Address & internalAddress) const
{
  internalAddress = PIPSocket::GetInvalidAddress();
  return false;
}


PString PNatMethod::GetInterface() const
{
  PIPSocket::Address internalAddress;
  return GetInterfaceAddress(internalAddress) ? internalAddress.AsString(true) : PString::Empty();
}


bool PNatMethod::Open(const PIPSocket::Address &)
{
  return true;
}


bool PNatMethod::CreateSocket(PUDPSocket * & socket, const PIPSocket::Address & binding, WORD localPort, PObject * context, Component component)
{
  PWaitAndSignal m(m_mutex);

  socket = InternalCreateSocket(component, context);
  if (socket == NULL)
    return false;

  if (localPort != 0) {
    if (socket->Listen(binding, 5, localPort))
      return true;
  }
  else {
    if (m_singlePortRange.Listen(*socket, binding))
      return true;
  }

  delete socket;
  socket = NULL;
  return false;
}


PBoolean PNatMethod::CreateSocketPair(PUDPSocket * & socket1, PUDPSocket * & socket2, const PIPSocket::Address & binding, PObject * context)
{
  PWaitAndSignal mutex(m_mutex);

  if (m_pairedPortRange.IsValid()) {
    PTRACE(1, "Invalid local UDP port range " << m_pairedPortRange);
    return false;
  }

  PIPSocket * sockets[2];
  sockets[0] = socket1 = InternalCreateSocket(eComponent_RTP, context);
  sockets[1] = socket2 = InternalCreateSocket(eComponent_RTCP, context);

  if (socket1 == NULL || socket2 == NULL)
    return false;

  if (m_pairedPortRange.Listen(sockets, 2, binding))
    return true;

  delete socket1;
  socket1 = NULL;
  delete socket2;
  socket2 = NULL;
  return false;
}


bool PNatMethod::IsAvailable(const PIPSocket::Address &, PObject *)
{
  return m_active;
}


PNatMethod::RTPSupportTypes PNatMethod::GetRTPSupport(bool force)
{
  switch (GetNatType(force)) {
    // types that do support RTP 
    case OpenNat:
      return RTPSupported;

    // types that support RTP if media sent first
    case ConeNat:
    case RestrictedNat:
    case PortRestrictedNat:
    case SymmetricNat:
      return RTPIfSendMedia;

    // types that do not support RTP
    case BlockedNat:
      return RTPUnsupported;

    // types that have unknown RTP support
    default:
      return RTPUnknown;
  }
}


void PNatMethod::PrintOn(ostream & strm) const
{
  strm << GetFriendlyName() << (IsActive() ? " active" : " deactivated") << ',';

  PString server = GetServer();
  if (!server.IsEmpty())
    strm << ' ' << server;

  if (m_natType != UnknownNat) {
    strm << " replies " << GetNatTypeName();
    if (m_externalAddress.IsValid())
      strm << " with address " << m_externalAddress;
  }

  PIPSocket::Address iface;
  if (GetInterfaceAddress(iface))
    strm << " on interface " << iface;
}


PObject::Comparison PNatMethod::Compare(const PObject & obj) const
{
  const PNatMethod & other = dynamic_cast<const PNatMethod &>(obj);
  if (m_priority < other.m_priority)
    return LessThan;
  if (m_priority > other.m_priority)
    return GreaterThan;
  return EqualTo;
}


void PNatMethod::SetPortRanges(WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax) 
{
  PWaitAndSignal mutex(m_mutex);
  m_singlePortRange.Set(portBase, portMax);
  m_pairedPortRange.Set((portPairBase+1)&0xfffe, portPairMax);
}


////////////////////////////////////////////////////

PNATUDPSocket::PNATUDPSocket(PNatMethod::Component component)
  : m_component(component)
{
}


PString PNATUDPSocket::GetName() const
{
  PStringStream str;
  str << GetNatName() << ':';

  AddressAndPort ap;
  if (GetBaseAddress(ap))
    str << " base=" << ap;
  if (GetLocalAddress(ap))
    str << " local=" << ap;
  if (GetPeerAddress(ap))
    str << " peer= " << ap;

  return str;
}


void PNATUDPSocket::GetCandidateInfo(PNatCandidate & candidate)
{ 
  candidate.m_type = PNatCandidate::HostType;
  candidate.m_component = m_component;
  InternalGetBaseAddress(candidate.m_baseTransportAddress);
  InternalGetLocalAddress(candidate.m_localTransportAddress);
}


PString PNATUDPSocket::GetBaseAddress() const
{
  PIPSocketAddressAndPort ap;
  return GetBaseAddress(ap) ? ap.AsString() :PString::Empty();
}


bool PNATUDPSocket::GetBaseAddress(PIPSocketAddressAndPort & addrAndPort) const
{
  return const_cast<PNATUDPSocket *>(this)->InternalGetBaseAddress(addrAndPort);
}


bool PNATUDPSocket::InternalGetBaseAddress(PIPSocketAddressAndPort & addr)
{
  return PUDPSocket::InternalGetLocalAddress(addr);
}


////////////////////////////////////////////////////

PNatCandidate::PNatCandidate(Types type,
                             PNatMethod::Component component,
                             const char * foundation,
                             unsigned priority,
                             const char * protocol)
  : m_type(type)
  , m_component(component)
  , m_priority(priority)
  , m_foundation(foundation)
  , m_protocol(protocol)
  , m_networkCost(0)
  , m_networkId(0)
{
}


PObject::Comparison PNatCandidate::Compare(const PObject & obj) const
{
  const PNatCandidate & other = dynamic_cast<const PNatCandidate &>(obj);

  Comparison result;
  if ((result = Compare2(m_networkCost, other.m_networkCost)) == EqualTo &&
      (result = Compare2(m_priority, other.m_priority)) == EqualTo &&
      (result = Compare2(m_type, other.m_type)) == EqualTo &&
      (result = Compare2(m_component, other.m_component)) == EqualTo &&
      (result = m_protocol.Compare(other.m_protocol)) == EqualTo)
       result = m_foundation.Compare(other.m_foundation);

  return result;
}


void PNatCandidate::PrintOn(ostream & strm) const
{
  bool columns = strm.width() < 0;

  static const char * const TypeNames[NumTypes] = {
    "Host", "Server-Reflexive", "Peer-Reflexive", "Relay", "Final"
  };
  strm << left << setw(columns ? strlen(TypeNames[ServerReflexiveType]) : 0) << TypeNames[m_type] << ' ';

  static const unsigned AddrWidth = 21;
  switch (m_type) {
    case HostType :
      strm << right << setw(columns ? AddrWidth : 0) << m_baseTransportAddress;
      if (columns)
        strm << setw(AddrWidth+2) << ' ';
      break;
    case ServerReflexiveType :
    case PeerReflexiveType :
    case RelayType :
      strm << right << setw(columns ? AddrWidth : 0) << m_baseTransportAddress
           << '/' << left << setw(columns ? AddrWidth+1 : 0);
      if (m_localTransportAddress.IsValid())
        strm << m_localTransportAddress;
      else
        strm << "----";
      break;
    default:
      if (columns)
        strm << setw((AddrWidth+1)*2) << ' ';
      break;
  }

  if (columns || !m_protocol.IsEmpty())
    strm << right << setw(4) << m_protocol;

  strm << " component=" << m_component
       << " priority=" << left << setw(columns ? 10 : 0) <<  m_priority;

  if (columns || m_networkCost > 0 || m_networkId > 0)
    strm << " network-cost=" << setw(columns ? 3 : 0) << m_networkCost
         << " network-id=" << setw(columns ? 2 : 0) << m_networkId;

  if (columns || !m_foundation.IsEmpty())
    strm << " foundation=\"" << left << setw(columns ? 15 : 0) << (m_foundation+'"');
}


//////////////////////////////////////////////////////////////////////////
//
// Fixed, preconfigured, NAT support
//

PCREATE_NAT_PLUGIN(Fixed, "Fixed Router");

PNatMethod_Fixed::PNatMethod_Fixed(unsigned priority)
  : PNatMethod(priority)
  , m_interfaceAddress(PIPSocket::GetInvalidAddress())
{
}


const char * PNatMethod_Fixed::MethodName()
{
  return PPlugin_PNatMethod_Fixed::ServiceName();
}


PCaselessString PNatMethod_Fixed::GetMethodName() const
{
  return MethodName();
}


PString PNatMethod_Fixed::GetServer() const
{
  PWaitAndSignal mutex(m_mutex);

  if (m_externalAddress.IsValid())
    return PSTRSTRM(m_externalAddress << '/' << m_natType);

  return PString::Empty();
}


bool PNatMethod_Fixed::SetServer(const PString & str)
{
  PWaitAndSignal mutex(m_mutex);

  if (str.IsEmpty()) {
    m_natType = OpenNat;
    m_externalAddress = PIPSocket::GetInvalidAddress();
    return true;
  }

  PINDEX pos = str.FindLast('/');
  if (pos == P_MAX_INDEX) {
    m_natType = ConeNat;
    return m_externalAddress.Parse(str, 1);
  }

  int newType = str.Mid(pos+1).AsInteger();
  if (newType < 0 || newType >= NumNatTypes)
    return false;

  m_natType = (NatTypes)newType;
  return m_externalAddress.Parse(str.Left(pos), 1);
}


PNATUDPSocket * PNatMethod_Fixed::InternalCreateSocket(Component component, PObject *)
{
  return new Socket(component, m_externalAddress.GetAddress());
}


PNatMethod_Fixed::Socket::Socket(PNatMethod::Component component, const PIPSocket::Address & externalAddress)
  : PNATUDPSocket(component)
  , m_externalAddress(externalAddress)
{
}


bool PNatMethod_Fixed::Socket::InternalGetLocalAddress(PIPSocketAddressAndPort & addr)
{
  if (!PNATUDPSocket::InternalGetLocalAddress(addr))
    return false;

  if (m_externalAddress.IsValid())
    addr.SetAddress(m_externalAddress);

  return true;
}


void PNatMethod_Fixed::InternalUpdate()
{
}


bool PNatMethod_Fixed::GetInterfaceAddress(PIPSocket::Address & addr) const
{
  PWaitAndSignal mutex(m_mutex);

  addr = m_interfaceAddress;
  return true;
}


bool PNatMethod_Fixed::Open(const PIPSocket::Address & addr)
{
  PWaitAndSignal mutex(m_mutex);

  m_interfaceAddress = addr;
  return m_interfaceAddress.IsValid();
}


bool PNatMethod_Fixed::IsAvailable(const PIPSocket::Address & binding, PObject * context)
{
  PWaitAndSignal mutex(m_mutex);

  return PNatMethod::IsAvailable(binding, context) &&
         m_externalAddress.IsValid() &&
         (binding.IsAny() || m_interfaceAddress.IsAny() || binding == m_interfaceAddress);
}


#endif // P_NAT
