/*
 * $Id: ipsock.h,v 1.4 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.4  1994/08/22 00:46:48  robertj
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

#ifdef __GNU__
#pragma interface
#endif


#ifndef _PSOCKET
#include <socket.h>
#endif

PDECLARE_CLASS(PIPSocket, PSocket)
  public:

#ifdef P_HAS_BERKELEY_SOCKETS

    BOOL LookupHost(const PString & host_address, sockaddr_in * address);

#endif


// Class declaration continued in platform specific header file ///////////////
