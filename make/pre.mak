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

ifndef PTLIB_PRE_INCLUDED
PTLIB_PRE_INCLUDED:=1

PTLIB_TOP_LEVEL_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))..)

PTLIB_CONFIG_MAK := ptlib_config.mak
ifneq ($(PTLIB_PLATFORM_DIR),)
  include $(PTLIB_PLATFORM_DIR)/make/$(PTLIB_CONFIG_MAK)
  PTLIB_INCFLAGS := -I$(PTLIB_TOP_LEVEL_DIR)/include
  ifeq ($(PTLIB_TOP_LEVEL_DIR),$(PTLIB_PLATFORM_DIR))
    PTLIB_LIBDIR = $(PTLIB_PLATFORM_DIR)/lib_$(target)
  else
    PTLIB_LIBDIR = $(PTLIB_PLATFORM_DIR)
  endif
else ifndef PTLIBDIR
  include $(shell pkg-config ptlib --variable=makedir)/$(PTLIB_CONFIG_MAK)
  PTLIB_INCFLAGS := $(shell pkg-config ptlib --cflags-only-I)
  PTLIB_LIBDIR := $(shell pkg-config ptlib --variable=libdir)
else
  ifeq (,$(target))
    ifneq (,$(OS))
      ifneq (,$(CPU))
        target = $(OS)_$(CPU)
      endif
    endif
  endif
  ifneq (,$(wildcard $(PTLIBDIR)/lib_$(target)/make/$(PTLIB_CONFIG_MAK)))
    include $(PTLIBDIR)/lib_$(target)/make/$(PTLIB_CONFIG_MAK)
  else
    include $(PTLIBDIR)/make/$(PTLIB_CONFIG_MAK)
  endif
  PTLIB_INCFLAGS := -I$(PTLIBDIR)/include
  PTLIB_LIBDIR = $(PTLIBDIR)/lib_$(target)
  LIBDIRS := $(PTLIBDIR) $(LIBDIRS)  # Submodules built with make lib
endif


ifeq ($(P_SHAREDLIB),0)
  STATIC_BUILD:=yes
endif

ifeq ($(P_PROFILING),yes)
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

DEBUG_SUFFIX := _d
STATIC_SUFFIX := _s

ifeq ($(DEBUG_BUILD),yes)
  LIB_DEBUG_SUFFIX := $(DEBUG_SUFFIX)
else
  LIB_DEBUG_SUFFIX :=
endif

ifeq ($(STATIC_BUILD),yes)
  LIB_STATIC_SUFFIX := $(STATIC_SUFFIX)
else
  LIB_STATIC_SUFFIX :=
endif

ifeq ($(OBJDIR_SUFFIX)$(DEBUG_BUILD),yes)
  OBJDIR_SUFFIX := $(DEBUG_SUFFIX)
endif

PTLIB_OBJDIR = $(PTLIB_LIBDIR)/obj$(OBJDIR_SUFFIX)

PTLIB_LIB_BASE           := pt
PTLIB_LIB_FILE_BASE      := lib$(PTLIB_LIB_BASE)
PTLIB_OPT_LIB_FILE_BASE   = $(PTLIB_LIBDIR)/$(PTLIB_LIB_FILE_BASE)
PTLIB_OPT_SHARED_LINK     = $(PTLIB_OPT_LIB_FILE_BASE).$(SHAREDLIBEXT)
PTLIB_OPT_STATIC_FILE     = $(PTLIB_OPT_LIB_FILE_BASE)$(STATIC_SUFFIX).$(STATICLIBEXT)
PTLIB_DEBUG_LIB_FILE_BASE = $(PTLIB_LIBDIR)/$(PTLIB_LIB_FILE_BASE)$(DEBUG_SUFFIX)
PTLIB_DEBUG_SHARED_LINK   = $(PTLIB_DEBUG_LIB_FILE_BASE).$(SHAREDLIBEXT)
PTLIB_DEBUG_STATIC_FILE   = $(PTLIB_DEBUG_LIB_FILE_BASE)$(STATIC_SUFFIX).$(STATICLIBEXT)

PTLIB_FILE_VERSION = $(PTLIB_MAJOR).$(PTLIB_MINOR)$(PTLIB_STAGE)$(PTLIB_BUILD)

ifneq (,$(findstring $(target_os),Darwin cygwin mingw))
  PTLIB_OPT_SHARED_FILE   = $(PTLIB_OPT_LIB_FILE_BASE).$(PTLIB_FILE_VERSION).$(SHAREDLIBEXT)
  PTLIB_DEBUG_SHARED_FILE = $(PTLIB_DEBUG_LIB_FILE_BASE).$(PTLIB_FILE_VERSION).$(SHAREDLIBEXT)
else
  PTLIB_OPT_SHARED_FILE   = $(PTLIB_OPT_LIB_FILE_BASE).$(SHAREDLIBEXT).$(PTLIB_FILE_VERSION)
  PTLIB_DEBUG_SHARED_FILE = $(PTLIB_DEBUG_LIB_FILE_BASE).$(SHAREDLIBEXT).$(PTLIB_FILE_VERSION)
endif

ifeq ($(DEBUG_BUILD),yes)
  PTLIB_STATIC_LIB_FILE = $(PTLIB_DEBUG_STATIC_FILE)
  PTLIB_SHARED_LIB_LINK = $(PTLIB_DEBUG_SHARED_LINK)
  PTLIB_SHARED_LIB_FILE = $(PTLIB_DEBUG_SHARED_FILE)
else
  PTLIB_STATIC_LIB_FILE = $(PTLIB_OPT_STATIC_FILE)
  PTLIB_SHARED_LIB_LINK = $(PTLIB_OPT_SHARED_LINK)
  PTLIB_SHARED_LIB_FILE = $(PTLIB_OPT_SHARED_FILE)
endif

PTLIB_LIBS = -L$(PTLIB_LIBDIR) -l$(PTLIB_LIB_BASE)$(LIB_DEBUG_SUFFIX)$(LIB_STATIC_SUFFIX)


###############################################################################
# Add common directory to include path
# Note also have include directory that is always relative to the
# ptlib_config.mak file in PTLIB_PLATFORM_INC_DIR
#

ifeq (,$(findstring $(PTLIB_INCFLAGS),$(CPPFLAGS)))
  CPPFLAGS := $(PTLIB_INCFLAGS) $(CPPFLAGS)
endif

ifeq (,$(findstring $(PTLIB_PLATFORM_INC_DIR),$(CPPFLAGS)))
  CPPFLAGS := -I$(PTLIB_PLATFORM_INC_DIR) $(CPPFLAGS)
endif


###############################################################################
#
# Set up compiler flags and macros for debug/release versions
#

SEPARATE_DEBUG_INFO:=no

ifeq ($(DEBUG_BUILD),yes)
  CPPFLAGS += $(DEBUG_CPPFLAGS)
  CXXFLAGS += $(DEBUG_CFLAGS)
  CFLAGS   += $(DEBUG_CFLAGS)
  LDFLAGS  := $(DEBUG_CFLAGS) $(LDFLAGS)
else
  CPPFLAGS += $(OPT_CPPFLAGS)
  CXXFLAGS += $(OPT_CFLAGS)
  CFLAGS   += $(OPT_CFLAGS)
  LDFLAGS  := $(OPT_CFLAGS) $(LDFLAGS)
  ifneq ($(STRIP),)
    ifneq ($(DSYMUTIL)$(OBJCOPY),)
      CXXFLAGS += $(DEBUG_CFLAGS)
      CFLAGS   += $(DEBUG_CFLAGS)
      LDFLAGS  := $(DEBUG_CFLAGS) $(LDFLAGS)
      SEPARATE_DEBUG_INFO:=yes
    endif
  endif
endif

endif # PTLIB_PRE_INCLUDED
