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

dnl PTLIB_CHECK_FDSIZE
dnl check for select_large_fdset (Solaris)
dnl Arguments: $STDCCFLAGS
dnl Return:    $STDCCFLAGS
AC_DEFUN([PTLIB_CHECK_FDSIZE],
         [
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
            STDCCFLAGS="$STDCCFLAGS -DFD_SETSIZE=${ptlib_fdsize}"
          fi

          AC_MSG_RESULT(${ptlib_fdsize})
         ])

