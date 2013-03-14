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

ifndef PTLIBDIR
  $(error Must have PTLIBDIR defined to build plugins)
endif

include $(PTLIBDIR)/make/ptbuildopts.mak

PLUGIN_FILENAME = $(PLUGIN_NAME)_pwplugin.$(LIB_SUFFIX)

OBJDIR = $(PTLIB_LIBDIR)/$(PLUGIN_FAMILY)

TARGET = $(OBJDIR)/$(PLUGIN_FILENAME)

ifeq ($(target_os),solaris)
  LDSOPTS += -R$(PTLIB_LIBDIR) -G
else
  ifneq ($(target_os),Darwin)
    LDSOPTS += -shared
  endif
endif

ifeq ($(VERBOSE),)
Q = @echo $@ ;
endif


ifeq ($(target_cpu),x86_64)
  CPPFLAGS += -fPIC
endif

ifeq ($(target_cpu),hppa)
  CPPFLAGS += -fPIC
endif

ifeq ($(P_SHAREDLIB),1)
  PLUGIN_LIBS += $(PTLIB_LIBDIR)/$(PTLIB_FILE)
  CPPFLAGS += -DP_SHAREDLIB
endif

$(OBJDIR)/$(PLUGIN_FILENAME): $(PLUGIN_SOURCES)
	@mkdir -p $(OBJDIR)
	$(Q_CC)$(CXX) $(CPPFLAGS) $(CXXFLAGS) \
	$(LDSOPTS) $^ \
	$(PLUGIN_LIBS) \
	$(LDFLAGS) \
	-o $@

OBJS	 := $(patsubst %.c, $(OBJDIR)/%.o, $(patsubst %.cxx, $(OBJDIR)/%.o, $(patsubst %.cpp, $(OBJDIR)/%.o, $(notdir $(PLUGIN_SOURCES)))))

CLEAN_FILES += $(OBJDIR)/$(PLUGIN_FILENAME)

include $(PTLIBDIR)/make/common.mak
