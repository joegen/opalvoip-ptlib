/*
 * udpsock.h
 *
 * User Datagram Protocol socket I/O channel class.
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
 */

#ifndef PTLIB_UDPSOCKET_H
#define PTLIB_UDPSOCKET_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/ipdsock.h>
 
/**
   A socket channel that uses the UDP transport on the Internet Protocol.
 */
class PUDPSocket : public PIPDatagramSocket
{
  PCLASSINFO(PUDPSocket, PIPDatagramSocket);

  public:
  /**@name Construction */
  //@{
    /** Create a UDP socket. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */
    PUDPSocket(
      WORD port = 0,             ///< Port number to use for the connection.
      int iAddressFamily = AF_INET ///< Family
    );
    PUDPSocket(
      const PString & service,   ///< Service name to use for the connection.
      int iAddressFamily = AF_INET ///< Family
    );
    PUDPSocket(
      const PString & address,  ///< Address of remote machine to connect to.
      WORD port                 ///< Port number to use for the connection.
    );
    PUDPSocket(
      const PString & address,  ///< Address of remote machine to connect to.
      const PString & service   ///< Service name to use for the connection.
    );
  //@}

  /**@name Overrides from class PSocket */
  //@{
    /** Override of PChannel functions to allow connectionless reads
     */
    PBoolean Read(
      void * buf,   ///< Pointer to a block of memory to read.
      PINDEX len    ///< Number of bytes to read.
    );

    /** Override of PChannel functions to allow connectionless writes
     */
    PBoolean Write(
      const void * buf, ///< Pointer to a block of memory to write.
      PINDEX len        ///< Number of bytes to write.
    );

    /** Override of PSocket functions to allow connectionless writes
     */
    PBoolean Connect(
      const PString & address   ///< Address of remote machine to connect to.
    );
  //@}

  /**@name New functions for class */
  //@{
    /** Set the address to use for connectionless Write() or Windows QoS
     */
    void SetSendAddress(
      const Address & address,    ///< IP address to send packets.
      WORD port                   ///< Port to send packets.
    );
    void SetSendAddress(
      const PIPSocketAddressAndPort & addressAndPort
    );

    /** Get the address to use for connectionless Write().
     */
    void GetSendAddress(
      Address & address,    ///< IP address to send packets.
      WORD & port           ///< Port to send packets.
    ) const;
    void GetSendAddress(
      PIPSocketAddressAndPort & addressAndPort
    ) const;
    PString GetSendAddress() const;

    /** Get the address of the sender in the last connectionless Read().
        Note that thsi only applies to the Read() and not the ReadFrom()
        function.
     */
    void GetLastReceiveAddress(
      Address & address,    ///< IP address to send packets.
      WORD & port           ///< Port to send packets.
    ) const;
    void GetLastReceiveAddress(
      PIPSocketAddressAndPort & addressAndPort    ///< IP address and port to send packets.
    ) const;
    PString GetLastReceiveAddress() const;
  //@}

    // Normally, one would expect these to be protected, but they are just so darn
    // useful that it's just easier if they are public
    virtual bool InternalReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort);
    virtual void InternalSetSendAddress(const PIPSocketAddressAndPort & addr);
    virtual void InternalGetSendAddress(PIPSocketAddressAndPort & addr) const;
    virtual void InternalSetLastReceiveAddress(const PIPSocketAddressAndPort & addr);
    virtual void InternalGetLastReceiveAddress(PIPSocketAddressAndPort & addr) const;

  protected:
    // Open an IPv4 socket (for backward compatibility)
    virtual PBoolean OpenSocket();

    // Open an IPv4 or IPv6 socket
    virtual PBoolean OpenSocket(
      int ipAdressFamily
    );

    virtual bool InternalListen(const Address & bind, unsigned queueSize, WORD port, Reusability reuse);

    virtual const char * GetProtocolName() const;

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/udpsock.h"
#else
#include "unix/ptlib/udpsock.h"
#endif

    private:
      AddressAndPort m_sendAddressAndPort;
      AddressAndPort m_lastReceiveAddressAndPort;
};


#endif // PTLIB_UDPSOCKET_H


// End Of File ///////////////////////////////////////////////////////////////
