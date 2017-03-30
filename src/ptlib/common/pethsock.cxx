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
 */

#include <ptlib.h>
#include <ptlib/sockets.h>


#define PTraceModule() "EthSock"


#pragma pack(1)

struct PEthFrameHeader {
  PEthSocket::Address dst_addr;
  PEthSocket::Address src_addr;
  union {
    struct {
      uint16_t type;
      uint8_t  payload[1500];
    } ether;
    struct {
      uint16_t length;
      uint8_t  dsap;
      uint8_t  ssap;
      uint8_t  ctrl;
      uint8_t  oui[3];
      uint16_t type;
      uint8_t  payload[1492];
    } snap;
  };
};


#pragma pack()


#if P_PCAP

#if P_PCAP_PCAP_H
  #include <pcap/pcap.h>
#else
  #include <pcap.h>
#endif


#if _MSC_VER
  #pragma comment(lib, P_PCAP_LIBRARY1)
  #pragma comment(lib, P_PCAP_LIBRARY2)
#endif


struct PEthSocket::InternalData {
  pcap_t    * m_pcap;
  bpf_program m_program;
};


PStringArray PEthSocket::EnumInterfaces(bool detailed)
{
  PStringArray interfaces;

  pcap_if_t *alldevs;
  char errbuf[PCAP_ERRBUF_SIZE];
  if (pcap_findalldevs(&alldevs, errbuf) == -1) {
    PTRACE(1, &interfaces, "Could not enumerate interfaces, error: " << errbuf);
  }
  else {
    for (pcap_if_t * dev = alldevs; dev != NULL; dev = dev->next) {
      PStringStream strm;
      strm << dev->name;
      if (detailed) {
        strm << '\t' << dev->description;
#if _WIN32
        static const char DevPrefix[] = "\\Device\\NPF_";
        if (strncmp(dev->name, DevPrefix, sizeof(DevPrefix)-1) == 0) {
          PConfig registry("HKEY_LOCAL_MACH64\\"
                           "SYSTEM\\"
                           "CurrentControlSet\\"
                           "Control\\"
                           "Network\\"
                           "{4D36E972-E325-11CE-BFC1-08002BE10318}\\" +
                           PConstString(dev->name+sizeof(DevPrefix)-1) + "\\"
                           "Connection");
          PString name = registry.GetString("Name");
          if (!name.IsEmpty())
            strm << '\t' << name;
        }
#endif
      }
      interfaces += strm;
    }
    pcap_freealldevs(alldevs);
    PTRACE(4, &interfaces, "Enumerated interfaces:\n" << setfill('\n') << interfaces);
  }

  return interfaces;
}


PBoolean PEthSocket::Connect(const PString & newName)
{
  Close();

  m_channelName = newName.Left(newName.Find('\t'));

  char errbuf[PCAP_ERRBUF_SIZE];
  if ((m_internal->m_pcap = pcap_open_live(m_channelName, m_snapLength, m_promiscuous, GetReadTimeout().GetInterval(), errbuf)) == NULL) {
    PTRACE(1, "Could not open interface \"" << m_channelName << "\", error: " << errbuf);
    return false;
  }

  SetFilter(m_filter);
  os_handle = 1;
  return true;
}


PBoolean PEthSocket::Close()
{
  os_handle = -1;

  pcap_t * pcap = m_internal->m_pcap;
  m_internal->m_pcap = NULL;

  if (pcap != NULL)
    pcap_close(pcap);

  return true;
}


PEthSocket::MediumType PEthSocket::GetMedium()
{
  if (IsOpen()) {
    switch (pcap_datalink(m_internal->m_pcap)) {
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


bool PEthSocket::SetFilter(const PString & filter)
{
  pcap_freecode(&m_internal->m_program);

  char * filterStr = (char *)(const char *)filter;

  if (m_internal->m_pcap == NULL)
    return pcap_compile_nopcap(m_snapLength, DLT_EN10MB, &m_internal->m_program, filterStr, true, 0) >= 0;

  if (pcap_compile(m_internal->m_pcap, &m_internal->m_program, filterStr, true, 0) < 0)
    return false;

  if (pcap_setfilter(m_internal->m_pcap, &m_internal->m_program) < 0) {
    PTRACE(1, "Could not use filter \"" << filter << "\", error: " << pcap_geterr(m_internal->m_pcap));
    return false;
  }

  m_filter = filter;
  return true;
}


PBoolean PEthSocket::Read(void * data, PINDEX length)
{
  if (CheckNotOpen())
    return false;

  struct pcap_pkthdr *header;
  const u_char *pkt_data;
  int err = pcap_next_ex(m_internal->m_pcap, &header, &pkt_data);
  switch (err) {
    case 1 :
      memcpy(data, pkt_data, std::min(length, (PINDEX)header->caplen));
      SetLastReadCount(header->caplen);
      m_lastPacketTime = PTime(header->ts.tv_sec, header->ts.tv_usec);
      return true;

    case 0 :
      return SetErrorValues(Timeout, 0, LastReadError);

    default :
      if (m_internal->m_pcap == NULL)
        return SetErrorValues(NotOpen, err, LastReadError);

      PTRACE(2, "Read error: " << pcap_geterr(m_internal->m_pcap));
      return SetErrorValues(Miscellaneous, err, LastReadError);
  }
}


PBoolean PEthSocket::Write(const void * data, PINDEX length)
{
  if (!IsOpen())
    return false;

  if (!ConvertOSError(pcap_sendpacket(m_internal->m_pcap, (u_char *)data, length), LastWriteError)) {
    PTRACE(2, "Write error: " << pcap_geterr(m_internal->m_pcap));
    return false;
  }

  SetLastWriteCount(length);
  return true;
}


#else

struct PEthSocket::InternalData
{
  int m_dummy;
};


PStringArray PEthSocket::EnumInterfaces(bool)
{
  return PStringArray();
}


PBoolean PEthSocket::Connect(const PString & /*newName*/)
{
  return false;
}


PBoolean PEthSocket::Close()
{
  os_handle = -1;
  return true;
}


PEthSocket::MediumType PEthSocket::GetMedium()
{
  return MediumUnknown;
}


bool PEthSocket::SetFilter(const PString &)
{
  return false;
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


PEthSocket::PEthSocket(bool promiscuous, unsigned snapLength)
  : m_promiscuous(promiscuous)
  , m_snapLength(snapLength)
  , m_internal(new InternalData)
  , m_lastPacketTime(0)
{
  memset(m_internal, 0, sizeof(InternalData));
}


PEthSocket::~PEthSocket()
{
  Close();
  delete m_internal;
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
  return false;
}


///////////////////////////////////////////////////////////////////////////////

PEthSocket::Frame::Frame(PINDEX maxSize)
  : m_rawData(maxSize)
  , m_rawSize(0)
  , m_fragmentated(false)
  , m_fragmentProto(0)
  , m_fragmentProcessed(false)
{
}


PEthSocket::Frame::Frame(const Frame & frame)
  : m_rawData(frame.m_rawData, frame.m_rawSize) // Make copy, not reference
  , m_rawSize(frame.m_rawSize)
  , m_fragmentated(false)
  , m_fragmentProto(0)
  , m_fragmentProcessed(false)
  , m_timestamp(frame.m_timestamp)
{
  m_rawData.SetMinSize(sizeof(PEthFrameHeader));
}


void PEthSocket::Frame::PreRead()
{
  if (m_fragmentated) {
    m_fragments.SetSize(0);
    m_fragmentated = false;
  }
  m_fragmentProcessed = false;
}


bool PEthSocket::Frame::Write(PChannel & channel) const
{
  return channel.Write(m_rawData, m_rawSize);
}


bool PEthSocket::Frame::Read(PChannel & channel, PINDEX packetSize)
{
  PreRead();

  PINDEX size = std::min(packetSize, m_rawData.GetSize());
  do {
    if (!channel.Read(m_rawData.GetPointer(), size))
      return false;
    m_rawSize = channel.GetLastReadCount();
  } while ((size_t)m_rawSize < sizeof(Address)+sizeof(Address)+sizeof(2));

  const PEthSocket * ethChannel = dynamic_cast<PEthSocket *>(&channel);
  if (ethChannel != NULL && ethChannel->GetLastPacketTime().IsValid())
    m_timestamp = ethChannel->GetLastPacketTime();
  else
    m_timestamp.SetCurrentTime();

  return true;
}


int PEthSocket::Frame::GetDataLink(PBYTEArray & payload)
{
  Address src, dst;
  return GetDataLink(payload, src, dst);
}


int PEthSocket::Frame::GetDataLink(PBYTEArray & payload, Address & src, Address & dst)
{
  const PEthFrameHeader & header = m_rawData.GetAs<PEthFrameHeader>();

  if ((size_t)m_rawSize < sizeof(header.dst_addr)+sizeof(header.src_addr)+sizeof(header.snap.length)) {
    PTRACE(2, "Frame severely truncated, size=" << m_rawSize);
    return -1;
  }

  src = header.src_addr;
  dst = header.dst_addr;

  PINDEX len_or_type = ntohs(header.snap.length);

  // Ethernet II header
  if (len_or_type > 1500) {
    // Subtract off the Ethernet II header
    payload.Attach(header.ether.payload, m_rawSize - (sizeof(header.dst_addr)+sizeof(header.src_addr)+sizeof(header.ether.type)));
    return len_or_type;
  }

  // SNAP header
  if (header.snap.dsap == 0xaa && header.snap.ssap == 0xaa) {
    if (len_or_type < (PINDEX)(sizeof(header.snap)-sizeof(header.snap.payload))) {
      PTRACE(2, "Frame (SNAP) invalid, size=" << m_rawSize);
      return -1;
    }

    // Subtract off the 802.2 header and SNAP data
    len_or_type -= sizeof(header.snap)-sizeof(header.snap.payload);
    if (m_rawSize < len_or_type + (header.snap.payload - &m_rawData[0])) {
      PTRACE(2, "Frame (SNAP) truncated, size=" << m_rawSize);
      return -1;
    }

    payload.Attach(header.snap.payload, len_or_type);
    return ntohs(header.snap.type);
  }

  // Special case for Novell netware's stuffed up 802.3
  if (header.snap.dsap == 0xff && header.snap.ssap == 0xff) {
    if (m_rawSize < len_or_type + (&header.snap.dsap - &m_rawData[0])) {
      PTRACE(2, "Frame (802.3) truncated, size=" << m_rawSize);
      return -1;
    }
    payload.Attach(&header.snap.dsap, len_or_type); // Whole thing is IPX payload
    return 0x8137;
  }

  if (len_or_type < (PINDEX)(sizeof(header.snap.dsap)+sizeof(header.snap.ssap)+sizeof(header.snap.ctrl))) {
    PTRACE(2, "Frame (802.2) invalid, size=" << m_rawSize);
    return -1;
  }

  // Subtract off the 802.2 header
  len_or_type -= sizeof(header.snap.dsap)+sizeof(header.snap.ssap)+sizeof(header.snap.ctrl);
  if (m_rawSize < len_or_type + (header.snap.oui - &m_rawData[0])) {
    PTRACE(2, "Frame (802.2) truncated, size=" << m_rawSize);
    return -1;
  }

  payload.Attach(header.snap.oui, len_or_type);

  if (header.snap.dsap == 0xe0 && header.snap.ssap == 0xe0)
    return 0x8137;   // Special case for Novell netware's 802.2

  return header.snap.dsap;    // A pure 802.2 protocol id
}


BYTE * PEthSocket::Frame::CreateDataLink(const Address & src, const Address & dst, unsigned proto, PINDEX length)
{
  m_rawSize = length + 14;
  PEthFrameHeader & header = *(PEthFrameHeader *)m_rawData.GetPointer(sizeof(PEthFrameHeader));
  header.src_addr = src;
  header.dst_addr = dst;
  header.ether.type = htons((u_short)proto);
  return header.ether.payload;
}


int PEthSocket::Frame::GetIP(PBYTEArray & payload)
{
  PIPSocket::Address src, dst;
  return GetIP(payload, src, dst);
}


int PEthSocket::Frame::GetIP(PBYTEArray & payload, PIPSocket::Address & src, PIPSocket::Address & dst)
{
  // Already processed this frame as an IP fragment
  if (m_fragmentProcessed) {
    if (m_fragmentated) {
      payload.Attach(m_fragments, m_fragments.GetSize());
      return m_fragmentProto; // Next protocol layer
    }

    // Haven't got it all yet
    return -1;
  }

  PBYTEArray ip;
  if (GetDataLink(ip) != 0x800) // IPv4
    return -1;

  PINDEX totalLength = (ip[2]<<8)|ip[3]; // Total length of packet
  if (totalLength == 0)
    totalLength = ip.GetSize(); // presume to be part of TCP segmentation offload (TSO), whatever THAT is
  else if (totalLength > ip.GetSize()) {
    PTRACE(2, "Truncated IP packet, expected " << totalLength << ", got " << ip.GetSize());
    return -1;
  }

  PINDEX headerLength = (ip[0]&0xf)*4; // low 4 bits in DWORDS, is this in bytes
  if (totalLength < headerLength) {
    PTRACE(2, "Malformed IP header, length " << totalLength << " smaller than header " << headerLength);
    return -1;
  }

  payload.Attach(&ip[headerLength], totalLength-headerLength);

  src = PIPSocket::Address(4, ip+12);
  dst = PIPSocket::Address(4, ip+16);

  // Check for fragmentation
  bool isFragment = (ip[6] & 0x20) != 0;
  PINDEX fragmentOffset = (((ip[6]&0x1f)<<8)+ip[7])*8;
  PINDEX fragmentsSize = m_fragments.GetSize();

  if (fragmentsSize > 0) {
    /* Have a fragment re-assembly in progress, check if same IP pair. We are
       ignoring fragments on other pairs, eventually it should be a std::map
       of fragments for all IP pairs. But that is too hard for now.
    */
    if (m_fragmentSrcIP != src || m_fragmentDstIP != dst)
      return ip[9]; // Next protocol layer

    if (fragmentsSize > fragmentOffset) {
      PTRACE(5, "Repeated IP fragment at " << fragmentOffset << " on " << src << " -> " << dst);
      return -1;
    }

    if (fragmentsSize < fragmentOffset) {
      PTRACE(2, "Missing IP fragment, expected " << fragmentsSize << ", got " << fragmentOffset << " on " << src << " -> " << dst);
      m_fragments.SetSize(0);
      return -1;
    }
  }
  else {
    if (!isFragment)
      return ip[9]; // Next protocol layer

    // New fragmented IP start
    m_fragmentProto = ip[9]; // Next protocol layer
    m_fragmentSrcIP = src;
    m_fragmentDstIP = dst;
  }

  m_fragments.Concatenate(payload);
  m_fragmentProcessed = true;

  if (isFragment)
    return -1; // Haven't got it all yet

  payload.Attach(m_fragments, m_fragments.GetSize());
  m_fragmentated = true;

  return m_fragmentProto; // Next protocol layer
}


BYTE * PEthSocket::Frame::CreateIP(const PIPSocket::Address & src, const PIPSocket::Address & dst, unsigned proto, PINDEX length)
{
  length += 20;

  PBYTEArray dummy;
  Address srcMac, dstMac;
  if ((size_t)m_rawSize > sizeof(PEthSocket::Address)*2)
    GetDataLink(dummy, srcMac, dstMac);
  BYTE * ip = CreateDataLink(srcMac, dstMac, 0x800, length);
  memset(ip, 0, 20);
  ip[0] = 0x45;
  *(PUInt16b*)(ip+2) = (uint16_t)length;
  ip[9] = (uint8_t)proto;
  *(in_addr *)(ip+12) = src;
  *(in_addr *)(ip+16) = dst;
  return ip+20;
}


bool PEthSocket::Frame::GetUDP(PBYTEArray & payload, WORD & srcPort, WORD & dstPort)
{
  PIPSocketAddressAndPort src, dst;
  if (!GetUDP(payload, src, dst))
    return false;

  srcPort = src.GetPort();
  dstPort = dst.GetPort();
  return true;
}


bool PEthSocket::Frame::GetUDP(PBYTEArray & payload, PIPSocketAddressAndPort & src, PIPSocketAddressAndPort & dst)
{
  PBYTEArray udp;
  PIPSocket::Address srcIP, dstIP;
  if (GetIP(udp, srcIP, dstIP) != 0x11)
    return false;

  if (udp.GetSize() < 8) {
    PTRACE(2, "UDP truncated, size=" << udp.GetSize());
    return false;
  }

  src.SetAddress(srcIP);
  src.SetPort(udp.GetAs<PUInt16b>(0));
  dst.SetAddress(dstIP);
  dst.SetPort(udp.GetAs<PUInt16b>(2));

  payload.Attach(&udp[8], udp.GetSize() - 8);
  return true;
}


BYTE * PEthSocket::Frame::CreateUDP(const PIPSocketAddressAndPort & src, const PIPSocketAddressAndPort & dst, PINDEX length)
{
  length += 8;
  BYTE * udp = CreateIP(src.GetAddress(), dst.GetAddress(), 0x11, length);
  *(PUInt16b*)(udp+0) = src.GetPort();
  *(PUInt16b*)(udp+2) = dst.GetPort();
  *(PUInt16b*)(udp+4) = (uint16_t)length;
  *(uint16_t*)(udp+6) = 0;
  return udp+8;
}


bool PEthSocket::Frame::GetTCP(PBYTEArray & payload, WORD & srcPort, WORD & dstPort)
{
  PIPSocketAddressAndPort src, dst;
  if (!GetTCP(payload, src, dst))
    return false;

  srcPort = src.GetPort();
  dstPort = dst.GetPort();
  return true;
}


bool PEthSocket::Frame::GetTCP(PBYTEArray & payload, PIPSocketAddressAndPort & src, PIPSocketAddressAndPort & dst)
{
  PBYTEArray tcp;
  PIPSocket::Address srcIP, dstIP;
  if (GetIP(tcp, srcIP, dstIP) != 6)
    return false;

  PINDEX headerSize;
  if (tcp.GetSize() < 20 || tcp.GetSize() < (headerSize = (tcp[12]&0xf0)>>2)) {
    PTRACE(2, "TCP truncated, size=" << tcp.GetSize());
    return false;
  }

  src.SetAddress(srcIP);
  src.SetPort(tcp.GetAs<PUInt16b>(0));
  dst.SetAddress(dstIP);
  dst.SetPort(tcp.GetAs<PUInt16b>(2));

  payload.Attach(&tcp[headerSize], tcp.GetSize() - headerSize);
  return true;
}


BYTE * PEthSocket::Frame::CreateTCP(const PIPSocketAddressAndPort & src, const PIPSocketAddressAndPort & dst, PINDEX length)
{
  length += 20;

  BYTE * tcp = CreateIP(src.GetAddress(), dst.GetAddress(), 6, length);
  memset(tcp, 0, 20);
  tcp[12] = 0x50;
  *(PUInt16b*)(tcp+0) = src.GetPort();
  *(PUInt16b*)(tcp+2) = dst.GetPort();
  return tcp+20;
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


///////////////////////////////////////////////////////////////////////////////

PEthSocketThread::PEthSocketThread(const FrameNotifier & notifier)
  : m_notifier(notifier)
  , m_thread(NULL)
  , m_socket(NULL)
  , m_running(false)
{
}


bool PEthSocketThread::Start(const PString & device, const PString & filter, PThread::Priority priority)
{
  Stop();

  m_socket = CreateEthSocket();

  /* Only reliable way to exit the thread loop is to have a timeout and wait
     for the read to exit, then the thread can exit. Other methods such as
     closing the channel has difficult reace conditions and platform
     independence issues. */
  m_socket->SetReadTimeout(1000);

  if (m_socket->Connect(device) && m_socket->SetFilter(filter)) {
    m_running = true;
    m_thread = new PThreadObj<PEthSocketThread>(*this, &PEthSocketThread::MainLoop, false, "Sniffer", priority);
    return true;
  }

  delete m_socket;
  m_socket = NULL;
  return false;
}


void PEthSocketThread::Stop()
{
  if (m_thread == NULL)
    return;

  m_running = false;
  m_thread->WaitForTermination();

  delete m_thread;
  m_thread = NULL;

  delete m_socket;
  m_socket = NULL;
}


PEthSocket * PEthSocketThread::CreateEthSocket() const
{
  return new PEthSocket;
}


void PEthSocketThread::MainLoop()
{
  PTRACE(4, "Ethernet sniffer thread started, filter=\"" << m_socket->GetFilter() << '"');

  while (m_running) {
    if (m_socket->ReadFrame(m_frame))
      m_notifier(*m_socket, m_frame);
    else {
      switch (m_socket->GetErrorCode(PChannel::LastReadError)) {
        case PChannel::Timeout :
        case PChannel::NotOpen :
          break;

        default :
          PTRACE(1, "Ethernet read error: " << m_socket->GetErrorText(PChannel::LastReadError));
          m_running = false;
      }
    }
  }

  PTRACE(4, "Ethernet sniffer thread finished");
}


// End Of File ///////////////////////////////////////////////////////////////
