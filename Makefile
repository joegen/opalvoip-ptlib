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
#

# autoconf.mak uses this for if we are run as "make -f ../Makefile"
TOP_LEVEL_DIR := $(abspath $(dir $(firstword $(MAKEFILE_LIST))))

# All the configure outputs are relative to $(TARGET_DIR) not $(TOP_LEVEL_DIR)
# Note: TARGET_DIR is defined by autoconf.mak so must be relative paths
CONFIG_FILES := include/ptlib_config.h \
                make/ptlib_config.mak \
                ptlib_cfg.dxy \
                ptlib.pc \


include $(TOP_LEVEL_DIR)/make/autoconf.mak


# End of Makefile.in
