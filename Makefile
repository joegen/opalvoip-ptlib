#
# Makefile
#
# Make file for pwlib library
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
# Portions are Copyright (C) 1993 Free Software Foundation, Inc.
# All Rights Reserved.
# 
# Contributor(s): ______________________________________.
#
# $Log: Makefile,v $
# Revision 1.11  2000/04/26 01:03:46  robertj
# Removed tarfile creation target, this is done differently now.
#
# Revision 1.10  2000/02/04 19:32:16  craigs
# Added targets for unshared libraries etc
#
# Revision 1.9  1999/11/30 00:22:54  robertj
# Updated documentation for doc++
#
# Revision 1.8  1999/06/09 16:09:20  robertj
# Fixed tarball construction not include windows directories
#
# Revision 1.7  1999/06/09 15:41:18  robertj
# Added better UI to make files.
#
# Revision 1.6  1999/04/22 02:37:00  robertj
# Added history file.
#
# Revision 1.5  1999/03/10 04:26:57  robertj
# More documentation changes.
#
# Revision 1.4  1999/03/09 08:07:00  robertj
# Documentation support.
#
# Revision 1.3  1999/01/22 00:30:45  robertj
# Yet more build environment changes.
#
# Revision 1.2  1999/01/16 23:15:11  robertj
# Added ability to NOT have th gui stuff.
#
# Revision 1.1  1999/01/16 04:00:14  robertj
# Initial revision
#
#


ifneq (,$(wildcard src/pwlib/xlib))
HAS_GUI = 1
HAS_XLIB = 1
endif

ifneq (,$(wildcard src/pwlib/motif))
HAS_GUI = 1
HAS_MOTIF = 1
endif

SUBDIRS := src/ptlib/unix tools/asnparser
ifdef HAS_GUI
SUBDIRS += tools/pwrc
ifdef HAS_XLIB
SUBDIRS += src/pwlib/xlib
endif
ifdef HAS_MOTIF
SUBDIRS += src/pwlib/motif
endif
endif

opt :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) opt ;)

debug :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) debug ;)

both :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) both ;)

optshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=1 opt ;)

debugshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=1 debug ;)

bothshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=1 both ;)

optnoshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=0 opt ;)

debugnoshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=0 debug ;)

bothnoshared :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) P_SHAREDLIB=0 both ;)

clean :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) clean ;)

optclean :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) optclean ;)

debugclean :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) debugclean ;)

optdepend :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) optdepend ;)

debugdepend :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) debugdepend ;)

bothdepend :
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) bothdepend ;)

ptlib:
	$(MAKE) -C src/ptlib/unix both


docs: 
	doc++ --dir html --tables pwlib.dxx


# End of Makefile
