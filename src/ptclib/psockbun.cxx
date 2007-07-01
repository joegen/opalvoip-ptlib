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
 * $Log: psockbun.cxx,v $
 * Revision 1.8  2007/07/01 15:21:21  dsandras
 * Fixed potential problems with MJPEG-based cameras thanks Matthias
 * Schneider <ma30002000 yahoo de>. Thanks a lot!
 *
 * Revision 1.7  2007/06/25 05:44:01  rjongbloed
 * Fixed numerous issues with "bound" managed socket, ie associating
 *   listeners to a specific named interface.
 *
 * Revision 1.6  2007/06/22 04:51:40  rjongbloed
 * Fixed missing mutex release in socket bundle interface monitor thread shut down.
 *
 * Revision 1.5  2007/06/17 03:17:52  rjongbloed
 * Added using empty interface string as "just use predefined fixed interface"
 *
 * Revision 1.4  2007/06/10 06:26:54  rjongbloed
 * Major enhancements to the "socket bundling" feature:
 *   singleton thread for monitoring network interfaces
 *   a generic API for anything to be informed of interface changes
 *   PChannel derived class for reading/writing to bundled sockets
 *   many new API functions
 *
 * Revision 1.3  2007/05/28 11:26:50  hfriederich
 * Fix compilation
 *
 * Revision 1.2  2007/05/22 11:50:57  csoutheren
 * Further implementation of socket bundle
 *
 * Revision 1.1  2007/05/21 06:07:17  csoutheren
 * Add new socket bundle code to be used to OpalUDPListener
 *
 */

//////////////////////////////////////////////////

#ifdef __GNUC__
#pragma implementation "psockbun.h"
#endif

#include <ptlib.h>
#include <ptclib/psockbun.h>
#include <ptclib/pstun.h>


//////////////////////////////////////////////////

PInterfaceMonitorClient::PInterfaceMonitorClient()
{
  PInterfaceMonitor::GetInstance().AddClient(this);
}


PInterfaceMonitorClient::~PInterfaceMonitorClient()
{
  PInterfaceMonitor::GetInstance().RemoveClient(this);
}


PStringArray PInterfaceMonitorClient::GetInterfaces(BOOL includeLoopBack)
{
  return PInterfaceMonitor::GetInstance().GetInterfaces(includeLoopBack);
}


BOOL PInterfaceMonitorClient::GetInterfaceInfo(const PString & iface, InterfaceEntry & info)
{
  return PInterfaceMonitor::GetInstance().GetInterfaceInfo(iface, info);
}


//////////////////////////////////////////////////

static PMutex PInterfaceMonitorInstanceMutex;
static PInterfaceMonitor * PInterfaceMonitorInstance;

PInterfaceMonitor::PInterfaceMonitor(unsigned refresh)
  : refreshInterval(refresh)
  , updateThread(NULL)
{
  PInterfaceMonitorInstanceMutex.Wait();
  PAssert(PInterfaceMonitorInstance == NULL, PLogicError);
  PInterfaceMonitorInstance = this;
  PInterfaceMonitorInstanceMutex.Signal();
}


PInterfaceMonitor::~PInterfaceMonitor()
{
  Stop();
}


PInterfaceMonitor & PInterfaceMonitor::GetInstance()
{
  PInterfaceMonitorInstanceMutex.Wait();
  if (PInterfaceMonitorInstance == NULL) {
    static PInterfaceMonitor theInstance;
  }
  PInterfaceMonitorInstanceMutex.Signal();

  return *PInterfaceMonitorInstance;
}


BOOL PInterfaceMonitor::Start()
{
  PWaitAndSignal m(mutex);

  if (updateThread != NULL)
    return FALSE;

  PIPSocket::GetInterfaceTable(currentInterfaces);
  updateThread = new PThreadObj<PInterfaceMonitor>(*this, &PInterfaceMonitor::UpdateThreadMain);
  return TRUE;
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


static BOOL IsInterfaceInList(const PIPSocket::InterfaceEntry & entry,
                              const PIPSocket::InterfaceTable & list)
{
  for (PINDEX i = 0; i < list.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & listEntry = list[i];
    if ((entry.GetName() == listEntry.GetName()) && (entry.GetAddress() == listEntry.GetAddress()))
      return TRUE;
  }
  return FALSE;
}


static BOOL InterfaceListIsSubsetOf(const PIPSocket::InterfaceTable & subset,
                                    const PIPSocket::InterfaceTable & set)
{
  for (PINDEX i = 0; i < subset.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = subset[i];
    if (!IsInterfaceInList(entry, set))
      return FALSE;
  }

  return TRUE;
}


static BOOL CompareInterfaceLists(const PIPSocket::InterfaceTable & list1,
                                  const PIPSocket::InterfaceTable & list2)
{
  // if the sizes are different, then the list has changed. 
  if (list1.GetSize() != list2.GetSize())
    return FALSE;

  // ensure every element in list1 is in list2
  if (!InterfaceListIsSubsetOf(list1, list2))
    return FALSE;

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

    // look for interfaces to add that are in new list that are not in the old list
    PINDEX i;
    for (i = 0; i < newInterfaces.GetSize(); ++i) {
      PIPSocket::InterfaceEntry & newEntry = newInterfaces[i];
      if (!newEntry.GetAddress().IsLoopback() && !IsInterfaceInList(newEntry, oldInterfaces))
        OnAddInterface(newEntry);
    }

    // look for interfaces to remove that are in old list that are not in the new list
    for (i = 0; i < oldInterfaces.GetSize(); ++i) {
      PIPSocket::InterfaceEntry & oldEntry = oldInterfaces[i];
      if (!oldEntry.GetAddress().IsLoopback() && !IsInterfaceInList(oldEntry, newInterfaces))
        OnRemoveInterface(oldEntry);
    }
  }
}


void PInterfaceMonitor::UpdateThreadMain()
{
  // check for interface changes periodically
  do {
    RefreshInterfaceList();
  } while (!threadRunning.Wait(refreshInterval));
}


static PString MakeInterfaceDescription(const PIPSocket::InterfaceEntry & entry)
{
  return entry.GetAddress().AsString() + '%' + entry.GetName();
}


static BOOL SplitInterfaceDescription(const PString & iface,
                                      PIPSocket::Address & address,
                                      PString & name)
{
  if (iface.IsEmpty())
    return FALSE;

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


PStringArray PInterfaceMonitor::GetInterfaces(BOOL includeLoopBack)
{
  PWaitAndSignal guard(mutex);

  PStringArray names;

  names.SetSize(currentInterfaces.GetSize());
  PINDEX count = 0;

  for (PINDEX i = 0; i < currentInterfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = currentInterfaces[i];
    if (includeLoopBack || !entry.GetAddress().IsLoopback())
      names[count++] = MakeInterfaceDescription(entry);
  }

  names.SetSize(count);

  return names;
}


BOOL PInterfaceMonitor::GetInterfaceInfo(const PString & iface, PIPSocket::InterfaceEntry & info)
{
  PIPSocket::Address addr;
  PString name;
  if (!SplitInterfaceDescription(iface, addr, name))
    return FALSE;

  PWaitAndSignal m(mutex);

  for (PINDEX i = 0; i < currentInterfaces.GetSize(); ++i) {
    PIPSocket::InterfaceEntry & entry = currentInterfaces[i];
    if ((addr.IsAny()   || entry.GetAddress() == addr) &&
        (name.IsEmpty() || entry.GetName().NumCompare(name) == EqualTo)) {
      info = entry;
      return TRUE;
    }
  }

  return FALSE;
}


void PInterfaceMonitor::AddClient(PInterfaceMonitorClient * client)
{
  PWaitAndSignal m(mutex);

  if (currentClients.empty())
    Start();
  currentClients.push_back(client);
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


void PInterfaceMonitor::OnAddInterface(const PIPSocket::InterfaceEntry & entry)
{
  PWaitAndSignal m(mutex);

  for (ClientList_T::iterator iter = currentClients.begin(); iter != currentClients.end(); ++iter) {
    PInterfaceMonitorClient * client = *iter;
    if (client->LockReadWrite()) {
      client->OnAddInterface(entry);
      client->UnlockReadWrite();
    }
  }
}


void PInterfaceMonitor::OnRemoveInterface(const PIPSocket::InterfaceEntry & entry)
{
  PWaitAndSignal m(mutex);

  for (ClientList_T::iterator iter = currentClients.begin(); iter != currentClients.end(); ++iter) {
    PInterfaceMonitorClient * client = *iter;
    if (client->LockReadWrite()) {
      client->OnRemoveInterface(entry);
      client->UnlockReadWrite();
    }
  }
}


//////////////////////////////////////////////////

PMonitoredSockets::PMonitoredSockets(BOOL reuseAddr)
  : localPort(0)
  , reuseAddress(reuseAddr)
  , stun(NULL)
{
}


BOOL PMonitoredSockets::CreateSocket(SocketInfo & info)
{
  delete info.socket;

  if (stun == NULL)
  {
    info.socket = new PUDPSocket(localPort);
    return TRUE;
  }

  if (!stun->CreateSocket(info.socket)) {
    PTRACE(4, "OpalUDP\tSTUN could not create socket!");
    return FALSE;
  }

  return TRUE;
}


BOOL PMonitoredSockets::DestroySocket(SocketInfo & info)
{
  if (info.socket == NULL)
    return FALSE;

  BOOL result = info.socket->Close();

  // This is pretty ugly, but needed to make sure multi-threading doesn't crash
  while (info.usageCount > 0) {
    UnlockReadWrite();
    PThread::Sleep(10);
    if (!LockReadWrite())
      return FALSE;
  }

  delete info.socket;
  info.socket = NULL;

  return result;
}


BOOL PMonitoredSockets::WriteToSocket(const void * buf,
                                      PINDEX len,
                                      const PIPSocket::Address & addr,
                                      WORD port,
                                      const SocketInfo & info,
                                      PINDEX & lastWriteCount)
{
#ifndef __BEOS__
  if (addr.IsBroadcast()) {
    if (!info.socket->SetOption(SO_BROADCAST, 1)) {
      PTRACE(2, "OpalUDP\tError allowing broadcast: " << info.socket->GetErrorText());
      return FALSE;
    }
  }
#else
  PTRACE(3, "RAS\tBroadcast option under BeOS is not implemented yet");
#endif

  BOOL ok = info.socket->WriteTo(buf, len, addr, port);

#ifndef __BEOS__
  if (addr.IsBroadcast())
    info.socket->SetOption(SO_BROADCAST, 0);
#endif

  lastWriteCount = info.socket->GetLastWriteCount();
  return ok;
}


PMonitoredSockets * PMonitoredSockets::Create(const PString & iface, BOOL reuseAddr)
{
  if (iface.IsEmpty() || iface == "*" || PIPSocket::Address(iface).IsAny())
    return new PMonitoredSocketBundle(reuseAddr);
  else
    return new PSingleMonitoredSocket(iface, reuseAddr);
}


//////////////////////////////////////////////////

PMonitoredSocketChannel::PMonitoredSocketChannel(const PMonitoredSocketsPtr & sock)
  : socketBundle(sock)
  , promiscuousReads(false)
  , closing(FALSE)
  , remotePort(0)
  , lastReceivedAddress(PIPSocket::GetDefaultIpAny())
  , lastReceivedPort(0)
{
}


BOOL PMonitoredSocketChannel::IsOpen() const
{
  return !closing && socketBundle != NULL && socketBundle->IsOpen();
}


BOOL PMonitoredSocketChannel::Close()
{
  closing = TRUE;
  return TRUE;
}


BOOL PMonitoredSocketChannel::Read(void * buffer, PINDEX length)
{
  if (!IsOpen())
    return FALSE;

  do {
    PString iface = GetInterface();
    if (!socketBundle->ReadFrom(buffer, length, lastReceivedAddress, lastReceivedPort, iface, lastReadCount, readTimeout))
      return FALSE;

    if (promiscuousReads)
      return TRUE;

    if (remoteAddress.IsAny())
      remoteAddress = lastReceivedAddress;
    if (remotePort == 0)
      remotePort = lastReceivedPort;

  } while (remoteAddress != lastReceivedAddress || remotePort != lastReceivedPort);
  return TRUE;
}


BOOL PMonitoredSocketChannel::Write(const void * buffer, PINDEX length)
{
  return socketBundle != NULL && socketBundle->WriteTo(buffer, length, remoteAddress, remotePort, GetInterface(), lastWriteCount);
}


void PMonitoredSocketChannel::SetInterface(const PString & iface)
{
  PIPSocket::InterfaceEntry info;
  if (socketBundle != NULL && socketBundle->GetInterfaceInfo(iface, info))
    currentInterface = MakeInterfaceDescription(info);
  else
    currentInterface = iface;
}


const PString & PMonitoredSocketChannel::GetInterface()
{
  if (currentInterface.Find('%') == P_MAX_INDEX)
    SetInterface(currentInterface);

  return currentInterface;
}


BOOL PMonitoredSocketChannel::GetLocal(PIPSocket::Address & address, WORD & port)
{
  port = socketBundle->GetPort();
  return socketBundle->GetAddress(GetInterface(), address);
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

PMonitoredSocketBundle::PMonitoredSocketBundle(BOOL reuseAddr)
  : PMonitoredSockets(reuseAddr)
  , closing(FALSE)
{
}


PMonitoredSocketBundle::~PMonitoredSocketBundle()
{
  Close();
}


BOOL PMonitoredSocketBundle::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (IsOpen() && localPort != 0  && localPort == port)
    return TRUE;

  closing = FALSE;

  localPort = port;

  // Close and re-open all sockets
  while (!socketInfoMap.empty())
    CloseSocket(socketInfoMap.begin());

  PStringArray interfaces = GetInterfaces();
  for (PINDEX i = 0; i < interfaces.GetSize(); ++i)
    OpenSocket(interfaces[i]);

  return !socketInfoMap.empty();
}


BOOL PMonitoredSocketBundle::IsOpen() const
{
  PSafeLockReadOnly guard(*this);
  return !closing && !socketInfoMap.empty();
}


BOOL PMonitoredSocketBundle::Close()
{
  if (!LockReadWrite())
    return FALSE;

  closing = TRUE;

  while (!socketInfoMap.empty())
    CloseSocket(socketInfoMap.begin());

  UnlockReadWrite();

  return TRUE;
}


BOOL PMonitoredSocketBundle::GetAddress(const PString & iface, PIPSocket::Address & address) const
{
  PSafeLockReadOnly guard(*this);

  SocketInfoMap_T::const_iterator iter = socketInfoMap.find(iface);
  if (iter != socketInfoMap.end()) 
    return iter->second.socket->GetLocalAddress(address);

  address = PIPSocket::GetDefaultIpAny();
  return FALSE;
}


void PMonitoredSocketBundle::OpenSocket(const PString & iface)
{
  PIPSocket::Address binding;
  PString name;
  SplitInterfaceDescription(iface, binding, name);

  SocketInfo info;
  CreateSocket(info);
  if (!info.socket->Listen(binding, 0, localPort, reuseAddress?PIPSocket::CanReuseAddress:PIPSocket::AddressIsExclusive)) 
    delete info.socket;
  else {
    if (localPort == 0)
      localPort = info.socket->GetPort();
    socketInfoMap[iface] = info;
  }
}


void PMonitoredSocketBundle::CloseSocket(const SocketInfoMap_T::iterator & iterSocket)
{
  //Already locked by caller

  if (iterSocket == socketInfoMap.end())
    return;

  DestroySocket(iterSocket->second);
  socketInfoMap.erase(iterSocket);
}


BOOL PMonitoredSocketBundle::WriteTo(const void * buf,
                                     PINDEX len,
                                     const PIPSocket::Address & addr,
                                     WORD port,
                                     const PString & iface,
                                     PINDEX & lastWriteCount)
{
  if (!LockReadWrite())
    return FALSE;

  BOOL ok = TRUE;

  if (iface.IsEmpty()) {
    for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
      if (!WriteToSocket(buf, len, addr, port, iter->second, lastWriteCount))
        ok = FALSE;
    }
  }
  else {
    SocketInfoMap_T::iterator iter = socketInfoMap.find(iface);
    ok = iter != socketInfoMap.end() && WriteToSocket(buf, len, addr, port, iter->second, lastWriteCount);
  }

  UnlockReadWrite();

  return ok;
}


BOOL PMonitoredSocketBundle::ReadFrom(void * buf,
                                      PINDEX len,
                                      PIPSocket::Address & addr,
                                      WORD & port,
                                      PString & iface,
                                      PINDEX & lastReadCount,
                                      const PTimeInterval & timeout)
{
  BOOL ok = FALSE;

  if (!LockReadWrite())
    return FALSE;

  if (iface.IsEmpty()) {
    // If interface is empty, then grab the next datagram on any of the interfaces
    PSocket::SelectList readers;

    for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
      readers += *iter->second.socket;
      iter->second.usageCount++;
    }

    UnlockReadWrite();
    if (!readers.IsEmpty())
      ok = PSocket::Select(readers, timeout) == PChannel::NoError;
    if (!LockReadWrite())
      return FALSE;

    PUDPSocket * socket = NULL;
    if (ok) {
      socket = (PUDPSocket *)&readers[0];
      ok = socket->ReadFrom(buf, len, addr, port);
      lastReadCount = socket->GetLastReadCount();
    }

    for (SocketInfoMap_T::iterator iter = socketInfoMap.begin(); iter != socketInfoMap.end(); ++iter) {
      if (iter->second.socket == socket)
        iface = iter->first;
      iter->second.usageCount--;
    }
  }
  else {
  // if interface is not empty, use that specific interface
    SocketInfoMap_T::iterator iter = socketInfoMap.find(iface);
    if (iter != socketInfoMap.end()) {
      iter->second.usageCount++;

      iter->second.socket->SetReadTimeout(timeout);

      UnlockReadWrite();
      ok = iter->second.socket->ReadFrom(buf, len, addr, port);
      if (!LockReadWrite())
        return FALSE;

      lastReadCount = iter->second.socket->GetLastReadCount();

      iter->second.usageCount--;
    }
  }

  UnlockReadWrite();

  return ok;
}


void PMonitoredSocketBundle::OnAddInterface(const InterfaceEntry & entry)
{
  // Already locked
  if (closing)
    return;

  OpenSocket(MakeInterfaceDescription(entry));
}


void PMonitoredSocketBundle::OnRemoveInterface(const InterfaceEntry & entry)
{
  // Already locked
  if (closing)
    return;

  CloseSocket(socketInfoMap.find(MakeInterfaceDescription(entry)));
}


//////////////////////////////////////////////////

PSingleMonitoredSocket::PSingleMonitoredSocket(const PString & _theInterface, BOOL reuseAddr)
  : PMonitoredSocketBundle(reuseAddr)
  , theInterface(_theInterface)
{
}


PSingleMonitoredSocket::~PSingleMonitoredSocket()
{
  DestroySocket(theInfo);
}


PStringArray PSingleMonitoredSocket::GetInterfaces(BOOL /*includeLoopBack*/)
{
  PSafeLockReadOnly guard(*this);

  PStringList names;
  if (!theEntry.GetAddress().IsAny())
    names.AppendString(MakeInterfaceDescription(theEntry));
  return names;
}


BOOL PSingleMonitoredSocket::Open(WORD port)
{
  PSafeLockReadWrite guard(*this);

  if (theEntry.GetAddress().IsAny()) {
    if (!GetInterfaceInfo(theInterface, theEntry))
      return FALSE;
  }

  if (theInfo.socket == NULL) {
    if (!CreateSocket(theInfo))
      return FALSE;
  }

  if (!theInfo.socket->Listen(theEntry.GetAddress(), 0, port, reuseAddress?PIPSocket::CanReuseAddress:PIPSocket::AddressIsExclusive))
    return FALSE;

  localPort = theInfo.socket->GetPort();
  return TRUE;
}


BOOL PSingleMonitoredSocket::IsOpen() const
{
  PSafeLockReadOnly guard(*this);

  return theInfo.socket != NULL && theInfo.socket->IsOpen();
}


BOOL PSingleMonitoredSocket::Close()
{
  PSafeLockReadWrite guard(*this);

  return DestroySocket(theInfo);
}


BOOL PSingleMonitoredSocket::GetAddress(const PString & iface, PIPSocket::Address & address) const
{
  PSafeLockReadOnly guard(*this);

  if (guard.IsLocked() && theInfo.socket != NULL && IsInterface(iface))
    return theInfo.socket->GetLocalAddress(address);

  address = PIPSocket::GetDefaultIpAny();
  return FALSE;
}


BOOL PSingleMonitoredSocket::WriteTo(const void * buf,
                                     PINDEX len,
                                     const PIPSocket::Address & addr,
                                     WORD port,
                                     const PString & iface,
                                     PINDEX & lastWriteCount)
{
  PSafeLockReadWrite guard(*this);

  if (guard.IsLocked() && theInfo.socket != NULL && IsInterface(iface))
    return WriteToSocket(buf, len, addr, port, theInfo, lastWriteCount);

  return FALSE;
}


BOOL PSingleMonitoredSocket::ReadFrom(void * buf,
                                      PINDEX len,
                                      PIPSocket::Address & addr,
                                      WORD & port,
                                      PString & iface,
                                      PINDEX & lastReadCount,
                                      const PTimeInterval & timeout)
{
  if (!LockReadWrite())
    return FALSE;

  BOOL ok = FALSE;

  if (IsInterface(iface)) {
    theInfo.usageCount++;

    theInfo.socket->SetReadTimeout(timeout);

    UnlockReadWrite();
    ok = theInfo.socket->ReadFrom(buf, len, addr, port);
    if (!LockReadWrite())
      return FALSE;

    lastReadCount = theInfo.socket->GetLastReadCount();

    theInfo.usageCount--;
  }

  UnlockReadWrite();

  return ok;
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
  }
}


void PSingleMonitoredSocket::OnRemoveInterface(const InterfaceEntry & entry)
{
  // Already locked

  if (entry != theEntry)
    return;

  theEntry = InterfaceEntry();
  theInfo.socket->Close();
  delete theInfo.socket;
  theInfo.socket = NULL;
}


BOOL PSingleMonitoredSocket::IsInterface(const PString & iface) const
{
  if (iface.IsEmpty())
    return TRUE;

  PINDEX percent1 = iface.Find('%');
  PINDEX percent2 = theInterface.Find('%');

  if (percent1 != P_MAX_INDEX && percent2 != P_MAX_INDEX)
    return iface.Mid(percent1+1).NumCompare(theInterface.Mid(percent2+1)) == EqualTo;

  return PIPSocket::Address(iface.Left(percent1)) == PIPSocket::Address(theInterface.Left(percent2));
}
