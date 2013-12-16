#
# toplevel.mak
#
# Make file for building ptlib library itself
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
# Contributor(s): ______________________________________.
#
# $Revision$
# $Author$
# $Date$
#

include $(PTLIBDIR)/make/ptbuildopts.mak

ifeq ($(DEBUG),)
default :: optshared
else
default :: debugshared
endif

include $(PTLIBDIR)/make/ptlib.mak

SUBDIRS := src
ifeq (1, $(HAS_PLUGINS))
SUBDIRS += plugins
endif

ifeq (1, $(HAS_SAMPLES))
SUBDIRS += samples/hello_world \
           samples/map_dict \
           samples/netif \
           samples/sockbundle \
           samples/thread
ifdef HAS_IPV6
SUBDIRS += samples/ipv6test
endif
ifdef HAS_DNS_RESOLVER
SUBDIRS += samples/dnstest
endif
ifdef HAS_STUN
SUBDIRS += samples/stunclient
endif
ifdef HAS_URL
SUBDIRS += samples/url
endif
ifdef HAS_HTTP
SUBDIRS += samples/httptest \
           samples/find_ip
endif
ifdef HAS_VCARD
SUBDIRS += samples/vcard
endif
ifdef HAS_ODBC
SUBDIRS += samples/ODBC
endif
ifdef HAS_LUA
SUBDIRS += samples/lua
endif
ifdef HAS_PCAP
SUBDIRS += samples/ether
endif
ifdef HAS_VIDEO
SUBDIRS += samples/vidtest
endif
endif

optshared   debugshared   bothshared   :: P_SHAREDLIB=1
optnoshared debugnoshared bothnoshared :: P_SHAREDLIB=0

# all these targets are just passed to all subdirectories
$(STANDARD_TARGETS) ::
	@set -e; $(foreach dir,$(addprefix $(PTLIBDIR)/,$(SUBDIRS)),if test -d $(dir) ; then $(MAKE) -C $(dir) $@; fi; )


ifneq (,$(SVN))

update: svnupdate bothdepend both

svnupdate:
	$(SVN) update
	@echo =====================================================

endif

ptlib:
	$(MAKE) -C $(PTLIBDIR)/src both

DOCS_DIR := $(PTLIBDIR)/html
DOXYGEN_CFG := $(PTLIBDIR)/ptlib_cfg.dxy
DOXYGEN_OUT := /tmp/ptlib_doxygen.out
DOXYGEN_GRAPH_CFG := /tmp/ptlib_graph_cfg.dxy

.PHONY:docs
docs:
	rm -rf $(DOCS_DIR)
	cd $(PTLIBDIR)
	doxygen $(DOXYGEN_CFG) > $(DOXYGEN_OUT) 2>&1

.PHONY:graphdocs
graphdocs:
	rm -rf $(DOCS_DIR)
	sed "s/HAVE_DOT.*=.*/HAVE_DOT=YES/" $(DOXYGEN_CFG) > $(DOXYGEN_GRAPH_CFG)
	doxygen $(DOXYGEN_GRAPH_CFG) > $(DOXYGEN_OUT) 2>&1
	rm $(DOXYGEN_GRAPH_CFG)


distclean: clean
	cd $(PTLIBDIR)
	rm -rf config.log config.err autom4te.cache config.status a.out aclocal.m4 lib*

sterile: distclean
	cd $(PTLIBDIR)
	rm -rf configure


################################################################################

ifeq ($(prefix),$(PTLIBDIR))

install uninstall:
	@echo install/uninstall not available as prefix=PTLIBDIR
	@false

else

ifeq ($(target_os),mingw)
ARCH_INCLUDE=msos
else
ARCH_INCLUDE=unix
endif


install:
	( for dir in $(DESTDIR)$(libdir) \
		     $(DESTDIR)$(prefix)/bin \
		     $(DESTDIR)$(includedir)/ptlib \
                     $(DESTDIR)$(includedir)/ptlib/$(ARCH_INCLUDE)/ptlib \
                     $(DESTDIR)$(includedir)/ptclib \
                     $(DESTDIR)$(datarootdir)/ptlib/make ; \
		do mkdir -p $$dir ; chmod 755 $$dir ; \
	done )
	( for lib in  $(PTLIB_LIBDIR)/$(PTLIB_SONAME) \
	              $(PTLIB_LIBDIR)/$(PTLIB_DEBUG_SONAME) \
	              $(PTLIB_LIBDIR)/lib$(PTLIB_BASE)_s.a \
	              $(PTLIB_LIBDIR)/lib$(PTLIB_BASE)_d_s.a ; \
          do \
	  ( if test -e $$lib ; then \
		$(INSTALL) -m 755 $$lib $(DESTDIR)$(libdir); \
	  fi ) \
	done )
	( if test -e $(PTLIB_LIBDIR)/$(PTLIB_SONAME); then \
	    (cd $(DESTDIR)$(libdir) ; \
		rm -f $(PTLIB_FILE) ; \
		ln -sf $(PTLIB_SONAME) $(PTLIB_FILE) \
	    ) \
	fi )
	( if test -e $(PTLIB_LIBDIR)/$(PTLIB_DEBUG_SONAME); then \
	    (cd $(DESTDIR)$(libdir) ; \
		rm -f $(PTLIB_DEBUG_FILE) ; \
		ln -sf $(PTLIB_DEBUG_SONAME) $(PTLIB_DEBUG_FILE) \
	    ) \
	fi )
ifeq (1, $(HAS_PLUGINS))
	if test -e $(PTLIB_LIBDIR)/device/; then \
	cd $(PTLIB_LIBDIR)/device/; \
	(  for dir in ./* ;\
		do mkdir -p $(DESTDIR)$(libdir)/$(DEV_PLUGIN_DIR)/$$dir ; \
		chmod 755 $(DESTDIR)$(libdir)/$(DEV_PLUGIN_DIR)/$$dir ; \
		(for fn in ./$$dir/*.so ; \
			do $(INSTALL) -m 755 $$fn $(DESTDIR)$(libdir)/$(DEV_PLUGIN_DIR)/$$dir; \
		done ); \
	done ) ; \
	fi
endif
	$(INSTALL) -m 444 include/ptlib.h                $(DESTDIR)$(includedir)
	$(INSTALL) -m 444 include/ptbuildopts.h          $(DESTDIR)$(includedir)
	(for fn in include/ptlib/*.h include/ptlib/*.inl; \
		do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptlib; \
	done)
	(for fn in include/ptlib/$(ARCH_INCLUDE)/ptlib/*.h include/ptlib/$(ARCH_INCLUDE)/ptlib/*.inl ; \
		do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptlib/$(ARCH_INCLUDE)/ptlib ; \
	done)
	(for fn in include/ptclib/*.h ; \
		do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptclib; \
	done)
	(for fn in make/*.mak ; \
		do $(INSTALL) -m 444 $$fn $(DESTDIR)$(datarootdir)/ptlib/make; \
	done)

	mkdir -p $(DESTDIR)$(libdir)/pkgconfig
	chmod 755 $(DESTDIR)$(libdir)/pkgconfig
	$(INSTALL) -m 644 ptlib.pc $(DESTDIR)$(libdir)/pkgconfig/

uninstall:
	rm -rf $(DESTDIR)$(includedir)/ptlib \
	       $(DESTDIR)$(includedir)/ptclib \
	       $(DESTDIR)$(includedir)/ptlib.h \
	       $(DESTDIR)$(includedir)/ptbuildopts.h \
	       $(DESTDIR)$(datarootdir)/ptlib \
	       $(DESTDIR)$(libdir)/$(PTLIB_PLUGIN_DIR) \
	       $(DESTDIR)$(libdir)/pkgconfig/ptlib.pc \
	       $(DESTDIR)$(libdir)/lib$(PTLIB_BASE)_s.a \
	       $(DESTDIR)$(libdir)/$(PTLIB_FILE) \
	       $(DESTDIR)$(libdir)/$(PTLIB_SONAME) \
	       $(DESTDIR)$(libdir)/lib$(PTLIB_BASE)_d_s.a \
	       $(DESTDIR)$(libdir)/$(PTLIB_DEBUG_FILE) \
	       $(DESTDIR)$(libdir)/$(PTLIB_DEBUG_SONAME)
 
endif

# End of Makefile.in
