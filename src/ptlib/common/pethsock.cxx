/*
 * $Id: pethsock.cxx,v 1.2 1998/09/14 12:37:51 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: pethsock.cxx,v $
 * Revision 1.2  1998/09/14 12:37:51  robertj
 * Added function to parse type and payload address out of ethernet/802.2 packet.
 *
 * Revision 1.1  1998/08/31 12:59:55  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>

PEthSocket::Address::Address()
{
  memset(b, 0xff, sizeof(b));
}


PEthSocket::Address::Address(const BYTE * addr)
{
  memcpy(b, PAssertNULL(addr), sizeof(b));
}


PEthSocket::Address::Address(const Address & addr)
{
  ls.l = addr.ls.l;
  ls.s = addr.ls.s;
}


PEthSocket::Address::Address(const PString & str)
{
  operator=(str);
}


PEthSocket::Address & PEthSocket::Address::operator=(const Address & addr)
{
  ls.l = addr.ls.l;
  ls.s = addr.ls.s;
  return *this;
}


PEthSocket::Address & PEthSocket::Address::operator=(const PString & str)
{
  memset(b, 0, sizeof(b));

  int shift = 0;
  PINDEX byte = 5;
  PINDEX pos = str.GetLength();
  while (pos-- > 0) {
    int c = str[pos];
    if (c != '-') {
      if (isdigit(c))
        b[byte] |= (c - '0') << shift;
      else if (isxdigit(c))
        b[byte] |= (toupper(c) - 'A' + 10) << shift;
      else {
        memset(this, 0, sizeof(*this));
        return *this;
      }
      if (shift == 0)
        shift = 4;
      else {
        shift = 0;
        byte--;
      }
    }
  }

  return *this;
}


PEthSocket::Address::operator PString() const
{
  return psprintf("%02X-%02X-%02X-%02X-%02X-%02X", b[0], b[1], b[2], b[3], b[4], b[5]);
}


void PEthSocket::Frame::Parse(WORD & type, BYTE * & payload, PINDEX & length)
{
  WORD len_or_type = ntohs(snap.length);
  if (len_or_type > sizeof(*this)) {
    type = len_or_type;
    payload = ether.payload;
    // Subtract off the Ethernet II header
    length -= sizeof(dst_addr)+sizeof(src_addr)+sizeof(snap.length);
    return;
  }

  if (snap.dsap == 0xaa && snap.ssap == 0xaa) {
    type = ntohs(snap.type);   // SNAP header
    payload = snap.payload;
    // Subtract off the 802.2 header and SNAP data
    length = len_or_type - (sizeof(snap)-sizeof(snap.payload));
    return;
  }
  
  if (snap.dsap == 0xff && snap.ssap == 0xff) {
    type = TypeIPX;   // Special case for Novell netware's stuffed up 802.3
    payload = &snap.dsap;
    length = len_or_type;  // Whole thing is IPX payload
    return;
  }

  if (snap.dsap == 0xe0 && snap.ssap == 0xe0)
    type = TypeIPX;   // Special case for Novell netware's 802.2
  else
    type = snap.dsap;    // A pure 802.2 protocol id

  payload = snap.oui;
  // Subtract off the 802.2 header
  length = len_or_type - (sizeof(snap.dsap)+sizeof(snap.ssap)+sizeof(snap.ctrl));
}


const char * PEthSocket::GetProtocolName() const
{
  return "eth";
}


BOOL PEthSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways(PUnimplementedFunction);
  return FALSE;
}


BOOL PEthSocket::GetIpAddress(PIPSocket::Address & addr)
{
  PIPSocket::Address net_mask;
  return EnumIpAddress(0, addr, net_mask);
}


BOOL PEthSocket::GetIpAddress(PIPSocket::Address & addr, PIPSocket::Address & net_mask)
{
  return EnumIpAddress(0, addr, net_mask);
}


BOOL PEthSocket::ReadPacket(PBYTEArray & buffer,
                            Address & dest,
                            Address & src,
                            WORD & type,
                            PINDEX & length,
                            BYTE * & payload)
{
  Frame * frame = (Frame *)buffer.GetPointer(sizeof(Frame));
  const PINDEX MinFrameSize = sizeof(frame->dst_addr)+sizeof(frame->src_addr)+sizeof(frame->snap.length);

  do {
    if (!Read(frame, sizeof(*frame)))
      return FALSE;
  } while (lastReadCount < MinFrameSize);

  dest = frame->dst_addr;
  src = frame->src_addr;
  length = lastReadCount;
  frame->Parse(type, payload, length);

  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
