/*
 * $Id: ipsock.h,v 1.6 1994/12/15 12:47:14 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.6  1994/12/15 12:47:14  robertj
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

#ifdef P_HAS_BERKELEY_SOCKETS

    BOOL LookupHost(
      const PString & host_address,
      sockaddr_in * address
    );
    /* Internal function used when the library is using a Berkley sockets
       compatible system. It will obtain the address in a socket ready form
       given a host domain name or dot form IP address as a string.
     */

#endif


// Class declaration continued in platform specific header file ///////////////
