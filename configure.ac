:nl
dnl PTLib configuration
dnl

AC_INIT([PTLib], m4_esyscmd([sed -n \
                             -e "s/ *#define *MAJOR_VERSION *\\(.*\\)/\\1./p" \
                             -e "s/ *#define *MINOR_VERSION *\\(.*\\)/\\1/p" \
                             -e "s/ *#define *BUILD_TYPE *AlphaCode/alpha/p" \
                             -e "s/ *#define *BUILD_TYPE *BetaCode/beta/p" \
                             -e "s/ *#define *BUILD_TYPE *ReleaseCode/./p" \
                             -e "s/ *#define *BUILD_NUMBER *\\(.*\\)/\\1/p" \
                             version.h | tr -d '\n']))

AC_PREREQ([2.66])

AC_CONFIG_AUX_DIR(make)

AC_CONFIG_SRCDIR([include/ptlib.h])

m4_include(make/ptlib.m4)
m4_include(make/m4_ax_pthread.m4)


dnl ########################################################################
dnl check for minimum sizing

AC_ARG_ENABLE(minsize, AS_HELP_STRING([--enable-minsize],[Minimise the size of the lib by disabling least used options]))

dnl these are the default configure settings, minimum or full
if test "${enable_minsize}x" = "yesx" ; then
  DEFAULT_RESOLVER=no
  DEFAULT_OPENLDAP=no
  DEFAULT_OPENSSL=no
  DEFAULT_EXPAT=no
  DEFAULT_SDL=no
  DEFAULT_GSTREAMER=no
  DEFAULT_SASL=no
  DEFAULT_MLIB=no
  DEFAULT_PCAP=no
  DEFAULT_LUA=no
  DEFAULT_V8=no
  DEFAULT_ODBC=no
  DEFAULT_TTS=no
  DEFAULT_ASN=1
  DEFAULT_NAT=no
  DEFAULT_STUN=no
  DEFAULT_STUNSRVR=no
  DEFAULT_PIPECHAN=no
  DEFAULT_DTMF=no
  DEFAULT_WAVFILE=no
  DEFAULT_SOCKS=no
  DEFAULT_FTP=no
  DEFAULT_SNMP=no
  DEFAULT_TELNET=no
  DEFAULT_CLI=no
  DEFAULT_REMCONN=no
  DEFAULT_SERIAL=no
  DEFAULT_POP3SMTP=no
  DEFAULT_VIDEO=no
  DEFAULT_URL=no
  DEFAULT_HTTP=no
  DEFAULT_HTTPFORMS=no
  DEFAULT_HTTPSVC=no
  DEFAULT_SSDP=no
  DEFAULT_VCARD=no
  DEFAULT_SOAP=no
  DEFAULT_XMLRPC=no
  DEFAULT_JABBER=no
  DEFAULT_CONFIG_FILE=no
  DEFAULT_QOS=no
  DEFAULT_VIDFILE=no
  DEFAULT_FFVDEV=no
  DEFAULT_VXML=no
fi

DEFAULT_APPSHARE=no
DEFAULT_SHM_VIDEO=no
DEFAULT_SHM_AUDIO=no
DEFAULT_PORTAUDIO=no
DEFAULT_VFW_CAPTURE=no
DEFAULT_SAMPLES=no
DEFAULT_PLUGINS=yes
DEFAULT_EXCEPTIONS=no


dnl ########################################################################
dnl Normalise target_os and target_cpu

dnl MSWIN_DISPLAY winver,Advanced Windows API
dnl MSWIN_DEFINE  winver,WINVER=0x502

dnl MSWIN_DISPLAY android,Android build (using vs-android)
dnl MSWIN_DEFINE  android,P_ANDROID
dnl MSWIN_DEFAULT android,Disabled

case "$target_os" in
   linux )
      OS_TAG="P_LINUX"
   ;;

   gnu )
      OS_TAG="P_GNU_HURD"
   ;;

   FreeBSD )
      OS_TAG="P_FREEBSD"
   ;;

   OpenBSD )
      OS_TAG="P_OPENBSD"
   ;;

   NetBSD )
      OS_TAG="P_NETBSD"
   ;;

   solaris )
      OS_TAG="P_SOLARIS"

      dnl According to Andre Schulze <as8@rncmm2.urz.tu-dresden.de>
      MY_COMPILE_IFELSE(
         [for upad128_t],
         [],
         [#include <sys/types.h>],
         [upad128_t upad; upad._q = 0.0;],
         [AC_DEFINE(P_HAS_UPAD128_T)]
      )

      dnl check for select_large_fdset
      ptlib_fdsize_file=/etc/system
      ptlib_fdsize=`cat ${ptlib_fdsize_file} | grep rlim_fd_max | cut -c1`

      if test "x${ptlib_fdsize}" = "x#"; then
         ptlib_fdsize=4098
      else
         ptlib_fdsize=`cat ${ptlib_fdsize_file} | grep rlim_fd_max | cut -f2 -d'='`
         if test "x${ptlib_fdsize}" = "x"; then
            ptlib_fdsize=4098
         fi
      fi

      if test "x${ptlib_fdsize}" != "x4098"; then
         CPPFLAGS="$CPPFLAGS -DFD_SETSIZE=${ptlib_fdsize}"
      fi
   ;;

   Darwin )
      OS_TAG="P_MACOSX" 
      dnl Where "port" libraries live
      CPPFLAGS="$CPPFLAGS -I/opt/local/include"
      LIBS="-L/opt/local/lib $LIBS"
   ;;

   iPhone* )
      OS_TAG="P_IOS" 
   ;;

   beos )
      OS_TAG="P_BEOS"
   ;;

   cygwin )
      OS_TAG="P_CYGWIN"
   ;;

   mingw )
      OS_TAG="P_MINGW"
   ;;

   * )
      OS_TAG="P_TARGET_OS"
esac

AC_DEFINE_UNQUOTED($OS_TAG, $target_release)


if test "$target_64bit" = "1" ; then
  AC_DEFINE([P_64BIT], [1], [64bit architecture])
  CPPFLAGS="$CPPFLAGS -DP_64BIT"
fi

if test "$target_os" = "linux-gnulp" ; then
  AC_DEFINE([P_LPIA], [1], [Low Power Intel Architecture])
fi


dnl ########################################################################
dnl Check for non-installed "environment variable" mode
if test "x$prefix" != "x" && test "x$prefix" != "xNONE" ; then
   if test "$prefix" = `pwd`; then
      INTERNAL_PTLIBDIR="$prefix"
   fi
else
   if test "x$PTLIBDIR" != "x" ; then
      AC_MSG_NOTICE(PTLIBDIR environment variable detected as $PTLIBDIR)
      ac_default_prefix=$PTLIBDIR
      INTERNAL_PTLIBDIR="$PTLIBDIR"
   fi
fi

if test "x$INTERNAL_PTLIBDIR" != "x"; then
   libdir+=_$target
   datarootdir="\${exec_prefix}"
   makedir="\${exec_prefix}/make"
   VERSION_DIR="$INTERNAL_PTLIBDIR"
else
   makedir="\${datarootdir}/ptlib/make"
   VERSION_DIR="$srcdir"
fi

AC_SUBST(makedir)


dnl ########################################################################
dnl extract the PTLib version
dnl MSWIN_DISPLAY version,Version
dnl MSWIN_VERSION version,PTLIB_MAJOR=MAJOR_VERSION
dnl MSWIN_VERSION version,PTLIB_MINOR=MINOR_VERSION
dnl MSWIN_VERSION version,PTLIB_BUILD=BUILD_NUMBER
dnl MSWIN_VERSION version,PTLIB_VERSION=VERSION

MY_VERSION_FILE([$VERSION_DIR/version.h], [PTLIB])


dnl MSWIN_DISPLAY    version_helpers,Version Helpers
dnl MSWIN_CHECK_FILE version_helpers,VersionHelpers.h,P_VERSION_HELPERS=1


dnl ########################################################################
dnl Find programs we need

AC_PATH_PROG(BISON, bison)


dnl ########################################################################
dnl Mostly, we are C++

AC_LANG(C++)
AC_MSG_NOTICE(Default extension for C++ files is $ac_ext)


dnl ########################################################################
dnl check for old gcc version

AC_MSG_CHECKING(checking if pragma implementation should be used)

gcc_version=`$CC -dumpversion`
if test "x$gcc_version" != "x" ; then
   if test `echo $gcc_version | sed 's/\..*$//'` -lt 4 ; then
      CPPFLAGS="$CPPFLAGS -DP_USE_PRAGMA"
      AC_MSG_RESULT(yes)
   else
      AC_MSG_RESULT(no)
   fi
   AC_MSG_NOTICE($CC version is $gcc_version);
fi

dnl ########################################################################
dnl look for compile flags

MY_COMPILE_IFELSE(
   [compiler -felide-constructors],
   [-Werror -felide-constructors],
   [],
   [],
   [CXXFLAGS="-felide-constructors $CXXFLAGS"]
)

MY_COMPILE_IFELSE(
   [compiler -Wreorder],
   [-Werror -Wreorder],
   [],
   [],
   [CXXFLAGS="-Wreorder $CXXFLAGS"]
)


AC_LANG_PUSH(C)

MY_COMPILE_IFELSE(
   [compiler -fno-diagnostics-show-caret],
   [-Werror -fno-diagnostics-show-caret],
   [],
   [],
   [CPPFLAGS="$CPPFLAGS -fno-diagnostics-show-caret"]
)

MY_COMPILE_IFELSE(
   [compiler -fno-caret-diagnostics],
   [-Werror -fno-caret-diagnostics],
   [],
   [],
   [CPPFLAGS="$CPPFLAGS -fno-caret-diagnostics"]
)

AC_ARG_ENABLE(deprecated, AS_HELP_STRING([--disable-deprecated],[Stop compiler warning about deprecated functions]))

if test "${enable_deprecated}" = "no" ; then
   MY_COMPILE_IFELSE(
      [compiler -Wno-deprecated-declarations],
      [-Werror -Wno-deprecated-declarations],
      [],
      [],
      [CPPFLAGS="$CPPFLAGS -Wno-deprecated-declarations"]
   )
fi

AC_LANG_POP(C)

AC_MSG_CHECKING(if compiler uses RTTI by default)
AC_COMPILE_IFELSE([AC_LANG_PROGRAM(
   [
      class A
      {
        public:
          virtual void fred() { }
      };
      class B : public A
      {
        public:
          virtual void fred() { }
      };
   ],
   [
      B t;
      A * p = dynamic_cast<A *>(&t);
   ])],
   [
      AC_MSG_RESULT(yes)
   ],
   [
      AC_MSG_RESULT(no)
      CPPFLAGS="$CPPFLAGS -frtti"
   ]
)


dnl ########################################################################
dnl this reduces the size of all executables and libraries

PTLIB_SIMPLE_OPTION([exceptions], [EXCEPTIONS], [enable C++ exceptions])

if test "x${HAS_EXCEPTIONS}" = "x1" ; then
   CXXFLAGS="-fexceptions $CXXFLAGS"
else
   CXXFLAGS="-fno-exceptions $CXXFLAGS"
fi


dnl ########################################################################
dnl Memory allocator optimisations

AC_MSG_CHECKING(C++ memory allocator)

AC_ARG_WITH(
   [allocator],
   AS_HELP_STRING([--with-allocator=std,mt,bitmap],[Set the allocator type]),
   [],
   [withval="std"]
)

if test "$withval" = "std"; then
   AC_MSG_RESULT(std)
elif test "$withval" = "mt"; then
   AC_TRY_COMPILE(
      [#include <ext/mt_allocator.h>],
      [typedef __gnu_cxx::__mt_alloc<char> TestAllocator;],
      [
         AC_DEFINE(P_GNU_ALLOCATOR, 1)
         AC_MSG_RESULT([mt])
      ],
      [AC_MSG_RESULT([std (mt unavailable)])]
  )
elif test "$withval" = "bitmap"; then
   AC_TRY_COMPILE(
      [
         #include <ext/mt_allocator.h>
         #include <ext/bitmap_allocator.h>
      ],
      [
         typedef __gnu_cxx::__mt_alloc<char> VariableAllocator;
         typedef __gnu_cxx::bitmap_allocator<char> FixedAllocator;
      ],
      [
         AC_DEFINE(P_GNU_ALLOCATOR, 2)
         AC_MSG_RESULT(bitmap)
      ],
      [AC_ERROR([bitmap_allocator not available])]
  )
else
  AC_ERROR([Unknown allocator type $withval])
fi


dnl ########################################################################
dnl check the endianness

dnl MSWIN_DISPLAY byteorder,Byte Order
dnl MSWIN_DEFINE  byteorder,PBYTE_ORDER=PLITTLE_ENDIAN

AC_C_BIGENDIAN()
if test "x${ac_cv_c_bigendian}" = "xyes" ; then
   AC_DEFINE_UNQUOTED(PBYTE_ORDER, PBIG_ENDIAN, "byte order")
else
   AC_DEFINE_UNQUOTED(PBYTE_ORDER, PLITTLE_ENDIAN, "byte order")
fi


dnl Solaris swab should not be used
if test "$target_os" != "solaris"; then
   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
fi


dnl ########################################################################
dnl check for headers
AC_CHECK_HEADERS(limits.h alloca.h signal.h fcntl.h sys/time.h termios.h sys/termios.h \
                 ioctl.h sys/ioctl.h sys/sockio.h sys/sysctl.h sys/socket.h \
                 arpa/inet.h netdb.h ifaddrs.h netinet/in.h netinet/tcp.h \
                 net/if_types.h net/if_dl.h netinet/if_ether.h linux/errqueue.h)
AC_CHECK_HEADER(net/if.h, [AC_DEFINE(HAVE_NET_IF_H,1)],, [#include <sys/socket.h>])
AC_HEADER_DIRENT()


dnl ########################################################################
dnl check for number of parms to readdir
MY_COMPILE_IFELSE(
   [if readdir_r has 2 parms],
   [],
   [
      #define _POSIX_PTHREAD_SEMANTICS
      #include <dirent.h>
   ],
   [
      DIR *s;
      struct dirent * e;
      readdir_r(s, e);
   ],
   [P_HAS_POSIX_READDIR_R=2],
   [
      MY_COMPILE_IFELSE(
         [if readdir_r has 3 parms],
         [],
         [#include <dirent.h>],
         [
            DIR *s;
            struct dirent * e;
            struct dirent * p;
            readdir_r(s, e, &p);
         ],
         [P_HAS_POSIX_READDIR_R=3],
         [P_HAS_POSIX_READDIR_R=0]
      )
   ]
)
AC_DEFINE_UNQUOTED(P_HAS_POSIX_READDIR_R, ${P_HAS_POSIX_READDIR_R})



dnl ########################################################################
dnl check for recvmsg

MY_COMPILE_IFELSE(
   [for recvmsg],
   [],
   [
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
   ],
   [
      struct msghdr *msg;
      recvmsg(0, msg, 0);
   ],
   [AC_DEFINE(P_HAS_RECVMSG, 1)]
)


dnl ########################################################################
dnl check for rt_msghdr

MY_COMPILE_IFELSE(
   [for rt_msghdr],
   [],
   [
      #include <net/route.h>
   ],
   [
      struct rt_msghdr msg;
      msg.rtm_msglen = 0;
   ],
   [AC_DEFINE(P_HAS_RT_MSGHDR, 1)]
)


dnl ########################################################################
dnl check for recvmsg option MSG_ERRQUEUE

MY_COMPILE_IFELSE(
   [for recvmsg flag MSG_ERRQUEUE],
   [],
   [
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
   ],
   [
      int fd = -1;
      msghdr readData;
      recvmsg(fd, &readData, MSG_ERRQUEUE);
   ],
   [AC_DEFINE(P_HAS_RECVMSG_MSG_ERRQUEUE, 1)]
)


dnl ########################################################################
dnl check for recvmsg option IP_RECVERR

MY_COMPILE_IFELSE(
   [for sockopt option IP_RECVERR],
   [],
   [
      #include <sys/types.h>
      #include <sys/socket.h>
      #include <netinet/in.h>
   ],
   [
      int fd = -1;
      int v = 1;
      setsockopt(fd, IPPROTO_IP, IP_RECVERR, &v, sizeof(v));
   ],
   [AC_DEFINE(P_HAS_RECVMSG_IP_RECVERR, 1)]
)


dnl ########################################################################
dnl check for arguments to getpwuid_r

MY_COMPILE_IFELSE(
   [for getpwuid_r with 4 arguments],
   [],
   [
      #include <sys/types.h>
      #include <pwd.h>
   ],
   [
      uid_t uid;
      struct passwd * pwd;
      char * buffer;
      size_t bufsize; 
      getpwuid_r(uid, pwd, buffer, bufsize);
   ],
   [AC_DEFINE(P_GETPWUID_R4, 1)],
   [MY_COMPILE_IFELSE(
      [for getpwuid_r with 5 arguments],
      [],
      [
         #include <sys/types.h>
         #include <pwd.h>
      ],
      [
         uid_t uid;
         struct passwd * pwd;
         char * buffer;
         size_t bufsize;
         struct passwd *result;
         getpwuid_r(uid, pwd, buffer, bufsize, &result);
      ],
      [AC_DEFINE(P_GETPWUID_R5, 1)]
   )]
)


dnl ########################################################################
dnl check for arguments to getgrgid_r

MY_COMPILE_IFELSE(
   [for getgrgid_r with 4 arguments],
   [],
   [
      #include <sys/types.h>
      #include <grp.h>
   ],
   [
      gid_t gid;
      struct group * grp;
      char * buffer;
      size_t bufsize; 
      getgrgid_r(gid, grp, buffer, bufsize);
   ],
   [AC_DEFINE(P_GETGRGID_R4, 1)],
   [MY_COMPILE_IFELSE(
      [for getgrgid_r with 5 arguments],
      [],
      [
         #include <sys/types.h>
         #include <grp.h>
      ],
      [
         gid_t gid;
         struct group * grp;
         char * buffer;
         size_t bufsize;
         struct group *result;
         getgrgid_r(gid, grp, buffer, bufsize, &result);
      ],
      [AC_DEFINE(P_GETGRGID_R5, 1)]
   )]
)


dnl ########################################################################
dnl check for arguments to getpwnam_r

MY_COMPILE_IFELSE(
   [for getpwnam_r with 4 arguments],
   [],
   [
      #include <sys/types.h>
      #include <pwd.h>
   ],
   [
      const char * name;
      struct passwd * pwd;
      char * buffer;
      size_t bufsize; 
      getpwnam_r(name, pwd, buffer, bufsize);
   ],
   [AC_DEFINE(P_GETPWNAM_R4, 1)],
   [MY_COMPILE_IFELSE(
      [for getpwnam_r with 5 arguments],
      [],
      [
         #include <sys/types.h>
         #include <pwd.h>
      ],
      [
         const char * name;
         struct passwd * pwd;
         char * buffer;
         size_t bufsize;
         struct passwd *result;
         getpwnam_r(name, pwd, buffer, bufsize, &result);
      ],
      [AC_DEFINE(P_GETPWNAM_R5, 1)]
   )]
)


dnl ########################################################################
dnl check for arguments to getgrnam_r

MY_COMPILE_IFELSE(
   [for getgrnam_r with 4 arguments],
   [],
   [
      #include <sys/types.h>
      #include <grp.h>
   ],
   [
      const char * name;
      struct group * grp;
      char * buffer;
      size_t bufsize; 
      getgrnam_r(name, pwd, buffer, bufsize);
   ],
   [AC_DEFINE(P_GETGRNAM_R4, 1)],
   [MY_COMPILE_IFELSE(
      [for getgrnam_r with 5 arguments],
      [],
      [
         #include <sys/types.h>
         #include <grp.h>
      ],
      [
         const char * name;
         struct group * grp;
         char * buffer;
         size_t bufsize;
         struct group *result;
         getgrnam_r(name, grp, buffer, bufsize, &result);
      ],
      [AC_DEFINE(P_GETGRNAM_R5, 1)]
  )]
)


dnl ########################################################################
dnl check if setpgrp takes non or two arguments
MY_COMPILE_IFELSE(
   [if setpgrp takes no parameters],
   [],
   [#include <unistd.h>],
   [pid_t p = setpgrp();],
   [AC_DEFINE(P_SETPGRP_NOPARM, 1)]
)
   

dnl ########################################################################
dnl see if using GNU glibc++, because then we have atomic integer operations

AC_MSG_CHECKING(if atomic integer available)
AC_TRY_COMPILE(
   [#include <atomic>],
   [std::atomic<unsigned> val],
   [
      AC_DEFINE(P_STD_ATOMIC, 1)
      AC_MSG_RESULT([yes (std)])
   ],
   [
   AC_TRY_LINK(
      [],
      [
         int val;
         __sync_fetch_and_add(&val, 1);
      ],
      [
         AC_DEFINE(P_ATOMICITY_BUILTIN,1)
         AC_MSG_RESULT([yes (builtin)])
      ],
      AC_TRY_COMPILE(
         [#include <bits/atomicity.h>],
         [_Atomic_word val],
         [P_ATOMICITY_HEADER="<bits/atomicity.h>"],
         AC_TRY_COMPILE(
            [#include <ext/atomicity.h>],
            [_Atomic_word val],
            [P_ATOMICITY_HEADER="<ext/atomicity.h>"],
            [AC_MSG_RESULT([no (SIMULATING!)])]
         )
      )
   )

   if test -n "$P_ATOMICITY_HEADER" ; then
      AC_MSG_RESULT(yes $P_ATOMICITY_HEADER)
      AC_DEFINE_UNQUOTED(P_ATOMICITY_HEADER, $P_ATOMICITY_HEADER)

      AC_MSG_CHECKING(if atomic __exchange_and_add is in __gnu_cxx namespace)
      AC_TRY_COMPILE(
         [#include P_ATOMICITY_HEADER],
         [
            _Atomic_word val;
            __gnu_cxx::__exchange_and_add(&val, 1);
         ],
         [
            AC_MSG_RESULT(yes)
            AC_DEFINE_UNQUOTED(P_ATOMICITY_NAMESPACE, __gnu_cxx::)
         ],
         [AC_MSG_RESULT(no)]
      )
   fi
   ]
)


dnl ########################################################################
dnl check for pthreads library

AX_PTHREAD([], [AC_MSG_ERROR(Must have pthreads!)])

HAS_PTHREADS=yes
AC_DEFINE(P_PTHREADS, 1, "PThreads found")

LIBS="$PTHREAD_LIBS $LIBS"
CPPFLAGS="$CPPFLAGS -D_REENTRANT"
CFLAGS="$CFLAGS $PTHREAD_CFLAGS"
CXXFLAGS="$CXXFLAGS $PTHREAD_CFLAGS"
if test "$CC" != "$PTHREAD_CC"; then
    AC_MSG_WARN([Overriding default CC to $PTHREAD_CC])
    CC="$PTHREAD_CC"
fi

AC_ARG_ENABLE(
   pthread_kill,
   AS_HELP_STRING([--disable-pthread_kill],[Disable use of pthread_kill for checking on thread terminated]),
   [AC_DEFINE(P_NO_PTHREAD_KILL, 1, "No pthread_kill")]
)


dnl ########################################################################
dnl check for rt library

AC_CHECK_LIB(rt, clock_gettime, HAS_RT=yes, HAS_RT=no)
if test ${HAS_RT} = yes ; then
  LIBS="-lrt $LIBS"
fi


dnl ########################################################################
dnl check for semaphores
dnl MSWIN_DISPLAY semaphore,Semaphore Support
dnl MSWIN_DEFINE  semaphore,P_HAS_SEMAPHORES

MY_COMPILE_IFELSE(
   [if has Unix semaphores],
   [],
   [#include <semaphore.h>],
   [
      sem_t *s;
      sem_init(s, 0, 0);
   ],
   [
      AC_DEFINE(P_HAS_SEMAPHORES)

      MY_COMPILE_IFELSE(
         [if named semaphores are available],
         [],
         [#include <semaphore.h>],
         [sem_t *s = sem_open("test", O_CREAT)],
         [AC_DEFINE(P_HAS_NAMED_SEMAPHORES)]
      )

      MY_COMPILE_IFELSE(
         [if has sem_timedwait],
         [],
         [#include <semaphore.h>],
         [
            sem_t *s;
            const struct timespec * t;
            sem_timedwait(s, t);
         ],
         [AC_DEFINE(P_HAS_SEMAPHORES_XPG6, 1, "Supports XPG6 Semaphores")]
      )
   ]
)
   

dnl ########################################################################
dnl check for XPG6 compliant pthreads (timed spinlocks)

MY_COMPILE_IFELSE(
   [if has pthread_mutex_timedlock],
   [],
   [#include <pthread.h>],
   [pthread_mutex_timedlock(NULL, NULL)],
   [AC_DEFINE(P_PTHREADS_XPG6, 1, "XPG6 compliant pthreads found")]
)


dnl ########################################################################
dnl check for recursive mutexes
MY_COMPILE_IFELSE(
   [if has NP recursive mutexes],
   [],
   [#include <pthread.h>],
   [
      pthread_mutexattr_t attr;
      pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
   ],
   [AC_DEFINE(P_HAS_RECURSIVE_MUTEX, 1)],
   [MY_COMPILE_IFELSE(
      [if has recursive mutexes],
      [],
      [#include <pthread.h>],
      [
         pthread_mutexattr_t attr;
         pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
      ],
      [AC_DEFINE(P_HAS_RECURSIVE_MUTEX, 2)]
   )]
)


dnl ########################################################################
dnl check for gethostbyaddr_r

MY_COMPILE_IFELSE(
   [for gethostbyaddr_r],
   [],
   [#include <netdb.h>],
   [
      const void *addr;
      socklen_t len;
      int type;
      struct hostent *ret;
      char *buf;
      size_t buflen;
      struct hostent **result;
      int *h_errnop;
      gethostbyaddr_r(addr, len, type, ret, buf, buflen, result, h_errnop);
   ],
   [AC_DEFINE(P_GETHOSTBYNAME_R, 1)]
)


dnl ########################################################################
dnl check for NetLink support

MY_COMPILE_IFELSE(
   [for NetLink support],
   [],
   [
      #include <asm/types.h>
      #include <sys/socket.h>
      #include <linux/netlink.h>
      #include <linux/rtnetlink.h>
      #include <linux/genetlink.h>
   ],
   [
      struct sockaddr_nl sa;
      sa.nl_family = AF_NETLINK;
      sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
      int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
   ],
   [AC_DEFINE(P_HAS_NETLINK, 1)]
)


dnl ########################################################################
dnl look for IPV6 functions

dnl MSWIN_DISPLAY    ipv6,IPv6 Support
dnl MSWIN_CHECK_FILE ipv6,include\ws2tcpip.h,P_HAS_IPV6=1
dnl MSWIN_CHECK_FILE ipv6,include\um\ws2tcpip.h,P_HAS_IPV6=1
dnl MSWIN_CHECK_DIR  ipv6,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  ipv6,%ProgramW6432%\Windows Kits\8.0

AC_ARG_ENABLE(ipv6, AS_HELP_STRING([--disable-ipv6],[disable IPV6 support]))

HAS_IPV6=

if test "$enable_ipv6" = "no" ; then
   AC_MSG_NOTICE(IPV6 disabled)
else
   MY_COMPILE_IFELSE(
      [for IPv6 support],
      [],
      [
         #include <sys/types.h>
         #include <sys/socket.h>
         #include <netinet/in.h>
         #include <netdb.h>
      ],
      [
         struct sockaddr_in6 s;
         s.sin6_scope_id = 0;
         getnameinfo(0, 0, 0, 0, 0, 0, 0);
      ],
      [
         HAS_IPV6=1
         AC_DEFINE(P_HAS_IPV6, 1, "IPV6 enabled")
      ]
   )
fi

PTLIB_SUBST(IPV6)


MY_COMPILE_IFELSE(
   [for inet_ntop],
   [],
   [
      #include <sys/types.h>
      #include <netinet/in.h>
      #include <arpa/inet.h>
   ],
   [
      int af;
      void * src;
      char * dst;
      socklen_t len;
      inet_ntop(af, src, dst, len);
   ],
   [AC_DEFINE(P_HAS_INET_NTOP, 1)]
)


MY_COMPILE_IFELSE(
   [for IPV6_ADD_MEMBERSHIP],
   [],
   [#include <netinet/in.h>],
   [int x = IPV6_ADD_MEMBERSHIP;],
   [AC_DEFINE(P_HAS_IPV6_ADD_MEMBERSHIP, 1)]
)


dnl ########################################################################
dnl check for async I/O

AC_CHECK_HEADERS(aio.h, [AC_DEFINE(P_HAS_AIO, 1)])


dnl ########################################################################
dnl check for wchar and friends

dnl MSWIN_DISPLAY wchar,wchar Support
dnl MSWIN_DEFINE  wchar,P_HAS_WCHAR

dnl MSWIN_DISPLAY wstream,wchar stream Support
dnl MSWIN_DEFINE  wstream,P_HAS_WOSTREAM

AC_CHECK_HEADERS(
   wchar.h,
   [
      AC_DEFINE(P_HAS_WCHAR, 1)
      AC_DEFINE(P_HAS_WOSTREAM, 1)
   ]
)


dnl ########################################################################
dnl check for iostream::set_rdbuf

dnl MSWIN_DISPLAY    set_rdbuf,iostream has set_rdbuf
dnl MSWIN_DEFINE     set_rdbuf,P_HAS_SET_RDBUF
dnl MSWIN_IF_FEATURE set_rdbuf,!android

MY_COMPILE_IFELSE(
   [if has iostream::set_rdbuf],
   [],
   [#include <iostream>],
   [
      struct MyStream : public std::iostream {
        MyStream()
          : std::iostream(NULL)
        {
          set_rdbuf(NULL);
        }
      };
   ],
   [AC_DEFINE(P_HAS_SET_RDBUF, 1, "set_rdbuf found")]
)


dnl ########################################################################
dnl look for stack back trace capability

AC_ARG_ENABLE(backtrace, AS_HELP_STRING([--disable-backtrace],[disable stack back trace support]))

HAS_BACKTRACE=

if test "$enable_backtrace" = "no" ; then
   AC_MSG_NOTICE(Stack back trace disabled)
else
   MY_COMPILE_IFELSE(
      [for stack back trace support],
      [],
      [#include <execinfo.h>],
      [
         void* addrlist[100];
         int addrlen = backtrace(addrlist, sizeof(addrlist)/sizeof(void*));
         char** symbollist = backtrace_symbols(addrlist, addrlen);
      ],
      [
         HAS_BACKTRACE=1
         AC_DEFINE(P_HAS_BACKTRACE, 1, "Stack back trace enabled")

	 MY_COMPILE_IFELSE(
	    [for C++ demangle support],
	    [],
	    [#include <cxxabi.h>],
	    [
	       char name[100];
	       size_t size;
	       int status;
	       abi::__cxa_demangle("something", name, &size, &status);
	    ],
	    [AC_DEFINE(P_HAS_DEMANGLE, 1, "C++ demangle enabled")]
	 )
      ]
   )
fi

PTLIB_SUBST(BACKTRACE)


dnl ########################################################################
dnl look for MEMORY_CHECK enabled.

AC_ARG_ENABLE(memcheck, AS_HELP_STRING([--enable-memcheck],[enable leak testing code (off by default)]))
AC_MSG_NOTICE(Memory checking... $enable_memcheck)
if test "$enable_memcheck" = "yes" ; then
   AC_DEFINE(PMEMORY_CHECK, 1)
fi


dnl ########################################################################
dnl check for tracing
dnl

AC_ARG_ENABLE(tracing, AS_HELP_STRING([--disable-tracing],[Remove PTRACE and all trace logging]))
AC_ARG_ENABLE(trace-instance, AS_HELP_STRING([--disable-trace-instance],[Disable object instance in trace logging]))

if test "x${enable_tracing}" = "xno" ; then
   AC_MSG_NOTICE(PTRACE and all trace logging removed)
   CPPFLAGS="$CPPFLAGS -DPTRACING=0 -DPASN_NOPRINTON -DPASN_LEANANDMEAN"
else
   if test "x${enable_trace_instance}" = "xno" ; then
      AC_MSG_NOTICE(PTRACE without object instance trace logging included)
      CPPFLAGS="$CPPFLAGS -DPTRACING=1"
   else
      AC_MSG_NOTICE(PTRACE and all trace logging included)
      CPPFLAGS="$CPPFLAGS -DPTRACING=2"
   fi
fi


dnl ########################################################################
dnl check for regex library
dnl mingw does not provide regcomp, so search for it in regex library

AC_ARG_ENABLE(
   internalregex,
   AS_HELP_STRING([--enable-internalregex],[Force internal regex library]),
   internalregex=$enableval
)
if test "$internalregex" = "yes" ; then
   AC_MSG_NOTICE(Forcing use of internal regex library)
else
   AC_SEARCH_LIBS(
      regcomp,
      regex,
      [
         AC_SUBST(HAS_REGEX, 1)
         AC_DEFINE(P_REGEX, 1, "Regex library found")
      ]
   )
fi

PTLIB_SUBST(REGEX)


dnl ########################################################################
dnl check for dynamic library code (dlopen and friends)

dnl MSWIN_DISPLAY dll,DLL Support
dnl MSWIN_DEFINE  dll,P_DYNALINK


dnl ########################################################################
dnl look for DNS resolver code (must be done before OpenLDAP)
dnl on some systems, the resolver is in libc
dnl but needs resolv.h included which defines res_search as __res_search

dnl MSWIN_DISPLAY    resolver,DNS Resolver
dnl MSWIN_CHECK_FILE resolver,include\windns.h,P_DNS_RESOLVER=1
dnl MSWIN_CHECK_FILE resolver,include\um\windns.h,P_DNS_RESOLVER=1
dnl MSWIN_CHECK_DIR  resolver,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  resolver,%ProgramW6432%\Windows Kits\8.0

AC_ARG_ENABLE(
   [resolver],
   AS_HELP_STRING([--disable-resolver],[disable DNS resolver support]),
   [enable_resolver=$enableval],
   [enable_resolver=${DEFAULT_RESOLVER:-yes}]
)

HAS_DNS_RESOLVER=0
if test "$enable_resolver" = "no" ; then
   AC_MSG_NOTICE(DNS resolver disabled)
else
   AC_CHECK_FUNC(
      [res_ninit],
      [
         HAS_DNS_RESOLVER=1
         AC_DEFINE(P_HAS_RES_NINIT, 1)
      ],
      [MY_LINK_IFELSE(
         [for res_ninit in -lresolv],
         [],
         [-lresolv],
         [
            #include <netinet/in.h>
            #include <resolv.h>
         ],
         [
            res_state p;
            res_ninit(p);
         ],
         [
            HAS_DNS_RESOLVER=1
            AC_DEFINE(P_HAS_RES_NINIT, 1)
            LIBS="-lresolv $LIBS"
         ],
         [AC_CHECK_FUNC(
            [res_search],
            [HAS_DNS_RESOLVER=1],
            [MY_LINK_IFELSE(
               [for res_search in -lresolv],
               [],
               [-lresolv],
               [
                  #include <netinet/in.h>
                  #include <resolv.h>
               ],
               [res_search(NULL, 0, 0, NULL, 0)],
               [
                  HAS_DNS_RESOLVER=1
                  LIBS="-lresolv $LIBS"
               ],
               [AC_SEARCH_LIBS(
                  [__res_search],
                  [resolv],
                  [HAS_DNS_RESOLVER=1]
                  [AC_CHECK_HEADER(
                     [windns.h],
                     [
                        HAS_DNS_RESOLVER=1
                        LIBS="-ldnsapi $LIBS"
                     ],
                     [],
                     [#include <windows.h>]
                  )]
               )]
            )]
         )]
      )]
   )
fi

AC_DEFINE_UNQUOTED(P_DNS_RESOLVER, $HAS_DNS_RESOLVER)
PTLIB_SUBST(DNS_RESOLVER, $HAS_DNS_RESOLVER)


dnl ########################################################################
dnl look for pcap functions

dnl MSWIN_DISPLAY    pcap,WinPCAP Support
dnl MSWIN_CHECK_FILE pcap,include\pcap\pcap.h,P_PCAP_WIN32=1
dnl MSWIN_CHECK_FILE pcap,lib\x64\wpcap.lib,P_PCAP_WIN64=1
dnl MSWIN_DIR_SYMBOL pcap,PCAP_DIR
dnl MSWIN_CHECK_DIR  pcap,..\WpdPack
dnl MSWIN_CHECK_DIR  pcap,..\external\WpdPack*
dnl MSWIN_CHECK_DIR  pcap,\WpdPack\
dnl MSWIN_CHECK_DIR  pcap,c:\WpdPack\

AC_ARG_ENABLE(
   [pcap],
   AS_HELP_STRING([--disable-pcap],[disable PCAP support]),
   [],
   [enable_pcap=$DEFAULT_PCAP]
)

HAS_PCAP=
if test "$enable_pcap" = "no" ; then
   AC_MSG_NOTICE(PCAP disabled)
else
   MY_LINK_IFELSE(
      [for PCAP support],
      [],
      [-lpcap],
      [#include <pcap/pcap.h>],
      [pcap_t *h],
      [
         AC_DEFINE(P_PCAP, 1, "PCAP found")
         LIBS="-lpcap $LIBS"
         HAS_PCAP=1
      ]
   )
fi

PTLIB_SUBST(PCAP)


dnl ########################################################################
dnl check for libjpeg

dnl MSWIN_DISPLAY    libjpeg,libjpeg support
dnl MSWIN_CHECK_FILE libjpeg,lib\jpeg.lib,P_LIBJPEG_WIN32=1
dnl MSWIN_CHECK_FILE libjpeg,lib\win32\libjpeg.lib,P_LIBJPEG_WIN32=1
dnl MSWIN_CHECK_FILE libjpeg,lib\x64\libjpeg.lib,P_LIBJPEG_WIN64=1
dnl MSWIN_DIR_SYMBOL libjpeg,LIBJPEG_DIR
dnl MSWIN_FIND_FILE  libjpeg,P_LIBJPEG_LIBRARY,jpeg.lib,lib
dnl MSWIN_CHECK_DIR  libjpeg,\libjpeg-turbo
dnl MSWIN_CHECK_DIR  libjpeg,..\external\libjpeg-turbo

AC_LANG_PUSH(C)

PTLIB_MODULE_OPTION(
   [LIBJPEG],
   [libjpeg],
   [libJPEG support],
   [libjpeg],
   [],
   [-ljpeg],
   [
      #include <stdio.h>
      #include <jpeglib.h>
   ],
   [
     struct jpeg_decompress_struct d;
     unsigned char * b;
     unsigned long s;
     jpeg_mem_src(&d, b, s);
   ]
)

AC_LANG_POP(C)


dnl ########################################################################
dnl check for SUN mediaLib library

MY_ARG_ENABLE(
   [mlib],
   [SUN mlib support],
   [$DEFAULT_MLIB],
   [AC_CHECK_LIB(
      [mlib],
      [mlib_VideoColorJFIFYCC2RGB420_Nearest],
      [
         AC_DEFINE(P_MEDIALIB)
         LIBS="-lmlib $LIBS"
      ]
   )]
)

dnl ########################################################################
dnl look for Intel Integrated Performance Primitives

dnl MSWIN_DISPLAY    ipp,Intel Integrated Performance Primitives
dnl MSWIN_IF_FEATURE ipp,!android
dnl MSWIN_CHECK_FILE ipp,include/ipp.h,P_IPP=1
dnl MSWIN_FIND_FILE  ipp,P_IPP_CC_LIB32,ippcc.lib,lib/ia32
dnl MSWIN_FIND_FILE  ipp,P_IPP_CC_LIB64,ippcc.lib,lib/intel64
dnl MSWIN_FIND_FILE  ipp,P_IPP_CORE_LIB32,ippcore.lib,lib/ia32
dnl MSWIN_FIND_FILE  ipp,P_IPP_CORE_LIB64,ippcore.lib,lib/intel64
dnl MSWIN_DIR_SYMBOL ipp,IPP_DIR
dnl MSWIN_CHECK_DIR  ipp,%IPPROOT%


dnl ########################################################################
dnl Look for SASL

dnl MSWIN_DISPLAY    sasl,Cyrus SASL
dnl MSWIN_CHECK_FILE sasl,include\sasl.h,P_SASL_WIN32=1
dnl MSWIN_DIR_SYMBOL sasl,SASL_DIR
dnl MSWIN_CHECK_DIR  sasl,..\sasl\
dnl MSWIN_CHECK_DIR  sasl,..\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,..\external\sasl\
dnl MSWIN_CHECK_DIR  sasl,..\external\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,\sasl\
dnl MSWIN_CHECK_DIR  sasl,\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,c:\sasl\
dnl MSWIN_CHECK_DIR  sasl,c:\cyrus-sasl\
  
AC_ARG_ENABLE(
   [sasl],
   AS_HELP_STRING([--disable-sasl],[disable SASL support]),
   [enable_sasl=$enableval],
   [enable_sasl=$DEFAULT_SASL]
)

if test "$enable_sasl" = "no" ; then
   AC_MSG_NOTICE(SASL disabled)
else 
   AC_CHECK_LIB(
      [sasl2],
      [sasl_version],
      [
         HAS_SASL2=1
         AC_DEFINE(P_SASL, 2)
         LIBS="-lsasl2 $LIBS"
      ],
      [
         HAS_SASL2=
         AC_CHECK_LIB(
            [sasl],
            [sasl_client_init],
            [
               HAS_SASL=1
               AC_DEFINE(P_SASL, 1)
               LIBS="-lsasl $LIBS"
            ],
            [HAS_SASL=]
         )
      ]
   )

   if test "x$HAS_SASL$HAS_SASL2" != "x" ; then
      AC_CHECK_HEADER([sasl/sasl.h], [AC_DEFINE(P_HAS_SASL_SASL_H, 1)])
   fi
fi

PTLIB_SUBST(SASL2)
PTLIB_SUBST(SASL)


dnl ########################################################################
dnl look for OpenLDAP (requires SASL)

dnl MSWIN_DISPLAY    openldap,Open LDAP
dnl MSWIN_IF_FEATURE openldap,sasl
dnl MSWIN_CHECK_FILE openldap,include\ldap.h|OpenLDAP,P_LDAP_WIN32=1
dnl MSWIN_DIR_SYMBOL openldap,LDAP_DIR
dnl MSWIN_CHECK_DIR  openldap,..\openldap\
dnl MSWIN_CHECK_DIR  openldap,..\external\openldap\
dnl MSWIN_CHECK_DIR  openldap,\openldap\
dnl MSWIN_CHECK_DIR  openldap,c:\openldap\

AC_LANG_PUSH(C)

PTLIB_MODULE_OPTION(
   [OPENLDAP],
   [openldap],
   [Open LDAP support],
   [ldap],
   [],
   [-lldap],
   [#include <ldap.h>],
   [ldap_init("localhost", 1234)],
   [],
   [],
   [HAS_DNS_RESOLVER]
)

if test "x$HAS_OPENLDAP" = "x1" && test "x$OS_TAG" = "xP_MACOSX" ; then
   LIBS+="-llber $LIBS"
fi


AC_LANG_POP(C)


dnl ########################################################################
dnl look for OpenSSL
dnl check for environment variable, then assume standard system install
dnl this process is made incredibly more complex by the fact that RedHad 9
dnl is completely fscked. The default OpenSSL installation requires Kerberos,
dnl even when Kerberos is not installed, requiring an explicit #define to
dnl make it compile
dnl As if that is not enough, autoconf gets very confused if configure.in
dnl contains a AC_CHECK_HEADER directive that never gets used. So we have 
dnl to ensure that it is always called

dnl MSWIN_DISPLAY    openssl,Open SSL (32 bit)
dnl MSWIN_CHECK_FILE openssl,inc32\openssl\ssl.h,P_SSL_WIN32=1
dnl MSWIN_CHECK_FILE openssl,include\openssl\ssl.h,P_SSL_WIN32=2
dnl MSWIN_FIND_FILE  openssl,P_SSL_AES,aes.h,...
dnl MSWIN_DIR_SYMBOL openssl,SSL_WIN32_DIR
dnl MSWIN_CHECK_DIR  openssl,..\openssl\
dnl MSWIN_CHECK_DIR  openssl,..\external\openssl\
dnl MSWIN_CHECK_DIR  openssl,\openssl\
dnl MSWIN_CHECK_DIR  openssl,\libs\openssl\
dnl MSWIN_CHECK_DIR  openssl,%ProgramFiles%\OpenSSL-Win32\
dnl MSWIN_CHECK_DIR  openssl,%ProgramW6432%\OpenSSL-Win32\
dnl MSWIN_CHECK_DIR  openssl,%ProgramFiles%\OpenSSL\
dnl MSWIN_CHECK_DIR  openssl,%ProgramW6432%\OpenSSL\
dnl MSWIN_CHECK_DIR  openssl,\OpenSSL-Win32

dnl MSWIN_DISPLAY    openssl64,Open SSL (64 bit)
dnl MSWIN_CHECK_FILE openssl64,include\openssl\ssl.h,P_SSL_WIN64=1
dnl MSWIN_DIR_SYMBOL openssl64,SSL_WIN64_DIR
dnl MSWIN_CHECK_DIR  openssl64,..\openssl64\
dnl MSWIN_CHECK_DIR  openssl64,..\external\openssl64\
dnl MSWIN_CHECK_DIR  openssl64,\openssl64\
dnl MSWIN_CHECK_DIR  openssl64,\libs\openssl64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramFiles%\OpenSSL-Win64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramW6432%\OpenSSL-Win64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramFiles%\OpenSSL64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramW6432%\OpenSSL64\
dnl MSWIN_CHECK_DIR  openssl64,\OpenSSL-Win64

PTLIB_MODULE_OPTION(
   [OPENSSL], 
   [openssl],
   [OpenSSL support],
   [openssl], 
   [],[],
   [#include <openssl/ssl.h>],
   [TLSv1_method()],
   [
      MY_COMPILE_IFELSE(
         [for OPENSSL const arg to d2i_AutoPrivateKey],
         [$OPENSSL_CFLAGS],
         [#include <openssl/evp.h>],
         [
            EVP_PKEY **a;
            const unsigned char **p;
            long l;
            d2i_AutoPrivateKey(a, p, l);
         ],
         [AC_DEFINE([P_SSL_USE_CONST], [1], [SSL with const])]
      )

      MY_COMPILE_IFELSE(
         [for OPENSSL AES header],
         [$OPENSSL_CFLAGS],
         [#include <openssl/aes.h>],
         [],
         [
            AC_DEFINE([P_SSL_AES], [1],[openssl with AES yes])
            HAS_SSL_AES=1
         ]
      )

      MY_COMPILE_IFELSE(
         [for OPENSSL d2i_DHparams_bio new variant],
         [$OPENSSL_CFLAGS],
         [
            #include <openssl/dh.h>
            #include <openssl/asn1.h>
         ],
         [
            BIO * in;
            d2i_DHparams_bio(in, NULL);
         ],
         [],
         [AC_DEFINE(P_d2i_DHparams_bio_OLD, 1)]
      )
   ]
)
PTLIB_SUBST(SSL_AES)


dnl ########################################################################
dnl look for expat XML parser

dnl MSWIN_DISPLAY    expat,Expat XML (local build)
dnl MSWIN_CHECK_FILE expat,lib\expat.h,P_EXPAT_BUILD=1
dnl MSWIN_DIR_SYMBOL expat,EXPAT_BUILD_DIR
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_DEBUG,expat.lib,Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_RELEASE,expat.lib,Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_DEBUG,libexpat.lib,win32/bin/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_RELEASE,libexpat.lib,win32/bin/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_DEBUG,libexpatMT.lib,win32/bin/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_RELEASE,libexpatMT.lib,win32/bin/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W64_DEBUG,libexpatMT.lib,win32/bin64/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W64_RELEASE,libexpatMT.lib,win32/bin64/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM5_DEBUG,expat.lib,wm5/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM5_RELEASE,expat.lib,wm5/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM6_DEBUG,expat.lib,wm6/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM6_RELEASE,expat.lib,wm6/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W64_DEBUG,expat.lib,x64/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W64_RELEASE,expat.lib,x64/Release
dnl MSWIN_CHECK_DIR  expat,..\expat\
dnl MSWIN_CHECK_DIR  expat,..\external\expat\
dnl MSWIN_CHECK_DIR  expat,\expat
dnl MSWIN_CHECK_DIR  expat,c:\expat\
dnl MSWIN_CHECK_DIR  expat,\libs\expat\

dnl MSWIN_DISPLAY    expat32,Expat XML (precompiled)
dnl MSWIN_CHECK_FILE expat32,Source\lib\expat.h,P_EXPAT_WIN32=1
dnl MSWIN_DIR_SYMBOL expat32,EXPAT_WIN32_DIR
dnl MSWIN_FIND_FILE  expat32,P_EXPAT_LIBRARY,libexpatMT.lib,bin
dnl MSWIN_CHECK_DIR  expat32,%ProgramFiles%\Expat 2.*
dnl MSWIN_CHECK_DIR  expat32,%ProgramW6432%\Expat 2.*

PTLIB_MODULE_OPTION(
   [EXPAT],
   [expat],
   [expat XML support],
   [expat],
   [],
   [-lexpat],
   [#include <expat.h>],
   [XML_ParserCreate(NULL)]
)


dnl ########################################################################
dnl look for Lua library
dnl MSWIN_DISPLAY    lua,Lua interpreter
dnl MSWIN_CHECK_FILE lua,include\lua.h,P_LUA_WIN32=1
dnl MSWIN_DIR_SYMBOL lua,LUA_DIR
dnl MSWIN_CHECK_DIR  lua,%ProgramFiles%\Lua\5.1
dnl MSWIN_CHECK_DIR  lua,%ProgramW6432%\Lua\5.1

AC_LANG_PUSH(C)

LUA_PKG=`pkg-config --list-all | sed -n "s/\(lua[[0-9\.-]]*\).*/\1/p" | head -1`
PTLIB_MODULE_OPTION(
   [LUA],
   [lua],
   [Lua script support],
   [$LUA_PKG], 
   [],[],
   [#include <lua.h>],
   [lua_newstate(NULL, NULL)]
)

AC_LANG_POP(C)


dnl ########################################################################
dnl look for V8 library
dnl MSWIN_DISPLAY    v8,V8 Javascript interpreter
dnl MSWIN_CHECK_FILE v8,include\v8.h|NewFromUtf8,P_V8_WIN32=3
dnl MSWIN_CHECK_FILE v8,include\v8.h,P_V8_WIN32=1
dnl MSWIN_FIND_FILE  v8,P_V8_DEBUG_LIB,v8.lib,build/Debug/...
dnl MSWIN_FIND_FILE  v8,P_V8_RELEASE_LIB,v8.lib,build/Release/...
dnl MSWIN_DIR_SYMBOL v8,V8_DIR
dnl MSWIN_CHECK_DIR  v8,..\external\v8

PTLIB_MODULE_OPTION(
   [V8],
   [v8],
   [V8 Javascript script support],
   [v8],
   [],
   [-lv8],
   [#include <v8.h>],
   [
      v8::Isolate * isolate = v8::Isolate::GetCurrent();
   ],
   [MY_COMPILE_IFELSE(
      [V8 Javascript API version 2],
      [$V8_CFLAGS],
      [#include <v8.h>],
      [
         v8::Isolate * isolate;
         const char * name;
         v8::String::NewFromUtf8(isolate, name);
         v8::V8::InitializeICU();
      ],
      [MY_COMPILE_IFELSE(
         [V8 Javascript API version 3],
         [$V8_CFLAGS],
         [#include <v8.h>],
         [
            v8::Isolate * isolate;
            isolate->SetEventLogger(NULL);
         ],
         [AC_DEFINE(P_V8_API, 3)],
         [AC_DEFINE(P_V8_API, 2)]
      )],
      [AC_DEFINE(P_V8_API, 1)]
   )]
)


dnl ########################################################################
dnl check for enabling config file support
dnl MSWIN_DISPLAY    configfile,Config file protocol support
dnl MSWIN_DEFINE     configfile,P_CONFIG_FILE

PTLIB_SIMPLE_OPTION([configfile], [CONFIG_FILE], [config file support])

dnl ########################################################################
dnl check for URL support
dnl MSWIN_DISPLAY    url,URL Support
dnl MSWIN_DEFINE     url,P_URL

PTLIB_SIMPLE_OPTION([url], [URL], [URL support])

dnl ########################################################################
dnl check for HTTP support
dnl MSWIN_DISPLAY    http,HTTP Support
dnl MSWIN_DEFINE     http,P_HTTP
dnl MSWIN_IF_FEATURE http,url

PTLIB_SIMPLE_OPTION([http], [HTTP], [HTTP support], [HAS_URL])

dnl ########################################################################
dnl check for HTTPFORMS support
dnl MSWIN_DISPLAY    httpforms,HTTP forms Support
dnl MSWIN_DEFINE     httpforms,P_HTTPFORMS
dnl MSWIN_IF_FEATURE httpforms,http&configfile

PTLIB_SIMPLE_OPTION([httpforms], [HTTPFORMS], [HTTP forms support], [HAS_HTTP], [HAS_CONFIG_FILE])


dnl ########################################################################
dnl check for HTTPSVC support
dnl MSWIN_DISPLAY    httpsvc,HTTP service Support
dnl MSWIN_DEFINE     httpsvc,P_HTTPSVC
dnl MSWIN_IF_FEATURE httpsvc,httpforms

PTLIB_SIMPLE_OPTION([httpsvc], [HTTPSVC], [HTTP server support], [HAS_HTTPFORMS])

dnl ########################################################################
dnl check for SSDP support
dnl MSWIN_DISPLAY    ssdp,SSDP support
dnl MSWIN_DEFINE     ssdp,P_SSDP
dnl MSWIN_IF_FEATURE ssdp,http

PTLIB_SIMPLE_OPTION([ssdp], [SSDP], [SSDP support], [HAS_HTTP])

dnl ########################################################################
dnl check for VCard support
dnl MSWIN_DISPLAY    vcard,VCard support
dnl MSWIN_DEFINE     vcard,P_VCARD

PTLIB_SIMPLE_OPTION([vcard], [VCARD], [VCard support], [HAS_URL])

dnl ########################################################################
dnl Jabber
dnl MSWIN_DISPLAY    jabber,Jabber
dnl MSWIN_DEFINE     jabber,P_JABBER
dnl MSWIN_IF_FEATURE jabber,expat
dnl MSWIN_IF_FEATURE jabber,expat32

PTLIB_SIMPLE_OPTION([jabber], [JABBER], [Jabber support], [HAS_EXPAT])

dnl ########################################################################
dnl Speech
dnl MSWIN_DISPLAY    sapi,Speech API
dnl MSWIN_CHECK_FILE sapi,include\sphelper.h,P_SAPI=1
dnl MSWIN_CHECK_FILE sapi,include\um\sphelper.h,P_SAPI=1
dnl MSWIN_DEFINE     sapi,P_TTS
dnl MSWIN_CHECK_DIR  sapi,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  sapi,%ProgramW6432%\Windows Kits\8.0

PTLIB_SIMPLE_OPTION([tts], [TTS], [Text-To-Speech server support])

dnl ########################################################################
dnl VXML 
dnl MSWIN_DISPLAY    vxml,VXML Support
dnl MSWIN_DEFINE     vxml,P_VXML
dnl MSWIN_IF_FEATURE vxml,http&sapi&expat
dnl MSWIN_IF_FEATURE vxml,http&sapi&expat32

PTLIB_SIMPLE_OPTION([vxml], [VXML], [VXML support], [HAS_EXPAT], [HAS_HTTP], [HAS_TTS])

dnl ########################################################################
dnl XMLRPC
dnl MSWIN_DISPLAY    xmlrpc,XMLRPC Support
dnl MSWIN_DEFINE     xmlrpc,P_XMLRPC
dnl MSWIN_IF_FEATURE xmlrpc,http&expat
dnl MSWIN_IF_FEATURE xmlrpc,http&expat32

PTLIB_SIMPLE_OPTION([xmlrpc], [XMLRPC], [XMLRPC support], [HAS_EXPAT], [HAS_HTTP])

dnl ########################################################################
dnl SOAP
dnl MSWIN_DISPLAY    soap,SOAP Support
dnl MSWIN_DEFINE     soap,P_SOAP
dnl MSWIN_IF_FEATURE soap,http&expat
dnl MSWIN_IF_FEATURE soap,http&expat32

PTLIB_SIMPLE_OPTION([soap], [SOAP], [SOAP support], [HAS_EXPAT], [HAS_HTTP])

dnl ########################################################################
dnl MSWIN_DISPLAY       atl,Active Template Library
dnl MSWIN_CHECK_FILE    atl,atlbase.h,P_ATL=1

dnl ########################################################################
dnl check for enabling ASN support
dnl MSWIN_DISPLAY    asn,ASN Support
dnl MSWIN_DEFINE     asn,P_ASN

PTLIB_SIMPLE_OPTION([asn], [ASN], [ASN decoding/encoding support])

dnl ########################################################################
dnl check for enabling NAT support
dnl MSWIN_DISPLAY    nat,NAT Support
dnl MSWIN_DEFINE     nat,P_NAT
dnl MSWIN_DISPLAY    nat,NAT traversal support
 
PTLIB_SIMPLE_OPTION([nat],     [NAT],     [NAT traversal support])

dnl ########################################################################
dnl check for enabling STUN support
dnl MSWIN_DISPLAY    stun,STUN Support
dnl MSWIN_DEFINE     stun,P_STUN
dnl MSWIN_DISPLAY    stunsrv,STUN server support
dnl MSWIN_DEFINE     stunsrv,P_STUNSRVR
 
PTLIB_SIMPLE_OPTION([stun],     [STUN],     [STUN support],        [HAS_NAT])
PTLIB_SIMPLE_OPTION([stunsrvr], [STUNSRVR], [STUN server support], [HAS_NAT])

dnl ########################################################################
dnl check for enabling PIPECHAN support
dnl MSWIN_DISPLAY    pipechan,pipe channel Support
dnl MSWIN_DEFINE     pipechan,P_PIPECHAN

PTLIB_SIMPLE_OPTION([pipechan], [PIPECHAN], [sub-process (PPipeChannel) support])

dnl ########################################################################
dnl check for enabling DTMF support
dnl MSWIN_DISPLAY  dtmf,DTMF encoding/decoding support
dnl MSWIN_DEFINE   dtmf,P_DTMF

PTLIB_SIMPLE_OPTION([dtmf], [DTMF], [DTMF protocol support])

dnl ########################################################################
dnl check for enabling SOCKS support
dnl MSWIN_DISPLAY    socks,SOCKS protocol support
dnl MSWIN_DEFINE     socks,P_SOCKS

PTLIB_SIMPLE_OPTION([socks], [SOCKS], [SOCKS protocol support])

dnl ########################################################################
dnl check for enabling FTP support
dnl MSWIN_DISPLAY    ftp,FTP protocol support
dnl MSWIN_DEFINE     ftp,P_FTP

PTLIB_SIMPLE_OPTION([ftp], [FTP], [FTP protocol support])

dnl ########################################################################
dnl check for enabling SNMP support
dnl MSWIN_DISPLAY    snmp,SNMP protocol support
dnl MSWIN_DEFINE     snmp,P_SNMP

PTLIB_SIMPLE_OPTION([snmp], [SNMP], [SNMP protocol support])

dnl ########################################################################
dnl check for enabling TELNET support
dnl MSWIN_DISPLAY    telnet,Telnet protocol support
dnl MSWIN_DEFINE     telnet,P_TELNET

PTLIB_SIMPLE_OPTION([telnet], [TELNET], [TELNET protocol support])

dnl ########################################################################
dnl check for enabling CLI support
dnl MSWIN_DISPLAY    cli,Command line interpreter support
dnl MSWIN_DEFINE     cli,P_CLI

PTLIB_SIMPLE_OPTION([cli], [CLI], [command line interpreter support])

dnl ########################################################################
dnl check for enabling REMCONN support
dnl MSWIN_DISPLAY    remconn,remote connection support
dnl MSWIN_DEFINE     remconn,P_REMCONN

PTLIB_SIMPLE_OPTION([remconn], [REMCONN], [remote connection support], [HAS_CONFIG_FILE], [HAS_PIPECHAN])

dnl ########################################################################
dnl check for enabling serial port handling
dnl MSWIN_DISPLAY    serial,Serial Support
dnl MSWIN_DEFINE     serial,P_SERIAL

PTLIB_SIMPLE_OPTION([serial], [SERIAL], [serial port support])

dnl ########################################################################
dnl check for POP3/SMTP support
dnl MSWIN_DISPLAY    pop3smtp,POP3/SMTP Support
dnl MSWIN_DEFINE     pop3smtp,P_POP3SMTP

PTLIB_SIMPLE_OPTION([pop3smtp], [POP3SMTP], [POP3/SMTP support])

dnl ########################################################################
dnl check for sound support
dnl MSWIN_DISPLAY audio,Sound support
dnl MSWIN_DEFINE  audio,P_AUDIO

PTLIB_SIMPLE_OPTION([audio], [AUDIO], [audio support])

dnl ########################################################################
dnl check for enabling WAVFILE support
dnl MSWIN_DISPLAY    wavfile,WAV file support
dnl MSWIN_DEFINE     wavfile,P_WAVFILE

PTLIB_SIMPLE_OPTION([wavfile], [WAVFILE], [WAVFILE support], [HAS_AUDIO])


dnl ########################################################################
dnl look for QoS functions

dnl MSWIN_DISPLAY    gqos,Microsoft Windows GQOS Support
dnl MSWIN_CHECK_FILE gqos,qossp.h|QOS_DESTADDR,P_GQOS=1

dnl MSWIN_DISPLAY    qwave,Microsoft Windows qWAVE (QoS) Support
dnl MSWIN_CHECK_FILE qwave,qos2.h,P_QWAVE=1
dnl MSWIN_CHECK_FILE qwave,qos2.h|QOSSetOutgoingDSCPValue,P_QWAVE_DSCP=1


dnl ########################################################################
dnl check for video support
dnl MSWIN_DISPLAY    video,Video Support
dnl MSWIN_DEFINE     video,P_VIDEO

PTLIB_SIMPLE_OPTION([video], [VIDEO], [video device support])

if test "x${HAS_VIDEO}" != "x1" ; then
  AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
fi

dnl ########################################################################
dnl check for enabling video file support
dnl MSWIN_DISPLAY    vidfile,Video file support
dnl MSWIN_DEFINE     vidfile,P_VIDFILE

PTLIB_SIMPLE_OPTION([vidfile], [VIDFILE], [video file support], [HAS_VIDEO])

dnl ########################################################################
dnl check for enabling FFMpeg video file support
dnl MSWIN_DISPLAY    ffvdev,FFMpeg video file support
dnl MSWIN_DEFINE     ffvdev,P_FFVDEV

PTLIB_SIMPLE_OPTION([ffvdev], [FFVDEV], [FFMpeg video file support], [HAS_VIDEO], [HAS_PIPECHAN])

dnl ########################################################################
dnl check for enabling app share support
dnl MSWIN_DISPLAY    appshare,Application share support
dnl MSWIN_DEFINE     appshare,P_APPSHARE

PTLIB_SIMPLE_OPTION([appshare], [APPSHARE], [application sharing support], [HAS_VIDEO])

dnl ########################################################################
dnl enable shared memory video devices (static video plugin)

PTLIB_SIMPLE_OPTION([shmvideo], [SHM_VIDEO], [shared memory video devices])

dnl ########################################################################
dnl enable shared memory video devices (static video plugin)

PTLIB_SIMPLE_OPTION([shmvideo], [SHM_VIDEO], [shared memory video devices])


dnl ########################################################################
dnl look for Curses
dnl MS-Windows uses it's own API indicated by P_CURSES=2. Unix uses ncurses.

dnl MSWIN_DISPLAY    curses,Curses (text mode windows) support
dnl MSWIN_IF_FEATURE curses,!android
dnl MSWIN_DEFINE     curses,P_CURSES=2

PTLIB_MODULE_OPTION(
   [CURSES], 
   [curses],
   [disable Curses (text mode windows) support],
   [ncurses], 
   [],
   [-lncurses],
   [#include <ncurses.h>],
   [initscr()]
)


dnl ########################################################################
dnl look for SDL

dnl MSWIN_DISPLAY    libsdl,Simple DirectMedia Layer
dnl MSWIN_CHECK_FILE libsdl,lib\x86\SDL.lib,P_SDL_WIN32=1
dnl MSWIN_CHECK_FILE libsdl,lib\x64\SDL.lib,P_SDL_WIN64=1
dnl MSWIN_DIR_SYMBOL libsdl,SDL_DIR
dnl MSWIN_CHECK_DIR  libsdl,..\sdl*
dnl MSWIN_CHECK_DIR  libsdl,..\external\sdl*
dnl MSWIN_CHECK_DIR  libsdl,\sdl\
dnl MSWIN_CHECK_DIR  libsdl,c:\sdl\
dnl MSWIN_CHECK_DIR  libsdl,%ProgramFiles%\OSSBuild\GStreamer\v0.10.6\sdk
dnl MSWIN_CHECK_DIR  libsdl,%ProgramW6432%\OSSBuild\GStreamer\v0.10.6\sdk

PTLIB_MODULE_OPTION(
   [SDL], 
   [sdl],
   [disable libSDL (video display) support],
   [sdl], 
   [],[],
   [
      #include <SDL.h>
      #undef main
   ],
   [SDL_Init(SDL_INIT_VIDEO|SDL_INIT_NOPARACHUTE)],
   [],
   [],
   [HAS_VIDEO]
)


dnl ########################################################################
dnl check for GStreamer library

dnl MSWIN_DISPLAY    gstreamer,GStreamer (OSSBuild)
dnl MSWIN_CHECK_FILE gstreamer,include\gstreamer-0.10\gst\gst.h,P_GSTREAMER_WIN32=1
dnl MSWIN_DIR_SYMBOL gstreamer,GSTREAMER_DIR
dnl MSWIN_CHECK_DIR  gstreamer,\gstreamer-sdk\0.10\x86
dnl MSWIN_CHECK_DIR  gstreamer,%ProgramFiles%\gstreamer-sdk\0.10\x86
dnl MSWIN_CHECK_DIR  gstreamer,%ProgramW6432%\gstreamer-sdk\0.10\x86

PTLIB_MODULE_OPTION(
   [GSTREAMER], 
   [gstreamer],
   [GStreamer support],
   [gstreamer-app-0.10], 
   [],[],
   [#include <gst/gst.h>],
   [gst_version_string()]
)


dnl ########################################################################
dnl look for ODBC code

dnl MSWIN_DISPLAY    odbc,ODBC support
dnl MSWIN_CHECK_FILE odbc,include\odbcinst.h,P_ODBC=1
dnl MSWIN_CHECK_FILE odbc,include\um\odbcinst.h,P_ODBC=1
dnl MSWIN_CHECK_DIR  odbc,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  odbc,%ProgramW6432%\Windows Kits\8.0

AC_LANG_PUSH(C)

PTLIB_MODULE_OPTION(
   [ODBC],
   [odbc],
   [ODBC database support],
   [odbc],
   [],
   [-lodbc],
   [#include <sql.h>],
   [SQLFreeHandle(0, 0)],
   [],
   [PTLIB_MODULE_OPTION(
      [ODBC],
      [odbc],
      [unixODBC database support],
      [unixODBC],
      [],
      [-lodbc],
      [#include <sql.h>],
      [SQLFreeHandle(0, 0)]
   )]
)

if test "$HAS_ODBC" = "1"; then
   MY_COMPILE_IFELSE(
      [ODBC defines Windows types],
      [$ODBC_CFLAGS],
      [#include <sqltypes.h>],
      [BYTE b],
      [AC_DEFINE(P_ODBC_DEFINES_WINDOWS_TYPES, 1)]
   )
fi

AC_LANG_POP(C)


dnl ########################################################################
dnl check for Windows video capture support.

PTLIB_SIMPLE_OPTION([vfw], [VFW_CAPTURE], [enable video for windows capture], [HAS_VIDEO])

AC_ARG_WITH(
   [directx-includedir],
   AS_HELP_STRING([--with-directx-includedir=DIR],[Location of DirectX include files]),
   [with_directx_dir="$withval"],
   [with_directx_dir="include"]
)

if test "x${HAS_VIDEO}" = "x1" ; then
  case "$target_os" in
    cygwin* | mingw* )
      old_CPPFLAGS="$CPPFLAGS"
      CPPFLAGS="$CPPFLAGS -I${with_directx_dir}"
      ptlib_has_directx=yes
      AC_CHECK_HEADERS([dshow.h], [], [ptlib_has_directx=no])
      CPPFLAGS="$old_CPPFLAGS"

      if test "x${ptlib_has_directx}" = "xyes" ; then
         HAS_DIRECTSHOW=1
         HAS_DIRECTSOUND=1
         AC_DEFINE(P_DIRECTSHOW, 1)
         AC_DEFINE(P_DIRECTX, 1)
         AC_DEFINE(P_DIRECTSOUND, 1)
         CPPFLAGS="$CPPFLAGS -I${with_directx_dir}"
         LIBS="-ldsound -ldxerr9 -ldxguid -lstrmiids -lole32 -luuid -loleaut32 -lquartz $LIBS"
      fi
    ;;
  esac

  if test "${HAS_VIDEO_CAPTURE}z" = "z" ; then
    AC_MSG_NOTICE(No Video Capture support)
    AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
  fi
fi

AC_SUBST(HAS_DIRECTSHOW)
AC_SUBST(HAS_DIRECTSOUND)


dnl ########################################################################
dnl dynamic libraries

MY_CHECK_DLFCN([
   MY_ADD_MODULE_FLAGS([DLFCN])
   PTLIB_SUBST([DYNALINK], 1)
])


dnl ########################################################################
dnl Plug ins!
dnl

dnl MSWIN_DISPLAY plugins,Plugin Support
dnl MSWIN_DEFINE  plugins,P_PLUGINS

PTLIB_SIMPLE_OPTION([plugins], [PLUGINS], [plugin support])

if test "x$enable_plugins" != "xno" ; then
   AC_MSG_CHECKING(where plugins are being installed)
   AC_ARG_WITH([plugin-installdir],
                AS_HELP_STRING([--with-plugin-installdir=DIR],[Location where plugins are installed, starting at the lib dir]),
                [PTLIB_PLUGIN_DIR="$withval"],
                [PTLIB_PLUGIN_DIR="ptlib-${PTLIB_VERSION}"]
              )

   AC_SUBST(PTLIB_PLUGIN_DIR)

   EXPANDED_PLUGIN_DIR="${libdir}/${PTLIB_PLUGIN_DIR}"
   if test "x${exec_prefix}" = "xNONE" ; then
     if test "x${prefix}" = "xNONE" ; then
       EXPANDED_PLUGIN_DIR=`echo ${EXPANDED_PLUGIN_DIR} | sed s#\\${exec_prefix}#/usr/local#`
     else
       EXPANDED_PLUGIN_DIR=`echo ${EXPANDED_PLUGIN_DIR} | sed s#\\${exec_prefix}#${prefix}#`
     fi
   else
     EXPANDED_PLUGIN_DIR=`echo ${EXPANDED_PLUGIN_DIR} | sed s#\\${exec_prefix}#${exec_prefix}#`
   fi

   case "$target_os" in
     cygwin* | mingw* )
       EXPANDED_PLUGIN_DIR="plugins"
     ;;
   esac

   AC_MSG_RESULT(${EXPANDED_PLUGIN_DIR})

   AC_DEFINE_UNQUOTED([P_DEFAULT_PLUGIN_DIR], ["${EXPANDED_PLUGIN_DIR}"], [Default plugin directory])


   if test "x${enable_audio}" != "xno" ; then

      dnl ########################################################################
      dnl look for Direct Sound

      dnl MSWIN_DISPLAY    dsound,Direct Sound Support
      dnl MSWIN_CHECK_FILE dsound,include\dsound.h,P_DIRECTSOUND=1
      dnl MSWIN_CHECK_FILE dsound,include\um\dsound.h,P_DIRECTSOUND=1
      dnl MSWIN_FIND_FILE  dsound,P_DIRECTSOUND_DXERR_H,dxerr.h,include
      dnl MSWIN_FIND_FILE  dsound,P_DIRECTSOUND_LIB,dsound.lib,lib
      dnl MSWIN_CHECK_DIR  dsound,%ProgramFiles%\Windows Kits\8.0
      dnl MSWIN_CHECK_DIR  dsound,%ProgramW6432%\Windows Kits\8.0


      dnl ########################################################################
      dnl look for Direct Show (video)

      dnl MSWIN_DISPLAY    dshow,Direct Show Support
      dnl MSWIN_CHECK_FILE dshow,include\dshow.h,P_DIRECTSHOW=1
      dnl MSWIN_CHECK_FILE dshow,include\um\dshow.h,P_DIRECTSHOW=1
      dnl MSWIN_FIND_FILE  dshow,P_DIRECTSHOW_QEDIT_H,Qedit.h,include
      dnl MSWIN_CHECK_DIR  dshow,%ProgramFiles%\Windows Kits\8.0
      dnl MSWIN_CHECK_DIR  dshow,%ProgramW6432%\Windows Kits\8.0


      dnl #########################################################################
      dnl check for Alsa sound support

      AC_ARG_ENABLE(alsa, AS_HELP_STRING([--disable-alsa],[disable ALSA audio support]))
      PTLIB_ALSA=no
      if test "x${enable_alsa}" = "xno" ; then
         AC_MSG_NOTICE(ALSA audio disabled by user)
      else
         AC_CHECK_HEADER(
            [alsa/asoundlib.h],
            [
               AC_SUBST(HAS_ALSA, 1)
               PTLIB_ALSA=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for ESD sound support

      AC_ARG_ENABLE(esd, AS_HELP_STRING([--disable-esd],[disable ESD audio support]))
      PTLIB_ESD=no
      if test "x${enable_esd}" = "xno" ; then
         AC_MSG_NOTICE(ESD audio disabled by user)
      else
         MY_PKG_CHECK_MODULE(
            [ESD],
            [esound],
            [#include <esd.h>],
            [esd_get_server_info(0)],
            [PTLIB_SUBST([ESD], [yes])]
         )
      fi

      dnl #########################################################################
      dnl check for OSS sound support

      AC_ARG_ENABLE(oss, AS_HELP_STRING([--disable-oss],[disable OSS audio support]))
      PTLIB_OSS=no
      if test "x${enable_oss}" = "xno" ; then
         AC_MSG_NOTICE(OSS audio disabled by user)
      else
         AC_CHECK_HEADER(
            [sys/soundcard.h],
            [
               AC_SUBST(HAS_OSS, 1)
               PTLIB_OSS=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for Pulse sound support

      dnl Pulse seems to lock up on some machines, so disabled by default
      dnl AC_ARG_ENABLE(pulse, AS_HELP_STRING([--disable-pulse],[disable Pulse audio support]))
      AC_ARG_ENABLE(pulse, AS_HELP_STRING([--enable-pulse],[enable Pulse audio support]),
                    [enable_pulse="$enableval"], [enable_pulse=disabled])
      PTLIB_PULSE=no
      if test "x${enable_pulse}" = "xno" ; then
         AC_MSG_NOTICE(Pulse audio disabled by user)
      elif test "x${enable_pulse}" = "xdisabled" ; then
         AC_MSG_NOTICE(Pulse audio disabled by default)
      else
         MY_LINK_IFELSE(
            [for Pulse support],
            [],
            [-lpulse],
            [#include <pulse/context.h>],
            [pa_context * conteext = pa_context_new(NULL, "conf")],
            [
               AC_SUBST(HAS_PULSE, 1)
               PTLIB_PULSE=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for sunaudio sound support

      AC_ARG_ENABLE(sunaudio, AS_HELP_STRING([--disable-sunaudio],[disable Sun audio support]))
      PTLIB_SUNAUDIO=no
      if test "x${enable_sunaudio}" = "xno" ; then
         AC_MSG_NOTICE(Sun audio disabled by user)
      else
         AC_CHECK_HEADER(
            [sys/mixer.h],
            [
               AC_SUBST(HAS_SUN_AUDIO, 1)
               PTLIB_SUNAUDIO=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for shm sound support

      AC_ARG_ENABLE(shmaudio, AS_HELP_STRING([--disable-shmaudio],[disable shm audio support]))
      PTLIB_AUDIOSHM=no
      if test "x${enable_shmaudio}" = "xno" ; then
         AC_MSG_NOTICE(shm audio disabled by user)
      else
         AC_CHECK_FUNC(
            [shmifOpen],
            [
               AC_SUBST(HAS_SHM_AUDIO, 1)
               PTLIB_AUDIOSHM=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for portaudio sound support

      PTLIB_MODULE_OPTION(
         [PORTAUDIO],
         [portaudio],
         [PortAudio library],
         [portaudio],
         [],
         [-;portaudio],
         [#include <portaudio.h>],
         [Pa_GetDeviceCount()],
         [AC_CHECK_HEADER(
            [portmixer.h],
            [
               AC_SUBST(HAS_PORTMIXER, 1)
               AC_DEFINE(P_PORTMIXER, 1)
               LIBS="-lportmixer $LIBS"
            ]
         )]
      )

   fi # enable_audio

   if test "x$enable_video" != "xno" ; then

      dnl #########################################################################
      dnl check for V4L video support

      AC_ARG_ENABLE(v4l, AS_HELP_STRING([--disable-v4l],[disable V4L video support]))
      PTLIB_V4L=no
      if test "x${enable_v4l}" = "xno" ; then
         AC_MSG_NOTICE(V4L video disabled by user)
      else
         AC_CHECK_HEADER(
            [linux/videodev.h],
            [
               AC_SUBST(HAS_V4L, 1)
               PTLIB_V4L=yes
            ]
         )
      fi

      dnl #########################################################################
      dnl check for V4L2 video support
      dnl ########################################################################

      AC_ARG_ENABLE(v4l2, AS_HELP_STRING([--disable-v4l2],[disable V4L2 video support]))

      if test "x$enable_v4l2" = "xno" ; then
        AC_MSG_NOTICE(V4L2 video disabled by user)
      else
         AC_CHECK_HEADER(
            ["linux/videodev2.h"],
            [V4L2_HEADER="linux/videodev2.h"],
            [AC_CHECK_HEADER(
               ["sys/videodev2.h"],
               [V4L2_HEADER="sys/videodev2.h"],
               [AC_CHECK_HEADER(
                  ["sys/videoio.h"],
                  [V4L2_HEADER="sys/videoio.h"]
               )]
            )]
         )

         if test "x$V4L2_HEADER" != "x" ; then
            AC_SUBST([V4L2_CFLAGS],[-DV4L2_HEADER="\"\\\"$V4L2_HEADER\\\"\""])
            AC_SUBST([V4L2_LIBS],[])
            AC_SUBST(HAS_V4L2, 1)
            PTLIB_V4L2=yes
         fi
      fi

      dnl #########################################################################
      dnl check for BSD video capture support

      AC_ARG_ENABLE(bsdvideo, AS_HELP_STRING([--disable-bsdvideo],[disable BSD video support]))
      if test "x$enable_bsdvideo" = "xno" ; then
         AC_MSG_NOTICE(BSD video disabled by user)
      else
         AC_CHECK_HEADER(
            [machine/ioctl_meteor.h],
            [AC_SUBST(HAS_BSDVIDEOCAP, 1)],
            [
               AC_CHECK_HEADER(
                  [i386/ioctl_meteor.h],
                  [AC_SUBST(HAS_BSDVIDEOCAP, 1)],
                  [
                     AC_CHECK_HEADER(
                        [dev/ic/bt8xx.h],
                        [AC_SUBST(HAS_BSDVIDEOCAP, 1)]
                     )
                  ]
               )
            ]
         )
      fi
      if test "x$HAS_BSDVIDEOCAP" = "x1"; then
         PTLIB_BSDVIDEOCAP=yes
      else
         PTLIB_BSDVIDEOCAP=no
      fi

      dnl #########################################################################
      dnl check for AVC video support

      AC_ARG_ENABLE(avc, AS_HELP_STRING([--disable-avc],[disable IEEE1394/AVC video support]))
      PTLIB_AVC1394=no
      if test "x$enable_avc" = "xno" ; then
         AC_MSG_NOTICE(IEEE1394/AVC video disabled by user)
      else
         AC_CHECK_HEADER(
            [libavc1394/avc1394.h],
            [
               AC_CHECK_HEADER(
                  [libdv/dv.h],
                  [
                     AC_CHECK_HEADER(
                        [libraw1394/raw1394.h],
                        [
                           AC_SUBST(HAS_AVC1394, 1)
                           PTLIB_AVC1394=yes
                        ]
                     )
                  ]
               )
            ]
         )
      fi

      dnl #########################################################################
      dnl check for DC video support

      AC_ARG_ENABLE(dc, AS_HELP_STRING([--disable-dc],[disable IEEE1394/DC video support]))
      PTLIB_DC1394=no
      if test "x$enable_dc" = "xno" ; then
         AC_MSG_NOTICE(IEEE1394/DC video disabled by user)
      else
         AC_CHECK_HEADER(
            [libdc1394/dc1394_control.h],
            [
               AC_CHECK_HEADER(
                  [libraw1394/raw1394.h],
                  [
                     AC_SUBST(HAS_DC1394, 1)
                     PTLIB_DC1394=yes
                  ],
               )
            ]
         )
      fi

      if test "x${HAS_DC1394}" = "x1" ; then
         AC_MSG_CHECKING(if dc1394_dma_setup_capture has 12 parms)
         AC_COMPILE_IFELSE(
            [AC_LANG_SOURCE([
               #include "libdc1394/dc1394_control.h"
               int main(int ac,char **av)
               {
                 raw1394handle_t handle;
                 nodeid_t node;
                 int channel;
                 int format;
                 int mode;
                 int speed;
                 int frame_rate; 
                 int num_dma_buffers;
                 int do_extra_buffering;
                 int drop_frames;
                 const char *dma_device_file;
                 dc1394_cameracapture *camera;
                 dc1394_dma_setup_capture(handle, node, channel, format, mode, speed, frame_rate, num_dma_buffers,
			                  do_extra_buffering, drop_frames, dma_device_file, camera);
                 return 0;
               }
            ])],
            [
               AC_SUBST(DC_CFLAGS, -DNEW_DC_API)
               AC_MSG_RESULT(yes)
            ],
            [
               AC_SUBST(DC_CFLAGS)
               AC_MSG_RESULT(no)
            ]
         )
      fi

   fi
   dnl end of test for video

fi # enable_plugins


dnl #########################################################################
dnl check to see if samples are enabled

PTLIB_SIMPLE_OPTION([samples], [SAMPLES], [enable samples build])


dnl ########################################################################
dnl output files

AC_CONFIG_FILES(ptlib.pc)
AC_CONFIG_FILES(ptlib_cfg.dxy)
AC_CONFIG_FILES(make/ptlib_config.mak)
AC_CONFIG_HEADERS(${ac_top_build_prefix}include/ptlib_config.h:include/ptlib_config.h.in)

dnl ########################################################################
dnl Add a Makefile in the platform dependent output directory, if necessary

AC_CONFIG_COMMANDS(
  [Platform-Makefile],
  [
    if test -n "${srcdir}" && test `cd ${srcdir} ; pwd` != `pwd`; then
      echo "all: ; @\$(MAKE) --file=${srcdir}/Makefile \$(MAKECMDGOALS)" > ${ac_top_build_prefix}Makefile
    fi
  ]
)


dnl ########################################################################
dnl Do it!

AC_OUTPUT()


dnl ########################################################################
dnl Summary
dnl ########################################################################
echo ""
echo "=========================== Configuration =============================="
echo "                          OS Type :  $target_os"
echo "                     Machine Type :  $target_cpu"
echo ""
echo "                           prefix :  $prefix"
echo "                      exec_prefix :  $exec_prefix"
echo "                       includedir :  $includedir"
echo "                           libdir :  $libdir"
echo "                      datarootdir :  $datarootdir"
echo "                          makedir :  $makedir"

echo ""

echo "                     DNS Resolver :  ${PTLIB_DNS_RESOLVER}"
echo "                             IPv6 :  ${PTLIB_IPV6}"
echo "            Packet Capture (PCAP) :  ${PTLIB_PCAP}"
echo "               Expat (XML parser) :  ${PTLIB_EXPAT}"
echo "                          OpenSSL :  ${PTLIB_OPENSSL}"
echo "                       SASL/SASL2 :  ${PTLIB_SASL}/${PTLIB_SASL2}"
echo "                         OpenLDAP :  ${PTLIB_OPENLDAP}"
echo "                             ODBC :  ${PTLIB_ODBC}"
echo "                              Lua :  ${PTLIB_LUA}"
echo "                       Javascript :  ${PTLIB_V8} (V8)"
echo "                           Curses :  ${PTLIB_CURSES}"
echo "                        GStreamer :  ${PTLIB_GSTREAMER}"

echo ""

echo "                            Audio :  ${PTLIB_AUDIO}"
echo "                             ALSA :  ${PTLIB_ALSA}"
echo "                              ESD :  ${PTLIB_ESD}"
echo "                              OSS :  ${PTLIB_OSS}"
echo "                            Pulse :  ${PTLIB_PULSE}"
echo "                              Sun :  ${PTLIB_SUNAUDIO}"
echo "                              shm :  ${PTLIB_AUDIOSHM}"

echo ""

echo "                            Video :  ${PTLIB_VIDEO}"
echo "                              SDL :  ${PTLIB_SDL}"
echo "                     Video4linux  :  ${PTLIB_V4L}"
echo "                     Video4linux2 :  ${PTLIB_V4L2}"
echo "                              BSD :  ${PTLIB_BSDVIDEOCAP}"
echo "                    IEEE-1394/AVC :  ${PTLIB_AVC1394}"
echo "                    IEEE-1394/DC  :  ${PTLIB_DC1394}"
echo "                       JPEG File  :  ${PTLIB_LIBJPEG}"

echo ""
echo "========================================================================"
