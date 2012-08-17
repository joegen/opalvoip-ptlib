#
# Makefile
#
# Make file for ptlib library
#
# Portable Tools Library
#
# Copyright (c) 1993-2012 Equivalence Pty. Ltd.
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

ifdef PTLIBDIR
  override CFG_ARGS+=--prefix=$(PTLIBDIR)
else
  export PTLIBDIR:=$(CURDIR)
  $(info Setting default PTLIBDIR to $(PTLIBDIR))
endif

TOP_LEVEL_MAKE := $(PTLIBDIR)/make/toplevel.mak
CONFIGURE      := $(PTLIBDIR)/configure
CONFIG_FILES   := $(PTLIBDIR)/ptlib.pc \
                  $(PTLIBDIR)/ptlib_cfg.dxy \
                  $(PTLIBDIR)/make/ptbuildopts.mak \
                  $(PTLIBDIR)/include/ptbuildopts.h

ifneq (,$(findstring --disable-plugins,$(CFG_ARGS)))
  CONFIG_FILES += $(PTLIBDIR)/plugins/Makefile \
                  $(PTLIBDIR)/plugins/vidinput_v4l2/Makefile \
                  $(PTLIBDIR)/plugins/vidinput_dc/Makefile
endif


AUTOCONF       := autoconf
ACLOCAL        := aclocal


ifneq ($(MAKECMDGOALS),config)
$(MAKECMDGOALS): default
endif

default: $(CONFIG_FILES)
	@$(MAKE) -f $(TOP_LEVEL_MAKE) $(MAKECMDGOALS)

.PHONY:config
config: $(CONFIGURE)
	$(CONFIGURE) $(CFG_ARGS)

# this complexity is so if any of CONFIG_FILES does not exist it is created
# with ./configure only being executed once.
FIRST_CONFIG := $(firstword $(CONFIG_FILES))
OTHER_CONFIGS := $(wordlist 2,1000,$(CONFIG_FILES))

$(FIRST_CONFIG): $(OTHER_CONFIGS) $(CONFIGURE) $(PLUGIN_CONFIG) $(addsuffix .in, $(CONFIG_FILES))
	$(CONFIGURE) $(CFG_ARGS)
	touch $@

$(OTHER_CONFIGS):
	touch $@


ifneq (,$(AUTOCONF))
ifneq (,$(shell which $(AUTOCONF)))
ifneq (,$(shell which $(ACLOCAL)))

$(CONFIGURE): $(CONFIGURE).ac $(ACLOCAL).m4 $(PTLIBDIR)/make/*.m4
	$(AUTOCONF)

$(ACLOCAL).m4:
	$(ACLOCAL)

else # autoconf installed

$(CONFIGURE): $(CONFIGURE).ac
	@echo ---------------------------------------------------------------------
	@echo The configure script requires updating but autoconf not is installed.
	@echo Either install autoconf or execute the command:
	@echo touch $@
	@echo ---------------------------------------------------------------------

endif # aclocal installed
endif # autoconf installed
endif # autoconf enabled

# End of Makefile.in
