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
# Revision 1.10  2001/03/14 06:24:55  robertj
# Fixed setting of symlinks for shared libraries to be relative paths.
#
# Revision 1.9  2000/05/19 01:26:31  robertj
# Added copyright notice
#


ifeq ($(P_SHAREDLIB),1)

ifndef MAJOR_VERSION
MAJOR_VERSION	:= 1
endif

ifndef MINOR_VERSION
MINOR_VERSION	:= 0
endif

ifndef BUILD_TYPE
BUILD_TYPE	:= a
else
BUILD_TYPE	:= $(subst pl,,$(subst beta,b,$(subst alpha,a,$(BUILD_TYPE))))
endif

ifndef BUILD_NUMBER
BUILD_NUMBER	:= 0
endif

LIBNAME_MAJ		= $(LIB_BASENAME).$(MAJOR_VERSION)
LIBNAME_MIN		= $(LIBNAME_MAJ).$(MINOR_VERSION)
LIBNAME_PAT		= $(LIBNAME_MIN).$(BUILD_NUMBER)$(BUILD_TYPE)

$(LIBDIR)/$(LIB_BASENAME): $(LIBDIR)/$(LIBNAME_PAT)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIB_BASENAME)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIBNAME_MAJ)
	cd $(LIBDIR) ; ln -sf $(LIBNAME_PAT) $(LIBNAME_MIN)

$(LIBDIR)/$(LIBNAME_PAT): $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	gcc -shared -Wl,-soname,$(LIB_BASENAME).1 -o $(LIBDIR)/$(LIBNAME_PAT) $(OBJS)

CLEAN_FILES += $(LIBDIR)/$(LIBNAME_PAT) $(LIBDIR)/$(LIB_BASENAME) $(LIBDIR)/$(LIBNAME_MAJ) $(LIBDIR)/$(LIBNAME_MIN)

install: $(LIBDIR)/$(LIBNAME_PAT)
	$(INSTALL) $(LIBDIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_PAT)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIB_BASENAME)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_MAJ)
	ln -s $(INSTALLLIB_DIR)/$(LIBNAME_PAT) $(INSTALLLIB_DIR)/$(LIBNAME_MIN)

else

$(LIBDIR)/$(LIB_BASENAME): $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
ifdef P_USE_RANLIB
	$(AR) rc $(LIBDIR)/$(LIB_BASENAME) $(OBJS)
	$(RANLIB) $(LIBDIR)/$(LIB_BASENAME)
else
	$(AR) rcs $(LIBDIR)/$(LIB_BASENAME) $(OBJS)
endif

CLEAN_FILES += $(LIBDIR)/$(LIB_BASENAME)

endif

