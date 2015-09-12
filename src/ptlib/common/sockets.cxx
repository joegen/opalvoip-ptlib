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
#include <ptlib/sockets.h>
#include <ptclib/random.h>

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


PIPSocket::sockaddr_wrapper::sockaddr_wrapper()
{
  Construct(invalid, 0);
}


PIPSocket::sockaddr_wrapper::sockaddr_wrapper(const PIPSocket::AddressAndPort & ipPort)
{
  Construct(ipPort.GetAddress(), ipPort.GetPort());
}


PIPSocket::sockaddr_wrapper::sockaddr_wrapper(const PIPSocket::Address & ip, WORD port)
{
  Construct(ip, port);
}


void PIPSocket::sockaddr_wrapper::Construct(const PIPSocket::Address & ip, WORD port)
{
  ptr = &storage;
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


socklen_t PIPSocket::sockaddr_wrapper::GetSize() const
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


PIPSocket::Address PIPSocket::sockaddr_wrapper::GetIP() const
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


WORD PIPSocket::sockaddr_wrapper::GetPort() const
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

#elif defined P_LINUX || defined(P_GNU_HURD) || defined(P_FREEBSD)

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

#elif (defined(P_PTHREADS) && !defined(P_THREAD_SAFE_LIBC)) || defined(__NUCLEUS_PLUS__) || defined (_WIN32)

#if defined(P_GETHOSTBYNAME_R)

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

#endif // P_GETHOSTBYNAME_R

#endif // Operating system

    mutex.Wait();

    if (localErrNo != NETDB_SUCCESS || retry == 0)
      return NULL;

    host = new PIPCacheData(host_info, addr.AsString());

    SetAt(key, host);
  }

  return host->GetHostAddress().IsValid() ? host : NULL;
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
  return ConvertOSError(::setsockopt(os_handle, level, option, (char *)&value, sizeof(value)));
}


PBoolean PSocket::SetOption(int option, const void * valuePtr, PINDEX valueSize, int level)
{
  return ConvertOSError(::setsockopt(os_handle, level, option, (char *)valuePtr, valueSize));
}


PBoolean PSocket::GetOption(int option, int & value, int level)
{
  socklen_t valSize = sizeof(value);
  return ConvertOSError(::getsockopt(os_handle, level, option, (char *)&value, &valSize));
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

  if (!sock1.ConvertOSError(Select(read, dummy1, dummy2, timeout)))
    return sock1.GetErrorCode();

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


PBoolean PSocket::ConvertOSError(P_INT_PTR libcReturnValue, ErrorGroup group)
{
  if (PChannel::ConvertOSError(libcReturnValue, group))
    return true;

  switch (lastErrorNumber[group]) {
    case ECONNRESET :
    case ECONNREFUSED :
    case EHOSTUNREACH :
    case ENETUNREACH :
      SetErrorValues(Unavailable, lastErrorNumber[group], group);
      break;

    case EMSGSIZE :
      return SetErrorValues(BufferTooSmall, lastErrorNumber[group], group);
  }

  return false;
}


//////////////////////////////////////////////////////////////////////////////
// PIPSocket

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
  PTRACE(4, &pHostByName(), "Cleared DNS cache.");
}


PString PIPSocket::GetName() const
{
  PStringStream str;
  str << GetProtocolName() << ':';

  AddressAndPort ap;
  if (GetLocalAddress(ap))
    str << " local=" << ap;
  if (GetPeerAddress(ap))
    str << " peer= " << ap;

  return str;
}


PString PIPSocket::GetHostName()
{
  char name[100];
  if (gethostname(name, sizeof(name)-1) != 0)
    return "localhost";
  name[sizeof(name)-1] = '\0';
  return name;
}


PBoolean PIPSocket::IsLocalHost(const PString & hostname)
{
  if (hostname.IsEmpty())
    return true;

  if (hostname *= "localhost")
    return true;

  Address addr(0);
  if (!GetHostAddress(hostname, addr))
    return false;

  if (addr.IsLoopback())  // Is 127.0.0.1 or ::1
    return true;

  InterfaceTable table;
  if (!GetInterfaceTable(table, true))
    return false;

  for (PINDEX i = 0; i < table.GetSize(); ++i) {
    if (table[i].GetAddress() == addr)
      return true;
  }

  return false;
}


PString PIPSocket::GetHostName(const PString & hostname)
{
  // lookup the host address using inet_addr, assuming it is a "." address
  Address temp(hostname);
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
  Address addr(hostname);
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


PString PIPSocket::GetLocalAddress() const
{
  PIPSocketAddressAndPort ap;
  return GetLocalAddress(ap) ? ap.AsString() : PString::Empty();
}


bool PIPSocket::GetLocalAddress(Address & addr) const
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetLocalAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  return true;
}


bool PIPSocket::GetLocalAddress(Address & addr, WORD & portNum) const
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetLocalAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  portNum = addrAndPort.GetPort();
  return true;
}


bool PIPSocket::GetLocalAddress(AddressAndPort & addr) const
{
  return const_cast<PIPSocket *>(this)->InternalGetLocalAddress(addr);
}


bool PIPSocket::InternalGetLocalAddress(PIPSocketAddressAndPort & addrAndPort)
{
  Address   peerv4;
  PIPSocket::sockaddr_wrapper sa;
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


PString PIPSocket::GetPeerAddress() const
{
  PIPSocketAddressAndPort ap;
  return GetPeerAddress(ap) ? ap.AsString() : PString::Empty();
}


bool PIPSocket::GetPeerAddress(Address & addr) const
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetPeerAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  return true;
}


bool PIPSocket::GetPeerAddress(Address & addr, WORD & portNum) const
{
  PIPSocketAddressAndPort addrAndPort;
  if (!GetPeerAddress(addrAndPort)) 
    return false;
  addr = addrAndPort.GetAddress();
  portNum = addrAndPort.GetPort();
  return true;
}


bool PIPSocket::GetPeerAddress(AddressAndPort & addr) const
{
  return const_cast<PIPSocket *>(this)->InternalGetPeerAddress(addr);
}


bool PIPSocket::InternalGetPeerAddress(PIPSocketAddressAndPort & addrAndPort)
{
  PIPSocket::sockaddr_wrapper sa;
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
  if (!addr.IsValid()) {
    PTRACE(2, "Cannot connect to invalid address");
    return false;
  }

  // close the port if it is already open
  if (IsOpen())
    Close();

  // make sure we have a port
  PAssert(port != 0, "Cannot connect socket without setting port");

  PIPSocket::sockaddr_wrapper sa(addr, port);

  // attempt to create a socket with the right family
  if (!OpenSocket(sa->sa_family)) {
    PTRACE(2, "Create failure: family=" << sa->sa_family << ", errno=" << GetErrorNumber() << ' ' << GetErrorText());
    return false;
  }

  if (localPort != 0 || (iface.IsValid() && !iface.IsAny())) {
    PIPSocket::sockaddr_wrapper bind_sa(iface, localPort);

    if (!SetOption(SO_REUSEADDR, 0)) {
      PTRACE(2, "setsockopt SO_REUSEADDR failure: errno=" << GetErrorNumber() << ' ' << GetErrorText());
      os_close();
      return false;
    }

    if (!ConvertOSError(::bind(os_handle, bind_sa, bind_sa.GetSize()))) {
      PTRACE(2, "bind failure: errno=" << GetErrorNumber() << ' ' << GetErrorText());
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
  if (!bindAddr.IsValid()) {
    PTRACE(2, "Cannot listen bound to invalid address");
    return false;
  }

  // make sure we have a port
  if (newPort != 0)
    port = newPort;

  PIPSocket::sockaddr_wrapper sa(bindAddr, port);

  // Always close and re-open as the bindAddr address family might change.
  os_close();

  // attempt to create a socket
  if (!OpenSocket(sa->sa_family)) {
    PTRACE(4, "OpenSocket for " << GetProtocolName() << " failed: " << GetErrorText());
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


//////////////////////////////////////////////////////////////////////////////

PIPSocket::QoS::QoS(QoSType type)
  : m_type(type)
  , m_dscp(-1)
  , m_remote(PIPSocket::GetInvalidAddress())
{
}


PIPSocket::QoS::QoS(const PString & str)
  : m_type(BestEffortQoS)
  , m_dscp(-1)
  , m_remote(PIPSocket::GetInvalidAddress())
{
  PStringStream strm(str);
  strm >> *this;
}


ostream & operator<<(ostream & strm, const PIPSocket::QoS & qos)
{
  if (qos.m_dscp >= 0)
    strm << "0x" << hex << qos.m_dscp << dec;
  else
    strm << 'C' << (int)qos.m_type;

  return strm;
}


istream & operator>>(istream & strm, PIPSocket::QoS & qos)
{
  if (strm.peek() != 'C')
    strm >> qos.m_dscp;
  else {
    strm.ignore(1);
    int i;
    strm >> i;
    qos.m_type = (PIPSocket::QoSType)i;
  }
  return strm;
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


const PIPSocket::Address & PIPSocket::Address::GetLoopback(unsigned IPV6_PARAM(version))
{
#if P_HAS_IPV6
  if (version == 6)
    return loopback6;
#endif
  return loopback4;
}


const PIPSocket::Address & PIPSocket::Address::GetAny(unsigned IPV6_PARAM(version))
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


const PIPSocket::Address PIPSocket::Address::GetBroadcast(unsigned IPV6_PARAM(version))
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


PIPSocket::Address::Address(PINDEX len, const BYTE * bytes, int 
#if P_HAS_IPV6
							scope
#endif
							)
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

  if (operator==((DWORD)other))
    return EqualTo;

  return ntohl(*this) < ntohl(other) ? LessThan : GreaterThan;
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
  if (m_version == 0)
    return PString::Empty();

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
    PIPSocket::sockaddr_wrapper sa(*this, 0);
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


bool PIPSocket::Address::IsSubNet(const Address & network, const Address & mask) const
{
  if (m_version != network.m_version || m_version != mask.m_version)
    return false;

#if P_HAS_IPV6
  if (m_version == 6) {
    for (PINDEX i = 0; i < 16; ++i) {
      if (((*this)[i] & mask[i]) != network[i])
        return false;
    }
    return true;
  }
#endif

  DWORD bitmask;
  if (!mask.IsAny())
    bitmask = mask;
  else if (network.Byte1() < 128)
    bitmask = inet_addr("255.0.0.0");
  else if (network.Byte1() < 192)
    bitmask = inet_addr("255.255.0.0");
  else if (network.Byte1() < 240)
    bitmask = inet_addr("255.255.255.0");
  else
    bitmask = inet_addr("255.255.255.255");

  return ((DWORD)*this & bitmask) == (DWORD)network;
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


void PIPSocket::RouteEntry::PrintOn(ostream & strm) const
{
  strm << network << '/' << net_mask;
  if (destination.IsValid())
    strm << " gw=" << destination;
  if (!interfaceName.IsEmpty())
    strm << " if=" << interfaceName;
  if (metric > 0)
    strm << " metric=" << metric;
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


PIPSocket::Address PIPSocket::GetInterfaceAddress(const PString & ifName, unsigned version)
{
  PIPSocket::InterfaceTable interfaceTable;
  if (PIPSocket::GetInterfaceTable(interfaceTable)) {
    for (PINDEX i = 0; i < interfaceTable.GetSize(); i++) {
      if (interfaceTable[i].GetName() == ifName && interfaceTable[i].GetAddress().GetVersion() == version)
        return interfaceTable[i].GetAddress();
    }
  }
  return GetInvalidAddress();
}


PString PIPSocket::GetInterfaceMACAddress(const char * ifName)
{
  PIPSocket::InterfaceTable interfaceTable;
  if (PIPSocket::GetInterfaceTable(interfaceTable)) {
    for (PINDEX i = 0; i < interfaceTable.GetSize(); i++) {
      if ((ifName == NULL || interfaceTable[i].GetName() == ifName)) {
        PString macAddrStr = interfaceTable[i].GetMACAddress();
        if (!macAddrStr.IsEmpty() && macAddrStr != "44-45-53-54-00-00") /* not Win32 PPP device */
          return macAddrStr;
      }
    }
  }
  return PString::Empty();
}


PIPSocket::Address PIPSocket::GetNetworkInterface(unsigned version)
{
  PIPSocket::InterfaceTable interfaceTable;
  if (PIPSocket::GetInterfaceTable(interfaceTable)) {
    for (PINDEX i = 0; i < interfaceTable.GetSize(); ++i) {
      PIPSocket::Address localAddr = interfaceTable[i].GetAddress();
      if (localAddr.GetVersion() == version && !localAddr.IsLoopback() && !localAddr.IsRFC1918())
        return localAddr;
    }
  }
  return GetInvalidAddress();
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
  return ConvertOSError(os_handle = (P_INT_PTR)os_socket(AF_INET, SOCK_STREAM, 0));
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
  if (CheckNotOpen())
    return false;

  flush();
  PINDEX writeCount = 0;

  do {
    Slice slice(((char *)buf)+writeCount, len);
    if (!os_vwrite(&slice, 1, 0, NULL, 0))
      return false;
    writeCount += lastWriteCount;
    len -= lastWriteCount;
  } while (len > 0);

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

  PIPSocket::sockaddr_wrapper sa;
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

  if (CheckNotOpen())
    return false;

  PIPSocket::sockaddr_wrapper sa;
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

  if (CheckNotOpen())
    return false;

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
    PIPSocket::sockaddr_wrapper sa(addr, port);
    ok = os_vwrite(slices, sliceCount, 0, sa, sa.GetSize());
  }
  
#else
  
  PIPSocket::sockaddr_wrapper sa(broadcast ? Address::GetBroadcast(addr.GetVersion()) : addr, port);
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
  SetPort(newPort);
  OpenSocket(iAddressFamily);
}


PUDPSocket::PUDPSocket(const PString & service, int iAddressFamily)
{
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


PBoolean PUDPSocket::OpenSocket()
{
  return OpenSocket(PF_INET);
}


PBoolean PUDPSocket::OpenSocket(int ipAdressFamily)
{
  return ConvertOSError(os_handle = os_socket(ipAdressFamily, SOCK_DGRAM, 0));
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


bool PUDPSocket::InternalReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort)
{
  if (!PIPDatagramSocket::InternalReadFrom(slices, sliceCount, ipAndPort))
    return false;

  InternalSetLastReceiveAddress(ipAndPort);
  return true;
}


PBoolean PUDPSocket::Read(void * buf, PINDEX len)
{
  PIPSocketAddressAndPort dummy;
  return ReadFrom(buf, len, dummy);
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
  m_sendAddressAndPort = addr;
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
  return m_sendAddressAndPort.AsString();
}


void PUDPSocket::InternalGetSendAddress(PIPSocketAddressAndPort & addr) const
{
  addr = m_sendAddressAndPort;
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
  return m_lastReceiveAddressAndPort.AsString();
}


void PUDPSocket::InternalGetLastReceiveAddress(PIPSocketAddressAndPort & ap) const
{
  ap = m_lastReceiveAddressAndPort;
}


void PUDPSocket::InternalSetLastReceiveAddress(const PIPSocketAddressAndPort & ap)
{
  m_lastReceiveAddressAndPort = ap;
}


//////////////////////////////////////////////////////////////////////////////

PBoolean PICMPSocket::OpenSocket(int)
{
  return false;
}

//////////////////////////////////////////////////////////////////////////////

PIPSocket::AddressAndPort::AddressAndPort(char separator)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(0)
  , m_separator(separator)
{
}


PIPSocket::AddressAndPort::AddressAndPort(WORD defaultPort, char separator)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(defaultPort)
  , m_separator(separator)
{
}


PIPSocket::AddressAndPort::AddressAndPort(const PString & str, WORD defaultPort, char separator, const char * proto)
  : m_address(PIPSocket::GetInvalidAddress())
  , m_port(defaultPort)
  , m_separator(separator)
{
  Parse(str, defaultPort, m_separator, proto);
}


PIPSocket::AddressAndPort::AddressAndPort(const PIPSocket::Address & addr, WORD defaultPort, char separator)
  : m_address(addr)
  , m_port(defaultPort)
  , m_separator(separator)
{
}


PIPSocketAddressAndPort::AddressAndPort(struct sockaddr *ai_addr, const int ai_addrlen)
  : m_address(ai_addr->sa_family, ai_addrlen, ai_addr)
  , m_port(ntohs((ai_addr->sa_family == AF_INET) ? ((sockaddr_in *)ai_addr)->sin_port : ((sockaddr_in6 *)ai_addr)->sin6_port))
  , m_separator(':')
{  
}


bool PIPSocket::AddressAndPort::Parse(const PString & str, WORD port, char separator, const char * proto)
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


PString PIPSocket::AddressAndPort::AsString(char separator) const
{
  PString str;

  if (m_address.IsValid()) {
    str = m_address.AsString(true, true);
    if (m_port != 0)
      str.sprintf("%c%u", separator ? separator : m_separator, m_port);
  }

  return str;
}


void PIPSocket::AddressAndPort::SetAddress(const PIPSocket::Address & addr, WORD port)
{
  m_address = addr;
  if (port != 0)
    m_port = port;
}


PObject::Comparison PIPSocket::AddressAndPort::Compare(const PObject & obj) const
{
  const AddressAndPort & other = dynamic_cast<const AddressAndPort &>(obj);
  Comparison c = m_address.Compare(other.m_address);
  if (c == EqualTo)
    c = Compare2(m_port, other.m_port);
  return c;
}


bool PIPSocket::AddressAndPort::MatchWildcard(const AddressAndPort & wild) const
{
  return (!wild.m_address.IsValid() || wild.m_address == m_address) &&
         ( wild.m_port == 0         || wild.m_port    == m_port);
}


///////////////////////////////////////////////////////////////////////////////

PIPSocket::PortRange::PortRange(WORD basePort, WORD maxPort)
  : m_base(basePort)
  , m_max(basePort != 0 && maxPort != 0 ? maxPort : basePort)
{
}


void PIPSocket::PortRange::Set(unsigned newBase, unsigned newMax, unsigned range, unsigned dflt)
{
  if (newBase == 0) {
    newBase = dflt;
    newMax = dflt;
    if (dflt > 0)
      newMax += range;
  }
  else {
    if (newBase < 1024)
      newBase = 1024;
    else if (newBase > 65500)
      newBase = 65500;

    if (newMax <= newBase)
      newMax = newBase + range;
    if (newMax > 65530) // Room for something to get 5 consecutive ports.
      newMax = 65530;
  }

  m_mutex.Wait();

  m_base = (WORD)newBase;
  m_max = (WORD)newMax;

  m_mutex.Signal();
}


bool PIPSocket::PortRange::Connect(PIPSocket & socket, const Address & addr, const Address & binding)
{
  m_mutex.Wait();
  WORD basePort = m_base;
  WORD maxPort = m_max;
  m_mutex.Signal();

  if (basePort == 0 || basePort == maxPort)
    return socket.Connect(binding, basePort, addr);

  WORD firstPort = (WORD)PRandom::Number(basePort, maxPort);
  WORD nextPort = firstPort;
  do {
    if (socket.Connect(binding, nextPort, addr))
      return true;
    if (socket.GetErrorNumber() != EADDRINUSE && socket.GetErrorNumber() != EADDRNOTAVAIL)
      return false;
    PTRACE(5, &socket, PTraceModule(), "Could not connect using local port " << nextPort);
    if (++nextPort >= maxPort)
      nextPort = basePort;
  } while (nextPort != firstPort);

  PTRACE(2, &socket, PTraceModule(), "Connect failed, exhausted all ports from " << basePort << " to " << maxPort);
  return false;
}


bool PIPSocket::PortRange::Listen(PIPSocket & socket, const Address & binding, unsigned queueSize, Reusability reuse)
{
  PIPSocket * sockets = &socket;
  return Listen(&sockets, 1, binding, queueSize, reuse);
}


bool PIPSocket::PortRange::Listen(PIPSocket ** sockets,
                                  PINDEX numSockets,
                                  const Address & binding,
                                  unsigned queueSize,
                                  Reusability reuse)
{
  if (!PAssert(sockets != NULL && sockets[0] != NULL, PInvalidParameter))
    return false;

  PWaitAndSignal mutex(m_mutex);

  if (m_base == 0 || m_base == m_max) {
    if (!sockets[0]->Listen(binding, queueSize, m_base, reuse))
      return false;
    WORD nextPort = sockets[0]->GetPort();
    for (WORD s = 1; s < numSockets; ++s) {
      if (PAssert(sockets[s] != NULL, PInvalidParameter) && !sockets[s]->Listen(binding, queueSize, nextPort+s, reuse))
        return false;
    }
    return true;
  }

  if (numSockets >= (m_max - m_base)) {
    PTRACE(2, NULL, PTraceModule(), "Listen failed, not enough room from "
           << m_base << " to " << m_max << " for " << numSockets << " sockets");
    return false;
  }

  /*
    Use a random port pair base number in the specified range for
    the creation of the RTP port pairs (this used to avoid issues with multiple
    NATed devices opening the same local ports and experiencing issues with
    the behaviour of the NAT device Refer: draft-jennings-behave-test-results-04 sect 3
  */
  WORD firstPort = (WORD)((PRandom::Number(m_base, m_max)/numSockets)*numSockets);
  WORD nextPort = firstPort;
  do {
    WORD s;
    for (s = 0; s < numSockets; ++s) {
      if (!PAssert(sockets[s] != NULL, PInvalidParameter))
        return false;
      if (!sockets[s]->Listen(binding, queueSize, nextPort+s, reuse)) {
        PTRACE(5, sockets[0], PTraceModule(), "Could not listen on port " << nextPort);
        break;
      }
    }
    if (s == numSockets)
      return true;

    nextPort += (WORD)numSockets;
    if ((nextPort + numSockets) > m_max)
      nextPort = m_base;
  } while (nextPort != firstPort);

  PTRACE(2, sockets[0], PTraceModule(), "Listen failed, exhausted all ports from " << m_base << " to " << m_max);
  return false;
}


// End Of File ///////////////////////////////////////////////////////////////
