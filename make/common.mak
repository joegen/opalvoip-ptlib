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


vpath %.cxx $(VPATH_CXX)
vpath %.c   $(VPATH_C)
vpath %.o   $(OBJDIR)
vpath %.dep $(DEPDIR)

#
# add common directory to include path - must be after PW and PT directories
#
STDCCFLAGS	:= $(STDCCFLAGS) -I$(PWLIBDIR)/include

#
# add any trailing libraries
#
LDLIBS		:= $(LDLIBS) $(ENDLDLIBS)

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
TMP_SRC	:= $(SOURCES:.c=.o)
OBJS	:= $(TMP_SRC:.cxx=.o) $(OBJS)
OBJS	:= $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(OBJS)))

#
# create list of dependency files 
#
DEPDIR	:= $(OBJDIR)
TMP_SRC	:= $(SOURCES:.c=.dep)
DEPS	:= $(TMP_SRC:.cxx=.dep)
DEPS	:= $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(DEPS)))

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
CLEAN_FILES	:= $(CLEAN_FILES) $(OBJS) $(DEPS) core

######################################################################
#
# rules for application
#
######################################################################

ifdef	PROG

ifndef TARGET
ifndef	SHAREDLIB
TARGET = $(OBJDIR)/$(PROG)
else
TARGET = $(OBJDIR)/$(PROG)_dll
endif
endif

ifdef BUILDFILES
OBJS	:= $(OBJS) $(OBJDIR)/buildnum.o
endif

ifndef STATIC

$(TARGET):	$(OBJS) $(PTLIB_FILE)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)

else

$(TARGET):	$(OBJS) $(PTLIB_FILE)
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
          ln -s $(SYSLIBDIR)/$$f $(LIBDIR)/$$f ; \
	done
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $@ $(LDLIBS)
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
	done

endif


ifdef GUI
$(TARGET):	$(PWLIB_FILE)
endif

CLEAN_FILES	:= $(CLEAN_FILES) $(OBJDIR)/$(PROG) $(OBJDI)/$(PROG)_dll

# ifdef PROG
endif

######################################################################
#
# common rule to make a release of the program
#
######################################################################

ifdef DEBUG

release:
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
VERSION:=$(strip \
	$(subst \#define,, $(subst MAJOR_VERSION,,\
		$(shell grep "define *MAJOR_VERSION" custom.cxx)))).$(strip \
	$(subst \#define,,$(subst MINOR_VERSION,,\
		$(shell grep "define *MINOR_VERSION" custom.cxx))))$(strip \
	$(subst \#define,,$(subst BUILD_TYPE,,$(subst BetaCode,beta,$(subst ReleaseCode,pl,\
		$(shell grep "define *BUILD_TYPE" custom.cxx))))))$(strip \
	$(subst \#define,,$(subst BUILD_NUMBER,,\
		$(shell grep "define *BUILD_NUMBER" custom.cxx))))
endif
endif


ifndef VERSION

release :
	@echo Must define VERSION macro or have custom.cxx file.

else

RELEASEPROGDIR=$(RELEASEDIR)/$(RELEASEBASEDIR)

release: $(TARGET) releasefiles
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
	@$(MAKE) DEBUG= $(TARGET)

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

both: opt debug
clean: optclean debugclean
bothdepend: optdepend debugdepend


shared:
	$(MAKE) SHAREDLIB=1 

bothshared:
	$(MAKE) DEBUG= shared; $(MAKE) DEBUG=1 shared



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

ifdef GUI
ifdef RESOURCE
$(RESOBJS) : $(RESCXX) $(RESCODE)

$(RESCXX) : $(RESHDR)

TMPRSRC	= resource.tmp

$(RESCXX) $(RESCODE) $(RESHDR): $(RESOURCE)
	@if test -e $(RESHDR) ; then mv $(RESHDR) $(TMPRSRC) ; fi
	$(PWRC) -v $(RCFLAGS) $(RESOURCE)
	@if test -e $(TMPRSRC) && diff -q $(RESHDR) $(TMPRSRC) ; then cp $(TMPRSRC) $(RESHDR) ; else rm -f $(TMPRSRC) ;  fi

endif
endif


######################################################################
#
# Include all of the dependencies
#
######################################################################

-include $(DEPDIR)/*.dep


# End of common.mak

