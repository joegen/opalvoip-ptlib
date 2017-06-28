dnl
dnl ptlib.m4
dnl

dnl Assumes my_macros.m4 included


dnl PTLIB_SUBST
dnl As AC_SUBST but defines a HAS_XXX and PTLIB_XXX substitutions
dnl $1 option name
dnl $2 optional value
AC_DEFUN([PTLIB_SUBST],[
   m4_ifnblank([$2], [HAS_$1=$2])

   if test "x$HAS_$1" = "xyes" ; then
      HAS_$1=1
   fi

   if test "x$HAS_$1" = "x0" || test "x$HAS_$1" = "xno" ; then
      HAS_$1=
   fi

   AC_SUBST([HAS_$1])

   if test "x$HAS_$1" = "x1" ; then
      PTLIB_$1=yes
      AC_DEFINE(P_$1, 1)
   else
      PTLIB_$1=no
   fi

   AC_SUBST([PTLIB_$1])
])


dnl PTLIB_SIMPLE_OPTION
dnl Change a given variable according to arguments and subst and define it
dnl $1 name of configure option
dnl $2 the variable to change, subst and define
dnl $3 the configure argument description
dnl $4..$6 dependency variable(s)
AC_DEFUN([PTLIB_SIMPLE_OPTION],[
   MY_ARG_ENABLE(
      [$1],
      [$3],
      [${DEFAULT_$2:-yes}],
      [PTLIB_SUBST($2, 1)],
      [HAS_$2=],
      [$4],
      [$5],
      [$6]
   )
])


dnl PTLIB_MODULE_OPTION
dnl Check for modules existence, with --disable-XXX and optional --with-XXX-dir
dnl $1 module name
dnl $2 option name
dnl $3 option help text
dnl $4 pkg name(s)
dnl $5 default CPPFLAGS
dnl $6 default LIBS
dnl $7 program headers
dnl $8 program main
dnl $9 success code
dnl $10 failure code
dnl $11 optional dependency
AC_DEFUN([PTLIB_MODULE_OPTION],[
   MY_MODULE_OPTION([$1],[$2],[$3],[$4],[$5],[$6],[$7],[$8],[$9],[$10],[$11])
   PTLIB_SUBST([$1], [$$1[_USABLE]])
])



dnl ##################################################################

AC_SUBST(SHARED_CPPFLAGS, "-fPIC")
AC_SUBST(SHARED_LDFLAGS, [['-shared -Wl,--build-id,-soname,$(LIB_SONAME)']])

AS_CASE([$target_os],
   Darwin | iPhone*, [
      SHARED_LDFLAGS='-dynamiclib -Wl,-install_name,@executable_path/$(LIB_SONAME)'
      AR="libtool"
      ARFLAGS="-static -o"
      RANLIB=
      CPPFLAGS="-stdlib=libc++ $CPPFLAGS"
      LDFLAGS="${LDFLAGS} -stdlib=libc++"
      LIBS="-framework AudioToolbox -framework CoreAudio -framework SystemConfiguration -framework Foundation -lobjc $LIBS"
   ]
)

AS_CASE([$target_os],
   iPhone*, [
      IOS_DEVROOT="`xcode-select -print-path`/Platforms/${target_os}.platform/Developer"
      IOS_SDKROOT=${IOS_DEVROOT}/SDKs/${target_os}${target_release}.sdk
      IOS_FLAGS="-arch $target_cpu -miphoneos-version-min=6.0 -isysroot ${IOS_SDKROOT}"
      CPPFLAGS="${IOS_FLAGS} $CPPFLAGS"
      LDFLAGS="${IOS_FLAGS} -L${IOS_SDKROOT}/usr/lib $LDFLAGS"
   ],

   Darwin, [
      CPPFLAGS="-mmacosx-version-min=$target_release $CPPFLAGS"
      LDFLAGS="-mmacosx-version-min=$target_release $LDFLAGS"
      LIBS="-framework AVFoundation -framework CoreVideo -framework CoreMedia -framework AudioUnit $LIBS"
   ],

   solaris, [
      CPPFLAGS="-D__inline=inline -DSOLARIS $CPPFLAGS"
      SHARED_LDFLAGS='-Bdynamic -G -h $(LIB_SONAME)'
   ],

   beos, [
      CPPFLAGS="-D__BEOS__ -DBE_THREADS -Wno-multichar -Wno-format $CPPFLAGS"
      LIBS="-lstdc++.r4 -lbe -lmedia -lgame -lroot -lsocket -lbind -ldl $LIBS"
      SHARED_LDFLAGS="-shared -nostdlib -nostart"
   ],

   cygwin | mingw, [
      SHARED_CPPFLAGS=""
      SHARED_LDFLAGS="-shared -Wl,--kill-at"
      SHAREDLIBEXT="dll"
      STATICLIBEXT="lib"
      LIBS="-lwinmm -lwsock32 -lws2_32 -lsnmpapi -lmpr -lcomdlg32 -lgdi32 -lavicap32 -liphlpapi -lole32 -lquartz $LIBS"
   ]
)

AS_VAR_IF([enable_force32], [yes], [
   AS_VAR_IF([target_os], [Darwin], [
      CPPFLAGS="-arch i386 $CPPFLAGS"
      LDFLAGS="$LDFLAGS -arch i386"
   ],[
      CPPFLAGS="-m32 $CPPFLAGS"
      LDFLAGS="$LDFLAGS -m32"
   ])
])


dnl Check for latest and greatest
AC_ARG_ENABLE(cpp11, AS_HELP_STRING([--enable-cpp11],[Enable C++11 build]),AC_SUBST(CPLUSPLUS_STD,"-std=c++11"))
AC_ARG_ENABLE(cpp14, AS_HELP_STRING([--enable-cpp14],[Enable C++14 build]),AC_SUBST(CPLUSPLUS_STD,"-std=c++14"))


dnl add additional information for the debugger to ensure the user can indeed
dnl debug coredumps and macros.

AC_SUBST(DEBUG_CPPFLAGS, "-D_DEBUG $DEBUG_CPPFLAGS")
AS_VAR_SET_IF([DEBUG_FLAG], ,
   MY_COMPILE_IFELSE(
      [debug build (-gdwarf-4)],
      [-gdwarf-4],
      [],
      [],
      [DEBUG_FLAG="-gdwarf-4"],
      MY_COMPILE_IFELSE(
         [debug build (-g3)],
         [-g3],
         [],
         [],
         [DEBUG_FLAG="-g3"],
         [DEBUG_FLAG="-g"]
      )
   )
)
AC_SUBST(DEBUG_CFLAGS, "$DEBUG_FLAG $DEBUG_CFLAGS")

AC_SUBST(OPT_CPPFLAGS, "-DNDEBUG $OPT_CPPFLAGS")
MY_COMPILE_IFELSE(
   [optimised build (-O3)],
   [-O3],
   [],
   [],
   [OPT_CFLAGS="-O3 $OPT_CFLAGS"],
   [OPT_CFLAGS="-O $OPT_CFLAGS"]
)
AC_SUBST(OPT_CFLAGS)

MY_COMPILE_IFELSE(
   [Don't omit frame pointers for stack walking (-fno-omit-frame-pointer)],
   [-fno-omit-frame-pointer],
   [],
   [],
   [
      CFLAGS="-fno-omit-frame-pointer $CFLAGS"
      CXXFLAGS="-fno-omit-frame-pointer $CXXFLAGS"
   ]
)


dnl Warn about everything, well, nearly everything

MY_COMPILE_IFELSE(
   [Disable unknown-pragmas warning (-Wno-unknown-pragmas)],
   [-Werror -Wno-unknown-pragmas],
   [],
   [],
   [CPPFLAGS="-Wno-unknown-pragmas $CPPFLAGS"]
)

AC_LANG_PUSH(C++)

MY_COMPILE_IFELSE(
   [Disable unused-private-field warning (-Wno-unused-private-field)],
   [-Werror -Wno-unused-private-field],
   [],
   [],
   [CXXFLAGS="-Wno-unused-private-field $CXXFLAGS"]
)

MY_COMPILE_IFELSE(
   [Disable overloaded-virtual warning (-Wno-overloaded-virtual)],
   [-Werror -Wno-overloaded-virtual],
   [],
   [],
   [CXXFLAGS="-Wno-overloaded-virtual $CXXFLAGS"]
)

MY_COMPILE_IFELSE(
   [Disable deprecated-declarations warning (-Wno-deprecated-declarations)],
   [-Werror -Wno-deprecated-declarations],
   [],
   [],
   [CXXFLAGS="$CXXFLAGS -Wno-deprecated-declarations"]
)

MY_COMPILE_IFELSE(
   [Disable potentially evaluated expression warning (-Wno-potentially-evaluated-expression)],
   [-Werror -Wno-potentially-evaluated-expression],
   [],
   [],
   [CXXFLAGS="$CXXFLAGS -Wno-potentially-evaluated-expression"]
)

AC_LANG_POP(C++)

MY_COMPILE_IFELSE(
   [warnings (-Wall)],
   [-Wall],
   [],
   [],
   [CPPFLAGS="-Wall $CPPFLAGS"]
)


dnl Check for profiling

AC_ARG_WITH(
   [profiling],
   AC_HELP_STRING([--with-profiling], [Enable profiling: gprof, eccam, raw or manual]),
   [
      AS_CASE([$with_profiling],
         [gprof], [
            MY_COMPILE_IFELSE(
               [has compiler supported profiling (-pg)],
               [-pg],
               [],
               [],
               [MY_ADD_FLAGS([],[],[-pg],[-pg])],
               [AC_MSG_ERROR([Compiler does not support gprof profiling])]
            )
         ],
         [eccam|raw], [
            MY_COMPILE_IFELSE(
               [has compiler supported instrumentation (-finstrument-functions)],
               [-finstrument-functions],
               [],
               [],
               [
                  AS_VAR_IF([with_profiling], [raw], [
                     AC_DEFINE(P_PROFILING, 1),
                  ],[
                     MY_COMPILE_IFELSE(
                        [has Eccam embedded profile libraries (-lEProfiler)],
                        [],
                        [-lEProfiler],
                        [],
                        [
                           MY_ADD_FLAGS([-lEProfiler],[],[],[])
                        ],
                        [AC_MSG_ERROR([Eccam profiling not installed])]
                     )
                  ])
                  MY_ADD_FLAGS([],[],[-finstrument-functions],[-finstrument-functions])
               ],
               [AC_MSG_ERROR([Compiler does not support gprof profiling])]
            )
         ],
         [manual], [
            AC_DEFINE(P_PROFILING, 2)
         ],
         AC_MSG_ERROR([Unknown profile method \"$with_profiling\"])
      )

      AC_SUBST(P_PROFILING, $with_profiling)
   ]
)


dnl End of file

