AC_INIT(include/ptlib.h)

AC_CONFIG_SUBDIRS(plugins)

AC_PROG_CXX

AC_LANG(C++)

dnl ########################################################################
dnl set the PWLIB directory to the current directory

PWLIBDIR=`pwd`
AC_SUBST(PWLIBDIR)
dnl set the pwlib installation dir for pwlib-config
AC_SUBST(PWINSTDIR)

dnl ########################################################################
dnl set the PREFIX accordingly
if test "x$prefix" = "xNONE"; then
   INSTALLPREFIX="/usr/local"
else
   INSTALLPREFIX="${prefix}"
fi

AC_SUBST(INSTALLPREFIX)


dnl ########################################################################
dnl set LIBDIR accordingly
LIBDIR=${libdir}
AC_SUBST(LIBDIR)


dnl ########################################################################
dnl extract the PWLIB version
MAJOR_VERSION=`cat ${PWLIBDIR}/version.h | grep MAJOR_VERSION | cut -f3 -d' '`
MINOR_VERSION=`cat ${PWLIBDIR}/version.h | grep MINOR_VERSION | cut -f3 -d' '`
BUILD_NUMBER=`cat ${PWLIBDIR}/version.h | grep BUILD_NUMBER | cut -f3 -d' '`
PWLIB_VERSION="${MAJOR_VERSION}.${MINOR_VERSION}.${BUILD_NUMBER}"
AC_SUBST(MAJOR_VERSION)
AC_SUBST(MINOR_VERSION)
AC_SUBST(BUILD_NUMBER)
AC_SUBST(PWLIB_VERSION)
AC_SUBST(MAJOR_VERSION, $MAJOR_VERSION)
AC_SUBST(MINOR_VERSION, $MINOR_VERSION)
AC_SUBST(BUILD_NUMBER, $BUILD_NUMBER)

AC_MSG_NOTICE(PTLib version is ${PWLIB_VERSION});

AC_DEFINE_UNQUOTED([PWLIB_MAJOR],   [$MAJOR_VERSION], [Major version])
AC_DEFINE_UNQUOTED([PWLIB_MINOR],   [$MINOR_VERSION], [Minor version])
AC_DEFINE_UNQUOTED([PWLIB_BUILD],   [$BUILD_NUMBER], [Build number])
AC_DEFINE_UNQUOTED([PWLIB_VERSION], ["$PWLIB_VERSION"], [pwlib version])

dnl ########################################################################
dnl set the OSTYPE and MACHTYPE

AC_CANONICAL_TARGET()

TARGETDIR=unix
OSTYPE=

case "$target_os" in
  linux*)          OSTYPE=linux ;  
                   OSRELEASE="\"`uname -r`\"";
		   OS_TAG="P_LINUX" ;
		   STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
                   ;;

  freebsd* )       OSTYPE=FreeBSD ;
		   OS_TAG="P_FREEBSD" ;
                   if test "x$OSRELEASE" = "x"; then
                     OSRELEASE="`sysctl -n kern.osreldate`" ;
                   fi
		   STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
		   ;;

  openbsd* )       OSTYPE=OpenBSD ;
		   OS_TAG="P_OPENBSD" ;
                   OSRELEASE="`sysctl -n kern.osrevision`" ;
		   ENDLDLIBS="-lossaudio" ;
		   STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
		   ;;

  netbsd* )        OSTYPE=NetBSD ;
		   OS_TAG="P_NETBSD" ;
                   OSRELEASE="`/sbin/sysctl -n kern.osrevision`" ;
		   ENDLDLIBS="-lossaudio" ;
		   STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
		   ;;

  solaris* | sunos* ) OSTYPE=solaris ;
		   OS_TAG="P_SOLARIS" ;
                   OSRELEASE=`uname -r | sed "s/5\.//g"` ;
		   STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
		   ;;

  darwin* )	   OSTYPE=Darwin ;
		   OS_TAG="P_MACOSX" ;
		   OSRELEASE="`uname -r | sed 's/\.//g'`" ;
                   SHAREDLIBEXT="dylib";
		   ;;

  beos*)           OSTYPE=beos ;
                   OS_TAG="P_BEOS" ;
                   OSRELEASE=`uname -r` ;
                   STDCCFLAGS="$STDCCFLAGS -D__BEOS__"
		   ;;

  * )		   OSTYPE="$target_os" ;
		   OS_TAG="P_UNKNOWN_OS" ;
  		   OSRELEASE="\"${OSTYPE}_`uname -r`\"" ;
		   AC_MSG_WARN("OS $target_os not recognized - proceed with caution!") ;
		   ;;
esac

MACHTYPE=

case "$target_cpu" in
   x86 | i686 | i586 | i486 | i386 ) MACHTYPE=x86
                   ;;

   x86_64)	   MACHTYPE=x86_64 ;
		   P_64BIT=1 ;
                   LIB64=1 ;
		   ;;

   alpha | alphaev56 | alphaev6 | alphaev67 | alphaev7) MACHTYPE=alpha ;
		   P_64BIT=1 ;
		   ;;

   sparc )         MACHTYPE=sparc ;
		   ;;

   powerpc )       MACHTYPE=ppc ;
		   ;;

   ppc )           MACHTYPE=ppc ;
		   ;;

   powerpc64 )     MACHTYPE=ppc64 ;
		   P_64BIT=1 ;
                   LIB64=1 ;
		   ;;

   ppc64 )         MACHTYPE=ppc64 ;
		   P_64BIT=1 ;
                   LIB64=1 ;
		   ;;

   ia64)	   MACHTYPE=ia64 ;
		   P_64BIT=1 ;
	  	   ;;

   s390x)	   MACHTYPE=s390x ;
		   P_64BIT=1 ;
                   LIB64=1 ;
		   ;;

   s390)	   MACHTYPE=s390 ;
		   ;;

   * )		   MACHTYPE=$target_cpu ;
		   AC_MSG_WARN("CPU $target_cpu not recognized - proceed with caution!") ;;
esac

AC_MSG_NOTICE(OSTYPE set to $OSTYPE);
AC_MSG_NOTICE(OSRELEASE set to $OSRELEASE);
AC_MSG_NOTICE(MACHTYPE set to $MACHTYPE);

AC_SUBST(MACHTYPE,  $MACHTYPE)
AC_SUBST(OSTYPE,    $OSTYPE)
AC_SUBST(OSRELEASE, $OSRELEASE)
AC_SUBST(TARGETDIR, $TARGETDIR)

AC_DEFINE_UNQUOTED($OS_TAG, $OSRELEASE)

AC_DEFINE([PHAS_TEMPLATES], [1], [Templates found])

if test "x$P_64BIT" != "x" ; then
  AC_DEFINE([P_64BIT], [1], [64bit architecture])
fi

dnl BeOS has it;s own threading system
if test ${OSTYPE} != beos ; then
  AC_DEFINE(P_PTHREADS, 1, "PThreads found")
fi

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
if test ${ac_cv_c_bigendian} = yes ; then
  PBYTE_ORDER=PBIG_ENDIAN
else
  PBYTE_ORDER=PLITTLE_ENDIAN
fi

AC_DEFINE_UNQUOTED(PBYTE_ORDER, $PBYTE_ORDER, "byte order")

dnl ########################################################################
dnl look for compile flags
AC_LANG_PUSH(C++)

AC_MSG_CHECKING(if linker accepts -felide-constructors)
AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
$CXX $CXXFLAGS -felide-constructors conftest.cc > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  STDCXXFLAGS="$STDCXXFLAGS -felide-constructors"
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if linker accepts -Wreorder)
AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
$STDCXX $CXXFLAGS -Wreorder conftest.cc > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  STDCXXFLAGS="$STDCXXFLAGS -Wreorder"
else
  AC_MSG_RESULT(no)
fi

# AC_MSG_CHECKING(if linker accepts -Woverloaded-virtual)
# AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
# $CXX $CFLAGS -Woverloaded-virtual conftest.cc > /dev/null 2>&1
# if test $? = 0 ; then
#   AC_MSG_RESULT(yes)
#   STDCCFLAGS="$STDCCFLAGS -Woverloaded-virtual"
# else
#   AC_MSG_RESULT(no)
# fi

AC_MSG_CHECKING(if compiler uses RTTI by default)
AC_COMPILE_IFELSE([[
#include <typeinfo>
int main(int ac,char **av)
{
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
  return 0;
}
]],DEFAULT_RTTI=1, DEFAULT_RTTI=0)
if test "$DEFAULT_RTTI" = "1" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
  STDCXXFLAGS="$STDCCFLAGS -rtti"
fi

GCSECTIONS=0

dnl Remove --gc-sections as the linker crashes when linking
dnl static executables with this option
dnl AC_MSG_CHECKING(if linker accepts --gc-sections)
dnl C_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
dnl CXX $CFLAGS -Wl,--gc-sections conftest.cc > /dev/null 2>&1
dnl if test $? = 0 ; then

if test 0 = 1 ;  then

  dnl  AC_MSG_RESULT(yes)

  AC_MSG_CHECKING(if compiler accepts -ffunction-sections)
  AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
  $CXX $CFLAGS -ffunction-sections -c conftest.cc > /dev/null 2>&1
  if test $? = 0 ; then
    AC_MSG_RESULT(yes)
    STDCCFLAGS="$STDCCFLAGS -ffunction-sections"
    GCSECTIONS=1
  else
    AC_MSG_RESULT(no)
  fi

  dnl remove data-sections as not only does is case the linker to
  dnl crash on some platforms, but seems to be broken w.r.t
  dnl closure of templates with static member functions
  dnl see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=8249
  dnl
  dnl AC_MSG_CHECKING(if compiler accepts -fdata-sections)
  dnl AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
  dnl $CXX $CFLAGS -fdata-sections -c conftest.cc > /dev/null 2>&1
  dnl if test $? = 0 ; then
  dnl   AC_MSG_RESULT(yes)
  dnl   STDCCFLAGS="$STDCCFLAGS -fdata-sections"
  dnl   GCSECTIONS=1
  dnl else
  dnl   AC_MSG_RESULT(no)
  dnl fi

  dnl as much as this flag would help, there is no known linker
  dnl that can handle pwlib when it has been compiled with it
  dnl AC_MSG_CHECKING(if compiler accepts -fvtable-gc)
  dnl AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
  dnl $CXX $CFLAGS -fvtable-gc -c conftest.cc > /dev/null 2>&1
  dnl if test $? = 0 ; then
  dnl   AC_MSG_RESULT(yes)
  dnl   STDCCFLAGS="$STDCCFLAGS -fvtable-gc"
  dnl   GCSECTIONS=1
  dnl else
  dnl   AC_MSG_RESULT(no)
  dnl fi

  if test $GCSECTIONS = 1; then
    if test "${OSTYPE}" != "Darwin" ; then
      P_STATIC_LDFLAGS="$P_STATIC_LDFLAGS -Wl,--gc-sections"
    fi
  fi
fi


AC_LANG_POP(C++)

dnl ########################################################################
dnl check for long double

AC_C_LONG_DOUBLE
if test $ac_cv_c_long_double != yes ; then
  AC_DEFINE([PNO_LONG_DOUBLE], [], [No long double])
fi

if test ${OSTYPE} = Darwin ; then
  STDCCFLAGS="$STDCCFLAGS -Wno-long-double"
fi

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
dnl check for recvmsg

P_HAS_RECVMSG=0
AC_MSG_CHECKING(for recvmsg)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>],
               [int t = SOL_IP; t = IP_PKTINFO; struct msghdr *msg; recvmsg(0, msg, 0);], P_HAS_RECVMSG=1)
if test "${P_HAS_RECVMSG}" = "1" ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_DEFINE_UNQUOTED(P_HAS_RECVMSG, ${P_HAS_RECVMSG})

dnl ########################################################################
dnl check for streams types

P_HAS_STL_STREAMS=0
AC_MSG_CHECKING(if using STL streams)
AC_TRY_COMPILE([#include <iostream>],
               [int v = (int)std::ios_base::seekdir();], P_HAS_STL_STREAMS=1)
if test "${P_HAS_STL_STREAMS}" = 1 ; then
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_DEFINE_UNQUOTED(P_HAS_STL_STREAMS, ${P_HAS_STL_STREAMS})

dnl ########################################################################
dnl see if using GNU glibc++, because then we have atomic integer operations

P_HAS_ATOMIC_INT=0
P_NEEDS_GNU_CXX_NAMESPACE=0
AC_MSG_CHECKING(if atomic integer available)
AC_TRY_COMPILE([#include <bits/atomicity.h>],
               [_Atomic_word val;], P_HAS_ATOMIC_INT=1)
if test ${P_HAS_ATOMIC_INT} = 1 ; then
  AC_MSG_RESULT(yes)
  AC_MSG_CHECKING(if __exchange_and_add is in __gnu_cxx namespace)
  AC_TRY_COMPILE([#include <bits/atomicity.h>],
                 [_Atomic_word val; __gnu_cxx::__exchange_and_add(&val, 1)], P_NEEDS_GNU_CXX_NAMESPACE=1)
  if test ${P_NEEDS_GNU_CXX_NAMESPACE} = 1 ; then
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
  fi
else
  AC_MSG_RESULT(no)
fi
AC_DEFINE_UNQUOTED(P_HAS_ATOMIC_INT, ${P_HAS_ATOMIC_INT})
AC_DEFINE_UNQUOTED(P_NEEDS_GNU_CXX_NAMESPACE, ${P_NEEDS_GNU_CXX_NAMESPACE})


dnl ########################################################################
dnl check for select_large_fdset (Solaris)

if test "${OSTYPE:-unknown}" = "solaris"; then
  AC_MSG_CHECKING(checking for FD_SETSIZE)
  _FD_SYSTEM_FILE=/etc/system
  _FD_SETSIZE=`cat $_FD_SYSTEM_FILE | grep rlim_fd_max | cut -c1`
  if test "$_FD_SETSIZE" = "#"; then
    _FD_SETSIZE=4098
  else
    _FD_SETSIZE=`cat $_FD_SYSTEM_FILE | grep rlim_fd_max | cut -f2 -d'='`
    if test "x$_FD_SETSIZE" = "x"; then
      _FD_SETSIZE=4098
    else
      STDCCFLAGS="$STDCCFLAGS -DFD_SETSIZE=$_FD_SETSIZE"
    fi
  fi
  AC_MSG_RESULT($_FD_SETSIZE)
fi

dnl ########################################################################
dnl check for Unix semaphores

dnl posix semaphore support in darwin/macosx is broken
if test ${OSTYPE} != Darwin ; then
  AC_TRY_COMPILE([#include <semaphore.h>],
                 [sem_t *s; sem_init(s, 0, 0);], P_HAS_SEMAPHORES=1)
  AC_MSG_CHECKING(if Unix semaphores are available)
  if test "${P_HAS_SEMAPHORES}x" != "x" ; then
    AC_DEFINE(P_HAS_SEMAPHORES)
    AC_MSG_RESULT(yes)
  else
    AC_MSG_RESULT(no)
    AC_MSG_WARN(This host does not support semaphores which are essential)
    AC_MSG_WARN(to implement 'critical sections'. This feature was added)
    AC_MSG_WARN(in PWLib 1.7.0 and improves the stability of multithreaded)
    AC_MSG_WARN(programs. This feature has been disabled for this host)
  fi
fi

dnl ########################################################################
dnl check for pthreads library

dnl MSWIN_DISPLAY semaphore,Semaphore Support
dnl MSWIN_DEFINE  semaphore,P_HAS_SEMAPHORES

dnl Linux, Mac OS X use libpthread and -lpthread
dnl FreeBSD uses the -pthread compiler option and uses libc_r

AC_CHECK_LIB(pthread, pthread_create, HAS_PTHREADS=yes, HAS_PTHREADS=no)
if test ${HAS_PTHREADS} = yes ; then
  ENDLDLIBS="$ENDLDLIBS -lpthread"
  STDCCFLAGS="$STDCCFLAGS -D_REENTRANT"
else
  dnl FreeBSD checks
  AC_CHECK_LIB(c_r, pthread_create, HAS_PTHREADS=yes, HAS_PTHREADS=no)
  if test ${HAS_PTHREADS} = yes ; then
    ENDLDLIBS="$ENDLDLIBS -pthread"
    STDCCFLAGS="$STDCCFLAGS -D_REENTRANT -pthread"
  fi
fi


if test ${HAS_PTHREADS} != yes ; then
  AC_MSG_ERROR(must have pthreads!)
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
  AC_MSG_RESULT(yes)
else
  AC_MSG_RESULT(no)
fi
AC_DEFINE_UNQUOTED(P_HAS_RECURSIVE_MUTEX, ${P_HAS_RECURSIVE_MUTEX})


dnl ########################################################################
dnl check for regex library

dnl yes this test needs to be outside the if below
dnl because autoconf needs to initialise stuff. without this
dnl all sorts of strange things happen
AC_CHECK_HEADERS(regex.h, HAS_REGEX=yes)

AC_ARG_ENABLE(internalregex,
        [  --enable-internalregex  force internal regex library],
        internalregex=$enableval)
if test "$internalregex" = "yes" ; then
  AC_MSG_NOTICE(Forcing use of internal regex library)
else
  if test "$HAS_REGEX" = "yes" ; then
    AC_CHECK_FUNC(regcomp, HAS_REGEX=yes)
    if test "$HAS_REGEX" = "yes" ; then
      AC_SUBST(HAS_REGEX, 1)
      AC_DEFINE(P_REGEX, 1, "Regex library found")
    fi
  fi
fi


dnl ########################################################################
dnl check for dynamic library code (dlopen and friends)

dnl MSWIN DLL Support,,P_DYNALINK

LIBS=""
HAS_LIBDL=no
P_DYNALINK=0
if test ${OSTYPE} = beos ; then
  P_DYNALINK=1
  AC_DEFINE(P_DYNALINK, 1, "dlopen implemented inside pwlib in BeOS")
elif test ${OSTYPE} = Darwin ; then
  P_DYNALINK=1
  AC_DEFINE(P_DYNALINK, 1, "dlopen implemented inside pwlib in Darwin")
else
  AC_CHECK_HEADERS(dlfcn.h, HAS_DLFCN=yes, HAS_DLFCN=no)
  if test "$HAS_DLFCN" = yes ; then
    AC_MSG_CHECKING(if dlopen is available)
    AC_TRY_COMPILE([#include <dlfcn.h>],
                    [void * p = dlopen("lib", 0);], HAS_DLOPEN=yes, HAS_DLOPEN=no)
    if test "$HAS_DLOPEN" = no ; then
      AC_MSG_RESULT(no)
    else
      AC_MSG_RESULT(yes)
      P_DYNALINK=1
      AC_DEFINE(P_DYNALINK, 1, "dlopen found")
      AC_CHECK_LIB(dl,dlopen,HAS_LIBDL=yes, HAS_LIBDL=no)
    fi
  fi
fi

dnl #########################################################################
dnl check to see if plugins enabled
dnl MSWIN Plugin Support,,P_HAS_PLUGINS
plugins="no"
AC_MSG_CHECKING(if plugins support needs to be compiled)
AC_ARG_ENABLE(plugins,
       [  --enable-plugins        enable plugins' support in PWLIB],
       plugins=$enableval)

HAS_PLUGINS=0
if test "${plugins}" = "no" ; then
  AC_MSG_RESULT(no)
elif test "${P_DYNALINK:-unset}" = "unset" ; then
  AC_MSG_RESULT(plugins disabled as dlopen not available)
else
  AC_MSG_RESULT(plugins enabled)
  HAS_PLUGINS=1
  AC_DEFINE(P_HAS_PLUGINS, 1, "plugins enabled")
fi

AC_SUBST(HAS_PLUGINS)

if test "x$LIB64" != "x" ; then
  AC_DEFINE_UNQUOTED([P_DEFAULT_PLUGIN_DIR], ["${INSTALLPREFIX}/lib64/pwlib"], [Default plugin directory])
else
  AC_DEFINE_UNQUOTED([P_DEFAULT_PLUGIN_DIR], ["${INSTALLPREFIX}/lib/pwlib"], [Default plugin directory])
fi

dnl ########################################################################
dnl look for DNS resolver code (must be done before OpenLDAP)
dnl on some systems, the resolver is in libc
dnl but needs resolv.h included which defines res_search as __res_search

dnl MSWIN_DISPLAY    resolver,DNS Resolver
dnl MSWIN_CHECK_FILE resolver,include\windns.h,P_DNS=1
dnl MSWIN_DIR_SYMBOL resolver,DNS_DIR
dnl MSWIN_CHECK_DIR  resolver,\Program Files\Microsoft SDK\

AC_ARG_ENABLE(resolver,
       [  --disable-resolver      disable DNS resolver support in PWLIB],
       resolver=$enableval)

if test "$resolver" = "no" ; then
  AC_DEFINE(P_DNS, 0)
else
  AC_CHECK_FUNC(res_search, HAVE_RESOLVER=1)
  if test "${HAVE_RESOLVER:-unset}" != "unset" ; then
    RESOLVER_LIBS=""
  else
    AC_MSG_CHECKING([for res_search in -lresolv])
    LIBS="-lresolv"
    AC_COMPILE_IFELSE([[
#include <netinet/in.h>
#include <resolv.h>
int main(int argc,char **argv){
 res_search (NULL, 0, 0, NULL, 0);
}]],HAVE_RESOLVER=1)
    if test "${HAVE_RESOLVER:-unset}" != "unset" ; then
      AC_MSG_RESULT(yes)
      RESOLVER_LIBS="-lresolv"
    else
      AC_SEARCH_LIBS(__res_search, resolv, HAVE_RESOLVER=1)
    fi
  fi

  if test "${HAVE_RESOLVER:-unset}" != "unset" ; then
    AC_DEFINE(P_DNS, 1, "resolver libraries found")
  fi
fi

dnl ########################################################################
dnl Look for SASL

dnl MSWIN_DISPLAY    sasl,Cyrus SASL
dnl MSWIN_CHECK_FILE sasl,include\sasl.h,P_SASL=1
dnl MSWIN_DIR_SYMBOL sasl,SASL_DIR
dnl MSWIN_CHECK_DIR  sasl,..\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,..\external\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,c:\cyrus-sasl\
  
AC_ARG_ENABLE(sasl,
       [  --disable-sasl          disable SASL support in PWLIB],
       sasl=$enableval)

if test "$sasl" = "no" ; then
  AC_MSG_NOTICE(SASL disabled)
else 

dnl Not all Unices have SASL installed as a separate library (Solaris)
AC_CHECK_LIB(sasl2, sasl_version, HAS_SASL2=1, HAS_SASL2=,$LDFLAGS -llber -lldap -lldap_r $RESOLVER_LIBS $ENDLDLIBS $OPENDLAPSYSLIBS)
if test "${HAS_SASL2}x" = "x" ; then
  dnl Some older Linux boxes have libsasl instead of libsasl2
  AC_CHECK_LIB(sasl, sasl_client_auth, HAS_SASL=1, HAS_SASL=,$LDFLAGS -llber -lldap -lldap_r $RESOLVER_LIBS $ENDLDLIBS $OPENDLAPSYSLIBS)
fi

dnl we need to track down the correct sasl.h
AC_MSG_CHECKING([if <sasl.h> works])
AC_COMPILE_IFELSE([[#include <sasl.h>
int main(int argc,char **argv){ int v = SASL_LOG_PASS; }]],HAS_INCLUDE_SASL_H=1)
if test "${HAS_INCLUDE_SASL_H}x" != "x" ; then
  AC_MSG_RESULT(yes)
  SASL_INCLUDED=1
else
  AC_MSG_RESULT(no)
  AC_MSG_CHECKING([if <sasl/sasl.h> works])
  AC_COMPILE_IFELSE([[#include <sasl/sasl.h>
  int main(int argc,char **argv){ int v = SASL_LOG_PASS; }]],P_HAS_SASL_SASL_H=1)
  if test "${P_HAS_SASL_SASL_H}x" != "x" ; then
    AC_MSG_RESULT(yes)
    SASL_INCLUDED=1
    AC_DEFINE(P_HAS_SASL_SASL_H)
  else
    AC_MSG_RESULT(no)
  fi
fi

if test "${HAS_SASL2}x" != "x" ; then
  HAS_SASL2=1
  ENDLDLIBS="$ENDLDLIBS -lsasl2"
  AC_DEFINE(P_SASL2)
  AC_SUBST(HAS_SASL2, 1)
elif test "${HAS_SASL}x" != "x" ; then
  HAS_SASL=1
  ENDLDLIBS="$ENDLDLIBS -lsasl"
  AC_DEFINE(P_SASL)
  AC_SUBST(HAS_SASL, 1)
else
  HAS_SASL=0
fi

fi dnl disable_Sasl

dnl ########################################################################
dnl look for OpenLDAP (requires SASL)

dnl MSWIN_DISPLAY    openldap,Open LDAP
dnl MSWIN_CHECK_FILE openldap,include\ldap.h|OpenLDAP,P_LDAP=1
dnl MSWIN_DIR_SYMBOL openldap,LDAP_DIR
dnl MSWIN_CHECK_DIR  openldap,..\openldap\
dnl MSWIN_CHECK_DIR  openldap,..\external\openldap\
dnl MSWIN_CHECK_DIR  openldap,\openldap\
dnl MSWIN_CHECK_DIR  openldap,c:\openldap\

AC_ARG_ENABLE(openldap,
       [  --disable-openldap      disable OpenLDAP support in PWLIB],
       openldap=$enableval)

if test "$openldap" = "no" ; then
  AC_SUBST(HAS_OPENLDAP, 0)
  AC_DEFINE(P_LDAP, 0)
else 
  if test "${HAVE_RESOLVER:-unset}" != "unset" ; then
    AC_CHECK_HEADERS(ldap.h, HAS_OPENLDAP=1)
  
    dnl posix4 is required by libldap_r on Solaris
    if test "${OSTYPE:-unknown}" = "solaris"; then
      OPENLDAPSYSLIBS=-lposix4
    fi

    dnl check also for lldap_r presence (required for thread safe OpenLDAP?)
    if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
      AC_CHECK_LIB(ldap, ldap_open, HAS_OPENLDAP=1, HAS_OPENLDAP=,-llber -lldap_r $RESOLVER_LIBS $ENDLDLIBS $OPENLDAPSYSLIBS)
    else
      AC_CHECK_FILE(/usr/local/include/ldap.h, HAS_OPENLDAP=1)
      if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
        OPENLDAPINCLUDEDIR=/usr/local/include
        OPENLDAPLIBDIR=/usr/local/lib
        AC_CHECK_LIB(ldap, ldap_open, HAS_OPENLDAP=1, HAS_OPENLDAP=,-L/usr/local/lib -llber -lldap_r $RESOLVER_LIBS $ENDLDLIBS $OPENLDAPSYSLIBS)
      fi
    fi
  fi

  if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
    AC_SUBST(HAS_OPENLDAP, 1)
    AC_DEFINE(P_LDAP, 1, "OpenLDAP found")
    ENDLDLIBS="-lldap -lldap_r -llber $ENDLDLIBS"
    if test "${OPENLDAPINCLUDEDIR}unset" != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${OPENLDAPINCLUDEDIR}"
    fi
    if test "${OPENLDAPLIBDIR}unset" != "unset" ; then
      LDFLAGS="$LDFLAGS -L${OPENLDAPLIBDIR}"
    fi
  fi
fi


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

dnl MSWIN_DISPLAY    openssl,Open SSL
dnl MSWIN_CHECK_FILE openssl,inc32\openssl\ssl.h,P_SSL=1
dnl MSWIN_DIR_SYMBOL openssl,SSL_DIR
dnl MSWIN_CHECK_DIR  openssl,..\openssl\
dnl MSWIN_CHECK_DIR  openssl,..\external\openssl\
dnl MSWIN_CHECK_DIR  openssl,\openssl\
dnl MSWIN_CHECK_DIR  openssl,c:\openssl\

AC_ARG_ENABLE(openssl,
       [  --disable-openssl       disable OpenSSL support in PWLIB],
       openssl=$enableval)

if test "$openssl" = "no" ; then
  AC_DEFINE(P_SSL, 0)
  AC_SUBST(HAS_OPENSSL, 0)
else

  dnl check to see OPENSSLDIR is set
  dnl if it is, we assume it correct - no further checking required
  if test ${OPENSSLDIR:-unset} != "unset" ; then
    AC_MSG_CHECKING([for ${OPENSSLDIR}/include/openssl/ssl.h])
    if test -e ${OPENSSLDIR}/include/openssl/ssl.h ; then
      AC_MSG_RESULT(yes)
      SSLHDR=1
      SSLINCLUDEDIR="${OPENSSLDIR}/include"
      SSLLIBDIR="${OPENSSLDIR}/lib"
    else
      AC_MSG_RESULT(no)
    fi
  fi


  dnl if OPENSSLDIR is not set, then check to see if OpenSSL
  dnl is installed in the standard system dirs. And also see if
  dnl if it works using crazy RedHat checks
  if test "${SSLHDR:-unset}" = "unset" ; then
    AC_CHECK_HEADERS(openssl/ssl.h,SSLHDR=1)
    if test "${SSLHDR:-unset}" = "unset" ; then
      AC_MSG_CHECKING([if openssl/ssl.h works with OPENSSL_NO_KRB5])
      AC_COMPILE_IFELSE([[#define OPENSSL_NO_KRB5
#include <openssl/ssl.h>
int main(int argc,char **argv){}]],SSLHDR=1)
      if test "${SSLHDR:-unset}" != "unset" ; then
        AC_MSG_RESULT(yes)
        SSLCCFLAGS=" -DOPENSSL_NO_KRB5"
        P_STATIC_LDFLAGS="$P_STATIC_LDFLAGS -L/usr/kerberos/lib"
        if test ${OSTYPE} = linux ; then
          dnl Linux static libs
          P_STATIC_ENDLDLIBS="-lkrb5 -lk5crypto -lcom_err"
        fi
      else
        AC_MSG_RESULT(no)
      fi
    fi
  fi

  dnl if we have not found OpenSSL so far, then look for it in 
  dnl several standard places, and check for crazy RedHat install too
  if test "${SSLHDR:-unset}" = "unset" ; then

    for openssl_dir in /usr/local/openssl /usr/local/ssl ; do
      AC_MSG_CHECKING([for OpenSSL in $openssl_dir])
      if test ! -f ${openssl_dir}/include/openssl/ssl.h ; then
        AC_MSG_RESULT(no)
      else
        AC_LANG_CONFTEST([[#include "${openssl_dir}/include/openssl/ssl.h"
int main(int argc,char **argv){ return 0; }]])
        $CXX $CXXFLAGS -I${openssl_dir}/include -c conftest.cc > /dev/null 2>&1
        if test $? = 0 ; then
          AC_MSG_RESULT(yes)
          SSLHDR=1
          SSLCCFLAGS=""
          SSLINCLUDEDIR="${openssl_dir}/include"
          SSLLIBDIR="${openssl_dir}/lib"
          break
        else
          AC_MSG_RESULT(no)
          AC_MSG_CHECKING([if ${openssl_dir}/include/openssl/ssl.h works with OPENSSL_NO_KRB5...])
          AC_LANG_CONFTEST([[#define OPENSSL_NO_KRB5
#include "${openssl_dir}/include/openssl/ssl.h"
int main(int argc,char **argv){}]])
          $CXX $CXXFLAGS -I${openssl_dir}/openssl/include -c conftest.cc > /dev/null 2>&1
          if test $? = 0 ; then
            AC_MSG_RESULT(yes)
            SSLHDR=1
            SSLCCFLAGS=" -DOPENSSL_NO_KRB5"
            SSLINCLUDEDIR="${openssl_dir}/include"
            SSLLIBDIR="${openssl_dir}/lib"
            P_STATIC_LDFLAGS="$P_STATIC_LDFLAGS -L/usr/kerberos/lib"
            if test ${OSTYPE} = linux ; then
              dnl Linux static libs
              P_STATIC_ENDLDLIBS="-lkrb5 -lk5crypto -lcom_err"
            fi
            break
          else
            AC_MSG_RESULT(no)
          fi
        fi
      fi
    done
  fi

  if test ${SSLHDR:-unset} != "unset" ; then
    AC_SUBST(HAS_OPENSSL, 1)
    AC_DEFINE(P_SSL, 1, "OpenSSL found")
    if test ${SSLINCLUDEDIR:-unset} != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${SSLINCLUDEDIR}"
    fi
    if test ${SSLLIBDIR:-unset} != "unset" ; then
      LDFLAGS="$LDFLAGS -L${SSLLIBDIR}"
    fi
    STDCCFLAGS="${STDCCFLAGS}${SSLCCFLAGS}"
    ENDLDLIBS="$ENDLDLIBS -lssl -lcrypto"
  fi
fi

AC_SUBST(P_STATIC_ENDLDLIBS)
AC_SUBST(P_STATIC_LDFLAGS)

dnl ########################################################################
dnl look for expat XML parser

dnl MSWIN_DISPLAY    expat,Expat XML
dnl MSWIN_CHECK_FILE expat,lib\expat.h,P_EXPAT=1
dnl MSWIN_DIR_SYMBOL expat,EXPAT_DIR
dnl MSWIN_CHECK_DIR  expat,..\expat\
dnl MSWIN_CHECK_DIR  expat,..\external\expat\
dnl MSWIN_CHECK_DIR  expat,\expat
dnl MSWIN_CHECK_DIR  expat,c:\expat\

AC_ARG_ENABLE(expat,
       [  --disable-expat         disable XML support in PWLIB],
       expat=$enableval)

if test "$expat" = "no" ; then
  AC_SUBST(HAS_EXPAT, 0)
  AC_DEFINE(P_EXPAT, 0)
else

  AC_CHECK_HEADERS(expat.h, HAS_EXPAT=1)
  if test "${HAS_EXPAT:-unset}" != "unset" ; then
    AC_CHECK_LIB(expat, XML_ParserCreate, HAS_EXPAT=1, HAS_EXPAT=)
  else
    AC_CHECK_FILE(/usr/local/include/expat.h, HAS_EXPAT=1)
    if test "${HAS_EXPAT:-unset}" != "unset" ; then
      EXPATINCLUDEDIR=/usr/local/include
      EXPATLIBDIR=/usr/local/lib
    fi
  fi

  if test "${HAS_EXPAT:-unset}" != "unset" ; then
    AC_SUBST(HAS_EXPAT, 1)
    AC_DEFINE(P_EXPAT, 1, "expat found")
    if test ${EXPATINCLUDEDIR:-unset} != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${EXPATINCLUDEDIR}"
    fi
    if test ${EXPATLIBDIR:-unset} != "unset" ; then
      LDFLAGS="$LDFLAGS -L${EXPATLIBDIR}"
    fi
    ENDLDLIBS="$ENDLDLIBS -lexpat"
  fi
fi


dnl ########################################################################
dnl Speech

dnl MSWIN_DISPLAY    sapi,Speech API
dnl MSWIN_CHECK_FILE sapi,include\sphelper.h,P_SAPI=1
dnl MSWIN_DIR_SYMBOL sapi,SAPI_DIR
dnl MSWIN_CHECK_DIR  sapi,\Program Files\Microsoft Speech SDK 5.1\


dnl ########################################################################
dnl look for IPV6 functions

dnl MSWIN_DISPLAY    ipv6,IPv6 Support
dnl MSWIN_CHECK_FILE ipv6,ws2tcpip.h|sin6_scope_id,P_HAS_IPV6=1
dnl MSWIN_DIR_SYMBOL ipv6,IPV6_DIR
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft Visual Studio\VC98\Include\
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft SDK\include\

AC_ARG_ENABLE(ipv6,
       [  --disable-ipv6          disable IPV6 support in PWLIB],
       ipv6=$enableval)

if test "$ipv6" = "no" ; then
  AC_DEFINE(P_HAS_IPV6, 0)
  AC_SUBST(HAS_IPV6, 0)
else
  AC_MSG_CHECKING(for IPv6 support)
  AC_TRY_COMPILE([#include <sys/types.h>
                  #include <netinet/in.h>],
                  [struct sockaddr_in6 s; s.sin6_scope_id = 0;], HAS_IPV6=yes, HAS_IPV6=no)
  if test $HAS_IPV6 = no ; then
    AC_MSG_RESULT(no)
  else
    AC_TRY_COMPILE([#include <sys/socket.h>
                     #include <netdb.h>],
                     [getnameinfo(NULL, 0, NULL, 0, NULL, 0, 0);], HAS_IPV6=yes, HAS_IPV6=no)
    if test $HAS_IPV6 = no ; then
      AC_MSG_RESULT(no)
    else
      AC_MSG_RESULT(yes)
      AC_DEFINE(P_HAS_IPV6, 1, "IPV6 enabled")
    fi
  fi
fi

dnl ########################################################################
dnl look for QoS functions

dnl MSWIN_DISPLAY    qos,QoS Support
dnl MSWIN_CHECK_FILE qos,qossp.h|QOS_DESTADDR,P_HAS_QOS=1
dnl MSWIN_DIR_SYMBOL qos,QOS_DIR
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft Visual Studio\VC98\Include\
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft SDK\include\

AC_ARG_ENABLE(qos,
       [  --disable-qos           disable QOS support in PWLIB],
       qos=$enableval)

if test "$qos" = "no" ; then
  AC_DEFINE([P_HAS_QOS], [0], [No QOS])
else
  AC_DEFINE([P_HAS_QOS], [1], [Have QOS])
fi


dnl ########################################################################
dnl look for SDL, but only if we have pthreads

dnl MSWIN_DISPLAY    libsdl,Simple DirectMedia Layer
dnl MSWIN_CHECK_FILE libsdl,include\SDL_video.h,P_SDL=1
dnl MSWIN_CHECK_FILE libsdl,lib\SDL.lib,P_SDL_LIBDIR="lib"
dnl MSWIN_DIR_SYMBOL libsdl,SDL_DIR
dnl MSWIN_CHECK_DIR  libsdl,..\sdl\
dnl MSWIN_CHECK_DIR  libsdl,..\external\sdl\
dnl MSWIN_CHECK_DIR  libsdl,\sdl\
dnl MSWIN_CHECK_DIR  libsdl,c:\sdl\

AC_ARG_ENABLE(sdl,
       [  --disable-sdl           disable SDL support in PWLIB],
       sdl=$enableval)

if test "$sdl" = "no" ; then
  AC_SUBST(HAS_SDL, 0)
  AC_DEFINE(P_SDL, 0)
else
  dnl SDL checks
  if test "${SDLLIB:-unset}" = "unset" ; then
    SDLLIB=SDL
  fi

  dnl Linux check
  AC_CHECK_HEADERS(SDL/SDL.h, HAS_SDL=1)
  if test "${HAS_SDL:-unset}" != "unset" ; then
    if test "${X11LIBDIR:-unset}" == "unset" ; then
	X11LIBDIR=/usr/X11R6/lib
    fi
    AC_CHECK_LIB(SDL, SDL_CreateYUVOverlay, HAS_SDL=1, HAS_SDL=, -L${X11LIBDIR} -lpthread -lX11 -lXext -laa)
  fi

  dnl freeBSD SDL check
  if test "${HAS_SDL:-unset}" = "unset" ; then
    AC_CHECK_FILE(/usr/local/include/SDL11/SDL.h, HAS_SDL=1)
    if test "${HAS_SDL:-unset}" != "unset" ; then
      SDLLIB=SDL-1.1
    fi
  fi

  if test "${HAS_SDL:-unset}" != "unset" ; then
    AC_SUBST(HAS_SDL, 1)
    AC_DEFINE(P_SDL, 1, "SDL found")
    if test ${SDLINCLUDEDIR:-unset} != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${SDLINCLUDEDIR}"
    fi
    if test ${SDLLIBDIR:-unset} != "unset" ; then
      LDFLAGS="$LDFLAGS -L${SDLLIBDIR}"
    fi
    ENDLDLIBS="$ENDLDLIBS -l${SDLLIB}"
  fi
fi


dnl ########################################################################
dnl check for video capture support.
dnl videodev.h on linux
dnl machine/ioctl_meteor on FreeBSD
dnl i386/ioctl_meteor on OpenBSD
dnl dev/ic/bt8xx on NetBSD

AC_ARG_ENABLE(video,
       [  --disable-video         disable non-plugin video device support in PWLIB],
       video=$enableval)

HAS_VIDEO_CAPTURE=0
if test "$HAS_PLUGINS" = "0" ; then
  if test "$video" != "no" ; then
    AC_CHECK_HEADER(linux/videodev.h, HAS_VIDEO_CAPTURE=1)
    dnl AC_CHECK_HEADER(machine/ioctl_meteor.h, HAS_VIDEO_CAPTURE=1)
    dnl AC_CHECK_HEADER(i386/ioctl_meteor.h, HAS_VIDEO_CAPTURE=1)
    dnl AC_CHECK_HEADER(dev/ic/bt8xx.h, HAS_VIDEO_CAPTURE=1)
    if test "${HAS_VIDEO_CAPTURE}" = "0" ; then
      AC_MSG_NOTICE(No Video Capture support)
      AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
    fi
  fi
fi
AC_SUBST(HAS_VIDEO_CAPTURE)

dnl ########################################################################
dnl enable shared memory video devices (static video plugin)

AC_ARG_ENABLE(shm_video,
		[  --enable-shm-video	  enable shared memory video devices (off by default)],
		shm_video=$enableval)

USE_SHM_VIDEO_DEVICES=0	
if test "$shm_video" = "yes" ; then
  USE_SHM_VIDEO_DEVICES=1
  AC_MSG_NOTICE(Shared memory video devices enabled)
fi
AC_SUBST(USE_SHM_VIDEO_DEVICES)

dnl ########################################################################
dnl check for various functions
AC_ARG_ENABLE(alsa, [  --enable-alsa           disable non-plugin ALSA audio support],enable_alsa=$enableval)

HAS_ALSA=0
if test "$HAS_PLUGINS" = "0" ; then
  if test "$enable_alsa" != "no" ; then
    AC_CHECK_HEADER(alsa/asoundlib.h, HAS_ALSA=1)
    AC_MSG_CHECKING(for ALSA sound support)
    if test "${HAS_ALSA}" = "1"; then
      AC_MSG_RESULT(yes)
      ENDLDLIBS="$ENDLDLIBS -lasound"
    else
      AC_MSG_RESULT(no)
    fi
  fi
fi
AC_SUBST(HAS_ALSA)

dnl ########################################################################
dnl check for various functions
dnl According to Andre Schulze <as8@rncmm2.urz.tu-dresden.de>
dnl Solaris swab should not be used
if test "$OSTYPE" = "solaris" ; then
  USE_SYSTEM_SWAB=
else
  AC_CHECK_FUNC(swab,USE_SYSTEM_SWAB=1)
fi
if test "x$ac_cv_func_swab" = "xyes" ; then
  AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()])
fi

dnl ########################################################################
dnl look for MEMORY_CHECK enabled.

AC_ARG_ENABLE(memcheck,
       [  --enable-memcheck       enable leak testing code (off by default)],
       memcheck=$enableval)

if test "$memcheck" = "yes" ; then
  AC_DEFINE(PMEMORY_CHECK, 1)
  AC_MSG_NOTICE(Memory checking enabled)
fi


dnl ########################################################################
dnl output make directives

dnl resolver needs to be at the end
if test "${HAVE_RESOLVER:-unset}" != "unset" ; then
  if test "${RESOLVER_LIBS:-unset}" != "unset" ; then
    ENDLDLIBS="$ENDLDLIBS $RESOLVER_LIBS"
  fi
fi

dnl dl needs to be at the end
if test "${OSTYPE}" != "Darwin" ; then
  if test ${HAS_LIBDL} = yes ; then
    ENDLDLIBS="$ENDLDLIBS -ldl"
  fi
fi

AC_SUBST(STDCCFLAGS)
AC_SUBST(STDCXXFLAGS)
AC_SUBST(OPTSTDCCFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(ENDLDLIBS)
AC_PROG_INSTALL

AC_CONFIG_FILES(make/ptbuildopts.mak)
AC_CONFIG_FILES([make/ptlib-config], [chmod a+x make/ptlib-config])
AC_CONFIG_FILES(Makefile)

dnl ########################################################################
dnl output header file
#AC_CONFIG_FILES(include/ptbuildopts.h.in)
AC_CONFIG_HEADERS(include/ptbuildopts.h)

AC_OUTPUT()
