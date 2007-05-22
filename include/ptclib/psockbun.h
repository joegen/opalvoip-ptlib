/*
 * psockbun.h
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
 * $Log: psockbun.h,v $
 * Revision 1.3  2007/05/22 11:50:47  csoutheren
 * Further implementation of socket bundle
 *
 * Revision 1.2  2007/05/21 06:35:37  csoutheren
 * Changed to be descended off PSafeObject
 *
 * Revision 1.1  2007/05/21 06:06:56  csoutheren
 * Add new socket bundle code to be used to OpalUDPListener
 *
 */

#ifndef _PSOCKBUN_H
#define _PSOCKBUN_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

//////////////////////////////////////////////////

#include <ptlib.h>
#include <ptlib/ipsock.h>
#include <ptlib/sockets.h>
#include <ptlib/safecoll.h>

class PInterfaceBundle : public PSafeObject
{
  PCLASSINFO(PInterfaceBundle, PSafeObject);
  public: 
    enum {
      Default_RefreshInterval = 5000
    };

    typedef PIPSocket::InterfaceTable InterfaceTable;
    typedef PIPSocket::InterfaceEntry InterfaceEntry;

    PInterfaceBundle(PINDEX refreshInterval = Default_RefreshInterval);
    virtual ~PInterfaceBundle();

    BOOL Open();
    BOOL IsOpen();
    void Close();
    void UpdateThreadMain();

    virtual void OnAddInterface(const InterfaceEntry & entry);
    virtual void OnRemoveInterface(const InterfaceEntry & entry);

    PStringList GetInterfaceList(BOOL includeLoopBack = FALSE);
    BOOL GetInterfaceInfo(const PString & iface, InterfaceEntry & info);

    void RefreshInterfaceList();

  protected:
    BOOL CompareInterfaceLists(const InterfaceTable & list1, const InterfaceTable & list2);
    BOOL InterfaceListIsSubsetOf(const InterfaceTable & subset, const InterfaceTable & set);
    BOOL IsInterfaceInList(const InterfaceEntry & entry, const InterfaceTable & list);

    PINDEX refreshInterval;

    PMutex mutex;
    InterfaceTable currentInterfaces;
    PThread * updateThread;
    BOOL threadRunning;
};

//////////////////////////////////////////////////

class PSocketBundle : public PInterfaceBundle
{
  PCLASSINFO(PSocketBundle, PInterfaceBundle)
  public:
    PSocketBundle(PINDEX refreshInterval = Default_RefreshInterval);

    struct SocketInfo {
      SocketInfo()
      { socket = NULL; inUse = FALSE; removed = FALSE; }

      PMutex mutex;
      PUDPSocket * socket;
      BOOL inUse;
      BOOL removed; 
    };
    typedef std::map<std::string, SocketInfo *> SocketInfoMap_T;
    typedef std::vector<SocketInfo *> SocketInfoList_T;
};

//////////////////////////////////////////////////

class PMultipleSocketBundle : public PSocketBundle
{
  PCLASSINFO(PMultipleSocketBundle, PSocketBundle)
  public:
    PMultipleSocketBundle(PINDEX refreshInterval = Default_RefreshInterval);
    ~PMultipleSocketBundle();

    BOOL Open(WORD _port);

    void OnAddInterface(const InterfaceEntry & entry);
    void OnRemoveInterface(const InterfaceEntry & entry);

    virtual BOOL SendTo(const void * buf, PINDEX len,
                        const PUDPSocket::Address & addr, WORD port,
                        const PString & iface,
                        PINDEX & lastWriteCount);

    virtual BOOL ReadFrom(void * buf, PINDEX len,
                             PIPSocket::Address & addr, WORD & port,
                             PString & iface,
                             PINDEX & lastReadCount);
  protected:
    WORD port;
    SocketInfoMap_T socketInfoMap;
    SocketInfoList_T removedSockets;
};

//////////////////////////////////////////////////

class PSingleSocketBundle : public PMultipleSocketBundle
{
  PCLASSINFO(PSingleSocketBundle, PMultipleSocketBundle)
  public:
    PSingleSocketBundle(const PString & theInterface, PINDEX refreshInterval = Default_RefreshInterval);

    void OnAddInterface(const InterfaceEntry & entry);
    void OnRemoveInterface(const InterfaceEntry & entry);

  protected:
    PString theInterface;
};

#endif
