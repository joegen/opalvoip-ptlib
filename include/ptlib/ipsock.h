/*
 * $Id: ipsock.h,v 1.28 1996/12/17 11:08:05 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
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
#include <socket.h>
#endif

PDECLARE_CLASS(PIPSocket, PSocket)
/* This class describes a type of socket that will communicate using the
   Internet Protocol.
 */

  protected:
    PIPSocket();
    /* Create a new Internet Protocol socket based on the port number
       specified.
     */


  public:
    class Address : public in_addr {
      public:
        Address();
        Address(const PString & dotNotation);
        Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4);
        Address(const in_addr & addr);
        Address(const Address & addr);
        Address & operator=(const in_addr & addr);
        Address & operator=(const Address & addr);
        operator PString() const;
        operator DWORD() const;
        BYTE Byte1() const;
        BYTE Byte2() const;
        BYTE Byte3() const;
        BYTE Byte4() const;
      friend ostream & operator<<(ostream & s, const Address & a);
    };


  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       <H2>Returns:</H2>
       the name of the channel.
     */


  // Overrides from class PSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      const Address & addr      // Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      WORD localPort,           // Local port number for connection
      const Address & addr      // Address of remote machine to connect to.
    );
    /* Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       <A>PIPSocket::SetPort()</A> function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */

    virtual BOOL Listen(
      unsigned queueSize = 5,  // Number of pending accepts that may be queued.
      WORD port = 0,           // Port number to use for the connection.
      Reusability reuse = AddressIsExclusive // Can/Cant listen more than once.
    );
    /* Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the <CODE>port</CODE> parameter is zero then the port number as
       defined by the object instance construction or the
       <A>PIPSocket::SetPort()</A> function.

       For the UDP protocol, the <CODE>queueSize</CODE> parameter is ignored.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


  // New functions for class
    static PString GetHostName();
    static PString GetHostName(
      const PString & hostname  // Hosts IP address to get name for
    );
    static PString GetHostName(
      const Address & addr    // Hosts IP address to get name for
    );
    /* Get the "official" host name for the host specified or if none, the host
       this process is running on. The host may be specified as an IP number
       or a hostname alias and is resolved to the canonical form.

       <H2>Returns:</H2>
       Name of the host or IP number of host.
     */

    static BOOL GetHostAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    static BOOL GetHostAddress(
      const PString & hostname,
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the specified host, or if none
       specified, for the host this process is running on.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    static PStringArray GetHostAliases(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    static PStringArray GetHostAliases(
      const Address & addr    // Hosts IP address
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    /* Get the alias host names for the specified host. This includes all DNS
       names, CNAMEs, names in the local hosts file and IP numbers (as "dot"
       format strings) for the host.

       <H2>Returns:</H2>
       array of strings for each alias for the host.
     */

    static BOOL IsLocalHost(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    /* Determine if the specified host is actually the local machine. This
       can be any of the host aliases or multi-homed IP numbers or even
       the special number 127.0.0.1 for the loopback device.

       <H2>Returns:</H2>
       TRUE if the host is the local machine.
     */

    BOOL GetLocalAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    BOOL GetLocalAddress(
      Address & addr,    // Variable to receive peer hosts IP address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the Internet Protocol address for the local host.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    BOOL GetPeerAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    BOOL GetPeerAddress(
      Address & addr,    // Variable to receive peer hosts IP address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the Internet Protocol address for the peer host the socket is
       connected to.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    PString GetLocalHostName();
    /* Get the host name for the local host.

       <H2>Returns:</H2>
       Name of the host, or an empty string if an error occurs.
     */

    PString GetPeerHostName();
    /* Get the host name for the peer host the socket is connected to.

       <H2>Returns:</H2>
       Name of the host, or an empty string if an error occurs.
     */

    static void ClearNameCache();
    /* Clear the name cache.
     */


// Class declaration continued in platform specific header file ///////////////
