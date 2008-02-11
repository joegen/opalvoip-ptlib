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
# $Revision$
# $Author$
# $Date$
#

######################################################################
#
# common rules
#
######################################################################

# Submodules built with make lib
LIBDIRS += $(PTLIBDIR)


ifndef OBJDIR
ifndef OBJDIR_PREFIX
OBJDIR_PREFIX=.
endif
OBJDIR	=	$(OBJDIR_PREFIX)/$(PT_OBJBASE)
endif

vpath %.cxx $(VPATH_CXX)
vpath %.cpp $(VPATH_CXX)
vpath %.c   $(VPATH_C)
vpath %.o   $(OBJDIR)
vpath %.dep $(DEPDIR)
vpath %.gch $(PTLIBDIR)/include

#
# add common directory to include path - must be after PT directories
#
STDCCFLAGS	+= -I$(PTLIBDIR)/include

ifneq ($(P_SHAREDLIB),1)

#ifneq ($(OSTYPE),Darwin) # Mac OS X does not really support -static
#LDFLAGS += -static
#endif

ifneq ($(P_STATIC_LDFLAGS),)
LDFLAGS += $(P_STATIC_LDFLAGS)
endif

ifneq ($(P_STATIC_ENDLDLIBS),)
ENDLDLIBS += $(P_STATIC_ENDLDLIBS)
endif

endif

#  clean whitespace out of source file list
SOURCES         := $(strip $(SOURCES))

#
# define rule for .cxx, .cpp and .c files
#
$(OBJDIR)/%.o : %.cxx 
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ; fi
	$(CXX) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) $(STDCXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.cpp 
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ; fi
	$(CXX) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) $(STDCXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c 
	@if [ ! -d $(OBJDIR) ] ; then mkdir -p $(OBJDIR) ; fi
	$(CC) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) -c $< -o $@

#
# create list of object files 
#
SRC_OBJS := $(SOURCES:.c=.o)
SRC_OBJS := $(SRC_OBJS:.cxx=.o)
SRC_OBJS := $(SRC_OBJS:.cpp=.o)
OBJS	 := $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(SRC_OBJS) $(OBJS)))

#
# create list of dependency files 
#
DEPDIR	 := $(OBJDIR)
SRC_DEPS := $(SOURCES:.c=.dep)
SRC_DEPS := $(SRC_DEPS:.cxx=.dep)
SRC_DEPS := $(SRC_DEPS:.cpp=.dep)
DEPS	 := $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(SRC_DEPS) $(DEPS)))

#
# define rule for .dep files
#
$(DEPDIR)/%.dep : %.cxx 
	@if [ ! -d $(DEPDIR) ] ; then mkdir -p $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CXX) $(CPPFLAGS) $(STDCCFLAGS:-g=) -M $< >> $@

$(DEPDIR)/%.dep : %.cpp 
	@if [ ! -d $(DEPDIR) ] ; then mkdir -p $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CXX) $(CPPFLAGS) $(STDCCFLAGS:-g=) -M $< >> $@

$(DEPDIR)/%.dep : %.c 
	@if [ ! -d $(DEPDIR) ] ; then mkdir -p $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CC) $(CPPFLAGS) $(STDCCFLAGS:-g=) -M $< >> $@

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
TARGET = $(OBJDIR)/$(PROG)
endif

ifdef BUILDFILES
OBJS += $(OBJDIR)/buildnum.o
endif

TARGET_LIBS	= $(PTLIBDIR)/lib/$(PTLIB_FILE)

# distinguish betweek building and using pwlib
ifeq (,$(wildcard $(PTLIBDIR)/src))
TARGET_LIBS     = $(SYSLIBDIR)/$(PTLIB_FILE)
endif

$(TARGET):	$(OBJS) $(TARGET_LIBS)
ifeq ($(OSTYPE),beos)
# BeOS won't find dynamic libraries unless they are in one of the system
# library directories or in the lib directory under the application's
# directory
	@if [ ! -L $(OBJDIR)/lib ] ; then cd $(OBJDIR); ln -s $(PT_LIBDIR) lib; fi
endif
	$(LD) -o $@ $(CFLAGS) $(LDFLAGS) $(OBJS) $(LDLIBS) $(ENDLDLIBS) $(ENDLDFLAGS)

ifdef DEBUG

ifneq (,$(wildcard $(PTLIBDIR)/src/ptlib/unix))
$(PT_LIBDIR)/$(PTLIB_FILE):
	$(MAKE) -C $(PTLIBDIR)/src/ptlib/unix debug
endif

else

ifneq (,$(wildcard $(PTLIBDIR)/src/ptlib/unix))
$(PT_LIBDIR)/$(PTLIB_FILE):
	$(MAKE) -C $(PTLIBDIR)/src/ptlib/unix opt
endif

endif

CLEAN_FILES += $(TARGET)

ifndef INSTALL_OVERRIDE

install:	$(TARGET)
	$(INSTALL) $(TARGET) $(INSTALLBIN_DIR)
endif

# ifdef PROG
endif

$(PTLIBDIR)/include/ptlib.h.gch/$(PT_OBJBASE): $(PTLIBDIR)/include/ptlib.h
	@if [ ! -d `dirname $@` ] ; then mkdir -p `dirname $@` ; fi
	$(CXX) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) $(STDCXXFLAGS) -x c++ -c $< -o $@

PCH_FILES =	$(PTLIBDIR)/include/ptlib.h.gch/$(PT_OBJBASE)

#ifdef USE_PCH
#PCH:		$(PCH_FILES)
#
#CLEAN_FILES  += $(PCH_FILES)
#else
PCH:		
#endif


######################################################################
#
# Main targets for build management
#
######################################################################

all :: debuglibs debugdepend debug optlibs optdepend opt

help:
	@echo "The following targets are available:"
	@echo "  make debug         Make debug version of application"
	@echo "  make opt           Make optimised version of application"
	@echo "  make both          Make both versions of application"
	@echo
	@echo "  make debugnoshared Make static debug version of application"
	@echo "  make optnoshared   Make static optimised version of application"
	@echo "  make bothnoshared  Make static both versions of application"
	@echo
	@echo "  make debugclean    Remove debug files"
	@echo "  make optclean      Remove optimised files"
	@echo "  make clean         Remove both debug and optimised files"
	@echo
	@echo "  make debugdepend   Create debug dependency files"
	@echo "  make optdepend     Create optimised dependency files"
	@echo "  make bothdepend    Create both debug and optimised dependency files"
	@echo
	@echo "  make debuglibs     Make debug libraries project depends on"
	@echo "  make optlibs       Make optimised libraries project depends on"
	@echo "  make bothlibs      Make both debug and optimised libraries project depends on"
	@echo
	@echo "  make all           Create debug & optimised dependencies & libraries"
	@echo
	@echo "  make version       Display version for project"
	@echo "  make tagbuild      Do a CVS tag of the source, and bump build number"
	@echo "  make release       Package up optimised version int tar.gz file"


ifdef DEBUG

debug :: PCH $(TARGET) 

opt ::
	@$(MAKE) DEBUG= PCH opt

debugclean ::
	rm -rf $(CLEAN_FILES)

optclean ::
	@$(MAKE) DEBUG= optclean

.DELETE_ON_ERROR : debugdepend

debugdepend :: $(DEPS)
	@echo Created dependencies.

optdepend ::
	@$(MAKE) DEBUG= optdepend

debuglibs :: libs

optlibs ::
	@$(MAKE) DEBUG= libs

libs ::
	set -e; for i in $(LIBDIRS); do $(MAKE) -C $$i debugdepend debug; done

else

debug :: 
	@$(MAKE) DEBUG=1 PCH debug

opt :: PCH $(TARGET)

debugclean ::
	@$(MAKE) DEBUG=1 debugclean

optclean ::
	rm -rf $(CLEAN_FILES)

.DELETE_ON_ERROR : optdepend

debugdepend ::
	@$(MAKE) DEBUG=1 debugdepend

optdepend :: $(DEPS)
	@echo Created dependencies.

debuglibs ::
	@$(MAKE) DEBUG=1 libs

optlibs :: libs

libs ::
	set -e; for i in $(LIBDIRS); do $(MAKE) -C $$i optdepend opt; done

endif

both :: opt debug
clean :: optclean debugclean
bothdepend :: optdepend debugdepend
bothlibs :: optlibs debuglibs

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

# Finally check that version numbers are not empty

  ifeq (,$(MAJOR_VERSION))
    override MAJOR_VERSION:=1
  endif
  ifeq (,$(MINOR_VERSION))
    override MINOR_VERSION:=0
  endif
  ifeq (,$(BUILD_TYPE))
    override BUILD_TYPE:=alpha
  endif
  ifeq (,$(BUILD_NUMBER))
    override BUILD_NUMBER:=0
  endif

# Check for VERSION either predefined or defined by previous section from VERSION_FILE
  ifndef VERSION
    VERSION:=$(MAJOR_VERSION).$(MINOR_VERSION)$(BUILD_TYPE)$(BUILD_NUMBER)
  endif # ifndef VERSION
endif # ifdef VERSION_FILE

# Build the CVS_TAG string from the components
ifndef CVS_TAG
  CVS_TAG := v$(MAJOR_VERSION)_$(MINOR_VERSION)$(subst .,_,$(BUILD_TYPE))$(BUILD_NUMBER)
endif

ifdef DEBUG

# Cannot do this in DEBUG mode, so do it without DEBUG

release ::
	$(MAKE) DEBUG= release

else


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

print-version:
	@echo $(VERSION)

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
	mv -f $(VERSION_FILE).new $(VERSION_FILE)
	cvs commit -m "Pre-tagging check in for $(CVS_TAG)." $(VERSION_FILE)
	cvs tag -c $(CVS_TAG)
	BLD=`expr $(BUILD_NUMBER) + 1` ; \
	echo "Incrementing to build number " $$BLD; \
	sed "s/$(BUILD_NUMBER_DEFINE)[ ]*[0-9][0-9]*/$(BUILD_NUMBER_DEFINE) $$BLD/" $(VERSION_FILE) > $(VERSION_FILE).new
	mv -f $(VERSION_FILE).new $(VERSION_FILE)
	cvs commit -m "Incremented build number after tagging to $(CVS_TAG)." $(VERSION_FILE)
	cvs -q update -r $(CVS_TAG)

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
# Include all of the dependencies
#
######################################################################

ifndef NODEPS
-include $(DEPDIR)/*.dep
endif


# End of common.mak

