
#
# Makefile for pwlib
#
# $Id: Makefile,v 1.25 1998/02/12 04:17:55 robertj Exp $
# $Log: Makefile,v $
# Revision 1.25  1998/02/12 04:17:55  robertj
# Made DEBUG variable optional
#
# Revision 1.24  1998/01/26 07:25:21  robertj
# Removed tiny files, as now have pragma in the common files.
#
# Revision 1.22  1998/01/04 09:41:20  robertj
# Changed perasn.cxx to asner.cxx
#
# Revision 1.21  1998/01/04 08:29:37  craigs
# Changed for Linux
#
# Revision 1.20  1997/04/22 11:00:01  craigs
# Added SSL and DLL functions
#
# Revision 1.19  1996/11/17 08:35:50  craigs
# Fixed distsrc target
#
# Revision 1.18  1996/11/17 02:35:12  craigs
# Removed SSL from list of build targets
#
# Revision 1.16  1996/10/31 10:30:03  craigs
# Lots of changes
#
# Revision 1.15  1996/09/21 05:41:41  craigs
# Added new common files
#
# Revision 1.14  1996/08/03 12:05:10  craigs
# *** empty log message ***
#
# Revision 1.13  1996/07/03 06:32:18  craigs
# Removed spurious lines causing warnings during library make
#
# Revision 1.12  1996/06/29 11:37:28  craigs
# Added unix system type to object directories
# Added support for Linux shared libraries
#
# Revision 1.11  1996/06/19 01:56:26  craigs
# Added uhttpsvc to the list
#
# Revision 1.10  1996/06/14 09:30:43  craigs
# Included buildinc stuff
#
# Revision 1.9  1996/05/10 11:03:22  craigs
# Fixed distsrc rule to only include ptlib stuff
#
# Revision 1.8  1996/05/03 13:48:58  craigs
# Sun4 fixes
#
# Revision 1.7  1996/05/02  10:53:24  craigs
# Fixed problem with SUN4 target
#
# Revision 1.6  1996/04/17 11:12:20  craigs
# Latest version pre mibmaster release 1.0
#
# Revision 1.5  1996/04/15 10:50:13  craigs
# Last revision prior to release of MibMaster
#
# Revision 1.4  1996/01/26 11:09:04  craigs
# *** empty log message ***
#
# Revision 1.3  1995/12/08 13:18:13  craigs
# Lots of changes
#
# Revision 1.2  1995/07/09 00:35:42  craigs
# Latest and greatest omnibus change
#
# Revision 1.1  1995/01/16 20:50:44  craigs
# Initial revision
#
# Revision 1.1  1993/07/12  21:22:16  craigs
# Initial revision
#
#

ifndef DEBUG
DEBUG		= 1
endif

COMMONSRC	= ../../common/ptlib/src

SOURCES		= switch.cxx \
		  osutil.cxx \
		  channel.cxx \
		  tlib.cxx \
		  assert.cxx \
		  config.cxx \
		  remconn.cxx \
		  serchan.cxx \
		  pipechan.cxx \
		  svcproc.cxx \
		  utfile.cxx \
		  usfile.cxx \
		  socket.cxx \
		  uicmp.cxx \
		  udll.cxx \
		  $(COMMONSRC)/object.cxx \
		  $(COMMONSRC)/contain.cxx \
		  $(COMMONSRC)/osutils.cxx \
		  $(COMMONSRC)/ptime.cxx \
		  $(COMMONSRC)/collect.cxx \
		  $(COMMONSRC)/sockets.cxx \
		  $(COMMONSRC)/inetprot.cxx \
		  $(COMMONSRC)/httpclnt.cxx \
		  $(COMMONSRC)/httpsrvr.cxx \
		  $(COMMONSRC)/html.cxx \
		  $(COMMONSRC)/http.cxx \
		  $(COMMONSRC)/httpform.cxx \
		  $(COMMONSRC)/httpsvc.cxx \
		  $(COMMONSRC)/inetmail.cxx \
		  $(COMMONSRC)/ftp.cxx \
		  $(COMMONSRC)/ftpclnt.cxx \
		  $(COMMONSRC)/ftpsrvr.cxx \
		  $(COMMONSRC)/telnet.cxx \
		  $(COMMONSRC)/pasn.cxx \
		  $(COMMONSRC)/psnmp.cxx \
		  $(COMMONSRC)/snmpclnt.cxx \
		  $(COMMONSRC)/snmpserv.cxx \
		  $(COMMONSRC)/cypher.cxx \
		  $(COMMONSRC)/pssl.cxx \
		  $(COMMONSRC)/asner.cxx 

OBJS	= $(OBJDIR)/regex.o

#######################################################

include unix.mak

#######################################################

all:	$(PTLIB_FILE)

ifdef REQUIRES_SEPARATE_SWITCH 
$(OBJDIR)/switch.o: ./switch.cxx
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) -c $< -o $@
endif

CLEAN_FILES	:= $(CLEAN_FILES) $(PTLIB_FILE)

$(OBJDIR)/regex.o:	../../common/ptlib/src/regex.cxx
	$(CPLUS) -DREGEX_MALLOC -D__STDC__ -DSTDC_HEADERS $(CFLAGS) $(STDCCFLAGS) -c -w $< -o $@

thelot:
	make alllibs
	( cd ../../apps/mibmastr; make clean ; make both; )
	( cd ../../apps/firedoor; make clean ; make both; )

test:
	(cd $(PWLIBDIR); \
          grep -h '^#include' common/contain.h common/ptlib.h common/sockets.h \
            | grep -v ptlib.inl \
            | sed -e 's@^[^\"]*[<\"]@common/@' -e 's/[>\"]$$//' \
	)

distsrc:
	(cd $(PWLIBDIR); tar -czvf ptlib-src.tgz \
	unix/src/*.cxx unix/src/*.h unix/src/Makefile unix/src/*.mak \
	unix/include/* common/*.h common/ptlib/* common/ptlib/src/* )

#######################################################

include common.mak

#######################################################


ifdef SHAREDLIB
$(PTLIB_FILE): $(PTLIB_FILE).1
	@if [ ! -e $(PTLIB_FILE) ] ; then ln -s $(PTLIB_FILE).1 $(PTLIB_FILE) ; fi

$(PTLIB_FILE).1: $(PTLIB_FILE).1.0
	@if [ ! -e $(PTLIB_FILE).1 ] ; then ln -s $(PTLIB_FILE).1.0 $(PTLIB_FILE).1 ; fi

$(PTLIB_FILE).1.0: $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
	gcc -shared -Wl,-soname,$(PTLIB_FILE).1 -o $(PTLIB_FILE).1.0 $(OBJS)

else

$(PTLIB_FILE): $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
ifndef RANLIB
	ar rcs $(PTLIB_FILE) $(OBJS)
else
	ar rc $(PTLIB_FILE) $(OBJS)
	ranlib $(PTLIB_FILE)
endif

endif

