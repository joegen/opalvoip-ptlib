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
# Revision 1.71  2000/04/07 06:22:33  rogerh
# Add comment about the -s flag and Mac OS X
#
# Revision 1.70  2000/04/06 20:12:33  craigs
# Added install targets
#
# Revision 1.69  2000/04/06 11:37:51  rogerh
# Add MacOS X support from Kevin Packard
#
# Revision 1.68  2000/04/03 22:31:08  rogerh
# Get a more exact FreeBSD version number using a kernel sysctl
#
# Revision 1.67  2000/03/17 03:47:00  craigs
# Changed DEBUG version to always be static
#
# Revision 1.66  2000/03/09 14:22:04  rogerh
# OpenBSD requires -lossaudio for OSS Audio support
#
# Revision 1.65  2000/03/08 12:17:09  rogerh
# Add OpenBSD support
#
# Revision 1.64  2000/03/08 07:09:38  rogerh
# Fix typo in previous commit
#
# Revision 1.63  2000/03/08 06:54:03  rogerh
# Add support for bash shell on FreeBSD
# 1) support bash OSTYPE which is of the form os+version eg freebsd3.4
# 2) Fixed problem where bash has HOSTTYPE of i386, and was matching the
#    ifneq (,$(findstring $(HOSTTYPE),i386-linux i486-linux)) test as 'i386' is a
#    substring of 'i386-linux'
#
# Revision 1.62  2000/03/03 00:37:42  robertj
# Fixed problem for when have GUI environment variable set, always builds GUI!
#
# Revision 1.61  2000/02/24 11:07:01  craigs
# Fixed problem with making PW projects
#
# Revision 1.60  2000/02/04 19:33:25  craigs
# Added ability to create non-shared versions of programs
#
# Revision 1.59  2000/02/03 23:46:26  robertj
# Added power PC linux variation, thanks Brad Midgley
#
# Revision 1.58  2000/01/25 04:55:36  robertj
# Added FreeBSD support for distinction between v3.x and later versions. Thanks Roger Hardiman.
#
# Revision 1.57  2000/01/25 04:05:23  robertj
# Fixed make files for GUI systems and moved object directories to lib directory.
#
# Revision 1.56  2000/01/22 00:51:18  craigs
# Added ability to compile in any directory, and to create shared libs
#
# Revision 1.55  2000/01/16 12:39:10  craigs
# Added detection of all known ix86 MACHTYPE variants
#
# Revision 1.54  2000/01/10 02:38:46  craigs
# Fixed problem when creating dependencies with OpenSSL
#
# Revision 1.53  2000/01/10 02:23:47  craigs
# Updated for new OpenSSL
#
# Revision 1.52  1999/11/11 08:11:01  robertj
# Reworded warning in vain hope people will understand!
#
# Revision 1.51  1999/10/22 10:21:46  craigs
# Added define to only include semaphore libraries on Linux platform
#
# Revision 1.50  1999/09/27 01:04:42  robertj
# BeOS support changes.
#
# Revision 1.49  1999/09/21 00:56:29  robertj
# Added more sound support for BeOS (thanks again Yuri!)
#
# Revision 1.48  1999/08/24 01:58:29  robertj
# Added normalisation of sun4 architecture MACHTYPE to be sparc.
#
# Revision 1.47  1999/08/09 12:46:07  robertj
# Added support for libc5 and libc6 compiles under Linux (libc6 uses pthreads).
#
# Revision 1.46  1999/07/31 03:53:16  robertj
# Allowed for override of object directory suffix
#
# Revision 1.45  1999/07/11 14:53:38  robertj
# Temporarily removed pthreads for linux as is not very portable on various linuxes
#
# Revision 1.44  1999/07/11 13:42:13  craigs
# pthreads support for Linux
#
# Revision 1.43  1999/07/03 04:31:53  robertj
# Fixed problems with not including oss.cxx in library if OSTYPE not "linux"
#
# Revision 1.42  1999/06/28 09:12:01  robertj
# Fixed problems with the order in which macros are defined especially on BeOS & Solaris
#
# Revision 1.41  1999/06/27 02:42:10  robertj
# Fixed BeOS compatability.
# Fixed error of platform name not supported, needed :: on main targets.
#
# Revision 1.40  1999/06/12 06:43:36  craigs
# Added PTLIB_ALT variable to allow differentiatio between libc5 and libc6 machines
#
# Revision 1.39  1999/06/09 15:41:18  robertj
# Added better UI to make files.
#
# Revision 1.38  1999/06/07 04:49:46  robertj
# Added support for SuSe linux variant.
#
# Revision 1.38  1999/06/07 04:47:18  robertj
# Added support for SUSE linux.
#
# Revision 1.37  1999/05/01 11:29:19  robertj
# Alpha linux port changes.
#
# Revision 1.36  1999/04/29 08:46:50  robertj
# Force use of GNU C compiler for .c files not only c++ files.
#
# Revision 1.35  1999/04/29 07:04:04  robertj
# Fixed missing -g in debug version
#
# Revision 1.34  1999/03/05 07:03:27  robertj
# Some more BeOS port changes.
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


ifndef PWLIBDIR
PWLIBDIR = $(HOME)/pwlib
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

ifneq (,$(findstring linux,$(HOSTTYPE)))
ifneq (,$(findstring $(HOSTTYPE),i386-linux i486-linux))
OSTYPE   := linux
MACHTYPE := x86
endif
endif

ifeq ($(OSTYPE),mklinux)
OSTYPE   := linux
MACHTYPE := ppc
endif

ifneq (,$(findstring $(OSTYPE),Linux linux-gnu))
OSTYPE := linux
endif

ifneq (,$(findstring $(OSTYPE),Solaris SunOS))
OSTYPE := solaris
endif

#Convert bash shell OSTYPE of 'freebsd3.4' to 'FreeBSD'
ifneq (,$(findstring freebsd,$(OSTYPE)))
OSTYPE := FreeBSD
endif

#Convert bash shell OSTYPE of 'openbsd2.6' to 'OpenBSD'
ifneq (,$(findstring openbsd,$(OSTYPE)))
OSTYPE := OpenBSD
endif

ifneq (,$(findstring macos,$(OSTYPE)))
OSTYPE := macos
endif

ifneq (,$(findstring $(MACHTYPE),sun4))
MACHTYPE := sparc
endif

ifneq (,$(findstring $(MACHTYPE),i386 i486 i586 i686 i86pc i686-pc-linux-gnu))
MACHTYPE := x86
endif

ifneq (,$(findstring i386, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring i486, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring i586, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring i686, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring x86, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring powerpc, $(MACHTYPE)))
MACHTYPE := ppc
endif


.PHONY: all debug opt both release clean debugclean optclean debugdepend optdepend bothdepend


ifeq (,$(findstring $(OSTYPE),linux FreeBSD OpenBSD solaris beos macos))

all ::
	@echo
	@echo ######################################################################
	@echo "Warning: OSTYPE=$(OSTYPE) support has not been confirmed.  This may"
	@echo "         be a new operating system not yet encountered, or more"
	@echo "         likely, the OSTYPE and MACHTYPE environment variables are"
	@echo "         set to unusual values. You may need to explicitly set these"
	@echo "         variables for the correct operation of this system."
	@echo
	@echo "         Currently supported OSTYPE names are:"
	@echo "              linux Linux linux-gnu mklinux"
	@echo "              solaris Solaris SunOS"
	@echo "              FreeBSD OpenBSD beos macos"
	@echo
	@echo "              **********************************"
	@echo "              *** DO NOT IGNORE THIS MESSAGE ***"
	@echo "              **********************************"
	@echo
	@echo "         The system almost certainly will not compile! When you get"
	@echo "         it working please send patches to support@equival.com.au"
	@echo ######################################################################
	@echo

debug :: all
opt :: all
both :: all
release :: all
clean :: all
debugclean :: all
optclean :: all
debugdepend :: all
optdepend :: all
bothdepend :: all

else

all ::
	@echo "The following targets are available:"
	@echo "    make debug       Make debug version of application"
	@echo "    make opt         Make optimised version of application"
	@echo "    make both        Make both versions of application"
	@echo "    make release     Package up optimised version int tar.gz file"
	@echo "    make clean       Remove both debug and optimised files"
	@echo "    make debugclean  Remove debug files"
	@echo "    make optclean    Remove optimised files"
	@echo "    make debugdepend Create debug dependency files"
	@echo "    make optdepend   Create optimised dependency files"
	@echo "    make bothdepend  Create both debug and optimised dependency files"

endif



####################################################

ifeq ($(OSTYPE),linux)

# Enable pthreads if we are using glibc 6
ifneq (,$(shell grep define.\*__GNU_LIBRARY__.\*6 /usr/include/features.h))
P_PTHREADS	= 1
else
ifndef PTLIB_ALT
PTLIB_ALT = libc5
endif
endif


# i486 Linux for x86, using gcc 2.7.2
STDCCFLAGS	+= -DP_LINUX


ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
endif

ifeq ($(MACHTYPE),alpha)
STDCCFLAGS	+= -DP_64BIT
endif

ifeq ($(MACHTYPE),ppc)
ENDIAN		:= PBIG_ENDIAN
endif

ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
STDCCFLAGS	+= -D_REENTRANT -DP_HAS_SEMAPHORES
endif

ifndef DEBUG
ifndef P_SHAREDLIB
P_SHAREDLIB=1
endif
endif

ifeq ($(P_SHAREDLIB),0)
LIB_TYPE	= _s
else
LDLIBS		+= -ldl
ifndef PROG
STDCCFLAGS	+= -fPIC
endif # PROG
endif # SHAREDLIB


STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a
SYSLIBDIR	:= /usr/lib
#LDFLAGS		+= --no-whole-archive --cref

endif # linux


####################################################

ifeq ($(OSTYPE),FreeBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
endif

ifndef OSRELEASE
OSRELEASE	:= $(shell /sbin/sysctl -n kern.osreldate)
endif

STDCCFLAGS	+= -DP_FREEBSD=$(OSRELEASE)

ifdef P_PTHREADS
CFLAGS	+= -pthread
endif

RANLIB		:= 1

endif # FreeBSD


####################################################

ifeq ($(OSTYPE),OpenBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
endif

STDCCFLAGS	+= -DP_OPENBSD
LDLIBS		+= -lossaudio

ifdef P_PTHREADS
CFLAGS	+= -pthread
endif

RANLIB		:= 1

endif # OpenBSD


####################################################

ifeq ($(OSTYPE),sunos)

# Sparc Sun 4x, using gcc 2.7.2

RANLIB		:= 1
REQUIRES_SEPARATE_SWITCH = 1

endif # sunos


####################################################

ifeq ($(OSTYPE),solaris)

#  Solaris (Sunos 5.x)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
DEBUG_FLAG	:= -gstabs+
else
ENDIAN		:= PBIG_ENDIAN
endif

OSRELEASE	:= $(subst 5.,,$(shell uname -r))

# Sparc Solaris 2.x, using gcc 2.7.2
STDCCFLAGS	+= -DP_SOLARIS=$(OSRELEASE)
LDLIBS		+= -lsocket -lnsl -ldl -lposix4
LDFLAGS		+= -R/usr/local/gnu/lib

#RANLIB		:= 1

STATIC_LIBS	:= libstdc++.a libg++.a 
SYSLIBDIR	:= /usr/local/gnu/lib

ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
STDCCFLAGS	+= -D_REENTRANT
endif

endif # solaris


####################################################

ifeq ($(OSTYPE),beos)

BE_THREADS := 0

# BeOS R4, using gcc from Cygnus version 2.9-beos-980929
LDLIBS		+= -lbe -lmedia -lgame

ifdef BE_THREADS
STDCCFLAGS	+= -DBE_THREADS -DP_PLATFORM_HAS_THREADS
endif

STDCCFLAGS	+= -Wno-multichar

endif # beos


####################################################

ifeq ($(OSTYPE),ultrix)

ENDIAN	:= PBIG_ENDIAN

# R2000 Ultrix 4.2, using gcc 2.7.x
STDCCFLAGS	+= -DP_ULTRIX

endif # ultrix


####################################################

ifeq ($(OSTYPE),hpux)

# HP/UX 9.x, using gcc 2.6.C3 (Cygnus version)
STDCCFLAGS	+= -DP_HPUX9

endif # hpux


####################################################
 
ifeq ($(OSTYPE),macos)
 
# MacOS X or later (derived from FreeBSD)
 
STDCCFLAGS	+= -DP_MACOSX
 
# pthreads not working in DP3, will revisit this on next release of OS X        - krp 03/17/00
#P_PTHREADS	:= 1    # DP3 system file <pthreads.h> has bug in macros "pthread_cleanup_push" and "pthread_cleanup_pop"
  
ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
else
ENDIAN		:= PBIG_ENDIAN
endif
  
RANLIB		:= 1

CC              := cc
CPLUS           := c++
 
endif # macos
 
 
###############################################################################
#
# Make sure some things are defined
#

ifndef	CC
CC		:= gcc
endif

ifndef CPLUS
CPLUS		:= g++
endif

ifndef INSTALL
INSTALL		:= install
endif

ifndef P_SHAREDLIB
P_SHAREDLIB=0
endif

ifndef ENDIAN
ENDIAN		:= PLITTLE_ENDIAN
endif

ifndef DEBUG_FLAG
DEBUG_FLAG	:= -g
endif

ifndef PTLIB_ALT
PLATFORM_TYPE = $(OSTYPE)_$(MACHTYPE)
else
PLATFORM_TYPE = $(OSTYPE)_$(PTLIB_ALT)_$(MACHTYPE)
endif

ifndef OBJ_SUFFIX
ifdef	DEBUG
OBJ_SUFFIX	:= d
else
OBJ_SUFFIX	:= r
endif # DEBUG
endif # OBJ_SUFFIX

ifeq ($(P_SHAREDLIB),0)
LIB_SUFFIX	= a
else
LIB_SUFFIX	= so
endif

ifndef LIB_TYPE
LIB_TYPE	=
endif

ifndef INSTALL_DIR
INSTALL_DIR	= /usr/local
endif

ifndef INSTALLBIN_DIR
INSTALLBIN_DIR	= $(INSTALL_DIR)/bin
endif

ifndef INSTALLLIB_DIR
INSTALLLIB_DIR	= $(INSTALL_DIR)/lib
endif

###############################################################################
#
# define some common stuff
#

SHELL		:= /bin/sh

.SUFFIXES:	.cxx .prc 

# Directories

UNIX_INC_DIR	= $(PWLIBDIR)/include/ptlib/unix
UNIX_SRC_DIR	= $(PWLIBDIR)/src/ptlib/unix

PW_LIBDIR	= $(PWLIBDIR)/lib

# set name of the PT library
PTLIB_BASE	= pt_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PTLIB_FILE	= lib$(PTLIB_BASE)$(LIB_TYPE).$(LIB_SUFFIX)
PT_OBJBASE	= obj_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PT_OBJDIR	= $(PW_LIBDIR)/$(PT_OBJBASE)

# set name of the PW library (may not be used)
PWLIB_BASE	= pw_$(GUI_TYPE)_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PWLIB_FILE	= lib$(PWLIB_BASE)$(LIB_TYPE).$(LIB_SUFFIX)
PW_OBJBASE	= obj_$(GUI_TYPE)_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PW_OBJDIR	= $(PW_LIBDIR)/$(PW_OBJBASE)

###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#

ifdef	DEBUG

STDCCFLAGS	+= $(DEBUG_FLAG) -D_DEBUG -DPMEMORY_CHECK=1
LDFLAGS		+= $(DEBLDFLAGS)

else

OPTCCFLAGS	+= -O2 -DNDEBUG
#OPTCCFLAGS	+= -DP_USE_INLINES=1
#OPTCCFLAGS	+= -fconserve-space
ifneq ($(OSTYPE),macos)
# Apple does not support -s to remove symbol table/relocation information 
LDFLAGS		+= -s
endif

endif # DEBUG


# define OpenSSL variables if installed
ifdef  OPENSSLDIR
STDCCFLAGS	+= -DP_SSL -I$(OPENSSLDIR)/include -I$(OPENSSLDIR)/crypto
LDFLAGS		+= -L$(OPENSSLDIR)/lib
ENDLDLIBS	+= -lssl -lcrypto
endif


# define Posix threads stuff
ifdef P_PTHREADS
STDCCFLAGS	+= -DP_PTHREADS
endif


# compiler flags for all modes
STDCCFLAGS	+= -DPBYTE_ORDER=$(ENDIAN) -Wall
#STDCCFLAGS	+= -fomit-frame-pointer
#STDCCFLAGS	+= -fno-default-inline
#STDCCFLAGS     += -Woverloaded-virtual
#STDCCFLAGS     += -fno-implement-inlines

# add OS directory to include path
STDCCFLAGS	+= -I$(UNIX_INC_DIR)


# add library directory to library path and include the library
LDFLAGS		+= -L$(PW_LIBDIR)

LDLIBS		+= -l$(PTLIB_BASE)$(LIB_TYPE)

# End of unix.mak
