#
# common.mak
#
# Common make rules included in ptlib.mak and pwlib.mak
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
# $Log: common.mak,v $
# Revision 1.45  2000/03/20 23:08:31  craigs
# Added showgui target to allow displaying the GUI settings
#
# Revision 1.44  2000/03/20 22:43:09  craigs
# Added totally new mechanism for detecting GUI
#
# Revision 1.43  2000/03/03 00:37:42  robertj
# Fixed problem for when have GUI environment variable set, always builds GUI!
#
# Revision 1.42  2000/02/24 11:02:11  craigs
# Fixed problems with PW make
#
# Revision 1.41  2000/02/16 11:30:25  craigs
# Added rule to force library build for applications
#
# Revision 1.40  2000/02/04 19:33:25  craigs
# Added ability to create non-shared versions of programs
#
# Revision 1.39  2000/01/22 00:51:18  craigs
# Added ability to compile in any directory, and to create shared libs
#
# Revision 1.38  1999/07/10 03:32:02  robertj
# Improved release version detection code.
#
# Revision 1.37  1999/07/03 04:31:53  robertj
# Fixed problems with not including oss.cxx in library if OSTYPE not "linux"
#
# Revision 1.36  1999/07/02 05:10:33  robertj
# Fixed bug in changing from debug default to opt build
#
# Revision 1.35  1999/06/28 09:12:01  robertj
# Fixed problems with the order in which macros are defined especially on BeOS & Solaris
#
# Revision 1.34  1999/06/27 02:42:10  robertj
# Fixed BeOS compatability.
# Fixed error of platform name not supported, needed :: on main targets.
#
# Revision 1.33  1999/06/09 15:41:18  robertj
# Added better UI to make files.
#
# Revision 1.32  1999/04/18 09:36:31  robertj
# Get date grammar build.
#
# Revision 1.31  1999/02/19 11:32:10  robertj
# Improved the "release" target to build release tar files.
#
# Revision 1.30  1999/01/16 09:56:24  robertj
# Changed some macros to more informative names.
#
# Revision 1.29  1999/01/16 04:00:05  robertj
# Added bothclean target
#
# Revision 1.28  1998/12/02 02:36:57  robertj
# New directory structure.
#
# Revision 1.27  1998/11/26 12:48:20  robertj
# Support for .c files.
#
# Revision 1.26  1998/11/26 11:40:03  craigs
# Added checking for resource compilation
#
# Revision 1.25  1998/11/26 07:29:19  craigs
# Yet another bash at a GUI build environment
#
# Revision 1.24  1998/11/24 03:41:32  robertj
# Fixed problem where failed make depend leaves bad .dep files behind
#
# Revision 1.23  1998/11/22 10:41:02  craigs
# New GUI build system - for sure!
#
# Revision 1.22  1998/11/22 08:11:31  craigs
# *** empty log message ***
#
# Revision 1.21  1998/09/24 04:20:49  robertj
# Added open software license.
#


######################################################################
#
# common rules
#
######################################################################

ifndef OBJDIR
ifneq   (,$(GUI_TYPE))
OBJDIR	=	./$(PW_OBJBASE)
else
OBJDIR	=	./$(PT_OBJBASE)
endif
endif

vpath %.cxx $(VPATH_CXX)
vpath %.c   $(VPATH_C)
vpath %.o   $(OBJDIR)
vpath %.dep $(DEPDIR)

#
# add common directory to include path - must be after PW and PT directories
#
STDCCFLAGS	+= -I$(PWLIBDIR)/include

#
# add any trailing libraries
#
LDLIBS += $(ENDLDLIBS)

#  clean whitespace out of source file list
SOURCES         := $(strip $(SOURCES))

#
# define rule for .cxx and .c files
#
$(OBJDIR)/%.o : %.cxx 
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ; fi
	$(CPLUS) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c 
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ; fi
	$(CC) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) -c $< -o $@

#
# create list of object files 
#
SRC_OBJS := $(SOURCES:.c=.o)
SRC_OBJS := $(SRC_OBJS:.cxx=.o)
OBJS	 := $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(SRC_OBJS) $(OBJS)))

#
# create list of dependency files 
#
DEPDIR	 := $(OBJDIR)
SRC_DEPS := $(SOURCES:.c=.dep)
SRC_DEPS := $(SRC_DEPS:.cxx=.dep)
DEPS	 := $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(SRC_DEPS) $(DEPS)))

#
# define rule for .dep files
#
$(DEPDIR)/%.dep : %.cxx 
	@if [ ! -d $(DEPDIR) ] ; then mkdir -p $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CPLUS) $(STDCCFLAGS) -M $< >> $@

$(DEPDIR)/%.dep : %.c 
	@if [ ! -d $(DEPDIR) ] ; then mkdir -p $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CC) $(STDCCFLAGS) -M $< >> $@

#
# add in good files to delete
#
CLEAN_FILES += $(OBJS) $(DEPS) core

######################################################################
#
# rules for application
#
######################################################################

ifdef	PROG

ifndef TARGET
ifeq	($(P_SHAREDLIB),0)
TARGET = $(OBJDIR)/$(PROG)
else
TARGET = $(OBJDIR)/$(PROG)
endif
endif

ifdef BUILDFILES
OBJS += $(OBJDIR)/buildnum.o
endif

TARGET_LIBS	= $(PW_LIBDIR)/$(PTLIB_FILE)
ifneq (,$(GUI_TYPE))
TARGET_LIBS	:= $(TARGET_LIBS) $(PW_LIBDIR)/$(PWLIB_FILE)
endif

$(TARGET):	$(OBJS) $(TARGET_LIBS)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

ifdef DEBUG

$(PW_LIBDIR)/$(PTLIB_FILE):
	$(MAKE) -C $(PWLIBDIR)/src/ptlib/unix debug

$(PW_LIBDIR)/$(PWLIB_FILE):
	$(MAKE) -C $(PWLIBDIR)/src/pwlib/$(PWLIB_GUI) debug

else

$(PW_LIBDIR)/$(PTLIB_FILE):
	$(MAKE) -C $(PWLIBDIR)/src/ptlib/unix opt

$(PW_LIBDIR)/$(PWLIB_FILE):
	$(MAKE) -C $(PWLIBDIR)/src/pwlib/$(PWLIB_GUI) opt

endif

CLEAN_FILES += $(TARGET)

# ifdef PROG
endif

######################################################################
#
# common rule to make a release of the program
#
######################################################################

ifdef DEBUG

release ::
	$(MAKE) DEBUG= release

else

ifndef RELEASEDIR
RELEASEDIR=releases
endif

ifndef RELEASEBASEDIR
RELEASEBASEDIR=$(PROG)
endif

ifndef VERSION
ifneq (,$(wildcard custom.cxx))
VERSION_FILE := custom.cxx
endif
ifneq (,$(wildcard version.h))
VERSION_FILE := version.h
endif

ifdef VERSION_FILE
VERSION:=$(strip \
	$(subst \#define,, $(subst MAJOR_VERSION,,\
		$(shell grep "define *MAJOR_VERSION" $(VERSION_FILE))))).$(strip \
	$(subst \#define,,$(subst MINOR_VERSION,,\
		$(shell grep "define *MINOR_VERSION" $(VERSION_FILE)))))$(strip \
	$(subst \#define,,$(subst BUILD_TYPE,,\
		$(subst AlphaCode,alpha,$(subst BetaCode,beta,$(subst ReleaseCode,pl,\
		$(shell grep "define *BUILD_TYPE" $(VERSION_FILE))))))))$(strip \
	$(subst \#define,,$(subst BUILD_NUMBER,,\
		$(shell grep "define *BUILD_NUMBER" $(VERSION_FILE)))))
endif
endif


ifndef VERSION

release ::
	@echo Must define VERSION macro or have version.h/custom.cxx file.

else

RELEASEPROGDIR=$(RELEASEDIR)/$(RELEASEBASEDIR)

release :: $(TARGET) releasefiles
	cp $(TARGET) $(RELEASEPROGDIR)/$(PROG)
	cd $(RELEASEDIR) ; tar chf - $(RELEASEBASEDIR) | gzip > $(PROG)_$(VERSION)_$(PLATFORM_TYPE).tar.gz
	rm -r $(RELEASEPROGDIR)

releasefiles ::
	-mkdir -p $(RELEASEPROGDIR)

endif

endif # else ifdef DEBUG


######################################################################
#
# Main targets for build management
#
######################################################################

ifdef DEBUG

debug :: $(TARGET)

opt ::
	@$(MAKE) DEBUG= opt

debugclean ::
	rm -rf $(CLEAN_FILES)

optclean ::
	@$(MAKE) DEBUG= optclean

.DELETE_ON_ERROR : debugdepend

debugdepend :: $(DEPS)
	@echo Created dependencies.

optdepend ::
	@$(MAKE) DEBUG= optdepend

libs:
	$(MAKE) -C $(PWLIBDIR) debug

else

debug ::
	@$(MAKE) DEBUG=1 debug

opt :: $(TARGET)

debugclean ::
	@$(MAKE) DEBUG=1 debugclean

optclean ::
	rm -rf $(CLEAN_FILES)

.DELETE_ON_ERROR : optdepend

debugdepend ::
	@$(MAKE) DEBUG=1 debugdepend

optdepend :: $(DEPS)
	@echo Created dependencies.

libs:
	$(MAKE) -C $(PWLIBDIR) opt

endif

both :: opt debug
clean :: optclean debugclean
bothdepend :: optdepend debugdepend


optshared ::
	$(MAKE) P_SHAREDLIB=1 opt

debugshared ::
	$(MAKE) P_SHAREDLIB=1 debug

bothshared ::
	$(MAKE) optshared debugshared

optnoshared ::
	$(MAKE) P_SHAREDLIB=0 opt

debugnoshared ::
	$(MAKE) P_SHAREDLIB=0 debug

bothnoshared ::
	$(MAKE) optshared debugshared



######################################################################
#
# rules for creating build number files
#
######################################################################
ifdef BUILDFILES
$(OBJDIR)/buildnum.o:	buildnum.c
	cc -o $(OBJDIR)/buildnum.o -c buildnum.c

#ifndef DEBUG
#buildnum.c:	$(SOURCES) $(BUILDFILES) 
#	buildinc buildnum.c
#else
buildnum.c:
#endif

endif


######################################################################
#
# rules for creating PW resources
#
######################################################################

ifneq (,$(GUI_TYPE))
ifdef RESOURCE

$(RESOBJS) : $(RESCXX) $(RESCODE)

$(RESCXX) : $(RESHDR)

TMPRSRC	= resource.tmp

$(RESCXX) $(RESCODE): $(RESHDR)

$(RESHDR): $(RESOURCE)
	@if test -e $(RESHDR) ; then mv $(RESHDR) $(TMPRSRC) ; fi
	$(PWRC_CMD) -v $(RCFLAGS) $(RESOURCE)
	@if test -e $(TMPRSRC) && diff -q $(RESHDR) $(TMPRSRC) ; then cp $(TMPRSRC) $(RESHDR) ; else rm -f $(TMPRSRC) ;  fi

$(RESOURCE) : $(PWRC)

$(PWRC) :
	$(MAKE) REALGUI=$(PWLIB_GUI) -C $(PWRC_DIR) opt

endif

showgui:
	@echo PWLIB_GUI    = $(PWLIB_GUI)
	@echo PWLIB_GUIDIR = $(PWLIB_GUIDIR)
endif


######################################################################
#
# Include all of the dependencies
#
######################################################################

-include $(DEPDIR)/*.dep


# End of common.mak

