/*
 * $Id: tcpsock.h,v 1.11 1995/06/17 00:47:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * TCP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
 * Revision 1.11  1995/06/17 00:47:31  robertj
 * Changed overloaded Open() calls to 3 separate function names.
 * More logical design of port numbers and service names.
 *
 * Revision 1.10  1995/06/04 12:46:25  robertj
 * Slight redesign of port numbers on sockets.
 *
 * Revision 1.9  1995/03/14 12:42:46  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.8  1995/03/12  04:46:40  robertj
 * Added more functionality.
 *
 * Revision 1.7  1995/01/03  09:36:22  robertj
 * Documentation.
 *
 * Revision 1.6  1995/01/01  01:07:33  robertj
 * More implementation.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Changed type of socket port number for better portability.
 * Added Out of Band data functions.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PTCPSOCKET

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PTCPSocket, PIPSocket)
/* Create a socket channel that uses the TCP transport on the Internal
   Protocol.
 */

  public:
    PTCPSocket(
      WORD port = 0             // Port number to use for the connection.
    );
    PTCPSocket(
      const PString & service   // Service name to use for the connection.
    );
    PTCPSocket(
      const PString & address,  // Address of remote machine to connect to.
      WORD port                 // Port number to use for the connection.
    );
    PTCPSocket(
      const PString & address,  // Address of remote machine to connect to.
      const PString & service   // Service name to use for the connection.
    );
    PTCPSocket(
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


  // New functions for class
    virtual BOOL WriteOutOfBand(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len          // Number of bytes pointed to by <CODE>buf</CODE>.
    );
    /* Write out of band data from the TCP/IP stream. This data is sent as TCP
       URGENT data which does not follow the usual stream sequencing of the
       normal channel data.

       This is subject to the write timeout and sets the lastWriteCount
       variable in the same way as usual <A><CODE>Write()</CODE></A> function.
       
       <H2>Returns:</H2>
       TRUE if all the bytes were sucessfully written.
     */

    virtual void OnOutOfBand(
      const void * buf,   // Data to be received as URGENT TCP data.
      PINDEX len          // Number of bytes pointed to by <CODE>buf</CODE>.
    );
    /* This is callback function called by the system whenever out of band data
       from the TCP/IP stream is received. A descendent class may interpret
       this data according to the semantics of the high level protocol.

       The default behaviour is for the out of band data to be ignored.
     */


  protected:

// Class declaration continued in platform specific header file ///////////////
