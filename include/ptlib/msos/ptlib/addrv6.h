/////////////////////////////////////////////////////////////////////////////
//
// addrv6.h : Definitions for getting local IP addresses, both ipv4 and ipv6
//
// Copyright (c) 2008 Dinsk, dinsk.net
// Developed for OPAL VoIP project, opalvoip.org
//
/////////////////////////////////////////////////////////////////////////////

#include <ptlib.h>

#ifdef _WIN32

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>

typedef USHORT ADDRESS_FAMILY;

typedef union _SOCKADDR_INET {
    SOCKADDR_IN Ipv4;
    SOCKADDR_IN6 Ipv6;
    ADDRESS_FAMILY si_family;    
} SOCKADDR_INET, *PSOCKADDR_INET;

typedef struct _IP_ADDRESS_PREFIX {
    SOCKADDR_INET Prefix;
    UINT8 PrefixLength;
} IP_ADDRESS_PREFIX, *PIP_ADDRESS_PREFIX;    

//
// Routing protocol values from RFC.
//
typedef enum {
    //
    // TODO: Remove the RouteProtocol* definitions.
    //
    RouteProtocolOther   = 1,
    RouteProtocolLocal   = 2,
    RouteProtocolNetMgmt = 3,
    RouteProtocolIcmp    = 4,
    RouteProtocolEgp     = 5,
    RouteProtocolGgp     = 6,
    RouteProtocolHello   = 7,
    RouteProtocolRip     = 8,
    RouteProtocolIsIs    = 9,
    RouteProtocolEsIs    = 10,
    RouteProtocolCisco   = 11,
    RouteProtocolBbn     = 12,
    RouteProtocolOspf    = 13,
    RouteProtocolBgp     = 14,

} NL_ROUTE_PROTOCOL, *PNL_ROUTE_PROTOCOL;

//
// NL_ROUTE_ORIGIN
//
// Define route origin values.
//

typedef enum _NL_ROUTE_ORIGIN {
    NlroManual,
    NlroWellKnown,
    NlroDHCP,
    NlroRouterAdvertisement,
    Nlro6to4,
} NL_ROUTE_ORIGIN, *PNL_ROUTE_ORIGIN;

typedef union _NET_LUID {
  ULONG64 Value;
  struct {
    ULONG64 Reserved  :24;
    ULONG64 NetLuidIndex  :24;
    ULONG64 IfType  :16;
  } Info;
} NET_LUID, *PNET_LUID;

typedef ULONG NET_IFINDEX;

typedef struct _MIB_IPFORWARD_ROW2 {
    //
    // Key Structure.
    //
    NET_LUID InterfaceLuid;
    NET_IFINDEX InterfaceIndex;
    IP_ADDRESS_PREFIX DestinationPrefix;
    SOCKADDR_INET NextHop;

    //
    // Read-Write Fields.
    //
    UCHAR SitePrefixLength;
    ULONG ValidLifetime;
    ULONG PreferredLifetime;
    ULONG Metric;
    NL_ROUTE_PROTOCOL Protocol;
    
    BOOLEAN Loopback;
    BOOLEAN AutoconfigureAddress;
    BOOLEAN Publish;
    BOOLEAN Immortal;

    //
    // Read-Only Fields.
    //
    ULONG Age;
    NL_ROUTE_ORIGIN Origin;
} MIB_IPFORWARD_ROW2, *PMIB_IPFORWARD_ROW2;  

typedef struct _MIB_IPFORWARD_TABLE2 {
    ULONG NumEntries;
    MIB_IPFORWARD_ROW2 Table[ANY_SIZE];
} MIB_IPFORWARD_TABLE2, *PMIB_IPFORWARD_TABLE2;

typedef ULONG NETIO_STATUS;

typedef NETIO_STATUS (WINAPI *GETIPFORWARDTABLE2)(
  __in   ADDRESS_FAMILY  Family,
  __out  PMIB_IPFORWARD_TABLE2 *Table
);

typedef void (WINAPI *FREEMIBTABLE)(
    IN PVOID Memory
    ); 

#ifdef __cplusplus
}
#endif

bool GetFirstIPV4AddressIn(const MIB_IPFORWARD_TABLE2& t, in_addr* sin_addr, ULONG* pIfIndex = NULL)
{
	for ( ULONG ul = 0L; ul < t.NumEntries; ul++  )
	{
		if(t.Table[ul].NextHop.si_family == AF_INET &&
			( t.Table[ul].NextHop.Ipv4.sin_addr.S_un.S_un_b.s_b1
			|| t.Table[ul].NextHop.Ipv4.sin_addr.S_un.S_un_b.s_b2
			|| t.Table[ul].NextHop.Ipv4.sin_addr.S_un.S_un_b.s_b3
			|| t.Table[ul].NextHop.Ipv4.sin_addr.S_un.S_un_b.s_b4 )
			)
		{	
			if( sin_addr )
				CopyMemory(sin_addr,  &(t.Table[ul].NextHop.Ipv4.sin_addr), sizeof(in_addr));
			if( pIfIndex )
				*pIfIndex = t.Table[ul].InterfaceIndex;
			return true;
		}
	}

	return false;
}

bool GetFirstIPV6AddressIn(const MIB_IPFORWARD_TABLE2& t, in6_addr* sin6_addr, ULONG* pIfIndex = NULL)
{
	for ( ULONG ul = 0L; ul < t.NumEntries; ul++  )
	{
		if(t.Table[ul].NextHop.si_family == AF_INET6 &&
		( t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[0]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[1]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[2]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[3]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[4]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[5]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[6]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[7]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[8]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[9]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[10]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[11]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[12]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[13]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[14]
		|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[15] ))
		{	
			if( sin6_addr )
				CopyMemory(sin6_addr,  &(t.Table[ul].NextHop.Ipv6.sin6_addr), sizeof(in6_addr));

			if( pIfIndex )
				*pIfIndex = t.Table[ul].InterfaceIndex;
			return true;
		}
	}

	return false; 
}

bool ValidateAddressIn(const MIB_IPFORWARD_TABLE2& t,  DWORD IfIndex, LPSOCKADDR lpSockAddr)
{
	if( !lpSockAddr )
		return false;

	if( !(lpSockAddr->sa_family == AF_INET || lpSockAddr->sa_family == AF_INET6 )
		)
		return false;

	if(lpSockAddr->sa_family == AF_INET)
	{
		sockaddr_in sin_addr;
		ZeroMemory(&sin_addr, sizeof(sin_addr));
		CopyMemory(&sin_addr,  lpSockAddr, sizeof(sin_addr));

		for ( ULONG ul = 0L; ul < t.NumEntries; ul++  )
		{
			if(IfIndex == t.Table[ul].InterfaceIndex)
			{
				if(t.Table[ul].NextHop.si_family == AF_INET6 &&
				( t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[0]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[1]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[2]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[3]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[4]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[5]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[6]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[7]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[8]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[9]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[10]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[11]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[12]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[13]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[14]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[15] ))
				{
					return true;
				}
			}
		}

	}
	else
	if(lpSockAddr->sa_family == AF_INET6)
	{
		sockaddr_in6 sin6_addr;
		ZeroMemory(&sin6_addr, sizeof(sin6_addr));
		CopyMemory(&sin6_addr,  lpSockAddr, sizeof(sin6_addr));
		for ( ULONG ul = 0L; ul < t.NumEntries; ul++  )
		{
			if(IfIndex == t.Table[ul].InterfaceIndex)
			{
				if(t.Table[ul].NextHop.si_family == AF_INET6 &&
				( t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[0]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[1]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[2]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[3]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[4]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[5]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[6]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[7]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[8]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[9]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[10]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[11]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[12]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[13]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[14]
				|| t.Table[ul].NextHop.Ipv6.sin6_addr.u.Byte[15] ))
				return true;
			}
		}
	}

	return false; // we shouldn't be here
}

bool GetInterfaceDescrByIndex(DWORD IfIndex,  PString& ifFriendlyName, PString& ifDescr, PString& ifName)
{
	BYTE *buffer = NULL;
	ULONG size = 0;

	DWORD error = GetAdaptersAddresses(AF_UNSPEC, 0, NULL, NULL, &size);
	buffer = (BYTE*) malloc(size);
	if( buffer )
	{
		error = GetAdaptersAddresses(AF_UNSPEC, 
			GAA_FLAG_INCLUDE_PREFIX
			|GAA_FLAG_SKIP_ANYCAST
			|GAA_FLAG_SKIP_DNS_SERVER
			|GAA_FLAG_SKIP_MULTICAST, 
			NULL, 
			(IP_ADAPTER_ADDRESSES *)buffer, &size);

		IP_ADAPTER_ADDRESSES* Current = (IP_ADAPTER_ADDRESSES*) buffer;
		while( Current )		
		{
			if( Current->IfIndex == IfIndex )
			{
				ifFriendlyName = Current->FriendlyName;
				ifDescr = Current->Description;
				ifName = Current->AdapterName;
				free(buffer);
				return true;
			}

			Current = Current->Next;
		}

		free(buffer);
	}

	return false; // we shouldn't really be here
}

#endif // _WIN32