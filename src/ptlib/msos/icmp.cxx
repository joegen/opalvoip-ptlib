/*
 * $Id: icmp.cxx,v 1.1 1996/05/15 21:11:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Classes Implementation
 *
 * Copyright 1996 Equivalence
 *
 * $Log: icmp.cxx,v $
 * Revision 1.1  1996/05/15 21:11:35  robertj
 * Initial revision
 *
 */

#include <ptlib.h>
#include <sockets.h>


#define	MAX_IP_LEN	60
#define	MAX_ICMP_LEN	76
#define	ICMP_DATA_LEN	(64-8)
#define	RX_BUFFER_SIZE	(MAX_IP_LEN+MAX_ICMP_LEN+ICMP_DATA_LEN)

#define	ICMP_ECHO	8


typedef struct {
  BYTE   type;
  BYTE   code;
  WORD   checksum;

  WORD   id;
  WORD   sequence;

  PInt64 sendtime;
  BYTE   data[ICMP_DATA_LEN-sizeof(PInt64)];
} ICMPPacket;


typedef struct {
  BYTE verIhl;
  BYTE typeOfService;
  BYTE totalLength;
  WORD identification;
  WORD fragOff;
  BYTE timeToLive;
  BYTE protocol;
  WORD checksum;
  BYTE sourceAddr[4];
  BYTE destAddr[4];
} IPHdr;


static WORD CalcChecksum(void * p, PINDEX len)
{
  WORD * ptr = (WORD *)p;
  PInt32 sum = 0;
  while (len > 1) {
    sum += *ptr++;
    len-=2;
  }

  if (len > 0) {
    WORD t = *(BYTE *)ptr;
    sum += t;
  }

  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return (WORD)~sum;
}


PICMPSocket::PICMPSocket()
  : PIPDatagramSocket(SOCK_RAW, "icmp")
{
  _Socket();
}


BOOL PICMPSocket::Ping(const PString & host, PTimeInterval & delay)
{
  if (!WritePing(host))
    return FALSE;

  Address addr;
  WORD seq;
  return ReadPing(addr, seq, delay);
}


BOOL PICMPSocket::WritePing(const PString & host, WORD sequenceNum)
{
  // find address of the host
  PIPSocket::Address addr;
  if (!GetHostAddress(host, addr)) {
    lastError = BadParameter;
    return FALSE;
  }

  // create the ICMP packet
  ICMPPacket packet;

  // clear the packet including data area
  memset(&packet.data, 0x00, sizeof(packet.data));

  packet.type     = ICMP_ECHO;
  packet.sequence = sequenceNum;
  packet.id       = (WORD)PProcess::Current()->GetProcessID();

  // set the send time
  packet.sendtime = PTimer::Tick().GetMilliSeconds();

  // calculate the checksum
  packet.checksum = CalcChecksum(&packet, sizeof(packet));

  // send the packet
  return WriteTo(&packet, sizeof(packet), addr, 0);
}


BOOL PICMPSocket::ReadPing(Address & addr,
                           WORD & sequenceNum,
                           PTimeInterval & delay)
{
  // receive a packet
  BYTE packet[RX_BUFFER_SIZE];
  IPHdr      * ipHdr;
  ICMPPacket * icmpPacket;
  WORD port;
  PInt64 now;
  WORD id = (WORD)PProcess::Current()->GetProcessID();

  for (;;) {
    if (!ReadFrom(packet, sizeof(packet), addr, port))
      return FALSE;
    now  = PTimer::Tick().GetMilliSeconds();
    ipHdr      = (IPHdr *)packet;
    icmpPacket = (ICMPPacket *)(packet + ((ipHdr->verIhl & 0xf) << 2));
    if (id == (WORD)icmpPacket->id)
      break;
  }

  // calc round trip time. Be careful, as unaligned "long long" ints
  // can cause problems on some platforms
#ifdef P_SUN4
  PInt64 then;
  memcpy(&then, &icmpPacket->sendtime, sizeof(PInt64));
  delay.SetInterval(now - then);
#else
  delay.SetInterval(now - icmpPacket->sendtime);
#endif

  sequenceNum = icmpPacket->sequence;
  return TRUE;
}


WORD PICMPSocket::GetPortByService(const PString &) const
{
  PAssertAlways(PUnsupportedFeature);
  return 0;
}


PString PICMPSocket::GetServiceByPort(WORD) const
{
  PAssertAlways(PUnsupportedFeature);
  return PString();
}
