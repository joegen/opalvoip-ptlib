/*
 * $Id: socket.h,v 1.7 1995/01/03 09:36:19 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.7  1995/01/03 09:36:19  robertj
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
    // Open a connection to another host using the port number.

    virtual BOOL Accept(
      const PString & hostname
    );
    // Wait for another host to establish a connection 


// Class declaration continued in platform specific header file ///////////////
