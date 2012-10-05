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
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptbuildopts.h>
#include <ptlib/sockets.h>

#include <ctype.h>

#define PTraceModule() "Socket"

#ifdef P_VXWORKS
// VxWorks variant of inet_ntoa() allocates INET_ADDR_LEN bytes via malloc
// BUT DOES NOT FREE IT !!!  Use inet_ntoa_b() instead.
#define INET_ADDR_LEN      18
extern "C" void inet_ntoa_b(struct in_addr inetAddress, char *pString);
#endif // P_VXWORKS

#ifdef __NUCLEUS_PLUS__
#include <ConfigurationClass.h>
#endif

#if P_QOS

#ifdef _WIN32
#include <winbase.h>
#include <winreg.h>

#ifndef _WIN32_WCE

void CALLBACK CompletionRoutine(DWORD dwError,
                                DWORD cbTransferred,
                                LPWSAOVERLAPPED lpOverlapped,
                                DWORD dwFlags);
                                

#endif  // _WIN32_WCE
#endif  // _WIN32
#endif // P_QOS


#if !defined(P_MINGW) && !defined(P_CYGWIN)
  #if P_HAS_IPV6 || defined(AI_NUMERICHOST)
    #define HAS_GETADDRINFO 1
  #endif
#else
  #if WINVER > 0x500
    #define HAS_GETADDRINFO 1
  #endif
#endif


///////////////////////////////////////////////////////////////////////////////
// PIPSocket::Address

static int g_defaultIpAddressFamily = PF_INET;  // PF_UNSPEC;   // default to IPV4
static bool g_suppressCanonicalName = false;

static PIPSocket::Address loopback4(127,0,0,1);
static PIPSocket::Address broadcast4(INADDR_BROADCAST);
static PIPSocket::Address any4(0,0,0,0);
static PIPSocket::Address invalid(0,NULL);

#if P_HAS_IPV6

  static PIPSocket::Address loopback6(16,(const BYTE *)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\001");
  static PIPSocket::Address broadcast6(16,(const BYTE *)"\377\002\0\0\0\0\0\0\0\0\0\0\0\0\0\001"); // IPV6 multicast address
  static PIPSocket::Address any6(16,(const BYTE *)"\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0");

  #define IPV6_PARAM(p) p
#else
  #define IPV6_PARAM(p)
#endif


void PIPSocket::SetSuppressCanonicalName(bool suppress)
{
  g_suppressCanonicalName = suppress;
}


bool PIPSocket::GetSuppressCanonicalName()
{
  return g_suppressCanonicalName;
}


int PIPSocket::GetDefaultIpAddressFamily()
{
  return g_defaultIpAddressFamily;
}


void PIPSocket::SetDefaultIpAddressFamily(int ipAdressFamily)
{
  g_defaultIpAddressFamily = ipAdressFamily;
}


void PIPSocket::SetDefaultIpAddressFamilyV4()
{
  SetDefaultIpAddressFamily(PF_INET);
}


#if P_HAS_IPV6

void PIPSocket::SetDefaultIpAddressFamilyV6()
{
  SetDefaultIpAddressFamily(PF_INET6);
}

PBoolean PIPSocket::IsIpAddressFamilyV6Supported()
{
  int s = (int)::socket(PF_INET6, SOCK_DGRAM, 0);
  if (s < 0)
    return false;

#if _WIN32
  closesocket(s);
#else
  _close(s);
#endif
  return true;
}

#endif // P_HAS_IPV6


const PIPSocket::Address & PIPSocket::GetDefaultIpAny()
{
#if P_HAS_IPV6
  if (g_defaultIpAddressFamily != PF_INET)
    return any6;
#endif

  return any4;
}


const PIPSocket::Address & PIPSocket::GetInvalidAddress()
{
  return invalid;
}


class Psockaddr
{
  public:
    Psockaddr() : ptr(&storage) { memset(&storage, 0, sizeof(storage)); }
    Psockaddr(const PIPSocket::Address & ip, WORD port);
    sockaddr* operator->() const { return addr; }
    operator sockaddr*()   const { return addr; }
    socklen_t GetSize() const;
    PIPSocket::Address GetIP() const;
    WORD GetPort() const;
  private:
    sockaddr_storage storage;
    union {
      sockaddr_storage * ptr;
      sockaddr         * addr;
      sockaddr_in      * addr4;
#if P_HAS_IPV6
      sockaddr_in6     * addr6;
#endif
    };
};


Psockaddr::Psockaddr(const PIPSocket::Address & ip, WORD port)
 : ptr(&storage) 
{
  memset(&storage, 0, sizeof(storage));

  switch (ip.GetVersion()) {
    case 4 :
#ifdef P_NETBSD
      addr4->sin_len    = sizeof(struct sockaddr_in);
#endif
      addr4->sin_family = AF_INET;
      addr4->sin_addr   = ip;
      addr4->sin_port   = htons(port);
      break;

#if P_HAS_IPV6
    case 6 :
#ifdef P_NETBSD
      addr6->sin6_len      = sizeof(struct sockaddr_in6);
#endif
      addr6->sin6_family   = AF_INET6;
      addr6->sin6_addr     = ip;
      addr6->sin6_port     = htons(port);
      addr6->sin6_flowinfo = 0;
      addr6->sin6_scope_id = ip.GetIPV6Scope();
      break;
#endif
  }
}


socklen_t Psockaddr::GetSize() const
{
  switch (addr->sa_family) {
    case AF_INET :
      return sizeof(sockaddr_in);
#if P_HAS_IPV6
    case AF_INET6 :
      // RFC 2133 (Old IPv6 spec) size is 24
      // RFC 2553 (New IPv6 spec) size is 28
      return sizeof(sockaddr_in6);
#endif
    default :
      return sizeof(storage);
  }
}


PIPSocket::Address Psockaddr::GetIP() const
{
  switch (addr->sa_family) {
    case AF_INET :
      return addr4->sin_addr;
#if P_HAS_IPV6
    case AF_INET6 :
      return PIPSocket::Address(addr6->sin6_addr, addr6->sin6_scope_id);
#endif
    default :
      return 0;
  }
}


WORD Psockaddr::GetPort() const
{
  switch (addr->sa_family) {
    case AF_INET :
      return ntohs(addr4->sin_port);
#if P_HAS_IPV6
    case AF_INET6 :
      return ntohs(addr6->sin6_port);
#endif
    default :
      return 0;
  }
}



#if (defined(_WIN32) || defined(WINDOWS)) && !defined(__NUCLEUS_MNT__)
static PWinSock dummyForWinSock; // Assure winsock is initialised
#endif

#if (defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)) || defined(__NUCLEUS_PLUS__)
#define REENTRANT_BUFFER_LEN 1024
#endif


class PIPCacheData : public PObject
{
  PCLASSINFO(PIPCacheData, PObject)
  public:
    PIPCacheData(struct hostent * ent, const char * original);
#if HAS_GETADDRINFO
    PIPCacheData(struct addrinfo  * addr_info, const char * original);
    void AddEntry(struct addrinfo  * addr_info);
#endif
    const PString & GetHostName() const { return hostname; }
    const PIPSocket::Address & GetHostAddress() const { return address; }
    const PStringArray& GetHostAliases() const { return aliases; }
    PBoolean HasAged() const;
  private:
    PString            hostname;
    PIPSocket::Address address;
    PStringArray       aliases;
    PTime              birthDate;
};



PDICTIONARY(PHostByName_private, PCaselessString, PIPCacheData);

class PHostByName : PHostByName_private
{
  public:
    PBoolean GetHostName(const PString & name, PString & hostname);
    PBoolean GetHostAddress(const PString & name, PIPSocket::Address & address);
    PBoolean GetHostAliases(const PString & name, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PString & name);
    PMutex mutex;
  friend void PIPSocket::ClearNameCache();
};

static PMutex creationMutex;
static PHostByName & pHostByName()
{
  PWaitAndSignal m(creationMutex);
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
      { return (addr[1] + addr[2] + addr[3])%41; }

  private:
    PIPSocket::Address addr;
};

PDICTIONARY(PHostByAddr_private, PIPCacheKey, PIPCacheData);

class PHostByAddr : PHostByAddr_private
{
  public:
    PBoolean GetHostName(const PIPSocket::Address & addr, PString & hostname);
    PBoolean GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address);
    PBoolean GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases);
  private:
    PIPCacheData * GetHost(const PIPSocket::Address & addr);
    PMutex mutex;
  friend void PIPSocket::ClearNameCache();
};

static PHostByAddr & pHostByAddr()
{
  PWaitAndSignal m(creationMutex);
  static PHostByAddr t;
  return t;
}

#define new PNEW


//////////////////////////////////////////////////////////////////////////////
// IP Caching

PIPCacheData::PIPCacheData(struct hostent * host_info, const char * original)
  : address(PIPSocket::GetInvalidAddress())
{
  if (host_info == NULL)
    return;

  hostname = host_info->h_name;
  if (host_info->h_addr != NULL)
#ifndef _WIN32_WCE
    address = *(DWORD *)host_info->h_addr;
#else
    address = PIPSocket::Address(host_info->h_length, (const BYTE *)host_info->h_addr);
#endif
  aliases.AppendString(host_info->h_name);

  PINDEX i;
  for (i = 0; host_info->h_aliases[i] != NULL; i++)
    aliases.AppendString(host_info->h_aliases[i]);

  for (i = 0; host_info->h_addr_list[i] != NULL; i++) {
#ifndef _WIN32_WCE
    PIPSocket::Address ip(*(DWORD *)host_info->h_addr_list[i]);
#else
    PIPSocket::Address ip(host_info->h_length, (const BYTE *)host_info->h_addr_list[i]);
#endif
    aliases.AppendString(ip.AsString());
  }

  for (i = 0; i < aliases.GetSize(); i++)
    if (aliases[i] *= original)
      return;

  aliases.AppendString(original);
}


#if HAS_GETADDRINFO

PIPCacheData::PIPCacheData(struct addrinfo * addr_info, const char * original)
  : address(PIPSocket::GetInvalidAddress())
{
  PINDEX i;
  if (addr_info == NULL)
    return;

  // Fill Host primary informations
  hostname = addr_info->ai_canonname; // Fully Qualified Domain Name (FQDN)
  if (g_suppressCanonicalName || hostname.IsEmpty())
    hostname = original;
  if (addr_info->ai_addr != NULL)
    address = PIPSocket::Address(addr_info->ai_family, addr_info->ai_addrlen, addr_info->ai_addr);

  // Next entries
  while (addr_info != NULL) {
    AddEntry(addr_info);
    addr_info = addr_info->ai_next;
  }

  // Add original as alias or allready added ?
  for (i = 0; i < aliases.GetSize(); i++) {
    if (aliases[i] *= original)
      return;
  }

  aliases.AppendString(original);
}


void PIPCacheData::AddEntry(struct addrinfo * addr_info)
{
  PINDEX i;

  if (addr_info == NULL)
    return;

  // Add canonical name
  bool add_it = true;
  for (i = 0; i < aliases.GetSize(); i++) {
    if (addr_info->ai_canonname != NULL && (aliases[i] *= addr_info->ai_canonname)) {
      add_it = false;
      break;
    }
  }

  if (add_it && addr_info->ai_canonname != NULL)
    aliases.AppendString(addr_info->ai_canonname);

  // Add IP address
  PIPSocket::Address ip(addr_info->ai_family, addr_info->ai_addrlen, addr_info->ai_addr);
  add_it = true;
  for (i = 0; i < aliases.GetSize(); i++) {
    if (aliases[i] *= ip.AsString()) {
      add_it = false;
      break;
    }
  }

  if (add_it)
    aliases.AppendString(ip.AsString());
}

#endif // HAS_GETADDRINFO


static PTimeInterval GetConfigTime(const char * /*key*/, DWORD dflt)
{
  //PConfig cfg("DNS Cache");
  //return cfg.GetInteger(key, dflt);
  return dflt;
}


PBoolean PIPCacheData::HasAged() const
{
  static PTimeInterval retirement = GetConfigTime("Age Limit", 300000); // 5 minutes
  PTime now;
  PTimeInterval age = now - birthDate;
  return age > retirement;
}


PBoolean PHostByName::GetHostName(const PString & name, PString & hostname)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL) {
    hostname = host->GetHostName();
    hostname.MakeUnique();
  }

  mutex.Signal();

  return host != NULL;
}


PBoolean PHostByName::GetHostAddress(const PString & name, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();

  return host != NULL;
}


PBoolean PHostByName::GetHostAliases(const PString & name, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(name);

  if (host != NULL)
    aliases = host->GetHostAliases();

  mutex.Signal();
  return host != NULL;
}


PIPCacheData * PHostByName::GetHost(const PString & name)
{
  mutex.Wait();

  PString key = name;
  PINDEX len = key.GetLength();

  // Check for a legal hostname as per RFC952
  // but drop the requirement for leading alpha as per RFC 1123
  if (key.IsEmpty() ||
      key.FindSpan("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-.") != P_MAX_INDEX ||
      key[len-1] == '-') {
    PTRACE_IF(3, key[0] != '[', "Illegal RFC952 characters in DNS name \"" << key << '"');
    return NULL;
  }

  // We lowercase this way rather than toupper() as that is locale dependent, and DNS names aren't.
  for (PINDEX i = 0; i < len; i++) {
    if (key[i] >= 'a')
      key[i] &= 0x5f;
  }

  PIPCacheData * host = GetAt(key);
  int localErrNo = NO_DATA;

  if (host != NULL && host->HasAged()) {
    SetAt(key, NULL);
    host = NULL;
  }

  if (host == NULL) {
    mutex.Signal();

#if HAS_GETADDRINFO

    struct addrinfo *res = NULL;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    if (!g_suppressCanonicalName)
      hints.ai_flags = AI_CANONNAME;
    hints.ai_family = g_defaultIpAddressFamily;
    localErrNo = getaddrinfo((const char *)name, NULL , &hints, &res);
    if (localErrNo != 0) {
      hints.ai_family = g_defaultIpAddressFamily == AF_INET6 ? AF_INET : AF_INET6;
      localErrNo = getaddrinfo((const char *)name, NULL , &hints, &res);
    }
    host = new PIPCacheData(localErrNo != NETDB_SUCCESS ? NULL : res, name);
    if (res != NULL)
      freeaddrinfo(res);

#else // HAS_GETADDRINFO

    int retry = 3;
    struct hostent * host_info;

#ifdef P_AIX

    struct hostent_data ht_data;
    memset(&ht_data, 0, sizeof(ht_data));
    struct hostent hostEnt;
    do {
      host_info = &hostEnt;
      ::gethostbyname_r(name,
                        host_info,
                        &ht_data);
      localErrNo = h_errno;
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#elif defined(P_RTEMS) || defined(P_CYGWIN) || defined(P_MINGW)

    host_info = ::gethostbyname(name);
    localErrNo = h_errno;

#elif defined P_VXWORKS

    struct hostent hostEnt;
    host_info = Vx_gethostbyname((char *)name, &hostEnt);
    localErrNo = h_errno;

#elif defined P_LINUX || defined(P_GNU_HURD) || defined(P_ANDROID)

    char buffer[REENTRANT_BUFFER_LEN];
    struct hostent hostEnt;
    do {
      if (::gethostbyname_r(name,
                            &hostEnt,
                            buffer, REENTRANT_BUFFER_LEN,
                            &host_info,
                            &localErrNo) == 0)
        localErrNo = NETDB_SUCCESS;
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#elif (defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)) || defined(__NUCLEUS_PLUS__)

    char buffer[REENTRANT_BUFFER_LEN];
    struct hostent hostEnt;
    do {
      host_info = ::gethostbyname_r(name,
                                    &hostEnt,
                                    buffer, REENTRANT_BUFFER_LEN,
                                    &localErrNo);
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#else

    host_info = ::gethostbyname(name);
    localErrNo = h_errno;

#endif

    if (localErrNo != NETDB_SUCCESS || retry == 0)
      host_info = NULL;
    host = new PIPCacheData(host_info, name);

#endif //HAS_GETADDRINFO

    mutex.Wait();

    SetAt(key, host);
  }

  if (host->GetHostAddress().IsValid())
    return host;

  PTRACE(4, "Name lookup of \"" << name << "\" failed: errno=" << localErrNo);
  return NULL;
}


PBoolean PHostByAddr::GetHostName(const PIPSocket::Address & addr, PString & hostname)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL) {
    hostname = host->GetHostName();
    hostname.MakeUnique();
  }

  mutex.Signal();
  return host != NULL;
}


PBoolean PHostByAddr::GetHostAddress(const PIPSocket::Address & addr, PIPSocket::Address & address)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    address = host->GetHostAddress();

  mutex.Signal();
  return host != NULL;
}


PBoolean PHostByAddr::GetHostAliases(const PIPSocket::Address & addr, PStringArray & aliases)
{
  PIPCacheData * host = GetHost(addr);

  if (host != NULL)
    aliases = host->GetHostAliases();

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

    int retry = 3;
    int localErrNo = NETDB_SUCCESS;
    struct hostent * host_info;

#ifdef P_AIX

    struct hostent_data ht_data;
    struct hostent hostEnt;
    do {
      host_info = &hostEnt;
      ::gethostbyaddr_r((char *)addr.GetPointer(), addr.GetSize(),
                        PF_INET, 
                        host_info,
                        &ht_data);
      localErrNo = h_errno;
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#elif defined P_RTEMS || defined P_CYGWIN || defined P_MINGW || defined P_ANDROID

    // Mutex here is not perfect, but will be 100% provided application only
    // ever use PTLib do name lookups, no direct calls to gethostbyaddr()
    mutex.Wait();
    host_info = ::gethostbyaddr(addr.GetPointer(), addr.GetSize(), PF_INET);
    localErrNo = h_errno;
    mutex.Signal();

#elif defined P_VXWORKS

    struct hostent hostEnt;
    host_info = Vx_gethostbyaddr(addr.GetPointer(), &hostEnt);

#elif defined P_LINUX || defined(P_GNU_HURD)

    char buffer[REENTRANT_BUFFER_LEN];
    struct hostent hostEnt;
    do {
      gethostbyaddr_r(addr.GetPointer(), addr.GetSize(),
                      PF_INET, 
                      &hostEnt,
                      buffer, REENTRANT_BUFFER_LEN,
                      &host_info,
                      &localErrNo);
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#elif (defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)) || defined(__NUCLEUS_PLUS__)

    char buffer[REENTRANT_BUFFER_LEN];
    struct hostent hostEnt;
    do {
      host_info = ::gethostbyaddr_r(addr.GetPointer(), addr.GetSize(),
                                    PF_INET, 
                                    &hostEnt,
                                    buffer, REENTRANT_BUFFER_LEN,
                                    &localErrNo);
    } while (localErrNo == TRY_AGAIN && --retry > 0);

#else

    host_info = ::gethostbyaddr(addr.GetPointer(), addr.GetSize(), PF_INET);
    localErrNo = h_errno;

#if defined(_WIN32) || defined(WINDOWS)  // Kludge to avoid strange 95 bug
    extern PBoolean P_IsOldWin95();
    if (P_IsOldWin95() && host_info != NULL && host_info->h_addr_list[0] != NULL)
      host_info->h_addr_list[1] = NULL;
#endif

#endif

    mutex.Wait();

    if (localErrNo != NETDB_SUCCESS || retry == 0)
      return NULL;

    host = new PIPCacheData(host_info, addr.AsString());

    SetAt(key, host);
  }

  return host->GetHostAddress().IsValid() ? host : NULL;
}


//////////////////////////////////////////////////////////////////////////////
// P_fd_set

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable:4127)
#endif

P_fd_set::P_fd_set()
{
  Construct();
  Zero();
}


P_fd_set::P_fd_set(SOCKET fd)
{
  Construct();
  Zero();
  FD_SET(fd, set);
}


P_fd_set & P_fd_set::operator=(SOCKET fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  Zero();
  FD_SET(fd, set);
  return *this;
}


P_fd_set & P_fd_set::operator+=(SOCKET fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  FD_SET(fd, set);
  return *this;
}


P_fd_set & P_fd_set::operator-=(SOCKET fd)
{
  PAssert(fd < max_fd, PInvalidParameter);
  FD_CLR(fd, set);
  return *this;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif


//////////////////////////////////////////////////////////////////////////////
// P_timeval

P_timeval::P_timeval()
{
  tval.tv_usec = 0;
  tval.tv_sec = 0;
  infinite = false;
}


P_timeval & P_timeval::operator=(const PTimeInterval & time)
{
  infinite = time == PMaxTimeInterval;
  tval.tv_usec = (long)(time.GetMilliSeconds()%1000)*1000;
  tval.tv_sec = time.GetSeconds();
  return *this;
}


//////////////////////////////////////////////////////////////////////////////
// PSocket

PSocket::PSocket()
{
  port = 0;
}


PBoolean PSocket::Connect(const PString &)
{
  PAssertAlways("Illegal operation.");
  return false;
}


PBoolean PSocket::Listen(unsigned, WORD, Reusability)
{
  PAssertAlways("Illegal operation.");
  return false;
}


PBoolean PSocket::Accept(PSocket &)
{
  PAssertAlways("Illegal operation.");
  return false;
}


PBoolean PSocket::SetOption(int option, int value, int level)
{
#ifdef _WIN32_WCE
  if(option == SO_RCVBUF || option == SO_SNDBUF || option == IP_TOS)
    return true;
#endif

  return ConvertOSError(::setsockopt(os_handle, level, option,
                                     (char *)&value, sizeof(value)));
}


PBoolean PSocket::SetOption(int option, const void * valuePtr, PINDEX valueSize, int level)
{
  return ConvertOSError(::setsockopt(os_handle, level, option,
                                     (char *)valuePtr, valueSize));
}


PBoolean PSocket::GetOption(int option, int & value, int level)
{
  socklen_t valSize = sizeof(value);
  return ConvertOSError(::getsockopt(os_handle, level, option,
                                     (char *)&value, &valSize));
}


PBoolean PSocket::GetOption(int option, void * valuePtr, PINDEX valueSize, int level)
{
  return ConvertOSError(::getsockopt(os_handle, level, option,
                                     (char *)valuePtr, (socklen_t *)&valueSize));
}


PBoolean PSocket::Shutdown(ShutdownValue value)
{
  return ConvertOSError(::shutdown(os_handle, value));
}


WORD PSocket::GetProtocolByName(const PString & name)
{
#if !defined(__NUCLEUS_PLUS__) && !defined(P_VXWORKS)
  struct protoent * ent = getprotobyname(name);
  if (ent != NULL)
    return ent->p_proto;
#endif

  return 0;
}


PString PSocket::GetNameByProtocol(WORD proto)
{
#if !defined(__NUCLEUS_PLUS__) && !defined(P_VXWORKS)
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
  if (service.FindSpan("0123456789") == P_MAX_INDEX)
    return (WORD)service.AsUnsigned();

#if defined( __NUCLEUS_PLUS__ )
  PAssertAlways("PSocket::GetPortByService: problem as no ::getservbyname in Nucleus NET");
  return 0;
#elif defined(P_VXWORKS)
  PAssertAlways("PSocket::GetPortByService: problem as no ::getservbyname in VxWorks");
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
#if !defined(__NUCLEUS_PLUS__) && !defined(P_VXWORKS)
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
  SelectList read, dummy1, dummy2;
  read += sock1;
  read += sock2;

  Errors lastError;
  int osError;
  if (!ConvertOSError(Select(read, dummy1, dummy2, timeout), lastError, osError))
    return lastError;

  switch (read.GetSize()) {
    case 0 :
      return 0;
    case 2 :
      return -3;
    default :
      return &read.front() == &sock1 ? -1 : -2;
  }
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


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

PIPSocket::PIPSocket()
{
}


void PIPSocket::ClearNameCache()
{
  pHostByName().mutex.Wait();
  pHostByName().RemoveAll();
  pHostByName().mutex.Signal();

  pHostByAddr().mutex.Wait();
  pHostByAddr().RemoveAll();
  pHostByAddr().mutex.Signal();

#if (defined(_WIN32) || defined(WINDOWS)) && !defined(__NUCLEUS_MNT__) // Kludge to avoid strange NT bug
  static PTimeInterval delay = GetConfigTime("NT Bug Delay", 0);
  if (delay != 0) {
    ::Sleep(delay.GetInterval());
    ::gethostbyname("www.microsoft.com");
  }
#endif
  PTRACE(4, NULL, NULL, "Cleared DNS cache.");
}


PString PIPSocket::GetName() const
{
  Psockaddr sa;
  socklen_t size = sa.GetSize();
  if (getpeername(os_handle, sa, &size) != 0)
    return PString::Empty();

  return GetHostName(sa.GetIP()) + psprintf(":%u", sa.GetPort());
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
  if (temp.IsValid())
    return GetHostName(temp);

  PString canonicalname;
  if (pHostByName().GetHostName(hostname, canonicalname))
    return canonicalname;

  return hostname;
}


PString PIPSocket::GetHostName(const Address & addr)
{
  if (!addr.IsValid())
    return addr.AsString();

  PString hostname;
  if (pHostByAddr().GetHostName(addr, hostname))
    return hostname;

  return addr.AsString(true);
}


PBoolean PIPSocket::GetHostAddress(Address & addr)
{
  return pHostByName().GetHostAddress(GetHostName(), addr);
}


PBoolean PIPSocket::GetHostAddress(const PString & hostname, Address & addr)
{
  if (hostname.IsEmpty())
    return false;

  // Check for special case of "[ipaddr]"
  if (hostname[0] == '[') {
    PINDEX end = hostname.Find(']');
    if (end != P_MAX_INDEX) {
      if (addr.FromString(hostname(1, end-1)))
        return true;
    }
  }

  // Assuming it is a "." address and return if so
  if (addr.FromString(hostname))
    return true;

  // otherwise lookup the name as a host name
  return pHostByName().GetHostAddress(hostname, addr);
}


PStringArray PIPSocket::GetHostAliases(const PString & hostname)
{
  PStringArray aliases;

  // lookup the host address using inet_addr, assuming it is a "." address
  Address addr = hostname;
  if (addr.IsValid())
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


PString PIPSocket::GetLocalAddress()
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetLocalAddress(addrAndPort)) 
    return PString::Empty();
  return addrAndPort.AsString();
}


bool PIPSocket::GetLocalAddress(Address & addr)
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetLocalAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  return true;
}


bool PIPSocket::GetLocalAddress(Address & addr, WORD & portNum)
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetLocalAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  portNum = addrAndPort.GetPort();
  return true;
}


bool PIPSocket::InternalGetLocalAddress(PIPSocketAddressAndPort & addrAndPort)
{
  Address   peerv4;
  Psockaddr sa;
  socklen_t size = sa.GetSize();
  if (!ConvertOSError(::getsockname(os_handle, sa, &size)))
    return false;

  addrAndPort.SetAddress(sa.GetIP());
  addrAndPort.SetPort(sa.GetPort());

#if P_HAS_IPV6
  // If the remote host is an IPv4 only host and our interface if an IPv4/IPv6 mapped
  // Then return an IPv4 address instead of an IPv6
  if (addrAndPort.GetAddress().IsV4Mapped()) {
    Address peer;
    if (GetPeerAddress(peer) && (peer.GetVersion() == 4 || peer.IsV4Mapped())) {
      Address addr = addrAndPort.GetAddress();
      addrAndPort.SetAddress(Address(addr[12], addr[13], addr[14], addr[15]));
    }
  }
#endif

  return true;
}


PString PIPSocket::GetPeerAddress()
{
  PIPSocketAddressAndPort addrAndPort;
  if (GetPeerAddress(addrAndPort)) 
    return addrAndPort.AsString();
  return PString::Empty();
}


bool PIPSocket::GetPeerAddress(Address & addr)
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetPeerAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  return true;
}


bool PIPSocket::GetPeerAddress(Address & addr, WORD & portNum)
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetPeerAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  portNum = addrAndPort.GetPort();
  return true;
}


bool PIPSocket::InternalGetPeerAddress(PIPSocketAddressAndPort & addrAndPort)
{
  Psockaddr sa;
  socklen_t size = sa.GetSize();
  if (!ConvertOSError(::getpeername(os_handle, sa, &size)))
    return false;

  addrAndPort.SetAddress(sa.GetIP());
  addrAndPort.SetPort(sa.GetPort());

  return true;
}


PString PIPSocket::GetLocalHostName()
{
  Address addr;

  if (GetLocalAddress(addr))
    return GetHostName(addr);

  return PString::Empty();
}


PString PIPSocket::GetPeerHostName()
{
  Address addr;

  if (GetPeerAddress(addr))
    return GetHostName(addr);

  return PString::Empty();
}


PBoolean PIPSocket::Connect(const PString & host)
{
  Address ipnum(host);
  if (ipnum.IsValid() || GetHostAddress(host, ipnum))
    return Connect(Address::GetAny(ipnum.GetVersion()), 0, ipnum);
  return false;
}


PBoolean PIPSocket::Connect(const Address & addr)
{
  return Connect(Address::GetAny(addr.GetVersion()), 0, addr);
}


PBoolean PIPSocket::Connect(WORD localPort, const Address & addr)
{
  return Connect(Address::GetAny(addr.GetVersion()), localPort, addr);
}


PBoolean PIPSocket::Connect(const Address & iface, const Address & addr)
{
  return Connect(iface, 0, addr);
}


PBoolean PIPSocket::Connect(const Address & iface, WORD localPort, const Address & addr)
{
  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  Psockaddr sa(addr, port);

  // attempt to create a socket with the right family
  if (!OpenSocket(sa->sa_family))
    return false;

  if (localPort != 0 || iface.IsValid()) {
    Psockaddr bind_sa(iface, localPort);

    if (!SetOption(SO_REUSEADDR, 0)) {
      os_close();
      return false;
    }
    
    if (!ConvertOSError(::bind(os_handle, bind_sa, bind_sa.GetSize()))) {
      os_close();
      return false;
    }
  }
  
  // attempt to connect
  if (os_connect(sa, sa.GetSize()))
    return true;
  
  os_close();
  return false;
}


bool PIPSocket::InternalListen(const Address & bindAddr,
                               unsigned,
                               WORD newPort,
                               Reusability reuse)
{
  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  Psockaddr sa(bindAddr, port);

  // Always close and re-open as the bindAddr address family might change.
  os_close();

  // attempt to create a socket
  if (!OpenSocket(sa->sa_family)) {
    PTRACE(4, "OpenSocket failed");
    return false;
  }

  int reuseAddr = reuse == CanReuseAddress ? 1 : 0;
  if (!SetOption(SO_REUSEADDR, reuseAddr)) {
    PTRACE(4, "SetOption(SO_REUSEADDR," << reuseAddr << ") failed: " << GetErrorText());
    os_close();
    return false;
  }

#if P_HAS_IPV6 && defined(IPV6_V6ONLY)
  if (bindAddr.GetVersion() == 6) {
    if (!SetOption(IPV6_V6ONLY, reuseAddr, IPPROTO_IPV6)) {
      PTRACE(4, "SetOption(IPV6_V6ONLY," << reuseAddr << ") failed: " << GetErrorText());
    }
  }
#endif

  if (!ConvertOSError(::bind(os_handle, sa, sa.GetSize()))) {
    PTRACE(4, "bind failed: " << GetErrorText());
    os_close();
    return false;
  }

  if (port != 0)
    return true;

  socklen_t size = sa.GetSize();
  if (!ConvertOSError(::getsockname(os_handle, sa, &size))) {
    PTRACE(4, "getsockname failed: " << GetErrorText());
    os_close();
    return false;
  }

  port = sa.GetPort();
  return true;
}


#if P_HAS_IPV6

/// Check for v4 mapped in v6 address ::ffff:a.b.c.d
PBoolean PIPSocket::Address::IsV4Mapped() const
{
  return m_version == 6 && (IN6_IS_ADDR_V4MAPPED(&m_v.m_six) || IN6_IS_ADDR_V4COMPAT(&m_v.m_six));
}

/// Check for link-local address
PBoolean PIPSocket::Address::IsLinkLocal() const
{
  return m_version == 6 && IN6_IS_ADDR_LINKLOCAL(&m_v.m_six);
}

/// Check for site-local address
PBoolean PIPSocket::Address::IsSiteLocal() const
{
  return m_version == 6 && IN6_IS_ADDR_SITELOCAL(&m_v.m_six);
}


PIPSocket::Address::Address(const in6_addr & addr)
 {
  if (IN6_IS_ADDR_LINKLOCAL(&addr)) {
    PTRACE(2, "Cannot create link-local IPV6 address without scope id");
    m_version = 0;
  }
  else {
    m_version = 6;
    m_v.m_six = addr;
    m_scope6  = 0;
  }
}

PIPSocket::Address::Address(const in6_addr & addr, int scope)
{
  m_version = 6;
  m_v.m_six = addr;
  m_scope6  = scope;
}


PIPSocket::Address & PIPSocket::Address::AssignIPV6(const in6_addr & addr, int scope)
{
  m_version = 6;
  m_v.m_six = addr;
  m_scope6  = scope;
  return *this;
}


bool PIPSocket::Address::operator*=(const PIPSocket::Address & addr) const
{
  if (m_version == addr.m_version)
    return operator==(addr);

  if (this->GetVersion() == 6 && this->IsV4Mapped()) 
    return PIPSocket::Address((*this)[12], (*this)[13], (*this)[14], (*this)[15]) == addr;
  else if (addr.GetVersion() == 6 && addr.IsV4Mapped()) 
    return *this == PIPSocket::Address(addr[12], addr[13], addr[14], addr[15]);

  return false;
}


bool PIPSocket::Address::operator==(in6_addr & addr) const
{
  return m_version == 6 && memcmp(&addr, &m_v.m_six, sizeof(addr)) == 0;
}


bool PIPSocket::Address::EqualIPV6(in6_addr & addr, int scope) const
{
  PIPSocket::Address a(addr, scope);
  return Compare(a) == EqualTo;
}


PIPSocket::Address::operator in6_addr() const
{
  switch (m_version) {
    default :
      return any6.m_v.m_six;
    case 6 :
      return m_v.m_six;
    case 4 :
      in6_addr a = {{{
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff,
        Byte1(), Byte2(), Byte3(), Byte4()
      }}};
      return a;
  }
}

#endif // P_HAS_IPV6


const PIPSocket::Address & PIPSocket::Address::GetLoopback(int IPV6_PARAM(version))
{
#if P_HAS_IPV6
  if (version == 6)
    return loopback6;
#endif
  return loopback4;
}


const PIPSocket::Address & PIPSocket::Address::GetAny(int IPV6_PARAM(version))
{
#if P_HAS_IPV6
  switch (version) {
    case 6 :
      return any6;
    case 4 :
      return any4;

    default :
#else
  {
#endif
      return PIPSocket::GetDefaultIpAny();
  }
}


const PIPSocket::Address PIPSocket::Address::GetBroadcast(int IPV6_PARAM(version))
{
#if P_HAS_IPV6
  if (version == 6)
    return broadcast6;
#endif
  return broadcast4;
}


PIPSocket::Address::Address()
{
#if P_HAS_IPV6
  if (g_defaultIpAddressFamily == AF_INET6)
    *this = loopback6;
  else
#endif
    *this = loopback4;
}


PIPSocket::Address::Address(const PString & dotNotation)
{
  operator=(dotNotation);
}


PIPSocket::Address::Address(PINDEX len, const BYTE * bytes, int scope)
{
  switch (len) {
#if P_HAS_IPV6
    case 16 :
      m_version = 6;
      memcpy(&m_v.m_six, bytes, len);
      m_scope6 = scope;
      break;
#endif
    case 4 :
      m_version = 4;
      memcpy(&m_v.m_four, bytes, len);
      m_scope6 = 0;
      break;

    default :
      m_version = 0;
  }
}


PIPSocket::Address::Address(const in_addr & addr)
{
  m_version = 4;
  m_v.m_four = addr;
}


// Create an IP (v4 or v6) address from a sockaddr (sockaddr_in, sockaddr_in6 or sockaddr_in6_old) structure
PIPSocket::Address::Address(const int ai_family, const int ai_addrlen, struct sockaddr *ai_addr)
{
  switch (ai_family) {
#if P_HAS_IPV6
    case AF_INET6:
      if (ai_addrlen < (int)sizeof(sockaddr_in6)) {
        PTRACE(1, "sockaddr size too small (" << ai_addrlen << ")  for family " << ai_family);
        break;
      }

      m_version = 6;
      m_v.m_six = ((struct sockaddr_in6 *)ai_addr)->sin6_addr;
      m_scope6  = ((struct sockaddr_in6 *)ai_addr)->sin6_scope_id;
      return;
#endif
    case AF_INET: 
      if (ai_addrlen < (int)sizeof(sockaddr_in)) {
        PTRACE(1, "sockaddr size too small (" << ai_addrlen << ")  for family " << ai_family);
        break;
      }

      m_version = 4;
      m_v.m_four = ((struct sockaddr_in  *)ai_addr)->sin_addr;
      m_scope6   = 0;
      return;

    default :
      PTRACE(1, "Illegal family (" << ai_family << ") specified.");
  }

  m_version = 0;
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
  m_version = 4;
  m_v.m_four  = addr;
  return *this;
}


PObject::Comparison PIPSocket::Address::Compare(const PObject & obj) const
{
  const PIPSocket::Address & other = (const PIPSocket::Address &)obj;

  if (m_version < other.m_version)
    return LessThan;
  if (m_version > other.m_version)
    return GreaterThan;

#if P_HAS_IPV6
  if (m_version == 6) {
    bool isLinkLocal = IN6_IS_ADDR_LINKLOCAL(&m_v.m_six);
    bool otherIsLinkLocal = IN6_IS_ADDR_LINKLOCAL(&other.m_v.m_six);

    if (isLinkLocal != otherIsLinkLocal) {
      if (isLinkLocal)
        return LessThan;
      else return GreaterThan;
    }

    int result = memcmp(&m_v.m_six, &other.m_v.m_six, sizeof(m_v.m_six));
    if (result < 0)
      return LessThan;
    if (result > 0)
      return GreaterThan;
    return EqualTo;
  }
#endif

  if ((DWORD)*this < other)
    return LessThan;
  if ((DWORD)*this > other)
    return GreaterThan;
  return EqualTo;
}


bool PIPSocket::Address::operator==(in_addr & addr) const
{
  PIPSocket::Address a(addr);
  return Compare(a) == EqualTo;
}


bool PIPSocket::Address::operator==(DWORD dw) const
{
  if (dw == INADDR_ANY)
    return IsAny();

  return m_version == 4 && ((DWORD)*this == dw);
}


PIPSocket::Address & PIPSocket::Address::operator=(const PString & dotNotation)
{
  FromString(dotNotation);
  return *this;
}


PString PIPSocket::Address::AsString(bool IPV6_PARAM(bracketIPv6),
                                     bool IPV6_PARAM(excludeScope)) const
{
#if defined(P_VXWORKS)
  char ipStorage[INET_ADDR_LEN];
  inet_ntoa_b(v.four, ipStorage);
  return ipStorage;    
#else
# if defined(P_HAS_IPV6)
  if (m_version == 6) {
    if (IsAny())
      return bracketIPv6 ? "[::]" : "::";
    char str[INET6_ADDRSTRLEN+3];
    Psockaddr sa(*this, 0);
    PAssertOS(getnameinfo(sa, sa.GetSize(), &str[bracketIPv6], INET6_ADDRSTRLEN, NULL, 0, NI_NUMERICHOST) == 0);
    if (bracketIPv6) {
      str[0] = '[';
      int len = strlen(str);
      str[len++] = ']';
      str[len] = '\0';
    }
    if (excludeScope) {
      char * percent = strchr(str, '%');
      if (percent != NULL) {
        if (bracketIPv6)
          *percent++ = ']';
        *percent = '\0';
      }
    }
    return str;
  }
#endif // P_HAS_IPV6
# if defined(P_HAS_INET_NTOP)
  char str[INET_ADDRSTRLEN+1];
  if (inet_ntop(AF_INET, (const void *)&m_v.m_four, str, INET_ADDRSTRLEN) == NULL)
    return PString::Empty();
  return str;
# else
  static PCriticalSection x;
  PWaitAndSignal m(x);
  return inet_ntoa(m_v.m_four);
#endif // P_HAS_INET_NTOP
#endif // P_VXWORKS
}


PBoolean PIPSocket::Address::FromString(const PString & str)
{
  m_version = 0;
  memset(&m_v, 0, sizeof(m_v));
  if (str.IsEmpty())
    return false;

#if P_HAS_IPV6
  m_scope6 = 0;

  struct addrinfo *res = NULL;
  struct addrinfo hints = { AI_NUMERICHOST, PF_UNSPEC }; // Could be IPv4: x.x.x.x or IPv6: x:x:x:x::x

  // Find out if string is in brackets [], as in ipv6 address
  PINDEX lbracket = str.Find('[');
  PINDEX rbracket = str.Find(']', lbracket);
  if (lbracket == P_MAX_INDEX || rbracket == P_MAX_INDEX)
    getaddrinfo((const char *)str, NULL , &hints, &res);
  else {
    PString ip6 = str(lbracket+1, rbracket-1);
    if (getaddrinfo((const char *)ip6, NULL , &hints, &res) != 0) {
      PINDEX percent = ip6.Find('%');
      if (percent > 0 && percent != P_MAX_INDEX) {
        // If have a scope qualifier, remove it as it might be for the remote system
        ip6.Delete(percent, P_MAX_INDEX);
        getaddrinfo((const char *)ip6, NULL , &hints, &res);
      }
    }
  }

  if (res != NULL) {
    if (res->ai_family == PF_INET6) {
      // IPv6 addr
      struct sockaddr_in6 * addr_in6 = (struct sockaddr_in6 *)res->ai_addr;
      m_version = 6;
      m_v.m_six = addr_in6->sin6_addr;
      m_scope6  = addr_in6->sin6_scope_id;
    }
    else {
      // IPv4 addr
      struct sockaddr_in * addr_in = (struct sockaddr_in *)res->ai_addr;
      m_version  = 4;
      m_v.m_four = addr_in->sin_addr;
    }

    freeaddrinfo(res);
    return true;
  }

  // Failed to parse, so check for IPv4 with %interface
#endif // P_HAS_IPV6

  PINDEX percent = str.FindSpan("0123456789.");
  if (percent != P_MAX_INDEX && str[percent] != '%')
    return false;

  if (percent > 0) {
    PString ip4 = str.Left(percent);
    DWORD iaddr;
    if ((iaddr = ::inet_addr((const char *)ip4)) != (DWORD)INADDR_NONE) {
      m_version = 4;
      m_v.m_four.s_addr = iaddr;
      return true;
    }
  }

  PString iface = str.Mid(percent+1);
  if (iface.IsEmpty())
    return false;

  PIPSocket::InterfaceTable interfaceTable;
  if (!PIPSocket::GetInterfaceTable(interfaceTable))
    return false;

  for (PINDEX i = 0; i < interfaceTable.GetSize(); i++) {
    if (interfaceTable[i].GetName().NumCompare(iface) == EqualTo) {
      *this = interfaceTable[i].GetAddress();
      return true;
    }
  }

  return false;
}


PIPSocket::Address::operator PString() const
{
  return AsString();
}


PIPSocket::Address::operator in_addr() const
{
#if P_HAS_IPV6
  if (m_version == 6)
    return IsV4Mapped() ? Address((*this)[12], (*this)[13], (*this)[14], (*this)[15]) : GetAny(4);
#endif

  return m_v.m_four;
}


BYTE PIPSocket::Address::operator[](PINDEX idx) const
{
  PASSERTINDEX(idx);
#if P_HAS_IPV6
  if (m_version == 6) {
    PAssert(idx <= 15, PInvalidParameter);
    return m_v.m_six.s6_addr[idx];
  }
#endif

  PAssert(idx <= 3, PInvalidParameter);
  return ((BYTE *)&m_v.m_four)[idx];
}


ostream & operator<<(ostream & s, const PIPSocket::Address & a)
{
  return s << a.AsString((s.flags()&ios::hex) != 0, (s.flags()&ios::fixed) != 0);
}

istream & operator>>(istream & s, PIPSocket::Address & a)
{
/// Not IPv6 ready !!!!!!!!!!!!!
  char dot1, dot2, dot3;
  unsigned b1, b2, b3, b4;
  s >> b1;
  if (!s.fail()) {
    if (s.peek() != '.')
      a = htonl(b1);
    else {
      s >> dot1 >> b2 >> dot2 >> b3 >> dot3 >> b4;
      if (!s.fail() && dot1 == '.' && dot2 == '.' && dot3 == '.')
        a = PIPSocket::Address((BYTE)b1, (BYTE)b2, (BYTE)b3, (BYTE)b4);
    }
  }
  return s;
}


PINDEX PIPSocket::Address::GetSize() const
{
  switch (m_version) {
#if P_HAS_IPV6
    case 6 :
      return 16;
#endif

    case 4 :
      return 4;
  }

  return 0;
}


bool PIPSocket::Address::IsAny() const
{
  switch (m_version) {
#if P_HAS_IPV6
    case 6 :
      return memcmp(&m_v.m_six, &any6.m_v.m_six, sizeof(m_v.m_six)) == 0;
#endif

    case 4 :
      return (DWORD)*this == INADDR_ANY;
  }
  return false;
}


bool PIPSocket::Address::IsLoopback() const
{
#if P_HAS_IPV6
  if (m_version == 6)
    return IN6_IS_ADDR_LOOPBACK(&m_v.m_six);
#endif
  return Byte1() == 127;
}


bool PIPSocket::Address::IsBroadcast() const
{
#if P_HAS_IPV6
  if (m_version == 6) // In IPv6, no broadcast exist. Only multicast
    return *this == broadcast6; // multicast address as as substitute
#endif

  return *this == broadcast4;
}


bool PIPSocket::Address::IsMulticast() const
{
#if P_HAS_IPV6
  if (m_version == 6)
    return IN6_IS_ADDR_MULTICAST(&m_v.m_six);
#endif

  return IN_MULTICAST(ntohl(m_v.m_four.s_addr));
}


bool PIPSocket::Address::IsRFC1918() const 
{
  PINDEX offset = 0;
#if P_HAS_IPV6
  if (m_version == 6) {
    if (IN6_IS_ADDR_LINKLOCAL(&m_v.m_six))
      return true;
    if (IN6_IS_ADDR_SITELOCAL(&m_v.m_six))
      return true;
    if (!IsV4Mapped())
      return false;
    offset = 12;
  }
#endif

  BYTE b1 = (*this)[offset];
  BYTE b2 = (*this)[offset+1];
  return (b1 == 10)
          ||
          (
            (b1 == 172)
            &&
            (b2 >= 16) && (b2 <= 31)
          )
          ||
          (
            (b1 == 192) 
            &&
            (b2 == 168)
          );
}


PIPSocket::InterfaceEntry::InterfaceEntry()
  : m_ipAddress(PIPSocket::GetInvalidAddress())
  , m_netMask(PIPSocket::GetInvalidAddress())
{
}

PIPSocket::InterfaceEntry::InterfaceEntry(const PString & name,
                                          const Address & address,
                                          const Address & mask,
                                          const PString & macAddress)
  : m_name(name.Trim())
  , m_ipAddress(address)
  , m_netMask(mask)
  , m_macAddress(macAddress)
{
  SanitiseName(m_name);
}


void PIPSocket::InterfaceEntry::SanitiseName(PString & name)
{
  /* HACK!!
     At various points in PTLib (and OPAL) the interface name is used in
     situations where there can be confusion in parsing. For example a URL can
     be http:://[::%interface]:2345 and a ] in interface name blows it to
     pieces. Similarly, "ip:port" style description which can be used a LOT,
     will also fail. At this late stage it is too hard to change all the other
     places, so we hack the fairly rare cases by translating those special
     characters.
   */ 
  name.Replace('[', '{', true);
  name.Replace(']', '}', true);
  name.Replace(':', ';', true);
}


void PIPSocket::InterfaceEntry::PrintOn(ostream & strm) const
{
  strm << m_ipAddress;
  if (!m_macAddress)
    strm << " <" << m_macAddress << '>';
  if (!m_name)
    strm << " (" << m_name << ')';
}


#ifdef __NUCLEUS_NET__
PBoolean PIPSocket::GetInterfaceTable(InterfaceTable & table)
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


PBoolean PIPSocket::GetNetworkInterface(PIPSocket::Address & addr)
{
  PIPSocket::InterfaceTable interfaceTable;
  if (PIPSocket::GetInterfaceTable(interfaceTable)) {
    PINDEX i;
    for (i = 0; i < interfaceTable.GetSize(); ++i) {
      PIPSocket::Address localAddr = interfaceTable[i].GetAddress();
      if (!localAddr.IsLoopback() && (!localAddr.IsRFC1918() || !addr.IsRFC1918()))
        addr = localAddr;
    }
  }
  return addr.IsValid();
}


PIPSocket::Address PIPSocket::GetRouteInterfaceAddress(PIPSocket::Address remoteAddress)
{
  PIPSocket::InterfaceTable hostInterfaceTable;
  PIPSocket::GetInterfaceTable(hostInterfaceTable);

  PIPSocket::RouteTable hostRouteTable;
  PIPSocket::GetRouteTable(hostRouteTable);

  if (hostInterfaceTable.IsEmpty())
    return PIPSocket::GetDefaultIpAny();

  for (PINDEX IfaceIdx = 0; IfaceIdx < hostInterfaceTable.GetSize(); IfaceIdx++) {
    if (remoteAddress == hostInterfaceTable[IfaceIdx].GetAddress()) {
      PTRACE(5, NULL, NULL, "Route packet for " << remoteAddress
              << " over interface " << hostInterfaceTable[IfaceIdx].GetName()
              << "[" << hostInterfaceTable[IfaceIdx].GetAddress() << "]");
      return hostInterfaceTable[IfaceIdx].GetAddress();
    }
  }

  PIPSocket::RouteEntry * route = NULL;
  for (PINDEX routeIdx = 0; routeIdx < hostRouteTable.GetSize(); routeIdx++) {
    PIPSocket::RouteEntry & routeEntry = hostRouteTable[routeIdx];

    DWORD network = (DWORD) routeEntry.GetNetwork();
    DWORD mask = (DWORD) routeEntry.GetNetMask();

    if (((DWORD)remoteAddress & mask) == network) {
      if (route == NULL)
        route = &routeEntry;
      else if ((DWORD)routeEntry.GetNetMask() > (DWORD)route->GetNetMask())
        route = &routeEntry;
    }
  }

  if (route != NULL) {
    for (PINDEX IfaceIdx = 0; IfaceIdx < hostInterfaceTable.GetSize(); IfaceIdx++) {
      if (route->GetInterface() == hostInterfaceTable[IfaceIdx].GetName()) {
        PTRACE(5, NULL, NULL, "Route packet for " << remoteAddress
                << " over interface " << hostInterfaceTable[IfaceIdx].GetName()
                << "[" << hostInterfaceTable[IfaceIdx].GetAddress() << "]");
        return hostInterfaceTable[IfaceIdx].GetAddress();
      }
    }
  }

  return PIPSocket::GetDefaultIpAny();
}


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


// By default IPv4 only adresses
PBoolean PTCPSocket::OpenSocket()
{
  return ConvertOSError(os_handle = os_socket(AF_INET, SOCK_STREAM, 0));
}


// ipAdressFamily should be AF_INET or AF_INET6
PBoolean PTCPSocket::OpenSocket(int ipAdressFamily) 
{
  return ConvertOSError(os_handle = os_socket(ipAdressFamily, SOCK_STREAM, 0));
}


const char * PTCPSocket::GetProtocolName() const
{
  return "tcp";
}


PBoolean PTCPSocket::Write(const void * buf, PINDEX len)
{
  flush();
  PINDEX writeCount = 0;

  while (len > 0) {
    Slice slice(((char *)buf)+writeCount, len);
    if (!os_vwrite(&slice, 1, 0, NULL, 0))
      return false;
    writeCount += lastWriteCount;
    len -= lastWriteCount;
  }

  lastWriteCount = writeCount;
  return true;
}


bool PTCPSocket::InternalListen(const Address & bindAddr,
                                unsigned queueSize,
                                WORD newPort,
                                Reusability reuse)
{
  if (!PIPSocket::InternalListen(bindAddr, queueSize, newPort, reuse))
    return false;

  if (ConvertOSError(::listen(os_handle, queueSize)))
    return true;

  os_close();
  return false;
}


PBoolean PTCPSocket::Accept(PSocket & socket)
{
  PAssert(PIsDescendant(&socket, PIPSocket), "Invalid listener socket");

  Psockaddr sa;
  socklen_t size = sa.GetSize();
  if (!os_accept(socket, sa, &size))
    return false;
    
  port = ((PIPSocket &)socket).GetPort(); 
  return true;
}


PBoolean PTCPSocket::WriteOutOfBand(void const * buf, PINDEX len)
{
#ifdef __NUCLEUS_NET__
  PAssertAlways("WriteOutOfBand unavailable on Nucleus Plus");
  //int count = NU_Send(os_handle, (char *)buf, len, 0);
  int count = ::send(os_handle, (const char *)buf, len, 0);
#elif defined(P_VXWORKS)
  int count = ::send(os_handle, (char *)buf, len, MSG_OOB);
#else
  int count = ::send(os_handle, (const char *)buf, len, MSG_OOB);
#endif
  if (count < 0) {
    lastWriteCount = 0;
    return ConvertOSError(count, LastWriteError);
  }
  else {
    lastWriteCount = count;
    return true;
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


bool PIPDatagramSocket::ReadFrom(void * buf, PINDEX len, Address & addr, WORD & port)
{
  PIPSocketAddressAndPort ap;
  Slice slice(buf, len); 
  bool stat = InternalReadFrom(&slice, 1, ap);
  addr = ap.GetAddress();
  port = ap.GetPort();
  return stat;
}

bool PIPDatagramSocket::ReadFrom(void * buf, PINDEX len, PIPSocketAddressAndPort & ipAndPort)
{
  Slice slice(buf, len); 
  return InternalReadFrom(&slice, 1, ipAndPort);
}


bool PIPDatagramSocket::ReadFrom(Slice * slices, size_t sliceCount, Address & addr, WORD & port)
{
  PIPSocketAddressAndPort ap;
  bool stat = InternalReadFrom(slices, sliceCount, ap);
  addr = ap.GetAddress();
  port = ap.GetPort();
  return stat;
}

bool PIPDatagramSocket::ReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort)
{
  return InternalReadFrom(slices, sliceCount, ipAndPort);
}

bool PIPDatagramSocket::InternalReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort)
{
  lastReadCount = 0;

  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

  Psockaddr sa;
  socklen_t size = sa.GetSize();
  if (!os_vread(slices, sliceCount, 0, sa, &size))
    return false;

  ipAndPort.SetAddress(sa.GetIP());
  ipAndPort.SetPort(sa.GetPort());

  return true;
}


bool PIPDatagramSocket::WriteTo(const void * buf, PINDEX len, const Address & addr, WORD port)
{
  PIPSocketAddressAndPort ap(addr, port);
  Slice slice((void *)buf, (size_t)len); 
  return InternalWriteTo(&slice, 1, ap);
}


bool PIPDatagramSocket::WriteTo(const void * buf, PINDEX len, const PIPSocketAddressAndPort & ipAndPort)
{
  Slice slice((void *)buf, len); 
  return InternalWriteTo(&slice, 1, ipAndPort);
}


bool PIPDatagramSocket::WriteTo(const Slice * slices, size_t sliceCount, const Address & addr, WORD port)
{
  PIPSocketAddressAndPort ap(addr, port);
  return InternalWriteTo(slices, sliceCount, ap);
}


bool PIPDatagramSocket::WriteTo(const Slice * slices, size_t sliceCount, const PIPSocketAddressAndPort & ipAndPort)
{
  return InternalWriteTo(slices, sliceCount, ipAndPort);
}


bool PIPDatagramSocket::InternalWriteTo(const Slice * slices, size_t sliceCount, const PIPSocketAddressAndPort & ipAndPort)
{
  lastWriteCount = 0;

  const PIPSocket::Address & addr = ipAndPort.GetAddress();
  WORD port = ipAndPort.GetPort();

  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

  PBoolean broadcast = addr.IsAny() || addr.IsBroadcast();
  if (broadcast) {
#ifdef P_BEOS
    PAssertAlways("Broadcast option under BeOS is not implemented yet");
    return false;
#else
    if (!SetOption(SO_BROADCAST, 1))
      return false;
#endif
  }
  
#ifdef P_MACOSX
  // Mac OS X does not treat 255.255.255.255 as a broadcast address (multihoming / ambiguity issues)
  // and such packets aren't sent to the layer 2 - broadcast address (i.e. ethernet ff:ff:ff:ff:ff:ff)
  // instead: send subnet broadcasts on all interfaces
  
  // TODO: Add IPv6 support
  
  PBoolean ok = false;
  if (broadcast) {
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(port);
    
    InterfaceTable interfaces;
    if (GetInterfaceTable(interfaces)) {
      for (int i = 0; i < interfaces.GetSize(); i++) {
        InterfaceEntry & iface = interfaces[i];
        if (iface.GetAddress().IsLoopback()) {
          continue;
        }
        // create network address from address / netmask
        DWORD ifAddr = iface.GetAddress();
        DWORD netmask = iface.GetNetMask();
        DWORD bcastAddr = (ifAddr & netmask) + ~netmask;
        
        sockAddr.sin_addr.s_addr = bcastAddr;
        
        bool result = os_vwrite(slices, sliceCount, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr) != 0);
        
        ok = ok || result;
      }
    }
    
  } else {
    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_addr = addr;
    sockAddr.sin_port = htons(port);
    ok = os_vwrite(slices, sliceCount, 0, (struct sockaddr *)&sockAddr, sizeof(sockAddr)) != 0;
  }
  
#else
  
  Psockaddr sa(broadcast ? Address::GetBroadcast(addr.GetVersion()) : addr, port);
  bool ok = os_vwrite(slices, sliceCount, 0, sa, sa.GetSize());
  
#endif // P_MACOSX

#ifndef P_BEOS
  if (broadcast)
    SetOption(SO_BROADCAST, 0);
#endif

  return ok;
}


//////////////////////////////////////////////////////////////////////////////
// PUDPSocket

PUDPSocket::PUDPSocket(WORD newPort, int iAddressFamily)
{
  m_sendPort = 0;
  SetPort(newPort);
  OpenSocket(iAddressFamily);
}

PUDPSocket::PUDPSocket(PQoS * qos, WORD newPort, int iAddressFamily)
#if P_HAS_IPV6
  : m_sendAddress(iAddressFamily == AF_INET ? loopback4 : loopback6),
    m_lastReceiveAddress(iAddressFamily == AF_INET ? loopback4 : loopback6)
#else
  : m_sendAddress(loopback4),
    m_lastReceiveAddress(loopback4)
#endif
{
#if P_QOS
  if (qos != NULL)
      qosSpec = *qos;
#endif
  m_sendPort = 0;
  SetPort(newPort);
  OpenSocket(iAddressFamily);
}


PUDPSocket::PUDPSocket(const PString & service, PQoS * qos, int iAddressFamily)
#if P_HAS_IPV6
  : m_sendAddress(iAddressFamily == AF_INET ? loopback4 : loopback6),
    m_lastReceiveAddress(iAddressFamily == AF_INET ? loopback4 : loopback6)
#else
  : m_sendAddress(loopback4),
    m_lastReceiveAddress(loopback4)
#endif
{
#if P_QOS
  if (qos != NULL)
      qosSpec = *qos;
#endif
  m_sendPort = 0;
  SetPort(service);
  OpenSocket(iAddressFamily);
}


PUDPSocket::PUDPSocket(const PString & address, WORD newPort)
{
  SetSendAddress(PIPSocketAddressAndPort());
  SetPort(newPort);
  Connect(address);
}


PUDPSocket::PUDPSocket(const PString & address, const PString & service)
{
  SetSendAddress(PIPSocketAddressAndPort());
  SetPort(service);
  Connect(address);
}

#if P_QOS

PBoolean PUDPSocket::ModifyQoSSpec(PQoS * qos)
{
  if (qos==NULL)
    return false;

  qosSpec = *qos;
  return true;
}


PQoS & PUDPSocket::GetQoSSpec()
{
  return qosSpec;
}

#endif

PBoolean PUDPSocket::ApplyQoS()
{
#if P_QOS
  char DSCPval = 0;
#ifndef _WIN32_WCE
  if (qosSpec.GetDSCP() < 0 ||
      qosSpec.GetDSCP() > 63) {
    if (qosSpec.GetServiceType() == SERVICETYPE_PNOTDEFINED)
      return true;
    else {
      switch (qosSpec.GetServiceType()) {
        case SERVICETYPE_GUARANTEED:
          DSCPval = PQoS::guaranteedDSCP;
          break;
        case SERVICETYPE_CONTROLLEDLOAD:
          DSCPval = PQoS::controlledLoadDSCP;
          break;
        case SERVICETYPE_BESTEFFORT:
        default:
          DSCPval = PQoS::bestEffortDSCP;
          break;
      }
    }
  }
  else
    DSCPval = (char)qosSpec.GetDSCP();
#else
  DSCPval = 0x38;
  disableGQoS = false;
#endif

#ifdef _WIN32
  if (disableGQoS)
    return false;

#ifndef _WIN32_WCE
  PBoolean usesetsockopt = false;

  OSVERSIONINFO versInfo;
  ZeroMemory(&versInfo,sizeof(OSVERSIONINFO));
  versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (!(GetVersionEx(&versInfo)))
    usesetsockopt = true;
  else {
    if (versInfo.dwMajorVersion < 5)
      usesetsockopt = true;

    if (disableGQoS)
          return false;

    PBoolean usesetsockopt = false;

    if (versInfo.dwMajorVersion == 5 &&
        versInfo.dwMinorVersion == 0)
      usesetsockopt = true;         //Windows 2000 does not always support QOS_DESTADDR
  }
#else
  PBoolean usesetsockopt = true;
#endif

  PBoolean retval = false;
  PIPSocketAddressAndPort sendAp;
  if (!usesetsockopt && sendAp.IsValid() && sendAp.GetPort() != 0) {
    sockaddr_in sa;
    sa.sin_family = AF_INET;
    sa.sin_port = htons(sendAp.GetPort());
    sa.sin_addr = sendAp.GetAddress();
    memset(sa.sin_zero,0,8);

    char * inBuf = new char[2048];
    memset(inBuf,0,2048);
    DWORD bufLen = 0;
    PWinQoS wqos(qosSpec, (struct sockaddr *)(&sa), inBuf, bufLen);

    DWORD dummy = 0;
    int irval = WSAIoctl(os_handle, SIO_SET_QOS, inBuf, bufLen, NULL, 0, &dummy, NULL, NULL);

    delete[] inBuf;

    return irval == 0;
  }

  if (!usesetsockopt)
    return retval;

#endif  // _WIN32

  unsigned int setDSCP = DSCPval<<2;

  int rv = 0;
  unsigned int curval = 0;
  socklen_t cursize = sizeof(curval);
  rv = ::getsockopt(os_handle,IPPROTO_IP, IP_TOS, (char *)(&curval), &cursize);
  if (curval == setDSCP)
    return true;    //Required DSCP already set


  rv = ::setsockopt(os_handle, IPPROTO_IP, IP_TOS, (char *)&setDSCP, sizeof(setDSCP));

  if (rv != 0) {
    int err;
#ifdef _WIN32
    err = WSAGetLastError();
#else
    err = errno;
#endif
    PTRACE(1, "QOS", "setsockopt failed with code " << err);
    return false;
  }
#endif  // P_QOS

  return true;
}


PBoolean PUDPSocket::OpenSocketGQOS(int af, int type, int proto)
{
#if defined(P_QOS) && defined(_WIN32)
    
  //Try to find a QOS-enabled protocol
  DWORD bufferSize = 0;
  DWORD numProtocols = WSAEnumProtocols(proto != 0 ? &proto : NULL, NULL, &bufferSize);
  if (!ConvertOSError(numProtocols) && WSAGetLastError() != WSAENOBUFS) 
    return false;

  LPWSAPROTOCOL_INFO installedProtocols = (LPWSAPROTOCOL_INFO)(new BYTE[bufferSize]);
  numProtocols = WSAEnumProtocols(proto != 0 ? &proto : NULL, installedProtocols, &bufferSize);
  if (!ConvertOSError(numProtocols)) {
    delete[] installedProtocols;
    return false;
  }

  LPWSAPROTOCOL_INFO qosProtocol = installedProtocols;
  for (DWORD i = 0; i < numProtocols; qosProtocol++, i++) {
    if ((qosProtocol->dwServiceFlags1 & XP1_QOS_SUPPORTED) &&
        (qosProtocol->iSocketType == type) &&
        (qosProtocol->iAddressFamily == af)) {
      os_handle = (int)WSASocket(af, type, proto, qosProtocol, 0, WSA_FLAG_OVERLAPPED);
      break;
    }
  }

  delete[] installedProtocols;

  if (!IsOpen())
    os_handle = (int)WSASocket(af, type, proto, NULL, 0, WSA_FLAG_OVERLAPPED);

  return ConvertOSError(os_handle);

#else  // P_QOS

  return ConvertOSError(os_handle = os_socket(af, type, proto));

#endif
}


#ifdef _WIN32

#define COULD_HAVE_QOS

static PBoolean CheckOSVersionFor(DWORD major, DWORD minor)
{
  OSVERSIONINFO versInfo;
  ZeroMemory(&versInfo,sizeof(OSVERSIONINFO));
  versInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
  if (GetVersionEx(&versInfo)) {
    if (versInfo.dwMajorVersion > major ||
       (versInfo.dwMajorVersion == major && versInfo.dwMinorVersion >= minor))
      return true;
  }
  return false;
}

#endif // _WIN32


PBoolean PUDPSocket::OpenSocket()
{
  return OpenSocket(PF_INET);
}


PBoolean PUDPSocket::OpenSocket(int ipAdressFamily)
{
#ifdef COULD_HAVE_QOS
  if (CheckOSVersionFor(5,1)) 
    return OpenSocketGQOS(ipAdressFamily, SOCK_DGRAM, 0);
#endif

  return ConvertOSError(os_handle = os_socket(ipAdressFamily,SOCK_DGRAM, 0));
}


const char * PUDPSocket::GetProtocolName() const
{
  return "udp";
}


PBoolean PUDPSocket::Connect(const PString & address)
{
  SetSendAddress(PIPSocketAddressAndPort());
  return PIPDatagramSocket::Connect(address);
}


bool PUDPSocket::InternalListen(const Address & bindAddr,
                                unsigned queueSize,
                                WORD newPort,
                                Reusability reuse)
{
  if (!bindAddr.IsMulticast())
    return PIPSocket::InternalListen(bindAddr, queueSize, newPort, reuse);

  if (!PIPSocket::InternalListen(Address::GetAny(bindAddr.GetVersion()), queueSize, newPort, CanReuseAddress))
    return false;

  bool ok;

#if defined(P_HAS_IPV6) && defined(P_HAS_IPV6_ADD_MEMBERSHIP)
  if (bindAddr.GetVersion() == 6) {
    struct ipv6_mreq mreq;
    mreq.ipv6mr_multiaddr = bindAddr;
    mreq.ipv6mr_interface = bindAddr.GetIPV6Scope();
    ok = SetOption(IPV6_ADD_MEMBERSHIP, &mreq, sizeof(mreq), IPPROTO_IPV6);
  }
  else
#endif
  {
    struct ip_mreq mreq;
    mreq.imr_multiaddr = bindAddr;
    mreq.imr_interface = Address::GetAny(4);
    ok = SetOption(IP_ADD_MEMBERSHIP, &mreq, sizeof(mreq), IPPROTO_IP);
  }

  if (!ok) {
    PTRACE(1, "Multicast join failed for " << bindAddr << " - " << GetErrorText());
    os_close();
    return false;
  }

  PTRACE(4, "Joined multicast group " << bindAddr);
  return true;
}


PBoolean PUDPSocket::Read(void * buf, PINDEX len)
{
  PIPSocketAddressAndPort ap;
  bool stat = PIPDatagramSocket::ReadFrom(buf, len, ap);
  InternalSetLastReceiveAddress(ap);
  return stat;
}


PBoolean PUDPSocket::Write(const void * buf, PINDEX len)
{
  PIPSocketAddressAndPort ap;
  GetSendAddress(ap);
  Slice slice((void *)buf, len);
  return PIPDatagramSocket::InternalWriteTo(&slice, 1, ap);
}


void PUDPSocket::SetSendAddress(const Address & newAddress, WORD newPort)
{
  InternalSetSendAddress(PIPSocketAddressAndPort(newAddress, newPort));
}


void PUDPSocket::SetSendAddress(const PIPSocketAddressAndPort & addressAndPort)
{
  InternalSetSendAddress(addressAndPort);
}


void PUDPSocket::InternalSetSendAddress(const PIPSocketAddressAndPort & addr)
{
  m_sendAddress = addr.GetAddress();
  m_sendPort    = addr.GetPort();
  ApplyQoS();
}


void PUDPSocket::GetSendAddress(Address & address, WORD & port) const
{
  PIPSocketAddressAndPort addr;
  InternalGetSendAddress(addr);
  address = addr.GetAddress();
  port    = addr.GetPort();
}


void PUDPSocket::GetSendAddress(PIPSocketAddressAndPort & addr) const
{
  InternalGetSendAddress(addr);
}


PString PUDPSocket::GetSendAddress() const
{
  return m_sendAddress.AsString(true) + psprintf(":%u", m_sendPort);
}


void PUDPSocket::InternalGetSendAddress(PIPSocketAddressAndPort & addr) const
{
  addr = PIPSocketAddressAndPort(m_sendAddress, m_sendPort);
}


void PUDPSocket::GetLastReceiveAddress(Address & address, WORD & port) const
{
  PIPSocketAddressAndPort ap;
  InternalGetLastReceiveAddress(ap);
  address = ap.GetAddress();
  port    = ap.GetPort();
}


void PUDPSocket::GetLastReceiveAddress(PIPSocketAddressAndPort & ap) const
{
  InternalGetLastReceiveAddress(ap);
}


PString PUDPSocket::GetLastReceiveAddress() const
{
  return m_lastReceiveAddress.AsString(true) + psprintf(":%u", m_lastReceivePort);
}


void PUDPSocket::InternalGetLastReceiveAddress(PIPSocketAddressAndPort & ap) const
{
  ap = PIPSocketAddressAndPort(m_lastReceiveAddress, m_lastReceivePort);
}


void PUDPSocket::InternalSetLastReceiveAddress(const PIPSocketAddressAndPort & ap)
{
  m_lastReceiveAddress = ap.GetAddress();
  m_lastReceivePort    = ap.GetPort();
}


PBoolean PUDPSocket::IsAlternateAddress(const Address &, WORD)
{
  return false;
}

PBoolean PUDPSocket::DoPseudoRead(int & /*selectStatus*/)
{
   return false;
}

//////////////////////////////////////////////////////////////////////////////

PBoolean PICMPSocket::OpenSocket(int)
{
  return false;
}

//////////////////////////////////////////////////////////////////////////////

PIPSocketAddressAndPort::PIPSocketAddressAndPort(char separator)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(0)
  , m_separator(separator)
{
}


PIPSocketAddressAndPort::PIPSocketAddressAndPort(WORD defaultPort, char separator)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(defaultPort)
  , m_separator(separator)
{
}


PIPSocketAddressAndPort::PIPSocketAddressAndPort(const PString & str, WORD defaultPort, char separator, const char * proto)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(defaultPort)
  , m_separator(separator)
{
  Parse(str, defaultPort, m_separator, proto);
}


PIPSocketAddressAndPort::PIPSocketAddressAndPort(const PIPSocket::Address & addr, WORD defaultPort, char separator)
  : m_address(addr)
  , m_port(defaultPort)
  , m_separator(separator)
{
}


PIPSocketAddressAndPort::PIPSocketAddressAndPort(struct sockaddr *ai_addr, const int ai_addrlen)
  : m_address(ai_addr->sa_family, ai_addrlen, ai_addr)
  , m_port(ntohs((ai_addr->sa_family == AF_INET) ? ((sockaddr_in *)ai_addr)->sin_port : ((sockaddr_in6 *)ai_addr)->sin6_port))
  , m_separator(':')
{  
}


PBoolean PIPSocketAddressAndPort::Parse(const PString & str, WORD port, char separator, const char * proto)
{
  if (separator != '\0')
    m_separator = separator;

  PINDEX pos = 0;
  if (str[0] == '[')
    pos = str.Find(']');
  pos = str.Find(m_separator, pos);
  if (pos != P_MAX_INDEX)
    port = PIPSocket::GetPortByService(proto, str.Mid(pos+1));

  if (port != 0)
    m_port = port;

  return PIPSocket::GetHostAddress(str.Left(pos), m_address) && m_port != 0;
}


PString PIPSocketAddressAndPort::AsString(char separator) const
{
  PString str;

  if (m_address.IsValid())
    str = m_address.AsString(true, true);

  if (m_port != 0)
    str.sprintf("%c%u", separator ? separator : m_separator, m_port);

  return str;
}


void PIPSocketAddressAndPort::SetAddress(const PIPSocket::Address & addr, WORD port)
{
  m_address = addr;
  if (port != 0)
    m_port = port;
}


bool PIPSocketAddressAndPort::MatchWildcard(const PIPSocketAddressAndPort & wild) const
{
  return (!wild.m_address.IsValid() || wild.m_address == m_address) &&
         ( wild.m_port == 0         || wild.m_port    == m_port);
}


// End Of File ///////////////////////////////////////////////////////////////
