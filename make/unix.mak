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
# Revision 1.33  1999/02/22 00:55:07  robertj
# BeOS port changes.
#
# Revision 1.33  1999/02/22 00:55:07  robertj
# BeOS port changes.
#
# Revision 1.32  1999/02/06 08:44:55  robertj
# Fixed mistake in last change, library must be at end of link command.
#
# Revision 1.31  1999/02/06 05:49:44  robertj
# BeOS port effort by Yuri Kiryanov <yk@altavista.net>
#
# Revision 1.30  1999/01/16 09:56:28  robertj
# Changed some macros to more informative names.
#
# Revision 1.29  1999/01/08 01:43:44  robertj
# Changes to minimise the command line length.
# FreeBSD pthreads support
#
# Revision 1.28  1999/01/02 00:55:48  robertj
# Improved OSTYPE detection.
# Supported solaris x86
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


ifndef DEBUG
DEBUG := 1
endif

ifndef PWLIBDIR
PWLIBDIR := $(HOME)/pwlib
endif


###############################################################################
#
#  Normalise environment variables so have OSTYPE and MACHTYPE correct
#

ifndef OSTYPE
OSTYPE := $(shell uname -s)
endif

ifndef MACHTYPE
MACHTYPE := $(shell uname -m)
endif

ifneq (,$(findstring $(HOSTTYPE),i386-linux i486-linux))
OSTYPE   := linux
MACHTYPE := x86
endif

ifeq ($(OSTYPE),Linux)
OSTYPE := linux
endif

ifeq ($(OSTYPE),mklinux)
OSTYPE   := linux
MACHTYPE := ppc
endif

ifneq (,$(findstring $(OSTYPE),Solaris SunOS))
OSTYPE := solaris
endif

ifneq (,$(findstring $(MACHTYPE),i386 i486 i586 i686 i86pc))
MACHTYPE := x86
endif


ifeq (,$(findstring $(OSTYPE),linux FreeBSD solaris))

all ::
	@echo ######################################################################
	@echo "Warning: OSTYPE=$(OSTYPE) support has not been confirmed. If you get"
	@echo "         it working please send patches to support@equival.com.au"
	@echo ######################################################################
	@echo

endif


###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#



PLATFORM_TYPE = $(OSTYPE)_$(MACHTYPE)
STDCCFLAGS	:= $(STDCCFLAGS) $(DEBUG_FLAG) -D_DEBUG -DPMEMORY_CHECK=1
LDFLAGS		:= $(LDFLAGS) $(DEBLDFLAGS)



ifdef	DEBUG
OPTCCFLAGS	:= $(OPTCCFLAGS) -O2 -DNDEBUG
#OPTCCFLAGS	:= $(OPTCCFLAGS) -DP_USE_INLINES=1
#OPTCCFLAGS	:= $(OPTCCFLAGS) -fconserve-space
LDFLAGS		:= $(LDFLAGS) -s

OBJ_SUFFIX	:= d
else
OBJ_SUFFIX	:= r
endif # DEBUG


####################################################

ifeq ($(OSTYPE),linux)

STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX

else
endif

ifeq ($(MACHTYPE),ppc)
ENDIAN		:= PBIG_ENDIAN
# i486 Linux for x86, using gcc 2.7.2
STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX -DP_HAS_INT64

endif


ifdef SHAREDLIB
ifndef PROG
PLATFORM_TYPE	:= $(PLATFORM_TYPE)_pic
STDCCFLAGS	:= $(STDCCFLAGS) -fPIC
endif # PROG
endif # SHAREDLIB

STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a
SYSLIBDIR	:= /usr/lib

endif # linux


####################################################

ifeq ($(OSTYPE),FreeBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	:= $(STDCCFLAGS) -DP_FREEBSD -DP_HAS_INT64
endif

STDCCFLAGS	:= $(STDCCFLAGS) -DP_FREEBSD

ifdef P_PTHREADS
CFLAGS	:= $(CFLAGS) -pthread
endif

RANLIB		:= 1

endif # FreeBSD


####################################################

ifeq ($(OSTYPE),sunos)

# Sparc Sun 4x, using gcc 2.7.2

RANLIB		:= 1
REQUIRES_SEPARATE_SWITCH = 1

endif # sunos


####################################################

ifeq ($(OSTYPE),solaris)

#  Solaris (Sunos 5.x)

#P_SSL		= $(PWLIBDIR)
P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
DEBUG_FLAG	:= -gstabs+
else
ENDIAN		:= PBIG_ENDIAN
endif

STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS=$(OSRELEASE) -DP_HAS_INT64

# Sparc Solaris 2.x, using gcc 2.7.2
STDCCFLAGS	:= $(STDCCFLAGS) -DP_SOLARIS=$(OSRELEASE)
LDLIBS		:= $(LDLIBS) -lsocket -lnsl -ldl -lposix4
LDFLAGS		:= -R/usr/local/gnu/lib

#RANLIB		:= 1

STATIC_LIBS	:= libstdc++.a libg++.a 
SYSLIBDIR	:= /usr/local/gnu/lib

ifdef P_PTHREADS
ENDLDLIBS	:= $(ENDLDLIBS) -lpthread
STDCCFLAGS	:= $(STDCCFLAGS) -D_REENTRANT
endif

endif # solaris


####################################################

STDCCFLAGS	:= $(STDCCFLAGS) -DP_HAS_INT64

ifdef BE_THREADS
STDCCFLAGS	:= $(STDCCFLAGS) -DBE_THREADS -DP_PLATFORM_HAS_THREADS
endif

endif # beos


####################################################

ifeq ($(OSTYPE),ultrix)

STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX -DP_HAS_INT64

# R2000 Ultrix 4.2, using gcc 2.7.x
STDCCFLAGS	:= $(STDCCFLAGS) -DP_ULTRIX

endif # ultrix


####################################################

ifeq ($(OSTYPE),hpux)

# HP/UX 9.x, using gcc 2.6.C3 (Cygnus version)
STDCCFLAGS	:= $(STDCCFLAGS) -DP_HPUX9

endif # hpux


###############################################################################
#
# Make sure some things are defined
#

ifndef ENDIAN
ENDIAN		:= PLITTLE_ENDIAN
endif

ifndef DEBUG_FLAG
DEBUG_FLAG	:= -g
endif


###############################################################################
#
CPLUS		= g++

CC		:= gcc
CPLUS		:= g++
SHELL		:= /bin/sh

.SUFFIXES:	.cxx .prc 

# Directories

UNIX_INC_DIR	= $(PWLIBDIR)/include/ptlib/unix
UNIX_SRC_DIR	= $(PWLIBDIR)/src/ptlib/unix

OBJDIR		= obj_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
LIBDIR		= $(PWLIBDIR)/lib


# set name of the PT library

PTLIB		= pt_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)

ifndef SHAREDLIB
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).a
LDFLAGS		:= $(LDFLAGS) -s

endif # DEBUG


# define SSL variables

ifdef P_SSL

SSLEAY		:= $(HOME)/src/SSLeay-0.6.6
SSLDIR		:= /usr/local/ssl
CFLAGS		:= $(CFLAGS) -DP_SSL -I$(SSLDIR)/include -I$(SSLEAY)/crypto
LDFLAGS		:= $(LDFLAGS) -L$(SSLDIR)/lib
ENDLDLIBS	:= $(ENDLDLIBS) -lssl -lcrypto

endif


# define Posix threads stuff
ifdef P_PTHREADS
STDCCFLAGS	:= $(STDCCFLAGS) -DP_PTHREADS
endif


# compiler flags for all modes
STDCCFLAGS	:= $(STDCCFLAGS) -DPBYTE_ORDER=$(ENDIAN) -Wall
#STDCCFLAGS	:= $(STDCCFLAGS) -fomit-frame-pointer
#STDCCFLAGS	:= $(STDCCFLAGS) -fno-default-inline
#STDCCFLAGS     := $(STDCCFLAGS) -Woverloaded-virtual
#STDCCFLAGS     := $(STDCCFLAGS) -fno-implement-inlines

# add OS directory to include path
STDCCFLAGS	:= $(STDCCFLAGS) -I$(UNIX_INC_DIR)


# add library directory to library path and include the library
LDFLAGS		:= $(LDFLAGS) -L$(LIBDIR)
LDLIBS		:= $(LDLIBS) -l$(PTLIB) 


#  clean whitespace out of source file list
SOURCES		:= $(strip $(SOURCES))


# End of unix.mak
