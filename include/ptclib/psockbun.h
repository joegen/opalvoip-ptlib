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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PSOCKBUN_H
#define PTLIB_PSOCKBUN_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptlib.h>
#include <ptlib/ipsock.h>
#include <ptlib/sockets.h>
#include <ptlib/safecoll.h>
#include <ptclib/pnat.h>
#include <map>


#define PINTERFACE_MONITOR_FACTORY_NAME "InterfaceMonitor"


//////////////////////////////////////////////////

class PInterfaceFilter : public PObject {
  PCLASSINFO(PInterfaceFilter, PObject);
  
  public:
    virtual PIPSocket::InterfaceTable FilterInterfaces(const PIPSocket::Address & destination,
                                                       PIPSocket::InterfaceTable & interfaces) const = 0;
};


//////////////////////////////////////////////////

/** This class is a singleton that will monitor the network interfaces on a
    machine and update a list aof clients on any changes to the number or
    addresses of the interfaces.

    A user may override this singleton by creating a derived class and making
    a static instance of it before any monitor client classes are created.
    This would typically be done in the users main program.
  */
class PInterfaceMonitor : public PProcessStartup
{
    PCLASSINFO(PInterfaceMonitor, PProcessStartup);
  public: 
    enum {
      DefaultRefreshInterval = 60000
    };

    PInterfaceMonitor(
      unsigned refreshInterval = DefaultRefreshInterval,
      bool runMonitorThread = true
    );
    virtual ~PInterfaceMonitor();

    /// Return the singleton interface for the network monitor
    PFACTORY_GET_SINGLETON(PProcessStartupFactory, PInterfaceMonitor);
    
    /// Change the refresh interval
    void SetRefreshInterval (unsigned refresh);
    
    /// Change whether the monitor thread should run
    void SetRunMonitorThread (bool runMonitorThread);

    /** Start monitoring network interfaces.
        If the monitoring thread is already running, then this will cause an
        refresh of the interface list as soon as possible. Note that this will
        happen asynchronously.
      */
    void Start();

    /// Stop monitoring network interfaces.
    void Stop();

    typedef PIPSocket::InterfaceEntry InterfaceEntry;

    /** Get an array of all current interface descriptors, possibly including
        the loopback (127.0.0.1) interface. Note the names are of the form
        ip%name, eg "10.0.1.11%3Com 3C90x Ethernet Adapter" or "192.168.0.10%eth0"
      */
    PStringArray GetInterfaces(
      bool includeLoopBack = false,  ///< Flag for if loopback is to included in list
      const PIPSocket::Address & destination = PIPSocket::GetDefaultIpAny()
                          ///< Optional destination for selecting specific interface
    );

    /** Returns whether destination is reachable through binding or not.
        The default behaviour returns true unless there is an interface
        filter installed an the filter does not return <code>binding</code> among
        it's interfaces.
      */
    bool IsValidBindingForDestination(
      const PIPSocket::Address & binding,     ///< Interface binding to test
      const PIPSocket::Address & destination  ///< Destination to test against the <code>binding</code>
    );

    /** Return information about an active interface given the descriptor
       string. Note that when searchin the descriptor may be a partial match
       e.g. "10.0.1.11" or "%eth0" may be used.
      */
    bool GetInterfaceInfo(
      const PString & iface,  ///< Interface desciptor name
      InterfaceEntry & info   ///< Information on the interface
    ) const;
    
    /** Returns whether the descriptor string equals the interface entry.
        Note that when searching the descriptor may be a partial match
        e.g. "10.0.1.11" or "%eth0" may be used.
      */
    static bool IsMatchingInterface(
      const PString & iface,        ///< Interface descriptor
      const InterfaceEntry & entry  ///< Interface entry
    );

    /**Information on the interface change.
      */
    struct InterfaceChange : public InterfaceEntry
    {
      InterfaceChange(const InterfaceEntry & entry, bool added)
        : InterfaceEntry(entry)
        , m_added(added)
        { }

      const bool m_added;
    };

    /// Type for disposition notifiers
    typedef PNotifierTemplate<InterfaceChange> Notifier;

    /// Macro to declare correctly typed interface notifier
    #define PDECLARE_InterfaceNotifier(cls, fn) PDECLARE_NOTIFIER2(PInterfaceMonitor, cls, fn, PInterfaceMonitor::InterfaceChange)

    /// Macro to create correctly typed interface notifier
    #define PCREATE_InterfaceNotifier(fn) PCREATE_NOTIFIER2(fn, PInterfaceMonitor::InterfaceChange)

    enum {
      DefaultPriority = 50,
    };

    /**Add a notifier for interface changes.
      */
    void AddNotifier(
      const Notifier & notifier,   ///< Notifier to be called by interface monitor
      unsigned priority = DefaultPriority   ///< Priority for notification
    );
    void RemoveNotifier(
      const Notifier & notifier    ///< Notifier to be called by interface monitor
    );

    /** Sets the monitor's interface filter. Note that the monitor instance
        handles deletion of the filter.
      */
    void SetInterfaceFilter(PInterfaceFilter * filter);
    bool HasInterfaceFilter() const { return m_interfaceFilter != NULL; }

  protected:
    virtual void OnShutdown();

    void UpdateThreadMain();

    virtual void RefreshInterfaceList();
    virtual void OnInterfacesChanged(const PIPSocket::InterfaceTable & addedInterfaces, const PIPSocket::InterfaceTable & removedInterfaces);

    typedef std::multimap<unsigned, Notifier> Notifiers;
    Notifiers m_notifiers;
    PDECLARE_MUTEX(m_notifiersMutex);

    PIPSocket::InterfaceTable m_interfaces;
   PDECLARE_MUTEX(m_interfacesMutex);

    bool           m_runMonitorThread;
    PTimeInterval  m_refreshInterval;
    PDECLARE_MUTEX(m_threadMutex);
    PThread      * m_updateThread;

    PInterfaceFilter * m_interfaceFilter;
    PIPSocket::RouteTableDetector * m_changedDetector;

  friend class PInterfaceMonitorClient;
};


//////////////////////////////////////////////////

/** This is a base class for UDP socket(s) that are monitored for interface
    changes. Two derived classes are available, one that is permanently
    bound to an IP address and/or interface name. The second will dynamically
    open/close ports as interfaces are added and removed from the system.
  */
class PMonitoredSockets : public PSafeObject
{
    PCLASSINFO(PMonitoredSockets, PSafeObject);
  protected:
    PMonitoredSockets(
      bool reuseAddr    ///< Flag for sharing socket/exclusve use
      P_NAT_PARAM(PNatMethods * natMethods)  ///< NAT method to use to create sockets.
    );

  public:
    typedef PIPSocket::InterfaceEntry InterfaceEntry;

    /** Open the socket(s) using the specified port. If port is zero then a
        system allocated port is used. In this case and when multiple
        interfaces are supported, all sockets use the same dynamic port value.

        Returns true if all sockets are opened.
     */
    virtual PBoolean Open(
      WORD port
    ) = 0;

    /// Indicate if the socket(s) are open and ready for reads/writes.
    PBoolean IsOpen() const { return m_opened; }

    /// Close all socket(s)
    virtual PBoolean Close() = 0;

    /// Return the local port number being used by the socket(s)
    WORD GetPort() const { return m_localPort; }

    /// Get the local address for the given interface.
    virtual PBoolean GetAddress(
      const PString & iface,        ///< Interface to get address for
      PIPSocket::Address & address, ///< Address of interface
      WORD & port,                  ///< Port listening on
      PBoolean usingNAT             ///< Require NAT address/port
    ) const = 0;

    struct BundleParams {
      BundleParams()
        : m_buffer(NULL)
        , m_length(0)
        , m_addr(0)
        , m_port(0)
        , m_lastCount(0)
        , m_errorCode(PChannel::NoError)
        , m_errorNumber(0)
      { }

      void * m_buffer;              ///< Data to read/write
      PINDEX m_length;              ///< Maximum length of data
      PIPSocket::Address m_addr;    ///< Remote IP address data came from, or is written to
      WORD m_port;                  ///< Remote port data came from, or is written to
      PString m_iface;              ///< Interface to use for read or write, also then one data was read on
      PINDEX m_lastCount;           ///< Actual length of data read/written
      PTimeInterval m_timeout;      ///< Time to wait for data
      PChannel::Errors m_errorCode; ///< Error code for read/write
      int m_errorNumber;            ///< Error number (OS specific) for read/write
    };

    /** Write to the remote address/port using the socket(s) available. If the
        iface parameter is empty, then the data is written to all socket(s).
        Otherwise the iface parameter indicates the specific interface socket
        to write the data to.
      */
    virtual void WriteToBundle(
      BundleParams & param ///< Info on data to write
    ) = 0;

    /** Read fram a remote address/port using the socket(s) available. If the
        iface parameter is empty, then the first data received on any socket(s)
        is used, and the iface parameter is set to the name of that interface.
        Otherwise the iface parameter indicates the specific interface socket
        to read the data from.
      */
    virtual void ReadFromBundle(
      BundleParams & param ///< Info on data to read
    ) = 0;

    /** Get an array of all current interface descriptors, possibly including
        the loopback (127.0.0.1) interface. Note the names are of the form
        ip%name, eg "10.0.1.11%3Com 3C90x Ethernet Adapter" or "192.168.0.10%eth0".
        If destination is not 'any' and a filter is set, filters the interface list
        before returning it.
      */
    virtual PStringArray GetInterfaces(
      bool includeLoopBack = false,  ///< Flag for if loopback is to included in list
      const PIPSocket::Address & destination = PIPSocket::GetDefaultIpAny()
                          ///< Optional destination for selecting specific interface
    );

    /** Return information about an active interface given the descriptor
       string. Note that when searchin the descriptor may be a partial match
       e.g. "10.0.1.11" or "%eth0" may be used.
      */
    virtual bool GetInterfaceInfo(
      const PString & iface,  ///< Interface desciptor name
      InterfaceEntry & info   ///< Information on the interface
    ) const;

    /** Create a new monitored socket instance based on the interface
        descriptor. This will create a multiple or single socket derived class
        of PMonitoredSockets depending on the iface parameter.

        If the iface parameter starts with a '%' or "*%" then a bundle is
        created for all IP addresses for a specific interface.

        If the iface has a non INADDR_ANY address value, with or without a
        %iface qualifier, then only that IP address is listened to.

        If the iface resolves to a specific INADDR_ANY or INADDR6_ANY, then
        only that version will be listened to, on all interfaces or 

        Finally if empty string or "*", than all IP versions and all interfaces
        are used.
      */
    static PMonitoredSockets * Create(
      const PString & iface,              ///< Interface name to create socket for
      bool reuseAddr = false              ///< Re-use or exclusive port number
      P_NAT_PARAM(PNatMethods * natMethods = NULL)     ///< NAT method
    );

  protected:
    struct SocketInfo {
      SocketInfo()
        : socket(NULL)
        , inUse(false)
      { }
      void Read(PMonitoredSockets & bundle, BundleParams & param);
      void Write(BundleParams & param);

      PUDPSocket * socket;
      bool         inUse;
    };
    friend struct SocketInfo;

    bool CreateSocket(
      SocketInfo & info,
      const PIPSocket::Address & binding
    );
    bool DestroySocket(SocketInfo & info);
    bool GetSocketAddress(
      const SocketInfo & info,
      PIPSocket::Address & address,
      WORD & port,
      bool usingNAT
    ) const;

    void ReadFromSocketList(
      PSocket::SelectList & readers,
      PUDPSocket * & socket,
      BundleParams & param
    );

    WORD          m_localPort;
    bool          m_reuseAddress;
#if P_NAT
    PNatMethods * m_natMethods;
#endif

    bool          m_opened;
    PUDPSocket    m_interfaceAddedSignal;
};

typedef PSafePtr<PMonitoredSockets> PMonitoredSocketsPtr;


//////////////////////////////////////////////////

/** This class can be used to access the bundled/monitored UDP sockets using
    the PChannel API.
  */
class PMonitoredSocketChannel : public PChannel
{
    PCLASSINFO(PMonitoredSocketChannel, PChannel);
  public:
  /**@name Construction */
  //@{
    /// Construct a monitored socket bundle channel
    PMonitoredSocketChannel(
      const PMonitoredSocketsPtr & sockets,  ///< Monitored socket bundle to use in channel
      bool shared                            ///< Monitored socket is shared by other channels
    );
  //@}

  /**@name Overrides from class PSocket */
  //@{
    virtual PBoolean IsOpen() const;
    virtual PBoolean Close();

    /** Override of PChannel functions to allow connectionless reads
     */
    virtual PBoolean Read(
      void * buffer,
      PINDEX length
    );

    /** Override of PChannel functions to allow connectionless writes
     */
    virtual PBoolean Write(
      const void * buffer,
      PINDEX length
    );
  //@}

  /**@name New functions for class */
  //@{
    /** Set the interface descriptor to be used for all reads/writes to this channel.
        The iface parameter can be a partial descriptor eg "%eth0".
      */
    void SetInterface(
      const PString & iface   ///< Interface descriptor
    );

    /// Get the current interface descriptor being used/
    PString GetInterface();

    /** Get the local IP address and port for the currently selected interface.
      */
    bool GetLocal(
      PIPSocket::Address & address, ///< IP address of local interface
      WORD & port,                  ///< Port listening on
      bool usingNAT                 ///< Require NAT address/port
    );
    bool GetLocal(
      PIPSocket::AddressAndPort & ap, ///< IP address and port of local interface
      bool usingNAT                 ///< Require NAT address/port
    );

    /// Set the remote address/port for all Write() functions
    void SetRemote(
      const PIPSocket::Address & address, ///< Remote IP address
      WORD port                           ///< Remote port number
    ) { m_remoteAP.SetAddress(address, port); }
    void SetRemote(
      const PIPSocket::AddressAndPort & ap ///< Remote IP address and port
    ) { m_remoteAP = ap; }

    /// Set the remote address/port for all Write() functions
    void SetRemote(
      const PString & hostAndPort ///< String of the form host[:port]
    );

    /// Get the current remote address/port for all Write() functions
    void GetRemote(
      PIPSocket::Address & addr,  ///< Remote IP address
      WORD & port                 ///< Remote port number
    ) const { addr = m_remoteAP.GetAddress(); port = m_remoteAP.GetPort(); }
    void GetRemote(
      PIPSocketAddressAndPort & ap  ///< Remote IP address and port
    ) const { ap = m_remoteAP; }

    /** Set flag for receiving UDP data from any remote address. If the flag
        is false then data received from anything other than the configured
        remote address and port is ignored.
      */
    void SetPromiscuous(
      bool flag   ///< New flag
    ) { m_promiscuousReads = flag; }

    /// Get flag for receiving UDP data from any remote address
    bool GetPromiscuous() { return m_promiscuousReads; }

    /// Get the IP address and port of the last received UDP data.
    void GetLastReceived(
      PIPSocket::Address & addr,  ///< Remote IP address
      WORD & port                 ///< Remote port number
    ) const { addr = m_lastReceivedAP.GetAddress(); port = m_lastReceivedAP.GetPort(); }
    void GetLastReceived(
      PIPSocketAddressAndPort & ap  ///< Remote IP address and port
    ) const { ap = m_lastReceivedAP; }

    /// Get the interface the last received UDP data was recieved on.
    PString GetLastReceivedInterface() const { return m_lastReceivedInterface; }

    /// Get the monitored socket bundle being used by this channel.
    const PMonitoredSocketsPtr & GetMonitoredSockets() const { return m_socketBundle; }
  //@}

  protected:
    PMonitoredSocketsPtr    m_socketBundle;
    bool                    m_sharedBundle;
    PString                 m_currentInterface;
    bool                    m_promiscuousReads;
    bool                    m_closing;
    PIPSocketAddressAndPort m_remoteAP;
    PIPSocketAddressAndPort m_lastReceivedAP;
    PString                 m_lastReceivedInterface;
    PDECLARE_MUTEX(m_mutex);
};


//////////////////////////////////////////////////

/** This concrete class bundles a set of UDP sockets which are dynamically
    adjusted as interfaces are added and removed from the system.
  */
class PMonitoredSocketBundle : public PMonitoredSockets
{
  PCLASSINFO(PMonitoredSocketBundle, PMonitoredSockets);
  public:
    PMonitoredSocketBundle(
      const PString & fixedInterface, ///< Interface name for bundle, "" is all interfaces
      unsigned ipVersion,             ///< Version of IP to listen
      bool reuseAddr                  ///< Flag for sharing socket/exclusve use
      P_NAT_PARAM(PNatMethods * natMethods = NULL)  ///< NAT method to use to create sockets.
    );
    ~PMonitoredSocketBundle();

    /** Get an array of all current interface descriptors, possibly including
        the loopback (127.0.0.1) interface. Note the names are of the form
        ip%name, eg "10.0.1.11%3Com 3C90x Ethernet Adapter" or "192.168.0.10%eth0".
        If destination is not 'any' and a filter is set, filters the interface list
        before returning it.
      */
    virtual PStringArray GetInterfaces(
      bool includeLoopBack = false,  ///< Flag for if loopback is to included in list
      const PIPSocket::Address & destination = PIPSocket::GetDefaultIpAny()
                          ///< Optional destination for selecting specific interface
    );

    /** Open the socket(s) using the specified port. If port is zero then a
        system allocated port is used. In this case and when multiple
        interfaces are supported, all sockets use the same dynamic port value.

        Returns true if all sockets are opened.
     */
    virtual PBoolean Open(
      WORD port
    );

    /// Close all socket(s)
    virtual PBoolean Close();

    /// Get the local address for the given interface.
    virtual PBoolean GetAddress(
      const PString & iface,        ///< Interface to get address for
      PIPSocket::Address & address, ///< Address of interface
      WORD & port,                  ///< Port listening on
      PBoolean usingNAT             ///< Require NAT address/port
    ) const;

    /** Write to the remote address/port using the socket(s) available. If the
        iface parameter is empty, then the data is written to all socket(s).
        Otherwise the iface parameter indicates the specific interface socket
        to write the data to.
      */
    virtual void WriteToBundle(
      BundleParams & param ///< Info on data to write
    );

    /** Read fram a remote address/port using the socket(s) available. If the
        iface parameter is empty, then the first data received on any socket(s)
        is used, and the iface parameter is set to the name of that interface.
        Otherwise the iface parameter indicates the specific interface socket
        to read the data from.
      */
    virtual void ReadFromBundle(
      BundleParams & param ///< Info on data to read
    );

  protected:
    PDECLARE_InterfaceNotifier(PMonitoredSocketBundle, OnInterfaceChange);
    PInterfaceMonitor::Notifier m_onInterfaceChange;

    typedef std::map<std::string, SocketInfo> SocketInfoMap_T;

    void OpenSocket(const PString & iface);
    void CloseSocket(SocketInfoMap_T::iterator iterSocket);

    SocketInfoMap_T m_socketInfoMap;
    PCaselessString m_fixedInterface;
    unsigned        m_ipVersion;
};


//////////////////////////////////////////////////

/** This concrete class monitors a single scoket bound to a specific interface
   or address. The interface name may be a partial descriptor such as
   "%eth0".
  */
class PSingleMonitoredSocket : public PMonitoredSockets
{
  PCLASSINFO(PSingleMonitoredSocket, PMonitoredSockets);
  public:
    PSingleMonitoredSocket(
      const PString & theInterface, ///< Interface to bind to
      bool reuseAddr    ///< Flag for sharing socket/exclusve use
      P_NAT_PARAM(PNatMethods * natMethods = NULL)  ///< NET method to use to create sockets.
    );
    ~PSingleMonitoredSocket();

    /** Get an array of all current interface descriptors, possibly including
        the loopback (127.0.0.1) interface. Note the names are of the form
        ip%name, eg "10.0.1.11%3Com 3C90x Ethernet Adapter" or "192.168.0.10%eth0"
      */
    virtual PStringArray GetInterfaces(
      bool includeLoopBack = false,  ///< Flag for if loopback is to included in list
      const PIPSocket::Address & destination = PIPSocket::GetDefaultIpAny()
                          ///< Optional destination for selecting specific interface
    );

    /** Open the socket(s) using the specified port. If port is zero then a
        system allocated port is used. In this case and when multiple
        interfaces are supported, all sockets use the same dynamic port value.

        Returns true if all sockets are opened.
     */
    virtual PBoolean Open(
      WORD port
    );

    /// Close all socket(s)
    virtual PBoolean Close();

    /// Get the local address for the given interface.
    virtual PBoolean GetAddress(
      const PString & iface,        ///< Interface to get address for
      PIPSocket::Address & address, ///< Address of interface
      WORD & port,                  ///< Port listening on
      PBoolean usingNAT             ///< Require NAT address/port
    ) const;

    /** Write to the remote address/port using the socket(s) available. If the
        iface parameter is empty, then the data is written to all socket(s).
        Otherwise the iface parameter indicates the specific interface socket
        to write the data to.
      */
    virtual void WriteToBundle(
      BundleParams & param ///< Info on data to write
    );

    /** Read fram a remote address/port using the socket(s) available. If the
        iface parameter is empty, then the first data received on any socket(s)
        is used, and the iface parameter is set to the name of that interface.
        Otherwise the iface parameter indicates the specific interface socket
        to read the data from.
      */
    virtual void ReadFromBundle(
      BundleParams & param ///< Info on data to read
    );


  protected:
    PDECLARE_InterfaceNotifier(PSingleMonitoredSocket, OnInterfaceChange);
    PInterfaceMonitor::Notifier m_onInterfaceChange;

    bool IsInterface(const PString & iface) const;

    PString        m_interface;
    InterfaceEntry m_entry;
    SocketInfo     m_info;
};


#endif // PTLIB_PSOCKBUN_H


// End Of File ///////////////////////////////////////////////////////////////
