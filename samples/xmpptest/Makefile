#
# Makefile
#
# Make file for application to test the XMPP classes
#
# Copyright (c) 2000-2013 Vox Lucida Pty. Ltd.
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


PROG		= xmpptest
SOURCES		:= \
	ConnectDialog.cxx \
	MainFrame.cxx     \
	main.cxx          \
	precompile.cxx    

#openphone.cxx : openphone.xrc
#	wxrc openphone.xrc -c -o openphone.cxx



STDCCFLAGS += `wx-config --cxxflags`

LDFLAGS += `wx-config  --libs`

ifdef PTLIBDIR
  include $(PTLIBDIR)/make/ptlib.mak
else
  include $(shell pkg-config ptlib --variable=makedir)/ptlib.mak
endif

# End of Makefile
