/*
 * $Id: ipdsock.h,v 1.2 1996/09/14 13:09:20 robertj Exp $
 *
 * Portable Windows Library
 *
 * IP Datagram Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipdsock.h,v $
 * Revision 1.2  1996/09/14 13:09:20  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.1  1996/05/15 21:11:16  robertj
 * Initial revision
 *
 */

#define _PIPDATAGRAMSOCKET

#ifdef __GNUC__
#pragma interface
#endif

PDECLARE_CLASS(PIPDatagramSocket, PIPSocket)
  protected:
    PIPDatagramSocket();
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


  public:
  // New functions for class
    virtual BOOL ReadFrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      Address & addr, // Address from which the datagram was received.
      WORD & port     // Port from which the datagram was received.
    );
    /* Read a datagram from a remote computer.
       
       <H2>Returns:</H2>
       TRUE if all the bytes were sucessfully written.
     */

    virtual BOOL WriteTo(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      const Address & addr, // Address to which the datagram is sent.
      WORD port           // Port to which the datagram is sent.
    );
    /* Write a datagram to a remote computer.

       <H2>Returns:</H2>
       TRUE if all the bytes were sucessfully written.
     */


// Class declaration continued in platform specific header file ///////////////
