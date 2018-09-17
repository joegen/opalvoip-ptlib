#
# autoconf.mak
#
# Make file for ptlib library using autoconf
#
# Portable Tools Library
#
# Copyright (c) 2013 Equivalence Pty. Ltd.
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

ifndef TOP_LEVEL_MAKE
  TOP_LEVEL_MAKE := $(abspath $(TOP_LEVEL_DIR)/make/toplevel.mak)
endif

ifndef M4_FILES
  M4_FILES := $(wildcard $(TOP_LEVEL_DIR)/make/*.m4)
endif

ifndef CONFIGURE
  CONFIGURE := $(TOP_LEVEL_DIR)/configure
endif

ifndef AUTOCONF
  AUTOCONF := autoconf
endif

ifndef AUTOCONF_VERSION
  AUTOCONF_VERSION :=68
endif

ifndef ACLOCAL
  ACLOCAL := aclocal
endif

ifndef ACLOCAL_M4
  ACLOCAL_M4 := $(TOP_LEVEL_DIR)/aclocal.m4
endif


# XCode sets this, don't want it
ifeq ($(OS),MACOS)
  OS:=
endif

ifeq ($(OS),)

  ifneq ($(CPU),)
    $(error Must define an OS for CPU=$(CPU))
  endif
  
  ifndef TARGETDIR
    ifeq ($(MULTIPLATFORM),yes)
      TARGETDIR := lib_$(shell uname -s)_$(shell uname -m)
    else
      TARGETDIR := $(CURDIR)
    endif
  endif
  
else # OS
  
  # Of course, Apple are always different
  ifeq ($(OS),iPhoneOS)
    CPU := armv7
    VENDOR := apple
    ABI = Mach-O
    HOST_CONFIG_PARAM := --enable-ios=iphone
  else ifeq ($(OS),iPhoneSimulator)
    CPU := x86
    VENDOR := apple
    ABI = Mach-O
    HOST_CONFIG_PARAM := --enable-ios=simulator
  else ifeq ($(CPU),)
    $(error Must define a CPU for OS=$(OS))
  endif

  ifeq ($(VENDOR),)
    VENDOR := none
  endif
  
  ifeq ($(ABI),)
    ABI=gnueabi
  endif

  ifeq ($(HOST),)
    HOST := $(CPU)-$(VENDOR)-$(OS)-$(ABI)
  endif

  ifndef HOST_CONFIG_PARAM
    HOST_CONFIG_PARAM := --host=$(HOST)
  endif
  
  ifndef TARGETDIR
    TARGETDIR := $(CURDIR)/lib_$(OS)_$(CPU)
  endif
  
  $(info Cross compile: OS=$(OS), CPU=$(CPU), HOST=$(HOST))
endif # OS


ifndef CONFIG_STATUS
  CONFIG_STATUS := $(TARGETDIR)/config.status
endif

ifndef CONFIG_PARMS
  ifneq (,$(wildcard $(CONFIG_STATUS)))
    CONFIG_PARMS = $(shell $(CONFIG_STATUS) --config)
  endif
endif

CONFIG_FILE_PATHS := $(addprefix $(TARGETDIR)/, $(CONFIG_FILES))

CONFIGURE_CMD := \
  if test ! -d "$(TARGETDIR)" ; then \
    mkdir $(TARGETDIR); \
  fi; \
  cd $(TARGETDIR); \
  $(CONFIGURE) $(HOST_CONFIG_PARAM) $(CONFIG_PARMS)



ifeq ($(V)$(VERBOSE),)
  Q := @
endif


# The following goals do not generate a call to configure
NO_CONFIG_GOALS += clean distclean config


# Everything other than the NO_CONFIG_GOALS depends on the configuration
ifneq (,$(MAKECMDGOALS))
  $(filter-out $(NO_CONFIG_GOALS),$(MAKECMDGOALS)) : $(CONFIG_FILE_PATHS) build_top_level
else
  .PHONY:default_goal
  default_goal : $(CONFIG_FILE_PATHS) build_top_level
endif


.PHONY:clean
.PHONY:distclean
ifneq (,$(wildcard $(CONFIG_STATUS)))
  clean distclean :: build_top_level
else
  clean ::
	@echo Cannot make clean until configured.
endif

distclean ::
	rm -f $(CONFIG_FILE_PATHS) $(CONFIG_STATUS)


.PHONY:config
config : $(CONFIGURE)
	$(CONFIGURE_CMD)


.PHONY:build_top_level
build_top_level:
	$(Q)$(MAKE) --file="$(TOP_LEVEL_MAKE)" --directory="$(TARGETDIR)" $(MAKECMDGOALS)


$(CONFIG_STATUS) : $(CONFIGURE) $(addprefix $(TOP_LEVEL_DIR)/,$(addsuffix .in,$(CONFIG_FILES)))
	$(CONFIGURE_CMD)

$(CONFIG_FILE_PATHS) : $(CONFIG_STATUS)
	@if test \! -f $@; then $(MAKE) $(AM_MAKEFLAGS) config; fi


ifeq ($(shell which $(AUTOCONF) > /dev/null && \
              which $(ACLOCAL) > /dev/null && \
              test `autoconf --version | sed -n "s/autoconf.*2.\\([0-9]*\\)/\\1/p"` -ge $(AUTOCONF_VERSION) \
              ; echo $$?),0)
  AUTOCONF_AVAILABLE := yes
else
  AUTOCONF_AVAILABLE := no
endif

ifeq ($(AUTOCONF_AVAILABLE),yes)

  $(CONFIGURE): $(CONFIGURE).ac $(M4_FILES) $(ACLOCAL_M4)
	$(AUTOCONF)

  $(ACLOCAL_M4):
	cd $(dir $@) && $(ACLOCAL)

else # autoconf

  $(CONFIGURE): $(CONFIGURE).ac
	@echo ---------------------------------------------------------------------
	@echo The configure script requires updating but autoconf not is installed.
	@echo Either install autoconf v2.$(AUTOCONF_VERSION) or later or execute the command:
	@echo touch $@
	@echo ---------------------------------------------------------------------

endif # autoconf good


# End of autoconf.mak
