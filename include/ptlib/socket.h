/*
 * $Id: socket.h,v 1.3 1994/08/21 23:43:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.3  1994/08/21 23:43:02  robertj
 * Changed type of socket port number for better portability.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PSOCKET

#ifndef _PCHANNEL
#include <channel.h>
#endif

PDECLARE_CLASS(PSocket, PChannel)
  public:
    PSocket();
      // create an unattached socket

    virtual BOOL Open (const PString & address, WORD portnum);
      // connect to another host 

    virtual BOOL Accept (const PString & address);
      // wait for another host to establish a connection 


// Class declaration continued in platform specific header file ///////////////
