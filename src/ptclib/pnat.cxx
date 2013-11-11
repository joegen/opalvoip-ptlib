
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


PNatMethod * PNatMethods::GetMethod(const PIPSocket::Address & binding)
{
  for (PNatMethods::iterator it = begin(); it != end(); ++it) {
    if (it->IsAvailable(binding) && it->GetNatType() != PNatMethod::BlockedNat) {
      PTRACE(5, "NAT\tFound method " << it->GetFriendlyName() << " on " << binding);
      return &*it;
    }
  }

  PTRACE(5, "NAT\tNo available methods on " << binding);
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
  PWaitAndSignal m(m_mutex);

  if ((PTime() - m_updateTime) > maxAge) {
    InternalUpdate();
    m_updateTime.SetCurrentTime();
  }

  return m_natType;
}


bool PNatMethod::GetExternalAddress(PIPSocket::Address & externalAddress, const PTimeInterval & maxAge)
{
  PWaitAndSignal m(m_mutex);

  if ((PTime() - m_updateTime) > maxAge) {
    InternalUpdate();
    m_updateTime.SetCurrentTime();
  }

  return static_cast<const PNatMethod *>(this)->GetExternalAddress(externalAddress);
}


bool PNatMethod::GetExternalAddress(PIPSocket::Address & externalAddress) const
{
  PWaitAndSignal m(m_mutex);

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


bool PNatMethod::CreateSocket(Component component, PUDPSocket * & socket, const PIPSocket::Address & binding, WORD localPort)
{
  PWaitAndSignal m(m_mutex);

  socket = new PNATUDPSocket(component);
  return socket->Listen(binding, 5, localPort);
}


bool PNatMethod::CreateSocketPair(PUDPSocket * & socket1, PUDPSocket * & socket2, const PIPSocket::Address & binding)
{
  PWaitAndSignal m(m_mutex);

  WORD localPort = m_pairedPortInfo.GetRandomPair();
  socket1 = new PNATUDPSocket(eComponent_RTP);
  socket2 = new PNATUDPSocket(eComponent_RTCP);
  return socket1->Listen(binding, 5, localPort) && socket2->Listen(binding, 5, localPort+1);
}


PBoolean PNatMethod::CreateSocketPair(PUDPSocket * & socket1,
                                      PUDPSocket * & socket2,
                                      const PIPSocket::Address & binding,
                                      void * /*userData*/)
{
  return CreateSocketPair(socket1, socket2, binding);
}


bool PNatMethod::IsAvailable(const PIPSocket::Address &)
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
  m_singlePortInfo.SetPorts(portBase, portMax);
  m_pairedPortInfo.SetPorts((portPairBase+1)&0xfffe, portPairMax);
}


void PNatMethod::PortInfo::SetPorts(WORD start, WORD end)
{
  PWaitAndSignal m(mutex);

  basePort = start;
  if (basePort > 0 && basePort < 1024)
    basePort = 1024;

  if (basePort == 0)
    maxPort = 0;
  else if (end == 0)
    maxPort = (WORD)PMIN(65535, basePort + 99);
  else if (end < basePort)
    maxPort = basePort;
  else
    maxPort = end;

  if (basePort == maxPort)
    currentPort = basePort;
  else
    currentPort = (WORD)PRandom::Number(basePort, maxPort-1);
}


WORD PNatMethod::PortInfo::GetNext(unsigned increment)
{
  PWaitAndSignal m(mutex);

  if (basePort == 0)
    return 0;

  WORD p = currentPort;

  currentPort = basePort + (((currentPort - basePort) + increment) % (maxPort - basePort));

  return p;
}


void PNatMethod::Activate(bool active)
{
  m_active = active;
}


void PNatMethod::SetAlternateAddresses(const PStringArray & /*addresses*/, void * /*userData*/)
{
}


WORD PNatMethod::PortInfo::GetRandomPair()
{
  static PRandom rand;
  WORD num = (WORD)rand.Generate(basePort-1, maxPort-2);
  if ((num&1) == 1)
    num++;  // Make sure the number is even
  return num;
}


////////////////////////////////////////////////////

PNATUDPSocket::PNATUDPSocket(PNatMethod::Component component)
  : m_component(component)
{
}


PNatCandidate PNATUDPSocket::GetCandidateInfo()
{ 
  PNatCandidate cand(PNatCandidate::eType_Host, m_component); 
  PUDPSocket::InternalGetLocalAddress(cand.m_baseAddress);
  PIPSocket::InternalGetLocalAddress(cand.m_baseAddress);
  return cand;
}


PString PNATUDPSocket::GetBaseAddress()
{
  PIPSocketAddressAndPort ap;
  if (!InternalGetBaseAddress(ap))
    return PString::Empty();
  else
    return ap.AsString();
}


bool PNATUDPSocket::GetBaseAddress(PIPSocketAddressAndPort & addrAndPort)
{
  return InternalGetBaseAddress(addrAndPort);
}


bool PNATUDPSocket::InternalGetBaseAddress(PIPSocketAddressAndPort & addr)
{
  return PUDPSocket::InternalGetLocalAddress(addr);
}


////////////////////////////////////////////////////

PNatCandidate::PNatCandidate()
  : m_type(eType_Unknown)
  , m_component(PNatMethod::eComponent_Unknown)
{
}


PNatCandidate::PNatCandidate(int type, PNatMethod::Component component)
  : m_type(type)
  , m_component(component)
{
}


PString PNatCandidate::AsString() const
{
  PStringStream strm;
  switch (m_type) {
    case eType_Host:
      strm << "Host " << m_baseAddress;
      break;
    case eType_ServerReflexive:
      strm << "ServerReflexive " << m_baseAddress << "/" << m_transport;
      break;
    case eType_PeerReflexive:
      strm << "PeerReflexive " << m_baseAddress << "/" << m_transport;
      break;
    case eType_Relay:
      strm << "Relay " << m_baseAddress << "/" << m_transport;
      break;
    default:
      strm << "Unknown";
      break;
  }
  return strm;
}


//////////////////////////////////////////////////////////////////////////
//
// Fixed, preconfigured, NAT support
//

PCREATE_NAT_PLUGIN(Fixed);

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
  if (m_externalAddress.IsValid())
    return PSTRSTRM(m_externalAddress << '/' << m_natType);

  return PString::Empty();
}


bool PNatMethod_Fixed::SetServer(const PString & str)
{
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


void PNatMethod_Fixed::InternalUpdate()
{
}


bool PNatMethod_Fixed::GetInterfaceAddress(PIPSocket::Address & addr) const
{
  addr = m_interfaceAddress;
  return true;
}


bool PNatMethod_Fixed::Open(const PIPSocket::Address & addr)
{
  m_interfaceAddress = addr;
  return m_interfaceAddress.IsValid();
}


bool PNatMethod_Fixed::IsAvailable(const PIPSocket::Address & binding)
{
  return PNatMethod::IsAvailable(binding) &&
         m_externalAddress.IsValid() &&
         (binding.IsAny() || binding == m_interfaceAddress);
}


#endif // P_NAT
