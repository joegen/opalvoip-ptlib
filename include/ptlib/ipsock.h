/*
 * ipsock.h
 *
 * Internet Protocol socket I/O channel class.
 *
 * Portable Windows Library
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
 *
 * $Log: ipsock.h,v $
 * Revision 1.44  2001/01/29 06:41:18  robertj
 * Added printing of entry of interface table.
 *
 * Revision 1.43  2000/06/26 11:17:19  robertj
 * Nucleus++ port (incomplete).
 *
 * Revision 1.42  1999/09/10 04:35:42  robertj
 * Added Windows version of PIPSocket::GetInterfaceTable() function.
 *
 * Revision 1.41  1999/09/10 02:31:42  craigs
 * Added interface table routines
 *
 * Revision 1.40  1999/08/30 02:21:03  robertj
 * Added ability to listen to specific interfaces for IP sockets.
 *
 * Revision 1.39  1999/08/08 09:04:01  robertj
 * Added operator>> for PIPSocket::Address class.
 *
 * Revision 1.38  1999/03/09 02:59:50  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.37  1999/02/23 07:19:22  robertj
 * Added [] operator PIPSocket::Address to get the bytes out of an IP address.
 *
 * Revision 1.36  1999/02/16 08:12:00  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.35  1998/12/21 07:22:50  robertj
 * Virtualised functions for SOCKS support.
 *
 * Revision 1.34  1998/12/18 04:34:14  robertj
 * PPC Linux GNU C compatibility.
 *
 * Revision 1.33  1998/11/30 08:57:32  robertj
 * New directory structure
 *
 * Revision 1.32  1998/11/22 11:30:08  robertj
 * Check route table function to get a list
 *
 * Revision 1.31  1998/11/19 05:18:22  robertj
 * Added route table manipulation functions to PIPSocket class.
 *
 * Revision 1.30  1998/09/23 06:20:45  robertj
 * Added open source copyright license.
 *
 * Revision 1.29  1997/12/11 10:28:57  robertj
 * Added operators for IP address to DWORD conversions.
 *
 * Revision 1.28  1996/12/17 11:08:05  robertj
 * Added DNS name cache clear command.
 *
 * Revision 1.27  1996/11/30 12:10:00  robertj
 * Added Connect() variant so can set the local port number on link.
 *
 * Revision 1.26  1996/11/16 10:48:49  robertj
 * Fixed missing const in PIPSocket::Address stream output operator..
 *
 * Revision 1.25  1996/11/04 03:40:54  robertj
 * Moved address printer from inline to source.
 *
 * Revision 1.24  1996/09/14 13:09:21  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.23  1996/08/25 09:33:55  robertj
 * Added function to detect "local" host name.
 *
 * Revision 1.22  1996/03/26 00:51:13  robertj
 * Added GetLocalAddress() variant that returns port number as well.
 *
 * Revision 1.21  1996/03/16 04:41:30  robertj
 * Changed all the get host name and get host address functions to be more consistent.
 *
 * Revision 1.20  1996/03/03 07:37:56  robertj
 * Added Reusability clause to the Listen() function on sockets.
 *
 * Revision 1.19  1996/02/25 03:00:31  robertj
 * Added operator<< to PIPSocket::Address.
 * Moved some socket functions to platform dependent code.
 *
 * Revision 1.18  1996/02/08 12:11:19  robertj
 * Added GetPeerAddress that returns a port.
 *
 * Revision 1.17  1996/01/28 14:07:31  robertj
 * Changed service parameter to PString for ease of use in GetPortByService function
 * Fixed up comments.
 *
 * Revision 1.16  1995/12/23 03:44:59  robertj
 * Fixed unix portability issues.
 *
 * Revision 1.15  1995/12/10 11:32:11  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.14  1995/10/14 14:57:26  robertj
 * Added internet address to string conversion functionality.
 *
 * Revision 1.13  1995/07/02 01:18:19  robertj
 * Added static functions to get the current host name/address.
 *
 * Revision 1.12  1995/06/17 00:41:40  robertj
 * More logical design of port numbers and service names.
 *
 * Revision 1.11  1995/03/18 06:26:44  robertj
 * Changed IP address variable for GNU compatibility.
 *
 * Revision 1.10  1995/03/14  12:41:38  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.9  1995/03/12  04:38:41  robertj
 * Added more functionality.
 *
 * Revision 1.8  1995/01/02  12:28:24  robertj
 * Documentation.
 * Added more socket functions.
 *
 * Revision 1.7  1995/01/01  01:07:33  robertj
 * More implementation.
 *
 * Revision 1.6  1994/12/15  12:47:14  robertj
 * Documentation.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Spelt Berkeley correctly.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PIPSOCKET

#ifdef __GNUC__
#pragma interface
#endif


#ifndef _PSOCKET
#include <ptlib/socket.h>
#endif

/** This class describes a type of socket that will communicate using the
   Internet Protocol.
 */
class PIPSocket : public PSocket
{
  PCLASSINFO(PIPSocket, PSocket);
  protected:
    /* Create a new Internet Protocol socket based on the port number
       specified.
     */
    PIPSocket();


  public:
    /**
      A class describing an IP address
     */
    class Address : public in_addr {
      public:
        /**@name Address constructors */
        //@{
        /// Create an IP address with the default address
        Address();

        /// Create an IP address with the default address
        Address(const PString & dotNotation);

        /// Create an IP address from four byte values
        Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4);

        /// Create an IP address from a four byte value in network byte order
        Address(DWORD dw);

        /// Create an IP address from an in_addr structure
        Address(const in_addr & addr);

        /// Copy an address from another Address
        Address(const Address & addr);

#ifdef __NUCLEUS_NET__
        Address(const struct id_struct & addr);
        Address & operator=(const struct id_struct & addr);
#endif

        /// Copy an address from another Address
        Address & operator=(const in_addr & addr);

        /// Copy an address from another Address
        Address & operator=(const Address & addr);

        /// Copy an address from a string
        Address & operator=(const PString & dotNotation);

        /// Copy an address from a four byte value in network order
        Address & operator=(DWORD dw);
        //@}

        /// Format an address as a string
        PString AsString() const;

        /// Format an address as a string
        operator PString() const;

        /// Return address in network order
        operator DWORD() const;

        /// Return first byte of IP address
        BYTE Byte1() const;

        /// Return second byte of IP address
        BYTE Byte2() const;

        /// Return third byte of IP address
        BYTE Byte3() const;

        /// Return fourth byte of IP address
        BYTE Byte4() const;

        /// return specified byte of IP address
        BYTE operator[](PINDEX idx) const;

        /// output address as a string to the specified string
        friend ostream & operator<<(ostream & s, const Address & a);

        /// output address as a string to the specified string
        friend istream & operator>>(istream & s, Address & a);
    };


  // Overrides from class PChannel
    /** Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       @return
       the name of the channel.
     */
    virtual PString GetName() const;


  // Overrides from class PSocket.
    /** Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       #PIPSocket::SetPort()# function.

       @return
       TRUE if the channel was successfully connected to the remote host.
     */
    virtual BOOL Connect(
      const PString & address   /// Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      const Address & addr      /// Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      WORD localPort,           /// Local port number for connection
      const Address & addr      /// Address of remote machine to connect to.
    );

    /** Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the #port# parameter is zero then the port number as
       defined by the object instance construction or the
       #PIPSocket::SetPort()# function.

       For the UDP protocol, the #queueSize# parameter is ignored.

       @return
       TRUE if the channel was successfully opened.
     */
    virtual BOOL Listen(
      unsigned queueSize = 5,  /// Number of pending accepts that may be queued.
      WORD port = 0,           /// Port number to use for the connection.
      Reusability reuse = AddressIsExclusive /// Can/Cant listen more than once.
    );
    virtual BOOL Listen(
      const Address & bind,     /// Local interface address to bind to.
      unsigned queueSize = 5,   /// Number of pending accepts that may be queued.
      WORD port = 0,            /// Port number to use for the connection.
      Reusability reuse = AddressIsExclusive /// Can/Can't listen more than once.
    );


  // New functions for class
    /** Get the "official" host name for the host specified or if none, the host
       this process is running on. The host may be specified as an IP number
       or a hostname alias and is resolved to the canonical form.

       @return
       Name of the host or IP number of host.
     */
    static PString GetHostName();
    static PString GetHostName(
      const PString & hostname  /// Hosts IP address to get name for
    );
    static PString GetHostName(
      const Address & addr    /// Hosts IP address to get name for
    );

    /** Get the Internet Protocol address for the specified host, or if none
       specified, for the host this process is running on.

       @return
       TRUE if the IP number was returned.
     */
    static BOOL GetHostAddress(
      Address & addr    /// Variable to receive hosts IP address
    );
    static BOOL GetHostAddress(
      const PString & hostname,
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      Address & addr    /// Variable to receive hosts IP address
    );

    /** Get the alias host names for the specified host. This includes all DNS
       names, CNAMEs, names in the local hosts file and IP numbers (as "dot"
       format strings) for the host.

       @return
       array of strings for each alias for the host.
     */
    static PStringArray GetHostAliases(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    static PStringArray GetHostAliases(
      const Address & addr    /// Hosts IP address
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );

    /** Determine if the specified host is actually the local machine. This
       can be any of the host aliases or multi-homed IP numbers or even
       the special number 127.0.0.1 for the loopback device.

       @return
       TRUE if the host is the local machine.
     */
    static BOOL IsLocalHost(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );

    /** Get the Internet Protocol address for the local host.

       @return
       TRUE if the IP number was returned.
     */
    virtual BOOL GetLocalAddress(
      Address & addr    /// Variable to receive hosts IP address
    );
    virtual BOOL GetLocalAddress(
      Address & addr,    /// Variable to receive peer hosts IP address
      WORD & port        /// Variable to receive peer hosts port number
    );

    /** Get the Internet Protocol address for the peer host the socket is
       connected to.

       @return
       TRUE if the IP number was returned.
     */
    virtual BOOL GetPeerAddress(
      Address & addr    /// Variable to receive hosts IP address
    );
    virtual BOOL GetPeerAddress(
      Address & addr,    /// Variable to receive peer hosts IP address
      WORD & port        /// Variable to receive peer hosts port number
    );

    /** Get the host name for the local host.

       @return
       Name of the host, or an empty string if an error occurs.
     */
    PString GetLocalHostName();

    /** Get the host name for the peer host the socket is connected to.

       @return
       Name of the host, or an empty string if an error occurs.
     */
    PString GetPeerHostName();

    /** Clear the name (DNS) cache.
     */
    static void ClearNameCache();

    /** Get the IP address that is being used as the gateway, that is, the
       computer that packets on the default route will be sent.

       The string returned may be used in the Connect() function to open that
       interface.

       Note that the driver does not need to be open for this function to work.

       @return
       TRUE if there was a gateway.
     */
    static BOOL GetGatewayAddress(
      Address & addr     /// Variable to receive the IP address.
    );

    /** Get the name for the interface that is being used as the gateway,
       that is, the interface that packets on the default route will be sent.

       The string returned may be used in the Connect() function to open that
       interface.

       Note that the driver does not need to be open for this function to work.

       @return

       String name of the gateway device, or empty string if there is none.
     */
    static PString GetGatewayInterface();

    /**
       Describes a route table entry
    */
    class RouteEntry : public PObject
    {
      PCLASSINFO(RouteEntry, PObject);
      public:
        /// create a route table entry from an IP address
        RouteEntry(const Address & addr) : network(addr) { }

        /// Get the network address associated with the route table entry
        Address GetNetwork() const { return network; }

        /// Get the network address mask associated with the route table entry
        Address GetNetMask() const { return net_mask; }

        /// Get the default gateway address associated with the route table entry
        Address GetDestination() const { return destination; }

        /// Get the network address name associated with the route table entry
        const PString & GetInterface() const { return interfaceName; }

        /// Get the network metric associated with the route table entry
        long GetMetric() const { return metric; }

      protected:
        Address network;
        Address net_mask;
        Address destination;
        PString interfaceName;
        long    metric;

      friend class PIPSocket;
    };

    PLIST(RouteTable, RouteEntry);

    /** Get the systems route table.

       @return
       TRUE if the route table is returned, FALSE if an error occurs.
     */
    static BOOL GetRouteTable(
      RouteTable & table      /// Route table
    );


    /**
      Describes an interface table entry
     */
    class InterfaceEntry : public PObject
    {
      PCLASSINFO(InterfaceEntry, PObject)

      public:
        /// create an interface entry from a name, IP addr and MAC addr
        InterfaceEntry(const PString & _name, const Address & _addr, const PString & _macAddr);

        /// Print to specified stream
        virtual void PrintOn(
          ostream &strm   // Stream to print the object into.
        ) const;

        /// Get the name of the interface
        const PString & GetName() const { return name; }

        /// Get the address associated with the interface
        Address GetAddress() const { return ipAddr; }

        /// Get the MAC address associate with the interface
        const PString & GetMACAddress() const { return macAddr; }

      protected:
        PString name;
        Address ipAddr;
        PString macAddr;
    };

    PLIST(InterfaceTable, InterfaceEntry);

    /** Get a list of all interfaces

       @return
       TRUE if the interface table is returned, FALSE if an error occurs.
     */
    static BOOL GetInterfaceTable(
      InterfaceTable & table      /// interface table
    );

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
