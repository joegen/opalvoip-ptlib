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

ifndef PTLIBDIR
PTLIBDIR=$(CURDIR)
endif

TOP_LEVEL_MAKE := $(PTLIBDIR)/make/toplevel.mak
CONFIG_FILES   := $(PTLIBDIR)/make/ptbuildopts.mak $(PTLIBDIR)/plugins/Makefile
CONFIGURE      := $(PTLIBDIR)/configure

AUTOCONF       := autoconf
ACLOCAL        := aclocal


ifneq (,$(MAKECMDGOALS))
$(MAKECMDGOALS): default
endif

default: $(CONFIG_FILES)
	@$(MAKE) -f $(TOP_LEVEL_MAKE) $(MAKECMDGOALS)

$(CONFIG_FILES): $(CONFIGURE)
	$(CONFIGURE) $(CFG_ARGS)

$(CONFIGURE): $(CONFIGURE).ac $(ACLOCAL).m4
	$(AUTOCONF)

$(ACLOCAL).m4:
	$(ACLOCAL)

$(CONFIG_FILES): $(addsuffix .in, $(CONFIG_FILES)) $(PTLIBDIR)/make/*.m4


# End of Makefile.in
