/*
 * $Id: ipdsock.h,v 1.1 1996/05/15 21:11:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * IP Datagram Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipdsock.h,v $
 * Revision 1.1  1996/05/15 21:11:16  robertj
 * Initial revision
 *
 */

#define _PIPDATAGRAMSOCKET

#ifdef __GNUC__
#pragma interface
#endif

PDECLARE_CLASS(PIPDatagramSocket, PIPSocket)
  public:
    PIPDatagramSocket(
      WORD type,
      WORD port
    );
    PIPDatagramSocket(
      WORD type,
      const char * protocol,
      const PString & service
    );
    PIPDatagramSocket(
      WORD type,
      const char * protocolName
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */

    // Overrides from class PIPSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    /* Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       <A>PIPSocket::SetPort()</A> function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    virtual BOOL Listen(
      unsigned queueSize = 5,  // Number of pending accepts that may be queued.
      WORD port = 0,           // Port number to use for the connection.
      Reusability reuse = AddressIsExclusive // Can/Cant listen more than once.
    );
    /* Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the <CODE>port</CODE> parameter is zero then the port number as
       defined by the object instance construction or the
       <A>PIPSocket::SetPort()</A> function.

       For the UDP protocol, the <CODE>queueSize</CODE> parameter is ignored.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */

    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number.

       You cannot accept on a UDP socket, so this function asserts.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


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


  protected:
    BOOL _Socket();
    /* Create a socket with correct protocol and type
     */

    WORD sockType;
    WORD protocol;


// Class declaration continued in platform specific header file ///////////////
