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

all :
	$(MAKE) -C src/ptlib/unix
	$(MAKE) -C tools/asnparser DEBUG=
ifdef HAS_GUI
	$(MAKE) -C tools/pwrc DEBUG=
ifdef HAS_XLIB
	$(MAKE) -C src/pwlib/xlib
endif
ifdef HAS_MOTIF
	$(MAKE) -C src/pwlib/motif
endif
endif

depend :
	$(MAKE) -C src/ptlib/unix depend
	$(MAKE) -C tools/asnparser DEBUG= depend
ifdef HAS_GUI
	$(MAKE) -C tools/pwrc DEBUG= depend
ifdef HAS_XLIB
	$(MAKE) -C src/pwlib/xlib depend
endif
ifdef HAS_MOTIF
	$(MAKE) -C src/pwlib/motif depend
endif
endif

clean :
	$(MAKE) -C src/ptlib/unix clean
	$(MAKE) -C tools/asnparser DEBUG= clean
ifdef HAS_GUI
	$(MAKE) -C tools/pwrc DEBUG= clean
ifdef HAS_XLIB
	$(MAKE) -C src/pwlib/xlib clean
endif
ifdef HAS_MOTIF
	$(MAKE) -C src/pwlib/motif clean
endif
endif

both :
	$(MAKE) DEBUG= ; $(MAKE) DEBUG=1

bothdepend :
	$(MAKE) DEBUG= depend ; $(MAKE) DEBUG=1 depend

bothclean :
	$(MAKE) DEBUG= clean ; $(MAKE) DEBUG=1 clean


ifdef HAS_GUI
GUI_SOURCES = include/pwclib include/pwlib src/pwclib src/pwlib
endif

TAR_SOURCES = mpl-1.0.htm Makefile make \
	include/ptclib include/ptlib src/ptclib src/ptlib $(GUI_SOURCES) \
	$(shell $(MAKE) --no-print-directory -C tools/asnparser tarfiles) \
	$(shell $(MAKE) --no-print-directory -C tools/pwrc tarfiles)

tarball:
	( cd .. ; tar zcf pwlib.tar.gz $(patsubst %, $(notdir $(shell pwd))/%, $(TAR_SOURCES)) )


# End of Makefile
