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


class PSTUNClient : public PObject
{
  PCLASSINFO(PSTUNClient, PObject);
  public:
    enum {
      DefaultPort = 3278
    };

    PSTUNClient(
      const PString & server
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
      PIPSocket::Address remoteAddress,
      WORD remotePort,
      PUDPSocket * & socket
    );

    BOOL CreateSocketPair(
      PIPSocket::Address remoteAddress,
      WORD remotePort,
      PUDPSocket * & socket1,
      PUDPSocket * & socket2
    );

  protected:
    PIPSocket::Address serverAddress;
    WORD               serverPort;
};



#endif // _PSTUN_H


// End of file ////////////////////////////////////////////////////////////////
