/*
 * $Id: pethsock.cxx,v 1.1 1998/08/31 12:59:55 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Implementation
 *
 * Copyright 1994 Equivalence
 *
 * $Log: pethsock.cxx,v $
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
  WORD len_or_type = ntohs(frame->snap.length);
  if (len_or_type > sizeof(Frame)) {
    type = len_or_type;
    length = lastReadCount - MinFrameSize;
    payload = frame->ether.payload;
  }
  else if (frame->snap.dsap == 0xaa && frame->snap.ssap == 0xaa) {
    type = ntohs(frame->snap.type);   // SNAP header
    payload = frame->snap.payload;
    length = len_or_type - 8;  // Subtract off the 802.2 header and SNAP data
  }
  else if (frame->snap.dsap == 0xff && frame->snap.ssap == 0xff) {
    type = TypeIPX;   // Special case for Novell netware's stuffed up 802.3
    payload = &frame->snap.dsap;
    length = len_or_type;  // Whole thing is IPX payload
  }
  else {
    if (frame->snap.dsap == 0xe0 && frame->snap.ssap == 0xe0)
      type = TypeIPX;   // Special case for Novell netware's 802.2
    else
      type = frame->snap.dsap;    // A pure 802.2 protocol id
    payload = frame->snap.oui;
    length = len_or_type - 3;     // Subtract off the 802.2 header
  }

  return TRUE;
}


// End Of File ///////////////////////////////////////////////////////////////
