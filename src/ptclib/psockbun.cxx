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

PInterfaceBundle::PInterfaceBundle(PINDEX _refreshInterval)
  : refreshInterval(_refreshInterval), updateThread(NULL)
{
}

PInterfaceBundle::~PInterfaceBundle()
{
  Close();
}

BOOL PInterfaceBundle::Open()
{
  if (updateThread != NULL)
    return FALSE;

  PIPSocket::GetInterfaceTable(currentInterfaces);

  // add all interfaces
  {
    PINDEX i;
    for (i = 0; i < currentInterfaces.GetSize(); ++i) {
      InterfaceEntry & entry = currentInterfaces[i];
      OnAddInterface(entry);
    }
  }

  threadRunning = TRUE;
  updateThread = new PThreadObj<PInterfaceBundle>(*this, &PInterfaceBundle::UpdateThreadMain);
  return TRUE;
}

BOOL PInterfaceBundle::IsOpen()
{
  return updateThread != NULL;
}


void PInterfaceBundle::Close()
{
  // shutdown the update thread
  threadRunning = FALSE;
  updateThread->WaitForTermination();
  delete updateThread;
  updateThread = NULL;

  // delete the entries in the interface list
  while (currentInterfaces.GetSize() > 0) {
    InterfaceEntry & entry = currentInterfaces[0];
    OnRemoveInterface(entry);
    currentInterfaces.RemoveAt(0);
  }
}

void PInterfaceBundle::RefreshInterfaceList()
{
  // get a new interface list
  InterfaceTable newInterfaces;
  PIPSocket::GetInterfaceTable(newInterfaces);

  PWaitAndSignal m(mutex);

  // if changed, then update the internal list
  if (!CompareInterfaceLists(currentInterfaces, newInterfaces)) {

    InterfaceTable oldInterfaces = currentInterfaces;
    currentInterfaces = newInterfaces;

    // look for interfaces to add that are in new list that are not in the old list
    PINDEX i;
    for (i = 0; i < newInterfaces.GetSize(); ++i) {
      InterfaceEntry & newEntry = newInterfaces[i];
      if (!newEntry.GetAddress().IsLoopback() && !IsInterfaceInList(newEntry, oldInterfaces))
        OnAddInterface(newEntry);
    }

    // look for interfaces to remove that are in old list that are not in the new list
    for (i = 0; i < oldInterfaces.GetSize(); ++i) {
      InterfaceEntry & oldEntry = oldInterfaces[i];
      if (!oldEntry.GetAddress().IsLoopback() && !IsInterfaceInList(oldEntry, newInterfaces))
        OnRemoveInterface(oldEntry);
    }
  }
}

void PInterfaceBundle::UpdateThreadMain()
{
  // check for interface changes every 5 seconds
  while (threadRunning) {
    Sleep(5000);
    RefreshInterfaceList();
  }
}

BOOL PInterfaceBundle::CompareInterfaceLists(const InterfaceTable & list1, const InterfaceTable & list2)
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

BOOL PInterfaceBundle::InterfaceListIsSubsetOf(const InterfaceTable & subset, const InterfaceTable & set)
{
  PINDEX i;
  for (i = 0; i < subset.GetSize(); ++i) {
    InterfaceEntry & entry = subset[i];
    if (!IsInterfaceInList(entry, set))
      return FALSE;
  }

  return TRUE;
}

BOOL PInterfaceBundle::IsInterfaceInList(const InterfaceEntry & entry, const InterfaceTable & list)
{
  PINDEX i;
  for (i = 0; i < list.GetSize(); ++i) {
    InterfaceEntry & listEntry = list[i];
    if ((entry.GetName() == listEntry.GetName()) && (entry.GetAddress() == listEntry.GetAddress()))
      return TRUE;
  }
  return FALSE;
}

PStringList PInterfaceBundle::GetInterfaceList(BOOL includeLoopBack)
{
  PStringList list;

  {
    PWaitAndSignal m(mutex);
    PINDEX i;
    for (i = 0; i < currentInterfaces.GetSize(); ++i) {
      InterfaceEntry & entry = currentInterfaces[i];
      if (includeLoopBack || !entry.GetAddress().IsLoopback())
        list.AppendString(entry.GetName());
    }
  }

  return list;
}

BOOL PInterfaceBundle::GetInterfaceInfo(const PString & iface, InterfaceEntry & info)
{
  if (iface.IsEmpty())
    return FALSE;

  {
    PWaitAndSignal m(mutex);
    PINDEX i;
    for (i = 0; i < currentInterfaces.GetSize(); ++i) {
      InterfaceEntry & entry = currentInterfaces[i];
      if (entry.GetName() == iface) {
        info = entry;
        return TRUE;
      }
    }
  }

  return FALSE;
}


void PInterfaceBundle::OnAddInterface(const InterfaceEntry &)
{
}

void PInterfaceBundle::OnRemoveInterface(const InterfaceEntry &)
{
}

//////////////////////////////////////////////////

PSocketBundle::PSocketBundle(PINDEX refreshInterval)
  : PInterfaceBundle(refreshInterval)
{
}

//////////////////////////////////////////////////

PMultipleSocketBundle::PMultipleSocketBundle(PINDEX _refreshInterval)
  : PSocketBundle(_refreshInterval)
{
}

PMultipleSocketBundle::~PMultipleSocketBundle()
{
  // delete the sockets
  // TODO
}

BOOL PMultipleSocketBundle::Open(WORD _port)
{
  if (IsOpen())
    return FALSE;

  port = _port;

  return PSocketBundle::Open();
}

void PMultipleSocketBundle::OnAddInterface(const InterfaceEntry & entry)
{
  PUDPSocket * socket = new PUDPSocket(port);
  if (!socket->Listen(entry.GetAddress())) 
    delete socket;
  else {
    SocketInfo * info = new SocketInfo;
    info->socket = socket;
    socketInfoMap.insert(SocketInfoMap_T::value_type(entry.GetName(), info));
    cout << "Interface added : " << entry.GetName() << endl;
  }
}

void PMultipleSocketBundle::OnRemoveInterface(const InterfaceEntry & entry)
{
  cout << "Interface removed : " << entry.GetName() << endl;
  SocketInfoMap_T::iterator r = socketInfoMap.find(entry.GetName());
  if (r != socketInfoMap.end()) {
    SocketInfo * info = r->second;
    socketInfoMap.erase(r);
    if (info->inUse) {
      info->removed = TRUE;
      removedSockets.push_back(info);
    }
    else
    {
      delete info->socket;
      delete info;
    }
  }
}


BOOL PMultipleSocketBundle::SendTo(const void * buf, PINDEX len,
                           const PUDPSocket::Address & addr, WORD port,
                           const PString & iface,
                           PINDEX & lastWriteCount)
{
  if (iface.IsEmpty())
    return FALSE;

  PWaitAndSignal m(mutex);
  SocketInfoMap_T::iterator r = socketInfoMap.find(iface);
  if (r == socketInfoMap.end()) 
    return FALSE;

  SocketInfo & info = *(r->second);
  PUDPSocket & udp = *info.socket;

  if (!udp.WriteTo(buf, len, addr, port))
    return FALSE;

  lastWriteCount = udp.GetLastWriteCount();
  return TRUE;
}

BOOL PMultipleSocketBundle::ReadFrom(void * buf, PINDEX len,
                             PIPSocket::Address & addr, WORD & port,
                             PString & iface,
                             PINDEX & lastReadCount)
{
  // if interface is not empty, use that specific interface
  if (!iface.IsEmpty()) {
    SocketInfo * info;
    {
      PWaitAndSignal m(mutex);
      SocketInfoMap_T::iterator r = socketInfoMap.find(iface);
      if (r == socketInfoMap.end()) 
        return FALSE;
      info = r->second;
      info->inUse = TRUE;
    }

    if (!info->socket->ReadFrom(buf, len, addr, port))
      return FALSE;

    {
      PWaitAndSignal m(mutex);
      if (info->removed) {
        SocketInfoList_T ::iterator r = ::find(removedSockets.begin(), removedSockets.end(), info);
        if (r != removedSockets.end())
          removedSockets.erase(r);
        delete info->socket;
        delete info;
        return FALSE;
      }
      lastReadCount = info->socket->GetLastReadCount();
      info->inUse = FALSE;
    }

    return TRUE;
  }

  // if interface is empty, wait on all interfaces
  // %%TODO - not implemented
  return FALSE;
}

//////////////////////////////////////////////////

PSingleSocketBundle::PSingleSocketBundle(const PString & _theInterface, PINDEX _refreshInterval)
  : PMultipleSocketBundle(_refreshInterval), theInterface(_theInterface)
{
}

void PSingleSocketBundle::OnAddInterface(const InterfaceEntry & entry)
{
  if (entry.GetName() == entry)
    PMultipleSocketBundle::OnAddInterface(entry);
}

void PSingleSocketBundle::OnRemoveInterface(const InterfaceEntry & entry)
{
  if (entry.GetName() == entry)
    PMultipleSocketBundle::OnRemoveInterface(entry);
}
