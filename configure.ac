dnl
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

AC_PREREQ([2.65])

AC_CONFIG_AUX_DIR(make)

AC_CONFIG_SRCDIR([include/ptlib.h])

m4_include(make/ptlib.m4)


PKG_PROG_PKG_CONFIG()
if test -z "$PKG_CONFIG" ; then
  AC_MSG_ERROR(PTLib must have pkg-config installed, 1)
fi


dnl ########################################################################
dnl Check for non-installed "environment variable" mode
OBJ_ARCH=
if test "x$PTLIBDIR" != "x" ; then
   AC_MSG_NOTICE(PTLIBDIR environment variable detected as $PTLIBDIR)
   ac_default_prefix=$PTLIBDIR
   if test "x$prefix" = "xNONE" ; then
      OBJ_ARCH=1
   fi
else
   PTLIBDIR=`pwd`
fi


dnl ########################################################################
dnl check for minimum sizing

AC_ARG_ENABLE(minsize, AS_HELP_STRING([--enable-minsize],[Minimise the size of the lib by disabling least used options]))

dnl these are the default configure settings, minimum or full
if test "${enable_minsize}x" = "yesx" ; then
  default_resolver=no
  default_openldap=no
  default_openssl=no
  default_expat=no
  default_sdl=no
  default_sasl=no
  default_mlib=no
  default_ipv6=no
  default_pcap=no
  default_lua=no
  HAS_TTS=
  HAS_ASN=1
  HAS_NAT=1
  HAS_STUN=1
  HAS_STUNSRVR=
  HAS_PIPECHAN=
  HAS_DTMF=
  HAS_WAVFILE=
  HAS_SOCKS=
  HAS_FTP=
  HAS_SNMP=
  HAS_TELNET=
  HAS_CLI=
  HAS_REMCONN=
  HAS_SERIAL=
  HAS_POP3SMTP=
  HAS_AUDIO=1
  HAS_VIDEO=1
  HAS_URL=1
  HAS_HTTP=1
  HAS_HTTPFORMS=
  HAS_HTTPSVC=
  HAS_SSDP=
  HAS_VCARD=
  HAS_SOAP=
  HAS_XMLRPC=
  HAS_JABBER=
  HAS_CONFIG_FILE=0
  HAS_QOS=
  HAS_VIDFILE=
  HAS_FFVDEV=
  HAS_APPSHARE=
  HAS_VXML=
  HAS_SHM_VIDEO=
  HAS_VFW_CAPTURE=
else
  default_tracing=yes
  default_resolver=yes
  default_openldap=yes
  default_openssl=yes
  default_expat=yes
  default_sdl=yes
  default_sasl=yes
  default_mlib=no
  default_ipv6=yes
  default_pcap=yes
  default_lua=yes
  HAS_TTS=1
  HAS_ASN=1
  HAS_NAT=1
  HAS_STUN=1
  HAS_STUNSRVR=1
  HAS_PIPECHAN=1
  HAS_DTMF=1
  HAS_WAVFILE=1
  HAS_SOCKS=1
  HAS_FTP=1
  HAS_SNMP=1
  HAS_TELNET=1
  HAS_CLI=1
  HAS_REMCONN=1
  HAS_SERIAL=1
  HAS_POP3SMTP=1
  HAS_AUDIO=1
  HAS_VIDEO=1
  HAS_URL=1
  HAS_HTTP=1
  HAS_HTTPFORMS=1
  HAS_HTTPSVC=1
  HAS_SSDP=1
  HAS_VCARD=1
  HAS_SOAP=1
  HAS_XMLRPC=1
  HAS_JABBER=1
  HAS_CONFIG_FILE=1
  HAS_QOS=1
  HAS_VIDFILE=1
  HAS_FFVDEV=1
  HAS_APPSHARE=
  HAS_SHM_VIDEO=1
  HAS_VXML=1
  HAS_VFW_CAPTURE=
fi

HAS_SAMPLES=no
HAS_PLUGINS=1
HAS_EXCEPTIONS=


dnl ########################################################################
dnl extract the PTLib version
dnl MSWIN_DISPLAY version,Version
dnl MSWIN_VERSION version,PTLIB_MAJOR=MAJOR_VERSION
dnl MSWIN_VERSION version,PTLIB_MINOR=MINOR_VERSION
dnl MSWIN_VERSION version,PTLIB_BUILD=BUILD_NUMBER
dnl MSWIN_VERSION version,PTLIB_VERSION=VERSION

MAJOR_VERSION=`cat ${PTLIBDIR}/version.h | grep MAJOR_VERSION | cut -f3 -d' '`
MINOR_VERSION=`cat ${PTLIBDIR}/version.h | grep MINOR_VERSION | cut -f3 -d' '`
BUILD_NUMBER=`cat ${PTLIBDIR}/version.h | grep BUILD_NUMBER | cut -f3 -d' '`
PTLIB_VERSION="${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER}"
BUILD_TYPE=`cat ${PTLIBDIR}/version.h | grep BUILD_TYPE | cut -f 3 -d ' ' | sed 's/BetaCode/-beta/' | sed 's/AlphaCode/-alpha/' | sed 's/ReleaseCode/\./'`

AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(BUILD_NUMBER)
AC_SUBST(PTLIB_VERSION)
AC_SUBST(BUILD_TYPE)
AC_SUBST(MAJOR_VERSION, $MAJOR_VERSION)
AC_SUBST(MINOR_VERSION, $MINOR_VERSION)
AC_SUBST(BUILD_NUMBER, $BUILD_NUMBER)

AC_MSG_NOTICE(PTLib version is ${PTLIB_VERSION});

AC_DEFINE_UNQUOTED([PTLIB_MAJOR],   [$MAJOR_VERSION], [Major version])
AC_DEFINE_UNQUOTED([PTLIB_MINOR],   [$MINOR_VERSION], [Minor version])
AC_DEFINE_UNQUOTED([PTLIB_BUILD],   [$BUILD_NUMBER], [Build number])
AC_DEFINE_UNQUOTED([PTLIB_VERSION], ["$PTLIB_VERSION"], [PTLib version])


dnl ########################################################################
dnl Normalise target_os and target_cpu

dnl MSWIN_DISPLAY winver,Advanced Windows API
dnl MSWIN_DEFINE  winver,WINVER=0x502

m4_include(make/canonical.m4)
MY_CANONICAL_TARGET()

AC_PROG_CC
AC_PROG_CXX
AC_MSG_NOTICE(Default extension for C++ files is $ac_ext)

AC_LANG(C++)


#default, most platforms do it this way
LDSOFLAGS="-shared -Wl,-soname,\$(PTLIB_SONAME)"

case "$target_os" in
   linux )
      OS_TAG="P_LINUX"
      need_pragma=yes
   ;;

   gnu )
      OS_TAG="P_GNU_HURD"
      need_pragma=yes
   ;;

   FreeBSD )
      OS_TAG="P_FREEBSD"
      need_pragma=yes
   ;;

   OpenBSD )
      OS_TAG="P_OPENBSD"
      ENDLDLIBS="-lossaudio"
      need_pragma=yes
   ;;

   NetBSD )
      OS_TAG="P_NETBSD"
      ENDLDLIBS="-lossaudio"
      need_pragma=yes
   ;;

   solaris )
      OS_TAG="P_SOLARIS"
      PTLIB_CFLAGS="$PTLIB_CFLAGS -D__inline=inline -DSOLARIS"
      LDSOFLAGS="-Bdynamic -G -h \$(PTLIB_SONAME)"
      need_pragma=yes
      dnl According to Andre Schulze <as8@rncmm2.urz.tu-dresden.de>
      PTLIB_CHECK_UPAD128( [AC_DEFINE(P_HAS_UPAD128_T)] )
      PTLIB_CHECK_FDSIZE()
   ;;

   Darwin )
      OS_TAG="P_MACOSX" 
      SHAREDLIBEXT="dylib"
      LDSOFLAGS="-dynamiclib"
      PTLIB_CFLAGS="$PTLIB_CFLAGS -DHAVE_GETTIMEOFDAY  -D__MACOSX_CORE__"
      ENDLDLIBS="-lresolv -framework CoreAudio -framework CoreFoundation -framework AudioUnit -lpthread -lm -framework CoreMidi -framework Carbon -framework AudioToolbox"
      ENDLDLIBS="$ENDLDLIBS -lresolv -lpthread -lm"
   ;;

   beos )
      OS_TAG="P_BEOS"
      PTLIB_CFLAGS="$PTLIB_CFLAGS -D__BEOS__"
      LDSOFLAGS="-shared -nostdlib -nostart"
      ENDLDLIBS="-lstdc++.r4"
   ;;

   cygwin )
      OS_TAG="P_CYGWIN"
      HAS_SHM_VIDEO=
   ;;

   mingw )
      OS_TAG="P_MINGW"
      PTLIB_CFLAGS="$PTLIB_CFLAGS -mms-bitfields"
      SHAREDLIBEXT="dll"
      LDSOFLAGS="-shared -Wl,--kill-at"
      ENDLDLIBS="-lwinmm -lwsock32 -lws2_32 -lsnmpapi -lmpr -lcomdlg32 -lgdi32 -lavicap32 -liphlpapi -lole32 -lquartz"
      HAS_SHM_VIDEO=
   ;;

   * )
      OS_TAG="P_TARGET_OS"
   ;;
esac

AC_DEFINE_UNQUOTED($OS_TAG, $target_release)


if test "$target_64bit" = "1" ; then
  AC_DEFINE([P_64BIT], [1], [64bit architecture])
  PTLIB_CFLAGS="$PTLIB_CFLAGS -DP_64BIT"
fi

if test "$target_os" = "linux-gnulp" ; then
  AC_DEFINE([P_LPIA], [1], [Low Power Intel Architecture])
fi


dnl ########################################################################
dnl set libdir with target if using PTLIBDIR as install prefix
if test "x$OBJ_ARCH" != "x" ; then
   libdir+=_$target
fi

dnl ########################################################################
dnl get the gcc version

USE_GCC=
USE_PCH=

if test x$GXX = xyes ; then
  USE_GCC=yes
  gcc_version=`$CXX -dumpversion`
  AC_MSG_NOTICE(gcc version is $gcc_version);
  GXX_MAJOR=`echo $gcc_version | sed 's/\..*$//'`
  GXX_MINOR=[`echo $gcc_version | sed -e 's/[0-9][0-9]*\.//' -e 's/\..*$//'`]
  GXX_PATCH=[`echo $gcc_version | sed -e 's/[0-9][0-9]*\.[0-9][0-9]*\.//' -e 's/\..*$//'`]

  dnl only enable pragma for gcc 4.0 and later
  AC_MSG_CHECKING(checking if pragma implementation should be used)
  if test ${GXX_MAJOR} -lt 4 ; then
    if test "${need_pragma}" = "yes" ; then
      PTLIB_CFLAGS="$PTLIB_CFLAGS -DP_USE_PRAGMA"
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  else
    AC_MSG_RESULT(no)
  fi

  dnl only enable PCH for gcc 4.0 and later
  AC_MSG_CHECKING(checking if precompiled headers available)
  if test ${GXX_MAJOR} -lt 4 ; then
    enable_pch=no
  else
    AC_ARG_ENABLE(pch, AS_HELP_STRING([--enable-pch],[Enable precompiled headers]))
  fi
  if test "${enable_pch}" = "yes" ; then
    AC_MSG_RESULT(yes)
    USE_PCH=1
  else
    AC_MSG_RESULT(no)
  fi
else
  USE_GCC=no
fi

AC_SUBST(USE_GCC)
AC_SUBST(USE_PCH)

dnl ########################################################################
dnl check if precompiled headers available, or disabled

dnl ########################################################################
dnl set the shared library extension, if not already set

if test "x${SHAREDLIBEXT}" = "x" ; then
  SHAREDLIBEXT="so"
fi
AC_SUBST(SHAREDLIBEXT, $SHAREDLIBEXT)
AC_DEFINE([P_SHAREDLIBEXT], [$SHAREDLIBEXT], [shared library extension])


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
dnl look for compile flags
AC_LANG_PUSH(C++)

AC_MSG_CHECKING(if compiler accepts -felide-constructors)
AC_LANG_CONFTEST([AC_LANG_PROGRAM([[]],[[]])])
$CXX $CXXFLAGS -felide-constructors conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  PTLIB_CXXFLAGS="$PTLIB_CXXFLAGS -felide-constructors"
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if linker accepts -Wreorder)
AC_LANG_CONFTEST([AC_LANG_PROGRAM([[]],[[]])])
$CXX $CXXFLAGS -Wreorder conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  PTLIB_CXXFLAGS="$PTLIB_CXXFLAGS -Wreorder"
else
  AC_MSG_RESULT(no)
fi


AC_ARG_ENABLE(deprecated, AS_HELP_STRING([--disable-deprecated],[Stop compiler warning about deprecated functions]))

if test "${enable_deprecated}" = "no" ; then
  AC_MSG_CHECKING(if linker accepts -Wno-deprecated-declarations)
  AC_LANG_CONFTEST([AC_LANG_PROGRAM([[]],[[]])])
  $CXX $CXXFLAGS -Wno-deprecated-declarations conftest.$ac_ext > /dev/null 2>&1
  if test $? = 0 ; then
    AC_MSG_RESULT(yes)
    PTLIB_CXXFLAGS="$PTLIB_CXXFLAGS -Wno-deprecated-declarations"
  else
    AC_MSG_RESULT(no)
  fi
fi


dnl add additional information for the debugger to ensure the user can indeed
dnl debug coredumps and macros.

AC_MSG_CHECKING(if debug build accepts -g3 -ggdb -O0)
AC_LANG_CONFTEST([AC_LANG_PROGRAM([[]],[[]])])
$CXX  $CXXFLAGS  -g3 -ggdb -O0 conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  DEBUG_FLAG="-g3 -ggdb -O0"
else
  AC_MSG_RESULT(no)
  DEBUG_FLAG="-g"
fi


AC_MSG_CHECKING(if compiler uses RTTI by default)
AC_COMPILE_IFELSE(
   [AC_LANG_SOURCE(
      [],
      [
         class A
         {
           public:
             virtual void fred() { }
         };
         class B : public A
         {
           public:
             void fred() { }
         };
         B t;
         A * p = dynamic_cast<A *>(&t);
      ]
   )],
   [
      AC_MSG_RESULT(yes)
   ],
   [
      AC_MSG_RESULT(no)
      PTLIB_CXXFLAGS="$PTLIB_CFLAGS -frtti"
   ]
)

AC_LANG_POP(C++)

dnl ########################################################################
dnl check for the size of int
AC_CHECK_SIZEOF(int)

dnl ########################################################################
dnl check for long double

AC_C_LONG_DOUBLE
if test $ac_cv_c_long_double != yes ; then
  AC_DEFINE([PNO_LONG_DOUBLE], [], [No long double])
fi

dnl if test ${target_os} = Darwin ; then
dnl   PTLIB_CFLAGS="$PTLIB_CFLAGS -Wno-long-double"
dnl fi

dnl ########################################################################
dnl we need flex, so we better make sure it's there
AC_PATH_PROG(FLEX, flex)
if test -z "$FLEX" ; then
        AC_MSG_ERROR(PTLib needs the GNU Project fast lexical analyzer generator flex, 1)
fi

dnl ########################################################################
dnl most systems have "ar", but some use "gar"
AC_CHECK_TOOL(AR, ar)
if test -z "$AR" ; then
  AC_CHECK_TOOL(AR, gar)
fi

dnl ########################################################################
dnl See if we have bison
AC_PATH_PROG(BISON, bison)

dnl ########################################################################
AC_CHECK_TOOL(SVN, svn)

dnl ########################################################################
dnl check for number of parms to readdir
AC_MSG_CHECKING(if readdir_r has 2 parms)
AC_TRY_COMPILE([#define _POSIX_PTHREAD_SEMANTICS
                #include <dirent.h>],
               [DIR *s; struct dirent * e; readdir_r(s, e);], P_HAS_POSIX_READDIR_R=2, P_HAS_POSIX_READDIR_R=0)
if test ${P_HAS_POSIX_READDIR_R} = 2 ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING(if readdir_r has 3 parms)
  AC_TRY_COMPILE([#include <dirent.h>],
                 [DIR *s; struct dirent * e; struct dirent * p; readdir_r(s, e, &p);], P_HAS_POSIX_READDIR_R=3, P_HAS_POSIX_READDIR_R=0)
  if test ${P_HAS_POSIX_READDIR_R} = 3 ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
fi
AC_DEFINE_UNQUOTED(P_HAS_POSIX_READDIR_R, ${P_HAS_POSIX_READDIR_R})



dnl ########################################################################
dnl check for tracing
dnl

AC_ARG_ENABLE(tracing, AS_HELP_STRING([--disable-tracing],[Remove PTRACE and all trace logging]))
AC_ARG_ENABLE(trace-instance, AS_HELP_STRING([--disable-trace-instance],[Disable PTraceObjectInstance in trace logging]))

if test "x${enable_tracing}" = "xno" ; then
  AC_MSG_NOTICE(PTRACE and all trace logging removed)
  PTLIB_CFLAGS="$PTLIB_CFLAGS -DPTRACING=0 -DPASN_NOPRINTON -DPASN_LEANANDMEAN"
else
  if test "x${enable_trace_instance}" = "xno" ; then
    AC_MSG_NOTICE(PTRACE without object instance trace logging included)
    PTLIB_CFLAGS="$PTLIB_CFLAGS -DPTRACING=1"
  else
    AC_MSG_NOTICE(PTRACE and all trace logging included)
    PTLIB_CFLAGS="$PTLIB_CFLAGS -DPTRACING=2"
  fi
fi


dnl ########################################################################
dnl check for Windows-like integer BOOL
dnl
dnl MSWIN_DISPLAY ansi-bool,ANSI C++ bool
dnl MSWIN_DEFINE  ansi-bool,P_USE_STANDARD_CXX_BOOL

AC_ARG_ENABLE(ansi-bool, AS_HELP_STRING([--disable-ansi-bool],[Use Windows-like integer BOOL over ANSI C++ bool]))

if test "x${enable_ansi_bool}" = "xno" ; then
  AC_MSG_NOTICE(Using Windows-like integer BOOL for PBoolean: (PFalse=0 PTrue=1))
else
  AC_MSG_NOTICE(Using ANSI C++ bool for PBoolean: (PFalse=false PTrue=true))
  AC_DEFINE(P_USE_STANDARD_CXX_BOOL)
fi


dnl ########################################################################
dnl check for recvmsg

P_HAS_RECVMSG=
AC_MSG_CHECKING(for recvmsg)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>],
               [ struct msghdr *msg; recvmsg(0, msg, 0);], P_HAS_RECVMSG=1)
if test "x${P_HAS_RECVMSG}" = "x1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_RECVMSG, 1)
else
  AC_MSG_RESULT(no)
fi

dnl ########################################################################
dnl check for recvmsg option MSG_ERRQUEUE

P_HAS_RECVMSG_MSG_ERRQUEUE=
AC_MSG_CHECKING(for recvmsg flag MSG_ERRQUEUE)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>],
               [ int fd = -1; msghdr readData; ::recvmsg(fd, &readData, MSG_ERRQUEUE); ], P_HAS_RECVMSG_MSG_ERRQUEUE=1)
if test "x${P_HAS_RECVMSG_MSG_ERRQUEUE}" = "x1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_RECVMSG_MSG_ERRQUEUE, 1)
else
  AC_MSG_RESULT(no)
fi

dnl ########################################################################
dnl check for recvmsg option IP_RECVERR

P_HAS_RECVMSG_IP_RECVERR=
AC_MSG_CHECKING(for sockopt option IP_RECVERR)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>],
               [ int fd = -1; int v = 1; setsockopt(fd, IPPROTO_IP, IP_RECVERR, &v, sizeof(v));], P_HAS_RECVMSG_IP_RECVERR=1)
if test "x${P_HAS_RECVMSG_IP_RECVERR}" = "x1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_RECVMSG_IP_RECVERR, 1)
else
  AC_MSG_RESULT(no)
fi


dnl ########################################################################
dnl check for IPV6_ADD_MEMBERSHIP

P_HAS_IPV6_ADD_MEMBERSHIP=
AC_MSG_CHECKING(for IPV6_ADD_MEMBERSHIP)
AC_TRY_COMPILE([#include <netinet/in.h>],
               [int x = IPV6_ADD_MEMBERSHIP;],
               P_HAS_IPV6_ADD_MEMBERSHIP=1)
if test "x${P_HAS_IPV6_ADD_MEMBERSHIP}" = "x1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_IPV6_ADD_MEMBERSHIP, 1)
else
  AC_MSG_RESULT(no)
fi

dnl ########################################################################
dnl check for NetLink support

P_HAS_NETLINK=0
AC_MSG_CHECKING(for NetLink support)
AC_TRY_COMPILE([#include <asm/types.h>
                #include <sys/socket.h>
                #include <linux/netlink.h>
                #include <linux/rtnetlink.h>
                #include <linux/genetlink.h>
                #include <memory.h>],
               [struct sockaddr_nl sa; memset(&sa, 0, sizeof(sa));
                sa.nl_family = AF_NETLINK; sa.nl_groups = RTMGRP_LINK | RTMGRP_IPV4_IFADDR;
                int fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);], P_HAS_NETLINK=1)
if test "${P_HAS_NETLINK}" = "1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_NETLINK, 1)
else
  AC_MSG_RESULT(no)
fi

dnl ########################################################################
dnl see if using GNU glibc++, because then we have atomic integer operations

AC_ARG_ENABLE(atomicity, AS_HELP_STRING([--disable-atomicity],[Do not use atomicity.h]))

P_HAS_ATOMIC_INT=0
P_NEEDS_GNU_CXX_NAMESPACE=0
AC_MSG_CHECKING(if atomic integer available)
if test "${enable_atomicity}" = "no" ; then
  AC_MSG_RESULT(disabled)
else
  AC_TRY_COMPILE([#include <bits/atomicity.h>],
                 [_Atomic_word val;], P_HAS_ATOMIC_INT=1,
                 AC_TRY_COMPILE([#include <ext/atomicity.h>],
                                [_Atomic_word val;], P_HAS_ATOMIC_INT=2)
  )
  if test ${P_HAS_ATOMIC_INT} != 0 ; then
    AC_MSG_RESULT(yes)
    AC_MSG_CHECKING(if __exchange_and_add is in __gnu_cxx namespace)
    if test ${P_HAS_ATOMIC_INT} = 1 ; then
      AC_TRY_COMPILE([#include <bits/atomicity.h>],
                     [_Atomic_word val; __gnu_cxx::__exchange_and_add(&val, 1)],
                     P_NEEDS_GNU_CXX_NAMESPACE=1)
    else
      P_HAS_ATOMIC_INT=1
      AC_TRY_COMPILE([#include <ext/atomicity.h>],
                     [_Atomic_word val; __gnu_cxx::__exchange_and_add(&val, 1)],
                     P_NEEDS_GNU_CXX_NAMESPACE=1)
    fi
    if test ${P_NEEDS_GNU_CXX_NAMESPACE} = 1 ; then
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  else
    AC_MSG_RESULT(no)
  fi
fi

AC_DEFINE_UNQUOTED(P_HAS_ATOMIC_INT, ${P_HAS_ATOMIC_INT})
AC_DEFINE_UNQUOTED(P_NEEDS_GNU_CXX_NAMESPACE, ${P_NEEDS_GNU_CXX_NAMESPACE})

dnl ########################################################################
dnl Memory allocator optimisations

AC_MSG_CHECKING(C++ memory allocator)

AC_ARG_WITH(allocator, AS_HELP_STRING([--with-allocator=std,mt,bitmap],[Set the allocator type]), [], [withval="std"])
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
    [
      AC_MSG_RESULT([std (mt unavailable)])
    ]
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
    [
      AC_ERROR([bitmap_allocator not available])
    ]
  )
else
  AC_ERROR([Unknown allocator type $withval])
fi


dnl ########################################################################
dnl check for Unix semaphores

# mingw implements itself the semaphores in src/ptlib/msos/win32.cxx
if test ${target_os} != mingw ; then

dnl posix unnamed semaphore support in darwin/macosx is broken
if test ${target_os} != Darwin ; then
  AC_TRY_COMPILE([#include <semaphore.h>],
                 [sem_t *s; sem_init(s, 0, 0);], P_HAS_SEMAPHORES=1)
  AC_MSG_CHECKING(if Unix semaphores are available)
  if test "${P_HAS_SEMAPHORES}x" != "x" ; then
    AC_DEFINE(P_HAS_SEMAPHORES)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
fi

if test "${P_HAS_SEMAPHORES}x" = "x" ; then

  AC_TRY_COMPILE([#include <semaphore.h>],
                 [sem_t *s = sem_open("test", O_CREAT)],
                 P_HAS_NAMED_SEMAPHORES=1 )
  AC_MSG_CHECKING(if named semaphores are available)
  if test "${P_HAS_NAMED_SEMAPHORES}x" != "x"; then
    AC_DEFINE(P_HAS_NAMED_SEMAPHORES)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_WARN(This host does not support semaphores which are essential)
    AC_MSG_WARN(to implement 'critical sections'. This feature was added)
    AC_MSG_WARN(in PWLib 1.7.0 and improves the stability of multithreaded)
    AC_MSG_WARN(programs. This feature has been disabled for this host)
  fi
fi

fi  # mingw

dnl ########################################################################
dnl check for pthreads library

dnl MSWIN_DISPLAY semaphore,Semaphore Support
dnl MSWIN_DEFINE  semaphore,P_HAS_SEMAPHORES

dnl Linux, Mac OS X use libpthread and -lpthread
dnl FreeBSD uses the -pthread compiler option and uses libc_r

if test "${target_os}" = "FreeBSD" ; then
   HAS_PTHREADS=yes
   ENDLDLIBS="$ENDLDLIBS -pthread"
   PTLIB_CFLAGS="$PTLIB_CFLAGS -D_REENTRANT -pthread"
   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
else
   AC_CHECK_LIB(pthread, pthread_create,
      [
         HAS_PTHREADS=yes
         ENDLDLIBS="$ENDLDLIBS -lpthread"
         PTLIB_CFLAGS="$PTLIB_CFLAGS -D_REENTRANT"
         AC_DEFINE(P_PTHREADS, 1, "PThreads found")
      ],
      [
         if test "${target_os}" != "mingw" ; then
            AC_MSG_ERROR(must have pthreads!)
         fi
         HAS_PTHREADS=no
      ]
   )
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
  ENDLDLIBS="$ENDLDLIBS -lrt"
fi


dnl ########################################################################
dnl check for XPG6 compliant pthreads (timed spinlocks)

if test ${HAS_PTHREADS} = yes ; then
  AC_MSG_CHECKING(if pthread_mutex_timedlock is available)
  AC_TRY_COMPILE([#include <pthread.h>],
                  [pthread_mutex_timedlock(NULL, NULL);], HAS_PTHREADS_XPG6=yes, HAS_PTHREADS_XPG6=no)
  if test ${HAS_PTHREADS_XPG6} = yes ; then
    AC_MSG_RESULT(yes)
    AC_DEFINE(P_PTHREADS_XPG6, 1, "XPG6 compliant pthreads found")
  else
    AC_MSG_RESULT(no)
  fi

  AC_CHECK_LIB(pthread, sem_timedwait, HAS_SEMAPHORES_XPG6=yes, HAS_SEMAPHORES_XPG6=no)
  if test ${HAS_SEMAPHORES_XPG6} = yes ; then
    AC_MSG_CHECKING(if sem_timedwait is compilable)
    AC_TRY_COMPILE([#include <semaphore.h>],
                    [sem_t *s; const struct timespec * t; sem_timedwait(s, t);], HAS_SEMAPHORES_XPG6=yes, HAS_SEMAPHORES_XPG6=no)
    if test ${HAS_SEMAPHORES_XPG6} = yes ; then
      AC_DEFINE(P_HAS_SEMAPHORES_XPG6, 1, "Supports XPG6 Semaphores")
      AC_MSG_RESULT(yes)
    else
      AC_MSG_RESULT(no)
    fi
  fi
fi

dnl ########################################################################
dnl check for recursive mutexes
P_HAS_RECURSIVE_MUTEX=0
AC_MSG_CHECKING(if recursive mutexes are available)
AC_TRY_COMPILE([#include <pthread.h>],
               [pthread_mutexattr_t attr; pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);], P_HAS_RECURSIVE_MUTEX=1)
if test ${P_HAS_RECURSIVE_MUTEX} = 1 ; then
  AC_MSG_RESULT(PTHREAD_MUTEX_RECURSIVE_NP)
else
  AC_TRY_COMPILE([#include <pthread.h>],
	   [pthread_mutexattr_t attr; pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);], P_HAS_RECURSIVE_MUTEX=2)
  if test ${P_HAS_RECURSIVE_MUTEX} = 2 ; then
    AC_MSG_RESULT(PTHREAD_MUTEX_RECURSIVE)
  else
    AC_MSG_RESULT(no)
  fi
fi
AC_DEFINE_UNQUOTED(P_HAS_RECURSIVE_MUTEX, ${P_HAS_RECURSIVE_MUTEX})


dnl ########################################################################
dnl check if setpgrp takes non or two arguments
P_SETPGRP_SYSTEMV=0
P_SETPGRP_NOPARM=0
AC_MSG_CHECKING(if setpgrp takes no parameters)
AC_TRY_COMPILE([#include <unistd.h>],
               [pid_t p = setpgrp();], P_SETPGRP_NOPARM=1)
if test ${P_SETPGRP_NOPARM} = 1 ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_DEFINE_UNQUOTED(P_SETPGRP_NOPARM, ${P_SETPGRP_NOPARM})


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
dnl check for regex library

dnl yes this test needs to be outside the if below
dnl because autoconf needs to initialise stuff. without this
dnl all sorts of strange things happen
AC_CHECK_HEADERS(regex.h, HAS_REGEX=yes)

AC_ARG_ENABLE(
   internalregex,
   AS_HELP_STRING([--enable-internalregex],[Force internal regex library]),
   internalregex=$enableval
)
if test "$internalregex" = "yes" ; then
  AC_MSG_NOTICE(Forcing use of internal regex library)
else
  if test "$HAS_REGEX" = "yes" ; then
    AC_CHECK_FUNC(regcomp, HAS_REGEX=yes)
    if test "$HAS_REGEX" = "yes" ; then
      AC_SUBST(HAS_REGEX, 1)
      AC_DEFINE(P_REGEX, 1, "Regex library found")
      if test ${target_os} = mingw ; then
        ENDLDLIBS="$ENDLDLIBS -lregex"
      fi
    fi
  fi
fi

PTLIB_NEW_OPTION(REGEX)


dnl ########################################################################
dnl check for dynamic library code (dlopen and friends)

dnl MSWIN_DISPLAY dll,DLL Support
dnl MSWIN_DEFINE  dll,P_DYNALINK


dnl #########################################################################
dnl check to see if samples are enabled

PTLIB_SIMPLE_OPTION([samples], [SAMPLES], [enable samples build])

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
   [enable_resolver=$default_resolver]
)

HAS_DNS_RESOLVER=
if test "$enable_resolver" = "no" ; then
  AC_MSG_NOTICE(DNS resolver disabled)
else
  PTLIB_FIND_RESOLVER([HAS_DNS_RESOLVER=1], [HAS_DNS_RESOLVER=])

  if test "x${HAS_DNS_RESOLVER}" = "x1" ; then
    AC_DEFINE(P_DNS_RESOLVER, 1, "resolver libraries found")
    if test "${HAS_RES_NINIT}z" != "z" ; then
      AC_DEFINE(P_HAS_RES_NINIT)
    fi
  fi
fi

AC_SUBST(HAS_DNS_RESOLVER)
PTLIB_NEW_OPTION(DNS_RESOLVER)


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
   [enable_sasl=$default_sasl]
)

if test "$enable_sasl" = "no" ; then
  AC_MSG_NOTICE(SASL disabled)
else 

  AC_CHECK_LIB([sasl2], [sasl_version], [HAS_SASL2=1], [HAS_SASL2=])
  AC_CHECK_LIB([sasl], [sasl_client_out], [HAS_SASL=1], [HAS_SASL=])

  PTLIB_CHECK_SASL_INCLUDE([
                            if test "x${SASL_HEADER}" = "xsasl" ; then
                              AC_DEFINE(P_HAS_SASL_SASL_H)
                            fi
                           ],
                           [
                            HAS_SASL=
                            HAS_SASL2=
                           ])

  if test "x${HAS_SASL2}" = "x1" ; then
    ENDLDLIBS="$ENDLDLIBS -lsasl2"
    AC_DEFINE_UNQUOTED(P_SASL,2)
  elif test "x${HAS_SASL}" = "x1" ; then
    ENDLDLIBS="$ENDLDLIBS -lsasl"
    AC_DEFINE_UNQUOTED(P_SASL,1)
  fi
fi dnl disable_Sasl
AC_SUBST(HAS_SASL2)
PTLIB_NEW_OPTION(SASL2)
AC_SUBST(HAS_SASL)
PTLIB_NEW_OPTION(SASL)


dnl ########################################################################
dnl check for SUN mediaLib library

AC_ARG_ENABLE(
   [mlib],
   AS_HELP_STRING([--enable-mlib],[enable SUN mlib support]),
   [enable_mlib=$enableval],
   [enable_mlib=$default_mlib]
)
if test "$enable_mlib" = "no" ; then
  AC_MSG_NOTICE(SUN mlib disabled)
else 
  HAS_MEDIALIB=no
  AC_CHECK_LIB(mlib, mlib_VideoColorJFIFYCC2RGB420_Nearest, HAS_MEDIALIB=yes, HAS_MEDIALIB=no)
  if test $HAS_MEDIALIB = yes; then
    AC_MSG_NOTICE(SUN mlib enabled)
    AC_DEFINE(P_MEDIALIB)
    ENDLDLIBS="-lmlib $ENDLDLIBS"
  else
    AC_MSG_ERROR(SUN mlib not found, 1)
  fi
fi

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

AC_ARG_ENABLE(
   [openldap],
   AS_HELP_STRING([--disable-openldap],[disable OpenLDAP support]),
   [enable_openldap=$enableval],
   [enable_openldap=$default_openldap]
)

HAS_LDAP=
if test "$enable_openldap" = "no" ; then
  AC_MSG_NOTICE(OpenLDAP disabled)
else 
  PTLIB_FIND_OPENLDAP([HAS_LDAP=1], [HAS_LDAP=])
  if test "x${HAS_LDAP}" = "x1" ; then
    AC_DEFINE(P_LDAP, 1, "OpenLDAP found")
    ENDLDLIBS="$ENDLDLIBS $OPENLDAP_LIBS"
    PTLIB_CFLAGS="$PTLIB_CFLAGS $OPENLDAP_CFLAGS"
  fi
fi

AC_SUBST(HAS_LDAP)
PTLIB_NEW_OPTION(LDAP)


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
dnl MSWIN_CHECK_DIR  openssl,\openssl\
dnl MSWIN_CHECK_DIR  openssl,\OpenSSL-Win32

dnl MSWIN_DISPLAY    openssl64,Open SSL (64 bit)
dnl MSWIN_CHECK_FILE openssl64,include\openssl\ssl.h,P_SSL_WIN64=1
dnl MSWIN_DIR_SYMBOL openssl64,SSL_WIN64_DIR
dnl MSWIN_CHECK_DIR  openssl64,%ProgramFiles%\OpenSSL-Win64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramW6432%\OpenSSL-Win64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramFiles%\OpenSSL64\
dnl MSWIN_CHECK_DIR  openssl64,%ProgramW6432%\OpenSSL64\
dnl MSWIN_CHECK_DIR  openssl64,\OpenSSL-Win64

AC_ARG_ENABLE(
   [openssl],
   AS_HELP_STRING([--disable-openssl],[disable OpenSSL support]),
   [enable_openssl=$enableval],
   [enable_openssl=$default_openssl]
)

if test "$enable_openssl" = "no" ; then
  AC_MSG_NOTICE(disabled)
else
  PKG_CHECK_MODULES([OPENSSL], 
                    [openssl], 
                    [
                     HAS_SSL=1
                     AC_DEFINE(P_SSL, 1, "OpenSSL found")
                     PTLIB_CHECK_OPENSSL_AES([
                                              AC_DEFINE([P_SSL_AES], [1],[openssl with AES yes])
                                              HAS_SSL_AES=1
                                            ])
                     PTLIB_OPENSSL_CONST( [AC_DEFINE([P_SSL_USE_CONST], [1], [SSL with const])] )
                     PTLIB_CFLAGS="$PTLIB_CFLAGS ${OPENSSL_CFLAGS}"
                     ENDLDLIBS="$ENDLDLIBS ${OPENSSL_LIBS}"
dnl                      PKG_REQUIRES="$PKG_REQUIRES ssl"
                    ], 
                    [HAS_SSL=])
fi 

AC_SUBST(HAS_SSL)
PTLIB_NEW_OPTION(SSL)

AC_SUBST(HAS_SSL_AES)
PTLIB_NEW_OPTION(SSL_AES)

dnl For now...
AC_SUBST(P_STATIC_ENDLDLIBS)
AC_SUBST(P_STATIC_LDFLAGS)



dnl ########################################################################
dnl check for variants of d2i_DHparams_bio from OpenSSL


AC_MSG_CHECKING(for d2i_DHparams_bio variants)
AC_TRY_COMPILE([#include <openssl/dh.h>
                #include <openssl/asn1.h>],
               [ BIO * in; d2i_DHparams_bio(in, NULL);],,P_d2i_DHparams_bio_OLD=1)
if test "x$P_d2i_DHparams_bio_OLD" = "x1" ; then
  AC_MSG_RESULT(old)
  AC_DEFINE(P_d2i_DHparams_bio_OLD, 1)
else
  AC_MSG_RESULT(new)
fi



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

AC_ARG_ENABLE(
   [expat],
   AS_HELP_STRING([--disable-expat],[disable expat XML support]),
   [enable_expat=$enableval],
   [enable_expat=$default_expat]
)


if test "$enable_expat" = "no" ; then
  AC_MSG_NOTICE(Expat disabled)
else
  PTLIB_FIND_EXPAT([HAS_EXPAT=1], [HAS_EXPAT=])

  if test "x${HAS_EXPAT}" = "x1" ; then
    AC_DEFINE(P_EXPAT, 1, "expat found")
    PTLIB_CFLAGS="$PTLIB_CFLAGS $EXPAT_CFLAGS"
    ENDLDLIBS="$ENDLDLIBS $EXPAT_LIBS"
  fi
fi
AC_SUBST(HAS_EXPAT)
PTLIB_NEW_OPTION(EXPAT)


dnl ########################################################################
dnl look for Lua library
dnl MSWIN_DISPLAY    lua,Lua interpreter
dnl MSWIN_CHECK_FILE lua,include\lua.h,P_LUA_WIN32=1
dnl MSWIN_DIR_SYMBOL lua,LUA_DIR
dnl MSWIN_CHECK_DIR  lua,%ProgramFiles%\Lua\5.1
dnl MSWIN_CHECK_DIR  lua,%ProgramW6432%\Lua\5.1

AC_ARG_ENABLE(
   [lua],
   AS_HELP_STRING([--disable-lua],[disable Lua script support]),
   [enable_lua=$enableval],
   [enable_lua=$default_lua]
)

HAS_LUA=
if test "x$enable_lua" = "xno" ; then
   AC_MSG_NOTICE(LUA disabled by user)
else
   PKG_CHECK_MODULES(
      [LUA], 
      [lua5.1], 
      [HAS_LUA=1],
      [PKG_CHECK_MODULES(
         [LUA], 
         [lua], 
         [HAS_LUA=1],
         [HAS_LUA=]
      )]
   )
fi

if test "x${HAS_LUA}" != "x" ; then
   AC_DEFINE(P_LUA, 1, "Lua found")
   PTLIB_CFLAGS="$PTLIB_CFLAGS $LUA_CFLAGS"
   LDFLAGS="$LDFLAGS $LUA_LDFLAGS"
   ENDLDLIBS="$ENDLDLIBS $LUA_LIBS"
fi
AC_SUBST(HAS_LUA)
PTLIB_NEW_OPTION(LUA)


dnl ########################################################################
dnl look for V8 library
dnl MSWIN_DISPLAY    v8,V8 Javascript interpreter
dnl MSWIN_CHECK_FILE v8,include\v8.h,P_V8_WIN32=1
dnl MSWIN_DIR_SYMBOL v8,V8_DIR
dnl MSWIN_CHECK_DIR  v8,..\external\v8

AC_ARG_ENABLE(
   [v8],
   AS_HELP_STRING([--disable-v8],[disable V8 Javascript script support]),
   [enable_v8=$enableval],
   [enable_v8=$default_v8]
)

HAS_V8=
if test "x$enable_v8" = "xno" ; then
   AC_MSG_NOTICE(V8 disabled by user)
else
  AC_MSG_CHECKING(for V8 Javascript support)
  SAVED_LDFLAGS=$LDFLAGS
  LDFLAGS="$LDFLAGS -lv8"
  AC_LINK_IFELSE(
     [AC_LANG_PROGRAM([#include <v8.h>],
                      [int i = v8::HandleScope::NumberOfHandles()]
                    )],
     [HAS_V8=1] ; [AC_MSG_RESULT([yes])],
     [HAS_V8=]  ; [AC_MSG_RESULT([no])]
  )
  LDFLAGS=$SAVED_LDFLAGS
fi

if test "x${HAS_V8}" != "x" ; then
  AC_DEFINE(P_V8, 1, "V8 found")
  PTLIB_CFLAGS="$PTLIB_CFLAGS $V8_CFLAGS"
  LDFLAGS="$LDFLAGS $V8_LDFLAGS"
  ENDLDLIBS="$ENDLDLIBS -lv8"
fi
AC_SUBST(HAS_V8)
PTLIB_NEW_OPTION(V8)


dnl ########################################################################
dnl look for IPV6 functions

dnl MSWIN_DISPLAY    ipv6,IPv6 Support
dnl MSWIN_CHECK_FILE ipv6,include\ws2tcpip.h,P_HAS_IPV6=1
dnl MSWIN_CHECK_FILE ipv6,include\um\ws2tcpip.h,P_HAS_IPV6=1
dnl MSWIN_CHECK_DIR  ipv6,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  ipv6,%ProgramW6432%\Windows Kits\8.0

AC_ARG_ENABLE(ipv6, AS_HELP_STRING([--disable-ipv6],[disable IPV6 support]))

if test "${enable_ipv6}z" = "z" ; then
  enable_ipv6=$default_ipv6
fi

HAS_IPV6=

if test "$enable_ipv6" = "no" ; then
  AC_MSG_NOTICE(IPV6 disabled)
else
  AC_MSG_CHECKING(for IPv6 support)
  AC_TRY_COMPILE([#include <sys/types.h>
                  #include <netinet/in.h>],
                  [struct sockaddr_in6 s; s.sin6_scope_id = 0;], HAS_IPV6=yes, HAS_IPV6=no)
  if test $HAS_IPV6 = no ; then
    AC_MSG_RESULT(no)
  else
    AC_TRY_COMPILE([#include <sys/types.h>
                     #include <sys/socket.h>
                     #include <netdb.h>],
                     [getnameinfo(0, 0, 0, 0, 0, 0, 0);], HAS_IPV6=yes, HAS_IPV6=no)
    if test $HAS_IPV6 = no ; then
      AC_MSG_RESULT(no)
    else
      AC_MSG_RESULT(yes)
      AC_DEFINE(P_HAS_IPV6, 1, "IPV6 enabled")
      HAS_IPV6=1
    fi
  fi
fi
AC_SUBST(HAS_IPV6)
PTLIB_NEW_OPTION(IPV6)

HAS_INET_NTOP=
AC_MSG_CHECKING(for inet_ntop)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <netinet/in.h>
                #include <arpa/inet.h>],
                [int af; void * src; char * dst; socklen_t len; inet_ntop(af, src, dst, len);], HAS_INET_NTOP=yes, HAS_INET_NTOP=no)
if test $HAS_INET_NTOP = no ; then
  AC_MSG_RESULT(no)
else
  AC_DEFINE(P_HAS_INET_NTOP, 1)
  AC_MSG_RESULT(yes)
fi

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

AC_ARG_ENABLE(pcap, AS_HELP_STRING([--disable-pcap],[disable PCAP support]))

if test "x$enable_pcap" = "x" ; then
   enable_pcap=$default_pcap
fi

HAS_PCAP=
if test "$enable_pcap" = "no" ; then
   AC_MSG_NOTICE(PCAP disabled)
else
   AC_MSG_CHECKING(for PCAP support)
   AC_TRY_COMPILE(
      [#include <pcap.h>],
      [pcap_t *h;],
      [
         AC_DEFINE(P_PCAP, 1, "PCAP found")
         ENDLDLIBS="$ENDLDLIBS -lpcap"
         HAS_PCAP=yes
      ],
      [HAS_PCAP=no]
   )
   AC_MSG_RESULT($HAS_PCAP)
fi
AC_SUBST(HAS_PCAP)
PTLIB_NEW_OPTION(PCAP)


dnl ########################################################################
dnl check for enabling config file support
dnl MSWIN_DISPLAY    configfile,Config file protocol support
dnl MSWIN_DEFINE     configfile,P_CONFIG_FILE

PTLIB_SIMPLE_OPTION([configfile], [CONFIG_FILE], [enable config file support])
PTLIB_NEW_OPTION(CONFIG_FILE)

dnl ########################################################################
dnl check for URL support
dnl MSWIN_DISPLAY    url,URL Support
dnl MSWIN_DEFINE     url,P_URL

PTLIB_SIMPLE_OPTION([url], [URL], [enable URL support])
PTLIB_NEW_OPTION(URL)

dnl ########################################################################
dnl check for HTTP support
dnl MSWIN_DISPLAY    http,HTTP Support
dnl MSWIN_DEFINE     http,P_HTTP
dnl MSWIN_IF_FEATURE http,url

PTLIB_SIMPLE_OPTION([http], [HTTP], [enable HTTP support], [HAS_URL])
PTLIB_NEW_OPTION(HTTP)

dnl ########################################################################
dnl check for HTTPFORMS support
dnl MSWIN_DISPLAY    httpforms,HTTP forms Support
dnl MSWIN_DEFINE     httpforms,P_HTTPFORMS
dnl MSWIN_IF_FEATURE httpforms,http&configfile

PTLIB_SIMPLE_OPTION([httpforms], [HTTPFORMS], [enable HTTP forms support], [HAS_HTTP], [HAS_CONFIG_FILE])
PTLIB_NEW_OPTION(HTTPFORMS)


dnl ########################################################################
dnl check for HTTPSVC support
dnl MSWIN_DISPLAY    httpsvc,HTTP service Support
dnl MSWIN_DEFINE     httpsvc,P_HTTPSVC
dnl MSWIN_IF_FEATURE httpsvc,httpforms

PTLIB_SIMPLE_OPTION([httpsvc], [HTTPSVC], [enable HTTP server support], [HAS_HTTPFORMS])
PTLIB_NEW_OPTION(HTTPSVC)

dnl ########################################################################
dnl check for SSDP support
dnl MSWIN_DISPLAY    ssdp,SSDP support
dnl MSWIN_DEFINE     ssdp,P_SSDP
dnl MSWIN_IF_FEATURE ssdp,http

PTLIB_SIMPLE_OPTION([ssdp], [SSDP], [enable SSDP support], [HAS_HTTP])

dnl ########################################################################
dnl check for VCard support
dnl MSWIN_DISPLAY    vcard,VCard support
dnl MSWIN_DEFINE     vcard,P_VCARD

PTLIB_SIMPLE_OPTION([vcard], [VCARD], [enable VCard support], [HAS_URL])

dnl ########################################################################
dnl Jabber
dnl MSWIN_DISPLAY    jabber,Jabber
dnl MSWIN_DEFINE     jabber,P_JABBER
dnl MSWIN_IF_FEATURE jabber,expat
dnl MSWIN_IF_FEATURE jabber,expat32

PTLIB_SIMPLE_OPTION([jabber], [JABBER], [enable Jabber support], [HAS_EXPAT])
PTLIB_NEW_OPTION(JABBER)

dnl ########################################################################
dnl Speech
dnl MSWIN_DISPLAY    sapi,Speech API
dnl MSWIN_CHECK_FILE sapi,include\sphelper.h,P_SAPI=1
dnl MSWIN_CHECK_FILE sapi,include\um\sphelper.h,P_SAPI=1
dnl MSWIN_DEFINE     sapi,P_TTS
dnl MSWIN_CHECK_DIR  sapi,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  sapi,%ProgramW6432%\Windows Kits\8.0

PTLIB_SIMPLE_OPTION([tts], [TTS], [enable Text-To-Speech server support])
PTLIB_NEW_OPTION(TTS)

dnl ########################################################################
dnl VXML 
dnl MSWIN_DISPLAY    vxml,VXML Support
dnl MSWIN_DEFINE     vxml,P_VXML
dnl MSWIN_IF_FEATURE vxml,http&sapi&expat
dnl MSWIN_IF_FEATURE vxml,http&sapi&expat32

PTLIB_SIMPLE_OPTION([vxml], [VXML], [enable VXML support], [HAS_EXPAT], [HAS_HTTP], [HAS_TTS])
PTLIB_NEW_OPTION(VXML)

dnl ########################################################################
dnl XMLRPC
dnl MSWIN_DISPLAY    xmlrpc,XMLRPC Support
dnl MSWIN_DEFINE     xmlrpc,P_XMLRPC
dnl MSWIN_IF_FEATURE xmlrpc,http&expat
dnl MSWIN_IF_FEATURE xmlrpc,http&expat32

PTLIB_SIMPLE_OPTION([xmlrpc], [XMLRPC], [enable XMLRPC support], [HAS_EXPAT], [HAS_HTTP])
PTLIB_NEW_OPTION(XMLRPC)

dnl ########################################################################
dnl SOAP
dnl MSWIN_DISPLAY    soap,SOAP Support
dnl MSWIN_DEFINE     soap,P_SOAP
dnl MSWIN_IF_FEATURE soap,http&expat
dnl MSWIN_IF_FEATURE soap,http&expat32

PTLIB_SIMPLE_OPTION([soap], [SOAP], [enable SOAP support], [HAS_EXPAT], [HAS_HTTP])
PTLIB_NEW_OPTION(SOAP)

dnl ########################################################################
dnl MSWIN_DISPLAY       atl,Active Template Library
dnl MSWIN_CHECK_FILE    atl,atlbase.h,P_ATL=1

dnl ########################################################################
dnl check for enabling ASN support
dnl MSWIN_DISPLAY    asn,ASN Support
dnl MSWIN_DEFINE     asn,P_ASN

PTLIB_SIMPLE_OPTION([asn], [ASN], [enable ASN decoding/encoding support])
PTLIB_NEW_OPTION(ASN)

dnl ########################################################################
dnl check for enabling NAT support
dnl MSWIN_DISPLAY    nat,NAT Support
dnl MSWIN_DEFINE     nat,P_NAT
dnl MSWIN_DISPLAY    nat,NAT traversal support
 
PTLIB_SIMPLE_OPTION([nat],     [NAT],     [enable NAT traversal support])
PTLIB_NEW_OPTION(NAT)

dnl ########################################################################
dnl check for enabling STUN support
dnl MSWIN_DISPLAY    stun,STUN Support
dnl MSWIN_DEFINE     stun,P_STUN
dnl MSWIN_DISPLAY    stunsrv,STUN server support
dnl MSWIN_DEFINE     stunsrv,P_STUNSRVR
 
PTLIB_SIMPLE_OPTION([stun],     [STUN],     [enable STUN support],        [HAS_NAT])
PTLIB_NEW_OPTION(STUN)
PTLIB_SIMPLE_OPTION([stunsrvr], [STUNSRVR], [enable STUN server support], [HAS_NAT])
PTLIB_NEW_OPTION(STUNSRVR)

dnl ########################################################################
dnl check for enabling PIPECHAN support
dnl MSWIN_DISPLAY    pipechan,pipe channel Support
dnl MSWIN_DEFINE     pipechan,P_PIPECHAN

PTLIB_SIMPLE_OPTION([pipechan], [PIPECHAN], [enable pipechannel support])
PTLIB_NEW_OPTION(PIPECHAN)

dnl ########################################################################
dnl check for enabling DTMF support
dnl MSWIN_DISPLAY  dtmf,DTMF encoding/decoding support
dnl MSWIN_DEFINE   dtmf,P_DTMF

PTLIB_SIMPLE_OPTION([dtmf], [DTMF], [enable DTMF protocol support])
PTLIB_NEW_OPTION(DTMF)

dnl ########################################################################
dnl check for enabling SOCKS support
dnl MSWIN_DISPLAY    socks,SOCKS protocol support
dnl MSWIN_DEFINE     socks,P_SOCKS

PTLIB_SIMPLE_OPTION([socks], [SOCKS], [enable SOCKS protocol support])
PTLIB_NEW_OPTION(SOCKS)

dnl ########################################################################
dnl check for enabling FTP support
dnl MSWIN_DISPLAY    ftp,FTP protocol support
dnl MSWIN_DEFINE     ftp,P_FTP

PTLIB_SIMPLE_OPTION([ftp], [FTP], [enable FTP protocol support])
PTLIB_NEW_OPTION(FTP)

dnl ########################################################################
dnl check for enabling SNMP support
dnl MSWIN_DISPLAY    snmp,SNMP protocol support
dnl MSWIN_DEFINE     snmp,P_SNMP

PTLIB_SIMPLE_OPTION([snmp], [SNMP], [enable SNMP protocol support])
PTLIB_NEW_OPTION(SNMP)

dnl ########################################################################
dnl check for enabling TELNET support
dnl MSWIN_DISPLAY    telnet,Telnet protocol support
dnl MSWIN_DEFINE     telnet,P_TELNET

PTLIB_SIMPLE_OPTION([telnet], [TELNET], [enable TELNET protocol support])
PTLIB_NEW_OPTION(TELNET)

dnl ########################################################################
dnl check for enabling CLI support
dnl MSWIN_DISPLAY    cli,Command ine interpreter support
dnl MSWIN_DEFINE     cli,P_CLI

PTLIB_SIMPLE_OPTION([cli], [CLI], [enable command line interpreter support])
PTLIB_NEW_OPTION(CLI)

dnl ########################################################################
dnl check for enabling REMCONN support
dnl MSWIN_DISPLAY    remconn,remote connection support
dnl MSWIN_DEFINE     remconn,P_REMCONN

PTLIB_SIMPLE_OPTION([remconn], [REMCONN], [enable remote connection support], [HAS_CONFIG_FILE], [HAS_PIPECHAN])
PTLIB_NEW_OPTION(REMCONN)

dnl ########################################################################
dnl check for enabling serial port handling
dnl MSWIN_DISPLAY    serial,Serial Support
dnl MSWIN_DEFINE     serial,P_SERIAL

PTLIB_SIMPLE_OPTION([serial], [SERIAL], [enable serial port support])
PTLIB_NEW_OPTION(SERIAL)

dnl ########################################################################
dnl check for POP3/SMTP support
dnl MSWIN_DISPLAY    pop3smtp,POP3/SMTP Support
dnl MSWIN_DEFINE     pop3smtp,P_POP3SMTP

PTLIB_SIMPLE_OPTION([pop3smtp], [POP3SMTP], [enable POP3/SMTP support])
PTLIB_NEW_OPTION(POP3SMTP)

dnl ########################################################################
dnl check for sound support
dnl MSWIN_DISPLAY audio,Sound support
dnl MSWIN_DEFINE  audio,P_AUDIO

PTLIB_SIMPLE_OPTION([audio], [AUDIO], [enable audio support])
PTLIB_NEW_OPTION(AUDIO)

dnl ########################################################################
dnl check for enabling WAVFILE support
dnl MSWIN_DISPLAY    wavfile,WAV file support
dnl MSWIN_DEFINE     wavfile,P_WAVFILE

PTLIB_SIMPLE_OPTION([wavfile], [WAVFILE], [enable WAVFILE support], [HAS_AUDIO])
PTLIB_NEW_OPTION(WAVFILE)


dnl ########################################################################
dnl look for QoS functions

dnl MSWIN_DISPLAY    gqos,Microsoft Windows GQOS Support
dnl MSWIN_CHECK_FILE gqos,include\qossp.h|QOS_DESTADDR,P_GQOS=1

dnl MSWIN_DISPLAY    qwave,Microsoft Windows qWAVE (QoS) Support
dnl MSWIN_CHECK_FILE qwave,include\qos2.h,P_QWAVE=1
dnl MSWIN_CHECK_FILE qwave,include\qos2.h|QOSSetOutgoingDSCPValue,P_QWAVE_DSCP=1


dnl ########################################################################
dnl check for video support
dnl MSWIN_DISPLAY    video,Video Support
dnl MSWIN_DEFINE     video,P_VIDEO

PTLIB_SIMPLE_OPTION([video], [VIDEO], [enable video device support])
PTLIB_NEW_OPTION(VIDEO)

if test "x${HAS_VIDEO}" != "x1" ; then
  AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
fi

dnl ########################################################################
dnl check for enabling video file support
dnl MSWIN_DISPLAY    vidfile,Video file support
dnl MSWIN_DEFINE     vidfile,P_VIDFILE

PTLIB_SIMPLE_OPTION([vidfile], [VIDFILE], [enable video file support], [HAS_VIDEO])
PTLIB_NEW_OPTION(VIDFILE)

dnl ########################################################################
dnl check for enabling FFMpeg video file support
dnl MSWIN_DISPLAY    ffvdev,FFMpeg video file support
dnl MSWIN_DEFINE     ffvdev,P_FFVDEV

PTLIB_SIMPLE_OPTION([ffvdev], [FFVDEV], [enable FFMpeg video file support], [HAS_FFVDEV], [HAS_PIPECHAN])
PTLIB_NEW_OPTION(FFVDEV)

dnl ########################################################################
dnl check for enabling app share support
dnl MSWIN_DISPLAY    appshare,Application share support
dnl MSWIN_DEFINE     appshare,P_APPSHARE

PTLIB_SIMPLE_OPTION([appshare], [APPSHARE], [enable application sharing support], [HAS_VIDEO])

dnl ########################################################################
dnl enable shared memory video devices (static video plugin)

PTLIB_SIMPLE_OPTION([shmvideo], [SHM_VIDEO], [enable shared memory video devices])
PTLIB_NEW_OPTION(SHM_VIDEO)

dnl ########################################################################
dnl look for SDL, but only if we have pthreads

dnl MSWIN_DISPLAY    libsdl,Simple DirectMedia Layer
dnl MSWIN_CHECK_FILE libsdl,lib\x86\SDL.lib,P_SDL_WIN32=1
dnl MSWIN_CHECK_FILE libsdl,lib\x64\SDL.lib,P_SDL_WIN64=1
dnl MSWIN_DIR_SYMBOL libsdl,SDL_DIR
dnl MSWIN_CHECK_DIR  libsdl,..\sdl*
dnl MSWIN_CHECK_DIR  libsdl,..\external\sdl*
dnl MSWIN_CHECK_DIR  libsdl,\sdl\
dnl MSWIN_CHECK_DIR  libsdl,c:\sdl\

AC_ARG_ENABLE(
   [sdl],
   AS_HELP_STRING([--disable-sdl],[disable SDL support]),
   [enable_sdl=$enableval],
   [enable_sdl=$default_sdl]
)

HAS_SDL=
if test "x${HAS_VIDEO}" != "x1" ; then
    AC_MSG_NOTICE(SDL disabled due to disabled dependency HAS_VIDEO)
else
  if test "$enable_sdl" = "no" ; then
    AC_MSG_NOTICE(SDL disabled)
  else
    PKG_CHECK_MODULES([SDL], 
                      [sdl], 
                      [
                       HAS_SDL=1
                       AC_DEFINE(P_SDL, 1, "SDL found")
                       PTLIB_CFLAGS="$PTLIB_CFLAGS ${SDL_CFLAGS}"
                       LDFLAGS="$LDFLAGS `pkg-config --libs-only-L sdl`"
                       ENDLDLIBS="$ENDLDLIBS `pkg-config --libs-only-l sdl`"
                      ],[HAS_SDL=])
  fi
fi
AC_SUBST(HAS_SDL)
PTLIB_NEW_OPTION(SDL)


dnl ########################################################################
dnl check for video capture support.
dnl videodev.h on linux
dnl machine/ioctl_meteor on FreeBSD
dnl i386/ioctl_meteor on OpenBSD
dnl dev/ic/bt8xx on NetBSD

HAS_VIDEO_CAPTURE=
if test "x${HAS_VIDEO}" = "x1" ; then
  if test "x${HAS_PLUGINS}" = "x1" ; then
    HAS_VIDEO_CAPTURE=1
  fi
  case "$target_os" in
    cygwin* | mingw* )
  	  PTLIB_FIND_DIRECTX([HAS_VIDEO_CAPTURE=1
	 	              ENDLDLIBS="$ENDLDLIBS ${DIRECTX_LIBS}"
			      PTLIB_CFLAGS="$PTLIB_CFLAGS ${DIRECTX_INCLUDES}"
                              HAS_DIRECTSHOW=1
                              HAS_DIRECTSOUND=1
			      AC_DEFINE(P_DIRECTSHOW, 1)
			      AC_DEFINE(P_DIRECTX, 1)
			      AC_DEFINE(P_DIRECTSOUND, 1)
		             ])
		;;
  esac

  if test "${HAS_VIDEO_CAPTURE}z" = "z" ; then
    AC_MSG_NOTICE(No Video Capture support)
    AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
  fi
fi

AC_SUBST(HAS_DIRECTSHOW)
AC_SUBST(HAS_DIRECTSOUND)
AC_SUBST(HAS_VIDEO_CAPTURE)
PTLIB_NEW_OPTION(VIDEO_CAPTURE)

PTLIB_SIMPLE_OPTION([vfw], [VFW_CAPTURE], [enable video for windows capture], [HAS_VIDEO])



dnl ########################################################################
dnl look for MEMORY_CHECK enabled.

AC_ARG_ENABLE(memcheck, AS_HELP_STRING([--enable-memcheck],[enable leak testing code (off by default)]))
AC_MSG_NOTICE(Memory checking... $enable_memcheck)
if test "$enable_memcheck" = "yes" ; then
  AC_DEFINE(PMEMORY_CHECK, 1)
fi


dnl ########################################################################
dnl look for ODBC code

dnl MSWIN_DISPLAY    odbc,ODBC support
dnl MSWIN_CHECK_FILE odbc,include\odbcinst.h,P_ODBC=1
dnl MSWIN_CHECK_FILE odbc,include\um\odbcinst.h,P_ODBC=1
dnl MSWIN_CHECK_DIR  odbc,%ProgramFiles%\Windows Kits\8.0
dnl MSWIN_CHECK_DIR  odbc,%ProgramW6432%\Windows Kits\8.0

AC_ARG_ENABLE(
   [odbc],
   AS_HELP_STRING([--disable-odbc],[disable ODBC support]),
   [enable_odbc=$enableval],
   [enable_odbc=$default_odbc]
)

if test "$enable_odbc" = "no" ; then
  AC_MSG_NOTICE(ODBC disabled)
else
  PTLIB_FIND_ODBC([HAS_ODBC=1], [HAS_ODBC=])

  if test "x${HAS_ODBC}" = "x1" ; then
    AC_DEFINE(P_ODBC, 1, "ODBC found")
    PTLIB_CFLAGS="$PTLIB_CFLAGS $ODBC_CFLAGS"
    ENDLDLIBS="$ENDLDLIBS $ODBC_LIBS"
    AC_MSG_NOTICE(ODBC support enabled)
  fi
fi
AC_SUBST(HAS_ODBC)
PTLIB_NEW_OPTION(ODBC)

dnl ########################################################################
dnl output make directives

dnl resolver needs to be at the end
if test "${HAS_DNS_RESOLVER:-unset}" != "unset" ; then
   ENDLDLIBS="$ENDLDLIBS ${RESOLVER_LIBS}"
fi

dnl ########################################################################
dnl this reduces the size of all executables and libraries

case "$target_os" in
  solaris* | sunos* ) ;;
  *)
    PTLIB_SIMPLE_OPTION([exceptions], [EXCEPTIONS], [enable C++ exceptions])

    if test "x${HAS_EXCEPTIONS}" = "x1" ; then
      PTLIB_CFLAGS="$PTLIB_CFLAGS -fexceptions"
    else
      PTLIB_CFLAGS="$PTLIB_CFLAGS -fno-exceptions"
    fi
    ;;
esac


dnl ########################################################################
dnl Plug ins!
dnl

dnl MSWIN_DISPLAY plugins,Plugin Support
dnl MSWIN_DEFINE  plugins,P_PLUGINS

case "$target_os" in
   cygwin* | mingw* )
      PTLIB_SIMPLE_OPTION([plugins], [PLUGINS], [enable plugin support])
      AC_DEFINE(P_DYNALINK, 1, "Support for dynamic loading of libraries")
   ;;

   *)
      PTLIB_FIND_LIBDL([HAS_LIBDL=1], [HAS_LIBDL=])
      if test "x$HAS_LIBDL" = "x1" ; then
         AC_DEFINE(P_DYNALINK, 1, "Support for dynamic loading of libraries")
         ENDLDLIBS="$ENDLDLIBS $DL_LIBS"
         PTLIB_CFLAGS="$PTLIB_CFLAGS $DL_CFLAGS"
      fi

      PTLIB_SIMPLE_OPTION([plugins], [PLUGINS], [enable plugin support], [HAS_LIBDL])
   ;;
esac

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


   DEV_PLUGIN_DIR="${PTLIB_PLUGIN_DIR}/devices"
   AC_SUBST(DEV_PLUGIN_DIR)


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
         AC_CHECK_HEADER(
            [esd.h],
            [
               AC_SUBST(HAS_ESD, 1)
               PTLIB_ESD=yes
            ]
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

      AC_ARG_ENABLE(pulse, AS_HELP_STRING([--disable-pulse],[disable Pulse audio support]))
      PTLIB_PULSE=no
      if test "x${enable_pulse}" = "xno" ; then
         AC_MSG_NOTICE(OSS audio disabled by user)
      else
         AC_CHECK_HEADER(
            [pulse/simple.h],
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
         AC_MSG_NOTICE(OSS audio disabled by user)
      else
         AC_CHECK_HEADER(
            [sys/mixer.h],
            [
               AC_SUBST(HAS_SUNAUDIO, 1)
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
            [sem_timedwait],
            [
               AC_SUBST(HAS_AUDIOSHM, 1)
               PTLIB_AUDIOSHM=yes
            ]
         )
      fi

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
         case "$target_os" in
            solaris* | sunos* ) V4L2_HEADER="sys/videodev2.h"   ; ;;
            netbsd* )           V4L2_HEADER="sys/videoio.h"     ; ;;
            * )	        V4L2_HEADER="linux/videodev2.h" ; ;;
         esac

         AC_CHECK_HEADER(
            [$V4L2_HEADER], 
            [HAS_V4L2=1],
            [
		          # We can't use AC_CHECK_HEADER again because
                          # the value is cached, perhaps we have another
                          # method to do this more cleanly ?
		          AC_MSG_CHECKING([for $V4L2_HEADER with sys/types.h])
		          AC_TRY_COMPILE([
					      #ifdef __linux__
					      #include <sys/types.h>
					      #include <linux/types.h>
                                              #else
                                              #ifdef __sun
                                              #include <sys/types.h>
                                              #include <sys/time.h>
                                              #else
					      #error "not supported"
					      #endif
					      #endif
					      #include "$V4L2_HEADER"
				          ],
				          [
				              return 0;
				          ],
				          [
				              HAS_V4L2=1
				              AC_MSG_RESULT(yes)
				          ],
                                          [
				              HAS_V4L2=
				              AC_MSG_RESULT(no)
                                          ])
                        ], 
		      [
		      #ifdef __linux__
		      #include <linux/types.h>
		      #endif
		      ])
        AC_MSG_CHECKING(for V4L2 video support)
        if test "x${HAS_V4L2}" = "x1"; then
          AC_MSG_RESULT(yes)
          PTLIB_V4L2=yes
          AC_SUBST([V4L2_CXXFLAGS],[-DV4L2_HEADER=\'\"$V4L2_HEADER\"\'])
        else
          HAS_V4L2=
          PTLIB_V4L2=no
          AC_MSG_RESULT(no)
        fi
        AC_SUBST(HAS_V4L2)

        if test "x${HAS_V4L2}" = "x1"; then
          AC_CHECK_HEADER([libv4l2.h], 
                          [AC_CHECK_LIB([v4l2], [v4l2_fd_open], [HAS_LIBV4L=1])]
                         )
                   
          AC_MSG_CHECKING([for libv4l2 support])
          if test "x${HAS_LIBV4L}" = "x1"; then
            AC_MSG_RESULT(yes)
          else
            AC_MSG_RESULT(no)
          fi
          AC_SUBST(HAS_LIBV4L)
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

#until avc lib header API change is solved, default to disabled.
#      AC_ARG_ENABLE(avc, AS_HELP_STRING([--disable-avc],[disable IEEE1394/AVC video support]))
AC_ARG_ENABLE(avc, AS_HELP_STRING([--enable-avc],[enable IEEE1394/AVC video support - NOTE DOES NOT COMPILE!]),
[enable_avc="$enableval"],[enable_avc=no])
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

#until avc lib header API change is solved, default to disabled.
#      AC_ARG_ENABLE(dc, AS_HELP_STRING([--disable-dc],[disable IEEE1394/DC video support]))
AC_ARG_ENABLE(dc, AS_HELP_STRING([--enable-dc],[enable IEEE1394/DC video support - NOTE DOES NOT COMPILE!]),
[enable_dc="$enableval"],[enable_dc=no])
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

   AC_CONFIG_FILES(plugins/Makefile)
   AC_CONFIG_FILES(plugins/vidinput_v4l2/Makefile)
   AC_CONFIG_FILES(plugins/vidinput_dc/Makefile)

fi # enable_plugins


dnl ########################################################################
dnl final declarations

AC_SUBST(CC)
AC_SUBST(CXX)
AC_SUBST(PTLIB_CFLAGS)
AC_SUBST(PTLIB_CXXFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(LDSOFLAGS)
AC_SUBST(ENDLDLIBS)
AC_SUBST(DEBUG_FLAG)

AC_PROG_INSTALL

AC_CONFIG_FILES(ptlib.pc)
AC_CONFIG_FILES(ptlib_cfg.dxy)
AC_CONFIG_FILES(make/ptbuildopts.mak)

dnl ########################################################################
dnl output header file

if test -n "$exec_profix" -a "$exec_prefix" != "NONE" ; then
  cfg_prefix=$exec_prefix/
fi

AC_CONFIG_HEADERS(${cfg_prefix}include/ptbuildopts.h:include/ptbuildopts.h.in)

AC_OUTPUT()

dnl ########################################################################
dnl Summary
dnl ########################################################################
echo ""
echo "=========================== Configuration =============================="
echo "                          OS Type :  $target_os"
echo "                     Machine Type :  $target_cpu"
echo "                           prefix :  $prefix"
echo "                      exec_prefix :  $exec_prefix"
echo "                           libdir :  $libdir"

echo ""

echo "                     DNS Resolver :  ${PTLIB_DNS_RESOLVER}"
echo "                             IPv6 :  ${PTLIB_IPV6}"
echo "            Packet Capture (PCAP) :  ${PTLIB_PCAP}"
echo "               Expat (XML parser) :  ${PTLIB_EXPAT}"
echo "                          OpenSSL :  ${PTLIB_SSL}"
echo "                       SASL/SASL2 :  ${PTLIB_SASL}/${PTLIB_SASL2}"
echo "                             LDAP :  ${PTLIB_LDAP}"
echo "                             ODBC :  ${PTLIB_ODBC}"
echo "                              Lua :  ${PTLIB_LUA}"
echo "                             Java :  ${PTLIB_V8} (V8)"

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

echo ""
echo "========================================================================"
