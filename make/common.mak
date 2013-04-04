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

ORIGINAL_MAKEFILE = $(firstword $(MAKEFILE_LIST))

# Submodules built with make lib
ifdef PTLIBDIR
  LIBDIRS += $(PTLIBDIR)
endif


ifndef OBJDIR
ifndef OBJDIR_PREFIX
OBJDIR_PREFIX=.
endif
OBJDIR = $(OBJDIR_PREFIX)/obj_$(target)$(OBJ_SUFFIX)
endif

vpath %.cxx $(VPATH_CXX)
vpath %.cpp $(VPATH_CXX)
vpath %.c   $(VPATH_C)
vpath %.o   $(OBJDIR)
vpath %.dep $(DEPDIR)

#
# add common directory to include path - must be after PT directories
#
ifdef PTLIBDIR
  CPPFLAGS := -I$(PTLIBDIR)/include $(CPPFLAGS)
else
  CPPFLAGS := $(shell pkg-config ptlib --cflags-only-I) $(CPPFLAGS)
endif

ifneq ($(P_SHAREDLIB),1)

#ifneq ($(target_os),Darwin) # Mac OS X does not really support -static
#LDFLAGS += -static
#endif

endif


ifeq ($(V)$(VERBOSE),)
Q    = @
Q_CC = @echo [CC] `echo $< | sed s/$PWD//` ; 
Q_DEP= @echo [DEP] `echo $< | sed s/$PWD//` ; 
Q_AR = @echo [AR] `echo $@ | sed s/$PWD//` ; 
Q_LD = @echo [LD] `echo $@ | sed s/$PWD//` ; 
endif


#
# define rule for .cxx, .cpp and .c files
#
$(OBJDIR)/%.o : %.cxx 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CC)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.cpp 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CC)$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

$(OBJDIR)/%.o : %.c 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CC)$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

#
# create list of object files 
#
SRC_OBJS := $(SOURCES:.c=.o)
SRC_OBJS := $(SRC_OBJS:.cxx=.o)
SRC_OBJS := $(SRC_OBJS:.cpp=.o)
OBJS	 := $(strip $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(SRC_OBJS) $(OBJS))))

#
# create list of dependency files 
#
DEPDIR	 := $(OBJDIR)
SRC_DEPS := $(SOURCES:.c=.dep)
SRC_DEPS := $(SRC_DEPS:.cxx=.dep)
SRC_DEPS := $(SRC_DEPS:.cpp=.dep)
DEPS	 := $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(SRC_DEPS) $(DEPS)))
DEPFLAGS := $(subst $(DEBUG_FLAG),,$(CPPFLAGS))

#
# define rule for .dep files
#
$(DEPDIR)/%.dep : %.cxx 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CXX) $(DEPFLAGS) $(CFLAGS) -M $< >> $@

$(DEPDIR)/%.dep : %.cpp 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CXX) $(DEPFLAGS) $(CFLAGS) -M $< >> $@

$(DEPDIR)/%.dep : %.c 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CC) $(DEPFLAGS) $(CFLAGS) -M $< >> $@

#
# add in good files to delete
#
CLEAN_FILES += $(OBJS) $(DEPS) core* $(TARGET)


######################################################################
#
# rules for application
#
######################################################################

ifdef	PROG

ifndef TARGET
TARGET = $(OBJDIR)/$(PROG)
endif

TARGET_LIBS = $(PTLIB_LIBDIR)/$(PTLIB_FILE)

$(TARGET):	$(OBJS) $(TARGET_LIBS)
ifeq ($(target_os),beos)
# BeOS won't find dynamic libraries unless they are in one of the system
# library directories or in the lib directory under the application's
# directory
	@if [ ! -L $(OBJDIR)/lib ] ; then cd $(OBJDIR); ln -s $(PTLIB_LIBDIR) lib; fi
endif
	$(Q_LD)$(LD) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

ifneq (,$(wildcard $(PTLIBDIR)/src/ptlib/unix))
$(TARGET_LIBS) :
ifdef DEBUG
	$(MAKE) -f $(ORIGINAL_MAKEFILE) -C $(PTLIBDIR) debug
else
	$(MAKE) -f $(ORIGINAL_MAKEFILE) -C $(PTLIBDIR) opt
endif
endif # have source


else # PROG

  ifdef SHARED_LIB_LINK

    ifndef TARGET
      TARGET = $(SHARED_LIB_LINK)
    endif

    $(SHARED_LIB_LINK): $(SHARED_LIB_FILE)
	$Q cd $(dir $@) ; rm -f $@ ; $(LN_S) $(notdir $<) $(notdir $@)

  endif # SHARED_LIB_LINK


  ifdef SHARED_LIB_FILE

    ifndef TARGET
      TARGET = $(SHARED_LIB_FILE)
    endif

    ifndef
      LIB_SONAME = $(notdir $(SHARED_LIB_FILE))
    endif

    $(SHARED_LIB_FILE): $(STATIC_LIB_FILE)
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_LD)$(LD) -o $@ $(SHARED_LDFLAGS:INSERT_SONAME=$(LIB_SONAME)) $(OBJS) $(LDFLAGS)

  endif # SHARED_LIB_FILE


  ifdef STATIC_LIB_FILE
    ifndef TARGET
      TARGET = $(STATIC_LIB_FILE)
    endif

    $(STATIC_LIB_FILE): $(OBJS)
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_AR)$(AR) $(ARFLAGS) $@ $(OBJS)
      ifneq ($(RANLIB),)
	$Q$(RANLIB) $@
     endif

  endif # STATIC_LIB_FILE

endif # PROG


######################################################################
#
# Main targets for build management
#
######################################################################

STANDARD_TARGETS=\
opt         debug         both \
optdepend   debugdepend   bothdepend \
optshared   debugshared   bothshared \
optnoshared debugnoshared bothnoshared \
optclean    debugclean    clean \
release

.PHONY: all $(STANDARD_TARGETS)

default_target : $(TARGET)

default_clean :
	rm -rf $(CLEAN_FILES)

.DELETE_ON_ERROR : default_depend
default_depend :: $(DEPS)

libs ::
ifneq ($(LIBDIRS),)
	@set -e; for i in $(LIBDIRS); do $(MAKE) -f $(ORIGINAL_MAKEFILE) -C $$i default_depend default_target; done
else
	@true
endif

help:
	@echo "The following targets are available:"
	@echo "  make debug         Make debug version of application"
	@echo "  make opt           Make optimised version of application"
	@echo "  make both          Make both versions of application"
	@echo
	@echo "  make debugstatic   Make static debug version of application"
	@echo "  make optstatic     Make static optimised version of application"
	@echo "  make bothstatic    Make static both versions of application"
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
	@echo "  make release       Package up optimised version int tar.gz file"


all :: debuglibs debugdepend debug optlibs optdepend opt
clean :: optclean debugclean optnosharedclean debugnosharedclean
depend :: default_depend

both :: opt debug
bothshared :: optshared debugshared
bothstatic :: optstatic debugstatic
bothnoshared :: optnoshared debugnoshared
bothdepend :: optdepend debugdepend
bothlibs :: optlibs debuglibs

opt ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= default_target

optshared ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= P_SHAREDLIB=1 default_target

optstatic optnoshared ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= P_SHAREDLIB=0 default_target

optclean ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= default_clean

optstaticclean optnosharedclean ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= P_SHAREDLIB=0 default_clean

optdepend ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= default_depend

optlibs ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= libs


debug :: 
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 default_target

debugshared ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 P_SHAREDLIB=1 default_target

debugstatic debugnoshared ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 P_SHAREDLIB=0 default_target

debugclean ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 default_clean

debugstaticclean debugnosharedclean ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 P_SHAREDLIB=0 default_clean

debugdepend ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 default_depend

debuglibs ::
	@$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG=1 libs



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

ifdef DEBUG

# Cannot do this in DEBUG mode, so do it without DEBUG

release ::
	$(MAKE) -f $(ORIGINAL_MAKEFILE) DEBUG= release

else


ifndef VERSION

release ::
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
	cd $(RELEASEDIR) ; tar chf - $(RELEASEBASEDIR) | gzip > $(PROG)_$(VERSION)_$(target).tar.gz
	rm -r $(RELEASEPROGDIR)

releasefiles ::
	-$(MKDIR_P) $(RELEASEPROGDIR)


version:
	@echo v$(VERSION)

ifndef VERSION_FILE

else # ifndef VERSION_FILE

endif # else ifndef VERSION_FILE

endif # else ifdef VERSION

endif # else ifdef DEBUG


######################################################################
#
# Include all of the dependencies
#
######################################################################

ifndef NODEPS
ifneq ($(wildcard $(DEPDIR)/*.dep),)
include $(DEPDIR)/*.dep
endif
endif


# End of common.mak

