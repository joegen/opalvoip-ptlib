/*
 * $Id: socket.h,v 1.2 1994/07/25 03:36:03 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.2  1994/07/25 03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PSOCKET

#ifndef _PCHANNEL
#include <channel.h>
#endif

PDECLARE_CLASS(PSocket, PChannel)
  public:
    Socket();
      // create an unattached socket

    virtual BOOL Open (const PString & address, int portnum);
      // connect to another host 

    virtual BOOL Accept (const PString & address);
      // wait for another host to establish a connection 


// Class declaration continued in platform specific header file ///////////////
