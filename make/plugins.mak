#
# plugins.mak
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
# $Revision$
# $Author$
# $Date$
#

PTLIB_MAKE_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST))))

# Same place as this make file
include $(PTLIB_MAKE_DIR)/pre.mak


SOURCES = $(PLUGIN_SOURCES)
OBJDIR = $(PTLIB_LIBDIR)/$(PLUGIN_FAMILY)/$(PLUGIN_NAME)

SHARED_LIB_FILE_BASE = $(PLUGIN_NAME)$(PTLIB_PLUGIN_SUFFIX).$(SHAREDLIBEXT)
SHARED_LIB_FILE = $(OBJDIR)/$(SHARED_LIB_FILE_BASE)

CPPFLAGS += $(PLUGIN_CPPFLAGS) $(SHARED_CPPFLAGS)
LIBS := $(PLUGIN_LIBS) $(PTLIB_LIBS) $(LIBS)

include $(PTLIB_MAKE_DIR)/post.mak


INSTALL_DIR = $(DESTDIR)$(libdir)/$(PTLIB_PLUGIN_DIR)/$(PLUGIN_FAMILY)

install:
	$(MKDIR_P) $(INSTALL_DIR)
	$(INSTALL) -m 755 $(SHARED_LIB_FILE) $(INSTALL_DIR)/$(SHARED_LIB_FILE_BASE)

uninstall:
	rm $(INSTALL_DIR)/$(SHARED_LIB_FILE_BASE)


# End of file
