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
# $Revision$
# $Author$
# $Date$
#

####################################################

STANDARD_TARGETS=\
opt         debug         both \
optdepend   debugdepend   bothdepend \
optshared   debugshared   bothshared \
optnoshared debugnoshared bothnoshared \
optclean    debugclean    clean \
release

.PHONY: all $(STANDARD_TARGETS)


####################################################

# Set default for shared library usage

ifndef P_SHAREDLIB
P_SHAREDLIB=1
endif

# -Wall must be at the start of the options otherwise
# any -W overrides won't have any effect
ifeq ($(USE_GCC),yes)
PTLIB_CFLAGS += -Wall 
endif

ifdef RPM_OPT_FLAGS
PTLIB_CFLAGS	+= $(RPM_OPT_FLAGS)
endif

ifneq ($(target_os),rtems)
ifndef SYSINCDIR
SYSINCDIR := /usr/include
endif
endif


# Empty LD so getys set by appropriate platform below
LD=

####################################################

PTLIB_CFLAGS += -Wformat -Wformat-security

ifeq ($(target_os),linux)

ifeq ($(target_cpu),x86)
ifdef CPUTYPE
ifeq ($(CPUTYPE),crusoe)
PTLIB_CFLAGS	+= -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686 -malign-functions=0 
PTLIB_CFLAGS      += -malign-jumps=0 -malign-loops=0
else
PTLIB_CFLAGS	+= -mcpu=$(CPUTYPE)
endif
endif
endif

ifeq ($(target_cpu),ia64)
PTLIB_CFLAGS     += -DP_64BIT
endif

ifeq ($(target_cpu),hppa64)
PTLIB_CFLAGS     += -DP_64BIT
endif

ifeq ($(target_cpu),s390x)
PTLIB_CFLAGS     += -DP_64BIT
endif

ifeq ($(target_cpu),x86_64)
PTLIB_CFLAGS     += -DP_64BIT
LDLIBS		+= -lresolv
endif

ifeq ($(target_cpu),ppc64)
PTLIB_CFLAGS     += -DP_64BIT
endif

ifeq ($(P_SHAREDLIB),1)
ifndef PROG
PTLIB_CFLAGS	+= -fPIC -DPIC
endif # PROG
endif # P_SHAREDLIB


STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a

endif # linux

####################################################

ifeq ($(OSTYPE),gnu)

ifeq ($(MACHTYPE),x86)
ifdef CPUTYPE
ifeq ($(CPUTYPE),crusoe)
STDCCFLAGS	+= -fomit-frame-pointer -fno-strict-aliasing -fno-common -pipe -mpreferred-stack-boundary=2 -march=i686 -malign-functions=0
STDCCFLAGS      += -malign-jumps=0 -malign-loops=0
else
STDCCFLAGS	+= -mcpu=$(CPUTYPE)
endif
endif
endif

ifeq ($(MACHTYPE),ia64)
STDCCFLAGS     += -DP_64BIT
endif

ifeq ($(MACHTYPE),x86_64)
STDCCFLAGS     += -DP_64BIT
LDLIBS		+= -lresolv
endif

ifeq ($(P_SHAREDLIB),1)
ifndef PROG
STDCCFLAGS	+= -fPIC -DPIC
endif # PROG
endif # P_SHAREDLIB


STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a

endif # gnu

####################################################

ifeq ($(target_os),FreeBSD)

ifeq ($(target_cpu),x86)
ifdef CPUTYPE
PTLIB_CFLAGS	+= -mcpu=$(CPUTYPE)
endif
endif

ifeq ($(target_cpu),amd64)
PTLIB_CFLAGS     += -DP_64BIT
endif

P_USE_RANLIB		:= 1
#PTLIB_CFLAGS      += -DP_USE_PRAGMA		# migrated to configure

ifeq ($(P_SHAREDLIB),1)
ifndef PROG
PTLIB_CFLAGS	+= -fPIC -DPIC
endif # PROG
endif # P_SHAREDLIB

endif # FreeBSD


####################################################

ifeq ($(target_os),OpenBSD)

ifeq ($(target_cpu),x86)
#PTLIB_CFLAGS	+= -m486
endif

LDLIBS		+= -lossaudio

P_USE_RANLIB		:= 1
#PTLIB_CFLAGS      += -DP_USE_PRAGMA		# migrated to configure


endif # OpenBSD


####################################################

ifeq ($(target_os),NetBSD)

ifeq ($(target_cpu),x86)
ifdef CPUTYPE
PTLIB_CFLAGS	+= -mcpu=$(CPUTYPE)
endif
endif

ifeq ($(target_cpu),x86_64)
PTLIB_CFLAGS	+= -DP_64BIT
endif

P_USE_RANLIB		:= 1
#PTLIB_CFLAGS      += -DP_USE_PRAGMA		# migrated to configure

ifndef PROG
PTLIB_CFLAGS	+= -fPIC -DPIC
endif # PROG

endif # NetBSD


####################################################

ifeq ($(target_os),AIX)

PTLIB_CFLAGS	+= -DP_AIX  
# -pedantic -g
# LDLIBS	+= -lossaudio

PTLIB_CFLAGS	+= -mminimal-toc

#P_USE_RANLIB		:= 1
PTLIB_CFLAGS      += -DP_USE_PRAGMA


endif # AIX


####################################################

ifeq ($(target_os),sunos)

# Sparc Sun 4x, using gcc 2.7.2

P_USE_RANLIB		:= 1
REQUIRES_SEPARATE_SWITCH = 1
#PTLIB_CFLAGS      += -DP_USE_PRAGMA	# migrated to configure

endif # sunos


####################################################

ifeq ($(target_os),solaris)

#  Solaris (Sunos 5.x)

PTLIB_CFLAGS +=-DSOLARIS -D__inline=inline
CXXFLAGS +=-DSOLARIS -D__inline=inline

ifeq ($(target_cpu),x86)
ifeq ($(USE_GCC),yes)
DEBUG_FLAG	:= -gstabs+
PTLIB_CFLAGS           += -DUSE_GCC
CXXFLAGS         += -DUSE_GCC
endif
endif

ENDLDLIBS	+= -lsocket -lnsl -ldl -lposix4

# Sparc Solaris 2.x, using gcc 2.x
#Brian CC		:= gcc

#P_USE_RANLIB		:= 1

#PTLIB_CFLAGS      += -DP_USE_PRAGMA	# migrated to configure

STATIC_LIBS	:= libstdc++.a libg++.a 

# Rest added by jpd@louisiana.edu, to get .so libs created!
ifndef DEBUG
ifeq ($(P_SHAREDLIB),1)
ifndef PROG
PTLIB_CFLAGS	+= -fPIC -DPIC
endif # PROG
endif
endif

endif # solaris

####################################################

ifeq ($(target_os),irix)

#  should work whith Irix 6.5

# IRIX using a gcc
CC		:= gcc
PTLIB_CFLAGS	+= -DP_IRIX
LDLIBS		+= -lsocket -lnsl

PTLIB_CFLAGS      += -DP_USE_PRAGMA

endif # irix


####################################################

ifeq ($(target_os),beos)

SYSLIBS     += -lbe -lmedia -lgame -lroot -lsocket -lbind -ldl 
PTLIB_CFLAGS	+= -DBE_THREADS -DP_USE_PRAGMA -Wno-multichar -Wno-format
LDLIBS		+= $(SYSLIBS)

MEMORY_CHECK := 0
endif # beos


####################################################

ifeq ($(target_os),ultrix)

# R2000 Ultrix 4.2, using gcc 2.7.x
PTLIB_CFLAGS	+= -DP_ULTRIX
PTLIB_CFLAGS      += -DP_USE_PRAGMA
endif # ultrix


####################################################

ifeq ($(target_os),hpux)
PTLIB_CFLAGS      += -DP_USE_PRAGMA
# HP/UX 9.x, using gcc 2.6.C3 (Cygnus version)
PTLIB_CFLAGS	+= -DP_HPUX9

endif # hpux


####################################################
 
ifeq ($(target_os),Darwin)
 
# MacOS X or later / Darwin

ifeq ($(target_cpu),x86)
PTLIB_CFLAGS	+= -m486
endif

ARCHIVE      := libtool -static -o
P_USE_RANLIB := 0

endif # Darwin


####################################################

ifeq ($(target_os),VxWorks)

ifeq ($(target_cpu),ARM)
PTLIB_CFLAGS	+= -mcpu=arm8 -DCPU=ARMARCH4
endif

PTLIB_CFLAGS	+= -DP_VXWORKS -DPHAS_TEMPLATES -DVX_TASKS
PTLIB_CFLAGS	+= -DNO_LONG_DOUBLE

PTLIB_CFLAGS	+= -Wno-multichar -Wno-format

MEMORY_CHECK := 0

PTLIB_CFLAGS      += -DP_USE_PRAGMA

LD		= ld
LDFLAGS		+= --split-by-reloc 65535 -r 

endif # VxWorks

 
####################################################

ifeq ($(target_os),rtems)

SYSINCDIR	:= $(RTEMS_MAKEFILE_PATH)/lib/include

LDFLAGS		+= -B$(RTEMS_MAKEFILE_PATH)/lib/ -specs=bsp_specs -qrtems
PTLIB_CFLAGS	+= -B$(RTEMS_MAKEFILE_PATH)/lib/ -specs=bsp_specs -ansi -fasm -qrtems

ifeq ($(CPUTYPE),mcpu32)
PTLIB_CFLAGS	+= -mcpu32
LDFLAGS		+= -mcpu32 
endif

ifeq ($(CPUTYPE),mpc860)
PTLIB_CFLAGS	+= -mcpu=860
LDFLAGS		+= -mcpu=860
endif

PTLIB_CFLAGS	+= -DP_RTEMS -DP_HAS_SEMAPHORES

P_SHAREDLIB	:= 0

endif # rtems

####################################################

ifeq ($(target_os),QNX)

ifeq ($(target_cpu),x86)
PTLIB_CFLAGS	+= -Wc,-m486
endif

PTLIB_CFLAGS	+= -DP_QNX -DP_HAS_RECURSIVE_MUTEX=1 -DFD_SETSIZE=1024
LDLIBS		+= -lasound
ENDLDLIBS       += -lsocket -lstdc++

P_USE_RANLIB	:= 1
PTLIB_CFLAGS      += -DP_USE_PRAGMA

ifeq ($(P_SHAREDLIB),1)
ifeq ($(USE_GCC),yes)
PTLIB_CFLAGS      += -shared
else
ifeq ($(target_os),solaris)
PTLIB_CFLAGS      += -G
endif
endif

endif

endif # QNX6

####################################################

ifeq ($(target_os),Nucleus)

# Nucleus using gcc
PTLIB_CFLAGS	+= -msoft-float -nostdinc $(DEBUGFLAGS)
PTLIB_CFLAGS	+= -D__NUCLEUS_PLUS__ -D__ppc -DWOT_NO_FILESYSTEM -DPLUS \
		   -D__HAS_NO_FLOAT -D__USE_STL__ \
                   -D__USE_STD__ \
		   -D__NUCLEUS_NET__ -D__NEWLIB__ \
		   -DP_USE_INLINES=0
ifndef WORK
WORK		= ${HOME}/work
endif
ifndef NUCLEUSDIR
NUCLEUSDIR	= ${WORK}/embedded/os/Nucleus
endif
ifndef STLDIR
STLDIR		= ${WORK}/embedded/packages/stl-3.2-stream
endif
PTLIB_CFLAGS	+= -I$(NUCLEUSDIR)/plus \
		-I$(NUCLEUSDIR)/plusplus \
		-I$(NUCLEUSDIR)/net \
		-I$(NUCLEUSDIR) \
		-I$(WORK)/embedded/libraries/socketshim/BerkleySockets \
		-I${STLDIR} \
		-I/usr/local/powerpc-motorola-eabi/include \
		-I${WORK}/embedded/libraries/configuration

MEMORY_CHECK	=	0
endif # Nucleus

####################################################

#ifeq ($(target_os),mingw)
#LDFLAGS += -enable-runtime-pseudo-reloc -fatal-warning
#endif # mingw

###############################################################################
#
# Make sure some things are defined
#

ifndef INSTALL
INSTALL := install
endif

ifndef LD
LD = $(CXX)
endif

ifndef AR
AR := ar
endif

ifndef ARCHIVE
  ifdef P_USE_RANLIB
    ARCHIVE := $(AR) rc
  else
    ARCHIVE := $(AR) rcs
  endif
endif

ifndef RANLIB
RANLIB := ranlib
endif


# Further configuration

ifndef DEBUG_FLAG
DEBUG_FLAG	:=  -g
endif

ifndef OBJ_SUFFIX
ifdef	DEBUG
OBJ_SUFFIX	:= _d
else
OBJ_SUFFIX	:=
endif # DEBUG
endif # OBJ_SUFFIX

ifndef STATICLIBEXT
STATICLIBEXT = a
endif

ifeq ($(P_SHAREDLIB),1)
LIB_SUFFIX	= $(SHAREDLIBEXT)
LIB_TYPE	=
else   
LIB_SUFFIX	= a 
LIB_TYPE	= _s
endif # P_SHAREDLIB

ifndef OBJDIR_SUFFIX
OBJDIR_SUFFIX = $(OBJ_SUFFIX)$(LIB_TYPE)
endif


###############################################################################
#
# define some common stuff
#

ifneq ($(target_os),solaris)
SHELL		:= /bin/sh
else
SHELL           := /bin/bash
endif

.SUFFIXES:	.cxx .prc 

# Required macro symbols

# Directories

ifdef PTLIBDIR
  PTLIB_LIBDIR	= $(PTLIBDIR)/lib_$(target)
else
  PTLIB_LIBDIR	= $(shell pkg-config ptlib --variable=libdir)
endif

# set name of the PT library
PTLIB_BASE	= pt$(OBJ_SUFFIX)
PTLIB_FILE	= lib$(PTLIB_BASE)$(LIB_TYPE).$(LIB_SUFFIX)
PTLIB_DEBUG_FILE= lib$(PTLIB_BASE)_d$(LIB_TYPE).$(LIB_SUFFIX)
PTLIB_OBJBASE	= obj$(OBJDIR_SUFFIX)
PTLIB_OBJDIR	= $(PTLIB_LIBDIR)/$(PTLIB_OBJBASE)

ifeq (,$(findstring $(target_os),Darwin cygwin mingw))
  PTLIB_SONAME = $(PTLIB_FILE).$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER)
  PTLIB_DEBUG_SONAME = $(PTLIB_DEBUG_FILE).$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER)
else
  PTLIB_SONAME = $(subst .$(LIB_SUFFIX),.$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER).$(LIB_SUFFIX),$(PTLIB_FILE))
  PTLIB_DEBUG_SONAME = $(subst .$(LIB_SUFFIX),.$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER).$(LIB_SUFFIX),$(PTLIB_DEBUG_FILE))
endif


###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#

ifdef	DEBUG

ifndef MEMORY_CHECK
MEMORY_CHECK := 1
endif

PTLIB_CFLAGS	+= $(DEBUG_FLAG) -D_DEBUG
LDFLAGS		+= $(DEBLDFLAGS)

else

PTLIB_CFLAGS	+= -DNDEBUG -D_FORTIFY_SOURCE=2

ifneq ($(target_os),Darwin)
  ifeq ($(target_os),solaris)
    ifeq ($(USE_GCC),yes)
      PTLIB_CFLAGS	+= -O3
    else
      PTLIB_CFLAGS	+= -xO3
    endif
  else
    PTLIB_CFLAGS	+= -Os 
  endif
else
  PTLIB_CFLAGS	+= -O2
endif

endif # DEBUG

# define ESDDIR variables if installed
ifdef  ESDDIR
PTLIB_CFLAGS	+= -I$(ESDDIR)/include -DUSE_ESD=1
ENDLDLIBS	+= $(ESDDIR)/lib/libesd.a  # to avoid name conflicts
HAS_ESD		= 1
endif

# feature migrated to configure.in
# #define templates if available
# ifndef NO_PTLIB_TEMPLATES
# PTLIB_CFLAGS	+= -DPHAS_TEMPLATES
# endif

# compiler flags for all modes
#PTLIB_CFLAGS	+= -fomit-frame-pointer
#PTLIB_CFLAGS	+= -fno-default-inline
#PTLIB_CFLAGS     += -Woverloaded-virtual
#PTLIB_CFLAGS     += -fno-implement-inlines


# add library directory to library path and include the library
LDFLAGS		+= -L$(PTLIB_LIBDIR)

LDLIBS		+= -l$(PTLIB_BASE)$(LIB_TYPE)

# End of unix.mak
