
#
# Makefile for pwlib
#
# $Id: Makefile,v 1.3 1995/12/08 13:18:13 craigs Exp $
# $Log: Makefile,v $
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

LIBRARY		= pt_unix
OS		= unix

#CFLAGS		= -E

SOURCES		= osutil.cxx \
		  channel.cxx \
		  tlib.cxx \
		  os_main.cxx \
		  config.cxx \
		  socket.cxx \
		  pipechan.cxx \
		  serchan.cxx \
		  ../../common/src/object.cxx \
		  ../../common/src/contain.cxx \
		  ../../common/src/collect.cxx \
		  ../../common/src/osutils.cxx \
		  ../../common/src/sockets.cxx \
		  ../../common/src/appsock.cxx 

DEBUG		= 1
#P_USE_INLINES	= 1

include ~/pwlib/lib/ptlib.mak



# Do not delete this line - make depend depends upon it

./osutil.o: osutil.cxx /usr/include/fcntl.h /usr/include/features.h \
  /usr/include/sys/cdefs.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h \
  /usr/include/gnu/types.h /usr/include/linux/fcntl.h \
  /usr/include/sys/stat.h /usr/include/linux/stat.h /usr/include/stdlib.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h /usr/include/errno.h \
  /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/syslimits.h \
  /usr/include/limits.h /usr/include/posix1_lim.h \
  /usr/include/linux/limits.h /usr/include/posix2_lim.h /usr/include/time.h \
  /usr/include/sys/time.h /usr/include/linux/time.h \
  /usr/include/localeinfo.h /usr/include/ctype.h \
  $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/unistd.h \
  /usr/include/posix_opt.h /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/string.h \
  /usr/include/g++/iostream.h /usr/include/g++/streambuf.h \
  /usr/include/g++/iomanip.h $(PWLIBDIR)/common/array.h \
  $(PWLIBDIR)/common/lists.h $(PWLIBDIR)/common/dict.h \
  $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/dirent.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
  $(PWLIBDIR)/unix/include/../../common/channel.h \
  $(PWLIBDIR)/unix/include/filepath.h \
  $(PWLIBDIR)/unix/include/../../common/filepath.h \
  $(PWLIBDIR)/unix/include/file.h \
  $(PWLIBDIR)/unix/include/../../common/file.h \
  $(PWLIBDIR)/unix/include/textfile.h \
  $(PWLIBDIR)/unix/include/../../common/textfile.h \
  $(PWLIBDIR)/unix/include/sfile.h \
  $(PWLIBDIR)/unix/include/../../common/sfile.h \
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  ../../common/osutil.inl $(PWLIBDIR)/unix/include/ptlib.inl
./channel.o: channel.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  $(PWLIBDIR)/unix/include/sockets.h \
  $(PWLIBDIR)/unix/include/../../common/sockets.h \
  $(PWLIBDIR)/unix/include/socket.h /usr/include/fcntl.h \
  /usr/include/linux/fcntl.h /usr/include/termio.h /usr/include/termios.h \
  /usr/include/linux/termios.h /usr/include/sys/ioctl.h \
  /usr/include/linux/ioctl.h /usr/include/sys/socket.h \
  /usr/include/linux/socket.h /usr/include/linux/sockios.h \
  /usr/include/netdb.h /usr/include/paths.h \
  $(PWLIBDIR)/unix/include/../../common/socket.h \
  $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
  /usr/include/sys/param.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/syslimits.h \
  /usr/include/limits.h /usr/include/posix2_lim.h /usr/include/linux/param.h \
  /usr/include/asm/param.h /usr/include/sys/bitypes.h \
  /usr/include/netinet/in.h /usr/include/linux/in.h \
  /usr/include/asm/byteorder.h \
  $(PWLIBDIR)/unix/include/../../common/ipsock.h \
  $(PWLIBDIR)/unix/include/udpsock.h \
  $(PWLIBDIR)/unix/include/../../common/udpsock.h \
  $(PWLIBDIR)/unix/include/tcpsock.h \
  $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
  $(PWLIBDIR)/common/telnet.h $(PWLIBDIR)/common/appsock.h \
  /usr/include/sys/wait.h /usr/include/waitflags.h /usr/include/linux/wait.h \
  /usr/include/waitstatus.h /usr/include/endian.h /usr/include/bytesex.h
./tlib.o: tlib.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  /usr/include/sys/time.h /usr/include/linux/time.h /usr/include/pwd.h
./os_main.o: os_main.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  /usr/include/ctype.h
./config.o: config.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h
../../common/src/object.o: ../../common/src/object.cxx \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h
../../common/src/contain.o: ../../common/src/contain.cxx \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  /usr/include/ctype.h $(PWLIBDIR)/common/contain.inl
../../common/src/collect.o: ../../common/src/collect.cxx \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h
../../common/src/osutils.o: ../../common/src/osutils.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  /usr/include/ctype.h
../../common/src/sockets.o: ../../common/src/sockets.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  $(PWLIBDIR)/unix/include/sockets.h \
  $(PWLIBDIR)/unix/include/../../common/sockets.h \
  $(PWLIBDIR)/unix/include/socket.h /usr/include/fcntl.h \
  /usr/include/linux/fcntl.h /usr/include/termio.h /usr/include/termios.h \
  /usr/include/linux/termios.h /usr/include/sys/ioctl.h \
  /usr/include/linux/ioctl.h /usr/include/sys/socket.h \
  /usr/include/linux/socket.h /usr/include/linux/sockios.h \
  /usr/include/netdb.h /usr/include/paths.h \
  $(PWLIBDIR)/unix/include/../../common/socket.h \
  $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
  /usr/include/sys/param.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/syslimits.h \
  /usr/include/limits.h /usr/include/posix2_lim.h /usr/include/linux/param.h \
  /usr/include/asm/param.h /usr/include/sys/bitypes.h \
  /usr/include/netinet/in.h /usr/include/linux/in.h \
  /usr/include/asm/byteorder.h \
  $(PWLIBDIR)/unix/include/../../common/ipsock.h \
  $(PWLIBDIR)/unix/include/udpsock.h \
  $(PWLIBDIR)/unix/include/../../common/udpsock.h \
  $(PWLIBDIR)/unix/include/tcpsock.h \
  $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
  $(PWLIBDIR)/common/telnet.h $(PWLIBDIR)/common/appsock.h
../../common/src/appsock.o: ../../common/src/appsock.cxx $(PWLIBDIR)/common/ptlib.h \
  $(PWLIBDIR)/unix/include/contain.h /usr/include/sys/types.h \
  /usr/include/linux/types.h /usr/include/asm/types.h /usr/include/unistd.h \
  /usr/include/features.h /usr/include/sys/cdefs.h /usr/include/posix_opt.h \
  /usr/include/gnu/types.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stddef.h \
  /usr/include/confname.h \
  $(PWLIBDIR)/unix/include/../../common/contain.h \
  $(PWLIBDIR)/common/object.h /usr/include/stdio.h \
  /usr/include/libio.h /usr/include/_G_config.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/stdarg.h /usr/include/stdlib.h \
  /usr/include/errno.h /usr/include/linux/errno.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/float.h /usr/include/alloca.h \
  /usr/include/string.h /usr/include/g++/iostream.h \
  /usr/include/g++/streambuf.h /usr/include/g++/iomanip.h \
  $(PWLIBDIR)/common/array.h $(PWLIBDIR)/common/lists.h \
  $(PWLIBDIR)/common/dict.h $(PWLIBDIR)/common/pstring.h \
  $(PWLIBDIR)/unix/include/ptime.h \
  $(PWLIBDIR)/unix/include/../../common/ptime.h /usr/include/time.h \
  $(PWLIBDIR)/unix/include/timeint.h \
  $(PWLIBDIR)/unix/include/../../common/timeint.h \
  $(PWLIBDIR)/unix/include/timer.h \
  $(PWLIBDIR)/unix/include/../../common/timer.h \
  $(PWLIBDIR)/unix/include/pdirect.h /usr/include/dirent.h \
  /usr/include/linux/limits.h /usr/include/linux/dirent.h \
  /usr/include/posix1_lim.h \
  $(PWLIBDIR)/unix/include/../../common/pdirect.h \
  $(PWLIBDIR)/unix/include/channel.h \
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
  $(PWLIBDIR)/unix/include/pipechan.h \
  $(PWLIBDIR)/unix/include/../../common/pipechan.h \
  $(PWLIBDIR)/unix/include/serchan.h \
  $(PWLIBDIR)/unix/include/../../common/serchan.h \
  $(PWLIBDIR)/common/modem.h \
  $(PWLIBDIR)/unix/include/config.h \
  $(PWLIBDIR)/unix/include/../../common/config.h \
  $(PWLIBDIR)/common/args.h \
  $(PWLIBDIR)/unix/include/thread.h /usr/include/setjmp.h \
  /usr/include/jmp_buf.h /usr/include/i386/jmp_buf.h /usr/include/signal.h \
  /usr/include/linux/signal.h \
  $(PWLIBDIR)/unix/include/../../common/thread.h \
  $(PWLIBDIR)/unix/include/process.h \
  $(PWLIBDIR)/unix/include/../../common/process.h \
  $(PWLIBDIR)/unix/include/dynalink.h \
  $(PWLIBDIR)/unix/include/../../common/dynalink.h \
  $(PWLIBDIR)/unix/include/sockets.h \
  $(PWLIBDIR)/unix/include/../../common/sockets.h \
  $(PWLIBDIR)/unix/include/socket.h /usr/include/fcntl.h \
  /usr/include/linux/fcntl.h /usr/include/termio.h /usr/include/termios.h \
  /usr/include/linux/termios.h /usr/include/sys/ioctl.h \
  /usr/include/linux/ioctl.h /usr/include/sys/socket.h \
  /usr/include/linux/socket.h /usr/include/linux/sockios.h \
  /usr/include/netdb.h /usr/include/paths.h \
  $(PWLIBDIR)/unix/include/../../common/socket.h \
  $(PWLIBDIR)/unix/include/ipsock.h /usr/include/arpa/inet.h \
  /usr/include/sys/param.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/limits.h \
  /usr/lib/gcc-lib/i486-linux/2.6.3/include/syslimits.h \
  /usr/include/limits.h /usr/include/posix2_lim.h /usr/include/linux/param.h \
  /usr/include/asm/param.h /usr/include/sys/bitypes.h \
  /usr/include/netinet/in.h /usr/include/linux/in.h \
  /usr/include/asm/byteorder.h \
  $(PWLIBDIR)/unix/include/../../common/ipsock.h \
  $(PWLIBDIR)/unix/include/udpsock.h \
  $(PWLIBDIR)/unix/include/../../common/udpsock.h \
  $(PWLIBDIR)/unix/include/tcpsock.h \
  $(PWLIBDIR)/unix/include/../../common/tcpsock.h \
  $(PWLIBDIR)/common/telnet.h $(PWLIBDIR)/common/appsock.h
