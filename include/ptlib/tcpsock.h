/*
 * $Id: tcpsock.h,v 1.2 1994/07/25 03:36:03 robertj Exp $
 *
 * Portable Windows Library
 *
 * TCP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
 * Revision 1.2  1994/07/25 03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PTCPSOCKET

PDECLARE_CLASS(PTCPSocket, PIPSocket)

  public:
    virtual BOOL Open (const PString address, int port);
      // Open a socket to a remote host on the specified socket


// Class declaration continued in platform specific header file ///////////////
