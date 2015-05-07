#
# Makefile
#
# Copyright (c) 2000-2013 Equivalence Pty. Ltd.
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
# The Original Code is Portable Tools Library.
#
# The Initial Developer of the Original Code is Equivalence Pty. Ltd.
#
# Contributor(s): ______________________________________.
#
# $Revision$
# $Author$
# $Date$
#
# Initial release of a console application to test XMPP messages between running applications.
# Thanks to Indranet Technologies ltd for sponsoring the work.
# Thanks to Federico Pinna & Reitek S.p.A for the inital work of getting XMPP going.

PROG = xmpp
SOURCES := main.cxx precompile.cxx

ifdef PTLIBDIR
  include $(PTLIBDIR)/make/ptlib.mak
else
  include $(shell pkg-config ptlib --variable=makedir)/ptlib.mak
endif

# End of Makefile
