/*
 * $Id: socket.h,v 1.17 1996/02/25 03:02:14 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.17  1996/02/25 03:02:14  robertj
 * Moved some socket functions to platform dependent code.
 * Added array of fds to os_select for unix threading support.
 *
 * Revision 1.16  1996/02/15 14:46:43  robertj
 * Added Select() function to PSocket.
 *
 * Revision 1.15  1995/12/23 03:46:54  robertj
 * Fixed portability issue with closingh sockets.
 *
 * Revision 1.14  1995/12/10 11:35:21  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.13  1995/10/14 15:05:54  robertj
 * Added functions for changing integer from host to network byte order.
 *
 * Revision 1.12  1995/06/17 11:13:25  robertj
 * Documentation update.
 *
 * Revision 1.11  1995/06/17 00:44:35  robertj
 * More logical design of port numbers and service names.
 * Changed overloaded Open() calls to 3 separate function names.
 *
 * Revision 1.10  1995/06/04 12:36:37  robertj
 * Slight redesign of port numbers on sockets.
 *
 * Revision 1.9  1995/03/14 12:42:39  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.8  1995/03/12  04:45:40  robertj
 * Added more functionality.
 *
 * Revision 1.7  1995/01/03  09:36:19  robertj
 * Documentation.
 *
 * Revision 1.6  1995/01/02  12:16:17  robertj
 * Moved constructor to platform dependent code.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Changed type of socket port number for better portability.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PSOCKET

#ifdef __GNUC__
#pragma interface
#endif


#ifndef _PCHANNEL
#include <channel.h>
#endif

class PSocket;

PLIST(PSocketList, PSocket);


PDECLARE_CLASS(PSocket, PChannel)
/* A network communications channel. This is based on the concepts in the
   Berkley Sockets library.
   
   A socket represents a bidirectional communications channel to a <I>port</I>
   at a remote <I>host</I>.
 */

  public:
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    ) = 0;
    /* Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    virtual BOOL Listen(
      unsigned queueSize = 5,  // Number of pending accepts that may be queued.
      WORD port = 0            // Port number to use for the connection.
    ) = 0;
    /* Listen on a socket for a remote host on the specified port number. This
       may be used for server based applications. A "connecting" socket begins
       a connection by initiating a connection to this socket. An active socket
       of this type is then used to generate other "accepting" sockets which
       establish a two way communications channel with the "connecting" socket.

       If the <CODE>port</CODE> parameter is zero then the port number as
       defined by the object instance construction or the descendent classes
       SetPort() function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    ) = 0;
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

    PDECLARE_CLASS(SelectList, PSocketList)
      public:
        SelectList()
          { DisallowDeleteObjects(); }
        void operator+=(PSocket & sock)
          { Append(&sock); }
        void operator-=(PSocket & sock)
          { Remove(&sock); }
    };

    static int Select(
      PSocket & sock1,        // First socket to check for readability.
      PSocket & sock2         // Second socket to check for readability.
    );
    static int Select(
      PSocket & sock1,        // First socket to check for readability.
      PSocket & sock2,        // Second socket to check for readability.
      const PTimeInterval & timeout // Timeout for wait on read/write data.
    );
    static BOOL Select(
      SelectList & read       // List of sockets to check for readability.
    );
    static BOOL Select(
      SelectList & read,      // List of sockets to check for readability.
      const PTimeInterval & timeout // Timeout for wait on read/write data.
    );
    static BOOL Select(
      SelectList & read,      // List of sockets to check for readability.
      SelectList & write      // List of sockets to check for writability.
    );
    static BOOL Select(
      SelectList & read,      // List of sockets to check for readability.
      SelectList & write,     // List of sockets to check for writability.
      const PTimeInterval & timeout // Timeout for wait on read/write data.
    );
    static BOOL Select(
      SelectList & read,      // List of sockets to check for readability.
      SelectList & write,     // List of sockets to check for writability.
      SelectList & except     // List of sockets to check for exceptions.
    );
    static BOOL Select(
      SelectList & read,      // List of sockets to check for readability.
      SelectList & write,     // List of sockets to check for writability.
      SelectList & except,    // List of sockets to check for exceptions.
      const PTimeInterval & timeout // Timeout for wait on read/write data.
    );
    /* Select a socket with available data. This function will block until the
       timeout or data is available to be read or written to the specified
       sockets.

       The <CODE>read</CODE>, <CODE>write</CODE> and <CODE>except</CODE> lists
       are modified by the call so that only the sockets that have data
       available are present. If the call timed out then all of these lists
       will be empty.

       If no timeout is specified then the call will block until a socket
       has data available.

       <H2>Returns:</H2>
       TRUE if the select was successful or timed out, FALSE if an error
       occurred. If a timeout occurred then the lists returned will be empty.

       For the versions taking sockets directly instead of lists the integer
       returned is -1 for an error, 0 for a timeout, 1 for the first socket
       having read data, 2 for the second socket and 3 for both.
     */


#ifdef P_HAS_BERKELEY_SOCKETS
    inline static WORD  Host2Net(WORD  v) { return htons(v); }
    inline static DWORD Host2Net(DWORD v) { return htonl(v); }
      // Convert from host to network byte order

    inline static WORD  Net2Host(WORD  v) { return ntohs(v); }
    inline static DWORD Net2Host(DWORD v) { return ntohl(v); }
      // Convert from network to host byte order
#else
    inline static WORD  Host2Net(WORD  v);
    inline static DWORD Host2Net(DWORD v);
      // Convert from host to network byte order

    inline static WORD  Net2Host(WORD  v);
    inline static DWORD Net2Host(DWORD v);
      // Convert from network to host byte order
#endif


  protected:
    int _Close();
    // Close the socket without setting errors.


    int os_socket(
      int af,
      int type,
      int protocol
    );
    int os_connect(
      struct sockaddr * sin,
      int size
    );
    int os_accept(
      int sock,
      struct sockaddr * addr,
      int * size
    );
    static int os_select(
      int maxfds,
      fd_set & readfds,
      fd_set & writefds,
      fd_set & exceptfds,
      const PIntArray & allfds,
      const PTimeInterval & timeout
    );


// Class declaration continued in platform specific header file ///////////////
