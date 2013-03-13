dnl PTLIB_SUBST
dnl As AC_SUBST but defines a HAS_XXX and PTLIB_XXX substitutions
dnl $1 option name
dnl $2 optional value
AC_DEFUN([PTLIB_SUBST],[
   m4_ifnblank([$2], [HAS_$1=$2])

   if test "x${HAS_$1}" = "xyes" ; then
      HAS_$1=1
   fi

   if test "x${HAS_$1}" = "x0" || test "x${HAS_$1}" = "xno" ; then
      HAS_$1=
   fi

   AC_SUBST(HAS_$1)

   if test "x${HAS_$1}" = "x1" ; then
      AC_SUBST([PTLIB_$1], [yes])
      AC_DEFINE(P_$1, 1)
   else
      AC_SUBST([PTLIB_$1], [no])
   fi
])


dnl internal macro help
AC_DEFUN([PTLIB_SIMPLE_OPTION_DEPENDENCY],[
   m4_ifnblank([$1],[
      if test "x${HAS_$2}" = "x1" && test "x$$1" != "x1"; then
         AC_MSG_RESULT([disabled due to disabled dependency $1])
         HAS_$2=
      fi
   ])
])

dnl PTLIB_SIMPLE_OPTION
dnl Change a given variable according to arguments and subst and define it
dnl $1 name of configure option
dnl $2 the variable to change, subst and define
dnl $3 the configure argument description
dnl $4 dependency variable #1
dnl $5 dependency variable #2 
dnl ${HAS_$2} The (possibly) changed variable
AC_DEFUN([PTLIB_SIMPLE_OPTION],[
   AC_MSG_CHECKING([$3])

   AC_ARG_ENABLE(
      [$1],
      [AC_HELP_STRING([--enable-$1],[$3])],
      [
         if test "x$enableval" = "xyes"; then
            HAS_$2=1
         else
            HAS_$2=
            AC_MSG_RESULT([disabled by user])
         fi
      ]
   )

   PTLIB_SIMPLE_OPTION_DEPENDENCY([$4],[$2])
   PTLIB_SIMPLE_OPTION_DEPENDENCY([$5],[$2])
   PTLIB_SIMPLE_OPTION_DEPENDENCY([$6],[$2])
   PTLIB_SIMPLE_OPTION_DEPENDENCY([$7],[$2])

   if test "x${HAS_$2}" = "x1"; then
      AC_DEFINE([P_$2], [1], [$3])
      AC_MSG_RESULT([yes])
   fi

   PTLIB_SUBST($2)
])


dnl PTLIB_COMPILE_IFELSE
dnl As AC_COMPILE_IFELSE but saves and restores CFLAGS if fails
dnl $1 checking message
dnl $2 CFLAGS
dnl $3 program headers
dnl $4 program main
dnl $5 success code
dnl $6 failure code
AC_DEFUN([PTLIB_COMPILE_IFELSE],[
   oldCFLAGS="$CFLAGS"
   oldCXXFLAGS="$CXXFLAGS"
   CFLAGS="$CFLAGS $2"
   CXXFLAGS="$CXXFLAGS $2"
   AC_MSG_CHECKING([$1])
   AC_COMPILE_IFELSE(
      [AC_LANG_PROGRAM([[$3]],[[$4]])],
      [usable=yes],
      [usable=no]
   )
   AC_MSG_RESULT($usable)
   CFLAGS="$oldCFLAGS"
   CXXFLAGS="$oldCXXFLAGS"
   AS_IF([test AS_VAR_GET([usable]) = yes], [$5], [$6])[]
])


dnl PTLIB_LINK_IFELSE
dnl As AC_LINK_IFELSE but saves and restores CFLAGS & LDFLAGS if fails
dnl $1 checking message
dnl $2 CFLAGS
dnl $3 LIBS/LDFLAGS
dnl $4 program headers
dnl $5 program main
dnl $6 success code
dnl $7 failure code
AC_DEFUN([PTLIB_LINK_IFELSE],[
   oldCFLAGS="$CFLAGS"
   oldCXXFLAGS="$CXXFLAGS"
   oldLDFLAGS="$LDFLAGS"
   CFLAGS="$CFLAGS $2"
   CXXFLAGS="$CXXFLAGS $2"
   LDFLAGS="$LDFLAGS $3"
   AC_MSG_CHECKING($1)
   AC_LINK_IFELSE(
      [AC_LANG_PROGRAM([[$4]],[[$5]])],
      [usable=yes],
      [usable=no]
   )
   AC_MSG_RESULT($usable)
   CFLAGS="$oldCFLAGS"
   CXXFLAGS="$oldCXXFLAGS"
   LDFLAGS="$oldLDFLAGS"
   AS_IF([test AS_VAR_GET([usable]) = yes], [$6], [$7])[]
])


dnl PTLIB_PKG_CHECK_MODULE
dnl As PKG_CHECK_MODULES but does test compile so works wit cross compilers
dnl $1 module name
dnl $2 pkg name
dnl $3 program headers
dnl $4 program main
dnl $5 success code
dnl $6 failure code
AC_DEFUN([PTLIB_PKG_CHECK_MODULE],[
   PKG_CHECK_MODULES(
      [$1],
      [$2],
      [PTLIB_LINK_IFELSE(
         [for $1 usability],
         [$$1[_CFLAGS]],
         [$$1[_LIBS]],
         [$3],
         [$4],
         [
            PTLIB_CFLAGS="$PTLIB_CFLAGS $$1[_CFLAGS]"
            ENDLDLIBS="$ENDLDLIBS $$1[_LIBS]"
         ]
      )],
      [usable=no]
   )
   AS_IF([test AS_VAR_GET([usable]) = yes], [$5], [$6])[]
])


dnl PTLIB_MODULE_OPTION
dnl Check for modules existence, with --disable-XXX and optional --with-XXX-dir
dnl $1 module name
dnl $2 option name
dnl $3 option help text
dnl $4 pkg name(s)
dnl $5 default CFLAGS
dnl $6 default LIBS
dnl $7 program headers
dnl $8 program main
dnl $9 success code
dnl $10 failure code
dnl $11 optional dependency
AC_DEFUN([PTLIB_MODULE_OPTION],[
   AC_ARG_ENABLE(
      [$2],
      [AC_HELP_STRING([--disable-$2],[disable $3])],
      [
         usable="$enableval"
         if test "x$usable" = "xyes" ; then
            AC_MSG_NOTICE($3 disabled by user)
         fi
      ],
      [
         usable="$DEFAULT_$1"
         if test "x$usable" = "x" ; then
            usable=no
         fi
         if test "x$usable" = "xno" ; then
            AC_MSG_NOTICE($3 disabled by default)
         fi
      ]
   )

   m4_ifnblank([$11],[
      if test "x$m4_normalize($11)" != "x1" ; then
         AC_MSG_NOTICE($3 disabled due to disabled dependency $11)
         usable=no
      fi
   ])

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
         PTLIB_LINK_IFELSE(
            [for $1 usability],
            [$$1[_CFLAGS]],
            [$$1[_LIBS]],
            [$7],
            [$8],
            [
               PTLIB_CFLAGS="$PTLIB_CFLAGS $$1[_CFLAGS]"
               ENDLDLIBS="$ENDLDLIBS $$1[_LIBS]"
            ],
            [usable=no]
         )
      fi
   fi

   m4_ifnblank([$9$10],[AS_IF([test AS_VAR_GET([usable]) = yes], [$9], [$10])])

   PTLIB_SUBST($1, $usable)
])

