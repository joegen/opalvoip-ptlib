/*
 * pstun.h
 *
 * STUN client
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: pstun.h,v $
 * Revision 1.2  2003/02/04 05:05:55  craigs
 * Added new functions
 *
 * Revision 1.1  2003/02/04 03:31:04  robertj
 * Added STUN
 *
 */

#ifndef _PSTUN_H
#define _PSTUN_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>

class PSTUNUDPSocket : public PUDPSocket
{
  PCLASSINFO(PSTUNUDPSocket, PUDPSocket);
  public:
    PSTUNUDPSocket(int fd, const PIPSocket::Address & externalIP, WORD externalPort);

    virtual BOOL GetLocalAddress(
      Address & addr    /// Variable to receive hosts IP address
    );
    virtual BOOL GetLocalAddress(
      Address & addr,    /// Variable to receive peer hosts IP address
      WORD & port        /// Variable to receive peer hosts port number
    );

  protected:
    BOOL OpenSocket();
    PIPSocket::Address externalIP;
    WORD externalPort;
};

class PSTUNClient : public PObject
{
  PCLASSINFO(PSTUNClient, PObject);
  public:
    enum {
      DefaultPort = 3478
    };

    PSTUNClient(
      const PString & server,
      WORD portBase = 0,
      WORD portEnd = 0
    );

    BOOL SetServer(
      const PString & server
    );
    BOOL SetServer(
      PIPSocket::Address serverAddress,
      WORD serverPort = 0
    );

    enum NatTypes {
      UnknownNat,
      OpenNat,
      ConeNat,
      RestrictedNat,
      PortRestrictedNat,
      SymmetricNat,
      SymmetricFirewall,
      BlockedNat,
      NumNatTypes
    };

    NatTypes GetNatType();
    PString GetNatTypeName();

    BOOL CreateSocket(
      PUDPSocket * & socket
    );

    BOOL CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2
    );

  protected:
    NatTypes natType;
    BOOL     stunPossible;
    PIPSocket::Address serverAddress;
    WORD               serverPort;
    WORD portBase;
    WORD portEnd;
};



#endif // _PSTUN_H


// End of file ////////////////////////////////////////////////////////////////
