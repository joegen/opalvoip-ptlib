
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

#include <ptclib/pnat.h>

#if P_NAT

#include <ptclib/random.h>


static const char PNatMethodBaseClass[] = "PNatMethod";
template <> PNatMethod * PDevicePluginFactory<PNatMethod>::Worker::Create(const PDefaultPFactoryKey & method) const
{
   return PNatMethod::Create(method);
}

typedef PDevicePluginAdapter<PNatMethod> PDevicePluginPNatMethod;
PFACTORY_CREATE(PFactory<PDevicePluginAdapterBase>, PDevicePluginPNatMethod, PNatMethodBaseClass, true);


PNatStrategy::PNatStrategy()
{
   pluginMgr = NULL;
}

PNatStrategy::~PNatStrategy()
{
   natlist.RemoveAll();
}

void PNatStrategy::AddMethod(PNatMethod * method)
{
  natlist.Append(method);
}

PNatMethod * PNatStrategy::GetMethod(const PIPSocket::Address & address)
{
  for (PNatList::iterator i = natlist.begin(); i != natlist.end(); i++) {
    if (i->Open(address))
      return &*i;
  }

  return NULL;
}

PNatMethod * PNatStrategy::GetMethodByName(const PString & name)
{
  for (PNatList::iterator i = natlist.begin(); i != natlist.end(); i++) {
    if (i->GetName() == name) {
      return &*i;
	  }
  }

  return NULL;
}

bool PNatStrategy::RemoveMethod(const PString & meth)
{
  for (PNatList::iterator i = natlist.begin(); i != natlist.end(); i++) {
    if (i->GetName() == meth) {
      natlist.erase(i);
      return true;
    }
  }

  return false;
}

void PNatStrategy::SetPortRanges(WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax)
{
  for (PNatList::iterator i = natlist.begin(); i != natlist.end(); i++)
    i->SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


PNatMethod * PNatStrategy::LoadNatMethod(const PString & name)
{
   if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PNatMethod *)pluginMgr->CreatePluginsDeviceByName(name, PNatMethodBaseClass);
}

PStringArray PNatStrategy::GetRegisteredList()
{
  PPluginManager * plugMgr = &PPluginManager::GetPluginManager();
  return plugMgr->GetPluginsProviding(PNatMethodBaseClass);
}

///////////////////////////////////////////////////////////////////////

PNatMethod::PNatMethod()
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
    "Symmetric Firewall",
    "Blocked",
    "Partially Blocked"
  };

  if (type < NumNatTypes)
    return Names[type];
  
  return psprintf("<NATType %u>", type);
}

PNatMethod * PNatMethod::Create(const PString & name)
{
  return PFactory<PNatMethod>::CreateInstance(name);
}

PNatMethod * PNatMethod::Create(const PString & name, PPluginManager * pluginMgr)
{
  if (pluginMgr == NULL)
    pluginMgr = &PPluginManager::GetPluginManager();

  return (PNatMethod *)pluginMgr->CreatePluginsDeviceByName(name, PNatMethodBaseClass,0);
}

bool PNatMethod::CreateSocketPairAsync(const PString & /*token*/)
{
  return true;
}


bool PNatMethod::GetSocketPairAsync(const PString & /*token*/,
                                    PUDPSocket * & socket1,
                                    PUDPSocket * & socket2,
                                    const PIPSocket::Address & binding,
                                    void * userData)
{
  return CreateSocketPair(socket1, socket2, binding, userData);
}


PBoolean PNatMethod::CreateSocketPair(PUDPSocket * & socket1,
                                      PUDPSocket * & socket2,
                                      const PIPSocket::Address & binding,
                                      void * /*userData*/)
{
  return CreateSocketPair(socket1, socket2, binding);
}


void PNatMethod::PrintOn(ostream & strm) const
{
  strm << GetName() << " server " << GetServer();
}

void PNatMethod::SetPortRanges(WORD portBase, WORD portMax, WORD portPairBase, WORD portPairMax) 
{
  {
    PWaitAndSignal m(singlePortInfo.mutex);
    singlePortInfo.SetPorts(portBase, portMax);
  }
  {
    PWaitAndSignal m(pairedPortInfo.mutex);
    pairedPortInfo.SetPorts((portPairBase+1)&0xfffe, portPairMax);
  }
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
    currentPort = currentPort;
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

void PNatMethod::Activate(bool /*active*/)
{

}

void PNatMethod::SetAlternateAddresses(const PStringArray & /*addresses*/, void * /*userData*/)
{

}

WORD PNatMethod::RandomPortPair(unsigned int start, unsigned int end)
{
	WORD num;
	PRandom rand;
	num = (WORD)rand.Generate(start,end);
	if (PString(num).Right(1).FindOneOf("13579") != P_MAX_INDEX) 
			num++;  // Make sure the number is even

	return num;
}

////////////////////////////////////////////////////

PNATUDPSocket::PNATUDPSocket(PQoS * qos)
  : PUDPSocket(qos)
  , m_component(PNatMethod::eComponent_Unknown)
{
}

////////////////////////////////////////////////////

PCREATE_NAT_PLUGIN(Null);

class PNullNATSocket : public PNATUDPSocket
{
  PCLASSINFO(PNullNATSocket, PNATUDPSocket);
  public:
    PNullNATSocket()
    { }

    virtual bool OpenNAT(BYTE component)
    { m_component = component; return true; }

    virtual PNatCandidate GetCandidateInfo()
    {
      PNatCandidate candidate(PNatCandidate::eType_Host, m_component);
      PUDPSocket::GetLocalAddress(candidate.m_baseAddress);
      PUDPSocket::GetLocalAddress(candidate.m_transport);
      return candidate;
    }

    virtual bool InternalGetLocalAddress(PIPSocketAddressAndPort & addrAndPort)
    { return PUDPSocket::InternalGetLocalAddress(addrAndPort); }

    virtual bool InternalGetBaseAddress(PIPSocketAddressAndPort & addrAndPort)
    { return PUDPSocket::InternalGetLocalAddress(addrAndPort); }
};

////////////////////////////////////////////////////

PNatCandidate::PNatCandidate()
  : m_type(eType_Unknown)
  , m_component(PNatMethod::eComponent_Unknown)
{
}


PNatCandidate::PNatCandidate(int type, BYTE component)
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

#endif // P_NAT
