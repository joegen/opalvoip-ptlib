#
# pre.mak
#
# Common make rules included in various places, part 1
#
# Portable Tools Library
#
# Copyright (c) 1993-2013 Equivalence Pty. Ltd.
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

PTLIB_CONFIG_MAK := ptlib_config.mak
ifneq (,$(PTLIB_PLATFORM_DIR))
  include $(PTLIB_PLATFORM_DIR)/make/$(PTLIB_CONFIG_MAK)
else ifdef PTLIBDIR
  include $(PTLIBDIR)/make/$(PTLIB_CONFIG_MAK)
else
  include $(shell pkg-config ptlib --variable=makedir)/$(PTLIB_CONFIG_MAK)
endif


ifeq ($(P_SHAREDLIB),0)
  STATIC_BUILD:=yes
endif

ifndef STATIC_BUILD
  STATIC_BUILD:=no
endif

ifdef DEBUG
  DEBUG_BUILD:=yes
endif

ifndef DEBUG_BUILD
  DEBUG_BUILD:=no
endif


ifeq ($(V)$(VERBOSE),)
  ifeq ($(DEBUG_BUILD),yes)
    Q_SUFFIX := " dbg"
  endif
  Q   := @
  Q_CC = @echo [ CC$(Q_SUFFIX)] $(subst $(CURDIR)/,,$<) ; 
  Q_CXX= @echo [CXX$(Q_SUFFIX)] $(subst $(CURDIR)/,,$<) ; 
  Q_DEP= @echo [DEP$(Q_SUFFIX)] $(subst $(CURDIR)/,,$<) ; 
  Q_AR = @echo [ AR$(Q_SUFFIX)] $(subst $(CURDIR)/,,$@) ; 
  Q_LD = @echo [ LD$(Q_SUFFIX)] $(subst $(CURDIR)/,,$@) ; 
  Q_MAKE = @$(MAKE) --no-print-directory
else
  Q_MAKE = $(MAKE) --print-directory
endif


###############################################################################
#
# Determine the library name
#

ifeq ($(DEBUG_BUILD),yes)
  OBJ_SUFFIX := _d
else
  OBJ_SUFFIX :=
endif

ifndef OBJDIR_SUFFIX
  OBJDIR_SUFFIX = $(OBJ_SUFFIX)
endif

ifneq (,$(PTLIB_PLATFORM_DIR))
  PTLIB_LIBDIR = $(PTLIB_PLATFORM_DIR)
else ifdef PTLIBDIR
  PTLIB_LIBDIR = $(PTLIBDIR)/lib_$(target)
else
  PTLIB_LIBDIR = $(shell pkg-config ptlib --variable=libdir)
endif


PTLIB_OBJDIR = $(PTLIB_LIBDIR)/obj$(OBJDIR_SUFFIX)

PTLIB_PREFIX := pt
PTLIB_SHARED_BASE = lib$(PTLIB_PREFIX)
PTLIB_STATIC_FILE = $(PTLIB_SHARED_BASE)_s.$(STATICLIBEXT)
PTLIB_SHARED_LINK = $(PTLIB_SHARED_BASE).$(SHAREDLIBEXT)
PTLIB_DEBUG_SHARED_BASE = lib$(PTLIB_PREFIX)_d
PTLIB_DEBUG_STATIC_FILE = $(PTLIB_DEBUG_SHARED_BASE)_s.$(STATICLIBEXT)
PTLIB_DEBUG_SHARED_LINK = $(PTLIB_DEBUG_SHARED_BASE).$(SHAREDLIBEXT)

PTLIB_FILE_VERSION = $(PTLIB_MAJOR).$(PTLIB_MINOR)$(PTLIB_STAGE)$(PTLIB_BUILD)

ifneq (,$(findstring $(target_os),Darwin cygwin mingw))
  PTLIB_SHARED_FILE = $(PTLIB_SHARED_BASE).$(PTLIB_FILE_VERSION).$(SHAREDLIBEXT)
  PTLIB_DEBUG_SHARED_FILE = $(PTLIB_DEBUG_SHARED_BASE).$(PTLIB_FILE_VERSION).$(SHAREDLIBEXT)
else
  PTLIB_SHARED_FILE = $(PTLIB_SHARED_BASE).$(SHAREDLIBEXT).$(PTLIB_FILE_VERSION)
  PTLIB_DEBUG_SHARED_FILE = $(PTLIB_DEBUG_SHARED_BASE).$(SHAREDLIBEXT).$(PTLIB_FILE_VERSION)
endif


ifdef PTLIBDIR
  # Submodules built with make lib
  LIBDIRS += $(PTLIBDIR)
endif


###############################################################################
#
# Add common directory to include path
# Note also have include directory that is always relative to the
# ptlib_config.mak file in PTLIB_PLATFORM_INC_DIR
#

ifdef PTLIBDIR
  CPPFLAGS := -I$(PTLIBDIR)/include $(CPPFLAGS)
else
  CPPFLAGS := $(shell pkg-config ptlib --cflags-only-I) $(CPPFLAGS)
endif

ifeq (,$(findstring $(PTLIB_PLATFORM_INC_DIR),$(CPPFLAGS)))
  CPPFLAGS := -I$(PTLIB_PLATFORM_INC_DIR) $(CPPFLAGS)
endif


###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#

ifeq ($(DEBUG_BUILD),yes)
  CPPFLAGS += $(DEBUG_FLAGS)
  LDFLAGS  := $(DEBUG_FLAGS) $(LDFLAGS)
else
  CPPFLAGS += $(OPT_FLAGS)
  LDFLAGS  := $(OPT_FLAGS) $(LDFLAGS)
endif

