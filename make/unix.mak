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
# Revision 1.146  2002/11/22 10:14:07  robertj
# QNX port, thanks Xiaodan Tang
#
# Revision 1.145  2002/11/20 02:51:53  robertj
# Fixed endian-ness on sparc OpenBSD
#
# Revision 1.144  2002/11/12 11:56:46  rogerh
# Reinstate IPv6 support on the BSD platforms.
#
# Revision 1.143  2002/11/03 16:07:19  rogerh
# Remove IPV6 from FreeBSD until debugging is completed.
#
# Revision 1.142  2002/11/03 08:10:24  rogerh
# Add IPV6 check using header files. This works on FreeBSD.
#
# Revision 1.141  2002/11/02 00:48:58  robertj
# Changed test for IPv6, old test picked up old versions which are no good.
#
# Revision 1.140  2002/11/01 23:55:52  robertj
# Added automatic inclusion of IPv6 if present in system.
#
# Revision 1.139  2002/10/17 13:44:27  robertj
# Port to RTEMS, thanks Vladimir Nesic.
#
# Revision 1.138  2002/10/10 04:43:44  robertj
# VxWorks port, thanks Martijn Roest
#
# Revision 1.137  2002/10/03 04:12:45  robertj
# Allowed for locally built (uninstalled) openssl library.
#
# Revision 1.136  2002/09/16 01:08:59  robertj
# Added #define so can select if #pragma interface/implementation is used on
#   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
#
# Revision 1.135  2002/09/09 06:49:19  robertj
# Removed explicit run path, should use environment, thanks Nils Bokermann
#
# Revision 1.134  2002/08/21 00:00:31  dereks
# Patches from Ryutaroh, to improve firewire (linux only) support. Many thanks.
#
# Revision 1.133  2002/07/18 13:18:22  rogerh
# The patch to set big endian for linux on the sparc (by Kawahara Taro) was
# incorrectly added in version 1.91 and it made Alpha big endian instead. The
# patch is now in correctly. Alpha problem reported by Robert M. Riches Jr.
#
# Revision 1.132  2002/06/20 05:49:04  robertj
# Fixed typo to include templates by default.
#
# Revision 1.131  2002/06/14 11:14:30  rogerh
# Detect more cpu types. Submitted by Klaus Kaempf <kkaempf@suse.de>
#
# Revision 1.130  2002/06/13 10:01:20  rogerh
# We no longer need the /usr/include/g++/backward include path
#
# Revision 1.129  2002/06/10 08:37:58  rogerh
# Add instructions for compiling on FreeBSD 5.x with GCC 3.1
#
# Revision 1.128  2002/06/06 09:28:10  robertj
# Changed default build to use C++ templates, use NO_PWLIB_TEMPLATES to disable.
#
# Revision 1.127  2002/05/31 06:37:22  robertj
# Added PWLIB_TEMPLATES environment variable to compile using GCC templates.
#
# Revision 1.126  2002/05/07 01:57:51  craigs
# Always link with -ldl, not just when using shared libraries
#
# Revision 1.125  2002/04/18 05:12:20  robertj
# Changed /usr/include to SYSINCDIR helps with X-compiling, thanks Bob Lindell
#
# Revision 1.124  2002/04/09 02:30:18  robertj
# Removed GCC3 variable as __GNUC__ can be used instead, thanks jason Spence
#
# Revision 1.123  2002/04/02 15:07:02  rogerh
# Specify path to expat header and library if expat is in /usr/local.
# Required for FreeBSD. Reported by Vlad Marchenko <vlad@infonet.com.ua>
#
# Revision 1.122  2002/03/15 01:14:12  robertj
# Added search for expat library in /usr/local as well as /usr
#
# Revision 1.121  2002/02/25 22:58:04  robertj
# Moved GCC 3 version check to after CPLUS is assured to be defined.
#
# Revision 1.120  2002/02/25 19:51:18  dereks
# Update Firewire test routine. Thanks Ryutaroh
#
# Revision 1.119  2002/02/25 16:23:16  rogerh
# Test for GCC 3 in unix.mak and not it common.mak so -DGCC3 can be set
#
# Revision 1.118  2002/02/20 02:37:26  dereks
# Initial release of Firewire camera support for linux.
# Many thanks to Ryutaroh Matsumoto <ryutaroh@rmatsumoto.org>.
#
# Revision 1.117  2002/01/31 07:25:29  robertj
# Backed out someones changes which did not include platform in library name
#   thus making it impossible to have multiple platform builds using the
#   same NFS mounted directories.
#
# Revision 1.116  2002/01/28 00:19:59  craigs
# Made gcc 3 changes dependent on having gcc 3.0 installed
#
# Revision 1.115  2002/01/26 23:57:08  craigs
# Changed for GCC 3.0 compatibility, thanks to manty@manty.net
#
# Revision 1.114  2002/01/26 00:22:27  robertj
# Normalised MACHTYPE for sparc processors
#
# Revision 1.113  2002/01/15 12:17:53  craigs
# Changed Solaris flags to make fd set size 4096.
# This allows more OpenH323 connections under Solaris
# Thanks to johan.gnosspelius@pipebeach.com
#
# Revision 1.112  2001/12/17 23:33:50  robertj
# Solaris 8 porting changes, thanks James Dugal
#
# Revision 1.111  2001/12/06 06:27:35  craigs
# Added P_EXPAT flag
#
# Revision 1.110  2001/12/06 05:38:25  craigs
# Added detection of expat XML parser library
#
# Revision 1.109  2001/12/05 06:26:17  rogerh
# Make Darwin and Carbon use only static libaries.
#
# Revision 1.108  2001/11/27 22:42:13  robertj
# Changed to make system to better support non-shared library building.
#
# Revision 1.107  2001/11/25 23:47:05  robertj
# Changed sense of HAS_VIDEO_CAPTURE to NO_VIDEO_CAPTURE to reduce cmd line.
#
# Revision 1.106  2001/11/25 23:28:04  robertj
# Fixed correct setting of HAS_VIDEO_CAPTURE compile flag
#
# Revision 1.105  2001/10/31 00:45:20  robertj
# Added debuglibs, optlibs and bothlibs targets, moving help to where these
#   targets are in teh make file system.
#
# Revision 1.104  2001/10/11 02:20:54  robertj
# Added IRIX support (no audio/video), thanks Andre Schulze.
#
# Revision 1.103  2001/10/09 08:58:33  robertj
# Added "make help" target for displaying the usual build targets, is default
#   target unless overridden by applications Makefile
#
# Revision 1.102  2001/08/11 08:04:06  rogerh
# Add Mac OS Carbon changes from John Woods <jfw@jfwhome.funhouse.com>
#
# Revision 1.101  2001/08/06 19:27:24  rogerh
# Determine the OS Version for OpenBSD.
# Submitted by Marius Aamodt Eriksen <marius@umich.edu>
#
# Revision 1.100  2001/08/06 03:18:44  robertj
# Added better checking for openssl usage.
#
# Revision 1.99  2001/08/03 04:17:27  dereks
# Add options for "CPUTYPE=crusoe", which is helpful for sony vaio notebook
#
# Revision 1.98  2001/08/02 03:23:36  robertj
# Fixed exporting of new CPUTYPE variable so works in nested builds
#
# Revision 1.97  2001/08/02 03:02:27  robertj
# Allowed the actual CPU type to be passed to the code generator instead of
#   always using -m486. Use the CPUTYPE variable to override.
#
# Revision 1.96  2001/07/30 07:45:54  robertj
# Added "all" target with double colon.
#
# Revision 1.95  2001/07/19 09:27:12  rogerh
# Add support for EsounD and esd (the Enlightenment Sound Daemon).
# This allows OhPhone to run on platforms where EsounD and esd have been
# ported which includes Mac OS X.
# Code written by Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>.
#
# Revision 1.94  2001/07/09 06:16:15  yurik
# Jac Goudsmit's BeOS changes of July,6th. Cleaning up media subsystem etc.
#
# Revision 1.93  2001/06/30 06:59:06  yurik
# Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
#
# Revision 1.92  2001/05/29 03:32:53  craigs
# Added additional checks for OpenSSL
#
# Revision 1.91  2001/04/02 00:09:44  robertj
# Added big endian flag for Linux on sparc, thanks Kawahara Taro.
#
# Revision 1.90  2001/03/29 04:50:41  robertj
# Added STANDARD_TARGETS macro for all standard targets such as opt debug etc
# Changed "all" target for first target to be "default_target" so use makefile
# can have "all" as a target
#
# Revision 1.89  2001/03/07 06:55:27  yurik
# Changed email to current one
#
# Revision 1.88  2001/02/24 13:25:58  rogerh
# Turn on pthread support for Mac OS X / Darwin and rename the object
# directory from macos to Darwin
#
# Revision 1.87  2001/02/14 14:01:24  rogerh
# Handle uname of 'darwin' for Macos X machines
#
# Revision 1.86  2001/02/09 12:29:41  robertj
# Changed to allow for object directory suffix to be different from library.
#
# Revision 1.85  2001/01/16 19:19:02  rogerh
# Set BE_THREADS to 1 instead of 0 for consistency
#
# Revision 1.84  2001/01/16 14:21:02  rogerh
# Move -Wall to start of STDCCFLAGS so it can be overridden later.
# Make some BeOS changes. All submitted by Jac Goudsmit.
#
# Revision 1.83  2000/06/26 11:17:20  robertj
# Nucleus++ port (incomplete).
#
# Revision 1.82  2000/06/21 01:01:21  robertj
# AIX port, thanks Wolfgang Platzer (wolfgang.platzer@infonova.at).
#
# Revision 1.81  2000/05/18 22:23:21  rogerh
# Fix RANLIB usage on the BSD machines. RANLIB can now be used to
# specify the ranlib executable. P_USE_RANLIB must now be set to
# enable ranlib usage.
#
# Revision 1.80  2000/05/18 08:33:12  robertj
# Added variables for standard programs ar and ranlib
#
# Revision 1.79  2000/05/05 10:42:04  robertj
# Fixed support for older FreeBSD (location of sysctrl).
#
# Revision 1.78  2000/04/19 00:13:52  robertj
# BeOS port changes.
#
# Revision 1.77  2000/04/11 21:22:08  robertj
# Made CC compiler explicitly gcc on Solaris systems.
#
# Revision 1.76  2000/04/11 15:14:33  rogerh
# Tidy up NetBSD comments
#
# Revision 1.75  2000/04/10 16:18:16  rogerh
# More NetBSD support changes
#
# Revision 1.74  2000/04/10 11:36:16  rogerh
# Add NetBSD pthreads support
#
# Revision 1.73  2000/04/10 06:45:59  rogerh
# NetBSD needs the ossaudio liobrary
#
# Revision 1.72  2000/04/09 18:29:02  rogerh
# Add my NetBSD changes
#
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
# BeOS port effort by Yuri Kiryanov <openh323@kiryanov.com>
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

ifneq (,$(findstring $(OSTYPE),IRIX))
OSTYPE := irix
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
OSTYPE := Darwin
endif

ifneq (,$(findstring darwin,$(OSTYPE)))
OSTYPE := Darwin
endif

ifneq (,$(findstring AIX,$(OSTYPE)))
MACHTYPE := ppc
endif

ifneq (,$(findstring $(OS),VXWORKS))
OSTYPE := VxWorks
endif

ifneq (,$(findstring netbsd,$(OSTYPE)))
OSTYPE := NetBSD
endif

ifneq (,$(findstring sparc, $(MACHTYPE)))
MACHTYPE := sparc
endif

ifneq (,$(findstring sun4, $(MACHTYPE)))
MACHTYPE := sparc
endif

ifneq (,$(findstring i86, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring i386, $(MACHTYPE)))
MACHTYPE := x86
endif

ifneq (,$(findstring i486, $(MACHTYPE)))
MACHTYPE := x86
POSSIBLE_CPUTYPE := i486
endif

ifneq (,$(findstring i586, $(MACHTYPE)))
MACHTYPE := x86
POSSIBLE_CPUTYPE := i586
endif

ifneq (,$(findstring i686, $(MACHTYPE)))
MACHTYPE := x86
POSSIBLE_CPUTYPE := i686
endif

#make sure x86 does not match x86_64 by mistake
ifneq (,$(findstring x86, $(MACHTYPE)))
ifneq (,$(findstring x86_64, $(MACHTYPE)))
MACHTYPE := x86_64
else
MACHTYPE := x86
endif
endif

ifneq (,$(findstring powerpc, $(MACHTYPE)))
MACHTYPE := ppc
endif

ifneq (,$(findstring mips, $(MACHTYPE)))
MACHTYPE := mips
endif

ifneq (,$(findstring alpha, $(MACHTYPE)))
MACHTYPE := alpha
endif

ifneq (,$(findstring ppc, $(MACHTYPE)))
MACHTYPE := ppc
endif

ifneq (,$(findstring sparc, $(MACHTYPE)))
MACHTYPE := sparc
endif

ifneq (,$(findstring ia64, $(MACHTYPE)))
MACHTYPE := ia64
endif

ifneq (,$(findstring s390, $(MACHTYPE)))
ifneq (,$(findstring s390x, $(MACHTYPE)))
MACHTYPE := s390x
else
MACHTYPE := s390
endif
endif

ifneq (,$(findstring armv4l, $(MACHTYPE)))
MACHTYPE := armv4l
endif
ifndef CPUTYPE
CPUTYPE := $(POSSIBLE_CPUTYPE)
export CPUTYPE
endif



STANDARD_TARGETS=\
opt         debug         both \
optdepend   debugdepend   bothdepend \
optshared   debugshared   bothshared \
optnoshared debugnoshared bothnoshared \
optclean    debugclean    clean \
release tagbuild

.PHONY: all $(STANDARD_TARGETS)


ifeq (,$(findstring $(OSTYPE),linux FreeBSD OpenBSD NetBSD solaris beos Darwin Carbon AIX Nucleus VxWorks rtems QNX))

default_target :
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
	@echo "              FreeBSD OpenBSD NetBSD beos Darwin Carbon"
	@echo "              VxWorks rtems"
	@echo
	@echo "              **********************************"
	@echo "              *** DO NOT IGNORE THIS MESSAGE ***"
	@echo "              **********************************"
	@echo
	@echo "         The system almost certainly will not compile! When you get"
	@echo "         it working please send patches to support@equival.com.au"
	@echo ######################################################################
	@echo

$(STANDARD_TARGETS) :: default_target

else

default_target : help

endif

####################################################

# Set default for shared library usage

ifndef P_SHAREDLIB
ifndef DEBUG
P_SHAREDLIB=1
else
P_SHAREDLIB=0
endif
endif

# -Wall must be at the start of the options otherwise
# any -W overrides won't have any effect
STDCCFLAGS += -Wall


ifneq ($(OSTYPE),rtems)
ifndef SYSINCDIR
SYSINCDIR := /usr/include
endif
endif

####################################################

ifeq ($(OSTYPE),linux)

# i486 Linux for x86, using gcc 2.7.2
STDCCFLAGS	+= -DP_LINUX


# Enable pthreads if we are using glibc 6
ifneq (,$(shell grep define.\*__GNU_LIBRARY__.\*6 $(SYSINCDIR)/features.h))
P_PTHREADS	= 1
else
ifndef PTLIB_ALT
PTLIB_ALT = libc5
endif
endif

ifeq (,$(wildcard $(SYSINCDIR)/linux/videodev.h))
STDCCFLAGS	+= -DNO_VIDEO_CAPTURE
endif


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

ifeq ($(MACHTYPE),alpha)
STDCCFLAGS	+= -DP_64BIT
endif

ifeq ($(MACHTYPE),ia64)
STDCCFLAGS     += -DP_64BIT
endif

ifeq ($(MACHTYPE),s390x)
STDCCFLAGS     += -DP_64BIT
endif

ifeq ($(MACHTYPE),x86_64)
STDCCFLAGS     += -DP_64BIT
endif

ifeq ($(MACHTYPE),ppc)
ENDIAN		:= PBIG_ENDIAN
endif

ifeq ($(MACHTYPE),sparc)
ENDIAN		:= PBIG_ENDIAN
endif

ifneq ($(findstring $(MACHTYPE),s390 s390x),)
ENDIAN		:= PBIG_ENDIAN
endif

ifeq ($(MACHTYPE),ARM)
ENDIAN		:= PLITTLE_ENDIAN
endif

ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
STDCCFLAGS	+= -D_REENTRANT -DP_HAS_SEMAPHORES
endif

LDLIBS		+= -ldl
ifeq ($(P_SHAREDLIB),1)
ifndef PROG
STDCCFLAGS	+= -fPIC
endif # PROG
endif # P_SHAREDLIB

ifdef TRY_1394DC
ifneq (,$(wildcard $(SYSINCDIR)/libdc1394/dc1394_control.h))
ifneq (,$(shell grep drop_frames $(SYSINCDIR)/libdc1394/dc1394_control.h))
ENDLDLIBS      += -lraw1394 -ldc1394_control
STDCCFLAGS     += -DTRY_1394DC
TRY_1394DC     =  1
else
$(error "Libdc1394 is installed but its version is older than required. The 1394 camera module will not be compiled.")
TRY_1394DC     =
endif
else
$(error "TRY_1394DC is defined but $(SYSINCDIR)/libdc1394/dc1394_control.h does not exist.")
endif
endif


STATIC_LIBS	:= libstdc++.a libg++.a libm.a libc.a
SYSLIBDIR	:= /usr/lib
#LDFLAGS		+= --no-whole-archive --cref
STDCCFLAGS      += -DP_USE_PRAGMA


endif # linux


####################################################

ifeq ($(OSTYPE),FreeBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
ifdef CPUTYPE
STDCCFLAGS	+= -mcpu=$(CPUTYPE)
endif
endif

ifndef OSRELEASE
OSRELEASE	:= $(shell sysctl -n kern.osreldate)
endif

STDCCFLAGS	+= -DP_FREEBSD=$(OSRELEASE)

ifdef P_PTHREADS
CFLAGS	+= -pthread
endif

P_USE_RANLIB		:= 1
STDCCFLAGS      += -DP_USE_PRAGMA


endif # FreeBSD


####################################################

ifeq ($(OSTYPE),OpenBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
endif

ifeq ($(MACHTYPE),sparc)
ENDIAN          := PBIG_ENDIAN
endif

LDLIBS		+= -lossaudio

ifndef OSRELASE
OSRELEASE	:= $(shell sysctl -n kern.osrevision)
endif
 
STDCCFLAGS	+= -DP_OPENBSD=$(OSRELEASE)

ifdef P_PTHREADS
CFLAGS	+= -pthread
endif

P_USE_RANLIB		:= 1
STDCCFLAGS      += -DP_USE_PRAGMA


endif # OpenBSD


####################################################

ifeq ($(OSTYPE),NetBSD)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
endif

STDCCFLAGS	+= -DP_NETBSD
LDLIBS		+= -lossaudio

ifdef P_PTHREADS
STDCCFLAGS += -I/usr/pkg/pthreads/include
LDFLAGS	+= -L/usr/pkg/pthreads/lib
LDLIBS	+= -lpthread
CC              := /usr/pkg/pthreads/bin/pgcc
CPLUS           := /usr/pkg/pthreads/bin/pg++
endif

P_USE_RANLIB		:= 1
STDCCFLAGS      += -DP_USE_PRAGMA


endif # NetBSD


####################################################

ifeq ($(OSTYPE),AIX)

P_PTHREADS	:= 1

STDCCFLAGS	+= -DP_AIX  
# -pedantic -g
# LDLIBS		+= -lossaudio

#ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
STDCCFLAGS	+= -D_REENTRANT 
#-DP_HAS_SEMAPHORES
#endif


STDCCFLAGS	+= -mminimal-toc


ifdef P_PTHREADS
#CFLAGS	+= -pthread
endif

#P_USE_RANLIB		:= 1
STDCCFLAGS      += -DP_USE_PRAGMA


endif # AIX


####################################################

ifeq ($(OSTYPE),sunos)

# Sparc Sun 4x, using gcc 2.7.2

P_USE_RANLIB		:= 1
REQUIRES_SEPARATE_SWITCH = 1
STDCCFLAGS      += -DP_USE_PRAGMA


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

# Sparc Solaris 2.x, using gcc 2.x
CC		:= gcc
STDCCFLAGS	+= -DP_SOLARIS=$(OSRELEASE)
LDLIBS		+= -lsocket -lnsl -ldl -lposix4

#P_USE_RANLIB		:= 1

STDCCFLAGS      += -DP_USE_PRAGMA

STATIC_LIBS	:= libstdc++.a libg++.a 
SYSLIBDIR	:= /opt/openh323/lib

ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
# extend fd set size to 4096 to allow more connections,
# from johan.gnosspelius@pipebeach.com
STDCCFLAGS	+= -D_REENTRANT -DFD_SETSIZE=4096
endif

# Rest added by jpd@louisiana.edu, to get .so libs created!
ifndef DEBUG
ifndef P_SHAREDLIB
P_SHAREDLIB=1
ifndef PROG
STDCCFLAGS	+= -fPIC
endif # PROG
endif
endif

endif # solaris

####################################################

ifeq ($(OSTYPE),irix)

#  should work whith Irix 6.5

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),mips)
ENDIAN		:= PBIG_ENDIAN
endif

# IRIX using a gcc
CC		:= gcc
STDCCFLAGS	+= -DP_IRIX
LDLIBS		+= -lsocket -lnsl -ldl

ifdef P_PTHREADS
ENDLDLIBS	+= -lpthread
STDCCFLAGS	+= -D_REENTRANT
endif

STDCCFLAGS      += -DP_USE_PRAGMA

endif # irix


####################################################

ifeq ($(OSTYPE),beos)

BE_THREADS := 1

# Uncomment the next line if you have the
# BeOS Network Environment (BONE) installed.
# If you run a standard R5 install, comment it out.
# NOTE: support for compiling without BONE will likely
# be droppped.
#BE_BONE := 1

# Uncomment the next line if you have the
# Media Kit Update installed (probably you don't, unless
# you are a Be Inc. employee or a registered Beta tester).
# The media kit update has an extended BMediaRecorder
# implemented in the library. For the OpenH323 project,
# it just means that it's possible to get a list of
# recording devices, which is something that's hard to
# implement with the MediaRecorder class that's included
# in pwlib.
#MEDIA_KIT_UPDATE := 1

# Be Inc. is working on a version of OpenSSL that's integrated
# in the operating system.
#BE_OPENSSL := 1

ifdef BE_OPENSSL
STDCCFLAGS  += -DP_SSL
SYSLIBS     += -lopenssl
HAS_OPENSSL = 1
endif

SYSLIBS     += -lbe -lmedia -lgame -lroot

ifdef BE_THREADS
STDCCFLAGS	+= -DBE_THREADS -DP_PLATFORM_HAS_THREADS
endif

STDCCFLAGS	+= -Wno-multichar -Wno-format

ifdef BE_BONE
SYSLIBS		+= -lsocket -lbind
else
SYSLIBS     += -lnet
STDCCFLAGS  += -DBE_BONELESS
endif

ifdef MEDIA_KIT_UPDATE
STDCCFLAGS  += -DMEDIA_KIT_UPDATE
endif

LDLIBS		+= $(SYSLIBS)

MEMORY_CHECK := 0

ifdef PROFILE
STDCCFLAGS += -p
LDFLAGS += -p
endif

STDCCFLAGS      += -DP_USE_PRAGMA

endif # beos


####################################################

ifeq ($(OSTYPE),ultrix)

ENDIAN	:= PBIG_ENDIAN

# R2000 Ultrix 4.2, using gcc 2.7.x
STDCCFLAGS	+= -DP_ULTRIX
STDCCFLAGS      += -DP_USE_PRAGMA
endif # ultrix


####################################################

ifeq ($(OSTYPE),hpux)
STDCCFLAGS      += -DP_USE_PRAGMA
# HP/UX 9.x, using gcc 2.6.C3 (Cygnus version)
STDCCFLAGS	+= -DP_HPUX9

endif # hpux


####################################################
 
ifeq ($(OSTYPE),Darwin)
 
# MacOS X or later / Darwin
 
STDCCFLAGS      += -DNO_LONG_DOUBLE -DP_MACOSX
ENDLDLIBS       += -framework CoreServices -framework CoreAudio
P_MACOSX	:= 1

# To compile using esound. Uncomment the ESDDIR line, and
# enter the base path where esound is installed.
# For Fink users, the base is /sw
# For others that compiled the esound package without fink,
# it typically is /usr/local

#ESDDIR =/sw

# Quicktime support is still a long way off. But for development purposes,
# I am inluding the flags to allow QuickTime to be linked.
# Uncomment them if you wish, but it will do nothing for the time being.

#HAS_QUICKTIMEX := 1
#STDCCFLAGS     += -DHAS_QUICKTIMEX
#ENDLDLIBS      += -framework QuickTime
 
P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -m486
else
ENDIAN		:= PBIG_ENDIAN
endif

P_SHAREDLIB	:= 0 
P_USE_RANLIB	:= 1

CC              := cc
CPLUS           := c++
 
endif # Darwin

ifeq ($(OSTYPE),Carbon)

# MacOS 9 or X using Carbonlib calls

STDCCFLAGS	+= -DP_MACOS

# I'm having no end of trouble with the debug memory allocator.
MEMORY_CHECK    := 0

# Carbon is only available for full Mac OS X, not pure Darwin, so the only
# currently available architecture is PPC.
ENDIAN		:= PBIG_ENDIAN
P_MAC_MPTHREADS := 1
STDCCFLAGS	+= -DP_MAC_MPTHREADS -DP_PLATFORM_HAS_THREADS
# stupid Projct Builder compiler
STDCCFLAGS	+= -DNO_LONG_DOUBLE
# 
LDLIBS		+= -prebind -framework CoreServices -framework QuickTime -framework Carbon
  
P_SHAREDLIB	:= 0 
P_USE_RANLIB	:= 1

CC              := cc
CPLUS           := c++
 
endif # Carbon

####################################################

ifeq ($(OSTYPE),VxWorks)

ifeq ($(MACHTYPE),ARM)
STDCCFLAGS	+= -mcpu=arm8 -DCPU=ARMARCH4
endif

STDCCFLAGS	+= -DP_VXWORKS -DPHAS_TEMPLATES -DVX_TASKS
STDCCFLAGS	+= -DNO_LONG_DOUBLE

STDCCFLAGS	+= -Wno-multichar -Wno-format

MEMORY_CHECK := 0

STDCCFLAGS      += -DP_USE_PRAGMA

endif # VxWorks

 
####################################################

ifeq ($(OSTYPE),rtems)

CC              := $(MACHTYPE)-rtems-gcc --pipe
CPLUS           := $(MACHTYPE)-rtems-g++
#LD              := $(MACHTYPE)-rtems-ld
#AR              := $(MACHTYPE)-rtems-ar
#RUNLIB          := $(MACHTYPE)-rtems-runlib

SYSLIBDIR	:= $(RTEMS_MAKEFILE_PATH)/lib
SYSINCDIR	:= $(RTEMS_MAKEFILE_PATH)/lib/include

LDFLAGS		+= -B$(SYSLIBDIR)/ -specs=bsp_specs -qrtems
STDCCFLAGS	+= -B$(SYSLIBDIR)/ -specs=bsp_specs -ansi -fasm -qrtems

ifeq ($(CPUTYPE),mcpu32)
STDCCFLAGS	+= -mcpu32
LDFLAGS		+= -mcpu32 
endif

STDCCFLAGS	+= -DP_RTEMS -DNO_VIDEO_CAPTURE -DP_HAS_SEMAPHORES

P_SHAREDLIB	:= 0
P_PTHREADS	:= 1

endif # rtems

####################################################

ifeq ($(OSTYPE),QNX)

P_PTHREADS	:= 1

ifeq ($(MACHTYPE),x86)
STDCCFLAGS	+= -Wc,-m486
endif

STDCCFLAGS	+= -DP_QNX -DP_HAS_RECURSIVE_MUTEX -DFD_SETSIZE=1024
LDLIBS		+= -lasound
ENDLDLIBS       += -lsocket -lstdc++

CC		:= qcc -Vgcc_ntox86
CPLUS		:= qcc -Vgcc_ntox86_gpp

P_USE_RANLIB	:= 1
STDCCFLAGS      += -DP_USE_PRAGMA

ifeq ($(P_SHAREDLIB),1)
STDCCFLAGS      += -shared
OPTCCFLAGS      += -shared
endif

endif # QNX6


###############################################################################
#
# Make sure some things are defined
#

ifndef	CC
CC := gcc
endif

ifndef CPLUS
CPLUS := g++
endif

ifndef INSTALL
INSTALL := install
endif

ifndef AR
AR := ar
endif

ifndef RANLIB
RANLIB := ranlib
endif


# Further configuration

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

ifndef OBJDIR_SUFFIX
OBJDIR_SUFFIX = $(OBJ_SUFFIX)
endif

ifeq ($(P_SHAREDLIB),1)
LIB_SUFFIX	= so
else
LIB_SUFFIX	= a
ifndef DEBUG
LIB_TYPE	= _s
endif
endif # P_SHAREDLIB

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

####################################################

ifeq ($(OSTYPE),Nucleus)

# Nucleus using gcc
STDCCFLAGS	+= -msoft-float -nostdinc -g
STDCCFLAGS	+= -D__NUCLEUS_PLUS__ -D__ppc -DWOT_NO_FILESYSTEM -DPLUS \
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
STDCCFLAGS	+= -I$(NUCLEUSDIR)/plus \
		-I$(NUCLEUSDIR)/plusplus \
		-I$(NUCLEUSDIR)/net \
		-I$(NUCLEUSDIR) \
		-I$(PWLIBDIR)/include/ptlib/Nucleus++ \
		-I$(WORK)/embedded/libraries/socketshim/BerkleySockets \
		-I${STLDIR} \
		-I/usr/local/powerpc-motorola-eabi/include \
		-I${WORK}/embedded/libraries/configuration

UNIX_SRC_DIR	= $(PWLIBDIR)/src/ptlib/Nucleus++
ENDIAN		= PBIG_ENDIAN
MEMORY_CHECK	=	0
endif # Nucleus


###############################################################################
#
# define some common stuff
#

SHELL		:= /bin/sh

.SUFFIXES:	.cxx .prc 

# Required macro symbols

# Directories

UNIX_INC_DIR	= $(PWLIBDIR)/include/ptlib/unix
ifndef UNIX_SRC_DIR
UNIX_SRC_DIR	= $(PWLIBDIR)/src/ptlib/unix
endif

PW_LIBDIR	= $(PWLIBDIR)/lib

# set name of the PT library
PTLIB_BASE	= pt_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PTLIB_FILE	= lib$(PTLIB_BASE)$(LIB_TYPE).$(LIB_SUFFIX)
PT_OBJBASE	= obj_$(PLATFORM_TYPE)_$(OBJDIR_SUFFIX)
PT_OBJDIR	= $(PW_LIBDIR)/$(PT_OBJBASE)

# set name of the PW library (may not be used)
PWLIB_BASE	= pw_$(GUI_TYPE)_$(PLATFORM_TYPE)_$(OBJ_SUFFIX)
PWLIB_FILE	= lib$(PWLIB_BASE)$(LIB_TYPE).$(LIB_SUFFIX)
PW_OBJBASE	= obj_$(GUI_TYPE)_$(PLATFORM_TYPE)_$(OBJDIR_SUFFIX)
PW_OBJDIR	= $(PW_LIBDIR)/$(PW_OBJBASE)

###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#

ifdef	DEBUG

ifndef MEMORY_CHECK
MEMORY_CHECK := 1
endif

STDCCFLAGS	+= $(DEBUG_FLAG) -D_DEBUG -DPMEMORY_CHECK=$(MEMORY_CHECK)
LDFLAGS		+= $(DEBLDFLAGS)

else

OPTCCFLAGS	+= -O3 -DNDEBUG
#OPTCCFLAGS	+= -DP_USE_INLINES=1
#OPTCCFLAGS	+= -fconserve-space
ifneq ($(OSTYPE),Carbon)
ifneq ($(OSTYPE),Darwin)
# Apple does not support -s to remove symbol table/relocation information 
LDFLAGS		+= -s
endif
endif

endif # DEBUG


# define OpenSSL variables if installed
ifndef OPENSSLDIR

ifneq (,$(wildcard $(SYSINCDIR)/openssl))
OPENSSLDIR := $(SYSINCDIR)
export OPENSSLDIR
endif

ifneq ($(OSTYPE),rtems)
ifneq (,$(wildcard /usr/local/ssl))
OPENSSLDIR := /usr/local/ssl
export OPENSSLDIR
endif
endif

endif

ifdef  OPENSSLDIR
ifneq (,$(wildcard $(OPENSSLDIR)))
STDCCFLAGS	+= -DP_SSL -I$(OPENSSLDIR)/include -I$(OPENSSLDIR)/crypto
LDFLAGS		+= -L$(OPENSSLDIR)/lib -L$(OPENSSLDIR)
ENDLDLIBS	+= -lssl -lcrypto
HAS_OPENSSL	= 1
endif
endif


# define expat (XML parser) variables if installed
ifneq (,$(wildcard $(SYSINCDIR)/expat.h))
HAS_EXPAT	= 1
ENDLDLIBS	+= -lexpat
STDCCFLAGS	+= -DP_EXPAT
endif

ifneq ($(OSTYPE),rtems)
ifneq (,$(wildcard /usr/local/include/expat.h))
HAS_EXPAT	= 1
ENDLDLIBS	+= -lexpat
STDCCFLAGS	+= -DP_EXPAT -I /usr/local/include
LDFLAGS		+= -L /usr/local/lib
endif
endif


# define ESDDIR variables if installed
ifdef  ESDDIR
STDCCFLAGS	+= -I$(ESDDIR)/include
ENDLDLIBS	+= $(ESDDIR)/lib/libesd.a  # to avoid name conflicts
HAS_ESD		= 1
endif

# define Posix threads stuff
ifdef P_PTHREADS
STDCCFLAGS	+= -DP_PTHREADS
endif

# define IP v6 stuff
ifndef NO_IPv6
ifneq (,$(wildcard /proc/net/if_inet6))
STDCCFLAGS	+= -DP_HAS_IPV6
endif
endif

## define IP v6 stuff
ifndef NO_IPv6
ifneq (,$(wildcard /usr/include/netinet6/in6.h))
STDCCFLAGS	+= -DP_HAS_IPV6
endif
endif

#define templates if available
ifndef NO_PWLIB_TEMPLATES
STDCCFLAGS	+= -DPHAS_TEMPLATES
endif

# compiler flags for all modes
STDCCFLAGS	+= -DPBYTE_ORDER=$(ENDIAN)
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
