/*
 * pethsock.cxx
 *
 * Direct Ethernet socket I/O channel class.
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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptlib/sockets.h>

#if P_PCAP

#include <pcap/pcap.h>

#if _MSC_VER
  #pragma comment(lib, P_PCAP_LIBRARY1)
  #pragma comment(lib, P_PCAP_LIBRARY2)
#endif


#define M_PCAP reinterpret_cast<pcap_t *>(m_pcap)

PStringArray PEthSocket::EnumInterfaces(bool detailed) const
{
  PStringArray interfaces;

  pcap_if_t *alldevs;
  char errbuf[PCAP_ERRBUF_SIZE];
  if (pcap_findalldevs(&alldevs, errbuf) == -1) {
    PTRACE(1, "EthSock\tCould not enumerate interfaces, error: " << errbuf);
  }
  else {
    for (pcap_if_t * dev = alldevs; dev != NULL; dev = dev->next) {
      PStringStream strm;
      strm << dev->name;
      if (detailed)
        strm << '\t' << dev->description;
      interfaces += strm;
    }
    pcap_freealldevs(alldevs);
  }

  return interfaces;
}


PBoolean PEthSocket::Connect(const PString & newName)
{
  Close();

  channelName = newName.Left(newName.Find('\t'));

  char errbuf[PCAP_ERRBUF_SIZE];
  if ((m_pcap = pcap_open_live(channelName, 65536, m_filterMask, GetReadTimeout().GetInterval(), errbuf)) == NULL) {
    PTRACE(1, "EthSock\tCould not open interface \"" << channelName << "\", error: " << errbuf);
    return false;
  }

  os_handle = 1;
  return true;
}


PBoolean PEthSocket::Close()
{
  if (m_pcap != NULL) {
    pcap_close(M_PCAP);
    m_pcap = NULL;
  }

  os_handle = -1;
  return true;
}


PEthSocket::MediumTypes PEthSocket::GetMedium()
{
  if (IsOpen()) {
    switch (pcap_datalink(M_PCAP)) {
      case DLT_EN10MB :
        return Medium802_3;
      case DLT_PPP :
        return MediumWan;
      case DLT_LINUX_SLL :
        return MediumLinuxSLL;
    }
  }

  return MediumUnknown;
}


PBoolean PEthSocket::Read(void * data, PINDEX length)
{
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF, LastReadError);

  struct pcap_pkthdr *header;
  const u_char *pkt_data;
  int err = pcap_next_ex(M_PCAP, &header, &pkt_data);
  switch (err) {
    case 1 :
      memcpy(data, pkt_data, std::min(length, (PINDEX)header->caplen));
      lastReadCount = header->caplen;
      return true;

    case 0 :
      return SetErrorValues(Timeout, 0, LastReadError);

    default :
      return SetErrorValues(Miscellaneous, err, LastReadError);
  }
}


PBoolean PEthSocket::Write(const void * data, PINDEX length)
{
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF, LastWriteError);

  if (!ConvertOSError(pcap_sendpacket(M_PCAP, (u_char *)data, length), LastWriteError))
    return false;

  lastWriteCount = length;
  return true;
}


#else

PStringArray PEthSocket::EnumInterfaces(bool) const
{
  return PStringArray();
}


PBoolean PEthSocket::Connect(const PString & newName)
{
  PTRACE(1, "EthSock\tNo PCAP library compiled into system.");
  return false;
}


PBoolean PEthSocket::Close()
{
  os_handle = -1;
  return true;
}


PEthSocket::MediumTypes PEthSocket::GetMedium()
{
  return MediumUnknown;
}


PBoolean PEthSocket::Read(void *, PINDEX)
{
  return false;
}


PBoolean PEthSocket::Write(const void *, PINDEX)
{
  return false;
}

#endif


PEthSocket::PEthSocket()
  : m_filterMask(FilterPromiscuous)
  , m_pcap(NULL)
{
}


PEthSocket::~PEthSocket()
{
  Close();
}


bool PEthSocket::SetFilter(FilterMask mask)
{
  bool wasOpen = IsOpen();
  Close();
  m_filterMask = mask;
  return !wasOpen || Connect(channelName);
}


const char * PEthSocket::GetProtocolName() const
{
  return "eth";
}


PBoolean PEthSocket::OpenSocket()
{
  PAssertAlways(PUnimplementedFunction);
  return false;
}


PBoolean PEthSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways(PUnimplementedFunction);
  return PFalse;
}


PBoolean PEthSocket::ReadPacket(PBYTEArray & buffer,
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
      return PFalse;
  } while (lastReadCount < MinFrameSize);

  dest = frame->dst_addr;
  src = frame->src_addr;
  length = lastReadCount;
  frame->Parse(type, payload, length);

  return PTrue;
}


///////////////////////////////////////////////////////////////////////////////

PEthSocket::Address::Address()
{
  memset(b, 0xff, sizeof(b));
}


PEthSocket::Address::Address(const BYTE * addr)
{
  if (addr != NULL)
    memcpy(b, addr, sizeof(b));
  else
    memset(b, 0, sizeof(b));
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


bool PEthSocket::Address::operator==(const BYTE * eth) const
{
  if (eth != NULL)
    return memcmp(b, eth, sizeof(b)) == 0;
  else
    return ls.l == 0 && ls.s == 0;
}


bool PEthSocket::Address::operator!=(const BYTE * eth) const
{
  if (eth != NULL)
    return memcmp(b, eth, sizeof(b)) != 0;
  else
    return ls.l != 0 || ls.s != 0;
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


// End Of File ///////////////////////////////////////////////////////////////
