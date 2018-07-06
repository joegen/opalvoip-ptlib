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

PTLIB_TOP_LEVEL_DIR := $(abspath $(dir $(lastword $(MAKEFILE_LIST)))..)
ifneq ($(CURDIR),$(PTLIB_TOP_LEVEL_DIR))
  $(info Doing out-of-source PTLib build in $(CURDIR))
endif

export PTLIBDIR := $(PTLIB_TOP_LEVEL_DIR)
export PTLIB_PLATFORM_DIR := $(CURDIR)
include $(PTLIB_TOP_LEVEL_DIR)/make/pre.mak


###############################################################################

SUBDIRS :=

ifeq (1,$(HAS_PLUGINS))
  ifeq (1,$(HAS_ALSA))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_alsa
  endif

  ifeq (1,$(HAS_AUDIOSHM))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_shm
  endif

  ifeq (1,$(HAS_OSS))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_oss
  endif

  ifeq (1,$(HAS_PULSE))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_pulse
  endif

  ifeq (1,$(HAS_ESD))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_esd
  endif

  ifeq (1,$(HAS_SUNAUDIO))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/sound_sunaudio
  endif

  ifeq (1,$(HAS_V4L))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/vidinput_v4l
  endif

  ifeq (1,$(HAS_V4L2))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/vidinput_v4l2
  endif

  ifeq (1,$(HAS_BSDVIDEOCAP))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/vidinput_bsd
  endif

  ifeq (1,$(HAS_AVC1394))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/vidinput_avc
  endif

  ifeq (1,$(HAS_DC1394))
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/plugins/vidinput_dc
  endif
endif

ifeq (1, $(HAS_SAMPLES))
  SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/hello_world \
             $(PTLIB_TOP_LEVEL_DIR)/samples/map_dict \
             $(PTLIB_TOP_LEVEL_DIR)/samples/netif \
             $(PTLIB_TOP_LEVEL_DIR)/samples/sockbundle \
             $(PTLIB_TOP_LEVEL_DIR)/samples/timing \
             $(PTLIB_TOP_LEVEL_DIR)/samples/thread
  ifeq ($(HAS_IPV6),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/ipv6test
  endif
  ifeq ($(HAS_DNS_RESOLVER),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/dnstest
  endif
  ifeq ($(HAS_STUN),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/stunclient
  endif
  ifeq ($(HAS_URL),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/url
  endif
  ifeq ($(HAS_HTTP),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/httptest \
               $(PTLIB_TOP_LEVEL_DIR)/samples/find_ip
  endif
  ifeq ($(HAS_VCARD),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/vcard
  endif
  ifeq ($(HAS_ODBC),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/ODBC
  endif
  ifeq ($(HAS_LUA),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/lua
  endif
  ifeq ($(HAS_V8),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/javascript
  endif
  ifeq ($(HAS_PCAP),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/ether
  endif
  ifeq ($(HAS_AUDIO),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/audio
  endif
  ifeq ($(HAS_VIDEO),1)
    SUBDIRS += $(PTLIB_TOP_LEVEL_DIR)/samples/vidtest
  endif
endif


###############################################################################

OBJDIR	= $(PTLIB_OBJDIR)

VERSION_FILE = $(PTLIB_TOP_LEVEL_DIR)/version.h
REVISION_FILE = $(PTLIB_TOP_LEVEL_DIR)/revision.h

DOXYGEN_CFG := $(PTLIB_TOP_LEVEL_DIR)/ptlib_cfg.dxy

ifneq ($(STATIC_BUILD),yes)
  SHARED_LIB_LINK = $(PTLIB_SHARED_LIB_LINK)
  SHARED_LIB_FILE = $(PTLIB_SHARED_LIB_FILE)
endif
STATIC_LIB_FILE = $(PTLIB_STATIC_LIB_FILE)


COMPONENT_SRC_DIR  := $(PTLIB_TOP_LEVEL_DIR)/src/ptclib
COMMON_SRC_DIR     := $(PTLIB_TOP_LEVEL_DIR)/src/ptlib/common
PLUGIN_DIR         := $(PTLIB_TOP_LEVEL_DIR)/plugins
ifeq ($(target_os),mingw)
  PLATFORM_SRC_DIR := $(PTLIB_TOP_LEVEL_DIR)/src/ptlib/msos
else
  PLATFORM_SRC_DIR := $(PTLIB_TOP_LEVEL_DIR)/src/ptlib/unix
endif
VPATH_CXX          := $(PLATFORM_SRC_DIR) $(COMMON_SRC_DIR) $(COMPONENT_SRC_DIR) 
VPATH_MM           := $(PLATFORM_SRC_DIR)

DIST_CLEAN_FILES   += $(PTLIB_TOP_LEVEL_DIR)/include/ptlib_config.h $(PTLIB_TOP_LEVEL_DIR)/make/ptlib_config.mak


###############################################################################

ifeq ($(HAS_SASL)$(HAS_SASL2),1)
  SOURCES += $(COMPONENT_SRC_DIR)/psasl.cxx 
endif

ifeq ($(HAS_LDAP),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pldap.cxx \
             $(COMPONENT_SRC_DIR)/pils.cxx
endif

ifeq ($(HAS_SSL),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pssl.cxx 
endif

ifeq ($(HAS_SDL),1)
  SOURCES += $(COMPONENT_SRC_DIR)/vsdl.cxx
endif

ifeq ($(HAS_GSTREAMER),1)
  SOURCES += $(COMPONENT_SRC_DIR)/gstreamer.cxx
endif

ifeq ($(HAS_ODBC),1)
  SOURCES += $(COMPONENT_SRC_DIR)/podbc.cxx
endif


ifeq ($(HAS_VIDEO),1)

  SOURCES += $(COMMON_SRC_DIR)/vfakeio.cxx \
             $(COMMON_SRC_DIR)/videoio.cxx \
             $(COMMON_SRC_DIR)/vconvert.cxx \
             $(COMMON_SRC_DIR)/pvidchan.cxx \
             $(COMMON_SRC_DIR)/tinyjpeg.c \
             $(COMMON_SRC_DIR)/jidctflt.c

  ifeq ($(HAS_SHM_VIDEO),1)
    SOURCES += $(PLATFORM_SRC_DIR)/shmvideo.cxx
  endif

  ifeq ($(HAS_VFW_CAPTURE),1)
    SOURCES += $(PLATFORM_SRC_DIR)/vfw.cxx
  endif

  ifeq ($(HAS_DIRECTSHOW),1)
    SOURCES += $(PLATFORM_SRC_DIR)/directshow.cxx
  endif

  ifeq ($(target_os),Darwin)
    SOURCES += $(PLATFORM_SRC_DIR)/macvidcap.mm
  endif # Darwin

endif # HAS_VIDEO

## SOUND DRIVERS
## Note this is mostly handled by the plugin system
ifeq ($(HAS_AUDIO),1)

  SOURCES += $(COMMON_SRC_DIR)/sound.cxx 

  ifeq ($(target_os),mingw)
    SOURCES += $(PLATFORM_SRC_DIR)/sound_win32.cxx
  endif

  ifdef HAS_PORTAUDIO
    SOURCES += $(COMPONENT_SRC_DIR)/portaudio.cxx
  endif

  ifneq (,$(findstring $(target_os),Darwin iPhoneOS iPhoneSimulator))
    SOURCES += $(PLATFORM_SRC_DIR)/macaudio.mm
  endif # Darwin

endif  # HAS_AUDIO


## Various modules

SOURCES += $(COMPONENT_SRC_DIR)/pxml.cxx  # Outside HAS_EXPAT as need PXML::EscapeSpecialChars()

ifeq ($(HAS_EXPAT),1)

  ifeq ($(HAS_XMLRPC),1)
    SOURCES += $(COMPONENT_SRC_DIR)/pxmlrpc.cxx \
               $(COMPONENT_SRC_DIR)/pxmlrpcs.cxx 
  endif

  ifeq ($(HAS_SOAP),1)
    SOURCES += $(COMPONENT_SRC_DIR)/psoap.cxx 
  endif

  ifeq ($(HAS_VXML),1)
    SOURCES += $(COMPONENT_SRC_DIR)/vxml.cxx 
  endif

  ifeq ($(HAS_SASL)$(HAS_SASL2),1)
    SOURCES += $(COMPONENT_SRC_DIR)/xmpp.cxx \
               $(COMPONENT_SRC_DIR)/xmpp_c2s.cxx \
               $(COMPONENT_SRC_DIR)/xmpp_muc.cxx \
               $(COMPONENT_SRC_DIR)/xmpp_roster.cxx
  endif
endif # HAS_EXPAT

ifeq ($(HAS_LUA),1)
  SOURCES += $(COMPONENT_SRC_DIR)/lua.cxx
endif

ifeq ($(HAS_V8),1)
  SOURCES += $(COMPONENT_SRC_DIR)/jscript.cxx
endif

ifeq ($(HAS_DNS_RESOLVER),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pdns.cxx \
             $(COMPONENT_SRC_DIR)/enum.cxx 
endif

ifeq ($(HAS_TTS),1)
  SOURCES += $(COMPONENT_SRC_DIR)/ptts.cxx 
endif

ifeq ($(HAS_ASN),1)
  SOURCES += $(COMPONENT_SRC_DIR)/asner.cxx \
             $(COMPONENT_SRC_DIR)/pasn.cxx 
endif

ifeq ($(HAS_SNMP),1)
  SOURCES += $(COMPONENT_SRC_DIR)/snmpclnt.cxx \
             $(COMPONENT_SRC_DIR)/snmpserv.cxx \
             $(COMPONENT_SRC_DIR)/psnmp.cxx \
             $(COMPONENT_SRC_DIR)/snmp.cxx \
             $(COMPONENT_SRC_DIR)/rfc1155.cxx 
endif

ifeq ($(HAS_FTP),1)
  SOURCES += $(COMPONENT_SRC_DIR)/ftpclnt.cxx \
             $(COMPONENT_SRC_DIR)/ftpsrvr.cxx \
             $(COMPONENT_SRC_DIR)/ftp.cxx 
endif

ifeq ($(HAS_TELNET),1)
  SOURCES += $(COMPONENT_SRC_DIR)/telnet.cxx
endif

ifeq ($(HAS_CLI),1)
  SOURCES += $(COMPONENT_SRC_DIR)/cli.cxx
endif

ifeq ($(HAS_NAT),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pnat.cxx
endif

ifeq ($(HAS_STUN),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pstun.cxx
endif

ifeq ($(HAS_STUNSRVR),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pstunsrvr.cxx
endif


ifeq ($(HAS_SOCKS),1)
  SOURCES += $(COMPONENT_SRC_DIR)/socks.cxx 
endif

ifeq ($(HAS_PIPECHAN),1)
  ifeq ($(target_os),mingw)
    SOURCES += $(PLATFORM_SRC_DIR)/pipe.cxx \
               $(COMMON_SRC_DIR)/pipechan.cxx
  else
    SOURCES += $(PLATFORM_SRC_DIR)/pipechan.cxx 
  endif
endif

ifeq ($(HAS_REMCONN),1)
  SOURCES += $(PLATFORM_SRC_DIR)/remconn.cxx 
endif

ifeq ($(HAS_WAVFILE),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pwavfile.cxx \
             $(COMPONENT_SRC_DIR)/pwavfiledev.cxx
endif

ifeq ($(HAS_DTMF),1)
  SOURCES += $(COMPONENT_SRC_DIR)/dtmf.cxx \
             $(COMPONENT_SRC_DIR)/tonedev.cxx 
endif

ifeq ($(HAS_VCARD),1)
  SOURCES += $(COMPONENT_SRC_DIR)/vcard.cxx 
endif

ifeq ($(HAS_SERIAL),1)
  ifeq ($(target_os),mingw)
    SOURCES += $(PLATFORM_SRC_DIR)/winserial.cxx \
               $(COMMON_SRC_DIR)/serial.cxx
  else
    SOURCES += $(PLATFORM_SRC_DIR)/serchan.cxx
  endif
  SOURCES += $(COMPONENT_SRC_DIR)/modem.cxx 
endif

ifeq ($(HAS_POP3SMTP),1)
  SOURCES += $(COMPONENT_SRC_DIR)/inetmail.cxx 
endif

ifeq ($(HAS_URL),1)
  SOURCES += $(COMPONENT_SRC_DIR)/url.cxx 
endif

ifeq ($(HAS_HTTP),1)
  SOURCES += $(COMPONENT_SRC_DIR)/http.cxx \
             $(COMPONENT_SRC_DIR)/httpclnt.cxx \
             $(COMPONENT_SRC_DIR)/html.cxx \
             $(COMPONENT_SRC_DIR)/httpsrvr.cxx

  ifeq ($(HAS_SSDP),1)
    SOURCES += $(COMPONENT_SRC_DIR)/ssdp.cxx
  endif
endif

ifeq ($(HAS_HTTPFORMS),1)
  SOURCES += $(COMPONENT_SRC_DIR)/httpform.cxx
endif

ifeq ($(HAS_HTTPSVC),1)
  SOURCES += $(PLATFORM_SRC_DIR)/svcproc.cxx \
             $(COMPONENT_SRC_DIR)/httpsvc.cxx

  ifeq ($(HAS_SSL),1)
    SOURCES += $(COMPONENT_SRC_DIR)/shttpsvc.cxx
  endif
endif

ifeq ($(HAS_CONFIG_FILE),1)
  ifeq ($(target_os),mingw)
    SOURCES += $(PLATFORM_SRC_DIR)/wincfg.cxx \
               $(COMMON_SRC_DIR)/pconfig.cxx
  else
    SOURCES += $(PLATFORM_SRC_DIR)/config.cxx 
  endif
endif

ifeq ($(HAS_VIDFILE),1)
  SOURCES += $(COMPONENT_SRC_DIR)/pvidfile.cxx \
             $(COMPONENT_SRC_DIR)/pvfiledev.cxx 
endif

ifeq ($(HAS_MEDIAFILE),1)
  SOURCES += $(COMPONENT_SRC_DIR)/mediafile.cxx
endif

ifeq ($(HAS_CYPHER),1)
  SOURCES += $(COMPONENT_SRC_DIR)/cypher.cxx
endif

ifeq ($(HAS_VARTYPE),1)
  SOURCES += $(COMPONENT_SRC_DIR)/vartype.cxx
endif

ifeq ($(HAS_GUID),1)
  SOURCES += $(COMPONENT_SRC_DIR)/guid.cxx
endif

ifeq ($(HAS_SCRIPTS),1)
  SOURCES += $(COMPONENT_SRC_DIR)/script.cxx
endif

ifeq ($(HAS_SPOOLDIR),1)
  SOURCES += $(COMPONENT_SRC_DIR)/spooldir.cxx
endif

ifeq ($(HAS_SYSTEMLOG),1)
  SOURCES += $(COMPONENT_SRC_DIR)/syslog.cxx
endif

ifeq ($(HAS_PLUGINMGR),1)
  SOURCES += $(COMMON_SRC_DIR)/pluginmgr.cxx
endif

ifeq ($(HAS_CHANNEL_UTILS),1)
  SOURCES += $(COMPONENT_SRC_DIR)/qchannel.cxx \
             $(COMPONENT_SRC_DIR)/delaychan.cxx \
             $(COMPONENT_SRC_DIR)/memfile.cxx
endif

ifeq ($(HAS_NETWORKING),1)
  SOURCES += $(COMPONENT_SRC_DIR)/ipacl.cxx \
             $(COMPONENT_SRC_DIR)/inetprot.cxx \
             $(COMMON_SRC_DIR)/psockbun.cxx \
             $(COMMON_SRC_DIR)/sockets.cxx
  ifeq ($(target_os),mingw)
    SOURCES += $(PLATFORM_SRC_DIR)/icmp.cxx \
               $(PLATFORM_SRC_DIR)/winsock.cxx \
               $(COMMON_SRC_DIR)/pethsock.cxx
  else
    SOURCES += $(PLATFORM_SRC_DIR)/uicmp.cxx \
               $(PLATFORM_SRC_DIR)/socket.cxx
  endif
endif

ifeq ($(target_os),mingw)
  SOURCES += $(PLATFORM_SRC_DIR)/ptlib.cxx \
             $(PLATFORM_SRC_DIR)/win32.cxx \
             $(PLATFORM_SRC_DIR)/dllmain.cxx \
             $(COMMON_SRC_DIR)/pchannel.cxx
else
  SOURCES += $(PLATFORM_SRC_DIR)/udll.cxx \
             $(PLATFORM_SRC_DIR)/channel.cxx \
             $(PLATFORM_SRC_DIR)/osutil.cxx \
             $(PLATFORM_SRC_DIR)/tlib.cxx
endif

GETDATE_SOURCE = $(COMMON_SRC_DIR)/getdate.c
CLEAN_FILES = $(GETDATE_SOURCE)

SOURCES	+= \
	$(COMPONENT_SRC_DIR)/json.cxx \
	$(COMPONENT_SRC_DIR)/threadpool.cxx \
	$(COMPONENT_SRC_DIR)/random.cxx \
	$(COMPONENT_SRC_DIR)/notifier_ext.cxx \
	$(COMMON_SRC_DIR)/safecoll.cxx \
	$(COMMON_SRC_DIR)/ptime.cxx \
	$(GETDATE_SOURCE) \
	$(COMMON_SRC_DIR)/osutils.cxx \
	$(PLATFORM_SRC_DIR)/assert.cxx \
	$(COMMON_SRC_DIR)/collect.cxx \
	$(COMMON_SRC_DIR)/contain.cxx \
	$(COMMON_SRC_DIR)/object.cxx   # must be last module

ifneq ($(HAS_REGEX),1)
  OBJS = $(OBJDIR)/regcomp.o $(OBJDIR)/regexec.o $(OBJDIR)/regerror.o $(OBJDIR)/regfree.o
endif


###############################################################################

CPPFLAGS += $(SHARED_CPPFLAGS)

include $(PTLIB_TOP_LEVEL_DIR)/make/post.mak


###############################################################################

$(COMMON_SRC_DIR)/osutils.cxx: $(REVISION_FILE)

$(OBJDIR)/regcomp.o: $(COMMON_SRC_DIR)/regex/regcomp.c
	$(Q_CC)$(CC) $(CPPFLAGS) -DPOSIX_MISTAKE -I$(COMMON_SRC_DIR)/regex $(CFLAGS) -o $@ -c $<

$(OBJDIR)/regexec.o: $(COMMON_SRC_DIR)/regex/regexec.c
	$(Q_CC)$(CC) $(CPPFLAGS) -DPOSIX_MISTAKE -I$(COMMON_SRC_DIR)/regex $(CFLAGS) -o $@ -c $<

$(OBJDIR)/regerror.o: $(COMMON_SRC_DIR)/regex/regerror.c
	$(Q_CC)$(CC) $(CPPFLAGS) -DPOSIX_MISTAKE -I$(COMMON_SRC_DIR)/regex $(CFLAGS) -o $@ -c $<

$(OBJDIR)/regfree.o: $(COMMON_SRC_DIR)/regex/regfree.c
	$(Q_CC)$(CC) $(CPPFLAGS) -DPOSIX_MISTAKE -I$(COMMON_SRC_DIR)/regex $(CFLAGS) -o $@ -c $<


$(OBJDIR)/getdate.o: $(GETDATE_SOURCE)
	$(Q_CC)$(CC) $(CPPFLAGS) -Wno-write-strings $(CFLAGS) -c $< -o $@

$(DEPDIR)/getdate.dep: $(GETDATE_SOURCE)
	$(Q_CC)$(CC) $(CPPFLAGS) -M $< >> $@

GETDATE_TAB_C := $(COMMON_SRC_DIR)/getdate.tab.c

$(GETDATE_SOURCE): $(GETDATE_TAB_C)
	cp $< $@

ifdef BISON
$(GETDATE_TAB_C): $(COMMON_SRC_DIR)/getdate.y
	$(BISON) -o $@ $<
endif


################################################################################

ifeq ($(prefix),$(PTLIBDIR))

install uninstall:
	@echo install/uninstall not available as prefix=PTLIBDIR
	@false

else # PTLIBDIR

ifeq ($(target_os),mingw)
  OS_INCLUDE=msos
else
  OS_INCLUDE=unix
endif


install:
	for dir in $(DESTDIR)$(libdir) \
	           $(DESTDIR)$(libdir)/pkgconfig \
		   $(DESTDIR)$(prefix)/bin \
		   $(DESTDIR)$(includedir)/ptlib \
	           $(DESTDIR)$(includedir)/ptlib/$(OS_INCLUDE)/ptlib \
	           $(DESTDIR)$(includedir)/ptclib \
	           $(DESTDIR)$(datarootdir)/ptlib/make ; \
	do \
	    $(MKDIR_P) $$dir ; \
	    chmod 755 $$dir ; \
	done
	for lib in $(PTLIB_OPT_SHARED_FILE) $(PTLIB_OPT_SHARED_FILE).$(DEBUGINFOEXT) \
	           $(PTLIB_DEBUG_SHARED_FILE) $(PTLIB_DEBUG_SHARED_FILE).$(DEBUGINFOEXT) \
	           $(PTLIB_OPT_STATIC_FILE) \
	           $(PTLIB_DEBUG_STATIC_FILE) ; \
	do \
	   if test -e $$lib ; then \
	      $(INSTALL) -m 755 $$lib $(DESTDIR)$(libdir); \
	   fi \
	done
	cd $(DESTDIR)$(libdir) ; \
	$(LN_S) -f $(notdir $(PTLIB_OPT_SHARED_FILE)) $(notdir $(PTLIB_OPT_SHARED_LINK))
	if test -e $(PTLIB_DEBUG_SHARED_FILE); then \
	   cd $(DESTDIR)$(libdir) ; \
	   $(LN_S) -f $(notdir $(PTLIB_DEBUG_SHARED_FILE)) $(notdir $(PTLIB_DEBUG_SHARED_LINK)) ; \
	fi
	$(INSTALL) -m 444 include/ptlib.h $(DESTDIR)$(includedir)
	$(INSTALL) -m 444 include/ptlib_wx.h $(DESTDIR)$(includedir)
	$(INSTALL) -m 444 include/ptlib_config.h $(DESTDIR)$(includedir)
	for fn in include/ptlib/*.h include/ptlib/*.inl; \
	   do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptlib; \
	done
	for fn in include/ptlib/$(OS_INCLUDE)/ptlib/*.h include/ptlib/$(OS_INCLUDE)/ptlib/*.inl ; \
	   do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptlib/$(OS_INCLUDE)/ptlib ; \
	done
	for fn in include/ptclib/*.h ; \
	   do $(INSTALL) -m 444 $$fn $(DESTDIR)$(includedir)/ptclib; \
	done
	for fn in make/*.mak ; \
	   do $(INSTALL) -m 444 $$fn $(DESTDIR)$(datarootdir)/ptlib/make; \
	done
	$(INSTALL) -m 644 ptlib.pc $(DESTDIR)$(libdir)/pkgconfig
	$(foreach dir,$(SUBDIRS),$(MAKE) -C $(dir) install && ) true


uninstall:
	rm -rf $(DESTDIR)$(includedir)/ptlib \
	       $(DESTDIR)$(includedir)/ptclib \
	       $(DESTDIR)$(includedir)/ptlib.h \
	       $(DESTDIR)$(includedir)/ptlib_config.h \
	       $(DESTDIR)$(includedir)/ptbuildopts.h \
	       $(DESTDIR)$(datarootdir)/ptlib \
	       $(DESTDIR)$(libdir)/$(PTLIB_PLUGIN_DIR) \
	       $(DESTDIR)$(libdir)/pkgconfig/ptlib.pc
	rm -f $(DESTDIR)$(libdir)/$(notdir $(PTLIB_OPT_STATIC_FILE)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_DEBUG_STATIC_FILE)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_OPT_SHARED_FILE)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_DEBUG_SHARED_FILE)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_OPT_SHARED_FILE).$(DEBUGINFOEXT)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_DEBUG_SHARED_FILE).$(DEBUGINFOEXT)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_OPT_SHARED_LINK)) \
	      $(DESTDIR)$(libdir)/$(notdir $(PTLIB_DEBUG_SHARED_LINK))

endif # PTLIBDIR


# End of Makefile.in
