/*
 * $Id: ipsock.h,v 1.14 1995/10/14 14:57:26 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
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
      WORD port = 0           // Port number to use for the connection.
    );
    PIPSocket(
      const char * protocol,  // Protocol name to use for port look up.
      const char * service    // Service name to use for the connection.
    );

  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       <H2>Returns:</H2>
       the name of the channel.
     */


  // Overrides from class PSocket.
    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number. This is an
       "accepting" socket. When a "listening" socket has a pending connection
       to make, this will accept a connection made by the "connecting" socket
       created to establish a link.

       The port that the socket uses is the one used in the <A>Listen()</A>
       command of the <CODE>socket</CODE> parameter.

       Note that this function will block until a remote system connects to the
       port number specified in the "listening" socket.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
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
        Address & operator=(const Address & addr);
        operator PString() const;
        operator DWORD() const;
    };

    static BOOL GetAddress(
      const PString & hostname,
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the specified host.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    static PStringArray GetHostAliases(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    /* Get the alias host names for the specified host.

       <H2>Returns:</H2>
       array of strings for each alias for the host.
     */

    static BOOL GetHostAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the host this process is running
       on.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    static PString GetHostName();
    /* Get the host name for the host this process is running on.

       <H2>Returns:</H2>
       Name of the host, or an empty string if an error occurs.
     */

    BOOL GetLocalAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the local host.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    BOOL GetPeerAddress(
      Address & addr    // Variable to receive hosts IP address
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
       string service name.
     */


    virtual WORD GetPortByService(
      const PString & service   // Name of service to get port number for.
    ) const = 0;
    /* Get the port number for the specified service.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The <A>PTCPSocket</A> and <A>PUDPSocket</A>
       classes will implement this function.

       <H2>Returns:</H2>
       Port number for service name.
     */

    virtual PString GetServiceByPort(
      WORD port   // Number for service to find name of.
    ) const = 0;
    /* Get the service name from the port number.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The <A>PTCPSocket</A> and <A>PUDPSocket</A>
       classes will implement this function.

       <H2>Returns:</H2>
       Service name for port number.
     */


  protected:
    static WORD GetPortByService(
      const char * protocol,  // Protocol type for port lookup
      const char * service    // Name of service to get port number for.
    );
    /* Get the port number for the specified service.

       <H2>Returns:</H2>
       Port number for service name and protocol.
     */

    static PString GetServiceByPort(
      const char * protocol,  // Protocol type for port lookup
      WORD port   // Number for service to find name of.
    );
    /* Get the service name from the port number.

       <H2>Returns:</H2>
       Service name for port number and protocol.
     */


#ifdef P_HAS_BERKELEY_SOCKETS
    BOOL _Socket(
      int type  // Type of socket to open.
    );
    /* Create a socket using the specified protocol.

       <H2>Returns:</H2>
       TRUE if successful.
     */

    BOOL _Connect(
      const PString & host  // IP number of remote host to connect to.
    );
    /* Connect a socket to the specified host.

       <H2>Returns:</H2>
       TRUE if successful.
     */

    BOOL _Listen(
      unsigned queueSize   // Number of pending accepts that may be queued.
    );
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
