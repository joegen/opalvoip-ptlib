/*
 * pstun.cxx
 *
 * STUN Client
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
 * $Log: pstun.cxx,v $
 * Revision 1.1  2003/02/04 03:31:04  robertj
 * Added STUN
 *
 */

#ifdef __GNUC__
#pragma implementation "pstun.h"
#endif


#include <ptlib.h>
#include <ptclib/pstun.h>

#include "stun.h"


///////////////////////////////////////////////////////////////////////

PSTUNClient::PSTUNClient(const PString & server)
  : serverAddress(0),
    serverPort(3487)
{
  SetServer(server);
}


BOOL PSTUNClient::SetServer(const PString & server)
{
  PINDEX colon = server.Find(':');
  if (colon == P_MAX_INDEX) {
    if (!PIPSocket::GetHostAddress(server, serverAddress))
      return FALSE;
  }
  else {
    if (!PIPSocket::GetHostAddress(server.Left(colon), serverAddress))
      return FALSE;
    serverPort = (WORD)server.Mid(colon+1).AsUnsigned();
  }

  return serverAddress.IsValid() && serverPort != 0;
}


BOOL PSTUNClient::SetServer(PIPSocket::Address address, WORD port)
{
  serverAddress = address;
  serverPort = port;
  return serverAddress.IsValid() && serverPort != 0;
}


PSTUNClient::NatTypes PSTUNClient::GetNatType()
{
  StunAddress stunServerAddr;
  stunServerAddr.addrHdr.family = PF_INET;
  stunServerAddr.addrHdr.port = serverPort;
  stunServerAddr.addr.v4addr = ntohl(serverAddress);
  StunNatType stype = stunType(stunServerAddr, FALSE);
  static const NatTypes TranslationTable[] = {
    UnknownNat,
    OpenNat,
    ConeNat,
    RestrictedNat,
    PortRestrictedNat,
    SymmetricNat,
    SymmetricFirewall,
    BlockedNat
  };

  if (stype < PARRAYSIZE(TranslationTable))
    return TranslationTable[stype];

  return UnknownNat;
}


PString PSTUNClient::GetNatTypeName()
{
  static const char * const Names[NumNatTypes] = {
    "Unknown NAT",
    "Open NAT",
    "Cone NAT",
    "Restricted NAT",
    "Port Restricted NAT",
    "Symmetric NAT",
    "Symmetric Firewall",
    "Blocked"
  };

  return Names[GetNatType()];
}


BOOL PSTUNClient::CreateSocket(PIPSocket::Address remoteAddress,
                               WORD remotePort,
                               PUDPSocket * & socket)
{
  return FALSE;
}


BOOL PSTUNClient::CreateSocketPair(PIPSocket::Address remoteAddress,
                                   WORD remotePort,
                                   PUDPSocket * & socket1,
                                   PUDPSocket * & socket2)
{
 return FALSE;
}


// End of File ////////////////////////////////////////////////////////////////
