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
# Revision 1.1  1999/01/16 04:00:14  robertj
# Initial revision
#
#


all :
	$(MAKE) -C src/ptlib/unix
	$(MAKE) -C tools/asnparser DEBUG=
	$(MAKE) -C tools/pwrc DEBUG=
	$(MAKE) -C src/pwlib/xlib

depend :
	$(MAKE) -C src/ptlib/unix depend
	$(MAKE) -C tools/asnparser DEBUG= depend
	$(MAKE) -C tools/pwrc DEBUG= depend
	$(MAKE) -C src/pwlib/xlib depend

clean :
	$(MAKE) -C src/ptlib/unix clean
	$(MAKE) -C tools/asnparser DEBUG= clean
	$(MAKE) -C tools/pwrc DEBUG= clean
	$(MAKE) -C src/pwlib/xlib clean

both :
	$(MAKE) DEBUG=1 ; $(MAKE) DEBUG=

bothdepend :
	$(MAKE) DEBUG=1 depend ; $(MAKE) DEBUG= depend

bothclean :
	$(MAKE) DEBUG=1 clean ; $(MAKE) DEBUG= clean

TAR_SOURCES = Makefile make include src \
	$(shell $(MAKE) --no-print-directory -C tools/asnparser tarfiles) \
	$(shell $(MAKE) --no-print-directory -C tools/pwrc tarfiles)

tarball:
	( cd .. ; tar zcf pwlib.tar.gz $(patsubst %, $(notdir $(shell pwd))/%, $(TAR_SOURCES)) )


# End of Makefile
