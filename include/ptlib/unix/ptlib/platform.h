/*
 * platform.h
 *
 * Unix machine dependencies
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
 */

#ifndef PTLIB_PLATFORM_H
#define PTLIB_PLATFORM_H

#ifdef HAVE_STDINT_H
  #include <stdint.h>
#elif defined(HAVE_INTTYPES_H)
  #include <inttypes.h>
#endif

#ifdef HAVE_LIMITS_H
  #include <limits.h>
#endif

#ifdef HAVE_SYS_TYPES_H
  #include <sys/types.h>
#endif

#ifdef HAVE_DIRENT_H
  #include <dirent.h>
  #define NAMLEN(dirent) strlen ((dirent)->d_name)
#else
  #define dirent direct
  #define NAMLEN(dirent) ((dirent)->d_namlen)
  #ifdef HAVE_SYS_NDIR_H
    #include <sys/ndir.h>
  #endif
  #ifdef HAVE_SYS_DIR_H
    #include <sys/dir.h>
  #endif
  #ifdef HAVE_NDIR_H
    #include <ndir.h>
  #endif
#endif

#ifdef HAVE_SYS_STAT_H
  #include <sys/stat.h>
#endif

#ifdef STDC_HEADERS
  #include <stdlib.h>
  #include <stddef.h>
#else
  #ifdef HAVE_STDLIB_H
    #include <stdlib.h>
  #endif
#endif

#ifdef HAVE_STRING_H
  #if !defined STDC_HEADERS && defined HAVE_MEMORY_H
     #include <memory.h>
  #endif
  #include <string.h>
#endif

#ifdef HAVE_STRINGS_H
  #include <strings.h>
#endif

#ifdef HAVE_UNISTD_H
  #include <unistd.h>
#endif

#ifdef HAVE_ALLOCA_H
  #include <alloca.h>
#endif

#ifdef HAVE_SIGNAL_H
  #include <signal.h>
#else
  #include <sys/signal.h>
#endif

#ifdef HAVE_FCNTL_H
  #include <fcntl.h>
#else
  #include <sys/fcntl.h>
#endif

#ifdef HAVE_SYS_TIME_H
#include <sys/time.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETDB_H
  #include <netdb.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_NETINET_TCP_H
#include <netinet/tcp.h>
#endif

#if P_HAS_POLL
#include <poll.h>
#endif

///////////////////////////////////////////////////////////////////////////////
#if defined(P_LINUX)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

#define HAS_IFREQ

#if __GNU_LIBRARY__ < 6
  typedef int socklen_t;
#endif

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_GNU)

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_ANDROID)

#define P_THREAD_SAFE_LIBC
#define P_NO_CANCEL

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_FREEBSD)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

/* socklen_t is defined in FreeBSD 3.4-STABLE, 4.0-RELEASE and above */
#if (P_FREEBSD <= 340000)
  typedef int socklen_t;
#endif

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_OPENBSD)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_NETBSD)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_SOLARIS)

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

#if !defined(P_HAS_UPAD128_T)
  typedef union {
    long double _q;
    uint32_t _l[4];
  } upad128_t;
#endif

#if !defined(INADDR_NONE)
  #define INADDR_NONE     -1
#endif
#if P_SOLARIS < 7
  typedef int socklen_t;
#endif

#define HAS_IFREQ

#if __GNUC__ < 3
  extern "C" {
    int ftime (struct timeb *);
    pid_t wait3(int *status, int options, struct rusage *rusage);
    int gethostname(char *, int);
  };
#endif

///////////////////////////////////////////////////////////////////////////////
#elif defined (P_SUN4)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

#define HAS_IFREQ
#define raise(s)    kill(getpid(),s)

extern "C" {
  char *mktemp(char *);
  int accept(int, struct sockaddr *, int *);
  int connect(int, struct sockaddr *, int);
  int ioctl(int, int, void *);
  int recv(int, void *, int, int);
  int recvfrom(int, void *, int, int, struct sockaddr *, int *);
  int select(int, fd_set *, fd_set *, fd_set *, struct timeval *);
  int sendto(int, const void *, int, int, const struct sockaddr *, int);
  int send(int, const void *, int, int);
  int shutdown(int, int);
  int socket(int, int, int);
  int vfork();
  void closelog();
  void gettimeofday(struct timeval * tv, struct timezone * tz);
  void openlog(const char *, int, int);
  void syslog(int, char *, ...);
  int setpgrp(int, int);
  pid_t wait3(int *status, int options, struct rusage *rusage);
  int bind(int, struct sockaddr *, int);
  int listen(int, int);
  int getsockopt(int, int, int, char *, int *);
  int setsockopt(int, int, int, char *, int);
  int getpeername(int, struct sockaddr *, int *);
  int gethostname(char *, int);
  int getsockname(int, struct sockaddr *, int *);
  char * inet_ntoa(struct in_addr);
  int ftime (struct timeb *);
  struct hostent * gethostbyname(const char *);
  struct hostent * gethostbyaddr(const char *, int, int);
  struct servent * getservbyname(const char *, const char *);
};

#undef NL0
#undef NL1
#undef CR0
#undef CR1
#undef CR2
#undef CR3
#undef TAB0
#undef TAB1
#undef TAB2
#undef XTABS
#undef BS0
#undef BS1
#undef FF0
#undef FF1
#undef ECHO
#undef NOFLSH
#undef TOSTOP
#undef FLUSHO
#undef PENDIN


///////////////////////////////////////////////////////////////////////////////
#elif __BEOS__

#include <OS.h>
#include <cpp/stl.h>

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

#define SOCK_RAW 3 // raw-protocol interface, not suported in R4
#define PF_INET AF_INET
#define TCP_NODELAY 1
typedef int socklen_t;
#include <bone/arpa/inet.h>

#define wait3(s, o, r) waitpid(-1, s, o)
int seteuid(uid_t euid);
int setegid(gid_t gid);

#define RTLD_NOW        0x2
extern "C" {
  void *dlopen(const char *path, int mode);
  int dlclose(void *handle);
  void *dlsym(void *handle, const char *symbol);
};

///////////////////////////////////////////////////////////////////////////////
#elif defined (P_MACOSX) || defined(P_IOS)

#include "Availability.h"

#if defined(P_IOS)
#define P_IPHONEOS P_IOS // For backward compatibility
#endif

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

#define HAS_IFREQ


///////////////////////////////////////////////////////////////////////////////
#elif defined (P_AIX)

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined (P_IRIX)

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

typedef int socklen_t;

///////////////////////////////////////////////////////////////////////////////
#elif defined (P_VXWORKS)

#include <taskLib.h>
#include <semLib.h>
#include <sysLib.h>
#include <time.h>
#include <ioLib.h>
#include <selectLib.h>
#include <inetLib.h>
#include <hostLib.h>
#include <socklib.h>

// Prevent conflict between net/mbuf.h and some ASN.1 header files
// VxWorks uses some #define m_data <to-something-else> constructions
#undef m_data
#undef m_type

#define P_STRTOQ strtol
#define P_STRTOUQ strtoul

#define HAS_IFREQ

#define _exit(i)   exit(i)

typedef int socklen_t;

extern int h_errno;

#define SUCCESS    0
#define NOTFOUND   1

struct hostent * Vx_gethostbyname(char *name, struct hostent *hp);
struct hostent * Vx_gethostbyaddr(char *name, struct hostent *hp);

#define strcasecmp strcmp

#define P_HAS_SEMAPHORES
#define _THREAD_SAFE
#define P_THREADIDENTIFIER long


///////////////////////////////////////////////////////////////////////////////
#elif defined(P_RTEMS)

#define P_STRTOQ strtol
#define P_STRTOUQ strtoul

typedef int socklen_t;
typedef int64_t         quad_t;
extern "C" {
  int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *tv);
  int strcasecmp(const char *, const char *);
  int strncasecmp(const char *, const char *, size_t);
  char* strdup(const char *);
}
#define PSETPGRP()  tcsetprgrp(0, 0)
#define wait3(s, o, r) waitpid(-1, s, o)
#define seteuid setuid
#define setegid setgid
#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_QNX)

#define P_STRTOQ strtoll
#define P_STRTOUQ strtoull

#define HAS_IFREQ

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_CYGWIN)

#define P_STRTOQ strtoq
#define P_STRTOUQ strtouq

///////////////////////////////////////////////////////////////////////////////

// Other operating systems here

#else

#error No operating system selected.

#endif

///////////////////////////////////////////////////////////////////////////////

// includes common to all Unix variants

#ifdef P_HAS_AIO
  #include <aio.h>
#endif

typedef int SOCKET;

#ifndef PSETPGRP
  #if P_SETPGRP_NOPARM
    #define PSETPGRP()  setpgrp()
  #else
    #define PSETPGRP()  setpgrp(0, 0)
  #endif
#endif

typedef pid_t PProcessIdentifier;

#ifdef P_PTHREADS
  #ifndef _THREAD_SAFE
    #define _THREAD_SAFE 1
  #endif

  #include <pthread.h>
  typedef pthread_t PThreadIdentifier;
  #define PNullThreadIdentifier ((PThreadIdentifier)-1)
  #define P_THREAD_ID_FMT "0x%lx"

  #ifdef P_LINUX
    typedef pid_t PUniqueThreadIdentifier;
    #define P_UNIQUE_THREAD_ID_FMT "%u"
  #else
    typedef pthread_t PUniqueThreadIdentifier;
  #endif

  #if defined(P_HAS_SEMAPHORES) || defined(P_HAS_NAMED_SEMAPHORES)
    #include <semaphore.h>
  #endif  // P_HAS_SEMPAHORES

#elif defined(BE_THREADS)

  typedef thread_id PThreadIdentifier;
  #define PNullThreadIdentifier ((PThreadIdentifier)-1)
  #define P_THREAD_ID_FMT "0x%lx"
  typedef thread_id PUniqueThreadIdentifier;

#else

typedef pid_t PThreadIdentifier;
#define PNullThreadIdentifier ((PThreadIdentifier)-1)
#define P_THREAD_ID_FMT "%u"
typedef pid_t PUniqueThreadIdentifier;

#endif

#ifdef _DEBUG
  __inline void PBreakToDebugger() { kill(getpid(), SIGABRT); }
#else
  __inline void PBreakToDebugger() { }
#endif


///////////////////////////////////////////
//
//  define a macro for declaring classes so we can bolt
//  extra things to class declarations
//

#define PEXPORT
#define PSTATIC


///////////////////////////////////////////
//
// define some basic types and their limits
//

typedef  int16_t  PInt16; // 16 bit
typedef uint16_t PUInt16; // 16 bit
typedef  int32_t  PInt32; // 32 bit
typedef uint32_t PUInt32; // 32 bit
typedef  int64_t  PInt64; // 64 bit
typedef uint64_t PUInt64; // 64 bit

// Integer type that is same size as a pointer type.
typedef intptr_t      INT;

// Create "Windows" style definitions.

#if P_ODBC_DEFINES_WINDOWS_TYPES
  #define  ODBCINT64  int64_t
  #define UODBCINT64 uint64_t
  #include <sqltypes.h>
#else
  typedef void            VOID;
  typedef uint8_t         BYTE;
  typedef uint16_t        WORD;
  typedef uint32_t        DWORD;

  typedef char            CHAR;
  typedef signed char     SCHAR;
  typedef unsigned char   UCHAR;
  typedef short           SHORT;
  typedef signed short    SSHORT;
  typedef unsigned short  USHORT;
  typedef long            LONG;
  typedef signed long     SLONG;
  typedef unsigned long   ULONG;

  typedef float           SFLOAT;
  typedef double          SDOUBLE;
  typedef double          LDOUBLE;

  typedef void *          PTR;
  typedef void *          LPVOID;
  typedef CHAR *          LPSTR;

  typedef const CHAR *    LPCSTR;
  typedef DWORD *         LPDWORD;
  #define FAR

  typedef signed short    RETCODE;
  typedef void *          HWND;

  #ifdef P_HAS_WCHAR
    typedef wchar_t       WCHAR;
    typedef WCHAR *       LPWSTR;
    typedef const WCHAR * LPCWSTR;
  #endif

#endif // P_ODBC_DEFINES_WINDOWS_TYPES


#ifndef INT64_MAX
#define INT64_MAX	std::numeric_limits<PInt64>::max()
#endif

#ifndef UINT64_MAX
#define UINT64_MAX	std::numeric_limits<PUInt64>::max()
#endif


///////////////////////////////////////////
// Type used for array indexes and sizes

#if P_PINDEX_IS_SIZE_T

  typedef size_t PINDEX;
  #if P_64BIT
    const size_t P_MAX_INDEX = 0xffffffffffffffff;
  #else
    const size_t P_MAX_INDEX = 0xffffffff;
  #endif

  #define PINDEX_SIGNED 0

#else // P_PINDEX_IS_SIZE_T

  typedef int PINDEX;
  #define P_MAX_INDEX INT_MAX

  #define PINDEX_SIGNED 1

#endif // P_PINDEX_IS_SIZE_T

#endif // PTLIB_PLATFORM_H

#if __GLIBC__
  #include <byteswap.h>
#endif

#if P_HAS_RDTSC
  #include <x86intrin.h>
#endif

// End of file
