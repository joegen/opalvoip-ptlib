
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
 *
 * $Revision$
 * $Author$
 * $Date$
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

  PTRACE(5, "No available methods on " << binding);
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

  if ((PTime() - m_updateTime) > maxAge) {
    InternalUpdate();
    m_updateTime.SetCurrentTime();
  }

  return m_natType;
}


bool PNatMethod::GetExternalAddress(PIPSocket::Address & externalAddress, const PTimeInterval & maxAge)
{
  PWaitAndSignal mutex(m_mutex);

  if ((PTime() - m_updateTime) > maxAge) {
    InternalUpdate();
    m_updateTime.SetCurrentTime();
  }

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


void PNatMethod::PrintOn(ostream & strm) const
{
  strm << GetFriendlyName() << (IsActive() ? " active" : " deactivated");
  PString server = GetServer();
  if (!server.IsEmpty())
    strm << " server " << server;
  if (m_natType != UnknownNat) {
    strm << " replies " << GetNatTypeName();
    if (m_externalAddress.IsValid())
      strm << " with address " << m_externalAddress;
  }
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

PNatCandidate::PNatCandidate(Types type, PNatMethod::Component component, const char * foundation)
  : m_type(type)
  , m_component(component)
  , m_priority(0)
  , m_foundation(foundation)
  , m_protocol("udp")
{
}


void PNatCandidate::PrintOn(ostream & strm) const
{
  switch (m_type) {
    case HostType :
      strm << "Host " << m_baseTransportAddress;
      break;
    case ServerReflexiveType :
      strm << "Server Reflexive " << m_baseTransportAddress << '/' << m_localTransportAddress;
      break;
    case PeerReflexiveType :
      strm << "Peer Reflexive " << m_baseTransportAddress << '/' << m_localTransportAddress;
      break;
    case RelayType :
      strm << "Relay " << m_baseTransportAddress << '/' << m_localTransportAddress;
      break;
    default:
      strm << "Unknown";
      break;
  }
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
    m_natType = SymmetricNat;
    return m_externalAddress.Parse(str);
  }

  int newType = str.Mid(pos+1).AsInteger();
  if (newType < 0 || newType >= NumNatTypes)
    return false;

  m_natType = (NatTypes)newType;
  return m_externalAddress.Parse(str.Left(pos));
}


PNATUDPSocket * PNatMethod_Fixed::InternalCreateSocket(Component component, PObject *)
{
  return new PNATUDPSocket(component);
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
         (binding.IsAny() || binding == m_interfaceAddress);
}


#endif // P_NAT
