
/*
 * $Id: uicmp.cxx,v 1.5 1998/01/26 07:27:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Classes Implementation
 *
 * Copyright 1996 Equivalence
 *
 * $Log: uicmp.cxx,v $
 * Revision 1.5  1998/01/26 07:27:09  robertj
 * Added part support for extra ping info. Still needs TTL for traceroute.
 *
 * Revision 1.4  1996/11/16 11:12:56  craigs
 * Fixed problem with work misaligns under SOlaris
 *
 * Revision 1.3  1996/10/31 10:20:07  craigs
 * Moved ICMP implementation into here, as it is now platform dependent
 *
 * Revision 1.6  1996/09/14 13:09:34  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.5  1996/08/11 06:52:14  robertj
 * Oops
 *
 * Revision 1.4  1996/08/07 13:40:57  robertj
 * Fixed sparc memory alignment problem from int 64
 *
 * Revision 1.3  1996/06/03 10:03:10  robertj
 * Changed ping to return more parameters.
 *
 * Revision 1.2  1996/05/30 10:08:51  robertj
 * Fixed bug in ping (checksum incorrect).
 *
 * Revision 1.1  1996/05/15 21:11:35  robertj
 * Initial revision
 *
 */

#pragma implementation "icmpsock.h"

#include <ptlib.h>
#include <sockets.h>

#define	MAX_IP_LEN	60
#define	MAX_ICMP_LEN	76
#define	ICMP_DATA_LEN	(64-8)
#define	RX_BUFFER_SIZE	(MAX_IP_LEN+MAX_ICMP_LEN+ICMP_DATA_LEN)

#define	ICMP_ECHO_REPLY	0
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
  DWORD sum = 0;
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
{
  OpenSocket();
}


BOOL PICMPSocket::Ping(const PString & host)
{
  PingInfo info;
  return Ping(host, info);
}


BOOL PICMPSocket::Ping(const PString & host, PingInfo & info)
{
  if (!WritePing(host, info))
    return FALSE;

  return ReadPing(info);
}


BOOL PICMPSocket::WritePing(const PString & host, PingInfo & info)
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
  memset(&packet, 0, sizeof(packet));

  packet.type     = ICMP_ECHO;
  packet.sequence = info.sequenceNum;
  packet.id       = info.identifier;

  // set the send time
  packet.sendtime = PTimer::Tick().GetMilliSeconds();

  // calculate the checksum
  packet.checksum = CalcChecksum(&packet, sizeof(packet));

  // send the packet
  return WriteTo(&packet, sizeof(packet), addr, 0);
}


BOOL PICMPSocket::ReadPing(PingInfo & info)
{
  // receive a packet
  BYTE packet[RX_BUFFER_SIZE];
  IPHdr      * ipHdr;
  ICMPPacket * icmpPacket;
  WORD port;
  PInt64 now;
  PTimer timeout(GetReadTimeout());

  for (;;) {
    memset(&packet, 0, sizeof(packet));

    if (!ReadFrom(packet, sizeof(packet), info.remoteAddr, port))
      return FALSE;

    now  = PTimer::Tick().GetMilliSeconds();
    ipHdr      = (IPHdr *)packet;
    icmpPacket = (ICMPPacket *)(packet + ((ipHdr->verIhl & 0xf) << 2));
    if ((      icmpPacket->type == ICMP_ECHO_REPLY) && 
        ((WORD)icmpPacket->id   == info.identifier))
      break;

    if (!timeout.IsRunning())
      return FALSE;
  }

  info.remoteAddr = Address(ipHdr->sourceAddr[0], ipHdr->sourceAddr[1],
                            ipHdr->sourceAddr[2], ipHdr->sourceAddr[3]);
  info.localAddr  = Address(ipHdr->destAddr[0], ipHdr->destAddr[1],
                            ipHdr->destAddr[2], ipHdr->destAddr[3]);

  // calc round trip time. Be careful, as unaligned "long long" ints
  // can cause problems on some platforms
#if defined(P_SUN4) || defined(P_SOLARIS)
  PInt64 then;
  BYTE * pthen = (BYTE *)&then;
  BYTE * psendtime = (BYTE *)&icmpPacket->sendtime;
  memcpy(pthen, psendtime, sizeof(PInt64));
  info.delay.SetInterval(now - then);
#else
  info.delay.SetInterval(now - icmpPacket->sendtime);
#endif

  info.sequenceNum = icmpPacket->sequence;
  return TRUE;
}


BOOL PICMPSocket::OpenSocket()
{
  struct protoent * p = ::getprotobyname(GetProtocolName());
  if (p == NULL)
    return ConvertOSError(-1);
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_RAW, p->p_proto));
}


const char * PICMPSocket::GetProtocolName() const
{
  return "icmp";
}


PICMPSocket::PingInfo::PingInfo(WORD id)
{
  identifier = id;
  sequenceNum = 0;
  ttl = 255;
  buffer = NULL;
  status = Success;
}


// End Of File ///////////////////////////////////////////////////////////////
