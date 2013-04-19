#
# post.mak
#
# Common make rules included in various places, part 2
#
# Portable Windows Library
#
# Copyright (c) 1993-2013 Equivalence Pty. Ltd.
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
# Main targets for build management
#
######################################################################

STANDARD_TARGETS +=\
  opt         debug         both \
  optstatic   debugstatic   bothstatic \
  optdepend   debugdepend   bothdepend \
  optclean    debugclean    clean \
  optlibs     debuglibs     bothlibs \
  release

.PHONY: $(STANDARD_TARGETS) all help internal_shared internal_static internal_build internal_clean internal_depend internal_libs

# Default goal
opt ::


help :
	@echo "The following targets are available:"
	@echo "  make opt           Make optimised version of application"
	@echo "  make debug         Make debug version of application"
	@echo "  make both          Make both versions of application"
	@echo
	@echo "  make optstatic     Make static optimised version of application"
	@echo "  make debugstatic   Make static debug version of application"
	@echo "  make bothstatic    Make static both versions of application"
	@echo
	@echo "  make optdepend     Create optimised dependency files"
	@echo "  make debugdepend   Create debug dependency files"
	@echo "  make bothdepend    Create both debug and optimised dependency files"
	@echo
	@echo "  make optclean      Remove optimised files"
	@echo "  make debugclean    Remove debug files"
	@echo "  make clean         Remove both debug and optimised files"
	@echo "  make distclean     Remove everything"
	@echo
	@echo "  make optlibs       Make optimised libraries project depends on"
	@echo "  make debuglibs     Make debug libraries project depends on"
	@echo "  make bothlibs      Make both debug and optimised libraries project depends on"
	@echo
	@echo "  make all           Create debug & optimised dependencies & libraries"
	@echo
	@echo "  make version       Display version for project"
	@echo "  make release       Package up optimised version int tar.gz file"


LIBDIRS := $(filter-out $(LIBDIRS_EXCLUDE), $(LIBDIRS))
ifneq ($(LIBDIRS),)
  all :: bothlibs
endif
all :: bothdepend both
both :: opt debug
bothshared :: optshared debugshared
bothstatic :: optstatic debugstatic
bothdepend :: optdepend debugdepend
bothlibs :: optlibs debuglibs
opt :: optshared
debug :: debugshared
clean :: optclean debugclean

  optshared   optstatic   optclean   optdepend   optlibs :: INTERNAL_DEBUG_BUILD:=no
debugshared debugstatic debugclean debugdepend debuglibs :: INTERNAL_DEBUG_BUILD:=yes

optshared debugshared optclean debugclean optdepend debugdepend optlibs debuglibs :: INTERNAL_STATIC_BUILD:=no
optstatic debugstatic :: INTERNAL_STATIC_BUILD:=yes

optshared debugshared optstatic debugstatic optclean debugclean optdepend debugdepend optlibs debuglibs ::
	$(Q_MAKE) --file="$(firstword $(MAKEFILE_LIST))" DEBUG_BUILD=$(INTERNAL_DEBUG_BUILD) STATIC_BUILD=$(INTERNAL_STATIC_BUILD) internal_$(subst opt,,$(subst debug,,$@))


# For backward compatibility reasons
bothnoshared :: optnoshared debugnoshared
optnoshared :: optstatic
debugnoshared :: debugstatic


######################################################################
# common rules

ifndef OBJDIR
  ifndef OBJDIR_PREFIX
    OBJDIR_PREFIX=.
  endif
  OBJDIR = $(OBJDIR_PREFIX)/obj_$(target)$(OBJ_SUFFIX)
endif

#
# define rule for .cxx, .cpp and .c files
#
$(OBJDIR)/%.o : %.cxx 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CXX)$(CXX) $(strip $(CPPFLAGS) $(CXXFLAGS)) -c $< -o $@

$(OBJDIR)/%.o : %.cpp 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CXX)$(CXX) $(strip $(CPPFLAGS) $(CXXFLAGS)) -c $< -o $@

$(OBJDIR)/%.o : %.mm 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CC)$(CXX) $(strip $(CPPFLAGS) $(CXXFLAGS)) -c $< -o $@

$(OBJDIR)/%.o : %.c 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_CC)$(CC) $(strip $(CPPFLAGS) $(CFLAGS)) -c $< -o $@

#
# create list of object files 
#
SRC_OBJS := $(SOURCES:.c=.o)
SRC_OBJS := $(SRC_OBJS:.mm=.o)
SRC_OBJS := $(SRC_OBJS:.cxx=.o)
SRC_OBJS := $(SRC_OBJS:.cpp=.o)
OBJS	 := $(strip $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(SRC_OBJS) $(OBJS))))

#
# create list of dependency files 
#
DEPDIR	  = $(OBJDIR)
SRC_DEPS := $(SOURCES:.c=.dep)
SRC_DEPS := $(SRC_DEPS:.mm=.dep)
SRC_DEPS := $(SRC_DEPS:.cxx=.dep)
SRC_DEPS := $(SRC_DEPS:.cpp=.dep)
DEPS	  = $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(SRC_DEPS)))
DEPFLAGS := $(subst $(DEBUG_FLAG),,$(CPPFLAGS))

#
# define rule for .dep files
#
$(DEPDIR)/%.dep : %.cxx 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CXX) $(strip $(DEPFLAGS) $(CFLAGS)) -M $< >> $@

$(DEPDIR)/%.dep : %.cpp 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CXX) $(strip $(DEPFLAGS) $(CFLAGS)) -M $< >> $@

$(DEPDIR)/%.dep : %.mm 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CXX) $(strip $(DEPFLAGS) $(CFLAGS)) -M $< >> $@

$(DEPDIR)/%.dep : %.c 
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(Q_DEP)$(CC) $(strip $(DEPFLAGS) $(CFLAGS)) -M $< >> $@

vpath %.cxx $(VPATH_CXX)
vpath %.cpp $(VPATH_CXX)
vpath %.mm  $(VPATH_MM)
vpath %.c   $(VPATH_C)
vpath %.o   $(OBJDIR)
vpath %.dep $(DEPDIR)


######################################################################
# rules for application

ifdef PROG

  ifndef TARGET
    TARGET = $(OBJDIR)/$(PROG)
  endif

  TARGET_LIBS = $(PTLIB_LIBDIR)/$(PTLIB_FILE)

  $(TARGET) : $(OBJS) $(TARGET_LIBS)
	@if [ ! -d $(dir $@) ] ; then $(MKDIR_P) $(dir $@) ; fi
	$(Q_LD)$(LD) -o $@ $(OBJS) $(CFLAGS) $(LDFLAGS)

else # PROG -  so must be a library

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
	$(Q_LD)$(LD) -o $@ $(strip $(SHARED_LDFLAGS:INSERT_SONAME=$(LIB_SONAME)) $(OBJS) $(LDFLAGS))

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

# This goal is here, after TARGET is defined
internal_build internal_shared internal_static :: $(TARGET)
	@true


######################################################################
# Cleaning up

CLEAN_FILES += $(OBJS) $(DEPS) core* $(TARGET)

internal_clean ::
	rm -rf $(CLEAN_FILES)


DIST_CLEAN_FILES += $(CLEAN_FILES) config.log config.err autom4te.cache config.status a.out aclocal.m4 lib* samples/*/obj_*

distclean ::
	rm -rf $(DIST_CLEAN_FILES)


######################################################################
# Building dependent libraries

internal_libs ::
	$(Q)set -e; $(foreach dir,$(LIBDIRS), $(MAKE) --print-directory --directory="$(dir)" internal_build ; )


###############################################################################
# all the targets are passed to all subdirectories

ifneq (,$(SUBDIRS))
  export CC CXX LD AR RANLIB CPPFLAGS CFLAGS CXXFLAGS LDFLAGS ARFLAGS target target_os target_cpu LIBDIRS_EXCLUDE
  $(STANDARD_TARGETS) ::
	$(Q)set -e; $(foreach dir,$(SUBDIRS), $(MAKE) --print-directory --directory="$(dir)" $@;)
endif


######################################################################

ifneq (,$(SVN))
  update: svnupdate bothdepend both
  svnupdate:
	$(SVN) update
	@echo =====================================================
endif


######################################################################
# common rule to generate SVN revision

ifneq (,$(REVISION_FILE))
  $(REVISION_FILE) : $(REVISION_FILE).in
  ifeq (,$(SVN))
	$(Q)sed -e "s/.WCREV./`sed -n -e 's/.*Revision: \([0-9]*\).*/\1/p' $<`/" $< > $@
  else
	$(Q)sed "s/SVN_REVISION.*/SVN_REVISION `LC_ALL=C $(SVN) info $(dir $@) | sed -n 's/Revision: //p'`/" $< > $@.tmp
	$(Q)if diff -q $@ $@.tmp >/dev/null 2>&1; then \
	  rm $@.tmp; \
	else \
	  mv -f $@.tmp $@; \
	  echo "Revision file updated to `sed -n 's/.*SVN_REVISION *\(.*\)/\1/p' $@`" ; \
	fi

    .PHONY: $(REVISION_FILE)
  endif
endif


######################################################################
# common rule to generate documentation

ifneq (,$(DOXYGEN_CFG))
  ifndef DOCS_DIR
    DOCS_DIR := $(dir $(DOXYGEN_CFG))/html
  endif
  DOXYGEN_OUT := /tmp/ptlib_doxygen.out
  DOXYGEN_GRAPH_CFG := /tmp/ptlib_graph_cfg.dxy

  .PHONY:docs
  docs:
	rm -rf $(DOCS_DIR)
	cd $(PTLIB_TOP_LEVEL_DIR)
	doxygen $(DOXYGEN_CFG) > $(DOXYGEN_OUT) 2>&1

  .PHONY:graphdocs
  graphdocs:
	rm -rf $(DOCS_DIR)
	sed "s/HAVE_DOT.*=.*/HAVE_DOT=YES/" $(DOXYGEN_CFG) > $(DOXYGEN_GRAPH_CFG)
	doxygen $(DOXYGEN_GRAPH_CFG) > $(DOXYGEN_OUT) 2>&1
	rm $(DOXYGEN_GRAPH_CFG)
endif


######################################################################
# common rule to make a release of the program

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


ifndef VERSION
  version release ::
	@echo Must define VERSION macro or have version.h/custom.cxx file.
else
  version:
	@echo v$(VERSION)


  ifndef RELEASEDIR
    RELEASEDIR=releases
  endif

  ifndef RELEASEBASEDIR
    RELEASEBASEDIR=$(PROG)
  endif

  RELEASEPROGDIR=$(RELEASEDIR)/$(RELEASEBASEDIR)

  # Cannot do this in debug mode, so do it without
  release :: DEBUG_BUILD=no

  release :: $(TARGET) releasefiles
	cp $(TARGET) $(RELEASEPROGDIR)/$(PROG)
	cd $(RELEASEDIR) ; tar chf - $(RELEASEBASEDIR) | gzip > $(PROG)_$(VERSION)_$(target).tar.gz
	rm -r $(RELEASEPROGDIR)

  releasefiles ::
	-$(MKDIR_P) $(RELEASEPROGDIR)

endif # VERSION


######################################################################
#
# Include all of the dependencies
#
######################################################################

.DELETE_ON_ERROR : $(DEPS)
internal_depend :: $(DEPS)
	@true

ifndef NODEPS
  ifneq ($(wildcard $(DEPDIR)/*.dep),)
    include $(DEPDIR)/*.dep
  endif
endif


# End of post.mak
