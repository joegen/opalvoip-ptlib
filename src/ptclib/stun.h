//
// stund/stun.h
// 
// Copyright (c) 2002 Alan Hawrylyshen
//
/// @author Alan Hawrylyshen
// 
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
// 
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
//

#ifndef STUN_H
#define STUN_H


#include <iostream.h>


// define some basic types
typedef unsigned char  UInt8;
typedef unsigned short UInt16;
typedef unsigned int   UInt32;
typedef struct { unsigned char octet[8]; }  UInt128;


/// define a structure to hold a stun address 
const UInt8  IPv4Family = 0x01;
const UInt8  IPv6Family = 0x02;

typedef struct
{
    UInt16 port;
    UInt8 family;
    UInt8 pad;
} StunAddrHdr;

typedef struct
{
    StunAddrHdr addrHdr;
    union
    {
	UInt32  v4addr;
	UInt128 v6addr;
    } addr;
} StunAddress;


/// Define enum with different types of NAT 
typedef enum 
{
  StunTypeUnknown=0,
  StunTypeOpen,
  StunTypeConeNat,
  StunTypeRestrictedNat,
  StunTypePortRestrictedNat,
  StunTypeSymNat,
  StunTypeSymFirewall,
  StunTypeBlocked,
} StunNatType;


/// prints a StunAddress
ostream& 
operator<<( ostream& strm, const StunAddress& addr);


/// gets a consecutive pair (first is even) or public sockets
bool stunOpenSocketPair( StunAddress& stunServerAddr, StunAddress* sAddr, int* fd1, int* fd2, int port = 0 );


/// gets a UDP socket for use and returns it's public address
int
stunOpenSocket( StunAddress& stunServerAddr, StunAddress* sAddr, int port = 0 );


/// find the IP address of a the specified stun server - 
void 
stunParseServerName( char* serverName, StunAddress* stunServerAddr);


/// find out what type of NAT you are behind - takes a second or so
StunNatType
stunType( StunAddress& dest,bool verbose, int portBase = 0);


/// run a specific type of test 
void 
stunTest( StunAddress& dest, int testNum, bool verbose );


/// start running a stun server - this call never returns 
void
stunServer( StunAddress myAddr, StunAddress altAddr, bool verbose );


#endif
