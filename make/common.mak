######################################################################
#
# common rule
#
######################################################################

VPATH_CXX	:= $(VPATH_CXX) 
VPATH_H		:= $(VPATH_H) $(COMMONDIR) 

vpath %.cxx $(VPATH_CXX)
vpath %.h   $(VPATH_H)
vpath %.o   $(OBJDIR)

#
# add common directory to include path - must be after PW and PT directories
#
STDCCFLAGS	:= $(STDCCFLAGS) -I$(COMMONDIR)

#
# add any trailing libraries
#
LDLIBS		:= $(LDLIBS) $(ENDLDLIBS)

#
# define rule for .cxx files
#
$(OBJDIR)/%.o : %.cxx 
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	$(CPLUS) $(STDCCFLAGS) $(OPTCCFLAGS) $(CFLAGS) -c $< -o $@

#
# create list of object files 
#
OBJS		:= $(SOURCES:cxx=o) $(OBJS)
OBJS		:= $(EXTERNALOBJS) $(patsubst %.o, $(OBJDIR)/%.o, $(notdir $(OBJS)))

#
# create list of dependency files 
#
DEPDIR		:= $(OBJDIR)
DEPS		:= $(SOURCES:cxx=dep)
DEPS		:= $(patsubst %.dep, $(DEPDIR)/%.dep, $(notdir $(DEPS)))

#
# define rule for .dep files
#
$(DEPDIR)/%.dep : %.cxx 
	@if [ ! -d $(DEPDIR) ] ; then mkdir $(DEPDIR) ; fi
	@printf %s $(OBJDIR)/ > $@
	$(CPLUS) $(STDCCFLAGS) -M $< >> $@

#
# add in good files to delete
#
CLEAN_FILES	:= $(CLEAN_FILES) $(OBJS) $(DEPS)
CLEAN_FILES	:= $(CLEAN_FILES) core

######################################################################
#
# rules for application
#
######################################################################

ifdef	PROG

ifndef	SHAREDLIB
all:	$(OBJDIR)/$(PROG)
else
all:	$(OBJDIR)/$(PROG)_dll
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
	$(MAKE) DEBUG=; $(MAKE) DEBUG=1

shared:
	$(MAKE) SHAREDLIB=1 

bothshared:
	$(MAKE) DEBUG= shared; $(MAKE) DEBUG=1 shared

alllibs:
	$(MAKE) both
	$(MAKE) bothshared

static:
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
         ln -s $(SYSLIBDIR)/$$f $(LIBDIR)/$$f ; \
	done
	$(MAKE) DEBUG=
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
	done

######################################################################
#
# setup the lib directory
#
######################################################################
libdir:
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	ln -s ../unix/src/unix.mak $(LIBDIR)/unix.mak
	ln -s ../unix/src/common.mak $(LIBDIR)/common.mak
	ln -s ../unix/src/ptlib.mak $(LIBDIR)/ptlib.mak
	ln -s ../xlib/src/xlib.mak $(LIBDIR)/xlib.mak
	ln -s ../xlib/src/pwlib.mak $(LIBDIR)/pwlib.mak

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

$(RESCXX) $(RESCODE) $(RESHDR): $(RESOURCE)
	$(PWRC) -v $(PFLAGS) $(RESOURCE)

endif
endif


#
# Include all of the dependencies
#
-include $(DEPDIR)/*.dep

