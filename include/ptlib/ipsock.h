/*
 * ipsock.h
 *
 * Internet Protocol socket I/O channel class.
 *
 * Portable Tools Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_IPSOCKET_H
#define PTLIB_IPSOCKET_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/socket.h>


/**This class describes a type of socket that will communicate using the
   Internet Protocol.
   If P_HAS_IPV6 is not set, IPv4 only is supported.
   If P_HAS_IPV6 is set, both IPv4 and IPv6 adresses are supported, with
   IPv4 as default. This allows to transparently use IPv4, IPv6 or Dual
   stack operating systems.
 */
class PIPSocket : public PSocket
{
  PCLASSINFO(PIPSocket, PSocket);
  protected:
    /**Create a new Internet Protocol socket based on the port number
       specified.
     */
    PIPSocket();

  public:
    /**A class describing an IP address.
     */
    class Address : public PObject {
        PCLASSINFO_WITH_CLONE(Address, PObject);
      public:

        /**@name Address constructors */
        //@{
        /// Create an IPv4 address with the default address: 127.0.0.1 (loopback).
        Address();

        /**Create an IP address from string notation,
           eg dot notation x.x.x.x. for IPv4, or colon notation x:x:x::xxx for IPv6.
          */
        explicit Address(const PString & dotNotation);

        /// Create an IPv4 or IPv6 address from 4 or 16 byte values.
        Address(PINDEX len, const BYTE * bytes, int scope = 0);

        /// Create an IP address from four byte values.
        Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4);

        /// Create an IPv4 address from a four byte value in network byte order.
        Address(DWORD dw);

        /// Create an IPv4 address from an in_addr structure.
        Address(const in_addr & addr);

#if P_HAS_IPV6
        /// Create an IPv6 address from an in_addr structure.
        Address(const in6_addr & addr);
        Address(const in6_addr & addr, int scope);
#endif

        /// Create an IP (v4 or v6) address from a sockaddr (sockaddr_in,
        /// sockaddr_in6 or sockaddr_in6_old) structure.
        Address(const int ai_family, const int ai_addrlen,struct sockaddr *ai_addr);

#ifdef __NUCLEUS_NET__
        Address(const struct id_struct & addr);
        Address & operator=(const struct id_struct & addr);
#endif

        /// Copy an address from another IP v4 address.
        Address & operator=(const in_addr & addr);

#if P_HAS_IPV6
        /// Copy an address from another IPv6 address.
        Address & AssignIPV6(const in6_addr & addr, int scope);
#endif

        /// Copy an address from a string.
        Address & operator=(const PString & dotNotation);

        /// Copy an address from a four byte value in network order.
        Address & operator=(DWORD dw);
        //@}

        /// Compare two adresses for absolute (in)equality.
        Comparison Compare(const PObject & obj) const;
        bool operator==(const Address & addr) const { return Compare(addr) == EqualTo; }
        bool operator!=(const Address & addr) const { return Compare(addr) != EqualTo; }
#if P_HAS_IPV6
        bool operator ==(in6_addr & addr) const;
        bool operator !=(in6_addr & addr) const { return !operator==(addr); }

        bool EqualIPV6(in6_addr & addr, int scope) const;
        bool NotEqualIPV6(in6_addr & addr, int scope) const { return !EqualIPV6(addr, scope); }
#endif
        bool operator==(in_addr & addr) const;
        bool operator!=(in_addr & addr) const { return !operator==(addr); }
        bool operator==(DWORD dw) const;
        bool operator!=(DWORD dw) const   { return !operator==(dw); }
#ifdef P_VXWORKS
        bool operator==(long unsigned int u) const { return  operator==((DWORD)u); }
        bool operator!=(long unsigned int u) const { return !operator==((DWORD)u); }
#endif
#ifdef _WIN32
        bool operator==(unsigned u) const { return  operator==((DWORD)u); }
        bool operator!=(unsigned u) const { return !operator==((DWORD)u); }
#endif
#ifdef P_RTEMS
        bool operator==(u_long u) const { return  operator==((DWORD)u); }
        bool operator!=(u_long u) const { return !operator==((DWORD)u); }
#endif
#ifdef P_BEOS
        bool operator==(in_addr_t a) const { return  operator==((DWORD)a); }
        bool operator!=(in_addr_t a) const { return !operator==((DWORD)a); }
#endif
        bool operator==(int i) const      { return  operator==((DWORD)i); }
        bool operator!=(int i) const      { return !operator==((DWORD)i); }

        /// Compare two addresses for equivalence. This will return true
        /// if the two addresses are equivalent even if they are IPV6 and IPV4.
#if P_HAS_IPV6
        bool operator*=(const Address & addr) const;
#else
        bool operator*=(const Address & addr) const { return operator==(addr); }
#endif

        /// Format an address as a string.
        PString AsString(
          bool bracketIPv6 = false,  ///< An IPv6 address is enclosed in []'s
          bool excludeScope = false  ///< An IPv6 address includes %xxx for scope ID
        ) const;

        /// Convert string to IP address. Returns true if was a valid address.
        PBoolean FromString(
          const PString & str
        );

        /// Format an address as a string.
        operator PString() const;

        /// Return IPv4 address in network order.
        operator in_addr() const;

#if P_HAS_IPV6
        /// Return IPv6 address in network order.
        operator in6_addr() const;

        int GetIPV6Scope() const { return m_scope6; }
#endif

        /// Return IPv4 address in network order.
        operator DWORD() const;

        /// Return first byte of IPv4 address.
        BYTE Byte1() const;

        /// Return second byte of IPv4 address.
        BYTE Byte2() const;

        /// Return third byte of IPv4 address.
        BYTE Byte3() const;

        /// Return fourth byte of IPv4 address.
        BYTE Byte4() const;

        /// Return specified byte of IPv4 or IPv6 address.
        BYTE operator[](PINDEX idx) const;

        /// Get the address length (will be either 4 or 16).
        PINDEX GetSize() const;

        /// Get the pointer to IP address data.
        const char * GetPointer() const { return (const char *)&m_v; }

        /// Get the version of the IP address being used.
        unsigned GetVersion() const { return m_version; }

        /// Check for illegal address
        bool IsValid() const { return m_version == 4 || m_version == 6; }
        /// Check address 0.0.0.0 or ::.
        bool IsAny() const;

        /// Check address 127.0.0.1 or ::1.
        bool IsLoopback() const;

        /// Check for Broadcast address 255.255.255.255.
        bool IsBroadcast() const;

        /// Check if address is multicast group
        bool IsMulticast() const;

        /// Check if this address is within the sub-net
        bool IsSubNet(const Address & network, const Address & mask) const;

        /** Check if the remote address is a private address.
            For IPV4 this is specified RFC 1918 as the following ranges:
            \li    10.0.0.0 - 10.255.255.255.255
            \li  172.16.0.0 - 172.31.255.255
            \li 192.168.0.0 - 192.168.255.255

            For IPV6 this is specified as any address having "1111 1110 1" for the first nine bits.
        */
        bool IsRFC1918() const ;

#if P_HAS_IPV6
        /// Check for v4 mapped in v6 address ::ffff:a.b.c.d.
        bool IsV4Mapped() const;

            /// Check for link-local address 
        bool IsLinkLocal() const;

        //// Check for site-local address
        bool IsSiteLocal() const;
#endif

        static const Address & GetLoopback(unsigned version = 4);
        static const Address & GetAny(unsigned version = 4);
        static const Address GetBroadcast(unsigned version = 4);

      protected:
        /// Runtime test of IP addresse type.
        union {
          in_addr m_four;
#if P_HAS_IPV6
          in6_addr m_six;
#endif
        } m_v;
        unsigned m_version;
        int      m_scope6;

      /** Output IPv6 & IPv4 address as a string to the specified string.
          If the stream flag <b>hex</b> is set, then an IPv6 address is
          surrounded by square brackets [].
          If the stream flag <b>fixed</b> is set, the an IPv6 address will
          have the scope ID (%xxx) suppressed.
        */
      friend ostream & operator<<(ostream & s, const Address & a);

      /// Input IPv4 (not IPv6 yet!) address as a string from the specified string.
      friend istream & operator>>(istream & s, Address & a);
    };

    /**A class describing an IP address and port number combination.
     */
    class AddressAndPort : public PObject
    {
      public:
        AddressAndPort(
          char separator = ':'
        );
        AddressAndPort(
          WORD defaultPort,
          char separator = ':'
        );
        AddressAndPort(
          const PString & str,
          WORD defaultPort = 0,
          char separator = ':',
          const char * proto = NULL
        );
        AddressAndPort(
          const PIPSocket::Address & addr,
          WORD defaultPort = 0,
          char separator = ':'
        );
        AddressAndPort(
          struct sockaddr *ai_addr,
          const int ai_addrlen
        );

        bool Parse(
          const PString & str,
          WORD defaultPort = 0,
          char separator = ':',
          const char * proto = NULL
        );

        PString AsString(char separator = 0) const;

        const PIPSocket::Address & GetAddress() const { return m_address; }

        void SetAddress(
          const PIPSocket::Address & addr,
          WORD port = 0
        );

        WORD GetPort() const { return m_port; }

        void SetPort(
          WORD port
        ) { m_port = port; }

        bool IsValid() const
        {
          return m_address.IsValid() && m_port != 0;
        }

        virtual void PrintOn(ostream & strm) const
        {
          strm << AsString();
        }

        virtual Comparison Compare(const PObject & obj) const;

        bool MatchWildcard(
          const AddressAndPort & wild
        ) const;

      protected:
        PIPSocket::Address m_address;
        WORD               m_port;
        char               m_separator;
    };


    //**@name Overrides from class PObject */
    //@{
    /**Output the contents of the URL to the stream as a string.
     */
    virtual void PrintOn(
      ostream &strm   ///< Stream to print the object into.
    ) const;
    //@}

    //**@name Overrides from class PChannel */
    //@{
    /**Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       @return
       The name of the channel.
     */
    virtual PString GetName() const;

    /**Set the default IP address familly.
       Needed as lot of IPv6 stack are not able to receive IPv4 packets in IPv6 sockets
       They are not RFC 2553, chapter 7.3, compliant.
       As a consequence, when opening a socket to listen to port 1720 (for example) from any remot host
       one must decide whether this is an IPv4 or an IPv6 socket...
    */
    static int GetDefaultIpAddressFamily();
    static void SetDefaultIpAddressFamily(int ipAdressFamily); // PF_INET, PF_INET6
    static void SetDefaultIpAddressFamilyV4(); // PF_INET
#if P_HAS_IPV6
    static void SetDefaultIpAddressFamilyV6(); // PF_INET6
    static PBoolean IsIpAddressFamilyV6Supported();
#endif
    static const PIPSocket::Address & GetDefaultIpAny();
    static const PIPSocket::Address & GetInvalidAddress();

    /**Set flag for suppress getting canonical name when doing lookup via
       hostname.

       Some badly configured DNS servers can cause long delays when this
       feature is used.
      */
    static void SetSuppressCanonicalName(bool suppress);

    /**Get flag for suppress getting canonical name when doing lookup via
       hostname.

       Some badly configured DNS servers can cause long delays when this
       feature is used.
      */
    static bool GetSuppressCanonicalName();

    /**Open an IPv4 or IPv6 socket
     */
    virtual PBoolean OpenSocket(
      int ipAdressFamily=PF_INET
    ) = 0;
    //@}

    /**@name Overrides from class PSocket */
    //@{
    /** Class for handling a range of ports for local binding.
      */
    class PortRange {
      public:
        PortRange(WORD basePort = 0, WORD maxPort = 0);

        /// Set the port range parameters
        void Set(
          unsigned newBase,       ///< New base port
          unsigned newMax,        ///< New maximum port, if < newBase, then set to newBase
          unsigned dfltRange = 0, ///< If newMax == 0 then it is set to newBase plus this.
          unsigned dfltBase = 0   ///< If newbase == 0, then it is set to this value.
        );

        bool IsValid() const { return m_base != 0 && m_base <= m_max; }
        friend ostream & operator<<(ostream & strm, const PortRange & pr) { return strm << pr.m_base << '-' << pr.m_max; }

        /// Connect to remote
        bool Connect(
          PIPSocket & socket,         ///< Socket to connect on
          const Address & addr,       ///< Address of remote machine to connect to.
          const Address & binding = GetDefaultIpAny() ///< Local interface address to bind to.
        );

        /// Listen on the socket(s) with local port in the range.
        bool Listen(
          PIPSocket & socket,                          ///< Socket to listen on
          const Address & binding = GetDefaultIpAny(), ///< Local interface address to bind to.
          unsigned queueSize = 5,                      ///< Number of pending accepts that may be queued.
          Reusability reuse = AddressIsExclusive       ///< Can/Can't listen more than once.
        );
        bool Listen(
          PIPSocket ** sockets,                        ///< Socket(s) to listen on
          PINDEX numSockets = 1,                       ///< Number of sockets to listen on consecutive ports
          const Address & binding = GetDefaultIpAny(), ///< Local interface address to bind to.
          unsigned queueSize = 5,                      ///< Number of pending accepts that may be queued.
          Reusability reuse = AddressIsExclusive       ///< Can/Can't listen more than once.
        );

        /// Get base port for range.
        WORD GetBase() const { return m_base; }

        /// Get maximum port for range.
        WORD GetMax() const { return m_max; }

      protected:
        PDECLARE_MUTEX(m_mutex);
        WORD   m_base;
        WORD   m_max;
    };

    /**Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       PIPSocket::SetPort() function.

       @return
       true if the channel was successfully connected to the remote host.
     */
    virtual PBoolean Connect(
      const PString & address   ///< Address of remote machine to connect to.
    );
    virtual PBoolean Connect(
      const Address & addr      ///< Address of remote machine to connect to.
    );
    virtual PBoolean Connect(
      WORD localPort,           ///< Local port number for connection.
      const Address & addr      ///< Address of remote machine to connect to.
    );
    virtual PBoolean Connect(
      const Address & iface,    ///< Address of local interface to us.
      const Address & addr      ///< Address of remote machine to connect to.
    );
    virtual PBoolean Connect(
      const Address & iface,    ///< Address of local interface to us.
      WORD localPort,           ///< Local port number for connection.
      const Address & addr      ///< Address of remote machine to connect to.
    );

    /**Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the \p port parameter is zero then the port number as
       defined by the object instance construction or the
       PIPSocket::SetPort() function.

       For the UDP protocol, the \p queueSize parameter is ignored.

       @return
       true if the channel was successfully opened.
     */
    virtual PBoolean Listen(
      unsigned queueSize = 5,  ///< Number of pending accepts that may be queued.
      WORD port = 0,           ///< Port number to use for the connection.
      Reusability reuse = AddressIsExclusive ///< Can/Cant listen more than once.
    ) { return InternalListen(GetDefaultIpAny(), queueSize, port, reuse); }

    virtual PBoolean Listen(
      const Address & bind,     ///< Local interface address to bind to.
      unsigned queueSize = 5,   ///< Number of pending accepts that may be queued.
      WORD port = 0,            ///< Port number to use for the connection.
      Reusability reuse = AddressIsExclusive ///< Can/Can't listen more than once.
    ) { return InternalListen(bind, queueSize, port, reuse); }
    //@}

    /**@name New functions for class */
    //@{
    /**Get the "official" host name for the host specified or if none, the host
       this process is running on. The host may be specified as an IP number
       or a hostname alias and is resolved to the canonical form.

       @return
       Name of the host or IP number of host.
     */
    static PString GetHostName();
    static PString GetHostName(
      const PString & hostname  ///< Hosts IP address to get name for.
    );
    static PString GetHostName(
      const Address & addr    ///< Hosts IP address to get name for.
    );

    /**Get the Internet Protocol address for the specified host, or if none
       specified, for the host this process is running on.

       @return
       true if the IP number was returned.
     */
    static PBoolean GetHostAddress(
      Address & addr    ///< Variable to receive hosts IP address.
    );
    static PBoolean GetHostAddress(
      const PString & hostname,
      /**< Name of host to get address for. This may be either a domain name or
           an IP number in "dot" format.
       */
      Address & addr    ///< Variable to receive hosts IP address.
    );

    /**Get the alias host names for the specified host. This includes all DNS
       names, CNAMEs, names in the local hosts file and IP numbers (as "dot"
       format strings) for the host.

       @return
       Array of strings for each alias for the host.
     */
    static PStringArray GetHostAliases(
      /**Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      const PString & hostname
    );
    static PStringArray GetHostAliases(
      const Address & addr    ///< Hosts IP address.
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );

    /**Determine if the specified host is actually the local machine. This
       can be any of the host aliases or multi-homed IP numbers or even
       the special number 127.0.0.1 for the loopback device.

       @return
       true if the host is the local machine.
     */
    static PBoolean IsLocalHost(
      /**Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      const PString & hostname
    );

    /**Get the Internet Protocol address and port for the local host.

       @return
       false (or empty string) if the IP number was not available.
     */
    PString GetLocalAddress() const;
    bool GetLocalAddress(
      Address & addr    ///< Variable to receive hosts IP address.
    ) const;
    bool GetLocalAddress(
      Address & addr,    ///< Variable to receive peer hosts IP address.
      WORD & port        ///< Variable to receive peer hosts port number.
    ) const;
    bool GetLocalAddress(
      AddressAndPort & addr    ///< Variable to receive hosts IP address and port.
    ) const;

    /**Get the Internet Protocol address for the peer host and port the
       socket is connected to.

       @return
       false (or empty string) if the IP number was not available.
     */
    PString GetPeerAddress() const;
    bool GetPeerAddress(
      Address & addr    ///< Variable to receive hosts IP address.
      ) const;
    bool GetPeerAddress(
      Address & addr,    ///< Variable to receive peer hosts IP address.
      WORD & port        ///< Variable to receive peer hosts port number.
      ) const;
    bool GetPeerAddress(
      AddressAndPort & addr    ///< Variable to receive hosts IP address and port.
    ) const;

    /**Get the host name for the local host.

       @return
       Name of the host, or an empty string if an error occurs.
     */
    PString GetLocalHostName();

    /**Get the host name for the peer host the socket is connected to.

       @return
       Name of the host, or an empty string if an error occurs.
     */
    PString GetPeerHostName();

    /**Clear the name (DNS) cache.
     */
    static void ClearNameCache();

    /**Describe a route table entry.
     */
    class RouteEntry : public PObject
    {
      PCLASSINFO(RouteEntry, PObject);
      public:
        /// Create a route table entry from an IP address.
        RouteEntry(const Address & addr) : network(addr) { }

        /// Get the network address associated with the route table entry.
        Address GetNetwork() const { return network; }

        /// Get the network address mask associated with the route table entry.
        Address GetNetMask() const { return net_mask; }

        /// Get the default gateway address associated with the route table entry.
        Address GetDestination() const { return destination; }

        /// Get the network address name associated with the route table entry.
        const PString & GetInterface() const { return interfaceName; }

        /// Get the network metric associated with the route table entry.
        long GetMetric() const { return metric; }

        ///< Print the route table entry
        void PrintOn(ostream & strm) const;

      protected:
        Address network;
        Address net_mask;
        Address destination;
        PString interfaceName;
        long    metric;

      friend class PIPSocket;
    };

    PARRAY(RouteTable, RouteEntry);

    /**Get the systems route table.

       @return
       true if the route table is returned, false if an error occurs.
     */
    static PBoolean GetRouteTable(
      RouteTable & table      ///< Route table
    );

    /// Class for detector of Route Table changes
    class RouteTableDetector
    {
      public:
        virtual ~RouteTableDetector() { }
        virtual bool Wait(
          const PTimeInterval & timeout ///< Time to wait for refresh (may be ignored)
        ) = 0;
        virtual void Cancel() = 0;
    };

    /** Create an object that can wait for a change in the route table or
        active network interfaces.

        If the platform does not support this mechanism then a fake class is
        created using PSyncPoint to wait for the specified amount of time.

        @return Pointer to some object, never returns NULL.
      */
    static RouteTableDetector * CreateRouteTableDetector();

    /**Describe an interface table entry.
     */
    class InterfaceEntry : public PObject
    {
      PCLASSINFO(InterfaceEntry, PObject)

      public:
        /// Create an interface entry from a name, IP addr and MAC addr.
        InterfaceEntry();
        InterfaceEntry(
          const PString & name,
          const Address & addr,
          const Address & mask,
          const PString & macAddr
        );

        /// Print to specified stream.
        virtual void PrintOn(
          ostream &strm   // Stream to print the object into.
        ) const;

        /** Get the name of the interface.
            Note the name will havebeen sanitised of certain possible
            characters that can cause issues elsewhere in the system.
            Therefore, make sure that if you get a device name from some
            other source than the InterfaceEntry, the name is sanitised via
            PIPSocket::InterfaceEntry::SanitiseName() before comparing against
            the name returned here.
          */
        const PString & GetName() const { return m_name; }

        /// Get the address associated with the interface.
        Address GetAddress() const { return m_ipAddress; }

        /// Get the net mask associated with the interface.
        Address GetNetMask() const { return m_netMask; }

        /// Get the MAC address associate with the interface.
        const PString & GetMACAddress() const { return m_macAddress; }

        /// Sanitise a device name for use in PTLib
        static void SanitiseName(PString & name);

      protected:
        PString m_name;
        Address m_ipAddress;
        Address m_netMask;
        PString m_macAddress;

      friend class PIPSocket;
    };

    PARRAY(InterfaceTable, InterfaceEntry);

    /**Get a list of all interfaces.
       @return
       true if the interface table is returned, false if an error occurs.
     */
    static PBoolean GetInterfaceTable(
      InterfaceTable & table,      ///< interface table
      PBoolean includeDown = false     ///< Include interfaces that are down
    );

    /**Get the interface name for the specified local IP address.
      */
    static PString GetInterface(
      const Address & addr    ///< IP address of interface
    );

    /**Get the interface name for the specified local IP address.
      */
    static Address GetInterfaceAddress(
      const PString & ifName,   ///< Name of interface
      unsigned version = 4      ///< IP version number
    );

    /** Get MAC address of interface.
    */
    static PString GetInterfaceMACAddress(
      const char * ifName = NULL   ///< Name of interface, NULL is any
    );

    /**Get the address of the first interface to the "Internet".
       This searches the interfaces for one that has a "public" IP address and
       thus would be access to the Internet.

       @return false if only loopback/private interfaces could be found.
     */
    static Address GetNetworkInterface(
      unsigned version = 4  ///< IP version number
    );

    /**Get the IP address that is being used as the gateway, that is, the
       computer that packets on the default route will be sent.

       The string returned may be used in the Connect() function to open that
       interface.

       Note that the driver does not need to be open for this function to work.

       @return
       true if there was a gateway.
     */
    static Address GetGatewayAddress(
      unsigned version = 4  ///< IP version number
    );

    /**Get the name for the interface that is being used as the gateway,
       that is, the interface that packets on the default route will be sent.

       The string returned may be used in the Connect() function to open that
       interface.

       Note that the driver does not need to be open for this function to work.

       @return
       String name of the gateway device, or empty string if there is none.
     */
    static PString GetGatewayInterface(
      unsigned version = 4  ///< IP version number
    );

    /**Get the IP address for the interface that is being used as the gateway,
       that is, the interface that packets on the default route will be sent.

       This Function can be used to Bind the Listener to only the default Packet
       route in DHCP Environs.

       Note that the driver does not need to be open for this function to work.

       @return
       The Local Interface IP Address for Gatway Access.
     */
    static Address GetGatewayInterfaceAddress(
      unsigned version = 4  ///< IP version number
    );

    /**Get the interface address that will be used to reach the specified
       remote address. Uses longest prefix match when multiple matching interfaces
       are found.

       @return
       Network interface address.
      */
    static Address GetRouteInterfaceAddress(
      const Address & remoteAddress    ///< Remote address to route
    );

    /// The types of QoS supported, based on IEEE P802.1p TrafficClass parameter
    P_DECLARE_ENUM(QoSType,
      BackgroundQoS,        ///< Reduced priority
      BestEffortQoS,        ///< Effectively no QoS
      ExcellentEffortQoS,   ///< Try a bit harder
      CriticalQoS,          ///< Try really hard
      VideoQoS,             ///< Video, < 100 ms latency and jitter
      VoiceQoS,             ///< Voice, < 10 ms latency and jitter
      ControlQoS            ///< Important stuff, prioritise over everything
    );
  /**@name Quality of Service
     This describes in a platform and as protocol independent way as possible
     the quality of service. How it produces, for example, DiffServ and RSVP
     is up to the underlying operating system.

     When reading from a string, the codes "CS0" to "CS7" and "AF11", "AF12"
     etc, as defined by various RFC's, is supported to map to the correct
     DSCP values.
    */
  //@{
    struct QoS
    {
      QoS(QoSType type = BestEffortQoS);
      QoS(const PString & str);

      QoSType m_type;
      int     m_dscp; // If between 0 and 63, is used instead of default for QoSType.

      AddressAndPort m_remote;

      struct Flow {
        Flow() { memset(this, 0, sizeof(*this)); }
        unsigned m_maxBandwidth;    // bits/second, includes IP overhead
        unsigned m_maxPacketSize;   // Bytes, includes IP overhead
        unsigned m_maxLatency;      // Microseconds
        unsigned m_maxJitter;       // Microseconds
      } m_transmit, m_receive;

      friend ostream & operator<<(ostream & strm, const PIPSocket::QoS & qos);
      friend istream & operator>>(istream & strm, PIPSocket::QoS & qos);
    };

    /// Set the current Quality of Service
    virtual bool SetQoS(
      const QoS & qos   ///< New quality of service specification
    );

    /// Get the current Quality of Service
    const QoS & GetQoS() const { return m_qos; }
  //@}

    virtual bool InternalGetLocalAddress(AddressAndPort & addrAndPort);
    virtual bool InternalGetPeerAddress(AddressAndPort & addrAndPort);
    virtual bool InternalListen(const Address & bind, unsigned queueSize, WORD port, Reusability reuse);

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/ipsock.h"
#else
#include "unix/ptlib/ipsock.h"
#endif

  protected:
    QoS m_qos;

    class sockaddr_wrapper
    {
      public:
        sockaddr_wrapper();
        sockaddr_wrapper(const AddressAndPort & ipPort);
        sockaddr_wrapper(const Address & ip, WORD port);

        sockaddr* operator->() const { return addr; }
        operator sockaddr*()   const { return addr; }
        socklen_t GetSize() const;

        PIPSocket::Address GetIP() const;
        WORD GetPort() const;

      private:
        void Construct(const Address & ip, WORD port);

        sockaddr_storage storage;
        union {
          sockaddr_storage * ptr;
          sockaddr         * addr;
          sockaddr_in      * addr4;
    #if P_HAS_IPV6
          sockaddr_in6     * addr6;
    #endif
        };
    };
};

typedef PIPSocket::Address        PIPAddress;
typedef PIPSocket::AddressAndPort PIPAddressAndPort;
typedef PIPSocket::AddressAndPort PIPSocketAddressAndPort;

typedef std::vector<PIPSocket::AddressAndPort> PIPSocketAddressAndPortVector;


#endif // PTLIB_IPSOCKET_H


// End Of File ///////////////////////////////////////////////////////////////
