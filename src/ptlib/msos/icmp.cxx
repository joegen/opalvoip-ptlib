/*
 * $Id: icmp.cxx,v 1.7 1996/10/29 13:27:17 robertj Exp $
 *
 * Portable Windows Library
 *
 * Socket Classes Implementation
 *
 * Copyright 1996 Equivalence
 *
 * $Log: icmp.cxx,v $
 * Revision 1.7  1996/10/29 13:27:17  robertj
 * Change ICMP to use DLL rather than Winsock
 *
 */

#include <ptlib.h>
#include <sockets.h>

///////////////////////////////////////////////////////////////
//
// Definitions for Microsft ICMP library
//

// return values from IcmpSendEcho

#define IP_STATUS_BASE				11000
#define IP_SUCCESS					0
#define IP_BUF_TOO_SMALL			(IP_STATUS_BASE + 1)
#define IP_DEST_NET_UNREACHABLE		(IP_STATUS_BASE + 2)
#define IP_DEST_HOST_UNREACHABLE	(IP_STATUS_BASE + 3)
#define IP_DEST_PROT_UNREACHABLE	(IP_STATUS_BASE + 4)
#define IP_DEST_PORT_UNREACHABLE	(IP_STATUS_BASE + 5)
#define IP_NO_RESOURCES				(IP_STATUS_BASE + 6)
#define IP_BAD_OPTION				(IP_STATUS_BASE + 7)
#define IP_HW_ERROR					(IP_STATUS_BASE + 8)
#define IP_PACKET_TOO_BIG			(IP_STATUS_BASE + 9)
#define IP_REQ_TIMED_OUT			(IP_STATUS_BASE + 10)
#define IP_BAD_REQ					(IP_STATUS_BASE + 11)
#define IP_BAD_ROUTE				(IP_STATUS_BASE + 12)
#define IP_TTL_EXPIRED_TRANSIT		(IP_STATUS_BASE + 13)
#define IP_TTL_EXPIRED_REASSEM		(IP_STATUS_BASE + 14)
#define IP_PARAM_PROBLEM			(IP_STATUS_BASE + 15)
#define IP_SOURCE_QUENCH			(IP_STATUS_BASE + 16)
#define IP_OPTION_TOO_BIG			(IP_STATUS_BASE + 17)
#define IP_BAD_DESTINATION			(IP_STATUS_BASE + 18)
#define IP_ADDR_DELETED				(IP_STATUS_BASE + 19)
#define IP_SPEC_MTU_CHANGE			(IP_STATUS_BASE + 20)
#define IP_MTU_CHANGE				(IP_STATUS_BASE + 21)
#define IP_UNLOAD					(IP_STATUS_BASE + 22)
#define IP_GENERAL_FAILURE			(IP_STATUS_BASE + 50)
#define MAX_IP_STATUS				IP_GENERAL_FAILURE
#define IP_PENDING					(IP_STATUS_BASE + 255)


// ICMP request options structure

typedef struct ip_info {
     u_char Ttl;               /* Time To Live (used for traceroute) */
     u_char Tos;               /* Type Of Service (usually 0) */
     u_char Flags;             /* IP header flags (usually 0) */
     u_char OptionsSize;       /* Size of options data (usually 0, max 40) */
     u_char FAR *OptionsData;  /* Options data buffer */
} IPINFO;

//
// ICMP reply data
//
// The reply buffer will have an array of ICMP_ECHO_REPLY
// structures, followed by options and the data in ICMP echo reply
// datagram received. You must have room for at least one ICMP
// echo reply structure, plus 8 bytes for an ICMP header.
// 

typedef struct icmp_echo_reply {
     u_long Address;         /* source address */
     u_long Status;          /* IP status value (see below) */
     u_long RTTime;          /* Round Trip Time in milliseconds */
     u_short DataSize;       /* reply data size */
     u_short Reserved;    
     void FAR *Data;         /* reply data buffer */
     struct ip_info Options; /* reply options */
} ICMPECHO;

#ifdef __cplusplus
extern "C" {
#endif

//
// create an ICMP "handle"
// returns INVALID_HANDLE_VALUE on error 
//
//

HANDLE PASCAL IcmpCreateFile(void);


//
// close a handle allocated by IcmpCreateFile
// returns FALSE on error
//

BOOL PASCAL IcmpCloseHandle(HANDLE handle);

DWORD PASCAL IcmpSendEcho(
     HANDLE   handle,           /* handle returned from IcmpCreateFile() */
     u_long   destAddr,         /* destination IP address (in network order) */
     void   * sendBuffer,       /* pointer to buffer to send */
     WORD     sendLength,       /* length of data in buffer */
     IPINFO * requestOptions,   /* see structure definition above */
     void   * replyBuffer,      /* structure definitionm above */
     DWORD    replySize,        /* size of reply buffer */
     DWORD    timeout           /* time in milliseconds to wait for reply */
);

#ifdef __cplusplus
};
#endif


PICMPSocket::PICMPSocket()
{
  OpenSocket();
}

BOOL PICMPSocket::IsOpen() const
{
  return icmpHandle != NULL;
}


BOOL PICMPSocket::OpenSocket()
{
  return (icmpHandle = IcmpCreateFile()) != NULL;
}


BOOL PICMPSocket::Close()
{
  if (icmpHandle != NULL) 
    return IcmpCloseHandle(icmpHandle);
  else
	return TRUE;
}

const char * PICMPSocket::GetProtocolName() const
{
  return "icmp";
}

BOOL PICMPSocket::Ping(const PString & host)
{
  PingInfo info;
  return Ping(host, info);
}


BOOL PICMPSocket::Ping(const PString & host, PingInfo & info)
{
  // find address of the host
  PIPSocket::Address addr;
  if (!GetHostAddress(host, addr)) {
    lastError = BadParameter;
    return FALSE;
  }

  BYTE sendBuffer[64];
  IPINFO requestOptions;
  BYTE replyBuffer[256];

  memset(&requestOptions, 0, sizeof(requestOptions));
  memset(sendBuffer,     0, sizeof(sendBuffer));
  memset(replyBuffer,    0, sizeof(replyBuffer));

  requestOptions.Ttl = 255;

  if (IcmpSendEcho(
	         icmpHandle, 
	         addr,
             sendBuffer, sizeof(sendBuffer),   
             &requestOptions,      
             replyBuffer, sizeof(replyBuffer), 
             GetReadTimeout().GetInterval()) == 0)
    return FALSE;

  ICMPECHO * reply = (ICMPECHO *)replyBuffer;

  if (reply->Status != IP_SUCCESS)
    return FALSE;

  info.remoteAddr = Address((in_addr&)reply->Address);
  GetHostAddress(info.localAddr);
  info.delay.SetInterval(reply->RTTime);

  // leave these two fields unchanged
  //info.identifier   = 
  //info.sequenceNum  = 
  return TRUE;
}

// End Of File ///////////////////////////////////////////////////////////////
