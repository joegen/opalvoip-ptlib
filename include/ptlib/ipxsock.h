/*
 * ipxsock.h
 *
 * IPX protocol socket I/O channel class.
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
 * $Log: ipxsock.h,v $
 * Revision 1.4  1998/09/23 06:20:47  robertj
 * Added open source copyright license.
 *
 * Revision 1.3  1996/10/08 13:21:04  robertj
 * More IPX implementation.
 *
 * Revision 1.1  1996/09/14 13:00:56  robertj
 * Initial revision
 *
 */

#define _PIPXSOCKET

#ifdef __GNUC__
#pragma interface
#endif


#ifndef _PSOCKET
#include <socket.h>
#endif

PDECLARE_CLASS(PIPXSocket, PSocket)
/* This class describes a type of socket that will communicate using the
   IPX/SPX protocols.
 */

  public:
    PIPXSocket(
      WORD port = 0       // Port number to use for the connection.
    );
    /* Create a new IPX datagram socket.
     */


  public:
    class Address {
      public:
        union {
          struct {
            BYTE b1,b2,b3,b4;
          } b;
          struct {
            WORD w1,s_w2;
          } w;
          DWORD dw;
        } network;
        BYTE node[6];

        Address();
        Address(const Address & addr);
        Address(const PString & str);
        Address(DWORD netNum, const char * nodeNum);
        Address & operator=(const Address & addr);
        operator PString() const;
        BOOL IsValid() const;
      friend ostream & operator<<(ostream & s, Address & a)
        { return s << (PString)a; }
    };

  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an
       IPX/SPX socket this returns the network number, node number of the
       peer the socket is connected to, followed by the socket number it
       is connected to.

       <H2>Returns:</H2>
       the name of the channel.
     */


  // Overrides from class PSocket
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      const Address & address   // Address of remote machine to connect to.
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


  // New functions for class
    BOOL SetPacketType(
      int type    // IPX packet type for this socket.
    );
    /* Sets the packet type for datagrams sent by this socket.

       <H2>Returns:</H2>
       TRUE if the type was successfully set.
     */

    int GetPacketType();
    /* Gets the packet type for datagrams sent by this socket.

       <H2>Returns:</H2>
       type of packets or -1 if error.
     */


    static PString GetHostName(
      const Address & addr    // Hosts IP address to get name for
    );
    /* Get the host name for the host specified server.

       <H2>Returns:</H2>
       Name of the host or IPX number of host.
     */

    static BOOL GetHostAddress(
      Address & addr    // Variable to receive this hosts IP address
    );
    static BOOL GetHostAddress(
      const PString & hostname,
      /* Name of host to get address for. This may be either a server name or
         an IPX number in "colon" format.
       */
      Address & addr    // Variable to receive hosts IPX address
    );
    /* Get the IPX address for the specified host.

       <H2>Returns:</H2>
       TRUE if the IPX number was returned.
     */

    BOOL GetLocalAddress(
      Address & addr    // Variable to receive hosts IPX address
    );
    BOOL GetLocalAddress(
      Address & addr,    // Variable to receive peer hosts IPX address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the IPX/SPX address for the local host.

       <H2>Returns:</H2>
       TRUE if the IPX number was returned.
     */

    BOOL GetPeerAddress(
      Address & addr    // Variable to receive hosts IPX address
    );
    BOOL GetPeerAddress(
      Address & addr,    // Variable to receive peer hosts IPX address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the IPX/SPX address for the peer host the socket is
       connected to.

       <H2>Returns:</H2>
       TRUE if the IPX number was returned.
     */


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
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;


// Class declaration continued in platform specific header file ///////////////
