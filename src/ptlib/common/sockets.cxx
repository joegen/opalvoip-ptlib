/*
 * sockets.cxx
 *
 * Berkley sockets classes.
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
 * $Log: sockets.cxx,v $
 * Revision 1.116  2002/01/07 05:37:32  robertj
 * Changed to allow for a service name that starts with a number.
 *
 * Revision 1.115  2002/01/02 04:55:31  craigs
 * Fixed problem when PSocket::GetPortByService called with a number
 * that is a substring of a valid service name
 *
 * Revision 1.114  2001/12/13 09:18:07  robertj
 * Added function to convert PString to IP address with error checking that can
 *   distinguish between 0.0.0.0 or 255.255.255.255 and illegal address.
 * Added ability to decode bracketed IP addresss [10.1.2.3] as host name.
 *
 * Revision 1.113  2001/09/14 08:00:38  robertj
 * Added new versions of Conenct() to allow binding to a specific local interface.
 *
 * Revision 1.112  2001/09/10 02:51:23  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.111  2001/06/30 06:59:07  yurik
 * Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
 *
 * Revision 1.110  2001/05/24 02:07:31  yurik
 * ::setsockopt on WinCE is now not called if option is not supported
 *
 * Revision 1.109  2001/05/23 19:48:55  yurik
 * Fix submitted by Dave Cassel, dcassel@cyberfone.com,
 * allowing a connection between a client and a gatekeeper.
 *
 * Revision 1.108  2001/03/20 06:44:25  robertj
 * Lots of changes to fix the problems with terminating threads that are I/O
 *   blocked, especially when doing orderly shutdown of service via SIGTERM.
 *
 * Revision 1.107  2001/03/05 04:18:27  robertj
 * Added net mask to interface info returned by GetInterfaceTable()
 *
 * Revision 1.106  2001/01/29 06:41:32  robertj
 * Added printing of entry of interface table.
 *
 * Revision 1.105  2001/01/28 01:15:01  yurik
 * WinCE port-related
 *
 * Revision 1.104  2001/01/24 06:32:17  yurik
 * Windows CE port-related changes
 *
 * Revision 1.103  2000/06/26 11:17:21  robertj
 * Nucleus++ port (incomplete).
 *
 * Revision 1.102  2000/06/21 01:01:22  robertj
 * AIX port, thanks Wolfgang Platzer (wolfgang.platzer@infonova.at).
 *
 * Revision 1.101  2000/05/02 08:14:40  craigs
 * Fixed problem with "memory leak" reporting under Unix
 *
 * Revision 1.100  2000/04/27 02:43:45  robertj
 * Fixed warning about signedness mismatch.
 *
 * Revision 1.99  2000/04/19 00:13:52  robertj
 * BeOS port changes.
 *
 * Revision 1.98  2000/02/18 09:55:21  robertj
 * Added parameter so get/setsockopt can have other levels to SOL_SOCKET.
 *
 * Revision 1.97  1999/10/27 01:21:44  robertj
 * Improved portability of copy from host_info struct to IP address.
 *
 * Revision 1.96  1999/08/30 02:21:03  robertj
 * Added ability to listen to specific interfaces for IP sockets.
 *
 * Revision 1.95  1999/08/27 08:18:52  robertj
 * Added ability to get the host/port of the the last packet read/written to UDP socket.
 *
 * Revision 1.94  1999/08/08 09:04:01  robertj
 * Added operator>> for PIPSocket::Address class.
 *
 * Revision 1.93  1999/07/11 13:42:13  craigs
 * pthreads support for Linux
 *
 * Revision 1.92  1999/06/01 08:04:35  robertj
 * Fixed mistake from previous fix.
 *
 * Revision 1.91  1999/06/01 07:39:23  robertj
 * Added retries to DNS lookup if get temporary error.
 *
 * Revision 1.90  1999/03/09 08:13:52  robertj
 * Fixed race condition in doing Select() on closed sockets. Could go into infinite wait.
 *
 * Revision 1.89  1999/03/02 05:41:58  robertj
 * More BeOS changes
 *
 * Revision 1.88  1999/02/26 04:10:39  robertj
 * More BeOS port changes
 *
 * Revision 1.87  1999/02/25 03:43:35  robertj
 * Fixed warning when PINDEX is unsigned.
 *
 * Revision 1.86  1999/02/23 07:19:22  robertj
 * Added [] operator PIPSocket::Address to get the bytes out of an IP address.
 *
 * Revision 1.85  1999/02/16 08:08:06  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.84  1999/01/08 01:29:47  robertj
 * Support for pthreads under FreeBSD
 *
 * Revision 1.83  1999/01/06 10:58:01  robertj
 * Fixed subtle mutex bug in returning string hostname from DNS cache.
 *
 * Revision 1.82  1998/12/22 10:25:01  robertj
 * Added clone() function to support SOCKS in FTP style protocols.
 * Fixed internal use of new operator in IP cache.
 *
 * Revision 1.81  1998/12/18 04:34:37  robertj
 * PPC Linux GNU C compatibility.
 *
 * Revision 1.80  1998/11/30 04:47:52  robertj
 * New directory structure
 *
 * Revision 1.79  1998/11/14 06:28:36  robertj
 * Changed senatics of os_sendto to return TRUE if ANY bytes are sent.
 *
 * Revision 1.78  1998/11/08 12:05:04  robertj
 * Fixed multiple thread access problem with DNS aliases array.
 *
 * Revision 1.77  1998/10/01 09:05:35  robertj
 * Added check that port number is between 1 and 65535.
 *
 * Revision 1.76  1998/09/23 06:22:44  robertj
 * Added open source copyright license.
 *
 * Revision 1.75  1998/08/31 13:00:34  robertj
 * Prevented dependency on snmpapi.dll for all ptlib apps.
 *
 * Revision 1.74  1998/08/27 00:58:42  robertj
 * Resolved signedness problems with various GNU libraries.
 *
 * Revision 1.73  1998/08/25 14:07:43  robertj
 * Added getprotobyxxx wrapper functions.
 *
 * Revision 1.72  1998/08/25 11:09:20  robertj
 * Fixed parsing of 802.x header on ethernet frames.
 * Changed DNS cache to not cache temporary lookup failures, only an authoratative 'no such host'.
 *
 * Revision 1.71  1998/08/21 05:26:10  robertj
 * Fixed bug where write streams out to non-stream socket.
 * Added ethernet socket.
 *
 * Revision 1.70  1998/05/07 05:20:25  robertj
 * Fixed DNS lookup so only works around bug in old Win95 and not OSR2
 *
 * Revision 1.69  1998/03/20 03:18:21  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.68  1998/03/05 12:45:48  robertj
 * DNS cache and NT bug fix attempts.
 *
 * Revision 1.67  1998/01/26 02:49:22  robertj
 * GNU support.
 *
 * Revision 1.66  1998/01/26 00:49:28  robertj
 * Fixed bug in detecting local host on NT, 95 bug kludge was interfering with it.
 *
 * Revision 1.65  1998/01/06 12:43:23  craigs
 * Added definition of REENTRANT_BUFFER_LEN
 *
 * Revision 1.64  1998/01/04 07:25:09  robertj
 * Added pthreads compatible calls for gethostbyx functions.
 *
 * Revision 1.63  1997/12/18 05:06:13  robertj
 * Moved IsLocalHost() to platform dependent code.
 *
 * Revision 1.62  1997/12/11 10:30:35  robertj
 * Added operators for IP address to DWORD conversions.
 *
 * Revision 1.61  1997/10/03 13:33:22  robertj
 * Added workaround for NT winsock bug with RAS and DNS lookups.
 *
 * Revision 1.60  1997/09/27 00:58:39  robertj
 * Fixed race condition on socket close in Select() function.
 *
 * Revision 1.59  1997/06/06 10:56:36  craigs
 * Added new functions for connectionless UDP writes
 *
 * Revision 1.58  1997/01/04 07:42:18  robertj
 * Fixed GCC Warnings.
 *
 * Revision 1.57  1997/01/04 06:54:38  robertj
 * Added missing canonical name to alias list.
 *
 * Revision 1.56  1996/12/17 11:07:05  robertj
 * Added clear of name cache.
 *
 * Revision 1.55  1996/12/12 09:23:27  robertj
 * Fixed name cache to cache missing names as well.
 * Fixed new connect with specific local port so can be re-used (simultaneous FTP session bug)
 *
 * Revision 1.54  1996/12/05 11:46:39  craigs
 * Fixed problem with Win95 recvfrom not having timeouts
 *
 * Revision 1.53  1996/11/30 12:08:17  robertj
 * Added Connect() variant so can set the local port number on link.
 *
 * Revision 1.52  1996/11/16 10:49:03  robertj
 * Fixed missing const in PIPSocket::Address stream output operator..
 *
 * Revision 1.51  1996/11/16 01:43:49  craigs
 * Fixed problem with ambiguous DNS cache keys
 *
 * Revision 1.50  1996/11/10 21:08:31  robertj
 * Added host name caching.
 *
 * Revision 1.49  1996/11/04 03:40:22  robertj
 * Moved address printer from inline to source.
 *
 * Revision 1.48  1996/10/26 01:41:09  robertj
 * Compensated for Win'95 gethostbyaddr bug.
 *
 * Revision 1.47  1996/09/14 13:09:40  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.46  1996/08/25 09:33:32  robertj
 * Added function to detect "local" host name.
 *
 * Revision 1.45  1996/07/30 12:24:53  robertj
 * Fixed incorrect conditional stopping Select() from working.
 *
 * Revision 1.44  1996/07/27 04:10:35  robertj
 * Changed Select() calls to return error codes.
 *
 * Revision 1.43  1996/06/10 09:58:21  robertj
 * Fixed win95 compatibility with looking up zero address (got a response and shouldn't).
 *
 * Revision 1.42  1996/05/26 03:47:03  robertj
 * Compatibility to GNU 2.7.x
 *
 * Revision 1.39  1996/04/29 12:20:01  robertj
 * Fixed GetHostAliases() so doesn't overwrite names with IP numbers.
 *
 * Revision 1.38  1996/04/15 10:59:41  robertj
 * Opened socket on UDP sockets so ReadFrom/WriteTo work when no Connect/Listen.
 *
 * Revision 1.37  1996/03/31 09:06:41  robertj
 * Added socket shutdown function.
 *
 * Revision 1.35  1996/03/18 13:33:18  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
 *
 * Revision 1.34  1996/03/17 05:51:18  robertj
 * Fixed strange bug in accept cant have NULL address.
 *
 * Revision 1.33  1996/03/16 04:52:20  robertj
 * Changed all the get host name and get host address functions to be more consistent.
 *
 * Revision 1.32  1996/03/04 12:21:00  robertj
 * Split file into telnet.cxx
 *
 * Revision 1.31  1996/03/03 07:38:45  robertj
 * Added Reusability clause to the Listen() function on sockets.
 *
 * Revision 1.30  1996/03/02 03:25:13  robertj
 * Added Capability to get and set Berkeley socket options.
 *
 * Revision 1.29  1996/02/25 11:30:08  robertj
 * Changed Listen so can do a listen on a socket that is connected.
 *
 * Revision 1.28  1996/02/25 03:10:55  robertj
 * Moved some socket functions to platform dependent code.
 *
 * Revision 1.27  1996/02/19 13:30:15  robertj
 * Fixed bug in getting port by service name when specifying service by string number.
 * Added SO_LINGER option to socket to stop data loss on close.
 *
 * Revision 1.26  1996/02/15 14:46:44  robertj
 * Added Select() function to PSocket.
 *
 * Revision 1.25  1996/02/13 13:08:09  robertj
 * Fixed usage of sock_addr structure, not being cleared correctly.
 *
 * Revision 1.24  1996/02/08 12:27:22  robertj
 * Added function to get peer port as well as IP number..
 *
 * Revision 1.23  1996/02/03 11:07:37  robertj
 * Fixed buf in assuring error when converting string to IP number and string is empty.
 *
 * Revision 1.22  1996/01/28 14:08:13  robertj
 * Changed service parameter to PString for ease of use in GetPortByService function
 * Fixed up comments.
 * Added default value in string for service name.
 *
 * Revision 1.21  1996/01/23 13:19:13  robertj
 * Moved Accept() function to platform dependent code.
 *
 * Revision 1.20  1995/12/23 03:42:53  robertj
 * Unix portability issues.
 *
 * Revision 1.19  1995/12/10 11:42:23  robertj
 * Numerous fixes for sockets.
 *
 * Revision 1.18  1995/10/14 15:11:31  robertj
 * Added internet address to string conversion functionality.
 *
 * Revision 1.17  1995/07/02 01:21:23  robertj
 * Added static functions to get the current host name/address.
 *
 * Revision 1.16  1995/06/17 00:47:01  robertj
 * Changed overloaded Open() calls to 3 separate function names.
 * More logical design of port numbers and service names.
 *
 * Revision 1.15  1995/06/04 12:45:33  robertj
 * Added application layer protocol sockets.
 * Slight redesign of port numbers on sockets.
 *
 * Revision 1.14  1995/04/25 11:12:44  robertj
 * Fixed functions hiding ancestor virtuals.
 *
 * Revision 1.13  1995/04/01 08:31:54  robertj
 * Finally got a working TELNET.
 *
 * Revision 1.12  1995/03/18 06:27:49  robertj
 * Rewrite of telnet socket protocol according to RFC1143.
 *
 * Revision 1.11  1995/03/12  04:46:29  robertj
 * Added more functionality.
 *
 * Revision 1.10  1995/02/21  11:25:29  robertj
 * Further implementation of telnet socket, feature complete now.
 *
 * Revision 1.9  1995/01/27  11:16:16  robertj
 * Fixed missing cast in function, required by some platforms.
 *
 * Revision 1.8  1995/01/15  04:55:47  robertj
 * Moved all Berkley socket functions inside #ifdef.
 *
 * Revision 1.7  1995/01/04  10:57:08  robertj
 * Changed for HPUX and GNU2.6.x
 *
 * Revision 1.6  1995/01/03  09:37:52  robertj
 * Added constructor to open TCP socket.
 *
 * Revision 1.5  1995/01/02  12:28:25  robertj
 * Documentation.
 * Added more socket functions.
 *
 * Revision 1.4  1995/01/01  01:06:58  robertj
 * More implementation.
 *
 * Revision 1.3  1994/11/28  12:38:49  robertj
 * Added DONT and WONT states.
 *
 * Revision 1.2  1994/08/21  23:43:02  robertj
 * Some implementation.
 *
 * Revision 1.1  1994/08/01  03:39:05  robertj
 * Initial revision
 *
 */

#ifdef __NUCLEUS_PLUS__
#include <ConfigurationClass.h>
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>

#include <ctype.h>

#if (defined(_WIN32) || defined(WINDOWS)) && !defined(__NUCLEUS_MNT__)
static PWinSock dummyForWinSock; // Assure winsock is initialised
#endif

#if (defined(P_PTHREADS) && !defined(P_THREAD_SAFE_CLIB)) || defined(__NUCLEUS_PLUS__)
#define REENTRANT_BUFFER_LEN 1024
#endif


class PIPCacheData : public PObject
{
  PCLASSINFO(PIPCacheData, PObject)
  public:
    PIPCacheData(struct hostent * ent, const char * original);
    const PString & GetHostName() const { return hostname; }
    const PIPSocket::Address & GetHostAddress() const { return address; }
    const PStringList & GetHostAliases() const { return aliases; }
    BOOL HasAged() const;
  private:
    PString            hostname;
    PIPSocket::Address address;
    PStringList        aliases;
    PTime              birthDate;
};


PDICTIONARY(PHostByName_private, PCaselessString, PIPCacheData);

class PHostByName : PHostByName_private
{
  public:
    BOOL GetHostName(const PString & name, PString & hostname);
    BOOL GetHostAddress(const PString & name, PIPSocket::Address & address);
    BOOL GetHostAliases(const PString & name, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PString & name);
    PMutex mutex;
  friend void PIPSocket::ClearNameCache();
};

static PHostByName & pHostByName()
{
  static PHostByName t;
  return t;
}

class PIPCacheKey : public PObject
{
  PCLASSINFO(PIPCacheKey, PObject)
  public:
    PIPCacheKey(const PIPSocket::Address & a)
      { addr = a; }

    PObject * Clone() const
      { return new PIPCacheKey(*this); }

    PINDEX HashFunction() const
      { return (addr.Byte2() + addr.Byte3() + addr.Byte4())%41; }

  private:
    PIPSocket::Address addr;
};

PDICTIONARY(PHostByAddr_private, PIPCacheKey, PIPCacheData);

class PHostByAddr : PHostByAddr_private
{
  public:
    BOOL GetHostName(const PIPSocket::Address & addr, PString & hostname);
    BOOL GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address);
    BOOL GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PIPSocket::Address & addr);
    PMutex mutex;
  friend void PIPSocket::ClearNameCache();
};

static PHostByAddr & pHostByAddr()
{
  static PHostByAddr t;
  return t;
}

#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// IP Caching

PIPCacheData::PIPCacheData(struct hostent * host_info, const char * original)
{
  if (host_info == NULL) {
    address.s_addr = 0;
    return;
  }

  hostname = host_info->h_name;
#ifndef _WIN32_WCE
  address = *(DWORD *)host_info->h_addr;
#else
  if( host_info->h_addr )
	  address = PIPSocket::Address(
		  (BYTE) host_info->h_addr[0],
		  (BYTE) host_info->h_addr[1],
		  (BYTE) host_info->h_addr[2],
		  (BYTE) host_info->h_addr[3]);

#endif
  aliases.AppendString(host_info->h_name);

  PINDEX i;
  for (i = 0; host_info->h_aliases[i] != NULL; i++)
    aliases.AppendString(host_info->h_aliases[i]);

  for (i = 0; host_info->h_addr_list[i] != NULL; i++)
    aliases.AppendString(inet_ntoa(*(struct in_addr *)host_info->h_addr_list[i]));

  for (i = 0; i < aliases.GetSize(); i++)
    if (aliases[i] *= original)
      return;

  aliases.AppendString(original);
}


static PTimeInterval GetConfigTime(const char * key, DWORD dflt)
{
  PConfig cfg("DNS Cache");
  return cfg.GetInteger(key, dflt);
}

BOOL PIPCacheData::HasAged() const
{
  static PTimeInterval retirement = GetConfigTime("Age Limit", 300000); // 5 minutes
  PTime now;
  PTimeInterval age = now - birthDate;
  return age > retirement;
}


BOOL PHostByName::GetHostName(const PString & name, PString & hostname)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL) {
    hostname = host->GetHostName();
    hostname.MakeUnique();
  }

  mutex.Signal();

  return host != NULL;
}


BOOL PHostByName::GetHostAddress(const PString & name, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();

  return host != NULL;
}


BOOL PHostByName::GetHostAliases(const PString & name, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL) {
    const PStringList & a = host->GetHostAliases();
    aliases.SetSize(a.GetSize());
    for (PINDEX i = 0; i < a.GetSize(); i++)
      aliases[i] = a[i];
  }

  mutex.Signal();
  return host != NULL;
}


PIPCacheData * PHostByName::GetHost(const PString & name)
{
  mutex.Wait();

  PCaselessString key = name;
  PIPCacheData * host = GetAt(key);

  if (host != NULL && host->HasAged()) {
    SetAt(key, NULL);
    host = NULL;
  }

  if (host == NULL) {
    mutex.Signal();

#ifdef P_AIX
    struct hostent_data ht_data;
    memset(&ht_data, 0, sizeof(ht_data)); 
    struct hostent host_info;
#else
    struct hostent * host_info;
#endif

    int retry = 3;
    do {
#if ( ( defined(P_PTHREADS) && !defined(P_THREAD_SAFE_CLIB) ) || (defined(__NUCLEUS_PLUS__) ) )
      // this function should really be a static on PIPSocket, but this would
      // require allocating thread-local storage for the data and that's too much
      // of a pain!


#ifndef P_AIX	// that I get no warnings
      int localErrNo;
      char buffer[REENTRANT_BUFFER_LEN];
      struct hostent hostEnt;
#endif

#ifdef P_LINUX
      ::gethostbyname_r(name,
                        &hostEnt,
                        buffer, REENTRANT_BUFFER_LEN,
                        &host_info,
      		        &localErrNo);
      		      		        
      		        
#elif defined P_AIX
      ::gethostbyname_r(name,
                        &host_info,
                        &ht_data);		    
#else
      host_info = ::gethostbyname_r(name,
			 &hostEnt, buffer, REENTRANT_BUFFER_LEN,
			 &localErrNo);
#endif

#else
      host_info = ::gethostbyname(name);
#endif
    } while (h_errno == TRY_AGAIN && --retry > 0);

    mutex.Wait();

    if (retry == 0)
      return NULL;

#ifdef P_AIX
    host = new PIPCacheData (&host_info, (const char*) name);
#else
    host = new PIPCacheData(host_info, name);
#endif

    SetAt(key, host);
  }

  if (host->GetHostAddress() == 0)
    return NULL;

  return host;
}


BOOL PHostByAddr::GetHostName(const PIPSocket::Address & addr, PString & hostname)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL) {
    hostname = host->GetHostName();
    hostname.MakeUnique();
  }

  mutex.Signal();
  return host != NULL;
}


BOOL PHostByAddr::GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();
  return host != NULL;
}


BOOL PHostByAddr::GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL) {
    const PStringList & a = host->GetHostAliases();
    aliases.SetSize(a.GetSize());
    for (PINDEX i = 0; i < a.GetSize(); i++)
      aliases[i] = a[i];
  }

  mutex.Signal();
  return host != NULL;
}

PIPCacheData * PHostByAddr::GetHost(const PIPSocket::Address & addr)
{
  mutex.Wait();

  PIPCacheKey key = addr;
  PIPCacheData * host = GetAt(key);

  if (host != NULL && host->HasAged()) {
    SetAt(key, NULL);
    host = NULL;
  }

  if (host == NULL) {
    mutex.Signal();

#ifdef P_AIX
    struct hostent_data ht_data;
    struct hostent host_info;
#else    
    struct hostent * host_info;
#endif

    int retry = 3;
    do {
#if ( ( defined(P_PTHREADS) && !defined(P_THREAD_SAFE_CLIB) ) || ( defined(__NUCLEUS_PLUS__) ) )
      // this function should really be a static on PIPSocket, but this would
      // require allocating thread-local storage for the data and that's too much
      // of a pain!
      
#ifndef P_AIX	// that I get no warnings
      int localErrNo;
      char buffer[REENTRANT_BUFFER_LEN];
      struct hostent hostEnt;
#endif

#ifdef P_LINUX
      ::gethostbyaddr_r((const char *)&addr, sizeof(addr),
                        PF_INET, 
                        &hostEnt,
                        buffer, REENTRANT_BUFFER_LEN,
                        &host_info,
                        &localErrNo);
#elif P_AIX
      ::gethostbyaddr_r((char *)&addr, sizeof(addr),
                        PF_INET, 
                        &host_info,
                        &ht_data );
#else
      host_info = ::gethostbyaddr_r((const char *)&addr, sizeof(addr), PF_INET, 
                                    &hostEnt, buffer, REENTRANT_BUFFER_LEN, &localErrNo);
#endif

#else
      host_info = ::gethostbyaddr((const char *)&addr, sizeof(addr), PF_INET);
#if defined(_WIN32) || defined(WINDOWS)  // Kludge to avoid strange 95 bug
      extern P_IsOldWin95();
      if (P_IsOldWin95() && host_info != NULL && host_info->h_addr_list[0] != NULL)
        host_info->h_addr_list[1] = NULL;
#endif
#endif
    } while (h_errno == TRY_AGAIN && --retry > 0);

    mutex.Wait();

    if (retry == 0)
      return FALSE;

#ifdef P_AIX
    host = new PIPCacheData(&host_info, inet_ntoa(addr));
#else
    host = new PIPCacheData(host_info, inet_ntoa(addr));
#endif

    SetAt(key, host);
  }

  if (host->GetHostAddress() == 0)
    return NULL;

  return host;
}


//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::PSocket()
{
  port = 0;
}


BOOL PSocket::Connect(const PString &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::Accept(PSocket &)
{
  PAssertAlways("Illegal operation.");
  return FALSE;
}


BOOL PSocket::SetOption(int option, int value, int level)
{
#ifdef _WIN32_WCE
  if(option == SO_RCVBUF || option == SO_SNDBUF || option == IP_TOS)
    return TRUE;
#endif
  return ConvertOSError(::setsockopt(os_handle, level, option,
                                     (char *)&value, sizeof(value)));
}


BOOL PSocket::SetOption(int option, const void * valuePtr, PINDEX valueSize, int level)
{
  return ConvertOSError(::setsockopt(os_handle, level, option,
                                     (char *)valuePtr, valueSize));
}


BOOL PSocket::GetOption(int option, int & value, int level)
{
#ifdef BE_BONELESS
  return FALSE;
#else
  socklen_t valSize = sizeof(value);
  return ConvertOSError(::getsockopt(os_handle, level, option,
                                     (char *)&value, &valSize));
#endif
}


BOOL PSocket::GetOption(int option, void * valuePtr, PINDEX valueSize, int level)
{
#ifdef BE_BONELESS
  return FALSE;
#else
  return ConvertOSError(::getsockopt(os_handle, level, option,
                                     (char *)valuePtr, (socklen_t *)&valueSize));
#endif
}


BOOL PSocket::Shutdown(ShutdownValue value)
{
  return ConvertOSError(::shutdown(os_handle, value));
}


WORD PSocket::GetProtocolByName(const PString & name)
{
#if !defined(BE_BONELESS) && !defined(__NUCLEUS_PLUS__) && !defined(_WIN32_WCE)
  struct protoent * ent = getprotobyname(name);
  if (ent != NULL)
    return ent->p_proto;
#endif

  return 0;
}


PString PSocket::GetNameByProtocol(WORD proto)
{
#if !defined(BE_BONELESS) && !defined(__NUCLEUS_PLUS__) && !defined(_WIN32_WCE)
  struct protoent * ent = getprotobynumber(proto);
  if (ent != NULL)
    return ent->p_name;
#endif

  return psprintf("%u", proto);
}


WORD PSocket::GetPortByService(const PString & serviceName) const
{
  return GetPortByService(GetProtocolName(), serviceName);
}


WORD PSocket::GetPortByService(const char * protocol, const PString & service)
{
  // if the string is a valid integer, then use integer value
  // this avoids stupid problems like operating systems that match service
  // names to substrings (like "2000" to "taskmaster2000")
  if (strspn(service, "0123456789") == strlen(service))
    return (WORD)service.AsUnsigned();

#if defined( __NUCLEUS_PLUS__ )
  PAssertAlways("PSocket::GetPortByService: problem as no ::getservbyname in Nucleus NET");
  return 0;
#elif defined(_WIN32_WCE)
  PAssertAlways("PSocket::GetPortByService: problem for WindowsCE as no port given.");
  return 0;
#else
  PINDEX space = service.FindOneOf(" \t\r\n");
  struct servent * serv = ::getservbyname(service(0, space-1), protocol);
  if (serv != NULL)
    return ntohs(serv->s_port);

  long portNum;
  if (space != P_MAX_INDEX)
    portNum = atol(service(space+1, P_MAX_INDEX));
  else if (isdigit(service[0]))
    portNum = atoi(service);
  else
    portNum = -1;

  if (portNum < 0 || portNum > 65535)
    return 0;

  return (WORD)portNum;
#endif
}


PString PSocket::GetServiceByPort(WORD port) const
{
  return GetServiceByPort(GetProtocolName(), port);
}


PString PSocket::GetServiceByPort(const char * protocol, WORD port)
{
#if !defined(BE_BONELESS) && !defined(__NUCLEUS_PLUS__) && !defined(_WIN32_WCE)
  struct servent * serv = ::getservbyport(htons(port), protocol);
  if (serv != NULL)
    return PString(serv->s_name);
  else
#endif
    return PString(PString::Unsigned, port);
}


void PSocket::SetPort(WORD newPort)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = newPort;
}


void PSocket::SetPort(const PString & service)
{
  PAssert(!IsOpen(), "Cannot change port number of opened socket");
  port = GetPortByService(service);
}


WORD PSocket::GetPort() const
{
  return port;
}


PString PSocket::GetService() const
{
  return GetServiceByPort(port);
}


int PSocket::Select(PSocket & sock1, PSocket & sock2)
{
  return Select(sock1, sock2, PMaxTimeInterval);
}


int PSocket::Select(PSocket & sock1,
                    PSocket & sock2,
                    const PTimeInterval & timeout)
{
  if (!sock1.IsOpen() || !sock2.IsOpen())
    return NotOpen;

  int h1 = sock1.GetHandle();
  int h2 = sock2.GetHandle();

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
  fd_set readfds;
  FD_ZERO(&readfds);
  FD_SET(h1, &readfds);
  FD_SET(h2, &readfds);
  fd_set writefds;
  FD_ZERO(&writefds);
  fd_set exceptfds;
  FD_ZERO(&exceptfds);
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

  PIntArray allfds(4);
  allfds[0] = h1;
  allfds[1] = 1;
  allfds[2] = h2;
  allfds[3] = 1;
  int rval = os_select(PMAX(h1, h2)+1,
                                readfds, writefds, exceptfds, allfds, timeout);

  Errors lastError;
  int osError;
  if (!ConvertOSError(rval, lastError, osError))
    return lastError;

  rval = 0;
  if (FD_ISSET(h1, &readfds))
    rval -= 1;
  if (FD_ISSET(h2, &readfds))
    rval -= 2;

  return rval;
}


PChannel::Errors PSocket::Select(SelectList & read)
{
  SelectList dummy1, dummy2;
  return Select(read, dummy1, dummy2, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read, const PTimeInterval & timeout)
{
  SelectList dummy1, dummy2;
  return Select(read, dummy1, dummy2, timeout);
}


PChannel::Errors PSocket::Select(SelectList & read, SelectList & write)
{
  SelectList dummy1;
  return Select(read, write, dummy1, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 const PTimeInterval & timeout)
{
  SelectList dummy1;
  return Select(read, write, dummy1, timeout);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except)
{
  return Select(read, write, except, PMaxTimeInterval);
}


PChannel::Errors PSocket::Select(SelectList & read,
                                 SelectList & write,
                                 SelectList & except,
                                 const PTimeInterval & timeout)
{
  int maxfds = 0;
  PINDEX nextfd = 0;
  PIntArray allfds(2*(read.GetSize()+write.GetSize()+except.GetSize()));

#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
  fd_set readfds;
  FD_ZERO(&readfds);
  PINDEX i;
  for (i = 0; i < read.GetSize(); i++) {
    if (!read[i].IsOpen())
      return NotOpen;
    int h = read[i].GetHandle();
    FD_SET(h, &readfds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 1;
  }

  fd_set writefds;
  FD_ZERO(&writefds);
  for (i = 0; i < write.GetSize(); i++) {
    if (!write[i].IsOpen())
      return NotOpen;
    int h = write[i].GetHandle();
    FD_SET(h, &writefds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 2;
  }

  fd_set exceptfds;
  FD_ZERO(&exceptfds);
  for (i = 0; i < except.GetSize(); i++) {
    if (!except[i].IsOpen())
      return NotOpen;
    int h = except[i].GetHandle();
    FD_SET(h, &exceptfds);
    if (h > maxfds)
      maxfds = h;
    allfds[nextfd++] = h;
    allfds[nextfd++] = 4;
  }
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

  int retval = os_select(maxfds+1,readfds,writefds,exceptfds,allfds,timeout);

  Errors lastError;
  int osError;
  if (!ConvertOSError(retval, lastError, osError))
    return lastError;

  if (retval > 0) {
    for (i = 0; i < read.GetSize(); i++) {
      int h = read[i].GetHandle();
      if (h < 0)
        return Interrupted;
      if (!FD_ISSET(h, &readfds))
        read.RemoveAt(i--);
    }
    for (i = 0; i < write.GetSize(); i++) {
      int h = write[i].GetHandle();
      if (h < 0)
        return Interrupted;
      if (!FD_ISSET(h, &writefds))
        write.RemoveAt(i--);
    }
    for (i = 0; i < except.GetSize(); i++) {
      int h = except[i].GetHandle();
      if (h < 0)
        return Interrupted;
      if (!FD_ISSET(h, &exceptfds))
        except.RemoveAt(i--);
    }
  }
  else {
    read.RemoveAll();
    write.RemoveAll();
    except.RemoveAll();
  }

  return NoError;
}



//////////////////////////////////////////////////////////////////////////////
// PIPSocket

PIPSocket::PIPSocket()
{
}


void PIPSocket::ClearNameCache()
{
  pHostByName().mutex.Wait();
  pHostByAddr().mutex.Wait();
  pHostByName().RemoveAll();
  pHostByAddr().RemoveAll();
#if (defined(_WIN32) || defined(WINDOWS)) && !defined(__NUCLEUS_MNT__) // Kludge to avoid strange NT bug
  static PTimeInterval delay = GetConfigTime("NT Bug Delay", 0);
  if (delay != 0) {
    ::Sleep(delay.GetInterval());
    ::gethostbyname("www.microsoft.com");
  }
#endif
  pHostByName().mutex.Signal();
  pHostByAddr().mutex.Signal();
}


PString PIPSocket::GetName() const
{
  PString name;
  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (getpeername(os_handle, (struct sockaddr *)&address, &size) == 0)
    name = GetHostName(address.sin_addr) + psprintf(":%u", ntohs(address.sin_port));
  return name;
}


PString PIPSocket::GetHostName()
{
  char name[100];
  if (gethostname(name, sizeof(name)-1) != 0)
    return "localhost";
  name[sizeof(name)-1] = '\0';
  return name;
}


PString PIPSocket::GetHostName(const PString & hostname)
{
  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp = hostname;
  if (temp != 0)
    return GetHostName(temp);

  PString canonicalname;
  if (pHostByName().GetHostName(hostname, canonicalname))
    return canonicalname;

  return hostname;
}


PString PIPSocket::GetHostName(const Address & addr)
{
  if (addr == 0)
    return addr;

  PString hostname;
  if (pHostByAddr().GetHostName(addr, hostname))
    return hostname;

  return addr;
}


BOOL PIPSocket::GetHostAddress(Address & addr)
{
  return pHostByName().GetHostAddress(GetHostName(), addr);
}


BOOL PIPSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  if (hostname.IsEmpty())
    return FALSE;

  // Check for special case of "[ipaddr]"
  if (hostname[0] == '[') {
    PINDEX end = hostname.Find(']');
    if (end != P_MAX_INDEX) {
      if (addr.FromString(hostname(1, end-1)))
        return TRUE;
    }
  }

  // Assuming it is a "." address and return if so
  if (addr.FromString(hostname))
    return TRUE;

  // otherwise lookup the name as a host name
  return pHostByName().GetHostAddress(hostname, addr);
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address addr = hostname;
  if (addr != 0)
    pHostByAddr().GetHostAliases(addr, aliases);
  else
    pHostByName().GetHostAliases(hostname, aliases);

  return aliases;
}


PStringArray PIPSocket::GetHostAliases(const Address & addr)
{
  PStringArray aliases;

  pHostByAddr().GetHostAliases(addr, aliases);

  return aliases;
}


BOOL PIPSocket::GetLocalAddress(Address & addr)
{
  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (!ConvertOSError(::getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetLocalAddress(Address & addr, WORD & portNum)
{
  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (!ConvertOSError(::getsockname(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  portNum = ntohs(address.sin_port);
  return TRUE;
}
 
BOOL PIPSocket::GetPeerAddress(Address & addr)
{
  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (!ConvertOSError(::getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  return TRUE;
}


BOOL PIPSocket::GetPeerAddress(Address & addr, WORD & portNum)
{
  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (!ConvertOSError(::getpeername(os_handle,(struct sockaddr*)&address,&size)))
    return FALSE;

  addr = address.sin_addr;
  portNum = ntohs(address.sin_port);
  return TRUE;
}


PString PIPSocket::GetLocalHostName()
{
  PString name;

  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (ConvertOSError(::getsockname(os_handle, (struct sockaddr *)&address, &size)))
    name = GetHostName(address.sin_addr);

  return name;
}


PString PIPSocket::GetPeerHostName()
{
  PString name;

  sockaddr_in address;
  socklen_t size = sizeof(address);
  if (ConvertOSError(::getpeername(os_handle, (struct sockaddr *)&address, &size)))
    name = GetHostName(address.sin_addr);

  return name;
}


BOOL PIPSocket::Connect(const PString & host)
{
  Address ipnum;
  if (GetHostAddress(host, ipnum))
    return Connect(INADDR_ANY, 0, ipnum);
  return FALSE;
}


BOOL PIPSocket::Connect(const Address & addr)
{
  return Connect(INADDR_ANY, 0, addr);
}


BOOL PIPSocket::Connect(WORD localPort, const Address & addr)
{
  return Connect(INADDR_ANY, localPort, addr);
}


BOOL PIPSocket::Connect(const Address & iface, const Address & addr)
{
  return Connect(iface, 0, addr);
}


BOOL PIPSocket::Connect(const Address & iface, WORD localPort, const Address & addr)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  // attempt to create a socket
  if (!OpenSocket())
    return FALSE;

  // attempt to connect
  sockaddr_in sin;
  if (localPort != 0 || iface != INADDR_ANY) {
    if (!SetOption(SO_REUSEADDR, 1)) {
      os_close();
      return FALSE;
    }
    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = iface;
    sin.sin_port        = htons(localPort);       // set the port
    if (!ConvertOSError(::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin)))) {
      os_close();
      return FALSE;
    }
  }

  memset(&sin, 0, sizeof(sin));
  sin.sin_family = AF_INET;
  sin.sin_port   = htons(port);  // set the port
  sin.sin_addr   = addr;
  if (ConvertOSError(os_connect((struct sockaddr *)&sin, sizeof(sin))))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PIPSocket::Listen(unsigned queueSize, WORD newPort, Reusability reuse)
{
  return Listen(INADDR_ANY, queueSize, newPort, reuse);
}


BOOL PIPSocket::Listen(const Address & bindAddr,
                       unsigned,
                       WORD newPort,
                       Reusability reuse)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  // close the port if it is already open
  if (!IsOpen()) {
    // attempt to create a socket
    if (!OpenSocket())
      return FALSE;
  }

  // attempt to listen
  if (SetOption(SO_REUSEADDR, reuse == CanReuseAddress ? 1 : 0)) {
    // attempt to listen
    sockaddr_in sin;
    memset(&sin, 0, sizeof(sin));
    sin.sin_family      = AF_INET;
    sin.sin_addr.s_addr = bindAddr;
    sin.sin_port        = htons(port);       // set the port

#ifdef __NUCLEUS_NET__
    int bind_result;
    if (port == 0)
      bind_result = ::bindzero(os_handle, (struct sockaddr*)&sin, sizeof(sin));
    else
      bind_result = ::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin));
    if (ConvertOSError(bind_result)) {
#else
    if (ConvertOSError(::bind(os_handle, (struct sockaddr*)&sin, sizeof(sin)))) {
#endif
      socklen_t size = sizeof(sin);
      if (ConvertOSError(::getsockname(os_handle, (struct sockaddr*)&sin, &size))) {
        port = ntohs(sin.sin_port);
        return TRUE;
      }
    }
  }

  os_close();
  return FALSE;
}


PIPSocket::Address::Address()
{
  s_addr = inet_addr("127.0.0.1");
}


PIPSocket::Address::Address(const in_addr & addr)
{
  s_addr = addr.s_addr;
}


PIPSocket::Address::Address(const Address & addr)
{
  s_addr = addr.s_addr;
}


PIPSocket::Address::Address(const PString & dotNotation)
{
  operator=(dotNotation);
}


#ifdef __NUCLEUS_NET__
PIPSocket::Address::Address(const struct id_struct & addr)
{
  operator=(addr);
}


PIPSocket::Address & PIPSocket::Address::operator=(const struct id_struct & addr)
{
  s_addr = (((unsigned long)addr.is_ip_addrs[0])<<24) +
           (((unsigned long)addr.is_ip_addrs[1])<<16) +
           (((unsigned long)addr.is_ip_addrs[2])<<8) +
           (((unsigned long)addr.is_ip_addrs[3]));
  return *this;
}
#endif


PIPSocket::Address & PIPSocket::Address::operator=(const in_addr & addr)
{
  s_addr = addr.s_addr;
  return *this;
}


PIPSocket::Address & PIPSocket::Address::operator=(const Address & addr)
{
  s_addr = addr.s_addr;
  return *this;
}


PIPSocket::Address & PIPSocket::Address::operator=(const PString & dotNotation)
{
  if (::strspn(dotNotation, "0123456789.") < ::strlen(dotNotation))
    s_addr = 0;
  else {
    s_addr = inet_addr((const char *)dotNotation);
    if (s_addr == (DWORD)INADDR_NONE)
      s_addr = 0;
  }
  return *this;
}


PString PIPSocket::Address::AsString() const
{
  return inet_ntoa(*this);
}


BOOL PIPSocket::Address::FromString(const PString & dotNotation)
{
  if (::strspn(dotNotation, "0123456789.") < ::strlen(dotNotation))
    return FALSE;

  PINDEX dot1 = dotNotation.Find('.');
  if (dot1 == P_MAX_INDEX)
    return FALSE;

  PINDEX dot2 = dotNotation.Find('.', dot1+1);
  if (dot2 == P_MAX_INDEX)
    return FALSE;

  PINDEX dot3 = dotNotation.Find('.', dot2+1);
  if (dot3 == P_MAX_INDEX)
    return FALSE;

  unsigned b1 = dotNotation(0, dot1-1).AsUnsigned();
  if (b1 > 255)
    return FALSE;

  unsigned b2 = dotNotation(dot1+1, dot2-1).AsUnsigned();
  if (b2 > 255)
    return FALSE;

  unsigned b3 = dotNotation(dot2+1, dot3-1).AsUnsigned();
  if (b3 > 255)
    return FALSE;

  unsigned b4 = dotNotation.Mid(dot3+1).AsUnsigned();
  if (b4 > 255)
    return FALSE;

  *this = PIPSocket::Address((BYTE)b1, (BYTE)b2, (BYTE)b3, (BYTE)b4);
  return TRUE;
}


PIPSocket::Address::operator PString() const
{
  return inet_ntoa(*this);
}


BYTE PIPSocket::Address::operator[](PINDEX idx) const
{
  PASSERTINDEX(idx);
  PAssert(idx <= 3, PInvalidParameter);
  return ((BYTE *)this)[idx];
}


ostream & operator<<(ostream & s, const PIPSocket::Address & a)
{
  return s << inet_ntoa(a);
}


istream & operator>>(istream & s, PIPSocket::Address & a)
{
  char dot1, dot2, dot3;
  unsigned b1, b2, b3, b4;
  s >> b1 >> dot1 >> b2 >> dot2 >> b3 >> dot3 >> b4;
  if (!s && (dot1 != '.' || dot2 != '.' || dot3 != '.'))
    s.clear(ios::failbit);

  a = PIPSocket::Address((BYTE)b1, (BYTE)b2, (BYTE)b3, (BYTE)b4);
  return s;
}


PIPSocket::InterfaceEntry::InterfaceEntry(const PString & _name,
                                          const Address & _addr,
                                          const Address & _mask,
                                          const PString & _macAddr)
  : name(_name.Trim()),
    ipAddr(_addr),
    netMask(_mask),
    macAddr(_macAddr)
{
}


void PIPSocket::InterfaceEntry::PrintOn(ostream & strm) const
{
  strm << ipAddr;
  if (!macAddr)
    strm << " <" << macAddr << '>';
  if (!name)
    strm << " (" << name << ')';
}


#ifdef __NUCLEUS_NET__
BOOL PIPSocket::GetInterfaceTable(InterfaceTable & table)
{
    InterfaceEntry *IE;
    list<IPInterface>::iterator i;
    for(i=Route4Configuration->Getm_IPInterfaceList().begin();
            i!=Route4Configuration->Getm_IPInterfaceList().end();
            i++)
    {
        char ma[6];
        for(int j=0; j<6; j++) ma[j]=(*i).Getm_macaddr(j);
        IE = new InterfaceEntry((*i).Getm_name().c_str(), (*i).Getm_ipaddr(), ma );
        if(!IE) return false;
        table.Append(IE);
    }
    return true;
}
#endif


//////////////////////////////////////////////////////////////////////////////
// PTCPSocket

PTCPSocket::PTCPSocket(WORD newPort)
{
  SetPort(newPort);
}


PTCPSocket::PTCPSocket(const PString & service)
{
  SetPort(service);
}


PTCPSocket::PTCPSocket(const PString & address, WORD newPort)
{
  SetPort(newPort);
  Connect(address);
}


PTCPSocket::PTCPSocket(const PString & address, const PString & service)
{
  SetPort(service);
  Connect(address);
}


PTCPSocket::PTCPSocket(PSocket & socket)
{
  Accept(socket);
}


PTCPSocket::PTCPSocket(PTCPSocket & tcpSocket)
{
  Accept(tcpSocket);
}


PObject * PTCPSocket::Clone() const
{
  return new PTCPSocket(port);
}


BOOL PTCPSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_STREAM, 0));
}


const char * PTCPSocket::GetProtocolName() const
{
  return "tcp";
}


BOOL PTCPSocket::Write(const void * buf, PINDEX len)
{
  flush();
  PINDEX writeCount = 0;

  while (len > 0) {
    if (!os_sendto(((char *)buf)+writeCount, len, 0, NULL, 0))
      return FALSE;
    writeCount += lastWriteCount;
    len -= lastWriteCount;
  }

  lastWriteCount = writeCount;
  return TRUE;
}


BOOL PTCPSocket::Listen(unsigned queueSize, WORD newPort, Reusability reuse)
{
  return Listen(INADDR_ANY, queueSize, newPort, reuse);
}


BOOL PTCPSocket::Listen(const Address & bindAddr,
                        unsigned queueSize,
                        WORD newPort,
                        Reusability reuse)
{
  if (PIPSocket::Listen(bindAddr, queueSize, newPort, reuse) &&
      ConvertOSError(::listen(os_handle, queueSize)))
    return TRUE;

  os_close();
  return FALSE;
}


BOOL PTCPSocket::Accept(PSocket & socket)
{
  PAssert(socket.IsDescendant(PIPSocket::Class()), "Invalid listener socket");

  sockaddr_in address;
  address.sin_family = AF_INET;
  PINDEX size = sizeof(address);
  if (!ConvertOSError(os_handle = os_accept(socket, (struct sockaddr *)&address, &size)))
    return FALSE;

  port = ((PIPSocket &)socket).GetPort();
  return TRUE;
}


BOOL PTCPSocket::WriteOutOfBand(void const * buf, PINDEX len)
{
#ifdef __NUCLEUS_NET__
  PAssertAlways("WriteOutOfBand unavailable on Nucleus Plus");
  //int count = NU_Send(os_handle, (char *)buf, len, 0);
  int count = ::send(os_handle, (const char *)buf, len, 0);
#else
  int count = ::send(os_handle, (const char *)buf, len, MSG_OOB);
#endif
  if (count < 0) {
    lastWriteCount = 0;
    return ConvertOSError(count, LastWriteError);
  }
  else {
    lastWriteCount = count;
    return TRUE;
  }
}


void PTCPSocket::OnOutOfBand(const void *, PINDEX)
{
}


//////////////////////////////////////////////////////////////////////////////
// PIPDatagramSocket

PIPDatagramSocket::PIPDatagramSocket()
{
}


BOOL PIPDatagramSocket::ReadFrom(void * buf, PINDEX len,
                                 Address & addr, WORD & port)
{
  lastReadCount = 0;

  sockaddr_in sockAddr;
  PINDEX addrLen = sizeof(sockAddr);
  if (os_recvfrom(buf, len, 0, (struct sockaddr *)&sockAddr, &addrLen)) {
    addr = sockAddr.sin_addr;
    port = ntohs(sockAddr.sin_port);
  }

  return lastReadCount > 0;
}


BOOL PIPDatagramSocket::WriteTo(const void * buf, PINDEX len,
                                const Address & addr, WORD port)
{
  lastWriteCount = 0;

  sockaddr_in sockAddr;
  sockAddr.sin_family = AF_INET;
  sockAddr.sin_addr = addr;
  sockAddr.sin_port = htons(port);
  return os_sendto(buf, len, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr))
         && lastWriteCount >= len;
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

PUDPSocket::PUDPSocket(WORD newPort)
{
  sendPort = 0;
  SetPort(newPort);
  OpenSocket();
}


PUDPSocket::PUDPSocket(const PString & service)
{
  sendPort = 0;
  SetPort(service);
  OpenSocket();
}


PUDPSocket::PUDPSocket(const PString & address, WORD newPort)
{
  sendPort = 0;
  SetPort(newPort);
  Connect(address);
}


PUDPSocket::PUDPSocket(const PString & address, const PString & service)
{
  sendPort = 0;
  SetPort(service);
  Connect(address);
}


BOOL PUDPSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_DGRAM, 0));
}


const char * PUDPSocket::GetProtocolName() const
{
  return "udp";
}


BOOL PUDPSocket::Connect(const PString & address)
{
  sendPort = 0;
  return PIPDatagramSocket::Connect(address);
}


BOOL PUDPSocket::Read(void * buf, PINDEX len)
{
  return PIPDatagramSocket::ReadFrom(buf, len, lastReceiveAddress, lastReceivePort);
}


BOOL PUDPSocket::Write(const void * buf, PINDEX len)
{
  if (sendPort == 0)
    return PIPDatagramSocket::Write(buf, len);
  else
    return PIPDatagramSocket::WriteTo(buf, len, sendAddress, sendPort);
}


void PUDPSocket::SetSendAddress(const Address & newAddress, WORD newPort)
{
  sendAddress = newAddress;
  sendPort    = newPort;
}


void PUDPSocket::GetSendAddress(Address & address, WORD & port)
{
  address = sendAddress;
  port    = sendPort;
}


void PUDPSocket::GetLastReceiveAddress(Address & address, WORD & port)
{
  address = lastReceiveAddress;
  port    = lastReceivePort;
}


// End Of File ///////////////////////////////////////////////////////////////
