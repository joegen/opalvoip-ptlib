/*
 * $Id: ipsock.h,v 1.22 1996/03/26 00:51:13 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
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

  public:
    PIPSocket(
      WORD port = 0              // Port number to use for the connection.
    );
    PIPSocket(
      const char * protocol,     // Protocol name to use for port look up.
      const PString & service    // Service name to use for the connection.
    );
    /* Create a new Internet Protocol socket.

       A service name is a unique string contained in a system database. The
       parameter here may be either this unique name, an integer value or both
       separated by a space (name then integer). In the latter case the
       integer value is used if the name cannot be found in the database.
     */


  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       <H2>Returns:</H2>
       the name of the channel.
     */


  // New functions for class
#ifdef P_HAS_BERKELEY_SOCKETS
    class Address : public in_addr {
      public:
        Address(const in_addr & addr);
        Address & operator=(const in_addr & addr);
#else
    class Address {
      private:
        union {
          struct {
            BYTE s_b1,s_b2,s_b3,s_b4;
          } S_un_b;
          struct {
            WORD s_w1,s_w2;
          } S_un_w;
          DWORD S_addr;
        } S_un;
#endif
      public:
        Address();
        Address(const Address & addr);
        Address(const PString & dotNotation);
        Address(BYTE b1, BYTE b2, BYTE b3, BYTE b4);
        Address & operator=(const Address & addr);
        operator PString() const;
        operator DWORD() const;
        BYTE Byte1() const;
        BYTE Byte2() const;
        BYTE Byte3() const;
        BYTE Byte4() const;
      friend ostream & operator<<(ostream & s, Address & a)
        { return s << (PString)a; }
    };

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


    void SetPort(
      WORD port   // New port number for the channel.
    );
    void SetPort(
      const PString & service   // Service name to describe the port number.
    );
    /* Set the port number for the channel. This a 16 bit number representing
       an agreed high level protocol type. The string version looks up a
       database of names to find the number for the string name.

       A service name is a unique string contained in a system database. The
       parameter here may be either this unique name, an integer value or both
       separated by a space (name then integer). In the latter case the
       integer value is used if the name cannot be found in the database.
    
       The port number may not be changed while the port is open and the
       function will assert if an attempt is made to do so.
     */

    WORD GetPort() const;
    /* Get the port the TCP socket channel object instance is using.

       <H2>Returns:</H2>
       Port number.
     */

    PString GetService() const;
    /* Get a service name for the port number the TCP socket channel object
       instance is using.

       <H2>Returns:</H2>
       string service name or a string representation of the port number if no
       service with that number can be found.
     */


    virtual WORD GetPortByService(
      const PString & service   // Name of service to get port number for.
    ) const = 0;
    static WORD GetPortByService(
      const char * protocol,     // Protocol type for port lookup
      const PString & service    // Name of service to get port number for.
    );
    /* Get the port number for the specified service name.
    
       A name is a unique string contained in a system database. The parameter
       here may be either this unique name, an integer value or both separated
       by a space (name then integer). In the latter case the integer value is
       used if the name cannot be found in the database.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The <A>PTCPSocket</A> and <A>PUDPSocket</A>
       classes will implement this function.

       The static version of the function is independent of the socket type as
       its first parameter may be "tcp" or "udp", 

       <H2>Returns:</H2>
       Port number for service name, or 0 if service cannot be found.
     */

    virtual PString GetServiceByPort(
      WORD port   // Number for service to find name of.
    ) const = 0;
    static PString GetServiceByPort(
      const char * protocol,  // Protocol type for port lookup
      WORD port   // Number for service to find name of.
    );
    /* Get the service name from the port number.
    
       A service name is a unique string contained in a system database. The
       parameter here may be either this unique name, an integer value or both
       separated by a space (name then integer). In the latter case the
       integer value is used if the name cannot be found in the database.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The <A>PTCPSocket</A> and <A>PUDPSocket</A>
       classes will implement this function.

       The static version of the function is independent of the socket type as
       its first parameter may be "tcp" or "udp", 

       <H2>Returns:</H2>
       Service name for port number.
     */


  protected:
#ifdef P_HAS_BERKELEY_SOCKETS
    BOOL _Connect(
      const PString & host  // IP number of remote host to connect to.
    );
    /* Connect a socket to the specified host.

       <H2>Returns:</H2>
       TRUE if successful.
     */

    BOOL _Bind(Reusability reuse);
    /* Bind a socket to the protocol and listen for connections from remote
       hosts.

       <H2>Returns:</H2>
       TRUE if successful.
     */
#endif


  // Member variables
    WORD port;
    // Port to be used by the socket when opening the channel.


// Class declaration continued in platform specific header file ///////////////
