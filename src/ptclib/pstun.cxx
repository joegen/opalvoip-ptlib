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
 * Revision 1.7  2003/10/03 00:31:39  dereksmithies
 * Fix previouv fix - so IPV6 address is correctly copied.
 *
 * Revision 1.6  2003/10/03 00:06:58  dereksmithies
 * Fix typo on IPV6 test. A big thanks to Andrey S Pankov
 *
 * Revision 1.5  2003/02/05 06:26:49  robertj
 * More work in making the STUN usable for Symmetric NAT systems.
 *
 * Revision 1.4  2003/02/04 07:02:17  robertj
 * Added ip/port version of constructor.
 * Removed creating sockets for Open type.
 *
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

#include "stun.h"


///////////////////////////////////////////////////////////////////////

PSTUNClient::PSTUNClient(const PString & server,
                         WORD portBase, WORD portMax,
                         WORD portPairBase, WORD portPairMax)
  : serverAddress(0)
{
  serverPort = DefaultPort;
  basePort = 0;
  maxPort = 0;
  basePortPair = 0;
  maxPortPair = 0;
  natType = UnknownNat;

  SetServer(server);
  SetPortRanges(portBase, portMax, portPairBase, portPairMax);
}


PSTUNClient::PSTUNClient(const PIPSocket::Address & address, WORD port,
                         WORD portBase, WORD portMax,
                         WORD portPairBase, WORD portPairMax)
  : serverAddress(address),
    serverPort(port)
{
  basePort = 0;
  maxPort = 0;
  basePortPair = 0;
  maxPortPair = 0;
  natType = UnknownNat;

  SetPortRanges(portBase, portMax, portPairBase, portPairMax);
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


BOOL PSTUNClient::SetServer(const PIPSocket::Address & address, WORD port)
{
  serverAddress = address;
  serverPort = port;
  return serverAddress.IsValid() && serverPort != 0;
}


void PSTUNClient::SetPortRanges(WORD portBase, WORD portMax,
                                WORD portPairBase, WORD portPairMax)
{
  mutex.Wait();

  basePort = portBase;
  if (portBase == 0)
    maxPort = 0;
  else if (portMax == 0)
    maxPort = (WORD)(basePort+99);
  else if (portMax < portBase)
    maxPort = portBase;
  else
    maxPort = portMax;

  currentPort = basePort;

  basePortPair = (WORD)((portPairBase+1)&0xfffe);
  if (portPairBase == 0) {
    basePortPair = (WORD)((basePort+1)&0xfffe);
    maxPortPair = maxPort;
  }
  else if (portPairMax == 0)
    maxPortPair = (WORD)(basePortPair+99);
  else if (portPairMax < portPairBase)
    maxPortPair = portPairBase;
  else
    maxPortPair = portPairMax;

  currentPortPair = basePortPair;

  mutex.Signal();
}


static void SetStunAddress(const PIPSocket::Address & ip, WORD port, StunAddress & addr)
{
  switch (ip.GetVersion()) {
    case 4 :
      addr.addrHdr.family = AF_INET;
      addr.addr.v4addr    = ntohl((DWORD)ip);
      break;
#if P_HAS_IPV6
    case 6 :
      addr.addrHdr.family = AF_INET6;
       for ( PINDEX i = 0; i < ip.GetSize(); i++ )    
	 addr.addr.v6addr.octet[i] = ip[i];
      break;
#endif
  }

  addr.addrHdr.port = port;
}


PSTUNClient::NatTypes PSTUNClient::GetNatType(BOOL force)
{
  if (force || natType == UnknownNat) {
    StunAddress stunServerAddr;
    SetStunAddress(serverAddress, serverPort, stunServerAddr);

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
    natType = stype < PARRAYSIZE(TranslationTable) ? TranslationTable[stype] : UnknownNat;
  }

  return natType;
}


PString PSTUNClient::GetNatTypeName(BOOL force)
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

  return Names[GetNatType(force)];
}


BOOL PSTUNClient::CreateSocket(PUDPSocket * & socket)
{
  switch (GetNatType(FALSE)) {
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (basePort == 0 || basePort > maxPort)
        return FALSE;
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      return FALSE;
  }

  StunAddress stunServerAddr;
  SetStunAddress(serverAddress, serverPort, stunServerAddr);

  StunAddress vSockAddr;

  int fd;

  if (natType != SymmetricNat)
    fd = stunOpenSocket(stunServerAddr, &vSockAddr);
  else {
    mutex.Wait();

    WORD startPort = currentPort;

    do {
      currentPort++;
      if (currentPort > maxPort)
        currentPort = basePort;

      fd = stunOpenSocket(stunServerAddr, &vSockAddr, currentPort);

    } while (fd < 0 && currentPort != startPort);

    vSockAddr.addrHdr.port = currentPort;

    mutex.Signal();
  }

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
  switch (GetNatType(FALSE)) {
    case ConeNat :
    case RestrictedNat :
    case PortRestrictedNat :
      break;

    case SymmetricNat :
      if (basePort == 0 || basePort > maxPort)
        return FALSE;
// Don't do this at present as stunOpenSocketPair() seems to get all sorts of
// knickers in all sorts of twists.
return FALSE;
      break;

    default : // UnknownNet, SymmetricFirewall, BlockedNat
      return FALSE;
  }

  StunAddress stunServerAddr;
  SetStunAddress(serverAddress, serverPort, stunServerAddr);

  StunAddress vSockAddr;

  int fd1, fd2;
  BOOL ok;

  if (natType != SymmetricNat)
    ok = stunOpenSocketPair(stunServerAddr, &vSockAddr, &fd1, &fd2, 0);
  else {
    mutex.Wait();

    WORD startPort = currentPortPair;

    do {
      currentPortPair += 2;
      if (currentPortPair > maxPortPair)
        currentPortPair = basePortPair;

      ok = stunOpenSocketPair(stunServerAddr, &vSockAddr, &fd1, &fd2, currentPortPair);

    } while (!ok && currentPortPair != startPort);

    vSockAddr.addrHdr.port = currentPortPair;

    mutex.Signal();
  }

  if (!ok)
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
  : PUDPSocket(_externalPort),
    externalIP(_externalIP)
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
  return addr.IsValid();
}


BOOL PSTUNUDPSocket::GetLocalAddress(Address & addr, WORD & port)
{
  addr = externalIP;
  port = GetPort();
  return addr.IsValid() && port != 0;
}


// End of File ////////////////////////////////////////////////////////////////
