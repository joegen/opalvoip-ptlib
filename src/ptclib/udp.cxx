//
// stund/udp.cxx
// 
// Copyright (c) 2002 Alan Hawrylyshen, Jason Fischl
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
//
// A portion of this code is taken from lbproxy, a part of VOCAL.
//
// http://www.vovida.org/vocal/
//
// 

#include <cassert>
#include <cstdio>
#include <cstring>
#include <errno.h>
#include <iostream.h>
#include <stdlib.h>
#include <time.h>

#ifdef WIN32

#include <winsock2.h>
#include <stdlib.h>
#include <io.h>

#else

#include <arpa/inet.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>

#endif


#include "string.h"
#include "udp.h"


void
parseHostName( char* peerName,
               unsigned int* ip,
               unsigned short* portVal,
               unsigned int defaultPort )
{
   in_addr sin_addr;
    
   char host[512];
   strncpy(host,peerName,512);
   host[512-1]='\0';
   char* port = NULL;

   int portNum = defaultPort;
   
   // pull out the port part if present.
   char* sep = strchr(host,':');
   
   if ( sep == NULL )
   {
      portNum = defaultPort;
   }
   else
   {
      *sep = '\0';
      port = sep + 1;
      // set port part
      
      char* endPtr=NULL;
      
      portNum = strtol(port,&endPtr,0);
      
      if ( endPtr != NULL )
      {
         if ( *endPtr != '\0' )
         {
            portNum = defaultPort;
         }
      }
   }
    
   assert( portNum >= 1024 );
   assert( portNum <= 65000 );
   

    
   // figure out the host part 
   struct hostent* h;
   
   h = gethostbyname( host );
   if ( h == NULL )
   {
      *ip = ntohl( 0x7F000001L );
   }
   else
   {
      sin_addr = *(struct in_addr*)h->h_addr;
      *ip = ntohl( sin_addr.s_addr );
   }

   *portVal = portNum;
}


Socket
openPort( unsigned short port, unsigned int interfaceIp )
{
   Socket fd;
    
   fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
   if ( fd == INVALID_SOCKET )
   {
      int err = errno;
      cerr << "Could not create a UDP socket:" << err << endl;
      return INVALID_SOCKET;
   }
    
#if 0
   if ( port == 0 )
   {
      // just use an ephermieal port - no bind
      return fd;
   }
#endif
   
   struct sockaddr_in addr;
   memset((char*) &(addr),0, sizeof((addr)));
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(port);
    
   if ( (interfaceIp != 0) && 
       ( interfaceIp != 0x100007f ) )
   {
       addr.sin_addr.s_addr = htonl(interfaceIp);
   }
   
   if ( bind( fd,(struct sockaddr*)&addr, sizeof(addr)) != 0 )
   {
      int e = errno;
        
      switch (e)
      {
         case EADDRINUSE:
         {
            cerr << "Port " << port << " for receiving UDP is in use" << endl;
            return INVALID_SOCKET;
         }
         break;
         case EADDRNOTAVAIL:
         {
            cerr << "Cannot assign requested address" << endl;
            return INVALID_SOCKET;
         }
         break;
         default:
         {
            cerr << "Could not bind UDP receive port. Error=" << e << endl;
            return INVALID_SOCKET;
         }
         break;
      }
   }
   //clog << "Opened port " << port << " on fd " << fd << endl;

   assert( fd != INVALID_SOCKET  );

   return fd;
}


bool 
getMessage( Socket fd, char* buf, int* len,
	    unsigned int* srcIp, unsigned short* srcPort )
{
    assert( fd != INVALID_SOCKET );

   int originalSize = *len;

   struct sockaddr_in from;
   int fromLen = sizeof(from);

   *len = recvfrom(fd,
                   buf,
                   *len,
                   0,
                   (struct sockaddr *)&from,
                   (socklen_t *)&fromLen);

   if ( *len == SOCKET_ERROR )
   {
        int err = errno;

        switch (err)
        {
        case ENOTSOCK:
                cerr << "Error fd not a socket" <<   endl;
                break;
        default:
            cerr << "Error=" << err << endl;
        }

        return false;
   }

   if ( *len < 0 )
   {
     clog << "socket closed?" << endl;
     return false;
   }
    
   if ( *len == 0 )
   {
       clog << "socket closed?" << endl;
      return false;
   }
    
   *srcPort = ntohs(from.sin_port);
   *srcIp = ntohl(from.sin_addr.s_addr);

   if ( (*len)+1 >= originalSize )
   {
      cerr << "Received a message that was too large" << endl;
      return false;
   }
   buf[*len]=0;
    
   return true;
}


bool 
sendMessage( Socket fd, char* buf, int l, 
	     unsigned int dstIp, unsigned short dstPort )
{
   assert( fd != INVALID_SOCKET );
    
   int s;
   if ( dstPort == 0 )
   {
      // sending on a connected port 
      assert( dstIp == 0 );

      s = send(fd,buf,l,0);
   }
   else
   {
      assert( dstIp != 0 );
      assert( dstPort != 0 );
        
      struct sockaddr_in to;
      int toLen = sizeof(to);
      memset(&to,0,toLen);
        
     to.sin_family = AF_INET;
     to.sin_port = htons(dstPort);
      to.sin_addr.s_addr = htonl(dstIp);
        
      s = sendto(fd, buf, l, 0,(sockaddr*)&to, toLen);
   }
    
   if ( s == SOCKET_ERROR )
   {
      int e = errno;
      switch (e)
      {
         case ECONNREFUSED:
         case EHOSTDOWN:
         case EHOSTUNREACH:
         {
            // quietly ignore this 
         }
         break;
         case EAFNOSUPPORT:
         {
            cerr << "err EAFNOSUPPORT in send" << endl;
         }
         break;
         default:
         {
            cerr << "err " << e << " "  << strerror(e) << " in send" << endl;
         }
      }
      return false;
   }
    
   if ( s == 0 )
   {
      cerr << "no data sent in send" << endl;
      return false;
   }
    
   if ( s != l )
   {
      cerr << "only " << s << " out of " << l << " bytes sent" << endl;
      return false;
   }
    
   return true;
}


void
initNetwork()
{
#ifdef WIN32 
   WORD wVersionRequested = MAKEWORD( 2, 2 );
   WSADATA wsaData;
   int err;

   err = WSAStartup( wVersionRequested, &wsaData );
   if ( err != 0 ) 
   {
      // could not find a usable WinSock DLL
      cerr << "Could not load winsock" << endl;
      assert(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      exit(1);
   }
    
   /* Confirm that the WinSock DLL supports 2.2.*/
   /* Note that if the DLL supports versions greater    */
   /* than 2.2 in addition to 2.2, it will still return */
   /* 2.2 in wVersion since that is the version we      */
   /* requested.                                        */
    
   if ( LOBYTE( wsaData.wVersion ) != 2 ||
        HIBYTE( wsaData.wVersion ) != 2 ) 
   {
      /* Tell the user that we could not find a usable */
      /* WinSock DLL.                                  */
      WSACleanup( );
      cerr << "Bad winsock verion" << endl;
      assert(0); // is this is failing, try a different version that 2.2, 1.0 or later will likely work 
      exit(1);
   }    
#endif
}


// Local Variables:
// mode:c++
// c-file-style:"ellemtel"
// c-file-offsets:((case-label . +))
// indent-tabs-mode:nil
// End:
