/*
 * spxsock.h
 *
 * SPX socket channel class
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: spxsock.h,v $
 * Revision 1.2  1998/09/23 06:21:29  robertj
 * Added open source copyright license.
 *
 * Revision 1.1  1996/09/14 13:00:56  robertj
 * Initial revision
 *
 */

#define _PSPXSOCKET

#ifdef __GNUC__
#pragma interface
#endif

#include <ipxsock.h>


PDECLARE_CLASS(PSPXSocket, PIPXSocket)
/* Create a socket channel that uses the SPX transport over the IPX
   Protocol.
 */

  public:
    PSPXSocket(
      WORD port = 0             // Port number to use for the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.

       Note thate the "copy" constructor here is areally a "listening" socket
       the same as the PSocket & parameter version.
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


  protected:
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;


// Class declaration continued in platform specific header file ///////////////
