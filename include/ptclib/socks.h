/*
 * main.h
 *
 * PWLib application header file for GetCyberPatrol
 *
 * Copyright 98 Equivalence
 *
 * $Log: socks.h,v $
 * Revision 1.1  1998/12/22 10:34:17  robertj
 * Initial revision
 *
 */

#ifndef _SOCKS_H
#define _SOCKS_H

#include <ptlib/sockets.h>


class PSocksSocket : public PTCPSocket
{
/* This is an ancestor class allowing access to a SOCKS servers (version 4 and 5).
 */
  PCLASSINFO(PSocksSocket, PTCPSocket)

  public:
    PSocksSocket(
      WORD port = 0
    );

  // Overrides from class PSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    virtual BOOL Connect(
      const Address & addr      // Address of remote machine to connect to.
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

    BOOL Accept();
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

       A further note is that when the version that uses a parameter is used,
       the <CODE>socket</CODE> parameter is automatically closed and its
       operating system handle transferred to the current object. This occurs
       regardless of the return value of the function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */

    virtual BOOL GetLocalAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    virtual BOOL GetLocalAddress(
      Address & addr,    // Variable to receive peer hosts IP address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the Internet Protocol address for the local host.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */

    virtual BOOL GetPeerAddress(
      Address & addr    // Variable to receive hosts IP address
    );
    virtual BOOL GetPeerAddress(
      Address & addr,    // Variable to receive peer hosts IP address
      WORD & port        // Variable to receive peer hosts port number
    );
    /* Get the Internet Protocol address for the peer host the socket is
       connected to.

       <H2>Returns:</H2>
       TRUE if the IP number was returned.
     */


  // New functions for class
    enum {
      DefaultServerPort = 1080
    };
    BOOL SetServer(
      const PString & hostname,
      const char * service = "socks 1080"
    );
    BOOL SetServer(
      const PString & hostname,
      WORD port
    );

    void SetAuthentication(
      const PString & username,
      const PString & password
    );
    /* Set the username and password for the SOCKS server authentication. This
       is for the cleartext authentication only, GSSAPI, Kerberos etc is not
       yet supported.
     */

  protected:
    virtual BOOL ConnectServer();
    virtual BOOL SendSocksCommand(BYTE command, const char * hostname, Address addr) = 0;
    virtual BOOL ReceiveSocksResponse(Address & addr, WORD & port) = 0;
    int TransferHandle(PSocksSocket & destination);

    PString serverHost;
    WORD    serverPort;
    PString authenticationUsername;
    PString authenticationPassword;
    Address remoteAddress;
    WORD    remotePort;
    Address localAddress;
    WORD    localPort;

  private:
    virtual BOOL Connect(WORD localPort, const Address & addr);
};


class PSocks5Socket : public PSocksSocket
{
/* This class allows access to RFC1928 compliant SOCKS server.
 */
  PCLASSINFO(PSocks5Socket, PSocksSocket)

  public:
    PSocks5Socket(
      WORD port = 0
    );
    PSocks5Socket(
      const PString & host,
      WORD port = 0
    );

    virtual PObject * Clone() const;
    /* Create a copy of the class on the heap. The exact semantics of the
       descendent class determine what is required to make a duplicate of the
       instance. Not all classes can even <EM>do</EM> a clone operation.
       
       The main user of the clone function is the <A>PDictionary</A> class as
       it requires copies of the dictionary keys.

       The default behaviour is for this function to assert.

       <H2>Returns:</H2>
       pointer to new copy of the class instance.
     */


  protected:
    virtual BOOL ConnectServer();
    virtual BOOL SendSocksCommand(BYTE command, const char * hostname, Address addr);
    virtual BOOL ReceiveSocksResponse(Address & addr, WORD & port);
};


class PSocksUDPSocket : public PUDPSocket
{
/* This class allows access to RFC1928 compliant SOCKS server.
 */
  PCLASSINFO(PSocksUDPSocket, PUDPSocket)

  public:
    PSocksUDPSocket(
      WORD port = 0
    );
    PSocksUDPSocket(
      const PString & host,
      WORD port = 0
    );


  // Overrides from class PIPDatagramSocket.
    virtual BOOL ReadFrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      Address & addr, // Address from which the datagram was received.
      WORD & port     // Port from which the datagram was received.
    );
    /* Read a datagram from a remote computer.
       
       <H2>Returns:</H2>
       TRUE if any bytes were sucessfully read.
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
    PSocks5Socket socks;
    Address       serverAddress;
    WORD          serverPort;
};


#endif  // _SOCKS_H


// End of File ///////////////////////////////////////////////////////////////
