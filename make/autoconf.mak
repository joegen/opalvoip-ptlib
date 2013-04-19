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
# $Revision$
# $Author$
# $Date$
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

ifndef CONFIG_STATUS
  CONFIG_STATUS := $(CURDIR)/config.status
endif

ifndef AUTOCONF
  AUTOCONF := autoconf
endif

ifndef AUTOCONF_VERSION
  AUTOCONF_VERSION :=38
endif

ifndef ACLOCAL
  ACLOCAL := aclocal
endif

ifndef ACLOCAL_M4
  ACLOCAL_M4 := $(TOP_LEVEL_DIR)/aclocal.m4
endif

ifndef CONFIG_PARMS
  ifneq (,$(wildcard $(CONFIG_STATUS)))
    CONFIG_PARMS = $(shell $(CONFIG_STATUS) --config)
  endif
endif

ifndef PRIMARY_CONFIG_FILE
  PRIMARY_CONFIG_FILE := $(firstword $(CONFIG_FILES))
endif

ifeq ($(V)$(VERBOSE),)
  Q := @
endif


# The following goals do not generate a call to configure
NO_CONFIG_GOALS += clean distclean config


# Everything other than the NO_CONFIG_GOALS depends on the configuration
ifneq (,$(MAKECMDGOALS))
  $(filter-out $(NO_CONFIG_GOALS),$(MAKECMDGOALS)) : $(CONFIG_FILES) build_top_level
else
  .PHONY:default
  default : $(CONFIG_FILES) build_top_level
endif


.PHONY:clean
.PHONY:distclean
ifneq (,$(wildcard $(PRIMARY_CONFIG_FILE)))
  clean distclean :: build_top_level
else
  clean ::
	@echo Cannot make clean until configured.
endif

distclean ::
	rm -f $(CONFIG_FILES)


.PHONY:config
config : $(CONFIGURE)
	$(CONFIGURE) $(CONFIG_PARMS)


.PHONY:build_top_level
build_top_level:
	$(Q)$(MAKE) -f $(TOP_LEVEL_MAKE) $(MAKECMDGOALS)


$(PRIMARY_CONFIG_FILE) : $(CONFIGURE) \
                         $(addsuffix .in,$(subst $(CURDIR),$(TOP_LEVEL_DIR),$(CONFIG_FILES)))
	$(CONFIGURE) $(CONFIG_PARMS)
	touch $(CONFIG_FILES)

$(filter-out $(PRIMARY_CONFIG_FILE), $(CONFIG_FILES)) : $(PRIMARY_CONFIG_FILE)


ifeq ($(shell which $(AUTOCONF) > /dev/null && \
              which $(ACLOCAL) > /dev/null && \
              test `autoconf --version | sed -n "s/autoconf.*2.\\([0-9]*\\)/\\1/p"` -ge $(AUTOCONF_VERSION) \
              ; echo $$?),0)
  AUTOCONF_AVAILABLE := yes
else
  AUTOCONF_AVAILABLE := no
endif

ifeq ($(AUTOCONF_AVAILABLE),yes)

  $(CONFIGURE): $(subst $(CURDIR),$(TOP_LEVEL_DIR),$(CONFIGURE)).ac $(M4_FILES) $(ACLOCAL_M4)
	$(AUTOCONF)

  $(ACLOCAL_M4):
	cd $(dir $@) && $(ACLOCAL)

else # autoconf

  $(CONFIGURE): $(CONFIGURE).ac
	@echo ---------------------------------------------------------------------
	@echo The configure script requires updating but autoconf not is installed.
	@echo Either install autoconf v3.$(AUTOCONF_VERSION) or later or execute the command:
	@echo touch $@
	@echo ---------------------------------------------------------------------

endif # autoconf good


# End of autoconf.mak
