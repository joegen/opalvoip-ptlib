/*
 * $Id: socket.h,v 1.8 1995/03/12 04:45:40 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.8  1995/03/12 04:45:40  robertj
 * Added more functionality.
 *
 * Revision 1.7  1995/01/03  09:36:19  robertj
 * Documentation.
 *
 * Revision 1.6  1995/01/02  12:16:17  robertj
 * Moved constructor to platform dependent code.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Changed type of socket port number for better portability.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PSOCKET

#ifdef __GNUC__
#pragma interface
#endif


#ifndef _PCHANNEL
#include <channel.h>
#endif


PDECLARE_CLASS(PSocket, PChannel)
/* A network communications channel. This is based on the concepts in the
   Berkley Sockets library.
   
   A socket represents a bidirectional communications channel to a $I$port$I$
   at a remote $I$host$I$.
 */

  public:
    virtual BOOL Open(
      const PString & hostname,   // Remote host address.
      WORD portnum                // Remote port number.
    ) = 0;
    virtual BOOL Open(
      WORD port = 0             // Port number to use for the connection.
    ) = 0;
    virtual BOOL Open(
      PSocket & socket          // Listening socket making the connection.
    ) = 0;
    /* Open a socket to a remote host on the specified port number.
    
       The first form creates a "connecting" socket which is typically the
       client or initiator of a communications channel. This connects to a
       "listening" socket at the other end of the communications channel.

       The second form creates a "listening" socket which may be used for
       server based applications. A "connecting" socket begins a connection by
       initiating a connection to this socket. An active socket of this type
       is then used to generate other "accepting" sockets which establish a
       two way communications channel with the "connecting" socket.

       The third form opens an "accepting" socket. When a "listening" socket
       has a pending connection to make, this will accept a connection made
       by the "connecting" socket created to establish a link.

       If the $B$port$B$ parameter is zero then the port number as defined by
       the object instance construction or the $B$SetPort()$B$ function is
       used.

       Returns: TRUE if the channel was successfully opened.
     */


// Class declaration continued in platform specific header file ///////////////
