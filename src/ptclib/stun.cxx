//
// stund/stun.cxx
// 
// Copyright (c) 2002 Alan Hawrylyshen
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



#include <cassert>
#include <cstring>

#if !defined(_ERRNO_H)
#include <errno.h>
#endif

#ifndef WIN32
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#endif

#include "udp.h"
#include "stun.h"

typedef struct 
{
      UInt16 type;
      UInt16 length;
      UInt32 id;
} StunHdr;


// define structure to hold flags message 
const UInt32 ChangeIpFlag   = 1<<0;
const UInt32 ChangePortFlag = 1<<1;
const UInt32 DiscardFlag    = 1<<2;

typedef struct
{
      UInt32 value;
} StunFlagsBody;


// define structure to hold a stun attribute
const UInt16 MappedAddress = 0x0001;
const UInt16 ResponseAddress = 0x0002;
const UInt16 Flags = 0x0003;
const UInt16 SourceAddress = 0x0004;
const UInt16 ChangedAddress = 0x0005;
typedef struct
{
      UInt16 type;
      UInt16 length;
} StunAttributeHdr;

typedef struct
{
      StunAttributeHdr hdr;
      union
      {
            StunAddress   mappedAddress;
            StunAddress   responseAddress;
            StunFlagsBody flags;
            StunAddress   sourceAddress;
            StunAddress   changeAddress;
      } value;
} StunAttribute;


// define sturcture for a stun message 
const UInt16 RequestMsg  = 0x0001;
const UInt16 ResponseMsg = 0x0101;

// define the response msg
typedef struct
{
      UInt16 type;
      UInt16 length;
      UInt32 id;   
      struct
      {
            UInt16 type;
            UInt16 length;
            UInt16 port;
            UInt8  family;
            UInt8  pad;
            UInt32 v4addr;
      } mappedAddr;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt16 port;
            UInt8  family;
            UInt8  pad;
            UInt32 v4addr;
      } changeAddr;
      struct
      {
            UInt16 type;
            UInt16 length;
            UInt16 port;
            UInt8  family;
            UInt8  pad;
            UInt32 v4addr;
      } sourceAddr;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt16 port;
            UInt8  family;
            UInt8  pad;
            UInt32 v4addr;
      } responseAddr;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt32 flags;
      } flag;
} Stun4Response;


// define the a simple request
typedef struct
{
      UInt16 type;
      UInt16 length;
      UInt32 id;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt32 flags;
      } flag;
} StunRequestSimple;


// define the a simple request
typedef struct
{
      UInt16 type;
      UInt16 length;
      UInt32 id;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt32 flags;
      } flag;
      struct 
      {
            UInt16 type;
            UInt16 length;
            UInt16 port;
            UInt8  family;
            UInt8  pad;
            UInt32 v4addr;
      } responseAddr;
} Stun4RequestResponse;


static bool
stunParseResponse( char* buf, unsigned int bufLen, 
		   UInt32* id,
		   bool verbose=false,
		   StunAddress* mappedAddr=0,
		   StunAddress* changedAddr=0,
		   StunAddress* sourceAddr=0,
		   StunAddress* responseAddr=0,
		   UInt32* flags=0,
		   UInt16* msgType=0);

static bool
stunServerProcessMsg( char* buf, unsigned int bufLen, StunAddress& from, 
                      StunAddress& myAddr, StunAddress& altAddr, 
                      Stun4Response* resp, StunAddress* destination, bool* changePort,
                      bool verbose);

static void 
stunBuildReqSimple( StunRequestSimple* msg, unsigned int id=0, 
		    bool changePort=false, bool changeIp=false, bool discard=false );

  
// This funtion takes a single message sent to a stun server, parses
// and constructs an apropriate repsonse - returns true if message is
// valid

static bool
stunServerProcessMsg( char* buf,
                      unsigned int bufLen,
                      StunAddress& from, 
                      StunAddress& myAddr,
                      StunAddress& altAddr, 
                      Stun4Response* resp,
                      StunAddress* destination,
                      bool* changePort,
                      bool verbose)
{
    
   // set up information for default response 

   bool changeIp = false;
   *changePort = false;
   UInt32 id=0;
   UInt32 flags=0;
   UInt16 msgType=0;
   StunAddress mapped;
   StunAddress respondTo;
    
   bool ok = stunParseResponse( buf,
                                bufLen, 
                                &id, 
                                verbose,
                                &mapped,  
                                0,
                                0,
                                &respondTo,
                                &flags,
                                &msgType);
    
   if ( respondTo.addrHdr.port == 0 ) respondTo = from;
   if ( mapped.addrHdr.port == 0 ) mapped = from;
    
   if (!ok)
      return false;

   if ( msgType != RequestMsg )
      return false;
    
   if ( flags & DiscardFlag )
      return false;
   changeIp    = ( flags & ChangeIpFlag )?true:false;
   *changePort = ( flags & ChangePortFlag )?true:false;

   if (verbose)
   {
      clog << "Request is valid:" << endl;
      clog << "\t id=" << id << endl;
      clog << "\t changeIp=" << changeIp << endl;
      clog << "\t changePort=" << *changePort << endl;
      clog << "\t from = " << from << endl;
      clog << "\t respond to = " << respondTo << endl;
      clog << "\t mapped = " << mapped << endl;
   }
    
   // form the outgoing message
   resp->type = htons(ResponseMsg);
   resp->length = htons(sizeof(Stun4Response ));
   resp->id = htonl(id);
    
   resp->sourceAddr.type   = htons(SourceAddress);
   resp->sourceAddr.length = htons(sizeof(resp->sourceAddr));
   resp->sourceAddr.port   = htons((*changePort) ? altAddr.addrHdr.port : myAddr.addrHdr.port);
   resp->sourceAddr.family = IPv4Family;
   resp->sourceAddr.v4addr = htonl(myAddr.addr.v4addr);
    
   resp->mappedAddr.type   = htons(MappedAddress);
   resp->mappedAddr.length = htons(sizeof(resp->mappedAddr));
   resp->mappedAddr.port   = htons(mapped.addrHdr.port);
   resp->mappedAddr.family = IPv4Family;
   resp->mappedAddr.v4addr = htonl(mapped.addr.v4addr);
    
   resp->changeAddr.type   = htons(ChangedAddress);
   resp->changeAddr.length = htons(sizeof(resp->changeAddr));
   resp->changeAddr.port   = htons(altAddr.addrHdr.port);
   resp->changeAddr.family = IPv4Family;
   resp->changeAddr.v4addr = htonl(altAddr.addr.v4addr);

   resp->responseAddr.type   = htons(ResponseAddress);
   resp->responseAddr.length = htons(sizeof(resp->responseAddr));
   resp->responseAddr.port   = htons(respondTo.addrHdr.port);
   resp->responseAddr.family = IPv4Family;
   resp->responseAddr.v4addr = htonl(respondTo.addr.v4addr);

   resp->flag.type   = htons(Flags);
   resp->flag.length = htons(sizeof(resp->flag));
   resp->flag.flags  = htonl(0);

   destination->addrHdr.port = respondTo.addrHdr.port;
   destination->addr.v4addr  = respondTo.addr.v4addr;

   if ( changeIp )
   {
      resp->type = htons(RequestMsg);

      destination->addrHdr.port = myAddr.addrHdr.port;
      destination->addr.v4addr  = altAddr.addr.v4addr;

      if ( *changePort ) 
      {
         resp->flag.flags = resp->flag.flags | htonl( ChangePortFlag );
      }
   }

   return true;
}



static void
stunBuildReqSimple( StunRequestSimple* msg, unsigned int id,
		    bool changePort, bool changeIp, bool discard )
{
   assert( msg );
    
   static int nextId=1000;
   if ( id == 0 ) 
   {
      id = nextId++;
   }
    
   msg->type = htons(RequestMsg);
   msg->length = htons(sizeof( StunRequestSimple ));
   msg->id = htonl(id);
    
   msg->flag.type = htons(Flags);
   msg->flag.length = htons(sizeof( msg->flag ));
   msg->flag.flags = htonl( (changeIp?ChangeIpFlag:0) | 
                            (changePort?ChangePortFlag:0) | 
                            (discard?DiscardFlag:0) );
}


void
stunServer( StunAddress myAddr, StunAddress altAddr, bool verbose )
{
   assert( myAddr.addrHdr.port != 0 );
   assert( altAddr.addrHdr.port!= 0 );
   assert( myAddr.addr.v4addr  != 0 );
   assert( altAddr.addr.v4addr != 0 );
   
   char msg[udpMaxMessageLength];
   int msgLen = udpMaxMessageLength;

   StunAddress from;

   Stun4Response resp;
   StunAddress dest;
    
   Socket myFd  = openPort(myAddr.addrHdr.port, myAddr.addr.v4addr);
   Socket altFd = openPort(altAddr.addrHdr.port,myAddr.addr.v4addr);
    
   assert( myFd  != INVALID_SOCKET );
   assert( altFd != INVALID_SOCKET );

   while (true)
   {
      msgLen = sizeof(msg)/sizeof(*msg);

      bool ok;
      
      ok = getMessage( myFd, msg, &msgLen, &from.addr.v4addr, &from.addrHdr.port );
      
      if ( verbose && (!ok) )
      {
         clog << "Get message did not return a valid message" <<endl;
      }
       if ( !ok ) continue;
      
      if ( verbose )
      {
         clog << "Got a request (len=" << msgLen << ")" <<endl;
      }
      
    if ( msgLen <= 0 ) continue;

      bool changePort = false;
      ok = stunServerProcessMsg( msg,msgLen,from, myAddr,altAddr, &resp,&dest,&changePort, verbose );
     
      if ( dest.addr.v4addr == 0 ) ok=false;
      if ( dest.addrHdr.port == 0 ) ok=false;
      
      if ( ok )
      {
         assert( dest.addr.v4addr != 0 );
         assert( dest.addrHdr.port != 0 );

         if ( !changePort )
         {
            sendMessage( myFd, reinterpret_cast<char*>(&resp), sizeof(resp), 
                         dest.addr.v4addr, dest.addrHdr.port );
         }
         else
         {
            sendMessage( altFd,reinterpret_cast<char*>(&resp), sizeof(resp), 
                         dest.addr.v4addr, dest.addrHdr.port );
         }
      }
   }
}


ostream& 
operator<<( ostream& strm, const StunAddress& addr)
{
   UInt32 ip = addr.addr.v4addr;
   strm << ((int)(ip>>24)&0xFF) << ".";
   strm << ((int)(ip>>16)&0xFF) << ".";
   strm << ((int)(ip>> 8)&0xFF) << ".";
   strm << ((int)(ip>> 0)&0xFF) ;
  
   strm << ":" << addr.addrHdr.port;
  
   return strm;
}


// returns tru if it is a valid message 
static bool
stunParseResponse( char* buf, unsigned int bufLen, 
		   UInt32* id, 
		   bool verbose,
		   StunAddress* mappedAddr,  
		   StunAddress* changedAddr,
		   StunAddress* sourceAddr,
		   StunAddress* responseAddr,
		   UInt32* flags,
		   UInt16* msgType)
{
   assert( id );
   assert( buf );
  
   if ( mappedAddr  ) memset(mappedAddr,  0, sizeof(StunAddress));
   if ( changedAddr ) memset(changedAddr, 0, sizeof(StunAddress));
   if ( sourceAddr  ) memset(sourceAddr,  0, sizeof(StunAddress));
   if ( responseAddr) memset(responseAddr,0, sizeof(StunAddress));
  
   if ( bufLen < sizeof(StunHdr) )
      return false;

   StunHdr* hdr = reinterpret_cast<StunHdr*>(buf);
  
   UInt16 mType = ntohs(hdr->type);

   if ( (mType != ResponseMsg) && (mType != RequestMsg) )
      return false;
  
   if ( msgType )
   {
      *msgType = mType;
   }
   else
   {
      if ( mType != ResponseMsg ) 
      {
         return false;
      }
   }
  
   if ( ntohs(hdr->length) > bufLen ) 
   {
      return false;
   }
    
   if (id)
   {
      *id = ntohl(hdr->id);
	
   }
    
   char* body        = buf                + sizeof(StunHdr);
   unsigned int size = ntohs(hdr->length) - sizeof(StunHdr);

   while ( size > 0 )
   {
      if ( size < sizeof(StunAttributeHdr) )
         return false;

      StunAttribute* attr = reinterpret_cast<StunAttribute*>(body);
	
      unsigned int hdrLen = ntohs(attr->hdr.length);
	
      if ( hdrLen < sizeof(StunAttributeHdr) ) 
         return false;

      if ( hdrLen > size ) 
         return false;

      switch ( ntohs(attr->hdr.type) )
      {
         case MappedAddress:
         {
            if ( hdrLen < sizeof(StunAttributeHdr) + sizeof(StunAddrHdr) )
               return false;

            StunAddress* stunAddr = &attr->value.mappedAddress;
		
            if ( stunAddr->addrHdr.family != IPv4Family )
               return false;

            if ( hdrLen < ( sizeof(StunAttributeHdr)
                            + sizeof(StunAddrHdr)
                            + sizeof(stunAddr->addr.v4addr) ) )
               return false;

            if (mappedAddr )
            {
               mappedAddr->addrHdr.port   = ntohs(stunAddr->addrHdr.port);
               mappedAddr->addrHdr.family = IPv4Family;
               mappedAddr->addr.v4addr    = ntohl(stunAddr->addr.v4addr);
            }
         }
         break;

         case ResponseAddress:
         {
            if ( hdrLen < sizeof(StunAttributeHdr)+sizeof(StunAddrHdr) )
               return false;

            StunAddress* respAddr = &attr->value.responseAddress;
		
            if ( respAddr->addrHdr.family != IPv4Family )
               return false;

            if ( hdrLen < ( sizeof(StunAttributeHdr)
                            + sizeof(StunAddrHdr)
                            + sizeof(respAddr->addr.v4addr) ) )
               return false;

            if ( responseAddr )
            {
               responseAddr->addrHdr.port = ntohs(respAddr->addrHdr.port);
               responseAddr->addrHdr.family = IPv4Family;
               responseAddr->addr.v4addr =  ntohl(respAddr->addr.v4addr);
            }
         }
         break;  

         case Flags:
         {
            if ( hdrLen < sizeof(StunAttributeHdr)+sizeof(StunFlagsBody) )
               return false;
            if ( flags )
            {
               *flags = ntohl(attr->value.flags.value);
            }	    
         }
         break;
  
         case SourceAddress:
         {
            if ( hdrLen < sizeof(StunAttributeHdr)+sizeof(StunAddrHdr) )
               return false;

            StunAddress* stunAddr = &attr->value.mappedAddress;
		
            if ( stunAddr->addrHdr.family != IPv4Family )
               return false;

            if ( hdrLen < ( sizeof(StunAttributeHdr)
                            +sizeof(StunAddrHdr)
                            +sizeof(stunAddr->addr.v4addr) ) )
               return false;

            if ( sourceAddr )
            {
               sourceAddr->addrHdr.port   = ntohs(stunAddr->addrHdr.port);
               sourceAddr->addrHdr.family = IPv4Family;
               sourceAddr->addr.v4addr    =  ntohl(stunAddr->addr.v4addr);
            }
         }
         break;
  
         case ChangedAddress:
         {
            if ( hdrLen < sizeof(StunAttributeHdr)+sizeof(StunAddrHdr) )
               return false;
            StunAddress* stunAddr = &attr->value.mappedAddress;
		
            if ( stunAddr->addrHdr.family != IPv4Family )
               return false;

            if ( hdrLen < ( sizeof(StunAttributeHdr)
                            +sizeof(StunAddrHdr)
                            +sizeof(stunAddr->addr.v4addr) ) )
               return false;

            if ( changedAddr )
            {
               changedAddr->addrHdr.port   =  ntohs(stunAddr->addrHdr.port);
               changedAddr->addrHdr.family = IPv4Family;
               changedAddr->addr.v4addr    =  ntohl(stunAddr->addr.v4addr);
            }
         }
         break;
  
         default:
         {
            if ( attr->hdr.type <= 0x7FFF ) 
            {
               return false;
            }
         }
      }
	
      body += hdrLen;
      size -= hdrLen;
   }
    
   return true;
}


void
stunParseServerName( char* name, StunAddress* stunServerAddr)
{
   assert(name);
   assert(stunServerAddr);

   // TODO - put in DNS SRV stuff.

   parseHostName( name,
                  &stunServerAddr->addr.v4addr,
                  &stunServerAddr->addrHdr.port); 

   stunServerAddr->addrHdr.family =IPv4Family;
}

int
stunOpenSocket( StunAddress& dest, StunAddress* sAddr, int port )
{
   assert( dest.addr.v4addr != 0 );
   assert( dest.addrHdr.port != 0 );

   Socket myFd = openPort(port);

   char msg[udpMaxMessageLength];
   int msgLen = sizeof(msg)/sizeof(*msg);

   StunAddress from;

   StunRequestSimple req;
   stunBuildReqSimple( &req );

   sendMessage( myFd, 
                reinterpret_cast<char*>(&req), 
                req.length, 
                dest.addr.v4addr, 
                dest.addrHdr.port );
   
   getMessage( myFd, msg, &msgLen, &from.addr.v4addr, &from.addrHdr.port );

   StunAddress mappedAddr;
   StunAddress changedAddr;
   UInt32 id;
   stunParseResponse( msg, msgLen, &id, false, &mappedAddr, &changedAddr);
   
   clog << "--- stunOpenSocket --- " << endl;
   clog << "\treq  id=" << req.id << endl;
   clog << "\tresp id=" << id << endl;
   clog << "\tmappedAddr=" << mappedAddr << endl;

   *sAddr = mappedAddr;
   
   return myFd;
}


bool
stunOpenSocketPair( StunAddress& dest, StunAddress* sAddr, int* fd1, int* fd2 )
{
   assert( dest.addr.v4addr != 0 );
   assert( dest.addrHdr.port != 0 );
   
   const int NUM=3;
    
   char msg[udpMaxMessageLength];
   int msgLen =sizeof(msg)/sizeof(*msg);

   StunAddress from;
   int fd[NUM];
   int i;
   
   StunRequestSimple req;

   for( i=0; i<NUM; i++)
   {
      fd[i] = openPort();
   }
   
   for( i=0; i<NUM; i++)
   {
      stunBuildReqSimple( &req );
      sendMessage( fd[i], 
                   reinterpret_cast<char*>(&req),
                   req.length, 
                   dest.addr.v4addr, 
                   dest.addrHdr.port );
   }
   
   StunAddress mappedAddr[NUM];
   for( i=0; i<NUM; i++)
   {
      msgLen = sizeof(msg)/sizeof(*msg);
      getMessage( fd[i], 
                  msg,
                  &msgLen,
                  &from.addr.v4addr,
                  &from.addrHdr.port );

      StunAddress changedAddr;
      UInt32 id;

      stunParseResponse( msg,
                         msgLen,
                         &id, 
                         false,
                         &mappedAddr[i],
                         &changedAddr);
   }
  
   clog << "--- stunOpenSocketPair --- " << endl;

   for( i=0; i<NUM; i++)
   {
      clog << "\tmappedAddr=" << mappedAddr[i] << endl;
   }
   
   if ( mappedAddr[0].addrHdr.port %2 == 0 )
   {
      if (  mappedAddr[0].addrHdr.port+1 ==  mappedAddr[1].addrHdr.port )
      {
         *sAddr = mappedAddr[0];
         *fd1 = fd[0];
         *fd2 = fd[1];
         close( fd[2] );
         return true;
      }
   }
   else
   {
      if (( mappedAddr[1].addrHdr.port %2 == 0 ) 
          && (  mappedAddr[1].addrHdr.port+1 ==  mappedAddr[2].addrHdr.port ))
      {
         *sAddr = mappedAddr[1];
         *fd1 = fd[1];
         *fd2 = fd[2];
         close( fd[0] );
         return true;
      } 
   }
   
   for( i=0; i<NUM; i++)
   {
      close( fd[i] );
   }
   
   return false;
}



static void 
sendTest( int myFd, StunAddress& dest, int testNum )
{ 
   assert( dest.addr.v4addr != 0 );
   assert( dest.addrHdr.port != 0 );

   bool changePort=false;
   bool changeIP=false;
   bool discard=false;
 
   switch (testNum)
   {
      case 1:
      case 10:
         break;
      case 2:
         changePort=true;
         changeIP=true;
         break;
      case 3:
         changePort=true;
         break;
      case 4:
         changeIP=true;
         break;
      case 5:
         discard=true;
         break;
      default:
         cerr << "Test " << testNum <<" is unkown\n";
         assert(0);
   }
   
   StunRequestSimple req;

   stunBuildReqSimple( &req, testNum /* id */, 
		       changePort , changeIP , 
		       discard);
   
   sendMessage( myFd,
                reinterpret_cast<char*>(&req),
                sizeof(req),
                dest.addr.v4addr, 
                dest.addrHdr.port );
}


void 
stunTest( StunAddress& dest, int testNum, bool verbose )
{ 
   assert( dest.addr.v4addr != 0 );
   assert( dest.addrHdr.port != 0 );

   Socket myFd = openPort();

   sendTest( myFd, dest, testNum );
    
   char msg[udpMaxMessageLength];
   int msgLen = sizeof(msg)/sizeof(*msg);

   StunAddress from;
   getMessage( myFd,
               msg,
               &msgLen,
               &from.addr.v4addr,
               &from.addrHdr.port );
   clog << "Got a response" << endl;

   StunAddress mappedAddr;
   StunAddress changedAddr;
   UInt32 id;
   stunParseResponse( msg, 
                      msgLen,
                      &id, 
                      verbose,
                      &mappedAddr, 
                      &changedAddr);
   
   clog << "\t id=" << id << endl;
   clog << "\t mappedAddr=" << mappedAddr << endl;
   clog << "\t changedAddr=" << changedAddr << endl;
}


void stunGetTest( Socket myFd, 
		  StunAddress* mappedAddr, 
                  StunAddress* changedAddr,
                  UInt32* id,
                  bool verbose  )
{ 
   char msg[udpMaxMessageLength];
   int msgLen = sizeof(msg)/sizeof(*msg);

   StunAddress from;

   getMessage( myFd,
msg,
&msgLen,
&from.addr.v4addr,
&from.addrHdr.port );

   stunParseResponse( msg,
msgLen,
id,
verbose,
mappedAddr,
changedAddr );


}


 

StunNatType
stunType( StunAddress& dest, bool verbose, int portBase)
{ 
   assert( dest.addr.v4addr != 0 );
   assert( dest.addrHdr.port != 0 );
   
   StunAddress mappedAddr;
   StunAddress changedAddr;
   UInt32 id;
    
   Socket myFd = openPort(portBase);
    
   bool respTestI=false;
   bool isNat=true;
   StunAddress testIchangedAddr;
   StunAddress testImappedAddr;
   bool respTestI2=false; 
   bool mappedIpSame = true;
   StunAddress testI2mappedAddr;
   StunAddress testI2dest=dest;
   bool respTestII=false;
   bool respTestIII=false;

   sendTest( myFd, dest, 1 );
   int count=0;
   while ( count++ < 10 )
   {
      struct timeval tv;
      fd_set fdSet; int fdSetSize;
      FD_ZERO(&fdSet); fdSetSize=0;
      FD_SET(myFd,&fdSet); 
      fdSetSize = myFd+1;
      tv.tv_sec=0;
      tv.tv_usec=500*1000; // 200 ms 

      int  err = select(fdSetSize, &fdSet, NULL, NULL, &tv);
      int e = errno;
      if ( err == SOCKET_ERROR )
      {
         // error occured
         cerr << "Error " << e << " " << strerror(e) << " in select" << endl;
      }
      else if ( err == 0 )
      {
         // timeout occured 
         if (!respTestI ) 
         {
            sendTest( myFd, dest, 1 );
         }         
         if ((!respTestI2) && respTestI ) 
         {
            // check the address to send to if valid 
            if (  ( testI2dest.addr.v4addr != 0 ) &&
                  ( testI2dest.addrHdr.port != 0 ) )
            {
               sendTest( myFd, testI2dest, 10 );
            }
         }
         if (!respTestII )
         {
            sendTest( myFd, dest, 2 );
         }   
         if (!respTestIII )
         {
            sendTest( myFd, dest, 3 );
         }
      }
      else
      {
         //clog << "-----------------------------------------" << endl;
         assert( err>0 );
         // data is avialbe on some fd 
         if ( myFd!=INVALID_SOCKET ) if ( FD_ISSET(myFd,&fdSet) )
         {
            stunGetTest(myFd,
&mappedAddr,
&changedAddr,
&id,
verbose );
		  
            switch( id )
            {
               case 1:
                  testIchangedAddr = changedAddr;
                  testImappedAddr = mappedAddr;
                  respTestI=true;  
                  testI2dest.addr.v4addr = testIchangedAddr.addr.v4addr;
                  break;
               case 2:
                  respTestII=true;
                  break;
               case 3:
                  respTestIII=true;
                  break;
               case 10:
                  testI2mappedAddr = mappedAddr;
                  respTestI2=true;
                  mappedIpSame = false;
                  if ( (testI2mappedAddr.addr.v4addr  == testImappedAddr.addr.v4addr  ) &&
                       (testI2mappedAddr.addrHdr.port == testImappedAddr.addrHdr.port ))
                  { 
                     mappedIpSame = true;
                  }
			  
                  break;
            }
         }
      }
   }


      // see if we can bind to this address 
      //cerr << "try binding to " << testImappedAddr << endl;
      Socket s = openPort( 11000, testImappedAddr.addr.v4addr );
      if ( s != INVALID_SOCKET )
      {
         close(s);
         isNat = false;
         //cerr << "binding worked" << endl;
      }
      else
      {
         isNat = true;
         //cerr << "binding failed" << endl;
      }

      if (verbose)
      {
         clog << "test I = " << respTestI << endl;
         clog << "test II = " << respTestII << endl;
         clog << "test III = " << respTestIII << endl;
         clog << "test I(2) = " << respTestI2 << endl;
         clog << "is nat  = " << isNat <<endl;
         clog << "mapped IP same = " << mappedIpSame << endl;
      }
      // implement logic flow chart from draft RFC
      if ( respTestI )
   {
      if ( isNat )
      {
         if (respTestII)
         {
            return StunTypeConeNat;
         }
         else
         {
            if ( mappedIpSame )
            {
               if ( respTestIII )
               {
                  return StunTypeRestrictedNat;
               }
               else
               {
                  return StunTypePortRestrictedNat;
               }
            }
            else
            {
               return StunTypeSymNat;
            }
         }
      }
      else
      {
         if (respTestII)
         {
            return StunTypeOpen;
         }
         else
         {
            return StunTypeSymFirewall;
         }
      }
   }
   else
   {
      return StunTypeBlocked;
   }

   assert(0);
   return StunTypeUnknown;
}


// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:
