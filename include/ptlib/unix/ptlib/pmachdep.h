/*
 * machdep.h
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
 *
 * $Log: pmachdep.h,v $
 * Revision 1.22  1999/01/08 01:28:16  robertj
 * Added pthreads support for FreeBSD
 *
 * Revision 1.21  1999/01/07 03:37:15  robertj
 * dded default for pthreads, shortens command line in compile.
 *
 * Revision 1.20  1998/12/21 06:47:20  robertj
 * Solaris 5.7 support.
 *
 * Revision 1.19  1998/12/04 12:38:07  robertj
 * Fixed signal include
 *
 * Revision 1.18  1998/11/24 09:38:19  robertj
 * FreeBSD port.
 *
 * Revision 1.17  1998/11/14 01:08:25  robertj
 * PPC linux GNU compatibility.
 *
 * Revision 1.16  1998/09/24 04:11:45  robertj
 * Added open software license.
 *
 */

#ifndef _PMACHDEP_H
#define _PMACHDEP_H

///////////////////////////////////////////////////////////////////////////////
#if defined(P_LINUX)

#include <errno.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/termios.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <dlfcn.h>


#define HAS_IFREQ
#define PSETPGRP()  setpgrp()

#if __GNU_LIBRARY__ < 6
#define	P_LINUX_LIB_OLD
typedef int socklen_t;
#endif

#ifdef PPC
typedef size_t socklen_t;
#endif

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_FREEBSD)

#if defined(P_PTHREADS)
#define _THREAD_SAFE
#define P_THREAD_SAFE_CLIB

#include <pthread.h>
extern "C" int sigwait(sigset_t *set);
#endif

#include <errno.h>
#include <dlfcn.h>
#include <termios.h>
#include <sys/fcntl.h>
#include <sys/filio.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/signal.h>
#include <net/if.h>

#define HAS_IFREQ

#define PSETPGRP()  setpgrp(0, 0)

typedef int socklen_t;

///////////////////////////////////////////////////////////////////////////////
#elif defined(P_SOLARIS)

#include <errno.h>
#include <sys/sockio.h>
#include <sys/ioctl.h>
#include <sys/fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/filio.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <netinet/in.h>
#include <dlfcn.h>
#include <net/if.h>
#include <sys/sockio.h>

#define PSETPGRP()  setpgrp()

#define	INADDR_NONE	-1
#if P_SOLARIS < 7
typedef int socklen_t;
#endif

#define HAS_IFREQ

extern "C" {

int ftime (struct timeb *);
pid_t wait3(int *status, int options, struct rusage *rusage);
int gethostname(char *, int);

};

///////////////////////////////////////////////////////////////////////////////
#elif defined (P_SUN4)

#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <signal.h>
#include <net/if.h>
#include <sys/sockio.h>

#define HAS_IFREQ
#define PSETPGRP()  setpgrp(0, 0)
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
void bzero(void *, int);
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

#include <sys/termios.h>
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
};

#else

// Other operating systems here

#endif

#include <netdb.h>

#if defined(P_PTHREADS)
#define P_PLATFORM_HAS_THREADS
#include <pthread.h>
#endif


#endif // _PMACHDEP_H

// End of file
