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
 * Revision 1.3  2003/02/04 05:55:04  craigs
 * Added socket pair function
 *
 * Revision 1.2  2003/02/04 05:06:24  craigs
 * Added new functions
 *
 * Revision 1.1  2003/02/04 03:31:04  robertj
 * Added STUN
 *
 */

#ifdef __GNUC__
#pragma implementation "pstun.h"
#endif


#include <ptlib.h>
#include <ptclib/pstun.h>
#include <ptclib/random.h>

#include "stun.h"


///////////////////////////////////////////////////////////////////////

PSTUNClient::PSTUNClient(const PString & server, WORD _portBase, WORD _portEnd)
  : serverAddress(0),
    serverPort(3478),
    portBase(_portBase),
    portEnd(_portEnd)
{
  SetServer(server);
  natType      = UnknownNat;
  stunPossible = FALSE;
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
  stunServerAddr.addrHdr.port   = serverPort;
  stunServerAddr.addr.v4addr    = ntohl(serverAddress);

  StunNatType stype = stunType(stunServerAddr, FALSE, portBase);
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
  static const BOOL StunPossibleTable[] = {
    FALSE, // UnknownNat,
    TRUE,  // OpenNat,
    TRUE,  // ConeNat,
    TRUE,  // RestrictedNat,
    TRUE,  // PortRestrictedNat,
    FALSE, // SymmetricNat,
    FALSE, // SymmetricFirewall,
    FALSE, // BlockedNat
  };


  if (stype < PARRAYSIZE(TranslationTable)) {
    natType      = TranslationTable[stype];
    stunPossible = StunPossibleTable[stype];
    return natType;
  }

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

  if (natType == UnknownNat)
    GetNatType();
  return Names[natType];
}


BOOL PSTUNClient::CreateSocket(PUDPSocket * & socket)
{
  if (natType == UnknownNat) 
    GetNatType();

  if (!stunPossible)
    return FALSE;

  StunAddress vStunAddr;
  vStunAddr.addrHdr.family = PF_INET;
  vStunAddr.addrHdr.port   = serverPort; 
  vStunAddr.addr.v4addr    = ntohl(serverAddress);

  int port = 0;
  if (portBase != 0 && portEnd != 0)
    port = portBase + (PRandom::Number() % (portEnd - portBase));

  StunAddress vSockAddr;
  int fd = stunOpenSocket(vStunAddr, &vSockAddr, port);

  if (fd < 0)
    return FALSE;

  socket = new PSTUNUDPSocket(fd, 
                              PIPSocket::Address(htonl(vSockAddr.addr.v4addr)),
                              vSockAddr.addrHdr.port);

  return TRUE;
}


BOOL PSTUNClient::CreateSocketPair(PUDPSocket * & socket1,
                                   PUDPSocket * & socket2)
{
  if (natType == UnknownNat) 
    GetNatType();

  if (!stunPossible)
    return FALSE;

  StunAddress vStunAddr;
  vStunAddr.addrHdr.family = PF_INET;
  vStunAddr.addrHdr.port   = serverPort; 
  vStunAddr.addr.v4addr    = ntohl(serverAddress);

  int port = 0;
  if (portBase != 0 && portEnd != 0)
    port = portBase + (PRandom::Number() % (portEnd - portBase));

  StunAddress vSockAddr;
  int fd1, fd2;

  if (!stunOpenSocketPair(vStunAddr, &vSockAddr, &fd1, &fd2, port))
    return FALSE;

  socket1 = new PSTUNUDPSocket(fd1, 
                               PIPSocket::Address(htonl(vSockAddr.addr.v4addr)),
                               vSockAddr.addrHdr.port);
  socket2 = new PSTUNUDPSocket(fd2, 
                               PIPSocket::Address(htonl(vSockAddr.addr.v4addr)),
                               (WORD)(vSockAddr.addrHdr.port+1));
  return TRUE;
}

////////////////////////////////////////////////////////////////


PSTUNUDPSocket::PSTUNUDPSocket(int fd, 
                               const PIPSocket::Address & _externalIP, 
                               WORD _externalPort)
  : externalIP(_externalIP), externalPort(_externalPort)
{
  os_handle = fd;
}

BOOL PSTUNUDPSocket::OpenSocket()
{
  return TRUE;
}

BOOL PSTUNUDPSocket::GetLocalAddress(Address & addr)
{
  addr = externalIP;
  return TRUE;
}

BOOL PSTUNUDPSocket::GetLocalAddress(
      Address & addr,    /// Variable to receive peer hosts IP address
      WORD & port        /// Variable to receive peer hosts port number
    )
{
  addr = externalIP;
  port = externalPort;
  return TRUE;
}

// End of File ////////////////////////////////////////////////////////////////
