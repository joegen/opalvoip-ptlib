/*
 * $Id: udpsock.h,v 1.12 1997/06/06 10:54:11 craigs Exp $
 *
 * Portable Windows Library
 *
 * UDP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: udpsock.h,v $
 * Revision 1.12  1997/06/06 10:54:11  craigs
 * Added overrides and new functions for connectionless Writes
 *
 * Revision 1.11  1996/09/14 13:09:43  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.10  1996/05/15 10:19:15  robertj
 * Added ICMP protocol socket, getting common ancestor to UDP.
 *
 * Revision 1.9  1996/03/03 07:38:00  robertj
 * Added Reusability clause to the Listen() function on sockets.
 *
 * Revision 1.8  1995/12/10 11:44:45  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.7  1995/06/17 11:13:41  robertj
 * Documentation update.
 *
 * Revision 1.6  1995/06/17 00:48:01  robertj
 * Implementation.
 *
 * Revision 1.5  1995/01/03 09:36:24  robertj
 * Documentation.
 *
 * Revision 1.4  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.3  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PUDPSOCKET

#ifdef __GNUC__
#pragma interface
#endif

PDECLARE_CLASS(PUDPSocket, PIPDatagramSocket)
/* Create a socket channel that uses the UDP transport on the Internal
   Protocol.
 */

  public:
    PUDPSocket(
      WORD port = 0             // Port number to use for the connection.
    );
    PUDPSocket(
      const PString & service   // Service name to use for the connection.
    );
    PUDPSocket(
      const PString & address,  // Address of remote machine to connect to.
      WORD port                 // Port number to use for the connection.
    );
    PUDPSocket(
      const PString & address,  // Address of remote machine to connect to.
      const PString & service   // Service name to use for the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */

    BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Override of PChannel functions to allow connectionless writes
     */

    BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    /* Override of PSocket functions to allow connectionless writes
     */

    void SetSendAddress(const Address & address, WORD port);
    /* Set the address to use for connectionless Write
     */

  protected:
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;

    Address sendAddress;
    WORD sendPort;

// Class declaration continued in platform specific header file ///////////////
