
#
# Makefile for pwlib
#
# $Id: Makefile,v 1.9 1996/05/10 11:03:22 craigs Exp $
# $Log: Makefile,v $
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

DEBUG		= 1

SOURCES		= osutil.cxx \
		  channel.cxx \
		  tlib.cxx \
		  assert.cxx \
		  config.cxx \
		  socket.cxx \
		  svcproc.cxx \
		  remconn.cxx \
		  serchan.cxx \
		  pipechan.cxx \
		  utfile.cxx \
		  usfile.cxx \
		  uftp.cxx \
		  umailsock.cxx \
		  utelnet.cxx \
		  uappsock.cxx \
		  uhtml.cxx \
		  ucypher.cxx \
		  switch.cxx \
		  uhttp.cxx \
		  ../../common/src/object.cxx \
		  ../../common/src/contain.cxx \
		  ../../common/src/osutils.cxx \
		  ../../common/src/sockets.cxx \
		  ../../common/src/pmain.cxx \
		  ../../common/src/ptime.cxx \
		  ../../common/src/collect.cxx \

#######################################################

include unix.mak

#######################################################

all:	$(PTLIB_FILE)

$(OBJDIR)/switch.o: ./switch.cxx
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) -c $< -o $@

OBJS	:= $(OBJS) $(OBJDIR)/switch.o

CLEAN_FILES	:= $(CLEAN_FILES) $(PTLIB_FILE)

thelot:
	make both
	( cd ../../apps/mibmastr; make clean ; make both; )
	( cd ../../apps/firedoor; make clean ; make both; )

test:
	(cd $(PWLIBDIR); \
          grep -h '^#include' common/contain.h common/ptlib.h common/sockets.h \
            | grep -v ptlib.inl \
            | sed -e 's@^[^\"]*[<\"]@common/@' -e 's/[>\"]$$//' \
	)

distsrc:
	(cd $(PWLIBDIR); \
          find unix \( -name RCS -o -name obj_d -o -name obj_r \) -prune -o -type f -print > .list ; \
          grep -h '^#include' common/contain.h common/ptlib.h common/sockets.h | grep -v ptlib.inl | sed -e 's@^[^\"]*[<\"]@common/@' -e 's/[>\"]$$//' | sort -u >> .list ; \
          grep -l '^#include <ptlib.h>' common/src/*.cxx | sort -u >> .list ; \
          echo common/ptlib.h common/src/contain.cxx common/src/collect.cxx common/src/object.cxx >> .list ; \
	  tar -czvf ptlib-src.tgz `cat .list` ; rm .list )

#######################################################

include common.mak

#######################################################

$(PTLIB_FILE): $(OBJS)
	@if [ ! -d $(LIBDIR) ] ; then mkdir $(LIBDIR) ; fi
ifndef RANLIB
	ar rcs $(PTLIB_FILE) $(OBJS)
else
	ar rc $(PTLIB_FILE) $(OBJS)
	ranlib $(PTLIB_FILE)
endif


$(OBJDIR)/osutil.o: ./osutil.cxx 

$(OBJDIR)/channel.o: channel.cxx 

$(OBJDIR)/tlib.o: tlib.cxx 

$(OBJDIR)/assert.o: assert.cxx 

$(OBJDIR)/config.o: config.cxx 

$(OBJDIR)/socket.o: socket.cxx 

$(OBJDIR)/pipechan.o: pipechan.cxx 

$(OBJDIR)/serchan.o: serchan.cxx 

$(OBJDIR)/svcproc.o: svcproc.cxx 

$(OBJDIR)/remconn.o: remconn.cxx 

$(OBJDIR)/uappsock.o: uappsock.cxx 

$(OBJDIR)/uhttp.o: uhttp.cxx 

$(OBJDIR)/object.o: ../../common/src/object.cxx 

$(OBJDIR)/contain.o: ../../common/src/contain.cxx 

$(OBJDIR)/collect.o: ../../common/src/collect.cxx 

$(OBJDIR)/osutils.o: ../../common/src/osutils.cxx 

$(OBJDIR)/sockets.o: ../../common/src/sockets.cxx 

$(OBJDIR)/appsock.o: ../../common/src/appsock.cxx 

$(OBJDIR)/cypher.o: ../../common/src/cypher.cxx 

$(OBJDIR)/html.o: ../../common/src/html.cxx 

$(OBJDIR)/http.o: ../../common/src/http.cxx 

$(OBJDIR)/pmain.o: ../../common/src/pmain.cxx 

$(OBJDIR)/ptime.o: ../../common/src/ptime.cxx 

$(OBJDIR)/telnet.o: ../../common/src/telnet.cxx 

$(OBJDIR)/mailsock.o: ../../common/src/mailsock.cxx 

$(OBJDIR)/ftp.o: ../../common/src/ftp.cxx 
