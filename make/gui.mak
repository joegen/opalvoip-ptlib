ifndef GUI
GUI		= xlib
endif

OBJDIR          = obj_$(GUI)_$(OBJ_SUFFIX)_$(LIBID)

ifndef GUI_LIB_SUFFIX
GUI_LIB_SUFFIX  = $(OBJ_SUFFIX)
endif

GUIDIR          = $(PWLIBDIR)/$(GUI)
PWLIB           = pw_$(GUI)_$(LIB_SUFFIX)_$(LIBID)

ifndef SHAREDLIB
PWLIB_FILE      = $(LIBDIR)/lib$(PWLIB).a
else
PWLIB_FILE      = $(LIBDIR)/lib$(PWLIB).so
endif

VPATH_CXX       := $(GUIDIR)/src $(COMMONDIR)/pwlib/src $(VPATH_CXX)
VPATH_H         := $(GUIDIR)/include $(VPATH_H)

#
# add OS directory to include path
#
STDCCFLAGS      := -I$(GUIDIR)/include $(STDCCFLAGS)

#
# add OS library
#
LDLIBS	:= $(GUILIBS) $(LDLIBS)


#
#  rules for resource compilation
#
PWRC		= $(PWLIBDIR)/tools/pwrc/obj_$(OBJ_SUFFIX)_r/pwrc -a $(GUI)

#
# if we are using a resource file, then define the required files
#
ifdef RESOURCE
RESHDR		= $(RESOURCE:prc=h)
RESCODE		= $(RESOURCE:prc=res.cxx)
RESCXX		= $(RESOURCE:prc=cxx)
RESOBJS		= $(RESCXX:.cxx=.o) $(RESCODE:.cxx=.o)
PFLAGS		:= $(PFLAGS) -I "$(GUIDIR)/include;$(COMMONDIR)"
endif

ifdef RESOURCE
#
# create list of source files containing resource file if required
#
SOURCES		:= $(SOURCES) $(RESCODE) $(RESCXX)

#
# create list of object files containing resource file if required
#
CLEAN_FILES	:= $(CLEAN_FILES) $(RESHDR) $(RESCODE) $(RESCXX) $(RESOBJS)
endif
