# end of user configurable items

#
#  clean whitespace out of source file list
#
SOURCES		:= $(strip $(SOURCES))
# Revision 1.22  1998/09/24 04:20:53  robertj

#  defines for common Unix types
###############################################################################
#
ifeq ($(HOSTTYPE),sun4)
P_SUN4  	= 1
endif
ifndef OSTYPE
ifeq ($(HOSTTYPE),i486-linux)
P_LINUX		= 1
endif
ifndef MACHTYPE
#P_LINUX	= 1
#P_SUN4  	= 1
#P_SOLARIS	= 1
#P_HPUX		= 1
#P_ULTRIX	= 1

#STDCCFLAGS	:= -DPHAS_TEMPLATES

####################################################
#
#  Linux
#
endif # DEBUG

ifdef P_LINUX
ifeq ($(MACHTYPE),ppc)
# i486 Linux for x86, using gcc 2.6.x
#STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX -DP_HAS_INT64 -DPBYTE_ORDER=PLITTLE_ENDIAN -DPCHAR8=PANSI_CHAR -m486
ENDIAN		:= PBIG_ENDIAN
# i486 Linux for x86, using gcc 2.7.2
STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX -DP_HAS_INT64 -DPBYTE_ORDER=PLITTLE_ENDIAN -DPCHAR8=PANSI_CHAR -m486

endif
LIB_SUFFIX	= linuxpic
ifdef PROG
OBJ_SUFFIX	= linux
else
OBJ_SUFFIX	= linuxpic
ifdef SHAREDLIB
endif
else
OBJ_SUFFIX	= linux
OBJ_SUFFIX	= linux
endif
STDCCFLAGS	:= $(STDCCFLAGS) -fPIC
endif

endif # FreeBSD
#
#  Sun
#
####################################################

ifdef P_SUN4
####################################################
# Sparc Sun 4x, using gcc 2.6.3
STDCCFLAGS	:= $(STDCCFLAGS) -DP_SUN4 -DP_HAS_INT64 -DPBYTE_ORDER=PBIG_ENDIAN -DPCHAR8=PANSI_CHAR 


OBJ_SUFFIX	= sun4
# Sparc Sun 4x, using gcc 2.7.2

endif

ifdef P_SOLARIS
else
# Sparc Solaris 2.x, using gcc 2.6.3
STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS -DP_HAS_INT64 -DPBYTE_ORDER=PLITTLE_ENDIAN -DPCHAR8=PANSI_CHAR 
STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS=$(OSRELEASE)
OBJ_SUFFIX	= solaris

endif

endif # solaris
#
#  Other
#
endif # beos

ifdef P_ULTRIX
####################################################

STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX  -DP_HAS_INT64 -DPBYTE_ORDER=PBIG_ENDIAN -DPCHAR8=PANSI_CHAR 

OBJ_SUFFIX	= ultrix
STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX
endif

ifdef P_HPUX
####################################################

ifeq ($(OSTYPE),hpux)

OBJ_SUFFIX	= hpux

endif
STDCCFLAGS	:= $(STDCCFLAGS) -DP_HPUX9


###############################################################################
SHELL		= /bin/sh
CPLUS		:= g++
SHELL		:= /bin/sh
#
# if there is no PWLIBDIR variable set, then set one
#
ifndef PWLIBDIR
#PWLIBDIR	= /usr/local/pwlib
PWLIBDIR	= $(HOME)/pwlib
# define Posix threads stuff
ifdef P_PTHREADS
#
#  define names of some other programs we run
#
CPLUS		= g++

#
endif
#
STDCCFLAGS	:= $(STDCCFLAGS) -Wall
#STDCCFLAGS      := $(STDCCFLAGS) -fomit-frame-pointer
#STDCCFLAGS      := $(STDCCFLAGS) -fno-default-inline

# not normally used
STDCCFLAGS	:= $(STDCCFLAGS) -DPBYTE_ORDER=$(ENDIAN) -Wall
#STDCCFLAGS	:= $(STDCCFLAGS) -fomit-frame-pointer
#STDCCFLAGS	:= $(STDCCFLAGS) -fno-default-inline
#
# if using debug, add -g and set debug ID
#
ifdef	DEBUG

LIBID		= d
STDCCFLAGS	:= $(STDCCFLAGS) -DPMEMORY_CHECK=1 -D_DEBUG
STDCCFLAGS	:= $(STDCCFLAGS) -g
LDFLAGS		:= $(LDFLAGS) $(DEBLDFLAGS)

else

LIBID		= r
OPTCCFLAGS	:= $(OPTCCFLAGS) -O2 -DNDEBUG
#OPTCCFLAGS	:= $(OPTCCFLAGS) -fconserve-space
#OPTCCFLAGS	:= $(OPTCCFLAGS) -DP_USE_INLINES=1
LDFLAGS		:= $(LDFLAGS) -s

endif # DEBUG

OBJDIR		:= obj_$(OBJ_SUFFIX)_$(LIBID)

LIBDIR		= $(PWLIBDIR)/lib
COMMONDIR	= $(PWLIBDIR)/common

#
# add PW library directory to library path
#
LDFLAGS		:= $(LDFLAGS) -L$(LIBDIR) 

##########################################################################
#
#  set up for correct operating system
#

OS		= unix

#
# set name of the PT library
#
ifndef LIB_SUFFIX
LIB_SUFFIX	= $(OBJ_SUFFIX)
endif
OSDIR		= $(PWLIBDIR)/$(OS)
PTLIB		= pt_$(OS)_$(LIB_SUFFIX)_$(LIBID)

ifndef SHAREDLIB
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).a
else
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).so
endif

VPATH_CXX	:= $(VPATH_CXX) $(OSDIR)/src 
VPATH_H		:= $(VPATH_H) $(OSDIR)/include

#
#STDCCFLAGS     := $(STDCCFLAGS) -Woverloaded-virtual
#
STDCCFLAGS	:= -I$(OSDIR)/include $(STDCCFLAGS)

#
# add OS library
#
LDLIBS		:= $(LDLIBS) -l$(PTLIB) 


# End of unix.mak
