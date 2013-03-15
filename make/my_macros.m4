dnl
dnl my_macros.m4
dnl
dnl A bunch of useful macros for doing complex configure.ac
dnl scripts, especially when cross compiling.
dnl

AC_CANONICAL_TARGET()

AC_PROG_CC()
AC_PROG_CXX()
if test -z "$CXX" ; then
   AC_MSG_ERROR(C++ compiler is required, 1)
fi

AC_PROG_RANLIB()

AC_CHECK_TOOL(AR, ar)
if test -z "$AR" ; then
   AC_CHECK_TOOL(AR, gar)
fi


dnl
dnl  Modified AC_CANONICAL_TARGET
dnl
dnl  This generates "normalised" target_os, target_cpu, target_64bit and target
dnl  variables. The standard ones are a little too detailed for our use. Also,
dnl  we check for --enable-ios=X to preset above variables and compiler for
dnl  correct cross compilation, easier to remember that the --host command.
dnl  Finally, it defines the various compulsory progams (C, C++, ld, ar, etc)
dnl  and flags that are used by pretty much any build.
dnl
AC_DEFUN([MY_CANONICAL_TARGET], [
   dnl Special case for iOS to make cross compiling simpler to remember
   AC_ARG_ENABLE([ios], [AS_HELP_STRING([--enable-ios=iphone|simulator],[enable iOS support])])

   if test "$enable_ios" = "iphone" ; then
      target_vendor=apple
      target_os=iPhoneOS
      target_cpu=armv7
      target_release=`xcodebuild -showsdks | sed -n 's/.*iphoneos\(.*\)/\1/p' | sort | tail -n 1`
   elif test "$enable_ios" = "simulator" ; then
      target_vendor=apple
      target_os=iPhoneSimulator
      target_cpu=i686
      target_release=`xcodebuild -showsdks | sed -n 's/.*iphonesimulator\(.*\)/\1/p' | sort | tail -n 1`
   elif test "x$enable_ios" != "x" ; then
      AC_MSG_ERROR([Unknown iOS variant \"${enable_ios}\" - use either iphone or simulator])
   fi


   dnl Most unix'ish platforms are like this

   AC_SUBST(SHARED_CPPFLAGS, "-fPIC")
   AC_SUBST(SHARED_LDFLAGS, [["-shared -Wl,-soname,INSERT_SONAME"]])
   AC_SUBST(SHAREDLIBEXT, "so")
   AC_SUBST(STATICLIBEXT, "a")

   AC_SUBST(ARFLAGS, "rc")

   case "$target_os" in
      darwin* | iPhone* )
         SHARED_LDFLAGS="-dynamiclib"
         SHAREDLIBEXT="dylib"
         AR="libtool"
         ARFLAGS="-static -o"
         RANLIB=
      ;;

      cygwin* | mingw* )
         SHARED_LDFLAGS="-shared -Wl,--kill-at"
         SHAREDLIBEXT="dll"
         STATICLIBEXT="lib"
      ;;
   esac

   case "$target_os" in
      iPhone* )
         if test "x$target_release" == "x" ; then
            AC_MSG_ERROR([Unable to determine iOS release number])
         fi

         IOS_DEVROOT="`xcode-select -print-path`/Platforms/${target_os}.platform/Developer"
         IOS_SDKROOT=${IOS_DEVROOT}/SDKs/${target_os}${target_release}.sdk

         CXX="${IOS_DEVROOT}/usr/bin/g++"
         CC="${IOS_DEVROOT}/usr/bin/gcc"
         LD="${IOS_DEVROOT}/usr/bin/ld"
         CPPFLAGS="${CPPFLAGS} -arch $target_cpu -isysroot ${IOS_SDKROOT}"
         LDFLAGS="${LDFLAGS} -arch $target_cpu -isysroot ${IOS_SDKROOT} -L${IOS_SDKROOT}/usr/lib -framework SystemConfiguration -framework CoreFoundation"
      ;;

      darwin* )
         target_os=Darwin
         OS_MAJOR=`uname -r | sed 's/\..*$//'`
         OS_MINOR=[`uname -r | sed -e 's/[0-9][0-9]*\.//' -e 's/\..*$//'`]
         target_release=`expr $OS_MAJOR \* 100 + $OS_MINOR`
         CPPFLAGS="${CPPFLAGS} -D__MACOSX_CORE__"
         LDFLAGS="${LDFLAGS} -framework CoreAudio -framework SystemConfiguration -framework CoreFoundation"
      ;;

      linux* | Linux* | uclibc* )
         target_os=linux
      ;;

      freebsd* | kfreebsd* )
         target_os=FreeBSD
         target_release="`sysctl -n kern.osreldate`"
         AC_CHECK_TOOL(RANLIB, ranlib)
      ;;

      openbsd* )
         target_os=OpenBSD
         target_release="`sysctl -n kern.osrevision`"
      ;;

      netbsd* )
         target_os=NetBSD
         target_release="`/sbin/sysctl -n kern.osrevision`"
      ;;

      solaris* | sunos* )
         target_os=solaris
         target_release=`uname -r | sed "s/5\.//g"`
         CPPFLAGS="$CPPFLAGS -D__inline=inline -DSOLARIS"
         SHARED_LDFLAGS="-Bdynamic -G -h INSERT_SONAME"
      ;;

      beos* )
         target_os=beos
         CPPFLAGS="$CPPFLAGS D__BEOS__ -DBE_THREADS -Wno-multichar -Wno-format"
         LDFLAGS="-lstdc++.r4 -lbe -lmedia -lgame -lroot -lsocket -lbind -ldl"
         SHARED_LDFLAGS="-shared -nostdlib -nostart"
      ;;

      cygwin* )
         target_os=cygwin
      ;;

      mingw* )
         target_os=mingw
         CPPFLAGS="$CPPFLAGS -mms-bitfields"
         LDFLAGS="-lwinmm -lwsock32 -lws2_32 -lsnmpapi -lmpr -lcomdlg32 -lgdi32 -lavicap32 -liphlpapi -lole32 -lquartz"
      ;;

      * )
         AC_MSG_WARN([Operating system \"$target_os\" not recognized - proceed with caution!])
      ;;
   esac

   if test "x$target_release" = "x"; then
      target_release="`uname -r`"
   fi

   case "$target_cpu" in
      x86 | i686 | i586 | i486 | i386 )
         AC_MSG_CHECKING([64 bit system masquerading as 32 bit])
         AC_COMPILE_IFELSE(
            [AC_LANG_SOURCE([int t = __amd64__;])],
            [
               AC_MSG_RESULT(yes)
               target_cpu=x86_64
               target_64bit=1
            ],
            [
               AC_MSG_RESULT(no)
               target_cpu=x86
               target_64bit=0
            ]
         )
      ;;

      x86_64 | amd64 )
         target_cpu=x86_64
         target_64bit=1
      ;;

      alpha* )
         target_cpu=alpha
         target_64bit=1
      ;;

      sparc* )
         target_cpu=sparc
         target_64bit=1
      ;;

      ppc | powerpc )
         target_cpu=ppc
         target_64bit=0
      ;;

      ppc64 | powerpc64 )
         target_cpu=ppc64
         target_64bit=1
      ;;

      hppa64 | ia64 | s390x )
         target_64bit=1
      ;;

      arm* )
         target_64bit=0
      ;;

      * )
         AC_MSG_WARN([CPU \"$target_cpu\" not recognized - assuming 32 bit])
         target_64bit=0
      ;;
   esac

   AC_ARG_ENABLE(force32, AS_HELP_STRING([--enable-force32],[Force 32-bit x86 build]))
   if test "${enable_force32}" = "yes" ; then
      if test "$target_cpu" != "x86_64" ; then
         AC_MSG_ERROR([force32 option only available for 64 bit x86])
      fi

      target_cpu=x86
      target_64bit=0

      if test "$target_os" = "Darwin"; then
         CPPFLAGS="$CPPFLAGS -arch i386"
         LDFLAGS="$LDFLAGS -arch i386"
         SHARED_LDFLAGS="$SHARED_LDFLAGS -arch i386"
      else
         CPPFLAGS="$CPPFLAGS -m32"
         LDFLAGS="$LDFLAGS -m32"
         SHARED_LDFLAGS="$SHARED_LDFLAGS -m32"
      fi

      AC_MSG_NOTICE(Forcing 32 bit x86 compile)
   fi


   target=${target_os}_${target_cpu}

   AC_MSG_NOTICE([using \"$target_os\" release \"$target_release\" on \"$target_cpu\"])
])


dnl internal macro help
AC_DEFUN([MY_IFELSE],[
   m4_ifnblank([$2$3], [AS_IF([test "x$m4_normalize($1)" = "xyes"], [$2], [$3])])
])


dnl internal macro help
AC_DEFUN([MY_ARG_DEPENDENCY],[
   m4_ifnblank([$1],[
      if    test "x${enableval}" = "xyes" && \
            test "x$m4_normalize($1)" != "x1" && \
            test "x$m4_normalize($1)" != "xyes"; then
         AC_MSG_RESULT([disabled due to disabled dependency $1=$$1])
         enableval=no
      fi
   ])
])

dnl MY_ARG_ENABLE
dnl As AC_ARG_ENABLE, but with disable, defaults and dependecies
dnl $1 command line option name
dnl $2 command line help
dnl $3 default state
dnl $4 enabled script
dnl $5 disabled script
dnl $6..$9 dependency variable(s)
dnl The variable enable_$1 is defined to yes/no
AC_DEFUN([MY_ARG_ENABLE],[
   AC_MSG_CHECKING([$2])

   AC_ARG_ENABLE(
      [$1],
      [AC_HELP_STRING([--m4_bmatch([$2],[enable.*],[enable],[disable])-$1],
                         m4_bmatch([$2],[enable.*],[],[disable ])[$2])],
      [
         if test "x$enableval" = "xno"; then
            AC_MSG_RESULT([disabled by user])
         fi
      ],
      [
         enableval=m4_default([$3],m4_bmatch([$2],[enable.*],[no],[yes]))
         if test "x$enableval" = "xno"; then
            AC_MSG_RESULT([disabled by default])
         fi
      ]
   )

   MY_ARG_DEPENDENCY([$6])
   MY_ARG_DEPENDENCY([$7])
   MY_ARG_DEPENDENCY([$8])
   MY_ARG_DEPENDENCY([$9])

   if test "x${enableval}" = "xyes"; then
      AC_MSG_RESULT([yes])
   fi

   MY_IFELSE([enableval], [$4], [$5])

   enable_$1="$enableval"
])


dnl MY_COMPILE_IFELSE
dnl As AC_COMPILE_IFELSE but saves and restores CPPFLAGS if fails
dnl $1 checking message
dnl $2 CPPFLAGS
dnl $3 program headers
dnl $4 program main
dnl $5 success code
dnl $6 failure code
AC_DEFUN([MY_COMPILE_IFELSE],[
   oldCPPFLAGS="$CPPFLAGS"
   CPPFLAGS="$CPPFLAGS $2"
   AC_MSG_CHECKING([$1])
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([[$3]],[[$4]])],
      [usable=yes],
      [usable=no]
   )
   AC_MSG_RESULT($usable)
   CPPFLAGS="$oldCPPFLAGS"
   MY_IFELSE([usable], [$5], [$6])
])


dnl MY_LINK_IFELSE
dnl As AC_LINK_IFELSE but saves and restores CPPFLAGS & LDFLAGS if fails
dnl $1 checking message
dnl $2 CPPFLAGS
dnl $3 LIBS/LDFLAGS
dnl $4 program headers
dnl $5 program main
dnl $6 success code
dnl $7 failure code
AC_DEFUN([MY_LINK_IFELSE],[
   oldCPPFLAGS="$CPPFLAGS"
   oldLDFLAGS="$LDFLAGS"
   CPPFLAGS="$CPPFLAGS $2"
   LDFLAGS="$3 $LDFLAGS"
   AC_MSG_CHECKING($1)
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[$4]],[[$5]])],
      [usable=yes],
      [usable=no]
   )
   AC_MSG_RESULT($usable)
   CPPFLAGS="$oldCPPFLAGS"
   LDFLAGS="$oldLDFLAGS"
   MY_IFELSE([usable], [$6], [$7])
])


dnl MY_PKG_CHECK_MODULE
dnl As PKG_CHECK_MODULES but does test compile so works wit cross compilers
dnl $1 module name
dnl $2 pkg name
dnl $3 program headers
dnl $4 program main
dnl $5 success code
dnl $6 failure code
AC_DEFUN([MY_PKG_CHECK_MODULE],[
   PKG_CHECK_MODULES(
      [$1],
      [$2],
      [MY_LINK_IFELSE(
         [for $1 usability],
         [$$1[_CFLAGS]],
         [$$1[_LIBS]],
         [$3],
         [$4],
         [
            CPPFLAGS="$CPPFLAGS $$1[_CFLAGS]"
            LDFLAGS="$$1[_LIBS] $LDFLAGS"
         ]
      )],
      [usable=no]
   )
   MY_IFELSE([usable], [$5], [$6])
])


dnl MY_MODULE_OPTION
dnl Check for modules existence, with --disable-XXX and optional --with-XXX-dir
dnl $1 module name
dnl $2 command line option name
dnl $3 command line option help text
dnl $4 pkg name(s)
dnl $5 default CPPFLAGS
dnl $6 default LIBS
dnl $7 program headers
dnl $8 program main
dnl $9 success code
dnl $10 failure code
dnl $11 optional dependency
AC_DEFUN([MY_MODULE_OPTION],[
   MY_ARG_ENABLE([$2], [$3 enabled], [${DEFAULT_$1:-yes}], [usable=yes], [usable=no], [$11], [$12], [$13], [$14])

   if test "x$usable" = "xyes" ; then
      m4_ifnblank([$5$6],
         [AC_ARG_WITH(
            [$2-dir],
            AS_HELP_STRING([--with-$2-dir=<dir>],[location for $3]),
            [
               AC_MSG_NOTICE(Using directory $withval for $3)
               $1[_CFLAGS]="-I$withval/include $5"
               $1[_LIBS]="-L$withval/lib $6"
            ],
            [PKG_CHECK_MODULES(
               [$1],
               [$4],
               [],
               [
                  $1[_CFLAGS]="$5"
                  $1[_LIBS]="$6"
               ]
            )]
         )],
         [PKG_CHECK_MODULES(
            [$1],
            [$4],
            [],
            [usable=no]
         )]
      )

      if test "x$usable" = "xyes" ; then
         MY_LINK_IFELSE(
            [for $3 usability],
            [$$1[_CFLAGS]],
            [$$1[_LIBS]],
            [$7],
            [$8],
            [
               CPPFLAGS="$CPPFLAGS $$1[_CFLAGS]"
               LDFLAGS="$$1[_LIBS] $LDFLAGS"
            ],
            [usable=no]
         )
      fi
   fi

   MY_IFELSE([usable], [$9], [$10])

   AC_SUBST($1[_USABLE], $usable)
])

