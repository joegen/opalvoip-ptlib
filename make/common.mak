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
# add in good files to delete
#
CLEAN_FILES	:= $(CLEAN_FILES) $(OBJS)
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

depend: $(SOURCES)
	@md -- $(STDCCFLAGS) -- $(SOURCES)
	@mv Makefile Makefile.bak
	@sed '/^# Do not delete/,$$s%$(PWLIBDIR)%$$(PWLIBDIR)%g' < Makefile.bak > Makefile

######################################################################
#
# common rules for cleaning up
#
######################################################################

clean:
	rm -rf $(CLEAN_FILES) obj_$(OBJ_SUFFIX)*


######################################################################
#
# common rule to make both debug and non-debug version
#
######################################################################

both:
	make DEBUG=; make DEBUG=1

shared:
	make SHAREDLIB=1 

bothshared:
	make DEBUG= shared; make DEBUG=1 shared

alllibs:
	make both
	make bothshared

static:
	for f in $(STATIC_LIBS) ; do \
	  rm -f $(LIBDIR)/$$f ; \
         ln -s $(SYSLIBDIR)/$$f $(LIBDIR)/$$f ; \
	done
	make DEBUG=
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

