			Portable Windows Libary
			=======================


Contents
--------

	1.	Introduction
	2.	Apologies
	3.	CVS Access
	4.	Building PWLib
	5.	Using PWLib
	6.	Platform Specific Issues
	7.	Conclusion
        8.      Licensing



1. Introduction
---------------

PWLib is a moderately large class library that has its genesis many years ago as
a method to product applications to run on both Microsoft Windows and Unix
X-Windows systems. It also was to have a Macintosh port as well but this never
eventuated.

Since then the system has grown to having quite good application to areas other
than mere Windows GUI portability. Classes for I/O portability, multi-threading
portability, aid in producing unix daemons and NT services portably and all
sorts of internet protocols were added over the years.

All this over and above basic "container" classes such as arrays, linear lists,
sorted lists (RB Tree) and dictionaries (hash tables) which were all created
before STL became the answer to all our prayers. Unfortunately, due to intertia
and the GNU C++ inadequate support of templates, this library will probably not
be ported to STL in the near future.

The library was used extensively for all our in-house products. Then we decided
to support the open H323 project by throwing in some of the code written for
one of our products. Thus, required PWLib so it got thrown into the open source
world as well.

2. Apologies (not)
------------------

As you start using the library, the inevitable question "why did they do it that
way?" will come up. The more experienced out there will know that there are
several reasons for the way things are:

   *   Carefully considered design,
   *   Workarounds for portability and compiler subtleties,
   *   History, it may be too hard to change an early design decision,
   *   Complete arbitrariness, the absence of any compelling reason.

So, when you ask the next question "why didn't you do it this way?" The answer
will be one of the above. The last one being a synonym for "we didn't think of
that!"

The bottom line is, use the library as is or change it as you require. You can
even send in suggestions for improvements (or merely changes) and we may (or may
not) include them in the base line code. Just do not send us any mail starting
with the words "Why did you..." as the answer is quite likely to be "Because!"


3. CVS Access
-------------

There is a public CVS archive available at cvs.openh323.org. Note that there are
still some parts of PWLib that are not available, so make sure you use the
modules provided for check out and do not just try and check out "pwlib" on it's
own. If you do all you will get is this file.

The modules available are:
	ptlib_unix
	pwlib_xlib
	ptlib_win32
	pwlib_win32


4. Building PWLib
-----------------

For Windows.

1.	Start MSVC (v5 or v6). If you have another compiler you are on your 
        own!  Go into the Tools menu, Options item, Directories tab and add 
	to the Include files path:
		C:\PWLib\Include\PwLib\MSWIN      (if have full version)
		C:\PWLib\Include\PtLib\MSOS
		C:\PWLib\Include
	and add to the Lib Files path and the Executable Files path the
	following:
		C:\PWLib\Lib
        Also make sure this directory is in your PATH environment variable.

2.	Open the pwlib.dsw file in the pwlib top directory. If you have the
	minimum library it will come up with several requests to find .dsp
	files, just cancel past these.

3.	Note you will need bison and flex to compile the system. You can get 
	a copy from http://www.openh323.org/bin/flexbison.zip, follow the 
	instructions included in that package and put the executables 
	somewhere in your path.

4.	Use the Batch Build command and build the "ASNParser - Win32 Release",
        "pwtest - Win32 Release" and "pwtest - Win32 Debug" targets. make sure
        all other targets are not checked.

5.	That it, you are now on your own!


For unix.

1.	If you have not put pwlib it into your home directory (~/pwlib) then
	you will have to defined the environment variable PWLIBDIR to point to
	the correct directory.
        Also make sure you have added the $PWLIBDIR/lib directory to your 
        LD_LIBRARY_PATH environment variable if you intend to use shared 
        libraries (the default).

2.	Build the debug and release versions of the PWLib library as follows:
		cd ~/pwlib
		make both
	This may take some time. Note, you will need bison and flex for this to
	compile, most unix systems have these. WARNING: there is a bug in most 
	of the bison.simple files. See below for details.

	PWLib requires GNU Make. If GNU Make (gmake) is not your default make
	program (eg FreeBSD users), you will need to install GNU Make first
	and then use
		cd ~/pwlib
		gmake both


	If you are getting huge numbers of errors during the compile, then it 
        is likely your platform is not supported, or you have incorrectly set 
        the OSTYPE and MACHTYPE variables.

3.	That's all there is to it, you are now on your own!



Bison problem under Unix

The bison.simple file on many releases will not compile with the options used 
by the PWLib getdate.y grammar. The options are required to make the date 
parser thread safe so it is necessary to edit the bison.simple file to fix the 
problem.

The file is usually at /usr/lib/bison.simple but in the tradition of unix 
could actually be anywhere. We leave it up to you to find it.

The code:

	/* Prevent warning if -Wstrict-prototypes. */
	#ifdef __GNUC__
	int yyparse (void);
	#endif

should be changed to

	/* Prevent warning if -Wstrict-prototypes. */
	#ifdef __GNUC__
	#ifndef YYPARSE_PARAM
	int yyparse (void);
	#endif
	#endif

To prevent the incorrect function prototype from being defined. The getdate.y 
should then produce a getdate.tab.c file that will actually compile.


5. Using PWLib
--------------

What documentation there is consists of this document and all of the header
files. It was intended that a post processer go through the header files and
produces HTML help files, but this never got completed.

Detailed tutorials will almost certainly not be forthcoming. However, at least
giving you an indication on how to start an application would be usefull, so
here is the infamous "Hello world!" program.


// hello.cxx

#include <ptlib.h>

class Hello : public PProcess
{
  PCLASSINFO(Hello, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(Hello)

void Hello::Main()
{
  cout << "Hello world!\n";
}

// End of hello.cxx


The CREATE_PROCESS macro actually defines the main() function and creates an
instance of Hello. This assures that everything is initialised in the correct
order. C++ does initialisation of global statics badly (and destruction is even
worse), so try to put everything into your PProcess descedent rather than
globals.

A GUI application is very similar but is descended off PApplication rather than
PProcess, and would create a window as a descendent off the PMainWindow class.

The following is a simple Makefile for Unix platforms for the hello world 
program.


# Simple makefile for PTLib

PROG    = hello
SOURCES = hello.cxx

ifndef PWLIBDIR
PWLIBDIR=$(HOME)/pwlib
endif

include $(PWLIBDIR)/make/ptlib.mak

# End of Makefile



PWlib Classes
=============

The classes in PWLib fall into the following broad categories

	Containers
	I/O
	Threads & Processes
	GUI


Containers

While there are a number of container classes you wourld rarely actually descend
off them, you would use macros that declare type safe descendents. These are
simply templates instantiations when using a compiler that supports templates
in a simple manner (GNU C++ does not qualify in our opinion).

I/O

There are many classes descendend from a basic primitive call a PChannel, which
represents an entity for doing I/O. There are classes for files, serial ports,
various types of socket and pipes to sub-processes.

Threads & Processes

These classes support lightweight threading and functionality to do with the
process as a whole (for example argument parsing). The threading will be
pre-emptive on platforms that support it (Win32, platforms with pthreads eg
Linux and FreeBSD) and cooperative on those that don't.

GUI

There are a very large number of classes to support a GUI interface. This is not
very complete at this time. The Win32 implementation is quite usable, though it
doesn't include the latest and greatest out of Redmond. The pure xlib
implementation has quite a lot implemented but is by no means complete. A motif
implementation is in the works but has not progressed very far.


6. Platform Specific Issues
---------------------------
PWLib has been ported to several platforms. However on some systems not all of
the functionality has been implemented. This could be due to lack of support
at the OS level or simply due to lack of time or documentation when developing
the port.

6.1 FreeBSD Issues
------------------
Port Maintained by Roger Hardiman <roger@freebsd.org>
GetRouteTable() in socket.cxx has been added. It is used by
OenH323Proxy, but is not fully tested.

6.2 OpenBSD Issues
------------------
Port Maintained by Roger Hardiman <roger@freebsd.org>
GetRouteTable() in socket.cxx has been added. It is used by
OenH323Proxy, but is not fully tested.

6.3 NetBSD Issues
-----------------
Port Maintained by Roger Hardiman <roger@freebsd.org>
GetRouteTable() in socket.cxx has been added. It is used by
OenH323Proxy, but is not fully tested.

Video Capture using the bktr driver has not been added, just due to a lack
of time. It is easy to add, being 99.99% identical to the FreeBSD and
OpenBSD support.

6.4 Mac OS X (Darwin) Issues
----------------------------
Port Maintained by Roger Hardiman <roger@freebsd.org> but recently
Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu> has been leading
development.
Threads cannot be suspended once they are running, and trying to Suspend
a running thread will generate an Assertion Error.
Theads can be created in 'suspended' mode and then started with Resume
This is due to a lack of pthread_kill() in Dawrin 1.2
See http://www.publicsource.apple.com/bugs/X/Libraries/2686231.html

GetRouteTable() in socket.cxx has been added. It is used by
OenH323Proxy, but is not fully tested.

localtime_r() and gm_time() are missing.
So in osutil.cxx I have implemented os_localtime() and os_gmtime()
with localtime() and gm_time() which may not be thread safe.

Audio is supported through the Enlightenment Sound Daemon (esd) or EsounD.
You must have this installed this, and set the ESDDIR environment
variable to point to it prior to compiling PWLib.

There is no video support due to a lack of documentation and hardware.

6.5 BeOS Issues
---------------
Port Maintained by Yuri Kiryanov <openh323@kiryanov.com>. 
Current version supported is BeOS 5.0.2. 

Most important issue is lack of variable sample frequency from system sound producer node.
I made quite a few attempts to implement sound resampler in code, 
even with help of Be engineers, but eventually decided to wait until new Media Kit
with resampler built-in. 
Also network code needed more things, as OOB, which was promised in BONE. 
BONE will allow to make less #defines in network code as well.
As update will hit the Net, I'll get back to it ASAP.  

Look for more port-related info on http://www.dogsbone.com/be

6.6 Windows CE Issues
---------------------

Port Maintained by Yuri Kiryanov <openh323@kiryanov.com>. 
Versions supported is 2.x and 3.x (PocketPC). 

I haven't made all variations of target platforms executables and 
only tested on emulator and ARM, both 2.11  and PocketPC.

Note that asnparser is being built separately on NT and then used for .asn file processing. 

Biggest issue is poor multithreading capabilities of current implementation of Windows CE. 
Therefore I doubt CE 2.11 build could be useful at all. PocketPC builds look better.

Look for more port-related info on http://www.dogsbone.com/ce

6.7 Solaris Issues
------------------
On Solaris 8, you need to install GNU Ld (the loader) to get
shared libraries to compile. (otherwise there is an error with -soname)
You can get around this by using the static libraries and
compiling with make optnoshared and make debugnoshared

There is currently no implementation of GetRouteTable() in socket.cxx
so OpenH323Proxy will not work.


7. Conclusion
-------------

This package is far from a "product". There is very limited documentation and
support will be on an ad-hoc basis, send us an e-mail and we will probably
answer your question if it isn't too difficult.

It is supplied mainly to support the open H323 project, but that shouldn't stop
you from using it in whatever project you have in mind if you so desire. We like
it and use it all the time, and we don't want to get into any religious wars of
this class library over that one.

Look for more port-related info on http://www.dogsbone.com/ce



8. Licensing                 
------------

The bulk of this library is licensed under the MPL (Mozilla Public License)
version 1.0. In simple terms this license allows you to use the library for 
any purpose, commercial or otherwise, provided the library is kept in tact
as a separate entity and any changes made to the library are made publicly
available under the same (MPL) license. It is important to realise that that 
refers to changes to the library and not your application that is merely 
linked to the library.

Note that due to a restriction in the GPL, any application you write that 
uses anything another than GPL, eg our library with MPL, is technically in
breach of the GPL license. However, it should be noted that MPL does not
care about the license of the final application, and as only the author of
the GPL application is in breach of his own license and is unlikely to sue
themselves for that breach, in practice there is no problem with a GPL 
application using an MPL or any other commercial library.


The random number generator is based on code originally by Bob Jenkins.


Portions of this library are from the REGEX library and is under the
following license:

Copyright 1992, 1993, 1994, 1997 Henry Spencer.  All rights reserved.
This software is not subject to any license of the American Telephone
and Telegraph Company or of the Regents of the University of California.

Permission is granted to anyone to use this software for any purpose on
any computer system, and to alter it and redistribute it, subject
to the following restrictions:

1. The author is not responsible for the consequences of use of this
   software, no matter how awful, even if they arise from flaws in it.

2. The origin of this software must not be misrepresented, either by
   explicit claim or by omission.  Since few users ever read sources,
   credits must appear in the documentation.

3. Altered versions must be plainly marked as such, and must not be
   misrepresented as being the original software.  Since few users
   ever read sources, credits must appear in the documentation.

4. This notice may not be removed or altered.



================================================================================
Equivalence Pty. Ltd.
Home of FireDoor, MibMaster & PhonePatch

support@equival.com.au
http://www.equival.com.au (US Mirror - http://www.equival.com)
Fax: +61 2 4368 1395   Voice: +61 2 4368 2118
