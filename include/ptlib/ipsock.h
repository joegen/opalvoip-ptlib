/*
 * $Id: ipsock.h,v 1.2 1994/07/25 03:36:03 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.2  1994/07/25 03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PIPSOCKET

#ifndef _PSOCKET
#include <socket.h>
#endif

PDECLARE_CLASS(PIPSocket, PSocket)
  public:


// Class declaration continued in platform specific header file ///////////////
