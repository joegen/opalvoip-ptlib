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
# Revision 1.25  2002/12/04 19:35:47  rogerh
# Remove un-needed / at the end of PREFIX
#
# Revision 1.24  2002/11/13 23:45:19  robertj
# Added install and uninstall targets, thanks Damien Sandras
#
# Revision 1.23  2002/10/17 13:44:27  robertj
# Port to RTEMS, thanks Vladimir Nesic.
#
# Revision 1.22  2001/08/06 19:35:27  rogerh
# Include the relevent header file based on the version of OpenBSD.
# Submitted by Marius Aamodt Eriksen <marius@umich.edu>
#
# Revision 1.21  2001/07/30 07:45:54  robertj
# Added "all" target with double colon.
#
# Revision 1.20  2001/04/23 00:44:30  robertj
# Spelt update correctly!
#
# Revision 1.19  2001/04/23 00:43:55  robertj
# Added make update target to get from cvs and rebuild
#
# Revision 1.18  2001/04/17 06:30:37  robertj
# Altered so can use tagbuild target in root directory.
#
# Revision 1.17  2001/03/20 03:33:18  robertj
# Major improvement to multiple targets over subdirectories, thanks Jac Goudsmit
#
# Revision 1.16  2000/11/01 04:39:20  robertj
# Made sure opt is first so frech build works
#
# Revision 1.15  2000/11/01 02:42:46  robertj
# Added optnoshared to build all default target.
#
# Revision 1.14  2000/10/30 05:49:25  robertj
# Made make all do bothdepend both
#
# Revision 1.13  2000/06/26 11:17:19  robertj
# Nucleus++ port (incomplete).
#
# Revision 1.12  2000/04/26 02:50:12  robertj
# Fixed build of correct GUI directory.
#
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


all ::


ifeq ($(OSTYPE),Nucleus)
TARGETDIR=Nucleus++
else
TARGETDIR=unix
endif

include make/ptlib.mak

SUBDIRS := src/ptlib/$(TARGETDIR)

ifeq (,$(findstring $(OSTYPE),Nucleus rtems))
SUBDIRS += tools/asnparser

include make/defaultgui.mak
endif

ifdef GUI_TYPE
SUBDIRS += src/pwlib/$(GUI_TYPE)
endif


ifndef PREFIX
PREFIX=/usr/local
endif
 

# override P_SHAREDLIB for specific targets
optshared   debugshared   bothshared   :: P_SHAREDLIB=1
optnoshared debugnoshared bothnoshared :: P_SHAREDLIB=0

# all these targets are just passed to all subdirectories
$(subst tagbuild,,$(STANDARD_TARGETS)) ::
	set -e; $(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) $@;)

update:
	cvs update
	$(MAKE) bothdepend both

ptlib:
	$(MAKE) -C src/ptlib/$(TARGETDIR) both

docs: 
	doc++ --dir html --tables pwlib.dxx

install:
	cp -df lib/*so* $(PREFIX)/lib/
	cp -rf include/* $(PREFIX)/include/
	cp tools/asnparser/obj*/asnparser $(PREFIX)/bin/
	mkdir -p $(PREFIX)/share/pwlib/
	cp -rf make $(PREFIX)/share/pwlib/
	ln -s $(PREFIX)/lib/$(PTLIB_FILE) $(PREFIX)/lib/libpt.so
	chmod -R a+r $(PREFIX)/include/ptlib $(PREFIX)/include/ptclib $(PREFIX)/include/ptlib.h $(PREFIX)/share/pwlib

uninstall:
	rm -rf $(PREFIX)/include/ptlib $(PREFIX)/include/ptclib $(PREFIX)/include/ptlib.h $(PREFIX)/share/pwlib/
	rm -f $(PREFIX)/lib/$(PTLIB_FILE)* $(PREFIX)/lib/libpt.so $(PREFIX)/bin/asnparser 


# End of Makefile
