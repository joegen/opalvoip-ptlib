/*
 * $Id: tcpsock.h,v 1.10 1995/06/04 12:46:25 robertj Exp $
 *
 * Portable Windows Library
 *
 * TCP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
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


  // New functions for class.
    virtual BOOL Open(
      const PString & address   // Address of remote machine to connect to.
    );
    virtual BOOL Open(
      WORD port = 0             // Port number to use for the connection.
    );
    virtual BOOL Open(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number.
    
       The first form creates a "connecting" socket which is typically the
       client or initiator of a communications channel. This connects to a
       "listening" socket at the other end of the communications channel.

       The second form creates a "listening" socket which may be used for
       server based applications. A "connecting" socket begins a connection by
       initiating a connection to this socket. An active socket of this type
       is then used to generate other "accepting" sockets which establish a
       two way communications channel with the "connecting" socket.

       The third form opens an "accepting" socket. When a "listening" socket
       has a pending connection to make, this will accept a connection made
       by the "connecting" socket created to establish a link.

       If the <CODE>port</CODE> parameter is zero then the port number as
       defined by the object instance construction or the
       <A><CODE>SetPort()</CODE></A> function is used.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


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

    void SetPort(
      WORD port   // New port number for the channel.
    );
    void SetPort(
      const PString & service   // Service name to describe the port number.
    );
    /* Set the port number for the channel. This a 16 bit number representing
       an agreed high level protocol type. The string version looks up a
       database of names to find the number for the string name.

       The port number may not be changed while the port is open and the
       function will assert if an attempt is made to do so.
     */

    virtual WORD GetPort() const;
    virtual WORD GetPort(
      const PString & service   // Service name to describe the port number.
    ) const;
    /* Get a port number for the service name.
    
       The parameterless form gets the port the TCP socket channel object
       instance is using.

       The second form gets the port number for the specified service name.

       <H2>Returns:</H2>
       port number.
     */

    virtual PString GetService() const;
    virtual PString GetService(
      WORD port   // Port number to look up service name for.
    ) const;
    /* Get a service name for the port number.
    
       The parameterless form gets the service name for the port the TCP socket
       channel object instance is using.

       The second form gets the service name for the specified port number.

       <H2>Returns:</H2>
       string service name.
     */


  protected:
    WORD port;
    // Port to be used by the socket when opening the channel.


// Class declaration continued in platform specific header file ///////////////
