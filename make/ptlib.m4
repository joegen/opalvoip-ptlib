dnl
dnl ptlib.m4
dnl

m4_include(make/my_macros.m4)


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

