#
# lib.mak
#
# Make rules for building libraries rather than applications.
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
# Contributor(s): ______________________________________.
#       
# $Log: lib.mak,v $
# Revision 1.15  2001/11/27 22:42:13  robertj
# Changed to make system to better support non-shared library building.
#
# Revision 1.14  2001/07/07 06:51:52  robertj
# Added fix to remove shared libraries in make clean, thanks Jac Goudsmit
#
# Revision 1.13  2001/07/03 04:41:25  yurik
# Corrections to Jac's submission from 6/28
#
# Revision 1.12  2001/06/30 06:59:06  yurik
# Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
#
# Revision 1.11  2001/03/23 03:18:01  robertj
# Fixed addition of trailing dot at end of release versions of so libraries
#   caused to removal of the "pl" build type, changing it to a dot.
#
# Revision 1.10  2001/03/14 06:24:55  robertj
# Fixed setting of symlinks for shared libraries to be relative paths.
#
# Revision 1.9  2000/05/19 01:26:31  robertj
# Added copyright notice
#


LIBNAME_MAJ		= $(LIB_FILENAME).$(MAJOR_VERSION)
LIBNAME_MIN		= $(LIBNAME_MAJ).$(MINOR_VERSION)
LIBNAME_PAT		= $(LIBNAME_MIN).$(BUILD_NUMBER)$(BUILD_TYPE)

CLEAN_FILES += $(LIBDIR)/$(LIBNAME_PAT) $(LIBDIR)/$(LIB_FILENAME) $(LIBDIR)/$(LIBNAME_MAJ) $(LIBDIR)/$(LIBNAME_MIN)

ifneq ($(P_SHAREDLIB),1)
STATIC_LIB_FILE=$(LIBDIR)/$(LIB_FILENAME)
else
STATIC_LIB_FILE=$(LIBDIR)/$(LIB_BASENAME)_s.a

ifndef MAJOR_VERSION
MAJOR_VERSION	:= 1
endif

ifndef MINOR_VERSION
MINOR_VERSION	:= 0
endif

ifndef BUILD_TYPE
BUILD_TYPE	:= a
else
BUILD_TYPE	:= $(subst .,,$(subst beta,b,$(subst alpha,a,$(BUILD_TYPE))))
endif

ifndef BUILD_NUMBER
BUILD_NUMBER	:= 0
endif

ifeq ($(OSTYPE),beos)
# BeOS requires different options when building shared libraries
# Also, when building a shared library x that references symbols in libraries y,
# the y libraries need to be added to the linker command
LDSOOPTS = -nostdlib -nostart
EXTLIBS = $(SYSLIBS) -lstdc++.r4
else
LDSOOPTS = -shared
EXTLIBS =
endif

$(LIBDIR)/$(LIB_FILENAME): $(LIBDIR)/$(LIBNAME_PAT)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIB_FILENAME)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIBNAME_MAJ)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIBNAME_MIN)

$(LIBDIR)/$(LIBNAME_PAT): $(STATIC_LIB_FILE)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	gcc $(LDSOOPTS) -Wl,-soname,$(LIB_FILENAME).1 -o $(LIBDIR)/$(LIBNAME_PAT) $(EXTLIBS) $(OBJS)

install: $(LIBDIR)/$(LIBNAME_PAT)
	$(INSTALL) $(LIBDIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_PAT)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIB_FILENAME)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_MAJ)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_MIN)

endif # P_SHAREDLIB


$(STATIC_LIB_FILE): $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
ifdef P_USE_RANLIB
	$(AR) rc $(STATIC_LIB_FILE) $(OBJS)
	$(RANLIB) $(STATIC_LIB_FILE)
else
	$(AR) rcs $(STATIC_LIB_FILE) $(OBJS)
endif



# End of file ################################################################

