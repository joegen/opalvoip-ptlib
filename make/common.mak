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

ifndef	SHAREDLIB
all ::	$(OBJDIR)/$(PROG)
else
all ::	$(OBJDIR)/$(PROG)_dll
endif

ifdef BUILDFILES
OBJS	:= $(OBJS) $(OBJDIR)/buildnum.o
endif

$(OBJDIR)/$(PROG):	$(OBJS) $(PTLIB_FILE)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(OBJDIR)/$(PROG) $(LDLIBS)

$(OBJDIR)/$(PROG)_dll:	$(OBJS) $(PTLIB_FILE)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(OBJDIR)/$(PROG)_dll $(LDLIBS)

ifdef GUI
$(OBJDIR)/$(PROG):	$(PWLIB_FILE)

$(OBJDIR)/$(PROG)_dll:	$(PWLIB_FILE)
endif

CLEAN_FILES	:= $(CLEAN_FILES) $(OBJDIR)/$(PROG) $(OBJDIR)/$(PROG)_dll

# ifdef PROG
endif

######################################################################
#
# common rules for creating dependencies
#
######################################################################

.DELETE_ON_ERROR : depend

depend: $(DEPS)
	@echo Created dependencies.


######################################################################
#
# common rules for cleaning up
#
######################################################################

clean:
	rm -rf $(CLEAN_FILES) obj_$(OBJ_SUFFIX)*


######################################################################
#
# common rule to make a release of the program
#
######################################################################

ifndef RELEASEDIR
RELEASEDIR=releases
endif

ifndef RELEASEPROGDIR
RELEASEPROGDIR=$(PROG)
endif

ifdef VERSION
ifdef DEBUG
release:
	$(MAKE) DEBUG= release
else
release: $(OBJDIR)/$(PROG)
	cp $(OBJDIR)/$(PROG) $(RELEASEDIR)/$(RELEASEPROGDIR)
	cd $(RELEASEDIR) ; tar cf - $(RELEASEPROGDIR) | gzip > $(PROG)_$(VERSION)_$(OBJ_SUFFIX).tar.gz
endif
else
release:
	echo You must define a VERSION macro.
endif


######################################################################
#
# common rule to make both debug and non-debug version
#
######################################################################

both:
	$(MAKE) DEBUG=1; $(MAKE) DEBUG=

shared:
	$(MAKE) SHAREDLIB=1 

bothshared:
	$(MAKE) DEBUG= shared; $(MAKE) DEBUG=1 shared

bothdepend:
	$(MAKE) DEBUG= depend; $(MAKE) DEBUG=1 depend

static:
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
         ln -s $(SYSLIBDIR)/$$f $(LIBDIR)/$$f ; \
	done
	$(MAKE) DEBUG=
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
	done

libs:
	$(MAKE) -C $(PWLIBDIR)/src/ptlib/unix
ifdef GUI
	$(MAKE) -C $(PWLIBDIR)/src/pwlib/$(GUI)
endif


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

