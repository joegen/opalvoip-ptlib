AC_INIT(include/ptlib.h)

AC_CONFIG_SUBDIRS(plugins)

dnl PKG_LIBS=
dnl PKG_CFLAGS=
dnl PKG_REQUIRES=
PKG_PROG_PKG_CONFIG()
AC_PROG_CC
AC_PROG_CXX

AC_LANG(C++)

dnl PTLIB_SIMPLE
dnl Change a given variable according to arguments and subst and define it
dnl Arguments: $1 name of configure option
dnl            $2 the variable to change, subst and define
dnl            $3 the configure argument description
dnl            $4 dependency variable #1
dnl            $5 dependency variable #2 
dnl Return:    ${HAS_$2} The (possibly) changed variable
AC_DEFUN([PTLIB_SIMPLE_OPTION],
         [
dnl          if test "x${HAS_$2}" = "x"; then
dnl            AC_MSG_ERROR([No default specified for HAS_$2, please correct configure.ac])
dnl	  fi
          AC_MSG_CHECKING([$3])
          AC_ARG_ENABLE([$1],
                        [AC_HELP_STRING([--enable-$1],[$3])],
                        [
                         if test "x$enableval" = "xyes"; then
                           HAS_$2=1
                         else
                           HAS_$2=
                         fi
                        ])

          if test "x$4" != "x"; then
            if test "x$$4" != "x1"; then
              AC_MSG_NOTICE([$1 support disabled due to disabled dependency $4])
	      HAS_$2=
	    fi
	  fi

          if test "x$5" != "x"; then
            if test "x$$5" != "x1"; then
              AC_MSG_NOTICE([$1 support disabled due to disabled dependency $5])
	      HAS_$2=
	    fi
	  fi


          if test "x${HAS_$2}" = "x1"; then
            AC_DEFINE([P_$2], [1], [$3])
            HAS_$2=1
            AC_MSG_RESULT([yes])
          else
            HAS_$2=
            AC_MSG_RESULT([no])
          fi
          AC_SUBST(HAS_$2)
          
          
         ])

dnl PTLIB_FIND_DIRECTX
dnl Check for directX
dnl Arguments:
dnl Return:    $1 action if-found
dnl            $2 action if-not-found
dnl	       $DIRECTX_INCLUDES
dnl	       $DIRECTX_LIBS
AC_DEFUN([PTLIB_FIND_DIRECTX],
         [
	  ptlib_has_directx=yes
	  DIRECTX_INCLUDES=
	  DIRECTX_LIBS=

	  AC_ARG_WITH([directx-includedir],
	              AS_HELP_STRING([--with-directx-includedir=DIR],[Location of DirectX include files]),
	              [with_directx_dir="$withval"],
		      [with_directx_dir="include"]
	  )
	  
	  AC_MSG_CHECKING(for DirectX includes in ${with_directx_dir})
	  AC_MSG_RESULT()

	  old_CPPFLAGS="$CPPFLAGS"
	  CPPFLAGS="$CPPFLAGS -I${with_directx_dir}"
	  AC_LANG(C++)
	  
	  AC_CHECK_HEADERS([mingw_dshow_port.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([control.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([ddraw.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([dshow.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([dsound.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([dxerr9.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([ksuuids.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([strmif.h], [], [ptlib_has_directx=no])
	  AC_CHECK_HEADERS([uuids.h], [], [ptlib_has_directx=no])
	  CPPFLAGS="$old_CPPFLAGS"


	  if test "x${ptlib_has_directx}" = "xyes" ; then
	    AC_MSG_CHECKING([for DirectX libraries])
	    AC_MSG_RESULT()
	  fi
	  
	  if test "x${ptlib_has_directx}" = "xyes" ; then
	    DIRECTX_INCLUDES="-I${with_directx_dir}"
	    DIRECTX_LIBS="-ldsound -ldxerr9 -ldxguid -lstrmiids -lole32 -luuid -loleaut32 -lquartz"
	  fi
	  
          AS_IF([test AS_VAR_GET([ptlib_has_directx]) = yes], [$1], [$2])[]
         ])

dnl PTLIB_FIND_RESOLVER
dnl Check for dns resolver
dnl Arguments:
dnl Return:    $1 action if-found
dnl            $2 action if-not-found
dnl            $RESOLVER_LIBS
dnl            $$HAS_RES_INIT
AC_DEFUN([PTLIB_FIND_RESOLVER],
         [
          ptlib_has_resolver=no
          HAS_RES_NINIT=

          AC_CHECK_FUNC([res_ninit], 
                        [
                         HAS_RES_NINIT=1
                         ptlib_has_resolver=yes
                        ])

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_MSG_CHECKING([for res_ninit in -lresolv])
            old_LIBS="$LIBS"
            LIBS="$LIBS -lresolv"
            AC_LINK_IFELSE([[
                            #include <netinet/in.h>
                            #include <resolv.h>
                            int main(int argc,char **argv) {
                              res_state p; res_ninit(p);
                            }
                          ]],
                          [
                            HAS_RES_NINIT=1
                            ptlib_has_resolver=yes
                            RESOLVER_LIBS="-lresolv"
                          ])
            LIBS="${old_LIBS}"
            AC_MSG_RESULT(${ptlib_has_resolver})
          fi

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_CHECK_FUNC([res_search], [ptlib_has_resolver=yes])
          fi

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_MSG_CHECKING([for res_search in -lresolv])
            old_LIBS="$LIBS"
            LIBS="$LIBS -lresolv"
            AC_LINK_IFELSE([[
                            #include <netinet/in.h>
                            #include <resolv.h>
                            int main(int argc,char **argv){
                              res_search (NULL, 0, 0, NULL, 0);
                            }
                          ]],
                          [
                            ptlib_has_resolver=yes
                            RESOLVER_LIBS="-lresolv"
                          ])
            LIBS="${old_LIBS}"
            AC_MSG_RESULT(${ptlib_has_resolver})
          fi

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_SEARCH_LIBS([__res_search], [resolv], [ptlib_has_resolver=yes])
          fi

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_SEARCH_LIBS([__res_search], [resolv], [ptlib_has_resolver=yes])
          fi

          if test "x${ptlib_has_resolver}" = "xno" ; then
            AC_CHECK_HEADERS([windns.h],
                            [
                              ptlib_has_resolver=yes
                              RESOLVER_LIBS="-ldnsapi"
                            ])
          fi
          AS_IF([test AS_VAR_GET([ptlib_has_resolver]) = yes], [$1], [$2])[]
         ])

dnl PTLIB_OPENSSL_CONST
dnl Check for directX
dnl Arguments:
dnl Return:    $1 action if-found
dnl            $2 action if-not-found
AC_DEFUN([PTLIB_OPENSSL_CONST],
         [
          ptlib_openssl_const=no
          old_CFLAGS="$CFLAGS"
          CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
          AC_LANG(C)
          AC_MSG_CHECKING(for const arg to d2i_AutoPrivateKey)
          AC_TRY_COMPILE([#include <openssl/evp.h>],
                         [
                           EVP_PKEY **a; const unsigned char **p; long l;
                           d2i_AutoPrivateKey(a, p, l);
                         ],
                         [ptlib_openssl_const=yes]
                        )
          AC_MSG_RESULT(${ptlib_openssl_const})
          AC_LANG(C++)
          CFLAGS="${old_CFLAGS}"

          AS_IF([test AS_VAR_GET([ptlib_openssl_const]) = yes], [$1], [$2])[]
         ])

dnl PTLIB_CHECK_UPAD128
dnl Check for upad128_t (solaris only)
dnl Arguments:
dnl Return:    $1 action if-found
dnl            $2 action if-not-found
AC_DEFUN([PTLIB_CHECK_UPAD128],
         [
           ptlib_upad128=no

           AC_MSG_CHECKING(for upad128_t)
           AC_TRY_COMPILE([#include <sys/types.h>],
                          [upad128_t upad; upad._q = 0.0;],
                          [ptlib_upad128=yes])
           AC_MSG_RESULT(${ptlib_upad128})

           AS_IF([test AS_VAR_GET([ptlib_upad128]) = yes], [$1], [$2])[]
         ])

dnl PTLIB_OPENSSL_AES
dnl Check for directX
dnl Arguments:
dnl Return:    $1 action if-found
dnl            $2 action if-not-found
AC_DEFUN([PTLIB_OPENSSL_AES],
         [
          ptlib_openssl_aes=no
          old_CFLAGS="$CFLAGS"
          CFLAGS="$CFLAGS $OPENSSL_CFLAGS"
          AC_LANG(C)
          AC_CHECK_HEADERS([openssl/aes.h], [ptlib_openssl_aes=yes])
          AC_LANG(C++)
          CFLAGS="${old_CFLAGS}"
          AS_IF([test AS_VAR_GET([ptlib_openssl_aes]) = yes], [$1], [$2])[]
         ])

dnl ########################################################################
dnl libdl
dnl ########################################################################

dnl PTLIB_FIND_LBDL
dnl Try to find a library containing dlopen()
dnl Arguments: $1 action if-found
dnl            $2 action if-not-found
dnl Return:    $DL_LIBS The libs for dlopen()
AC_DEFUN([PTLIB_FIND_LIBDL],
         [
          ptlib_libdl=no
          AC_CHECK_HEADERS([dlfcn.h], [ptlib_dlfcn=yes], [ptlib_dlfcn=no])
          if test "$ptlib_dlfcn" = yes ; then
            AC_MSG_CHECKING(if dlopen is available)
            AC_LANG(C)
            AC_TRY_COMPILE([#include <dlfcn.h>],
                            [void * p = dlopen("lib", 0);], [ptlib_dlopen=yes], [ptlib_dlopen=no])
            if test "$ptlib_dlopen" = no ; then
              AC_MSG_RESULT(no)
            else
              AC_MSG_RESULT(yes)
              case "$target_os" in
                freebsd*|openbsd*|netbsd*|darwin*|beos*) 
                  AC_CHECK_LIB([c],[dlopen],
                              [
                                ptlib_libdl=yes
                                DL_LIBS="-lc"
                              ],
                              [ptlib_libdl=no])
                ;;
                *)
                  AC_CHECK_LIB([dl],[dlopen],
                              [
                                ptlib_libdl=yes
                                DL_LIBS="-ldl"
                              ],
                              [ptlib_libdl=no])
                ;;
               esac
            fi
          fi
          AS_IF([test AS_VAR_GET([ptlib_libdl]) = yes], [$1], [$2])[]
         ])


dnl ########################################################################
dnl set the PTLIB directory to the current directory

PTLIBDIR=`pwd`
AC_SUBST(PTLIBDIR)
dnl set the PTLib installation dir for ptlib-config
AC_SUBST(PTINSTDIR)

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
dnl set the OSTYPE and MACHTYPE

AC_CANONICAL_TARGET()

TARGETDIR=unix
OSTYPE=
NEEDS_PTHREADS=yes

case "$target_os" in
  linux*)          OSTYPE=linux ;  
                   OSRELEASE="\"`uname -r`\"";
		   OS_TAG="P_LINUX" ;
                   need_pragma=yes ;
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
                   ;;

  freebsd* )       OSTYPE=FreeBSD ;
		   OS_TAG="P_FREEBSD" ;
                   if test "x$OSRELEASE" = "x"; then
                     OSRELEASE="`sysctl -n kern.osreldate`" ;
                   fi
                   need_pragma=yes ;
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  openbsd* )       OSTYPE=OpenBSD ;
		   OS_TAG="P_OPENBSD" ;
                   OSRELEASE="`sysctl -n kern.osrevision`" ;
		   ENDLDLIBS="-lossaudio" ;
                   need_pragma=yes ;
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  netbsd* )        OSTYPE=NetBSD ;
		   OS_TAG="P_NETBSD" ;
                   OSRELEASE="`/sbin/sysctl -n kern.osrevision`" ;
		   ENDLDLIBS="-lossaudio" ;
                   need_pragma=yes ;
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  solaris* | sunos* ) OSTYPE=solaris ;
		   OS_TAG="P_SOLARIS" ;
                   OSRELEASE=`uname -r | sed "s/5\.//g"` ;
                   need_pragma=yes ;
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
                   dnl According to Andre Schulze <as8@rncmm2.urz.tu-dresden.de>
                   dnl Solaris swab should not be used
                   PTLIB_CHECK_UPAD128( [AC_DEFINE(P_HAS_UPAD128_T)] )
		   ;;

  darwin* )	   OSTYPE=Darwin ;
		   OS_TAG="P_MACOSX" ;
		   OS_MAJOR=`uname -r | sed 's/\..*$//'`
		   OS_MAJOR=`expr $OS_MAJOR \* 100`
		   OS_MINOR=[`uname -r | sed -e 's/[0-9][0-9]*\.//' -e 's/\..*$//'`]
		   OSRELEASE=`expr $OS_MAJOR + $OS_MINOR`
                   SHAREDLIBEXT="dylib";
                   if test -e /Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6/include/expat.h ; then
                     EXPATDIR=/Developer/SDKs/MacOSX10.4u.sdk/usr/X11R6
                   fi
		   AC_DEFINE(P_PTHREADS, 1, "PThreads found")
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  beos*)           OSTYPE=beos ;
                   OS_TAG="P_BEOS" ;
                   OSRELEASE=`uname -r` ;
                   STDCCFLAGS="$STDCCFLAGS -D__BEOS__"
                   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  cygwin*)         OSTYPE=cygwin ;
                   OS_TAG="P_CYGWIN" ;
                   OSRELEASE="`uname -r`";
                   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  mingw*)	   OSTYPE=mingw ;
		   OS_TAG="P_MINGW" ;
		   OSRELEASE=`uname -r` ;
		   TARGETDIR=msos ;
		   SHAREDLIBEXT="dll" ;
		   STDCCFLAGS="$STDCCFLAGS -mms-bitfields" ;
		   ENDLDLIBS="-lwinmm -lwsock32 -lsnmpapi -lmpr -lcomdlg32 -lgdi32 -lavicap32 -liphlpapi -lole32" ;
		   NEEDS_PTHREADS=no ;
		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
		   ;;

  * )		   OSTYPE="$target_os" ;
		   OS_TAG="P_UNKNOWN_OS" ;
  		   OSRELEASE="\"${OSTYPE}_`uname -r`\"" ;
  		   AC_CHECK_FUNC([swab], [AC_DEFINE([USE_SYSTEM_SWAB], [], [Use system swab()] )] )
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

   hppa )          MACHTYPE=hppa ;
		   ;;

   hppa64 )        MACHTYPE=hppa64 ;
		   P_64BIT=1 ;
                   LIB64=1 ;
		   ;;

   sparc|sparc64 ) MACHTYPE=sparc ;
		   P_64BIT=1 ;
                   LIB64=1 ;
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

if test "x$P_64BIT" != "x" ; then
  AC_DEFINE([P_64BIT], [1], [64bit architecture])
fi

if test "$target_os" = "linux-gnulp" ; then
  AC_DEFINE([P_LPIA], [1], [Low Power Intel Architecture])
fi


dnl solaris has it's own ldap library
if test ${OSTYPE} != solaris ; then
   LDAP_LIBRARIES="-llber -lldap_r"
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
      STDCCFLAGS="$STDCCFLAGS -DP_USE_PRAGMA"
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
    enablepch=no
  else
    enablepch=yes
    AC_ARG_ENABLE(pch,
         [  --enable-pch             disable precompiled headers],
         enablepch=$enableval)
  fi
  if test "${enablepch}" = "yes" ; then
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

if test ${OSTYPE} != "Darwin" ; then  
  AC_C_BIGENDIAN()
  if test ${ac_cv_c_bigendian} = yes ; then
    PBYTE_ORDER=PBIG_ENDIAN
  else
    PBYTE_ORDER=PLITTLE_ENDIAN
  fi

  AC_DEFINE_UNQUOTED(PBYTE_ORDER, $PBYTE_ORDER, "byte order")
fi

AC_MSG_NOTICE(Default extension for C++ files is $ac_ext)

dnl ########################################################################
dnl look for compile flags
AC_LANG_PUSH(C++)

AC_MSG_CHECKING(if linker accepts -felide-constructors)
AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
$CXX $CXXFLAGS -felide-constructors conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  STDCXXFLAGS="$STDCXXFLAGS -felide-constructors"
else
  AC_MSG_RESULT(no)
fi

AC_MSG_CHECKING(if linker accepts -Wreorder)
AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
$CXX $CXXFLAGS -Wreorder conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  STDCXXFLAGS="$STDCXXFLAGS -Wreorder"
else
  AC_MSG_RESULT(no)
fi


dnl add additional information for the debugger to ensure the user can indeed
dnl debug coredumps and macros.

AC_MSG_CHECKING(if debug build accepts -g3 -ggdb -O0)
AC_LANG_CONFTEST([[int main(int ac,char **av) { return 0; }]])
$CXX  $CXXFLAGS  -g3 -ggdb -O0 conftest.$ac_ext > /dev/null 2>&1
if test $? = 0 ; then
  AC_MSG_RESULT(yes)
  DEBUG_FLAG="-g3 -ggdb -O0"
else
  AC_MSG_RESULT(no)
  DEBUG_FLAG="-g"
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
  STDCXXFLAGS="$STDCCFLAGS -frtti"
fi

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

if test ${OSTYPE} = Darwin ; then
  STDCCFLAGS="$STDCCFLAGS -Wno-long-double"
fi

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
dnl we need bison, so we better make sure it's there
AC_PATH_PROG(BISON, bison)
if test -z "$BISON" ; then
        AC_MSG_ERROR(PTLib needs the GNU Project parser generator bison, 1)
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
dnl check for minimum sizing

AC_ARG_ENABLE(minsize,
       [  --enable-minsize        minimise the size of the lib by disabling least used options])
AC_ARG_ENABLE(openh323,
       [  --enable-openh323       include only options needed with openh323])
AC_ARG_ENABLE(opal,
       [  --enable-opal           include only options needed with opal])

dnl these are the default configure settings
HAS_SAMPLES=no
default_tracing=yes
default_resolver=yes
default_openldap=yes
default_openssl=yes
default_expat=yes
default_sdl=yes
default_sasl=yes
HAS_PLUGINS=1
HAS_TTS=1
HAS_ASN=1
HAS_STUN=1
HAS_PIPECHAN=1
HAS_DTMF=1
HAS_WAVFILE=1
HAS_SOCKS=1
HAS_FTP=1
HAS_SNMP=1
HAS_TELNET=1
HAS_REMCONN=1
HAS_SERIAL=1
HAS_POP3SMTP=1
HAS_AUDIO=1
HAS_VIDEO=1
HAS_URL=1
HAS_HTTP=1
HAS_HTTPFORMS=1
HAS_HTTPSVC=1
HAS_SOAP=1
HAS_XMLRPC=1
HAS_JABBER=1
HAS_CONFIG_FILE=1
default_alsa=yes
default_oss=yes
default_ipv6=yes
default_qos=yes
HAS_SOCKAGG=1
HAS_VIDFILE=1
HAS_APPSHARE=
default_shmvideo=yes
HAS_VXML=1

if test "${enable_openh323}x" = "yesx" -o "${enable_opal}x" = "yesx" ; then
  default_resolver=yes
  default_openldap=no
  default_openssl=yes
  default_expat=no
  default_sdl=yes
  default_sasl=no
  HAS_PLUGINS=1
  HAS_TTS=1
  HAS_ASN=1
  HAS_STUN=1
  HAS_PIPECHAN=1
  HAS_DTMF=1
  HAS_WAVFILE=1
  HAS_SOCKS=
  HAS_FTP=
  HAS_SNMP=
  HAS_TELNET=
  HAS_REMCONN=
  HAS_SERIAL=
  HAS_POP3SMTP=
  HAS_AUDIO=1
  HAS_VIDEO=1
  HAS_URL=1
  HAS_HTTP=1
  HAS_HTTPFORMS=
  HAS_HTTPSVC=
  HAS_SOAP=
  HAS_XMLRPC=
  HAS_JABBER=
  HAS_CONFIG_FILE=1
  default_alsa=yes
  default_oss=yes
  default_ipv6=yes
  default_qos=yes
  HAS_SOCKAGG=1
  HAS_VIDFILE=1
  HAS_APPSHARE=
  default_shmvideo=no
  HAS_VXML=1
elif test "${enable_minsize}x" = "yesx" ; then
  default_resolver=no
  default_openldap=no
  default_openssl=no
  default_expat=no
  default_sdl=no
  default_sasl=no
  HAS_PLUGINS=1
  HAS_TTS=
  HAS_ASN=1
  HAS_STUN=1
  HAS_PIPECHAN=
  HAS_DTMF=
  HAS_WAVFILE=
  HAS_SOCKS=
  HAS_FTP=
  HAS_SNMP=
  HAS_TELNET=
  HAS_REMCONN=
  HAS_SERIAL=
  HAS_POP3SMTP=
  HAS_AUDIO=1
  HAS_VIDEO=!
  HAS_URL=1
  HAS_HTTP=1
  HAS_HTTPFORMS=
  HAS_HTTPSVC=
  HAS_SOAP=
  HAS_XMLRPC=
  HAS_JABBER=
  HAS_CONFIG_FILE=0
  default_alsa=yes
  default_oss=no
  default_ipv6=no
  default_qos=no
  HAS_SOCKAGG=1
  HAS_VIDFILE=1
  HAS_APPSHARE=
  default_shmvideo=no
  HAS_VXML=
fi


dnl ########################################################################
dnl check for tracing
dnl

AC_ARG_ENABLE(tracing,
       [  --disable-tracing     Remove PTRACE and all trace logging])

if test "${enable_tracing}" = "no" ; then
  AC_MSG_NOTICE(PTRACE and all trace logging removed)
  STDCCFLAGS="$STDCCFLAGS -DPTRACING=0 -DPASN_NOPRINTON -DPASN_LEANANDMEAN"
else
  AC_MSG_NOTICE(PTRACE and all trace logging included)
  STDCCFLAGS="$STDCCFLAGS -DPTRACING=1"
fi


dnl ########################################################################
dnl check for Windows-like integer BOOL
dnl
dnl MSWIN_DISPLAY ansi-bool,ANSI C++ bool
dnl MSWIN_DEFINE  ansi-bool,P_USE_STANDARD_CXX_BOOL

AC_ARG_ENABLE(ansi-bool,
       [  --disable-ansi-bool     Use Windows-like integer BOOL over ANSI C++ bool])

if test "${enable_ansi_bool}x" = "x" ; then
  AC_MSG_NOTICE(Using ANSI C++ bool for PBoolean: (PFalse=false PTrue=true))
  AC_DEFINE(P_USE_STANDARD_CXX_BOOL)
else
  AC_MSG_NOTICE(Using Windows-like integer BOOL for PBoolean: (PFalse=0 PTrue=1))
fi


dnl ########################################################################
dnl check for recvmsg

P_HAS_RECVMSG=
AC_MSG_CHECKING(for recvmsg)
AC_TRY_COMPILE([#include <sys/types.h>
                #include <sys/socket.h>
                #include <netinet/in.h>],
               [int t = SOL_IP; t = IP_PKTINFO; struct msghdr *msg; recvmsg(0, msg, 0);], P_HAS_RECVMSG=1)
if test "${P_HAS_RECVMSG}" = "1" ; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(P_HAS_RECVMSG, 1)
else
  AC_MSG_RESULT(no)
fi

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

AC_ARG_ENABLE(atomicity,
       [  --disable-atomicity     Do not use atomicity.h])

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

dnl posix unnamed semaphore support in darwin/macosx is broken
if test ${OSTYPE} != Darwin ; then
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

if test "${P_HAS_SEMAPHORES}x" == "x" ; then
  
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
  if test ${OSTYPE} = FreeBSD ; then
    HAS_PTHREADS=yes
    ENDLDLIBS="$ENDLDLIBS -pthread"
    STDCCFLAGS="$STDCCFLAGS -D_REENTRANT -pthread"
  fi
fi


if test ${HAS_PTHREADS} != yes -a ${NEEDS_PTHREADS} = yes; then
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
      if test ${OSTYPE} = mingw ; then
        ENDLDLIBS="$ENDLDLIBS -lregex"
      fi
    fi
  fi
fi


dnl ########################################################################
dnl check for dynamic library code (dlopen and friends)

dnl MSWIN DLL Support,,P_DYNALINK

PTLIB_FIND_LIBDL([HAS_LIBDL=1], [HAS_LIBDL=])
if test "x$HAS_LIBDL" = "x1" ; then
  AC_DEFINE(P_DYNALINK, 1, "dlopen found")
  dnl The rest is done further down...
fi

dnl #########################################################################
dnl check to see if plugins enabled
dnl MSWIN_DISPLAY plugins,Plugin Support
dnl MSWIN_DEFINE  plugins,P_HAS_PLUGINS

PTLIB_SIMPLE_OPTION([plugins], [PLUGINS], [enable plugin support], [HAS_LIBDL])

if test "x$LIB64" != "x" ; then
  AC_DEFINE_UNQUOTED([P_DEFAULT_PLUGIN_DIR], ["${INSTALLPREFIX}/lib64/ptlib"], [Default plugin directory])
else
  AC_DEFINE_UNQUOTED([P_DEFAULT_PLUGIN_DIR], ["${INSTALLPREFIX}/lib/ptlib"], [Default plugin directory])
fi

dnl #########################################################################
dnl check to see if samples are enabled

PTLIB_SIMPLE_OPTION([samples], [SAMPLES], [enable samples build])

dnl ########################################################################
dnl look for DNS resolver code (must be done before OpenLDAP)
dnl on some systems, the resolver is in libc
dnl but needs resolv.h included which defines res_search as __res_search

dnl MSWIN_DISPLAY    resolver,DNS Resolver
dnl MSWIN_CHECK_FILE resolver,include\windns.h,P_DNS=1
dnl MSWIN_DIR_SYMBOL resolver,DNS_DIR
dnl MSWIN_CHECK_DIR  resolver,\program files\microsoft visual studio 8\vc\platformsdk\
dnl MSWIN_CHECK_DIR  resolver,\Program Files\Microsoft SDK\
dnl MSWIN_CHECK_DIR  resolver,\Program Files\Microsoft Platform SDK for Windows Server 2003 R2

AC_ARG_ENABLE([resolver],
	      AS_HELP_STRING([--disable-resolver],[disable DNS resolver support]),
	      [enable_resolver=$enableval],
	      [enable_resolver=$default_resolver]
	     )

HAS_RESOLVER=
if test "$enable_resolver" = "no" ; then
  AC_MSG_NOTICE(DNS resolver disabled)
else
  PTLIB_FIND_RESOLVER([HAS_RESOLVER=1], [HAS_RESOLVER=])

  if test "x${HAS_RESOLVER}" = "x1" ; then
    AC_DEFINE(P_DNS, 1, "resolver libraries found")
    if test "${HAS_RES_NINIT}z" != "z" ; then
      AC_DEFINE(P_HAS_RES_NINIT)
    fi
  fi
fi

AC_SUBST(HAS_RESOLVER)

dnl ########################################################################
dnl Look for SASL

dnl MSWIN_DISPLAY    sasl,Cyrus SASL
dnl MSWIN_CHECK_FILE sasl,include\sasl.h,P_SASL=1
dnl MSWIN_DIR_SYMBOL sasl,SASL_DIR
dnl MSWIN_CHECK_DIR  sasl,..\sasl\
dnl MSWIN_CHECK_DIR  sasl,..\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,..\external\sasl\
dnl MSWIN_CHECK_DIR  sasl,..\external\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,\sasl\
dnl MSWIN_CHECK_DIR  sasl,\cyrus-sasl\
dnl MSWIN_CHECK_DIR  sasl,c:\sasl\
dnl MSWIN_CHECK_DIR  sasl,c:\cyrus-sasl\
  
AC_ARG_ENABLE(sasl,
       [  --disable-sasl          disable SASL support])

if test "${enable_sasl}z" = "z" ; then
  enable_sasl=$default_sasl
fi

HAS_SASL=
HAS_SASL2=

if test "$enable_sasl" = "no" ; then
  AC_MSG_NOTICE(SASL disabled)
else 

  AC_MSG_CHECKING([for sasl_version in -lsasl2])
  my_LIBS=$LIBS
  LIBS="$LIBS -lsasl2"			
  AC_LINK_IFELSE([#ifdef __cplusplus
 				  extern "C"
   				  #endif
 				  char sasl_version ();
 				  int
 				  main ()
 				  {
 				   sasl_version ();
   					;
   				   return 0;
 				  }], HAS_SASL2=1, HAS_SASL2= )
  if test "${HAS_SASL2}x" = "x" ; then
	  AC_MSG_RESULT(no)
	  LIBS=$my_LIBS
	  LIBS="$LIBS -lsasl"
	  AC_MSG_CHECKING([for sasl_client_auth in sasl])
      AC_LINK_IFELSE([#ifdef __cplusplus
                  extern "C"
                  #endif
                  char sasl_client_out ();
                  int
                  main ()
                  {
                   sasl_client_out ();
                    ;
                   return 0;
                  }], HAS_SASL=1, HAS_SASL= )
	  if test "${HAS_SASL}x" = "x" ; then
		AC_MSG_RESULT(no)
	  else
		AC_MSG_RESULT(yes)	
	  fi	
  fi
  AC_MSG_RESULT(yes)

LIBS=$my_LIBS

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
    HAS_SASL2=
  fi
fi

if test "${HAS_SASL2}x" != "x" ; then
  HAS_SASL2=1
  ENDLDLIBS="$ENDLDLIBS -lsasl2"
  AC_DEFINE(P_SASL2)
elif test "${HAS_SASL}x" != "x" ; then
  HAS_SASL=1
  ENDLDLIBS="$ENDLDLIBS -lsasl"
  AC_DEFINE(P_SASL)
else
  HAS_SASL=0
fi
	
fi dnl disable_Sasl

AC_SUBST(HAS_SASL)
AC_SUBST(HAS_SASL2)

dnl ########################################################################
dnl check for mediaLib library

HAS_MEDIALIB=no
AC_CHECK_LIB(mlib, mlib_VideoColorJFIFYCC2RGB420_Nearest, HAS_MEDIALIB=yes, HAS_MEDIALIB=no)
if test $HAS_MEDIALIB = yes; then
    AC_DEFINE(P_MEDIALIB)
    ENDLDLIBS="-lmlib $ENDLDLIBS"
fi

dnl ########################################################################
dnl look for OpenLDAP (requires SASL)

dnl MSWIN_DISPLAY    openldap,Open LDAP
dnl MSWIN_IF_FEATURE openldap,sasl
dnl MSWIN_CHECK_FILE openldap,include\ldap.h|OpenLDAP,P_LDAP=1
dnl MSWIN_DIR_SYMBOL openldap,LDAP_DIR
dnl MSWIN_CHECK_DIR  openldap,..\openldap\
dnl MSWIN_CHECK_DIR  openldap,..\external\openldap\
dnl MSWIN_CHECK_DIR  openldap,\openldap\
dnl MSWIN_CHECK_DIR  openldap,c:\openldap\

AC_ARG_ENABLE(openldap,
       [  --disable-openldap      disable OpenLDAP support])

if test "${enable_openldap}z" = "z" ; then
  enable_openldap=$default_openldap
fi

HAS_OPENLDAP=
if test "$enable_openldap" = "no" ; then
  AC_MSG_NOTICE(OpenLDAP disabled)
else 
  if test "${HAS_RESOLVER:-unset}" != "unset" -o "${OSTYPE}" = "mingw" ; then
    AC_CHECK_HEADERS(ldap.h, HAS_OPENLDAP=1)
  
    dnl posix4 is required by libldap_r on Solaris
    if test "${OSTYPE:-unknown}" = "solaris"; then
      OPENLDAPSYSLIBS=-lposix4
    fi

    dnl check also for lldap_r presence (required for thread safe OpenLDAP?)
    if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
      AC_CHECK_LIB(ldap, ldap_open, HAS_OPENLDAP=1, HAS_OPENLDAP=,$LDAP_LIBRARIES $RESOLVER_LIBS $ENDLDLIBS $OPENLDAPSYSLIBS)
    else
      AC_CHECK_FILE(/usr/local/include/ldap.h, HAS_OPENLDAP=1)
      if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
        OPENLDAPINCLUDEDIR=/usr/local/include
        OPENLDAPLIBDIR=/usr/local/lib
        AC_CHECK_LIB(ldap, ldap_open, HAS_OPENLDAP=1, HAS_OPENLDAP=,-L/usr/local/lib $LDAP_LIBRARIES $RESOLVER_LIBS $ENDLDLIBS $OPENLDAPSYSLIBS)
      fi
    fi
  fi

  if test "${HAS_OPENLDAP:-unset}" != "unset" ; then
    HAS_OPENLDAP=1
    AC_DEFINE(P_LDAP, 1, "OpenLDAP found")
    ENDLDLIBS="-lldap $LDAP_LIBRARIES $ENDLDLIBS"
    if test "${OPENLDAPINCLUDEDIR}unset" != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${OPENLDAPINCLUDEDIR}"
    fi
    if test "${OPENLDAPLIBDIR}unset" != "unset" ; then
      LDFLAGS="$LDFLAGS -L${OPENLDAPLIBDIR}"
    fi
  fi
fi

AC_SUBST(HAS_OPENLDAP)


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
dnl MSWIN_CHECK_FILE openssl,inc32\openssl\ssl.h,P_OSSL=1
dnl MSWIN_CHECK_FILE openssl,include\openssl\ssl.h,P_OSSL=2
dnl MSWIN_FIND_FILE  openssl,P_OSSL_AES,aes.h,...
dnl MSWIN_DIR_SYMBOL openssl,SSL_DIR
dnl MSWIN_CHECK_DIR  openssl,..\openssl\
dnl MSWIN_CHECK_DIR  openssl,..\external\openssl\
dnl MSWIN_CHECK_DIR  openssl,\openssl\
dnl MSWIN_CHECK_DIR  openssl,\libs\openssl\
dnl MSWIN_CHECK_DIR  openssl,\Program Files\OpenSSL\
dnl MSWIN_CHECK_DIR  openssl,c:\openssl\

AC_ARG_ENABLE([openssl],
	      AS_HELP_STRING([--disable-openssl],[disable OpenSSL support]),
	      [enable_openssl=$enableval],
	      [enable_openssl=$default_openssl]
	     )

if test "$enable_openssl" = "no" ; then
  AC_MSG_NOTICE(OpenSSL disabled)
else
  PKG_CHECK_MODULES([OPENSSL], 
                    [openssl], 
                    [
                     HAS_OPENSSL=1
                     AC_DEFINE(P_OSSL, 1, "OpenSSL found")
                     PTLIB_OPENSSL_AES( [AC_DEFINE([P_OSSL_AES], [1],[openssl with AES yes])] )
                     PTLIB_OPENSSL_CONST( [AC_DEFINE([P_SSL_USE_CONST], [1], [SSL with const])] )
                     STDCCFLAGS="$STDCCFLAGS ${OPENSSL_CFLAGS}"
                     ENDLDLIBS="$ENDLDLIBS ${OPENSSL_LIBS}"
dnl                      PKG_REQUIRES="$PKG_REQUIRES ssl"
                    ], 
                    [HAS_OPENSSL=])
fi

AC_SUBST(HAS_OPENSSL)

dnl For now...
AC_SUBST(P_STATIC_ENDLDLIBS)
AC_SUBST(P_STATIC_LDFLAGS)




dnl ########################################################################
dnl look for expat XML parser

dnl MSWIN_DISPLAY    expat,Expat XML
dnl MSWIN_CHECK_FILE expat,lib\expat.h,P_WINEXPAT=1
dnl MSWIN_CHECK_FILE expat,Source\lib\expat.h,P_WINEXPAT=2
dnl MSWIN_DIR_SYMBOL expat,EXPAT_DIR
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_DEBUG,expat.lib,Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_W32_RELEASE,expat.lib,Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM5_DEBUG,expat.lib,wm5/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM5_RELEASE,expat.lib,wm5/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM6_DEBUG,expat.lib,wm6/Debug
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_WM6_RELEASE,expat.lib,wm6/Release
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_V1,libexpat.lib,lib
dnl MSWIN_FIND_FILE  expat,P_EXPAT_LIBRARY_V2,libexpatMT.lib,bin
dnl MSWIN_CHECK_DIR  expat,..\expat\
dnl MSWIN_CHECK_DIR  expat,..\external\expat\
dnl MSWIN_CHECK_DIR  expat,\expat
dnl MSWIN_CHECK_DIR  expat,c:\expat\
dnl MSWIN_CHECK_DIR  expat,\libs\expat\
dnl MSWIN_CHECK_DIR  expat,\Program Files\Expat 2.*


AC_ARG_ENABLE(expat,
       [  --disable-expat         disable XML support])

if test "${enable_expat}z" = "z" ; then
  enable_expat=$default_expat
fi

HAS_EXPAT=
if test "$enable_expat" = "no" ; then
  AC_MSG_NOTICE(Expat disabled)
else
  if test "${EXPATDIR}z" != "z" ; then
    AC_CHECK_HEADERS(${EXPATDIR}/lib/expat.h, HAS_EXPAT=1)
    if test "${HAS_EXPAT:-unset}" != "unset" ; then
      STDCCFLAGS="$STDCCFLAGS -I${EXPATDIR}/lib"
      LDFLAGS="$LDFLAGS -L${EXPATDIR}/.libs"
    else
      AC_CHECK_HEADERS(${EXPATDIR}/include/expat.h, HAS_EXPAT=1)
      if test "${HAS_EXPAT:-unset}" != "unset" ; then
        STDCCFLAGS="$STDCCFLAGS -I${EXPATDIR}/include"
        LDFLAGS="$LDFLAGS -L${EXPATDIR}/lib"
      fi
    fi
  else
    AC_CHECK_HEADERS(expat.h, HAS_EXPAT=1)
    if test "${HAS_EXPAT:-unset}" != "unset" ; then
      AC_CHECK_LIB(expat, XML_ParserCreate, HAS_EXPAT=1, HAS_EXPAT=)
    fi
  fi

  if test "${HAS_EXPAT:-unset}" != "unset" ; then
    AC_DEFINE(P_OEXPAT, 1, "expat found")
    ENDLDLIBS="$ENDLDLIBS -lexpat"
  fi
fi
AC_SUBST(HAS_EXPAT)

dnl ########################################################################
dnl look for IPV6 functions

dnl MSWIN_DISPLAY    ipv6,IPv6 Support
dnl MSWIN_CHECK_FILE ipv6,ws2tcpip.h|sin6_scope_id,P_HAS_IPV6=1
dnl MSWIN_DIR_SYMBOL ipv6,IPV6_DIR
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft Visual Studio .NET 2003\VC7\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft Visual Studio\VC98\Include\
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft SDK\Include\
dnl MSWIN_CHECK_DIR  ipv6,\Program Files\Microsoft Platform Sdk for Windows Server 2003 R2\Include

AC_ARG_ENABLE(ipv6,
       [  --disable-ipv6          disable IPV6 support])

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
    AC_TRY_COMPILE([#include <sys/socket.h>
                     #include <netdb.h>],
                     [getnameinfo(NULL, 0, NULL, 0, NULL, 0, 0);], HAS_IPV6=yes, HAS_IPV6=no)
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
dnl look for SDL, but only if we have pthreads

dnl MSWIN_DISPLAY    libsdl,Simple DirectMedia Layer
dnl MSWIN_CHECK_FILE libsdl,include\SDL_video.h,P_SDL=1
dnl MSWIN_DIR_SYMBOL libsdl,SDL_DIR
dnl MSWIN_FIND_FILE  libsdl,P_SDL_LIBRARY,SDL.lib,lib
dnl MSWIN_CHECK_DIR  libsdl,..\sdl*
dnl MSWIN_CHECK_DIR  libsdl,..\external\sdl*
dnl MSWIN_CHECK_DIR  libsdl,\sdl\
dnl MSWIN_CHECK_DIR  libsdl,c:\sdl\

AC_ARG_ENABLE([sdl],
	      AS_HELP_STRING([--disable-sdl],[disable SDL support]),
	      [enable_sdl=$enableval],
	      [enable_sdl=$default_sdl]
	     )

HAS_SDL=
if test "$enable_sdl" = "no" ; then
  AC_MSG_NOTICE(SDL disabled)
else

  PKG_CHECK_MODULES([SDL], 
                    [sdl], 
                    [
                     HAS_SDL=1
                     AC_DEFINE(P_SDL, 1, "SDL found")
                     STDCCFLAGS="$STDCCFLAGS ${SDL_CFLAGS}"
                     ENDLDLIBS="$ENDLDLIBS ${SDL_LIBS}"
dnl                      PKG_REQUIRES="$PKG_REQUIRES sdl"
                    ],[HAS_SDL=])

dnl to verify:
dnl     AC_CHECK_LIB(SDL, SDL_CreateYUVOverlay, HAS_SDL=1, HAS_SDL=, -L${X11LIBDIR} -lpthread -lX11 -lXext)
fi
AC_SUBST(HAS_SDL)


dnl ########################################################################
dnl check for enabling config file support
dnl MSWIN_DISPLAY    configfile,Config file protocol support
dnl MSWIN_DEFINE     configfile,P_CONFIG_FILE

PTLIB_SIMPLE_OPTION([configfile], [CONFIG_FILE], [enable config file support])

dnl ########################################################################
dnl check for URL support
dnl MSWIN_DISPLAY    url,URL Support
dnl MSWIN_DEFINE     url,P_URL

PTLIB_SIMPLE_OPTION([url], [URL], [enable URL support])

dnl ########################################################################
dnl check for HTTP support
dnl MSWIN_DISPLAY    http,HTTP Support
dnl MSWIN_DEFINE     http,P_HTTP
dnl MSWIN_IF_FEATURE http,url

PTLIB_SIMPLE_OPTION([http], [HTTP], [enable HTTP support], [HAS_URL])

dnl ########################################################################
dnl check for HTTPFORMS support
dnl MSWIN_DISPLAY    httpforms,HTTP forms Support
dnl MSWIN_DEFINE     httpforms,P_HTTPFORMS
dnl MSWIN_IF_FEATURE httpforms,http&configfile

PTLIB_SIMPLE_OPTION([httpforms], [HTTPFORMS], [enable HTTP forms support], [HAS_HTTP], [HAS_CONFIG_FILE])


dnl ########################################################################
dnl check for HTTPSVC support
dnl MSWIN_DISPLAY    httpsvc,HTTP service Support
dnl MSWIN_DEFINE     httpsvc,P_HTTPSVC
dnl MSWIN_IF_FEATURE httpsvc,httpforms

PTLIB_SIMPLE_OPTION([httpsvc], [HTTPSVC], [enable HTTP server support], [HAS_HTTPFORMS])

dnl ########################################################################
dnl Jabber
dnl MSWIN_DISPLAY    jabber,Jabber
dnl MSWIN_DEFINE     jabber,P_JABBER
dnl MSWIN_IF_FEATURE jabber,expat

PTLIB_SIMPLE_OPTION([jabber], [JABBER], [enable Jabber support], [HAS_EXPAT])

dnl ########################################################################
dnl VXML 
dnl MSWIN_DISPLAY    vxml,VXML Support
dnl MSWIN_DEFINE     vxml,P_VXML
dnl MSWIN_IF_FEATURE vxml,http&expat

PTLIB_SIMPLE_OPTION([vxml], [VXML], [enable VXML support], [HAS_EXPAT], [HAS_HTTP])

dnl ########################################################################
dnl XMLRPC
dnl MSWIN_DISPLAY    xmlrpc,XMLRPC Support
dnl MSWIN_DEFINE     xmlrpc,P_XMLRPC
dnl MSWIN_IF_FEATURE xmlrpc,http&expat

PTLIB_SIMPLE_OPTION([xmlrpc], [XMLRPC], [enable XMLRPC support], [HAS_EXPAT], [HAS_HTTP])

dnl ########################################################################
dnl SOAP
dnl MSWIN_DISPLAY    soap,SOAP Support
dnl MSWIN_DEFINE     soap,P_SOAP
dnl MSWIN_IF_FEATURE soap,http&expat

PTLIB_SIMPLE_OPTION([soap], [SOAP], [enable SOAP support], [HAS_EXPAT], [HAS_HTTP])

dnl ########################################################################
dnl Speech
dnl MSWIN_DISPLAY       sapi,Speech API
dnl MSWIN_CHECK_FILE    sapi,include\sphelper.h,P_SAPI=1
dnl MSWIN_DEFINE        sapi,P_TTS
dnl MSWIN_DIR_SYMBOL    sapi,SAPI_DIR
dnl MSWIN_CHECK_DIR     sapi,\Program Files\Microsoft Speech SDK 5.1\

PTLIB_SIMPLE_OPTION([tts], [TTS], [enable Text-To-Speech server support])

dnl ########################################################################
dnl check for enabling ASN support
dnl MSWIN_DISPLAY    asn,ASN Support
dnl MSWIN_DEFINE     asn,P_ASN

PTLIB_SIMPLE_OPTION([asn], [ASN], [enable ASN decoding/encoding support])

dnl ########################################################################
dnl check for enabling STUN support
dnl MSWIN_DISPLAY    stun,STUN Support
dnl MSWIN_DEFINE     stun,P_STUN

PTLIB_SIMPLE_OPTION([stun], [STUN], [enable STUN support])

dnl ########################################################################
dnl check for enabling PIPECHAN support
dnl MSWIN_DISPLAY    pipechan,pipe channel Support
dnl MSWIN_DEFINE     pipechan,P_PIPECHAN

PTLIB_SIMPLE_OPTION([pipechan], [PIPECHAN], [enable pipechannel support])

dnl ########################################################################
dnl check for enabling DTMF support
dnl MSWIN_DISPLAY  dtmf,DTMF encoding/decoding support
dnl MSWIN_DEFINE   dtmf,P_DTMF

PTLIB_SIMPLE_OPTION([dtmf], [DTMF], [enable DTMF protocol support])

dnl ########################################################################
dnl check for enabling WAVFILE support
dnl MSWIN_DISPLAY    wavfile,WAV file support
dnl MSWIN_DEFINE     wavfile,P_WAVFILE

PTLIB_SIMPLE_OPTION([wavfile], [WAVFILE], [enable WAVFILE support])

dnl ########################################################################
dnl check for enabling SOCKS support
dnl MSWIN_DISPLAY    socks,SOCKS protocol support
dnl MSWIN_DEFINE     socks,P_SOCKS

PTLIB_SIMPLE_OPTION([socks], [SOCKS], [enable SOCKS protocol support])

dnl ########################################################################
dnl check for enabling FTP support
dnl MSWIN_DISPLAY    ftp,FTP protocol support
dnl MSWIN_DEFINE     ftp,P_FTP

PTLIB_SIMPLE_OPTION([ftp], [FTP], [enable FTP protocol support])

dnl ########################################################################
dnl check for enabling SNMP support
dnl MSWIN_DISPLAY    snmp,SNMP protocol support
dnl MSWIN_DEFINE     snmp,P_SNMP

PTLIB_SIMPLE_OPTION([snmp], [SNMP], [enable SNMP protocol support])

dnl ########################################################################
dnl check for enabling TELNET support
dnl MSWIN_DISPLAY    telnet,Telnet protocol support
dnl MSWIN_DEFINE     telnet,P_TELNET

PTLIB_SIMPLE_OPTION([telnet], [TELNET], [enable TELNET protocol support])

dnl ########################################################################
dnl check for enabling REMCONN support
dnl MSWIN_DISPLAY    remconn,remote connection support
dnl MSWIN_DEFINE     remconn,P_REMCONN

PTLIB_SIMPLE_OPTION([remconn], [REMCONN], [enable remote connection support])

dnl ########################################################################
dnl check for enabling serial port handling
dnl MSWIN_DISPLAY    serial,Serial Support
dnl MSWIN_DEFINE     serial,P_SERIAL

PTLIB_SIMPLE_OPTION([serial], [SERIAL], [enable serial port support])

dnl ########################################################################
dnl check for POP3/SMTP support
dnl MSWIN_DISPLAY    pop3smtp,POP3/SMTP Support
dnl MSWIN_DEFINE     pop3smtp,P_POP3SMTP

PTLIB_SIMPLE_OPTION([pop3smtp], [POP3SMTP], [enable POP3/SMTP support])

dnl ########################################################################
dnl check for enabling socket aggregation
dnl MSWIN_DISPLAY    sockagg,Socket aggregation support
dnl MSWIN_DEFINE     sockagg,P_SOCKAGG

PTLIB_SIMPLE_OPTION([sockagg], [SOCKAGG], [enable socket aggregation])

dnl ########################################################################
dnl check for sound support
dnl MSWIN_DISPLAY audio,Sound support
dnl MSWIN_DEFINE  audio,P_AUDIO

PTLIB_SIMPLE_OPTION([audio], [AUDIO], [enable audio support])


dnl ########################################################################
dnl look for QoS functions

dnl MSWIN_DISPLAY    qos,QoS Support
dnl MSWIN_CHECK_FILE qos,qossp.h|QOS_DESTADDR,P_HAS_QOS=1
dnl MSWIN_DIR_SYMBOL qos,QOS_DIR
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft Visual Studio .NET 2003\VC7\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft Visual Studio\VC98\Include\
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft SDK\include\
dnl MSWIN_CHECK_DIR  qos,\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\

AC_ARG_ENABLE(qos,
       [  --disable-qos           disable QOS support])

if test "${enable_qos}z" = "z" ; then
  enable_qos=$default_qos
fi

if test "$enable_qos" != "no" ; then
  AC_DEFINE([P_HAS_QOS], [1], [Have QOS])
fi

dnl ########################################################################
dnl check for video support
dnl MSWIN_DISPLAY    video,Video Support
dnl MSWIN_DEFINE     video,P_VIDEO

PTLIB_SIMPLE_OPTION([video], [VIDEO], [enable video device support])
if test "x${HAS_VIDEO}" != "x1" ; then
  AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
fi

dnl ########################################################################
dnl check for enabling video file support
dnl MSWIN_DISPLAY    vidfile,Video file support
dnl MSWIN_DEFINE     vidfile,P_VIDFILE

PTLIB_SIMPLE_OPTION([vidfile], [VIDFILE], [enable video file support], [HAS_VIDEO])

dnl ########################################################################
dnl check for enabling app share support
dnl MSWIN_DISPLAY    appshare,Application share support
dnl MSWIN_DEFINE     appshare,P_APPSHARE

PTLIB_SIMPLE_OPTION([appshare], [APPSHARE], [enable application sharing support], [HAS_VIDEO])


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
  else
    dnl AC_CHECK_HEADER(machine/ioctl_meteor.h, HAS_VIDEO_CAPTURE=1)
    dnl AC_CHECK_HEADER(i386/ioctl_meteor.h, HAS_VIDEO_CAPTURE=1)
    dnl AC_CHECK_HEADER(dev/ic/bt8xx.h, HAS_VIDEO_CAPTURE=1)
    case "$target_os" in
      cygwin* | mingw* )
		PTLIB_FIND_DIRECTX([HAS_VIDEO_CAPTURE=1
			            ENDLDLIBS="$ENDLDLIBS ${DIRECTX_LIBS}"
dnl                                     PKG_LIBS="$PKG_LIBS ${DIRECTX_LIBS}"
				    STDCCFLAGS="$STDCCFLAGS ${DIRECTX_INCLUDES}"
dnl                                     PKG_CFLAGS="$PKG_CFLAGS ${DIRECTX_INCLUDES}"
				    AC_DEFINE(P_DIRECTSHOW, 1)
				    AC_DEFINE(P_DIRECTSOUND, 1)
		                   ])
		;;
      * )	
		AC_CHECK_HEADER([linux/videodev.h], [HAS_VIDEO_CAPTURE=1])
		;;
    esac
  fi

  if test "${HAS_VIDEO_CAPTURE}z" = "z" ; then
    AC_MSG_NOTICE(No Video Capture support)
    AC_DEFINE([NO_VIDEO_CAPTURE], [], [No video capture])
  fi

  dnl these are needed to pass through to plugin configure
  AC_ARG_ENABLE(v4l,      [  --enable-v4l            enable V4L video support])
  AC_ARG_ENABLE(v4l2,     [  --enable-v4l2           enable V4L2 video support])
  AC_ARG_ENABLE(bsdvideo, [  --enable-bsdvideo       enable BSD video support])
  AC_ARG_ENABLE(avc,      [  --enable-avc            enable IEEE1394/AVC video support])
  AC_ARG_ENABLE(dc,       [  --enable-dc             enable IEEE1394/DC video support])
fi

AC_SUBST(HAS_VIDEO_CAPTURE)

dnl ########################################################################
dnl enable shared memory video devices (static video plugin)


AC_ARG_ENABLE(shm_video,
		[  --enable-shmvideo       enable shared memory video devices],
		shm_video=$enableval)

if test "${enable_shmvideo}z" = "z" ; then
  shm_video=$default_shmvideo
fi

USE_SHM_VIDEO_DEVICES=
if test "$shm_video" = "yes" ; then
  USE_SHM_VIDEO_DEVICES=1
  AC_MSG_NOTICE(Shared memory video devices enabled)
else
  AC_MSG_NOTICE(Shared memory video devices disabled)
fi
AC_SUBST(USE_SHM_VIDEO_DEVICES)

dnl ########################################################################
dnl check for for audio drivers

HAS_ALSA=
HAS_OSS=
if test "$enable_audio" = "yes" ; then

  dnl these are needed to pass through to the plugin configure program
  AC_ARG_ENABLE(alsa, [  --enable-alsa           enable ALSA audio support],enable_alsa=$enableval)
  AC_ARG_ENABLE(oss,  [  --enable-oss            enable OSS audio support],enable_oss=$enableval)

  if test "$enable_plugins" = "no" ; then

    if test "${enable_alsa}z" = "z" ; then
      enable_alsa=$default_alsa
    fi

    if test "${enable_oss}z" = "z" ; then
      enable_oss=$default_oss
    fi

    if test "$enable_alsa" = "yes" ; then
      AC_CHECK_HEADER(alsa/asoundlib.h, HAS_ALSA=1)
      AC_MSG_CHECKING(for ALSA sound support)
      if test "${HAS_ALSA}z" != "z"; then
        AC_MSG_RESULT(yes)
        ENDLDLIBS="$ENDLDLIBS -lasound"
        HAS_ALSA=1
      else
        AC_MSG_RESULT(no)
      fi
    fi

    if test "${HAS_ALSA}z" = "z" ; then
      if test "$enable_oss" = "yes" ; then
        AC_CHECK_HEADER(sys/soundcard.h, SNDCARDHDR=1)
        AC_MSG_CHECKING(for OSS sound support)
        if test "${SNDCARDHDR}z" != "z"; then
          AC_SUBST(HAS_OSS, 1)
          AC_MSG_RESULT(yes)
        else
          AC_MSG_RESULT(no)
        fi
      fi
    fi
  fi
fi

AC_SUBST(HAS_ALSA)
AC_SUBST(HAS_OSS)


dnl ########################################################################
dnl look for Direct Sound

dnl MSWIN_DISPLAY    dsound,Direct Sound Support
dnl MSWIN_CHECK_FILE dsound,include\dsound.h,P_DIRECTSOUND=1
dnl MSWIN_CHECK_FILE dsound,include\armmacros.s,P_DIRECTSOUND_WINCE=1
dnl MSWIN_DIR_SYMBOL dsound,DIRECTX_DIR
dnl MSWIN_FIND_FILE  dsound,P_DIRECTSOUND_LIB64,dsound.lib,lib/x64
dnl MSWIN_FIND_FILE  dsound,P_DIRECTSOUND_LIB32,dsound.lib,lib/x86
dnl MSWIN_FIND_FILE  dsound,P_DIRECTSOUND_LIB32,dsound.lib,lib
dnl MSWIN_CHECK_DIR  dsound,\Program Files\Microsoft DirectX SDK\
dnl MSWIN_CHECK_DIR  dsound,\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\
dnl MSWIN_CHECK_DIR  dsound,\Program Files\Microsoft Visual Studio .NET 2003\VC7\PlatformSDK\
dnl MSWIN_CHECK_DIR  dsound,\Program Files\Microsoft SDK\
dnl MSWIN_CHECK_DIR  dsound,\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\


dnl ########################################################################
dnl look for Direct Show (video)

dnl MSWIN_DISPLAY    dshow,Direct Show Support
dnl MSWIN_CHECK_FILE dshow,include\dshow.h,P_DIRECTSHOW=1
dnl MSWIN_DIR_SYMBOL dshow,DIRECTSHOW_DIR
dnl MSWIN_CHECK_DIR  dshow,\Program Files\Microsoft SDKs\Windows\v6.*
dnl MSWIN_CHECK_DIR  dshow,\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\
dnl MSWIN_CHECK_DIR  dshow,\Program Files\Microsoft Visual Studio .NET 2003\VC7\PlatformSDK\
dnl MSWIN_CHECK_DIR  dshow,\Program Files\Microsoft SDK\
dnl MSWIN_CHECK_DIR  dshow,\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\


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
dnl look for ODBC code

dnl MSWIN_DISPLAY    odbc,ODBC support
dnl MSWIN_CHECK_FILE odbc,odbcinst.h,P_ODBC=1
dnl MSWIN_CHECK_DIR  odbc,\Program Files\Microsoft Visual Studio 8\VC\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  odbc,\Program Files\Microsoft Visual Studio .NET 2003\VC7\PlatformSDK\Include\
dnl MSWIN_CHECK_DIR  odbc,\Program Files\Microsoft SDK\include\
dnl MSWIN_CHECK_DIR  odbc,\Program Files\Microsoft Platform SDK for Windows Server 2003 R2\Include\

AC_ARG_ENABLE(odbc,
        [  --disable-odbc          disable ODBC support])
 
 if test "${enable_odbc}z" = "z" ; then
   enable_odbc=$default_odbc
 fi
 
 HAS_ODBC=
 if test "$enable_odbc" = "no" ; then
   AC_MSG_NOTICE(ODBC support disabled)
 else
   AC_CHECK_HEADERS(sql.h, HAS_ODBC=1)
   if test "${HAS_ODBC:-unset}" != "unset" ; then
     AC_CHECK_LIB(odbc, SQLAllocStmt, HAS_ODBC=1, HAS_ODBC=)
   else
     AC_CHECK_FILE(/usr/local/include/sql.h, HAS_ODBC=1)
     if test "${HAS_ODBC:-unset}" != "unset" ; then
       ODBCINCLUDEDIR=/usr/local/include
       ODBCLIBDIR=/usr/local/lib
     fi
   fi
 
   if test "${HAS_ODBC:-unset}" != "unset" ; then
     HAS_ODBC=1
     AC_DEFINE(P_ODBC, 1, "ODBC found")
     if test ${ODBCINCLUDEDIR:-unset} != "unset" ; then
       STDCCFLAGS="$STDCCFLAGS -I${ODBCINCLUDEDIR}"
     fi
     if test ${ODBCLIBDIR:-unset} != "unset" ; then
       LDFLAGS="$LDFLAGS -L${ODBCLIBDIR}"
     fi
     ENDLDLIBS="$ENDLDLIBS -lodbc"
     AC_MSG_NOTICE(ODBC support enabled)
   fi
 fi
 AC_SUBST(HAS_ODBC)
 


dnl ########################################################################
dnl output make directives

dnl resolver needs to be at the end
if test "${HAS_RESOLVER:-unset}" != "unset" ; then
    ENDLDLIBS="$ENDLDLIBS ${RESOLVER_LIBS}"
dnl     PKG_LIBS="$PKG_LIBS ${RESOLVER_LIBS}"
fi

dnl dl needs to be at the end
if test "x${HAS_LIBDL}" = "x1" ; then
  ENDLDLIBS="$ENDLDLIBS $DL_LIBS"
dnl   PKG_LIBS="$PKG_LIBS $DL_LIBS"
  STDCCFLAGS="$STDCCFLAGS $DL_CFLAGS"
dnl   PKG_CFLAGS="$PKG_CFLAGS $DL_CFLAGS"
fi

dnl ########################################################################
dnl this reduces the size of all executables and libraries

AC_ARG_ENABLE(exceptions,
       [  --enable-exceptions            enable C++ exceptions])

if test "${enable_exceptions}z" != "yesz" ; then
  AC_MSG_NOTICE(C++ exceptions disabled)
  STDCCFLAGS="$STDCCFLAGS -fno-exceptions"
else
  AC_DEFINE(P_EXCEPTIONS, 1)
  AC_MSG_NOTICE(C++ exceptions enabled)
  STDCCFLAGS="$STDCCFLAGS -fexceptions"
fi

dnl ########################################################################
dnl final declarations

AC_SUBST(CC)
AC_SUBST(CXX)
AC_SUBST(STDCCFLAGS)
AC_SUBST(STDCXXFLAGS)
AC_SUBST(OPTSTDCCFLAGS)
AC_SUBST(LDFLAGS)
AC_SUBST(ENDLDLIBS)
AC_SUBST(DEBUG_FLAG)
AC_PROG_INSTALL

AC_CONFIG_FILES(make/ptbuildopts.mak)
AC_CONFIG_FILES([make/ptlib-config], [chmod a+x make/ptlib-config])
AC_CONFIG_FILES(Makefile)
AC_CONFIG_FILES(ptlib_cfg.dxy)
AC_CONFIG_FILES(ptlib.pc)

dnl ########################################################################
dnl output header file
AC_CONFIG_HEADERS(include/ptbuildopts.h)

AC_OUTPUT()
