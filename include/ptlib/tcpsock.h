/*
 * $Id: tcpsock.h,v 1.18 1998/08/21 05:24:46 robertj Exp $
 *
 * Portable Windows Library
 *
 * TCP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
 * Revision 1.18  1998/08/21 05:24:46  robertj
 * Fixed bug where write streams out to non-stream socket.
 *
 * Revision 1.17  1996/09/14 13:09:42  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.16  1996/03/26 00:57:15  robertj
 * Added contructor that takes PTCPSocket so avoid copy constructor being used instead of accept.
 *
 * Revision 1.15  1996/03/03 07:37:59  robertj
 * Added Reusability clause to the Listen() function on sockets.
 *
 * Revision 1.14  1996/02/25 03:01:27  robertj
 * Moved some socket functions to platform dependent code.
 *
 * Revision 1.13  1995/12/10 11:43:34  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.12  1995/06/17 11:13:31  robertj
 * Documentation update.
 *
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
    PTCPSocket(
      PTCPSocket & tcpSocket    // Listening socket making the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.

       Note thate the "copy" constructor here is areally a "listening" socket
       the same as the PSocket & parameter version.
     */


  // Overrides from class PChannel.
    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       This override repeatedly writes if there is no error until all of the
       requested bytes have been written.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */


  // Overrides from class PSocket.
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

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */

    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number. This is an
       "accepting" socket. When a "listening" socket has a pending connection
       to make, this will accept a connection made by the "connecting" socket
       created to establish a link.

       The port that the socket uses is the one used in the <A>Listen()</A>
       command of the <CODE>socket</CODE> parameter.

       Note that this function will block until a remote system connects to the
       port number specified in the "listening" socket.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


  // New functions for class
    virtual BOOL WriteOutOfBand(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len          // Number of bytes pointed to by <CODE>buf</CODE>.
    );
    /* Write out of band data from the TCP/IP stream. This data is sent as TCP
       URGENT data which does not follow the usual stream sequencing of the
       normal channel data.

       This is subject to the write timeout and sets the
       <CODE>lastWriteCount</CODE> member variable in the same way as usual
       <A>PChannel::Write()</A> function.
       
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
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;


// Class declaration continued in platform specific header file ///////////////
