/*
 * $Id: ipsock.h,v 1.9 1995/03/12 04:38:41 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.9  1995/03/12 04:38:41  robertj
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

  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an IP
       socket this returns the host name of the peer the socket is connected
       to, followed by the socket number it is connected to.

       Returns: the name of the channel.
     */


  // New functions for class
    typedef BYTE Address[4];

    static BOOL GetAddress(
      const PString & hostname,
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the specified host.

       Returns: TRUE if the IP number was returned.
     */

    static PStringArray GetHostAliases(
      const PString & hostname
      /* Name of host to get address for. This may be either a domain name or
         an IP number in "dot" format.
       */
    );
    /* Get the alias host names for the specified host.

       Returns: array of strings for each alias for the host.
     */

    BOOL GetLocalAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the local host.

       Returns: TRUE if the IP number was returned.
     */

    BOOL GetPeerAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    /* Get the Internet Protocol address for the peer host the socket is
       connected to.

       Returns: TRUE if the IP number was returned.
     */

    PString GetLocalHostName();
    /* Get the host name for the local host.

       Returns: Name of the host, or an empty string if an error occurs.
     */

    PString GetPeerHostName();
    /* Get the host name for the peer host the socket is connected to.

       Returns: Name of the host, or an empty string if an error occurs.
     */


    virtual WORD GetPort(
      const PString & service   // Name of service to get port number for.
    ) const = 0;
    /* Get the port number for the specified service.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The $H$PTCPSocket and $H$PUDPSocket classes
       will implement this function.
     */

    virtual PString GetService(
      WORD port   // Number for service to find name of.
    ) const = 0;
    /* Get the service name from the port number.
    
       The exact behviour of this function is dependent on whether TCP or UDP
       transport is being used. The $H$PTCPSocket and $H$PUDPSocket classes
       will implement this function.
     */


// Class declaration continued in platform specific header file ///////////////
