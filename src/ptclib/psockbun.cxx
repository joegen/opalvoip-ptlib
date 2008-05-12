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


static const char FactoryName[] = PINTERFACE_MONITOR_FACTORY_NAME;
static PFactory<PProcessStartup>::Worker<PInterfaceMonitor> InterfaceMonitorFactory(FactoryName, true);

static int UDP_BUFFER_SIZE = 32767;


#define new PNEW


//////////////////////////////////////////////////

PInterfaceMonitorClient::PInterfaceMonitorClient(PINDEX _priority)
: priority(_priority)
{
  PInterfaceMonitor::GetInstance().AddClient(this);
}


PInterfaceMonitorClient::~PInterfaceMonitorClient()
{
  PInterfaceMonitor::GetInstance().RemoveClient(this);
}


PStringArray PInterfaceMonitorClient::GetInterfaces(bool includeLoopBack, const PIPSocket::Address & destination)
{
  return PInterfaceMonitor::GetInstance().GetInterfaces(includeLoopBack, destination);
}


PBoolean PInterfaceMonitorClient::GetInterfaceInfo(const PString & iface, InterfaceEntry & info)
{
  return PInterfaceMonitor::GetInstance().GetInterfaceInfo(iface, info);
}


//////////////////////////////////////////////////

PInterfaceMonitor::PInterfaceMonitor(unsigned refresh, bool _runMonitorThread)
  : runMonitorThread(_runMonitorThread)
  , refreshInterval(refresh)
  , updateThread(NULL)
  , interfaceFilter(NULL)
{
}


PInterfaceMonitor::~PInterfaceMonitor()
{
  Stop();

  delete interfaceFilter;
}


PInterfaceMonitor & PInterfaceMonitor::GetInstance()
{
  return *PFactory<PProcessStartup>::CreateInstanceAs<PInterfaceMonitor>(FactoryName);
}


bool PInterfaceMonitor::Start()
{
  PWaitAndSignal m(mutex);
  
  if (runMonitorThread && updateThread != NULL)
    return false; // Already running

  PIPSocket::GetInterfaceTable(currentInterfaces);
  PTRACE(4, "IfaceMon\tInitial interface list:\n" << setfill('\n') << currentInterfaces << setfill(' '));

  if (runMonitorThread) {
    updateThread = new PThreadObj<PInterfaceMonitor>(*this, &PInterfaceMonitor::UpdateThreadMain);
    updateThread->SetThreadName("Network Interface Monitor");
  }
  return true;
  
}


void PInterfaceMonitor::Stop()
{
  mutex.Wait();

  // shutdown the update thread
  if (updateThread != NULL) {
    threadRunning.Signal();

    mutex.Signal();
    updateThread->WaitForTermination();
    mutex.Wait();

    delete updateThread;
    updateThread = NULL;
  }

  mutex.Signal();
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

  PWaitAndSignal m(mutex);

  // if changed, then update the internal list
  if (!CompareInterfaceLists(currentInterfaces, newInterfaces)) {

    PIPSocket::InterfaceTable oldInterfaces = currentInterfaces;
    currentInterfaces = newInterfaces;
    
    PTRACE(4, "IfaceMon\tInterface change detected, new list:\n" << setfill('\n') << currentInterfaces << setfill(' '));
    
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
    
    OnInterfacesChanged(interfacesToAdd, interfacesToRemove);
  }
}


void PInterfaceMonitor::UpdateThreadMain()
{
  PTRACE(4, "IfaceMon\tStarted interface monitor thread.");

  // check for interface changes periodically
  do {
    RefreshInterfaceList();
  } while (!threadRunning.Wait(refreshInterval));

  PTRACE(4, "IfaceMon\tFinished interface monitor thread.");
}


static PString MakeInterfaceDescription(const PIPSocket::InterfaceEntry & entry)
{
  return entry.GetAddress().AsString() + '%' + entry.GetName();
}


static PBoolean SplitInterfaceDescription(const PString & iface,
                                      PIPSocket::Address & address,
                                      PString & name)
{
  if (iface.IsEmpty())
    return false;

  PINDEX percent = iface.Find('%');
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
  PWaitAndSignal guard(mutex);
  
  PIPSocket::InterfaceTable ifaces = currentInterfaces;
  
  if (interfaceFilter != NULL && !destination.IsAny()) {
    ifaces = interfaceFilter->FilterInterfaces(destination, ifaces);
  }

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
  PWaitAndSignal guard(mutex);
  
  if (interfaceFilter == NULL)
    return true;
  
  PIPSocket::InterfaceTable ifaces = currentInterfaces;
  ifaces = interfaceFilter->FilterInterfaces(destination, ifaces);
  for (PINDEX i = 0; i < ifaces.GetSize(); i++) {
    if (ifaces[i].GetAddress() == binding)
      return true;
  }
  return false;
}


bool PInterfaceMonitor::GetInterfaceInfo(const PString & iface, PIPSocket::InterfaceEntry & info)
{
  PIPSocket::Address addr;
  PString name;
  if (!SplitInterfaceDescription(iface, addr, name))
    return false;

  PWaitAndSignal m(mutex);

  for (PINDEX i = 0; i < currentInterfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = currentInterfaces[i];
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
  PWaitAndSignal m(mutex);
  
  delete interfaceFilter;
  interfaceFilter = filter;
}


void PInterfaceMonitor::AddClient(PInterfaceMonitorClient * client)
{
  PWaitAndSignal m(mutex);

  if (currentClients.empty()) {
    Start();
    currentClients.push_back(client);
  } else {
    for (ClientList_T::iterator iter = currentClients.begin(); iter != currentClients.end(); ++iter) {
      if ((*iter)->GetPriority() >= client->GetPriority()) {
        currentClients.insert(iter, client);
        return;
      }
    }
    currentClients.push_back(client);
  }
}


void PInterfaceMonitor::RemoveClient(PInterfaceMonitorClient * client)
{
  mutex.Wait();
  currentClients.remove(client);
  bool stop = currentClients.empty();
  mutex.Signal();
  if (stop)
    Stop();
}

void PInterfaceMonitor::OnInterfacesChanged(const PIPSocket::InterfaceTable & addedInterfaces,
                                            const PIPSocket::InterfaceTable & removedInterfaces)
{
  PWaitAndSignal m(mutex);
  
  for (ClientList_T::reverse_iterator iter = currentClients.rbegin(); iter != currentClients.rend(); ++iter) {
    PInterfaceMonitorClient * client = *iter;
    if (client->LockReadWrite()) {
      for (PINDEX i = 0; i < addedInterfaces.GetSize(); i++)
        client->OnAddInterface(addedInterfaces[i]);
      for (PINDEX i = 0; i < removedInterfaces.GetSize(); i++)
        client->OnRemoveInterface(removedInterfaces[i]);
      client->UnlockReadWrite();
    }
  }
}


void PInterfaceMonitor::OnRemoveSTUNClient(const PSTUNClient *stun)
{
  PWaitAndSignal m(mutex);
  
  for (ClientList_T::reverse_iterator iter = currentClients.rbegin(); iter != currentClients.rend(); ++iter) {
    PInterfaceMonitorClient *client = *iter;
    if (client->LockReadWrite()) {
      client->OnRemoveSTUNClient(stun);
      client->UnlockReadWrite();
    }
  }
}


//////////////////////////////////////////////////

PMonitoredSockets::PMonitoredSockets(bool reuseAddr, PSTUNClient * stunClient)
  : localPort(0)
  , reuseAddress(reuseAddr)
  , stun(stunClient)
  , opened(false)
{
}


bool PMonitoredSockets::CreateSocket(SocketInfo & info, const PIPSocket::Address & binding)
{
  delete info.socket;
  info.socket = NULL;
  
  if (stun != NULL) {
    PIPSocket::Address address;
    WORD port;
    stun->GetServer(address, port);
    if (PInterfaceMonitor::GetInstance().IsValidBindingForDestination(binding, address)) {
      if (stun->CreateSocket(info.socket, binding, localPort)) {
        PTRACE(4, "MonSock\tCreated bundled UDP socket via STUN internal="
               << binding << ':' << info.socket->PUDPSocket::GetPort()
               << " external=" << info.socket->GetLocalAddress());
        return true;
      }
    }
  }

  info.socket = new PUDPSocket;
  if (info.socket->Listen(binding, 0, localPort, reuseAddress?PIPSocket::CanReuseAddress:PIPSocket::AddressIsExclusive)) {
    PTRACE(4, "MonSock\tCreated bundled UDP socket " << binding << ':' << info.socket->GetPort());
    int sz = 0;
    if (info.socket->GetOption(SO_RCVBUF, sz) && sz < UDP_BUFFER_SIZE) {
      if (!info.socket->SetOption(SO_RCVBUF, UDP_BUFFER_SIZE)) {
        PTRACE(1, "MonSock\tSetOption(SO_RCVBUF) failed: " << info.socket->GetErrorText());
      }
    }

    return true;
  }

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
    PTRACE(4, "MonSock\tClosed UDP socket " << info.socket);
  else
    PTRACE(2, "MonSock\tClose failed for UDP socket " << info.socket);
#endif

  // This is pretty ugly, but needed to make sure multi-threading doesn't crash
  unsigned failSafe = 100; // Approx. two seconds
  while (info.inUse) {
    UnlockReadWrite();
    PThread::Sleep(20);
    if (!LockReadWrite())
      return false;
    if (--failSafe == 0) {
      PTRACE(1, "MonSock\tRead thread break for UDP socket " << info.socket << " taking too long.");
      break;
    }
  }

  PTRACE(4, "MonSock\tDeleting UDP socket " << info.socket);
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

  return usingNAT ? info.socket->GetLocalAddress(address, port)
                  : info.socket->PUDPSocket::GetLocalAddress(address, port);
}


PChannel::Errors PMonitoredSockets::WriteToSocket(const void * buf,
                                                  PINDEX len,
                                                  const PIPSocket::Address & addr,
                                                  WORD port,
                                                  const SocketInfo & info,
                                                  PINDEX & lastWriteCount)
{
  info.socket->WriteTo(buf, len, addr, port);
  lastWriteCount = info.socket->GetLastWriteCount();
  return info.socket->GetErrorCode(PChannel::LastWriteError);
}


PChannel::Errors PMonitoredSockets::ReadFromSocket(PSocket::SelectList & readers,
                                                   PUDPSocket * & socket,
                                                   void * buf,
                                                   PINDEX len,
                                                   PIPSocket::Address & addr,
                                                   WORD & port,
                                                   PINDEX & lastReadCount,
                                                   const PTimeInterval & timeout)
{
  // Assume is already locked

  socket = NULL;
  lastReadCount = 0;

  UnlockReadWrite();

  PChannel::Errors errorCode = PSocket::Select(readers, timeout);

  if (!LockReadWrite())
    return PChannel::NotOpen;

  if (!opened)
    return PChannel::NotOpen; // Closed, break out

  switch (errorCode) {
    case PChannel::NoError :
      break;

    case PChannel::NotOpen : // Interface went down
      if (!interfaceAddedSignal.IsOpen()) {
        interfaceAddedSignal.Listen(); // Reset if this was used to break Select() block
        return PChannel::Interrupted;
      }
      // Do next case

    default :
      return errorCode;
  }

  if (readers.IsEmpty())
    return PChannel::Timeout;

  socket = (PUDPSocket *)&readers.front();

  if (socket->ReadFrom(buf, len, addr, port)) {
    lastReadCount = socket->GetLastReadCount();
    return PChannel::NoError;
  }

  switch (socket->GetErrorNumber(PChannel::LastReadError)) {
    case ECONNRESET :
    case ECONNREFUSED :
      PTRACE(2, "MonSock\tUDP Port on remote not ready.");
      return PChannel::NoError;

    case EMSGSIZE :
      PTRACE(2, "MonSock\tRead UDP packet too large for buffer of " << len << " bytes.");
      return PChannel::NoError;

    case EBADF : // Interface went down
    case EINTR :
    case EAGAIN : // Shouldn't happen, but it does.
      return PChannel::Interrupted;
  }

  PTRACE(1, "MonSock\tSocket read UDP error ("
         << socket->GetErrorNumber(PChannel::LastReadError) << "): "
         << socket->GetErrorText(PChannel::LastReadError));
  return socket->GetErrorCode(PChannel::LastReadError); // Exit loop
}


PChannel::Errors PMonitoredSockets::ReadFromSocket(SocketInfo & info,
                                                   void * buf,
                                                   PINDEX len,
                                                   PIPSocket::Address & addr,
                                                   WORD & port,
                                                   PINDEX & lastReadCount,
                                                   const PTimeInterval & timeout)
{
  // Assume is already locked

  if (info.inUse) {
    PTRACE(2, "MonSock\tCannot read from multiple threads.");
    return PChannel::DeviceInUse;
  }

  lastReadCount = 0;

  PChannel::Errors errorCode;

  do {
    PSocket::SelectList sockets;
    if (info.socket == NULL || !info.socket->IsOpen())
      info.inUse = false; // socket closed by monitor thread. release the inUse flag
    else {
      sockets += *info.socket;
      info.inUse = true;
    }
    sockets += interfaceAddedSignal;

    PUDPSocket * socket;
    errorCode = ReadFromSocket(sockets, socket, buf, len, addr, port, lastReadCount, timeout);
  } while (errorCode == PChannel::NoError && lastReadCount == 0);

  info.inUse = false;
  return errorCode;
}


PMonitoredSockets * PMonitoredSockets::Create(const PString & iface, bool reuseAddr, PSTUNClient * stunClient)
{
  if (iface.IsEmpty() || iface == "*" || PIPSocket::Address(iface).IsAny())
    return new PMonitoredSocketBundle(reuseAddr, stunClient);
  else
    return new PSingleMonitoredSocket(iface, reuseAddr, stunClient);
}


void PMonitoredSockets::OnRemoveSTUNClient(const PSTUNClient *_stun)
{
  if (stun == _stun)
    stun = NULL;
}


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
    if (!SetErrorValues(socketBundle->ReadFromBundle(buffer, length,
                                                     lastReceivedAddress, lastReceivedPort,
                                                     lastReceivedInterface, lastReadCount, readTimeout),
                        0, LastReadError))
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
  return IsOpen() &&
         SetErrorValues(socketBundle->WriteToBundle(buffer, length,
                                                    remoteAddress, remotePort,
                                                    GetInterface(), lastWriteCount),
                        0, LastWriteError);
}


void PMonitoredSocketChannel::SetInterface(const PString & iface)
{
  PIPSocket::InterfaceEntry info;
  if (socketBundle != NULL && socketBundle->GetInterfaceInfo(iface, info))
    currentInterface = MakeInterfaceDescription(info);
  else
    currentInterface = iface;

  if (lastReceivedInterface.IsEmpty())
    lastReceivedInterface = currentInterface;
}


const PString & PMonitoredSocketChannel::GetInterface()
{
  if (currentInterface.Find('%') == P_MAX_INDEX)
    SetInterface(currentInterface);

  return currentInterface;
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

PMonitoredSocketBundle::PMonitoredSocketBundle(bool reuseAddr, PSTUNClient * stunClient)
  : PMonitoredSockets(reuseAddr, stunClient)
{
}


PMonitoredSocketBundle::~PMonitoredSocketBundle()
{
  Close();
}


PBoolean PMonitoredSocketBundle::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (IsOpen() && localPort != 0  && localPort == port)
    return true;

  opened = true;

  localPort = port;

  // Close and re-open all sockets
  while (!socketInfoMap.empty())
    CloseSocket(socketInfoMap.begin());

  PStringArray interfaces = GetInterfaces();
  for (PINDEX i = 0; i < interfaces.GetSize(); ++i)
    OpenSocket(interfaces[i]);

  return true;
}


PBoolean PMonitoredSocketBundle::Close()
{
  if (!LockReadWrite())
    return false;

  opened = false;

  while (!socketInfoMap.empty())
    CloseSocket(socketInfoMap.begin());
  interfaceAddedSignal.Close(); // Fail safe break out of Select()

  UnlockReadWrite();

  return true;
}


PBoolean PMonitoredSocketBundle::GetAddress(const PString & iface,
                                        PIPSocket::Address & address,
                                        WORD & port,
                                        PBoolean usingNAT) const
{
  if (iface.IsEmpty()) {
    address = PIPSocket::GetDefaultIpAny();
    port = localPort;
    return true;
  }

  PSafeLockReadOnly guard(*this);
  if (!guard.IsLocked())
    return false;

  SocketInfoMap_T::const_iterator iter = socketInfoMap.find(iface);
  return iter != socketInfoMap.end() && GetSocketAddress(iter->second, address, port, usingNAT);
}


void PMonitoredSocketBundle::OpenSocket(const PString & iface)
{
  PIPSocket::Address binding;
  PString name;
  SplitInterfaceDescription(iface, binding, name);

  SocketInfo info;
  if (CreateSocket(info, binding)) {
    if (localPort == 0)
      localPort = info.socket->GetPort();
    socketInfoMap[iface] = info;
  }
}


void PMonitoredSocketBundle::CloseSocket(SocketInfoMap_T::iterator iterSocket)
{
  //Already locked by caller

  if (iterSocket == socketInfoMap.end())
    return;

  DestroySocket(iterSocket->second);
  socketInfoMap.erase(iterSocket);
}


PChannel::Errors PMonitoredSocketBundle::WriteToBundle(const void * buf,
                                                       PINDEX len,
                                                       const PIPSocket::Address & addr,
                                                       WORD port,
                                                       const PString & iface,
                                                       PINDEX & lastWriteCount)
{
  if (!LockReadWrite())
    return PChannel::NotOpen;

  PChannel::Errors errorCode = PChannel::NoError;

  if (iface.IsEmpty()) {
    for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
      PChannel::Errors err = WriteToSocket(buf, len, addr, port, iter->second, lastWriteCount);
      if (err != PChannel::NoError)
        errorCode = err;
    }
  }
  else {
    SocketInfoMap_T::iterator iter = socketInfoMap.find(iface);
    if (iter != socketInfoMap.end())
      errorCode = WriteToSocket(buf, len, addr, port, iter->second, lastWriteCount);
    else
      errorCode = PChannel::NotFound;
  }

  UnlockReadWrite();

  return errorCode;
}


PChannel::Errors PMonitoredSocketBundle::ReadFromBundle(void * buf,
                                                        PINDEX len,
                                                        PIPSocket::Address & addr,
                                                        WORD & port,
                                                        PString & iface,
                                                        PINDEX & lastReadCount,
                                                        const PTimeInterval & timeout)
{
  if (!opened)
    return PChannel::NotOpen;

  if (!LockReadWrite())
    return PChannel::NotOpen;

  PChannel::Errors errorCode = PChannel::NotFound;

  if (iface.IsEmpty()) {
    do {
      // If interface is empty, then grab the next datagram on any of the interfaces
      PSocket::SelectList readers;

      for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
        if (iter->second.inUse) {
          PTRACE(2, "MonSock\tCannot read from multiple threads.");
          UnlockReadWrite();
          return PChannel::DeviceInUse;
        }
        if (iter->second.socket->IsOpen()) {
          readers += *iter->second.socket;
          iter->second.inUse = true;
        }
      }
      readers += interfaceAddedSignal;

      PUDPSocket * socket;
      errorCode = ReadFromSocket(readers, socket, buf, len, addr, port, lastReadCount, timeout);

      for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
        if (iter->second.socket == socket)
          iface = iter->first;
        iter->second.inUse = false;
      }
    } while (errorCode == PChannel::NoError && lastReadCount == 0);
  }
  else {
    // if interface is not empty, use that specific interface
    SocketInfoMap_T::iterator iter = socketInfoMap.find(iface);
    if (iter != socketInfoMap.end())
      errorCode = ReadFromSocket(iter->second, buf, len, addr, port, lastReadCount, timeout);
    else
      errorCode = PChannel::NotFound;
  }

  UnlockReadWrite();

  return errorCode;
}


void PMonitoredSocketBundle::OnAddInterface(const InterfaceEntry & entry)
{
  // Already locked
  if (opened) {
    OpenSocket(MakeInterfaceDescription(entry));
    PTRACE(3, "MonSock\tUDP socket bundle has added interface " << entry);
    interfaceAddedSignal.Close();
  }
}


void PMonitoredSocketBundle::OnRemoveInterface(const InterfaceEntry & entry)
{
  // Already locked
  if (opened) {
    CloseSocket(socketInfoMap.find(MakeInterfaceDescription(entry)));
    PTRACE(3, "MonSock\tUDP socket bundle has removed interface " << entry);
  }
}


//////////////////////////////////////////////////

PSingleMonitoredSocket::PSingleMonitoredSocket(const PString & _theInterface, bool reuseAddr, PSTUNClient * stunClient)
  : PMonitoredSockets(reuseAddr, stunClient)
  , theInterface(_theInterface)
{
}


PSingleMonitoredSocket::~PSingleMonitoredSocket()
{
  Close();
}


PStringArray PSingleMonitoredSocket::GetInterfaces(PBoolean /*includeLoopBack*/, const PIPSocket::Address & /*destination*/)
{
  PSafeLockReadOnly guard(*this);

  PStringList names;
  if (!theEntry.GetAddress().IsAny())
    names.AppendString(MakeInterfaceDescription(theEntry));
  return names;
}


PBoolean PSingleMonitoredSocket::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (opened && theInfo.socket != NULL && theInfo.socket->IsOpen())
    return false;

  opened = true;

  localPort = port;

  if (theEntry.GetAddress().IsAny())
    GetInterfaceInfo(theInterface, theEntry);

  if (theEntry.GetAddress().IsAny())
    return true;

  if (!CreateSocket(theInfo, theEntry.GetAddress()))
    return false;

  localPort = theInfo.socket->PUDPSocket::GetPort();
  return true;
}


PBoolean PSingleMonitoredSocket::Close()
{
  PSafeLockReadWrite guard(*this);

  opened = false;
  interfaceAddedSignal.Close(); // Fail safe break out of Select()
  return DestroySocket(theInfo);
}


PBoolean PSingleMonitoredSocket::GetAddress(const PString & iface,
                                        PIPSocket::Address & address,
                                        WORD & port,
                                        PBoolean usingNAT) const
{
  PSafeLockReadOnly guard(*this);

  return guard.IsLocked() && IsInterface(iface) && GetSocketAddress(theInfo, address, port, usingNAT);
}


PChannel::Errors PSingleMonitoredSocket::WriteToBundle(const void * buf,
                                                       PINDEX len,
                                                       const PIPSocket::Address & addr,
                                                       WORD port,
                                                       const PString & iface,
                                                       PINDEX & lastWriteCount)
{
  PSafeLockReadWrite guard(*this);

  if (guard.IsLocked() && theInfo.socket != NULL && IsInterface(iface))
    return WriteToSocket(buf, len, addr, port, theInfo, lastWriteCount);

  return PChannel::NotFound;
}


PChannel::Errors PSingleMonitoredSocket::ReadFromBundle(void * buf,
                                                        PINDEX len,
                                                        PIPSocket::Address & addr,
                                                        WORD & port,
                                                        PString & iface,
                                                        PINDEX & lastReadCount,
                                                        const PTimeInterval & timeout)
{
  if (!opened)
    return PChannel::NotOpen;

  if (!LockReadWrite())
    return PChannel::NotOpen;

  PChannel::Errors errorCode;
  if (IsInterface(iface))
    errorCode = ReadFromSocket(theInfo, buf, len, addr, port, lastReadCount, timeout);
  else
    errorCode = PChannel::NotFound;

  iface = theInterface;

  UnlockReadWrite();

  return errorCode;
}


void PSingleMonitoredSocket::OnAddInterface(const InterfaceEntry & entry)
{
  // Already locked

  PIPSocket::Address addr;
  PString name;
  if (!SplitInterfaceDescription(theInterface, addr, name))
    return;

  if (entry.GetAddress() == addr && entry.GetName().NumCompare(name) == EqualTo) {
    theEntry = entry;
    if (!Open(localPort))
      theEntry = InterfaceEntry();
    else {
      interfaceAddedSignal.Close();
      PTRACE(3, "MonSock\tBound UDP socket UP event on interface " << theEntry);
    }
  }
}


void PSingleMonitoredSocket::OnRemoveInterface(const InterfaceEntry & entry)
{
  // Already locked

  if (entry != theEntry)
    return;

  PTRACE(3, "MonSock\tBound UDP socket DOWN event on interface " << theEntry);
  theEntry = InterfaceEntry();
  DestroySocket(theInfo);
}


bool PSingleMonitoredSocket::IsInterface(const PString & iface) const
{
  if (iface.IsEmpty())
    return true;

  PINDEX percent1 = iface.Find('%');
  PINDEX percent2 = theInterface.Find('%');

  if (percent1 != P_MAX_INDEX && percent2 != P_MAX_INDEX)
    return iface.Mid(percent1+1).NumCompare(theInterface.Mid(percent2+1)) == EqualTo;

  return PIPSocket::Address(iface.Left(percent1)) == PIPSocket::Address(theInterface.Left(percent2));
}
