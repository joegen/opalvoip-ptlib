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
# Revision 1.61  2001/10/09 08:53:26  robertj
# Added LIBDIRS variable so can go "make libs" and make all libraries.
# Added "make version" target to display version of project.
# Added inclusion of library versions into "make tagbuild" check in.
#
# Revision 1.60  2001/08/07 08:24:42  robertj
# Fixed bug in tagbuild if have more than one BUILD_NUMBER in file.
#
# Revision 1.59  2001/07/30 07:45:54  robertj
# Added "all" target with double colon.
#
# Revision 1.58  2001/07/27 14:39:12  robertj
# Allowed libs target to have multiple definitions
#
# Revision 1.57  2001/06/30 06:59:06  yurik
# Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
#
# Revision 1.56  2001/06/29 06:47:30  robertj
# Added missing dollar sign
#
# Revision 1.55  2001/06/29 06:41:03  robertj
# Fixed make tagbuild for different #defines
#
# Revision 1.54  2001/05/29 03:31:48  craigs
# Removed BROKEN_GCC symbol, now that pwlib is totally gcc friendly
#
# Revision 1.53  2001/03/29 04:48:45  robertj
# Added tagbuild target to do CVS tag and autoincrement BUILD_NUMBER
# Changed order so version.h is used before custom.cxx
#
# Revision 1.52  2001/03/23 19:59:48  craigs
# Added detection of broken gcc versions
#
# Revision 1.51  2001/03/22 01:14:16  robertj
# Allowed for the version file #defines to configured by calling makefile.
#
# Revision 1.50  2000/11/02 04:46:42  craigs
# Added support for buildnum.h file for version numbers
#
# Revision 1.49  2000/10/01 01:08:10  craigs
# Fixed problems with Motif build
#
# Revision 1.48  2000/09/20 23:59:35  craigs
# Fixed problem with bothnoshared target
#
# Revision 1.47  2000/04/26 00:40:48  robertj
# Redesigned version number system to have single file to change on releases.
#
# Revision 1.46  2000/04/06 20:12:33  craigs
# Added install targets
#
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

# Submodules built with make lib
LIBDIRS += $(PWLIBDIR)


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
ifdef PWLIB_GUI_FLAG
STDCCFLAGS	+= -D$(PWLIB_GUI_FLAG)
endif

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
ifeq ($(OSTYPE),beos)
# BeOS won't find dynamic libraries unless they are in one of the system
# library directories or in the lib directory under the application's
# directory
	@if [ ! -L $(OBJDIR)/lib ] ; then cd $(OBJDIR); ln -s $(PW_LIBDIR) lib; fi
endif
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

ifndef INSTALL_OVERRIDE

install:	$(TARGET)
	$(INSTALL) $(TARGET) $(INSTALLBIN_DIR)
endif

# ifdef PROG
endif


######################################################################
#
# Main targets for build management
#
######################################################################

all :: debugdepend debug optdepend opt


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

libs ::
	set -e; for i in $(LIBDIRS); do $(MAKE) -C $$i debug; done

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

libs ::
	set -e; for i in $(LIBDIRS); do $(MAKE) -C $$i opt; done

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
	$(MAKE) optnoshared debugnoshared



######################################################################
#
# common rule to make a release of the program
#
######################################################################

ifdef DEBUG

# Cannot do this in DEBUG mode, so do it without DEBUG

release ::
	$(MAKE) DEBUG= release

else

# if user has not defined VERSION macro, calculate it from version file
ifndef VERSION

# if have not explictly defined VERSION_FILE, locate a default

ifndef VERSION_FILE
  ifneq (,$(wildcard buildnum.h))
    VERSION_FILE := buildnum.h
  else
    ifneq (,$(wildcard version.h))
      VERSION_FILE := version.h
    else
      ifneq (,$(wildcard custom.cxx))
        VERSION_FILE := custom.cxx
      endif
    endif
  endif
endif


ifdef VERSION_FILE

# Set default strings to search in VERSION_FILE

ifndef MAJOR_VERSION_DEFINE
MAJOR_VERSION_DEFINE:=MAJOR_VERSION
endif

ifndef MINOR_VERSION_DEFINE
MINOR_VERSION_DEFINE:=MINOR_VERSION
endif

ifndef BUILD_NUMBER_DEFINE
BUILD_NUMBER_DEFINE:=BUILD_NUMBER
endif


# If not specified, find the various version components in the VERSION_FILE

ifndef MAJOR_VERSION
MAJOR_VERSION:=$(strip $(subst \#define,, $(subst $(MAJOR_VERSION_DEFINE),,\
			$(shell grep "define *$(MAJOR_VERSION_DEFINE) *" $(VERSION_FILE)))))
endif
ifndef MINOR_VERSION
MINOR_VERSION:=$(strip $(subst \#define,, $(subst $(MINOR_VERSION_DEFINE),,\
			$(shell grep "define *$(MINOR_VERSION_DEFINE)" $(VERSION_FILE)))))
endif
ifndef BUILD_TYPE
BUILD_TYPE:=$(strip $(subst \#define,,$(subst BUILD_TYPE,,\
			$(subst AlphaCode,alpha,$(subst BetaCode,beta,$(subst ReleaseCode,.,\
			$(shell grep "define *BUILD_TYPE" $(VERSION_FILE))))))))
endif
ifndef BUILD_NUMBER
BUILD_NUMBER:=$(strip $(subst \#define,,$(subst $(BUILD_NUMBER_DEFINE),,\
			$(shell grep "define *$(BUILD_NUMBER_DEFINE)" $(VERSION_FILE)))))
endif


# Build the VERSION string from the components

VERSION:=$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER)


# Build the CVS_TAG string from the components

ifndef CVS_TAG
CVS_TAG := v$(MAJOR_VERSION)_$(MINOR_VERSION)$(subst .,_,$(BUILD_TYPE))$(BUILD_NUMBER)
endif

endif # ifdef VERSION_FILE

endif # ifndef VERSION


# Check for VERSION either predefined or defined by previosu section from VERSION_FILE

ifndef VERSION

release ::
	@echo Must define VERSION macro or have version.h/custom.cxx file.

tagbuild ::
	@echo Must define VERSION macro or have version.h/custom.cxx file.

else # ifdef VERSION

# "make release" definition

ifndef RELEASEDIR
RELEASEDIR=releases
endif

ifndef RELEASEBASEDIR
RELEASEBASEDIR=$(PROG)
endif

RELEASEPROGDIR=$(RELEASEDIR)/$(RELEASEBASEDIR)

release :: $(TARGET) releasefiles
	cp $(TARGET) $(RELEASEPROGDIR)/$(PROG)
	cd $(RELEASEDIR) ; tar chf - $(RELEASEBASEDIR) | gzip > $(PROG)_$(VERSION)_$(PLATFORM_TYPE).tar.gz
	rm -r $(RELEASEPROGDIR)

releasefiles ::
	-mkdir -p $(RELEASEPROGDIR)


version:
	@echo v$(VERSION) "  CVS tag:" `cvs status Makefile | grep "Sticky Tag" | sed -e "s/(none)/HEAD/" -e "s/(.*)//" -e "s/^.*://"`


ifndef VERSION_FILE

tagbuild ::
	@echo Must define VERSION_FILE macro or have version.h/custom.cxx file.

else # ifndef VERSION_FILE

ifndef CVS_TAG

tagbuild ::
	@echo Must define CVS_TAG macro or have version.h/custom.cxx file.

else # ifndef CVS_TAG

tagbuild ::
	sed $(foreach dir,$(LIBDIRS), -e "s/ $(notdir $(dir)):.*/ $(notdir $(dir)): $(shell $(MAKE) -s -C $(dir) version)/") $(VERSION_FILE) > $(VERSION_FILE).new
	@if test -e $(TMPRSRC) && diff -q $(RESHDR) $(TMPRSRC) ; \
		then mv -f $(VERSION_FILE).new $(VERSION_FILE) ; \
		else rm -f $(VERSION_FILE).new ;  \
	fi
	cvs commit -m "Pre-tagging check in for $(CVS_TAG)." $(VERSION_FILE)
	cvs tag -c $(CVS_TAG)
	let BLD=$(BUILD_NUMBER)+1 ; \
	echo "Incrementing to build number $$BLD"; \
	sed "s/$(BUILD_NUMBER_DEFINE)[ ]*[0-9][0-9]*/$(BUILD_NUMBER_DEFINE) $$BLD/" $(VERSION_FILE) > $(VERSION_FILE).new
	mv -f $(VERSION_FILE).new $(VERSION_FILE)
	cvs commit -m "Incremented build number after tagging to $(CVS_TAG)." $(VERSION_FILE)

endif # else ifndef CVS_TAG

endif # else ifndef VERSION_FILE

endif # else ifdef VERSION

endif # else ifdef DEBUG


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
	@if test -e $(TMPRSRC) && diff -q $(RESHDR) $(TMPRSRC) ; \
		then cp $(TMPRSRC) $(RESHDR) ; \
		else rm -f $(TMPRSRC) ;  fi

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

