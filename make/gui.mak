#
# gui.mak
#
# Second part of make rules, included in ptlib.mak and pwlib.mak.
# Note: Do not put any targets in the file. This should defaine variables
#       only, as targets are all in common.mak
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
# $Log: gui.mak,v $
# Revision 1.5  1998/12/02 02:37:06  robertj
# New directory structure.
#


ifndef GUI
GUI = xlib
endif


include $(PWLIBDIR)/make/$(GUI).mak


OBJDIR          = obj_$(GUI)_$(OBJ_SUFFIX)_$(LIBID)

GUI_INC_DIR	= $(PWLIBDIR)/include/pwlib/$(GUI)

ifndef GUI_LIB_SUFFIX
GUI_LIB_SUFFIX  = $(OBJ_SUFFIX)
endif

PWLIB           = pw_$(GUI)_$(LIB_SUFFIX)_$(LIBID)

ifndef SHAREDLIB
PWLIB_FILE      = $(LIBDIR)/lib$(PWLIB).a
else
PWLIB_FILE      = $(LIBDIR)/lib$(PWLIB).so
endif

#
# add OS directory to include path
#
STDCCFLAGS      := $(STDCCFLAGS) -I$(GUI_INC_DIR)

#
# add OS library
#
LDLIBS	:= -l$(PWLIB) $(GUILIB) $(LDLIBS)


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
RCFLAGS		:= $(RCFLAGS) -I "$(GUI_INC_DIR);$(PWLIBDIR)/include"

#
# create list of source files containing resource file if required
#
SOURCES		:= $(RESCODE) $(RESCXX) $(SOURCES)

#
# create list of object files containing resource file if required
#
CLEAN_FILES	:= $(CLEAN_FILES) $(RESHDR) $(RESCODE) $(RESCXX) $(RESOBJS)

endif


# End of gui.mak
