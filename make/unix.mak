#
# unix.mak
#
# First part of make rules, included in ptlib.mak and pwlib.mak.
# Note: Do not put any targets in the file. This should defaine variables
#       only, as targets are all in common.mak
#
# Portable Windows Library
#
# Copyright (c) 1993-1998 Equivalence Pty. Ltd.
#
# The contents of this file are subject to the Mozilla Public License
# Version 1.0 (the "License"); you may not use this file except in
# compliance with the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS"
# basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
# the License for the specific language governing rights and limitations
# under the License.
#
# The Original Code is Portable Windows Library.
#
# The Initial Developer of the Original Code is Equivalence Pty. Ltd.
#
# Portions are Copyright (C) 1993 Free Software Foundation, Inc.
# All Rights Reserved.
# 
# Contributor(s): ______________________________________.
#
# $Log: unix.mak,v $
# Revision 1.27  1998/12/02 02:37:41  robertj
# New directory structure.
#
# Revision 1.27  1998/12/02 02:37:41  robertj
# New directory structure.
#
# Revision 1.26  1998/11/24 09:39:19  robertj
# FreeBSD port.
#
# Revision 1.25  1998/11/22 08:11:41  craigs
# *** empty log message ***
#
# Revision 1.24  1998/11/16 07:30:15  robertj
# Removed confusion between sunos and solaris
#
# Revision 1.23  1998/11/14 10:47:22  robertj
# Support for PPC Linux, better arrangement of variables.
#
# Revision 1.22  1998/09/24 04:20:53  robertj
# Added open software license.
#
DEBUG=1

ifndef DEBUG
DEBUG := 1
PWLIBDIR = $(HOME)/pwlib

ifndef PWLIBDIR
PWLIBDIR := $(HOME)/pwlib


###############################################################################
#
ifeq ($(OSTYPE),mklinux)
OSTYPE=linux
MACHTYPE=ppc

ifndef OSTYPE
ifeq ($(HOSTTYPE),i486-linux)
OSTYPE=linux
MACHTYPE=x86

ifndef MACHTYPE
ifeq ($(HOSTTYPE),i386-linux)
OSTYPE=linux
MACHTYPE=x86
ifneq (,$(findstring $(HOSTTYPE),i386-linux i486-linux))
OSTYPE   := linux
ifeq ($(MACHTYPE),i486)
MACHTYPE=x86

ifeq ($(OSTYPE),Linux)
ifeq ($(MACHTYPE),i386)
MACHTYPE=x86
ifeq ($(OSTYPE),mklinux)
OSTYPE   := linux
ifneq (,$(findstring $(OSTYPE),Solaris SunOS))
ifndef OSTYPE
error ::
	@echo Must define OSTYPE environment variable

ifndef MACHTYPE
error ::
	@echo Must define MACHTYPE environment variable
	@echo ######################################################################
	@echo

####################################################
#
#  Linux
#
endif # DEBUG


####################################################

ifeq ($(OSTYPE),linux)

STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX
ENDIAN=PLITTLE_ENDIAN

else
ENDIAN=PBIG_ENDIAN

ifeq ($(MACHTYPE),ppc)
ENDIAN		:= PBIG_ENDIAN
# i486 Linux for x86, using gcc 2.7.2
STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX -DP_HAS_INT64 -DPBYTE_ORDER=$(ENDIAN) -DPCHAR8=PANSI_CHAR

endif

OBJ_SUFFIX	= pic
ifdef SHAREDLIB
ifndef PROG
PLATFORM_TYPE	:= $(PLATFORM_TYPE)_pic
STDCCFLAGS	:= $(STDCCFLAGS) -fPIC
STATIC_LIBS	= libstdc++.a libg++.a libm.a libc.a
SYSLIBDIR	= /usr/lib

STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a
SYSLIBDIR	:= /usr/lib

endif # linux
#
#  Sunos 4.1.x
#
####################################################


####################################################

ENDIAN=PLITTLE_ENDIAN
P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	:= $(STDCCFLAGS) -DP_FREEBSD -DP_HAS_INT64 -DPBYTE_ORDER=PLITTLE_ENDIAN -DPCHAR8=PANSI_CHAR 
endif
CFLAGS	:= $(CFLAGS) -pthread
endif

RANLIB		:= 1

endif # FreeBSD
#
#  Sunos 4.1.x
#
####################################################


####################################################

ifeq ($(OSTYPE),sunos)

# Sparc Sun 4x, using gcc 2.7.2

RANLIB		:= 1
REQUIRES_SEPARATE_SWITCH = 1

endif # sunos
#
#  Solaris (Sunos 5.x)
#
####################################################


####################################################

P_PTHREADS	= 1

#P_SSL		= $(PWLIBDIR)
ENDIAN=PLITTLE_ENDIAN

ENDIAN=PBIG_ENDIAN
DEBUG_FLAG	:= -gstabs+
else

STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS -DP_HAS_INT64 -DPBYTE_ORDER=$(ENDIAN) -DPCHAR8=PANSI_CHAR 

# Sparc Solaris 2.x, using gcc 2.7.2
STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS=$(OSRELEASE)
LDLIBS		:= $(LDLIBS) -lsocket -lnsl -ldl -lposix4
LDFLAGS		:= -R/usr/local/gnu/lib
STATIC_LIBS	= libstdc++.a libg++.a 
SYSLIBDIR	= /usr/local/gnu/lib

STATIC_LIBS	:= libstdc++.a libg++.a 
SYSLIBDIR	:= /usr/local/gnu/lib

ifdef P_PTHREADS
ENDLDLIBS	:= $(ENDLDLIBS) -lpthread
STDCCFLAGS	:= $(STDCCFLAGS) -D_REENTRANT
endif

endif # solaris
#
#  Other
#
endif # beos


####################################################

STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX  -DP_HAS_INT64 -DPBYTE_ORDER=PBIG_ENDIAN -DPCHAR8=PANSI_CHAR 

# R2000 Ultrix 4.2, using gcc 2.7.x
STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX


####################################################

ifeq ($(OSTYPE),hpux)

# HP/UX 9.x, using gcc 2.6.C3 (Cygnus version)
STDCCFLAGS	:= $(STDCCFLAGS) -DP_HPUX9

endif # hpux


###############################################################################
#
SHELL		= /bin/sh
CPLUS		:= g++
SHELL		:= /bin/sh
OBJ_SUFFIX	= $(OSTYPE)_$(MACHTYPE)


.SUFFIXES:	.cxx .prc 

# Directories

#
endif # DEBUG
#


# define SSL variables
SSLEAY		= $(HOME)/src/SSLeay-0.6.6
SSLDIR		= /usr/local/ssl

SSLEAY		:= $(HOME)/src/SSLeay-0.6.6
SSLDIR		:= /usr/local/ssl
CFLAGS		:= $(CFLAGS) -DP_SSL -I$(SSLDIR)/include -I$(SSLEAY)/crypto
LDFLAGS		:= $(LDFLAGS) -L$(SSLDIR)/lib
ENDLDLIBS	:= $(ENDLDLIBS) -lssl -lcrypto
#
endif
#

STDCCFLAGS          := $(STDCCFLAGS) -DP_PLATFORM_HAS_THREADS -DP_PTHREADS
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

#
# add PW library directory to library path
#
LDFLAGS		:= $(LDFLAGS) -L$(LIBDIR) 

##########################################################################
#
#  set up for correct operating system
#

#
# set name of the PT library
#
ifndef LIB_SUFFIX
LIB_SUFFIX	= $(OBJ_SUFFIX)
endif
PTLIB		= pt_$(LIB_SUFFIX)_$(LIBID)

ifndef SHAREDLIB
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).a
else
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).so
endif

#
#STDCCFLAGS     := $(STDCCFLAGS) -Woverloaded-virtual
#
#STDCCFLAGS     := $(STDCCFLAGS) -fno-implement-inlines

#
# add OS library
#
LDLIBS		:= $(LDLIBS) -l$(PTLIB) 
# add library directory to library path and include the library
LDFLAGS		:= $(LDFLAGS) -L$(LIBDIR)
#
LDLIBS		:= $(LDLIBS) -l$(PTLIB) 
#


#  clean whitespace out of source file list
######################################################################


SOURCES		:= $(strip $(SOURCES))



# End of unix.mak
