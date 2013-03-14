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

   AC_SUBST(LDSOFLAGS, [["-shared -Wl,-soname,INSERT_SONAME"]])
   AC_SUBST(SHAREDLIBEXT, "so")
   AC_SUBST(STATICLIBEXT, "a")

   AC_SUBST(ARFLAGS, "rc")

   case "$target_os" in
      darwin* | iPhone* )
         LDSOFLAGS="-dynamiclib"
         SHAREDLIBEXT="dylib"
         AR="libtool"
         ARFLAGS="-static -o"
         RANLIB=
      ;;

      cygwin* | mingw* )
         LDSOFLAGS="-shared -Wl,--kill-at"
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
         LDSOFLAGS="-Bdynamic -G -h INSERT_SONAME"
      ;;

      beos* )
         target_os=beos
         CPPFLAGS="$CPPFLAGS D__BEOS__ -DBE_THREADS -DP_USE_PRAGMA -Wno-multichar -Wno-format"
         LDFLAGS="-lstdc++.r4 -lbe -lmedia -lgame -lroot -lsocket -lbind -ldl"
         LDSOFLAGS="-shared -nostdlib -nostart"
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
         LDSOFLAGS="$LDSOFLAGS -arch i386"
      else
         CPPFLAGS="$CPPFLAGS -m32"
         LDFLAGS="$LDFLAGS -m32"
         LDSOFLAGS="$LDSOFLAGS -m32"
      fi

      AC_MSG_NOTICE(Forcing 32 bit x86 compile)
   fi


   target=${target_os}_${target_cpu}

   AC_MSG_NOTICE([using \"$target_os\" release \"$target_release\" on \"$target_cpu\"])
])

