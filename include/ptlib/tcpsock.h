/*
 * $Id: tcpsock.h,v 1.3 1994/08/21 23:43:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * TCP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
 * Revision 1.3  1994/08/21 23:43:02  robertj
 * Changed type of socket port number for better portability.
 * Added Out of Band data functions.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PTCPSOCKET

PDECLARE_CLASS(PTCPSocket, PIPSocket)

  public:
    virtual BOOL Open(const PString & address, WORD port);
      // Open a socket to a remote host on the specified port number

    virtual BOOL ReadOutOfBand(void * buf, PINDEX len);
      // Read out of band data from the TCP/IP stream. This is subject to the
      // read timeout and sets the lastReadCount variable in the same way as
      // usual Read() function.

    virtual BOOL WriteOutOfBand(const void * buf, PINDEX len);
      // Write out of band data from the TCP/IP stream. This is subject to the
      // write timeout and sets the lastWriteCount variable in the same way as
      // usual Write() function.


// Class declaration continued in platform specific header file ///////////////
