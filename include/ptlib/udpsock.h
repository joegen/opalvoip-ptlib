/*
 * $Id: udpsock.h,v 1.6 1995/06/17 00:48:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * UDP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: udpsock.h,v $
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


PDECLARE_CLASS(PUDPSocket, PIPSocket)
/* Create a socket checnnel that uses the UDP transport on the Internal
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
    PUDPSocket(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


  // Overrides from class PSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    /* Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       <A><CODE>SetPort()</CODE></A> function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    virtual BOOL Listen(
      unsigned queueSize = 5,  // Number of pending accepts that may be queued.
      WORD port = 0             // Port number to use for the connection.
    );
    /* Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the <CODE>port</CODE> parameter is zero then the port number as
       defined by the object instance construction or the
       <A><CODE>SetPort()</CODE></A> function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


  // Overrides from class PIPSocket.
    virtual WORD GetPortByService(
      const PString & service   // Name of service to get port number for.
    ) const;
    /* Get the port number for the specified service.
    
       <H2>Returns:</H2>
       Port number for service name.
     */

    virtual PString GetServiceByPort(
      WORD port   // Number for service to find name of.
    ) const;
    /* Get the service name from the port number.
    
       <H2>Returns:</H2>
       Service name for port number.
     */




// Class declaration continued in platform specific header file ///////////////
