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
OBJS		:= $(OBJS) $(SOURCES:cxx=o)
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

all:	$(OBJDIR)/$(PROG)

ifdef BUILDFILES
OBJS	:= $(OBJS) buildnum.o
endif

$(OBJDIR)/$(PROG):	$(OBJS) $(PTLIB_FILE)
	$(CPLUS) $(CFLAGS) $(LDFLAGS) $(OBJS) -o $(OBJDIR)/$(PROG) $(LDLIBS)

ifdef GUI
$(OBJDIR)/$(PROG):	$(PWLIB_FILE)
endif


CLEAN_FILES	:= $(CLEAN_FILES) $(OBJDIR)/$(PROG)

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
buildnum.o:	buildnum.c
	cc -o buildnum.o -c buildnum.c

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

