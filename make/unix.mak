# end of user configurable items

#
#  clean whitespace out of source file list
#
SRCS		:= $(strip $(SOURCES))
# Revision 1.22  1998/09/24 04:20:53  robertj

#  defines for common Unix types
###############################################################################
#
# Linux for x86, using gcc 2.6.x
STDCCFLAGS	:= $(STDCCFLAGS) -DP_LINUX -DPBYTE_ORDER=PLITTLE_ENDIAN -DPCHAR8=PANSI_CHAR #-DPHAS_TEMPLATES
####################################################

#STDCCFLAGS	:= $(STDCCFLAGS) -DP_HPUX9
STDCCFLAGS	:= $(STDCCFLAGS) -DP_HPUX9
# Sun 4x, using gcc 2.6.2
#STDCCFLAGS	:= $(STDCCFLAGS) -DP_SUN4



###############################################################################
SHELL		= /bin/sh
CPLUS		:= g++
SHELL		:= /bin/sh
#
# if there is no PWLIBDIR variable set, then set one
#
ifndef PWLIBDIR
#PWLIBDIR	= /usr/local/pwlib
PWLIBDIR	= $(HOME)/pwlib
# define Posix threads stuff
ifdef P_PTHREADS
#
#  define names of some other programs we run
#
CPLUS		= g++
BISON		= bison
FLEX		= flex

#
endif
#
STDCCFLAGS	:= $(STDCCFLAGS) -Wall -m486
#STDCCFLAGS      := $(STDCCFLAGS) -fomit-frame-pointer
#STDCCFLAGS      := $(STDCCFLAGS) -fno-default-inline

# not normally used
STDCCFLAGS	:= $(STDCCFLAGS) -DPBYTE_ORDER=$(ENDIAN) -Wall
#STDCCFLAGS	:= $(STDCCFLAGS) -fomit-frame-pointer
#STDCCFLAGS	:= $(STDCCFLAGS) -fno-default-inline
#
# if using debug, add -g and set debug ID
#
ifdef	DEBUG

LIBID		= d
STDCCFLAGS	:= $(STDCCFLAGS) -DPMEMORY_CHECK=1
STDCCFLAGS	:= $(STDCCFLAGS) -g

else

LIBID		= r
OPTCCFLAGS	:= $(OPTCCFLAGS) -O2
#OPTCCFLAGS	:= $(OPTCCFLAGS) -fconserve-space
#OPTCCFLAGS	:= $(OPTCCFLAGS) -DP_USE_INLINES=1
LDFLAGS		:= $(LDFLAGS) -s

endif # DEBUG

OBJDIR		= obj_$(LIBID)

LIBDIR		= $(PWLIBDIR)/lib
COMMONDIR	= $(PWLIBDIR)/common

#
# add PW library directory to library path
#
LDFLAGS		:= $(LDFLAGS) -L$(LIBDIR) 

##########################################################################
#
#  set up for correct operating system
#

OS		= unix

#
# set name of the PT library
#
OSDIR		= $(PWLIBDIR)/$(OS)
PTLIB		= pt$(LIBID)_$(OS)
PTLIB_FILE	= $(LIBDIR)/lib$(PTLIB).a
VPATH_CXX	:= $(VPATH_CXX) $(OSDIR)/src 
VPATH_H		:= $(VPATH_H) $(OSDIR)/include

#
#STDCCFLAGS     := $(STDCCFLAGS) -Woverloaded-virtual
#
STDCCFLAGS	:= $(STDCCFLAGS) -I$(OSDIR)/include

#
# add OS library
#
LDLIBS		:= $(LDLIBS) -l$(PTLIB) 

##########################################################################
#
#  set up common
#

VPATH_CXX	:= $(VPATH_CXX) $(COMMONDIR)/src 
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
# define rule for .y files
#
.y.cxx:
	$(BISON) -dtv $*.y
	@mv $*.tab.c $*.cxx
	@mv $*.tab.h $*.h
	@if [ -r y.output ] ; then mv y.output $*.out ; fi

# 
# define rule for .y files
#
.l.cxx:
	$(FLEX) -t $*.l > $*.cxx


# End of unix.mak
