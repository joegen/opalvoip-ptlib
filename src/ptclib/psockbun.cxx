/*
 * psockbun.cxx
 *
 * Socket and interface bundle code
 *
 * Portable Windows Library
 *
 * Copyright (C) 2007 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

//////////////////////////////////////////////////

#ifdef __GNUC__
#pragma implementation "psockbun.h"
#endif

#include <ptlib.h>
#include <ptclib/psockbun.h>

#include <ptclib/pstun.h>


PFACTORY_CREATE_SINGLETON(PProcessStartupFactory, PInterfaceMonitor);

#ifndef _WIN32_WCE
#define UDP_BUFFER_SIZE 32768
#else
#define UDP_BUFFER_SIZE 8192
#endif

#define PTraceModule() "MonSock"

#define new PNEW


//////////////////////////////////////////////////

PInterfaceMonitor::PInterfaceMonitor(unsigned refresh, bool runMonitorThread)
  : m_runMonitorThread(runMonitorThread)
  , m_refreshInterval(refresh)
  , m_updateThread(NULL)
  , m_interfaceFilter(NULL)
  , m_changedDetector(NULL)
{
}


PInterfaceMonitor::~PInterfaceMonitor()
{
  Stop();

  delete m_changedDetector;
  delete m_interfaceFilter;
}


void PInterfaceMonitor::SetRefreshInterval(unsigned refresh)
{
  m_refreshInterval = refresh;
}


void PInterfaceMonitor::SetRunMonitorThread(bool runMonitorThread)
{
  m_runMonitorThread = runMonitorThread;
}


void PInterfaceMonitor::Start()
{
  PWaitAndSignal guard(m_threadMutex);

  if (m_changedDetector == NULL) {
    m_interfacesMutex.Wait();
    PIPSocket::GetInterfaceTable(m_interfaces);
    PTRACE(3, "IfaceMon", "Initial interface list:\n" << setfill('\n') << m_interfaces << setfill(' '));
    m_interfacesMutex.Signal();

    if (m_runMonitorThread) {
      m_changedDetector = PIPSocket::CreateRouteTableDetector();
      m_updateThread = new PThreadObj<PInterfaceMonitor>(*this, &PInterfaceMonitor::UpdateThreadMain);
      m_updateThread->SetThreadName("Network Interface Monitor");
    }
  }
}


void PInterfaceMonitor::Stop()
{
  m_threadMutex.Wait();

  // shutdown the update thread
  if (m_changedDetector != NULL) {
    PTRACE(4, "IfaceMon", "Awaiting thread termination");

    m_changedDetector->Cancel();

    m_threadMutex.Signal();
    m_updateThread->WaitForTermination();
    m_threadMutex.Wait();

    delete m_updateThread;
    m_updateThread = NULL;

    delete m_changedDetector;
    m_changedDetector = NULL;
  }

  m_threadMutex.Signal();
}


void PInterfaceMonitor::OnShutdown()
{
  Stop();
}


static PBoolean IsInterfaceInList(const PIPSocket::InterfaceEntry & entry,
                              const PIPSocket::InterfaceTable & list)
{
  for (PINDEX i = 0; i < list.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & listEntry = list[i];
    if ((entry.GetName() == listEntry.GetName()) && (entry.GetAddress() == listEntry.GetAddress()))
      return true;
  }
  return false;
}


static PBoolean InterfaceListIsSubsetOf(const PIPSocket::InterfaceTable & subset,
                                    const PIPSocket::InterfaceTable & set)
{
  for (PINDEX i = 0; i < subset.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = subset[i];
    if (!IsInterfaceInList(entry, set))
      return false;
  }

  return true;
}


static PBoolean CompareInterfaceLists(const PIPSocket::InterfaceTable & list1,
                                  const PIPSocket::InterfaceTable & list2)
{
  // if the sizes are different, then the list has changed. 
  if (list1.GetSize() != list2.GetSize())
    return false;

  // ensure every element in list1 is in list2
  if (!InterfaceListIsSubsetOf(list1, list2))
    return false;

  // ensure every element in list1 is in list2
  return InterfaceListIsSubsetOf(list2, list1);
}


void PInterfaceMonitor::RefreshInterfaceList()
{
  // get a new interface list
  PIPSocket::InterfaceTable newInterfaces;
  PIPSocket::GetInterfaceTable(newInterfaces);

  m_interfacesMutex.Wait();

  // if changed, then update the internal list
  if (CompareInterfaceLists(m_interfaces, newInterfaces)) {
    m_interfacesMutex.Signal();
    return;
  }

  PIPSocket::InterfaceTable oldInterfaces = m_interfaces;
  m_interfaces = newInterfaces;

  PTRACE(3, "IfaceMon", "Interface change detected, new list:\n" << setfill('\n') << newInterfaces << setfill(' '));
  
  m_interfacesMutex.Signal();

  // calculate the set of interfaces to add / remove beforehand
  PIPSocket::InterfaceTable interfacesToAdd;
  PIPSocket::InterfaceTable interfacesToRemove;
  interfacesToAdd.DisallowDeleteObjects();
  interfacesToRemove.DisallowDeleteObjects();
  
  PINDEX i;
  // look for interfaces to add that are in new list that are not in the old list
  for (i = 0; i < newInterfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & newEntry = newInterfaces[i];
    if (!newEntry.GetAddress().IsLoopback() && !IsInterfaceInList(newEntry, oldInterfaces))
      interfacesToAdd.Append(&newEntry);
  }
  // look for interfaces to remove that are in old list that are not in the new list
  for (i = 0; i < oldInterfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & oldEntry = oldInterfaces[i];
    if (!oldEntry.GetAddress().IsLoopback() && !IsInterfaceInList(oldEntry, newInterfaces))
      interfacesToRemove.Append(&oldEntry);
  }

  PIPSocket::ClearNameCache();
  OnInterfacesChanged(interfacesToAdd, interfacesToRemove);
}


void PInterfaceMonitor::UpdateThreadMain()
{
  PTRACE(4, "IfaceMon", "Started interface monitor thread.");

  // check for interface changes periodically
  while (m_changedDetector->Wait(m_refreshInterval))
    RefreshInterfaceList();

  PTRACE(4, "IfaceMon", "Finished interface monitor thread.");
}


static PString MakeInterfaceDescription(const PIPSocket::InterfaceEntry & entry)
{
  return entry.GetAddress().AsString(true) + '%' + entry.GetName();
}


static bool SplitInterfaceDescription(const PString & iface,
                                      PIPSocket::Address & address,
                                      PString & name)
{
  if (iface.IsEmpty())
    return false;

  PINDEX right = 0;
  if (iface[0] == '[')
    right = iface.Find(']');
  PINDEX percent = iface.Find('%', right);
  switch (percent) {
    case 0 :
      address = PIPSocket::GetDefaultIpAny();
      name = iface.Mid(1);
      return !name.IsEmpty();

    case P_MAX_INDEX :
      address = iface;
      name = PString::Empty();
      return !address.IsAny();
  }

  if (iface[0] == '*')
    address = PIPSocket::GetDefaultIpAny();
  else
    address = iface.Left(percent);
  name = iface.Mid(percent+1);
  return !name.IsEmpty();
}


static PBoolean InterfaceMatches(const PIPSocket::Address & addr,
                             const PString & name,
                             const PIPSocket::InterfaceEntry & entry)
{
  if ((addr.IsAny()   || entry.GetAddress() == addr) &&
      (name.IsEmpty() || entry.GetName().NumCompare(name) == PString::EqualTo)) {
    return true;
  }
  return false;
}


PStringArray PInterfaceMonitor::GetInterfaces(bool includeLoopBack, 
                                              const PIPSocket::Address & destination)
{
  PWaitAndSignal guard(m_interfacesMutex);
  
  PIPSocket::InterfaceTable ifaces = m_interfaces;
  
  if (m_interfaceFilter != NULL && !destination.IsAny())
    ifaces = m_interfaceFilter->FilterInterfaces(destination, ifaces);

  PStringArray names;

  names.SetSize(ifaces.GetSize());
  PINDEX count = 0;

  for (PINDEX i = 0; i < ifaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = ifaces[i];
    if (includeLoopBack || !entry.GetAddress().IsLoopback())
      names[count++] = MakeInterfaceDescription(entry);
  }

  names.SetSize(count);

  return names;
}


bool PInterfaceMonitor::IsValidBindingForDestination(const PIPSocket::Address & binding,
                                                     const PIPSocket::Address & destination)
{
  PWaitAndSignal guard(m_interfacesMutex);
  
  if (m_interfaceFilter == NULL)
    return true;
  
  PIPSocket::InterfaceTable ifaces = m_interfaces;
  ifaces = m_interfaceFilter->FilterInterfaces(destination, ifaces);
  for (PINDEX i = 0; i < ifaces.GetSize(); i++) {
    if (ifaces[i].GetAddress() == binding)
      return true;
  }
  return false;
}


bool PInterfaceMonitor::GetInterfaceInfo(const PString & iface, PIPSocket::InterfaceEntry & info) const
{
  PIPSocket::Address addr;
  PString name;
  if (!SplitInterfaceDescription(iface, addr, name))
    return false;

  PWaitAndSignal guard(m_interfacesMutex);

  for (PINDEX i = 0; i < m_interfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = m_interfaces[i];
    if (InterfaceMatches(addr, name, entry)) {
      info = entry;
      return true;
    }
  }

  return false;
}


bool PInterfaceMonitor::IsMatchingInterface(const PString & iface, const PIPSocket::InterfaceEntry & entry)
{
  PIPSocket::Address addr;
  PString name;
  if (!SplitInterfaceDescription(iface, addr, name))
    return FALSE;
  
  return InterfaceMatches(addr, name, entry);
}


void PInterfaceMonitor::SetInterfaceFilter(PInterfaceFilter * filter)
{
  PWaitAndSignal guard(m_interfacesMutex);
  
  delete m_interfaceFilter;
  m_interfaceFilter = filter;
}


void PInterfaceMonitor::AddNotifier(const Notifier & notifier, unsigned priority)
{
  PWaitAndSignal guard(m_notifiersMutex);

  if (m_notifiers.empty())
    Start();

  m_notifiers.insert(Notifiers::value_type(priority, notifier));
}


void PInterfaceMonitor::RemoveNotifier(const Notifier & notifier)
{
  m_notifiersMutex.Wait();

  for (Notifiers::iterator it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
    if (it->second == notifier) {
      m_notifiers.erase(it);
      break;
    }
  }

  bool stop = m_notifiers.empty();

  m_notifiersMutex.Signal();

  if (stop)
    Stop();
}

void PInterfaceMonitor::OnInterfacesChanged(const PIPSocket::InterfaceTable & addedInterfaces,
                                            const PIPSocket::InterfaceTable & removedInterfaces)
{
  PWaitAndSignal guard(m_notifiersMutex);

  for (Notifiers::iterator it = m_notifiers.begin(); it != m_notifiers.end(); ++it) {
    for (PINDEX i = 0; i < addedInterfaces.GetSize(); i++)
      it->second(*this, InterfaceChange(addedInterfaces[i], true));
    for (PINDEX i = 0; i < removedInterfaces.GetSize(); i++)
      it->second(*this, InterfaceChange(removedInterfaces[i], false));
  }
}


#if P_NAT

void PInterfaceMonitor::OnRemoveNatMethod(const PNatMethod * natMethod)
{
  PWaitAndSignal guard(m_notifiersMutex);

  for (Notifiers::iterator it = m_notifiers.begin(); it != m_notifiers.end(); ++it)
    it->second(*this, InterfaceChange(natMethod, false));
}

#endif


//////////////////////////////////////////////////

PMonitoredSockets::PMonitoredSockets(bool reuseAddr P_NAT_PARAM(PNatMethod * nat))
  : localPort(0)
  , reuseAddress(reuseAddr)
#if P_NAT
  , natMethod(nat)
#endif
  , opened(false)
  , interfaceAddedSignal(localPort, PIPSocket::GetDefaultIpAddressFamily())
{
}


PStringArray PMonitoredSockets::GetInterfaces(bool includeLoopBack, const PIPSocket::Address & destination)
{
  return PInterfaceMonitor::GetInstance().GetInterfaces(includeLoopBack, destination);
}


bool PMonitoredSockets::GetInterfaceInfo(const PString & iface, InterfaceEntry & info) const
{
  return PInterfaceMonitor::GetInstance().GetInterfaceInfo(iface, info);
}


bool PMonitoredSockets::CreateSocket(SocketInfo & info, const PIPSocket::Address & binding)
{
  delete info.socket;
  info.socket = NULL;
  
#if P_NAT
  if (natMethod != NULL && natMethod->IsAvailable(binding)) {
    PIPSocket::Address address;
    WORD port;
    natMethod->GetServerAddress(address, port);
    if (PInterfaceMonitor::GetInstance().IsValidBindingForDestination(binding, address)) {
      if (natMethod->CreateSocket(info.socket, binding, localPort)) {
        info.socket->PUDPSocket::GetLocalAddress(address, port);
        PTRACE(4, "Created bundled UDP socket via " << natMethod->GetName()
               << ", internal=" << address << ':' << port << ", external=" << info.socket->GetLocalAddress());
        return true;
      }
    }
  }
#endif

  info.socket = new PUDPSocket(localPort, (int) (binding.GetVersion() == 6 ? AF_INET6 : AF_INET));
  if (info.socket->Listen(binding, 0, localPort, reuseAddress?PIPSocket::CanReuseAddress:PIPSocket::AddressIsExclusive)) {
    PTRACE(4, "Created bundled UDP socket " << binding << ':' << info.socket->GetPort());
    int sz = 0;
    if (info.socket->GetOption(SO_RCVBUF, sz) && sz < UDP_BUFFER_SIZE) {
      if (!info.socket->SetOption(SO_RCVBUF, UDP_BUFFER_SIZE)) {
        PTRACE(1, "SetOption(SO_RCVBUF) failed: " << info.socket->GetErrorText());
      }
    }

    return true;
  }

  PTRACE(1, "Could not listen on "
         << binding << ':' << localPort
         << " - " << info.socket->GetErrorText());
  delete info.socket;
  info.socket = NULL;
  return false;
}


bool PMonitoredSockets::DestroySocket(SocketInfo & info)
{
  if (info.socket == NULL)
    return false;

  PBoolean result = info.socket->Close();

#if PTRACING
  if (result)
    PTRACE(4, "Closed UDP socket " << info.socket);
  else
    PTRACE(2, "Close failed for UDP socket " << info.socket);
#endif

  // This is pretty ugly, but needed to make sure multi-threading doesn't crash
  unsigned failSafe = 100; // Approx. two seconds
  while (info.inUse) {
    UnlockReadWrite();
    PThread::Sleep(20);
    if (!LockReadWrite())
      return false;
    if (--failSafe == 0) {
      PTRACE(1, "Read thread break for UDP socket " << info.socket << " taking too long.");
      break;
    }
  }

  PTRACE(4, "Deleting UDP socket " << info.socket);
  delete info.socket;
  info.socket = NULL;

  return result;
}


bool PMonitoredSockets::GetSocketAddress(const SocketInfo & info,
                                         PIPSocket::Address & address,
                                         WORD & port,
                                         bool usingNAT) const
{
  if (info.socket == NULL)
    return false;

  if (usingNAT)
    return info.socket->GetLocalAddress(address, port);

  PIPSocketAddressAndPort addrAndPort;
  if (!info.socket->PUDPSocket::InternalGetLocalAddress(addrAndPort))
    return false;

  address = addrAndPort.GetAddress();
  port = addrAndPort.GetPort();
  return true;
}


void PMonitoredSockets::SocketInfo::Write(BundleParams & param)
{
  socket->WriteTo(param.m_buffer, param.m_length, param.m_addr, param.m_port);
  param.m_lastCount = socket->GetLastWriteCount();
  param.m_errorCode = socket->GetErrorCode(PChannel::LastWriteError);
  param.m_errorNumber = socket->GetErrorNumber(PChannel::LastWriteError);
}


void PMonitoredSockets::ReadFromSocketList(PSocket::SelectList & readers,
                                           PUDPSocket * & socket,
                                           BundleParams & param)
{
  // Assume is already locked

  socket = NULL;
  param.m_lastCount = 0;

  UnlockReadWrite();

  param.m_errorCode = PSocket::Select(readers, param.m_timeout);

  if (!LockReadWrite() || !opened) {
    param.m_errorCode = PChannel::NotOpen;  // Closed, break out
    return;
  }

  switch (param.m_errorCode) {
    case PChannel::NoError :
      break;

    case PChannel::NotOpen : // Interface went down
      if (!interfaceAddedSignal.IsOpen()) {
        interfaceAddedSignal.Listen(); // Reset if this was used to break Select() block
        param.m_errorCode = PChannel::Interrupted;
        PTRACE(4, "Interfaces changed");
        return;
      }
      // Do next case

    default :
      return;
  }

  if (readers.IsEmpty()) {
    param.m_errorCode = PChannel::Timeout;
    return;
  }

  socket = (PUDPSocket *)&readers.front();

  bool ok = socket->ReadFrom(param.m_buffer, param.m_length, param.m_addr, param.m_port);
  param.m_lastCount = socket->GetLastReadCount();
  param.m_errorCode = socket->GetErrorCode(PChannel::LastReadError);
  param.m_errorNumber = socket->GetErrorNumber(PChannel::LastReadError);

  if (ok)
    return;

  switch (param.m_errorCode) {
    case PChannel::Unavailable :
      PTRACE(3, "UDP Port on remote not ready.");
      break;

    case PChannel::BufferTooSmall :
      PTRACE(2, "Read UDP packet too large for buffer of " << param.m_length << " bytes.");
      break;

    case PChannel::NotFound :
      PTRACE(4, "Interface went down");
      param.m_errorCode = PChannel::Interrupted;
      break;

    default :
      PTRACE(1, "Socket read UDP error ("
             << socket->GetErrorNumber(PChannel::LastReadError) << "): "
             << socket->GetErrorText(PChannel::LastReadError));
  }
}


void PMonitoredSockets::SocketInfo::Read(PMonitoredSockets & bundle, BundleParams & param)
{
  // Assume is already locked

  if (inUse) {
    PTRACE(2, &bundle, NULL, "Cannot read from multiple threads.");
    param.m_errorCode = PChannel::DeviceInUse;
    return;
  }

  param.m_lastCount = 0;

  do {
    PSocket::SelectList sockets;
    if (socket == NULL || !socket->IsOpen())
      inUse = false; // socket closed by monitor thread. release the inUse flag
    else {
      sockets += *socket;
      inUse = true;
    }
    sockets += bundle.interfaceAddedSignal;

    PUDPSocket * socket;
    bundle.ReadFromSocketList(sockets, socket, param);
  } while (param.m_errorCode == PChannel::NoError && param.m_lastCount == 0);

  inUse = false;
}


PMonitoredSockets * PMonitoredSockets::Create(const PString & iface, bool reuseAddr P_NAT_PARAM(PNatMethod * natMethod))
{
  if (iface.IsEmpty() || iface == "*")
    return new PMonitoredSocketBundle(PString::Empty(), 0, reuseAddr P_NAT_PARAM(natMethod));

  PINDEX percent = iface.Find('%');

  if (percent == 0 || (percent == 1 && iface[0] == '*'))
    return new PMonitoredSocketBundle(iface.Mid(percent+1), 0, reuseAddr P_NAT_PARAM(natMethod));

  PIPSocket::Address ip(iface);
  if (!ip.IsValid())
    return NULL;
  
  if (ip.IsAny())
    return new PMonitoredSocketBundle(percent != P_MAX_INDEX ? iface.Mid(percent+1) : PString::Empty(),
                                      ip.GetVersion(), reuseAddr P_NAT_PARAM(natMethod));

  return new PSingleMonitoredSocket(ip.AsString(true), reuseAddr P_NAT_PARAM(natMethod));
}


#if P_NAT

void PMonitoredSockets::OnRemoveNatMethod(const PNatMethod * nat)
{
  if (natMethod == nat)
    natMethod = NULL;
}

#endif


//////////////////////////////////////////////////

PMonitoredSocketChannel::PMonitoredSocketChannel(const PMonitoredSocketsPtr & sock, bool shared)
  : socketBundle(sock)
  , sharedBundle(shared)
  , promiscuousReads(false)
  , closing(false)
  , remotePort(0)
  , lastReceivedAddress(PIPSocket::GetDefaultIpAny())
  , lastReceivedPort(0)
{
}


PBoolean PMonitoredSocketChannel::IsOpen() const
{
  return !closing && socketBundle != NULL && socketBundle->IsOpen();
}


PBoolean PMonitoredSocketChannel::Close()
{
  closing = true;
  return sharedBundle || socketBundle == NULL || socketBundle->Close();
}


PBoolean PMonitoredSocketChannel::Read(void * buffer, PINDEX length)
{
  if (!IsOpen())
    return false;

  do {
    lastReceivedInterface = GetInterface();
    PMonitoredSockets::BundleParams param;
    param.m_buffer = buffer;
    param.m_length = length;
    param.m_timeout = readTimeout;
    socketBundle->ReadFromBundle(param);
    lastReceivedAddress = param.m_addr;
    lastReceivedPort = param.m_port;
    lastReceivedInterface = param.m_iface;
    lastReadCount = param.m_lastCount;
    if (!SetErrorValues(param.m_errorCode, param.m_errorNumber, LastReadError))
      return false;

    if (promiscuousReads)
      return true;

    if (remoteAddress.IsAny())
      remoteAddress = lastReceivedAddress;
    if (remotePort == 0)
      remotePort = lastReceivedPort;

  } while (remoteAddress != lastReceivedAddress || remotePort != lastReceivedPort);
  return true;
}


PBoolean PMonitoredSocketChannel::Write(const void * buffer, PINDEX length)
{
  if (!IsOpen())
    return false;

  PMonitoredSockets::BundleParams param;
  param.m_buffer = (void *)buffer;
  param.m_length = length;
  param.m_addr = remoteAddress;
  param.m_port = remotePort;
  param.m_iface = GetInterface();
  param.m_timeout = readTimeout;
  socketBundle->WriteToBundle(param);
  lastWriteCount = param.m_lastCount;
  return SetErrorValues(param.m_errorCode, param.m_errorNumber, LastWriteError);
}


void PMonitoredSocketChannel::SetInterface(const PString & iface)
{
  mutex.Wait();

  PIPSocket::InterfaceEntry info;
  if (socketBundle != NULL && socketBundle->GetInterfaceInfo(iface, info))
    currentInterface = MakeInterfaceDescription(info);
  else
    currentInterface = iface;

  if (lastReceivedInterface.IsEmpty())
    lastReceivedInterface = currentInterface;

  mutex.Signal();
}


PString PMonitoredSocketChannel::GetInterface()
{
  PString iface;

  mutex.Wait();

  if (currentInterface.Find('%') == P_MAX_INDEX)
    SetInterface(currentInterface);

  iface = currentInterface;
  iface.MakeUnique();

  mutex.Signal();

  return iface;
}


bool PMonitoredSocketChannel::GetLocal(PIPSocket::Address & address, WORD & port, bool usingNAT)
{
  return socketBundle->GetAddress(GetInterface(), address, port, usingNAT);
}


void PMonitoredSocketChannel::SetRemote(const PIPSocket::Address & address, WORD port)
{
  remoteAddress = address;
  remotePort = port;
}


void PMonitoredSocketChannel::SetRemote(const PString & hostAndPort)
{
  PINDEX colon = hostAndPort.Find(':');
  if (colon == P_MAX_INDEX)
    remoteAddress = hostAndPort;
  else {
    remoteAddress = hostAndPort.Left(colon);
    remotePort = PIPSocket::GetPortByService("udp", hostAndPort.Mid(colon+1));
  }
}


//////////////////////////////////////////////////

PMonitoredSocketBundle::PMonitoredSocketBundle(const PString & fixedInterface,
                                               unsigned ipVersion,
                                               bool reuseAddr
                                               P_NAT_PARAM(PNatMethod * natMethod))
  : PMonitoredSockets(reuseAddr P_NAT_PARAM(natMethod))
  , m_fixedInterface(fixedInterface)
  , m_ipVersion(ipVersion)
{
  PInterfaceMonitor::GetInstance().AddNotifier(PCREATE_InterfaceNotifier(OnInterfaceChange));

  PTRACE(4, "Created socket bundle for "
         << (fixedInterface.IsEmpty() ? "all" : "fixed")
         << (ipVersion != 4 ? ipVersion != 6 ? " " : " IPv6 " : " IPv4 " )
         << "interface" << (fixedInterface.IsEmpty() ? "s." : ": ") << fixedInterface);
}


PMonitoredSocketBundle::~PMonitoredSocketBundle()
{
  Close();

  PInterfaceMonitor::GetInstance().RemoveNotifier(PCREATE_InterfaceNotifier(OnInterfaceChange));
}


PStringArray PMonitoredSocketBundle::GetInterfaces(bool /*includeLoopBack*/, const PIPSocket::Address & /*destination*/)
{
  PSafeLockReadOnly guard(*this);

  PStringList names;
  for (SocketInfoMap_T::iterator iter = m_socketInfoMap.begin(); iter != m_socketInfoMap.end(); ++iter)
    names += iter->first;
  return names;
}


PBoolean PMonitoredSocketBundle::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (IsOpen() && localPort != 0  && localPort == port)
    return true;

  opened = true;

  localPort = port;

  // Close and re-open all sockets
  while (!m_socketInfoMap.empty())
    CloseSocket(m_socketInfoMap.begin());

  PStringArray interfaces = PMonitoredSockets::GetInterfaces();
  for (PINDEX i = 0; i < interfaces.GetSize(); ++i)
    OpenSocket(interfaces[i]);

  return true;
}


PBoolean PMonitoredSocketBundle::Close()
{
  if (!LockReadWrite())
    return false;

  opened = false;

  while (!m_socketInfoMap.empty())
    CloseSocket(m_socketInfoMap.begin());
  interfaceAddedSignal.Close(); // Fail safe break out of Select()

  UnlockReadWrite();

  return true;
}


PBoolean PMonitoredSocketBundle::GetAddress(const PString & iface,
                                        PIPSocket::Address & address,
                                        WORD & port,
                                        PBoolean usingNAT) const
{
  PIPSocket::InterfaceEntry info;
  if (!GetInterfaceInfo(iface, info)) {
    address = PIPSocket::Address::GetAny(m_ipVersion);
    port = localPort;
    return true;
  }

  PSafeLockReadOnly guard(*this);
  if (!guard.IsLocked())
    return false;

  SocketInfoMap_T::const_iterator iter = m_socketInfoMap.find(MakeInterfaceDescription(info));
  return iter != m_socketInfoMap.end() && GetSocketAddress(iter->second, address, port, usingNAT);
}


void PMonitoredSocketBundle::OpenSocket(const PString & iface)
{
  PIPSocket::Address binding;
  PString name;
  SplitInterfaceDescription(iface, binding, name);

  if (!m_fixedInterface.IsEmpty() && m_fixedInterface != name) {
    PTRACE(4, "Interface \"" << iface << "\" is not on \"" << m_fixedInterface << '"');
    return;
  }

  if (m_ipVersion != 0 && binding.GetVersion() != m_ipVersion) {
    PTRACE(4, "Interface \"" << iface << "\" is not IPv" << m_ipVersion);
    return;
  }

  if (binding.IsAny() || binding.IsBroadcast()) {
    PTRACE(4, "Interface \"" << iface << "\" has no IPv" << m_ipVersion << " address.");
    return;
  }

  SocketInfo info;
  if (CreateSocket(info, binding)) {
    if (localPort == 0) {
      PIPSocketAddressAndPort addrAndPort;
      info.socket->PUDPSocket::InternalGetLocalAddress(addrAndPort);
      localPort = addrAndPort.GetPort();
    }
    m_socketInfoMap[iface] = info;
  }
}


void PMonitoredSocketBundle::CloseSocket(SocketInfoMap_T::iterator iterSocket)
{
  //Already locked by caller

  if (iterSocket == m_socketInfoMap.end())
    return;

  DestroySocket(iterSocket->second);
  m_socketInfoMap.erase(iterSocket);
}


void PMonitoredSocketBundle::WriteToBundle(BundleParams & param)
{
  if (!LockReadWrite()) {
    param.m_errorCode = PChannel::NotOpen;
    return;
  }

  if (param.m_iface.IsEmpty()) {
    for (SocketInfoMap_T::iterator iter = m_socketInfoMap.begin(); iter != m_socketInfoMap.end(); ++iter) {
      iter->second.Write(param);
      if (param.m_errorCode != PChannel::NoError)
        break;
    }
  }
  else {
    SocketInfoMap_T::iterator iter = m_socketInfoMap.find(param.m_iface);
    if (iter != m_socketInfoMap.end())
      iter->second.Write(param);
    else
      param.m_errorCode = PChannel::NotFound;
  }

  UnlockReadWrite();
}


void PMonitoredSocketBundle::ReadFromBundle(BundleParams & param)
{
  if (!opened || !LockReadWrite()) {
    param.m_errorCode = PChannel::NotOpen;
    return;
  }

  if (param.m_iface.IsEmpty()) {
    do {
      // If interface is empty, then grab the next datagram on any of the interfaces
      PSocket::SelectList readers;

      for (SocketInfoMap_T::iterator iter = m_socketInfoMap.begin(); iter != m_socketInfoMap.end(); ++iter) {
        if (iter->second.inUse) {
          PTRACE(2, "Cannot read from multiple threads.");
          UnlockReadWrite();
          param.m_errorCode = PChannel::DeviceInUse;
          return;
        }
        if (iter->second.socket->IsOpen()) {
          readers += *iter->second.socket;
          iter->second.inUse = true;
        }
      }
      readers += interfaceAddedSignal;

      PUDPSocket * socket;
      ReadFromSocketList(readers, socket, param);

      for (SocketInfoMap_T::iterator iter = m_socketInfoMap.begin(); iter != m_socketInfoMap.end(); ++iter) {
        if (iter->second.socket == socket)
          param.m_iface = iter->first;
        iter->second.inUse = false;
      }
    } while (param.m_errorCode == PChannel::NoError && param.m_lastCount == 0);
  }
  else {
    // if interface is not empty, use that specific interface
    SocketInfoMap_T::iterator iter = m_socketInfoMap.find(param.m_iface);
    if (iter != m_socketInfoMap.end())
      iter->second.Read(*this, param);
    else
      param.m_errorCode = PChannel::NotFound;
  }

  UnlockReadWrite();
}


void PMonitoredSocketBundle::OnInterfaceChange(PInterfaceMonitor &, PInterfaceMonitor::InterfaceChange entry)
{
  if (!opened || !LockReadWrite())
    return;

  if (entry.m_added) {
    OpenSocket(MakeInterfaceDescription(entry));
    PTRACE(3, "UDP socket bundle has added interface " << entry);
    interfaceAddedSignal.Close();
  }
  else {
    CloseSocket(m_socketInfoMap.find(MakeInterfaceDescription(entry)));
    PTRACE(3, "UDP socket bundle has removed interface " << entry);
#if P_NAT
    OnRemoveNatMethod(entry.m_natMethod);
#endif
  }

  UnlockReadWrite();
}


//////////////////////////////////////////////////

PSingleMonitoredSocket::PSingleMonitoredSocket(const PString & theInterface, bool reuseAddr P_NAT_PARAM(PNatMethod * natMethod))
  : PMonitoredSockets(reuseAddr P_NAT_PARAM(natMethod))
  , m_interface(theInterface)
{
  PInterfaceMonitor::GetInstance().AddNotifier(PCREATE_InterfaceNotifier(OnInterfaceChange));

  PTRACE(4, "Created monitored socket for interface " << theInterface);
}


PSingleMonitoredSocket::~PSingleMonitoredSocket()
{
  Close();

  PInterfaceMonitor::GetInstance().RemoveNotifier(PCREATE_InterfaceNotifier(OnInterfaceChange));
}


PStringArray PSingleMonitoredSocket::GetInterfaces(bool /*includeLoopBack*/, const PIPSocket::Address & /*destination*/)
{
  PSafeLockReadOnly guard(*this);

  PStringList names;
  if (m_entry.GetAddress().IsValid())
    names.AppendString(MakeInterfaceDescription(m_entry));
  return names;
}


PBoolean PSingleMonitoredSocket::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (opened && localPort == port && m_info.socket != NULL && m_info.socket->IsOpen())
    return true;

  Close();

  opened = true;

  localPort = port;

  if (!m_entry.GetAddress().IsValid() && !GetInterfaceInfo(m_interface, m_entry)) {
    PTRACE(3, "Not creating socket as interface \"" << m_entry.GetName() << "\" is  not up.");
    return true; // Still say successful though
  }

  if (!CreateSocket(m_info, m_entry.GetAddress()))
    return false;
    
  localPort = m_info.socket->PUDPSocket::GetPort();
  return true;
}


PBoolean PSingleMonitoredSocket::Close()
{
  PSafeLockReadWrite guard(*this);

  if (!opened)
    return true;

  opened = false;
  interfaceAddedSignal.Close(); // Fail safe break out of Select()
  return DestroySocket(m_info);
}


PBoolean PSingleMonitoredSocket::GetAddress(const PString & iface,
                                        PIPSocket::Address & address,
                                        WORD & port,
                                        PBoolean usingNAT) const
{
  PSafeLockReadOnly guard(*this);

  return guard.IsLocked() && IsInterface(iface) && GetSocketAddress(m_info, address, port, usingNAT);
}


void PSingleMonitoredSocket::WriteToBundle(BundleParams & param)
{
  PSafeLockReadWrite guard(*this);

  if (guard.IsLocked() && m_info.socket != NULL && IsInterface(param.m_iface))
    m_info.Write(param);
  else
    param.m_errorCode = PChannel::NotFound;
}


void PSingleMonitoredSocket::ReadFromBundle(BundleParams & param)
{
  if (!opened || !LockReadWrite()) {
    param.m_errorCode = PChannel::NotOpen;
    return;
  }

  if (IsInterface(param.m_iface))
    m_info.Read(*this, param);
  else
    param.m_errorCode = PChannel::NotFound;

  param.m_iface = m_interface;

  UnlockReadWrite();
}


void PSingleMonitoredSocket::OnInterfaceChange(PInterfaceMonitor &, PInterfaceMonitor::InterfaceChange entry)
{
  PSafeLockReadWrite guard(*this);
  if (!guard.IsLocked() || !opened)
    return;

  if (entry.m_added) {
    PIPSocket::Address addr;
    PString name;
    if (!SplitInterfaceDescription(m_interface, addr, name))
      return;

    if ((!addr.IsValid() || entry.GetAddress() == addr) && entry.GetName().NumCompare(name) == EqualTo) {
      m_entry = entry;
      if (!Open(localPort))
        m_entry = InterfaceEntry();
      else {
        interfaceAddedSignal.Close();
        PTRACE(3, "Bound UDP socket UP event on interface " << m_entry);
      }
    }
  }
  else {
    if (entry != m_entry)
      return;

    PTRACE(3, "Bound UDP socket DOWN event on interface " << m_entry);
    m_entry = InterfaceEntry();
    DestroySocket(m_info);
#if P_NAT
    OnRemoveNatMethod(entry.m_natMethod);
#endif
  }
}


bool PSingleMonitoredSocket::IsInterface(const PString & iface) const
{
  if (iface.IsEmpty())
    return true;

  PINDEX percent1 = iface.Find('%');
  PINDEX percent2 = m_interface.Find('%');

  if (percent1 != P_MAX_INDEX && percent2 != P_MAX_INDEX)
    return iface.Mid(percent1+1).NumCompare(m_interface.Mid(percent2+1)) == EqualTo;

  return PIPSocket::Address(iface.Left(percent1)) == PIPSocket::Address(m_interface.Left(percent2));
}
