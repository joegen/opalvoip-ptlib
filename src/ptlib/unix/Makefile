
#
# Makefile for pwlib
#
# $Id: Makefile,v 1.13 1996/07/03 06:32:18 craigs Exp $
# $Log: Makefile,v $
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

DEBUG		= 1

SOURCES		= switch.cxx \
		  osutil.cxx \
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
		  uhttp.cxx \
		  uicmp.cxx \
		  uhttpsvc.cxx \
		  uhttpform.cxx \
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

ifdef REQUIRES_SEPARATE_SWITCH 
$(OBJDIR)/switch.o: ./switch.cxx
	@if [ ! -d $(OBJDIR) ] ; then mkdir $(OBJDIR) ; fi
	$(CPLUS) $(CFLAGS) $(STDCCFLAGS) -c $< -o $@
endif

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

DISTINCS = common/contain.h common/ptlib.h common/sockets.h common/svcproc.h \
           common/remconn.h common/telnet.h common/ftp.h common/http.h \
           common/html.h common/mailsock.h common/cypher.h
DISTSRCS = common/src/contain.cxx common/src/collect.cxx common/src/object.cxx 

distsrc:
	(cd $(PWLIBDIR); \
          find unix \( -name RCS -o -name obj_d -o -name obj_r \) -prune -o -type f -print > .list ; \
          grep -h '^#include' $(DISTINCS) | grep -v ptlib.inl | sed -e 's@^[^\"]*[<\"]@common/@' -e 's/[>\"]$$//' | sort -u >> .list ; \
          grep -l '^#include <ptlib.h>' common/src/*.cxx | sort -u >> .list ; \
          echo $(DISTINCS) $(DISTSRCS) >> .list ; \
	  tar -czvf ptlib-src.tgz `cat .list` ; rm .list )

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

$(OBJDIR)/uhttpsvc.o: uhttpsvc.cxx 

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
# Do not delete this line - make depend depends upon it

./$(OBJDIR)/osutil.o: osutil.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 /usr/include/fcntl.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/sys/time.h \
 /usr/include/linux/time.h /usr/include/langinfo.h \
 /usr/include/locale.h ../../common/osutil.inl \
 $(PWLIBDIR)/unix/include/ptlib.inl
./$(OBJDIR)/channel.o: channel.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 /usr/include/sys/ioctl.h /usr/include/linux/ioctl.h \
 /usr/include/asm/ioctl.h /usr/include/sys/socketio.h \
 /usr/include/linux/sockios.h
./tlib.o: tlib.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/pwd.h \
 /usr/include/sys/wait.h /usr/include/waitflags.h \
 /usr/include/linux/wait.h /usr/include/waitstatus.h \
 /usr/include/sys/mman.h /usr/include/linux/mman.h \
 /usr/include/asm/mman.h /usr/include/sys/utsname.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h uerror.h
./$(OBJDIR)/assert.o: assert.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
./$(OBJDIR)/config.o: config.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
./$(OBJDIR)/socket.o: socket.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h /usr/include/sys/ioctl.h \
 /usr/include/linux/ioctl.h /usr/include/asm/ioctl.h \
 /usr/include/sys/socketio.h /usr/include/sys/fcntl.h \
 /usr/include/fcntl.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h
./$(OBJDIR)/svcproc.o: svcproc.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/svcproc.h \
 $(PWLIBDIR)/unix/include/../../common/svcproc.h \
 /usr/include/syslog.h /usr/include/sys/syslog.h uerror.h
./$(OBJDIR)/remconn.o: remconn.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/remconn.h \
 $(PWLIBDIR)/unix/include/../../common/remconn.h \
 /usr/include/sys/ioctl.h /usr/include/linux/ioctl.h \
 /usr/include/asm/ioctl.h /usr/include/sys/socketio.h \
 /usr/include/linux/sockios.h /usr/include/sys/socket.h \
 /usr/include/linux/socket.h /usr/include/asm/socket.h \
 /usr/include/linux/uio.h /usr/include/linux/if.h uerror.h
./$(OBJDIR)/serchan.o: serchan.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 /usr/include/fcntl.h /usr/include/linux/fcntl.h \
 /usr/include/asm/fcntl.h /usr/include/sys/ioctl.h \
 /usr/include/linux/ioctl.h /usr/include/asm/ioctl.h \
 /usr/include/sys/socketio.h /usr/include/linux/sockios.h \
 ../../common/src/serial.cxx
./$(OBJDIR)/pipechan.o: pipechan.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 /usr/include/sys/ioctl.h /usr/include/linux/ioctl.h \
 /usr/include/asm/ioctl.h /usr/include/sys/socketio.h \
 /usr/include/linux/sockios.h /usr/include/sys/wait.h \
 /usr/include/waitflags.h /usr/include/linux/wait.h \
 /usr/include/waitstatus.h /usr/include/fcntl.h \
 /usr/include/linux/fcntl.h /usr/include/asm/fcntl.h \
 /usr/include/termio.h ../../common/src/pipechan.cxx
./$(OBJDIR)/utfile.o: utfile.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
./$(OBJDIR)/usfile.o: usfile.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
./$(OBJDIR)/uftp.o: uftp.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h $(PWLIBDIR)/common/ftp.h \
 ../../common/src/ftp.cxx
./$(OBJDIR)/umailsock.o: umailsock.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h \
 $(PWLIBDIR)/common/mailsock.h \
 $(PWLIBDIR)/common/mime.h ../../common/src/mailsock.cxx
./$(OBJDIR)/utelnet.o: utelnet.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h ../../common/src/telnet.cxx
./$(OBJDIR)/uappsock.o: uappsock.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h \
 $(PWLIBDIR)/common/mime.h ../../common/src/appsock.cxx \
 $(PWLIBDIR)/common/mailsock.h
./$(OBJDIR)/uhtml.o: uhtml.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/common/html.h ../../common/src/html.cxx
./$(OBJDIR)/ucypher.o: ucypher.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/common/cypher.h ../../common/src/cypher.cxx \
 $(PWLIBDIR)/common/mime.h \
 $(PWLIBDIR)/common/appsock.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h
./$(OBJDIR)/switch.o: switch.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
./$(OBJDIR)/uhttp.o: uhttp.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h \
 $(PWLIBDIR)/common/http.h $(PWLIBDIR)/common/mime.h \
 $(PWLIBDIR)/common/url.h $(PWLIBDIR)/common/html.h \
 ../../common/src/http.cxx
./$(OBJDIR)/uicmp.o: uicmp.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h ../../common/src/icmp.cxx
../../common/src/$(OBJDIR)/object.o: ../../common/src/object.cxx \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h
../../common/src/$(OBJDIR)/contain.o: ../../common/src/contain.cxx \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/common/contain.inl
../../common/src/$(OBJDIR)/osutils.o: ../../common/src/osutils.cxx \
 $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
../../common/src/$(OBJDIR)/sockets.o: ../../common/src/sockets.cxx \
 $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h \
 $(PWLIBDIR)/unix/include/sockets.h /usr/include/sys/time.h \
 /usr/include/linux/time.h \
 $(PWLIBDIR)/unix/include/../../common/sockets.h \
 $(PWLIBDIR)/unix/include/socket.h /usr/include/netinet/in.h \
 /usr/include/sys/socket.h /usr/include/linux/socket.h \
 /usr/include/asm/socket.h /usr/include/linux/sockios.h \
 /usr/include/linux/uio.h /usr/include/linux/in.h \
 /usr/include/asm/byteorder.h \
 $(PWLIBDIR)/unix/include/../../common/socket.h \
 $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
 /usr/include/sys/param.h /usr/include/linux/param.h \
 /usr/include/asm/param.h \
 $(PWLIBDIR)/unix/include/../../common/ipsock.h \
 $(PWLIBDIR)/unix/include/ipdsock.h \
 $(PWLIBDIR)/unix/include/../../common/ipdsock.h \
 $(PWLIBDIR)/unix/include/udpsock.h \
 $(PWLIBDIR)/unix/include/../../common/udpsock.h \
 $(PWLIBDIR)/unix/include/icmpsock.h \
 $(PWLIBDIR)/unix/include/../../common/icmpsock.h \
 $(PWLIBDIR)/unix/include/tcpsock.h \
 $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
 $(PWLIBDIR)/common/telnet.h \
 $(PWLIBDIR)/common/appsock.h
../../common/src/$(OBJDIR)/pmain.o: ../../common/src/pmain.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
../../common/src/$(OBJDIR)/ptime.o: ../../common/src/ptime.cxx $(PWLIBDIR)/common/ptlib.h \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h \
 $(PWLIBDIR)/unix/include/ptime.h \
 $(PWLIBDIR)/unix/include/../../common/ptime.h \
 /usr/include/time.h $(PWLIBDIR)/unix/include/timeint.h \
 $(PWLIBDIR)/unix/include/../../common/timeint.h \
 $(PWLIBDIR)/unix/include/timer.h \
 $(PWLIBDIR)/unix/include/../../common/timer.h \
 $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
 /usr/include/linux/limits.h /usr/include/linux/dirent.h \
 /usr/include/posix1_lim.h \
 $(PWLIBDIR)/unix/include/../../common/pdirect.h \
 $(PWLIBDIR)/unix/include/channel.h \
 $(PWLIBDIR)/unix/include/pmachdep.h /usr/include/netdb.h \
 /usr/include/paths.h /usr/include/sys/termios.h \
 /usr/include/termios.h /usr/include/linux/termios.h \
 /usr/include/asm/termios.h \
 $(PWLIBDIR)/unix/include/../../common/channel.h \
 $(PWLIBDIR)/unix/include/filepath.h \
 $(PWLIBDIR)/unix/include/../../common/filepath.h \
 $(PWLIBDIR)/unix/include/file.h /usr/include/sys/stat.h \
 /usr/include/linux/stat.h \
 $(PWLIBDIR)/unix/include/../../common/file.h \
 $(PWLIBDIR)/unix/include/textfile.h \
 $(PWLIBDIR)/unix/include/../../common/textfile.h \
 $(PWLIBDIR)/unix/include/sfile.h \
 $(PWLIBDIR)/unix/include/../../common/sfile.h \
 $(PWLIBDIR)/unix/include/pipechan.h /usr/include/signal.h \
 /usr/include/linux/signal.h /usr/include/asm/signal.h \
 $(PWLIBDIR)/unix/include/../../common/pipechan.h \
 $(PWLIBDIR)/unix/include/serchan.h \
 $(PWLIBDIR)/unix/include/../../common/serchan.h \
 $(PWLIBDIR)/common/modem.h \
 $(PWLIBDIR)/unix/include/config.h \
 $(PWLIBDIR)/unix/include/../../common/config.h \
 $(PWLIBDIR)/common/args.h \
 $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
 /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h \
 $(PWLIBDIR)/unix/include/../../common/thread.h \
 $(PWLIBDIR)/unix/include/pprocess.h \
 $(PWLIBDIR)/unix/include/../../common/pprocess.h \
 $(PWLIBDIR)/unix/include/semaphor.h \
 $(PWLIBDIR)/unix/include/../../common/semaphor.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/limits.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/syslimits.h \
 /usr/include/limits.h /usr/include/posix2_lim.h \
 $(PWLIBDIR)/unix/include/dynalink.h \
 $(PWLIBDIR)/unix/include/../../common/dynalink.h
../../common/src/$(OBJDIR)/collect.o: ../../common/src/collect.cxx \
 $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
 /usr/include/linux/types.h /usr/include/asm/types.h \
 /usr/include/sys/bitypes.h /usr/include/unistd.h \
 /usr/include/features.h /usr/include/sys/cdefs.h \
 /usr/include/posix_opt.h /usr/include/gnu/types.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stddef.h \
 /usr/include/confname.h /usr/include/ctype.h /usr/include/endian.h \
 /usr/include/bytesex.h \
 $(PWLIBDIR)/unix/include/../../common/contain.h \
 $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
 /usr/include/libio.h /usr/include/_G_config.h \
 /usr/lib/gcc-lib/i486-linux/2.7.2/include/stdarg.h \
 /usr/include/stdlib.h /usr/include/errno.h /usr/include/linux/errno.h \
 /usr/include/asm/errno.h /usr/include/alloca.h /usr/include/string.h \
 /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
 /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
 $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
 $(PWLIBDIR)/common/pstring.h
