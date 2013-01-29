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

all ::

ifdef PTLIBDIR
  include $(PTLIBDIR)/make/ptlib.mak
else
  include $(shell pkg-config ptlib --variable=makedir)/ptlib.mak
endif

#SUBDIRS += ThreadSafe audio find_ip hello_world netif thread threadex dtmftest
SUBDIRS += audio find_ip ldaptest netif stunclient threadsafe dtmftest ipv6test md5 strtest thread timing

#SUBDIRS += pxml xmlrpc xmlrpcsrvr   #expat + some are broken
#SUBDIRS += vxmltest                 # no makefile

# all these targets are just passed to all subdirectories
$(subst tagbuild,,$(STANDARD_TARGETS)) ::
	for dir in $(SUBDIRS) ; do \
	if test -d $$dir ; then \
          $(MAKE) -C $$dir $@ ; \
        fi \
	done

# End of Makefile
