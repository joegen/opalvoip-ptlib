			Portable Tools Libary
			=====================


Contents
--------

   1.  Introduction
   2.  Apologies
   3.  CVS Access
   4.  Building PTLib
   5.  Using PTLib
   6.  IPv6 issues
   7.  Platform Specific Issues
   8.  Conclusion
   9.  Licensing



================================================================================

1. Introduction
---------------

PTLib (Portable Tools Library) is a moderately large class library that has it's
genesis many years ago as PWLib (portable Windows Library), a method to product
applications to run on both Microsoft Windows and Unix systems. It has also been
ported to other systems such as Mac OSX, VxWorks and other embedded systems

Since then the system has grown to include many classes that assist in writing
complete multi-platform applications. Classes for I/O portability, multi-
threading portability, aid in producing unix daemons and NT services portably
and all sorts of internet protocols were added over the years. So it became a
Portable Tools Library and was renamed to PTLib.

All this over and above basic "container" classes such as arrays, linear lists,
sorted lists (RB Tree) and dictionaries (hash tables) which were all created
before STL was standardized. Future versions of PTLib will see many of these
classes replaced or supplemented by STL.

The library was used extensively for all our in-house products. Then we decided
to support OpenH323, and then Opal, by throwing in some of the code written for
one of our products. Thus, required PTLib so it got thrown into the open source
world as well.



================================================================================

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



================================================================================

3. SVN Access
-------------

There is a public SVN archive available at svn.sourceforge.net. To extract, use a
command line like the following:

    svn co http://opalvoip.svn.sourceforge.net/svnroot/opalvoip/ptlib/trunk

If you would like see the structure of the SVN, then use the View SVN tool at:

    http://opalvoip.svn.sourceforge.net/viewvc/opalvoip/

================================================================================

4. Building PTLib
-----------------

This library is multi-platform, however there are only two major build systems
that are used. The Microsoft DevStudio environment for Windows and the GNU make
system for all of the various unix systems.

4.1. For Windows
----------------

See the online documentation at:

   http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLib

4.2. For Linux/Unix
-------------------

See the online documentation at:

   http://www.opalvoip.org/wiki/index.php?n=Main.BuildingPTLibUnix

================================================================================

5. Using PTLib
--------------

What documentation there is consists of this document and all of the header
files. It was intended that a post processer go through the header files and
produces HTML help files, but this never got completed.


5.1. Tutorial
-------------

Detailed tutorials will almost certainly not be forthcoming. However, at least
giving you an indication on how to start an application would be useful, so
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

ifndef PTLIBDIR
PTLIBDIR=$(HOME)/ptlib
endif

include $(PTLIBDIR)/make/ptlib.mak

# End of Makefile



--------------------------------------------------------------------------------
5.2. PTlib Classes
------------------

The classes in PTLib fall into the following broad categories

	Containers
	I/O
	Threads & Processes


5.2.1. Containers

While there are a number of container classes you wourld rarely actually descend
off them, you would use macros that declare type safe descendents. These are
simply templates instantiations when using a compiler that supports templates
in a simple manner (GNU C++ does not qualify in our opinion).

5.2.2. I/O

There are many classes descendend from a basic primitive call a PChannel, which
represents an entity for doing I/O. There are classes for files, serial ports,
various types of socket and pipes to sub-processes.

5.2.3. Threads & Processes

These classes support lightweight threading and functionality to do with the
process as a whole (for example argument parsing). The threading will be
pre-emptive on platforms that support it (Win32, platforms with pthreads eg
Linux and FreeBSD) and cooperative on those that don't.


================================================================================

6. IPv6 support in ptlib
------------------------

The IPv6 support in PTlib is supported and can be enabled or disabled via
the configure program.

When compiled with the IPv6 support, applications using only IPv4 are still 
fully backward compatible. PTLib is able to manage simultaneously IPv4 and
IPv6 connections.

6.1 Removed
-----------

6.2 Removed
-----------

6.3. Testing
------------

The test application sources can be found in the directory: opal/samples/simple
Once compiled the binaries are in simple/debug, release, obj_linux_x86_d, or
obj_linux_x86_r.
Under windows, the test application is simpleopal.exe
Under linux, the test application is simpleopal
IPv6 support can be tested on only one machine. Just open two shell/command windows.



6.3.1. IPv6 Address and port notation
-------------------------------------
IPv4 address and port are written in dot notation: xx.xx.xx.xx:4000
IPv6 global address are written in semi-colon notation: [xx:xx:xx:xx::xx]:4000
IPv6 scoped address ad a field for the scope: [xx:xx:xx:xx::xx%scope]:4000

Exemples:
Global address
[3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5]:4000
[3ffe:0b80:0002:f9c1::500b:0ea5]:4000

Scoped address
[fe80::232:56ff:fe95:315%lnc0]:4000
Scoped address are not supported yet.



6.3.2. Tests configuration
--------------------------
Tests 1,2,3 run on a single dual stack machine.
  IPv4 Address: 127.0.0.1, 10.0.0.6
  IPv6 Address: ::1, 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5

Tests 4,5,6 run on two dual stack machine.
PC1
  IPv4 Address: 10.0.0.6
  IPv6 Address: ::1, 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5
PC2
  IPv4 Address: 10.0.0.8
  IPv6 Address: ::1, 3ffe:0b80:0002:f9c1:0000:0000:500b:0eb6



6.3.3. Test 1: IPv4 <--> IPv4 local call
----------------------------------------
This test checks the backward compatibility with IPv4

In first shell/command window, listen on 127.0.0.1, wait for a call.
simple.exe -tttt -n -i 127.0.0.1 -l -a
In second shell/command window, listen on 10.0.0.6, call 127.0.0.1
simple.exe -tttt -n -i  10.0.0.6 -n 127.0.0.1



6.3.4. Test 2: IPv6 <--> IPv6 local call 
----------------------------------------
This test checks the IPv6 support

In first shell/command window, listen on ::1, wait for a call.
simple.exe -tttt -n -i ::1 -l -a
In second shell/command window, listen on IPv6 address, call ::1
simple.exe -tttt -n -i 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5 -n [::1]


6.3.5. Test 3: IPv4 <--> IPv6 local call
----------------------------------------
This test checks that simultaneous IPv4 and IPv6 calls are supported.

In first shell/command window, listen on 127.0.0.1, wait for a call.
simple.exe -tttt -n -i 127.0.0.1 -l -a
In second shell/command window, listen on IPv6 address, call 127.0.0.1
simple.exe -tttt -n -i 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5 -n 127.0.0.1



6.3.6. Test 4: IPv4 <--> IPv4 call between two hosts
----------------------------------------------------
This test checks the backward compatibility with IPv4

First host, listen on 10.0.0.6, wait for a call.
simple.exe -tttt -n -i 127.0.0.1 -l -a
Second host, listen on 10.0.0.8, call 10.0.0.6
simple.exe -tttt -n -i  10.0.0.8 -n 10.0.0.6



6.3.7. Test 5: IPv6 <--> IPv6 call between two hosts
----------------------------------------------------
This test checks the IPv6 support

First host, listen on 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5, wait for a call.
simple.exe -tttt -n -i 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5 -l -a
Second host, listen on 3ffe:0b80:0002:f9c1:0000:0000:500b:0eb6, call 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5
simple.exe -tttt -n -i 3ffe:0b80:0002:f9c1:0000:0000:500b:0eb6 -n [3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5]



6.3.8. Test 6: IPv4 <--> IPv6 call between two hosts
----------------------------------------------------
This test checks that simultaneous IPv4 and IPv6 calls are supported.

First host, listen on 10.0.0.6, wait for a call.
simple.exe -tttt -n -i 10.0.0.6 -l -a
Second host, listen on 3ffe:0b80:0002:f9c1:0000:0000:500b:0eb6, call 10.0.0.6
simple.exe -tttt -n -i 3ffe:0b80:0002:f9c1:0000:0000:500b:0eb6 -n 10.0.0.6



--------------------------------------------------------------------------------
6.4. Known limitations
--------------------

You must use IPv6 address with global scope. Tests with IPv6 local link address
fail.



--------------------------------------------------------------------------------
6.5. Questions
--------------

6.5.1. Removed
--------------

6.5.2. Removed
--------------

6.5.3. How to get an ipv6 address with a Global scope ?
-----------------------------------------------------

6.5.3.1. Manually
-----------------

Set one manually if you're not connected to IPv4 Internet or IPv6 backbone:
#ip -6 addr add 3ffe:0b80:0002:f9c1:0000:0000:500b:0ea5 dev eth0
(this address is owned by freenet6.net).

Check the address is set.
#ifconfig
eth0      Lien encap:Ethernet  HWaddr 00:08:D5:10:C7:BB
          inet adr:12.0.0.2  Bcast:12.255.255.255  Masque:255.0.0.0
          adr inet6: 3ffe:b80:2:f9c1::500b:ea5/128 Scope:Global  <- - - Ok, Global scope
          adr inet6: fe80::208:c7ff:fe59:bbc7/10 Scope:Lien <- - - [ Can't use this one ]
          UP BROADCAST RUNNING MULTICAST  MTU:1500  Metric:1
          RX packets:0 errors:0 dropped:0 overruns:0 frame:0
          TX packets:9 errors:0 dropped:0 overruns:9 carrier:0
          collisions:0
          RX bytes:0 (0.0 b)  TX bytes:534 (534.0 b)


6.5.3.2. Tunnel broker
----------------------

Get one from a free IPv6 tunnel broker.
Exemple: 
http://www.freenet6.net : Canadian tunnel broker
http://tb.ngnet.it      : Italian tunnel broker (Telecom Italia Research)


Note: The current (10/2002) freenet6 windows binary is buggy, use it to get the 
values, and set manually your tunnel.



--------------------------------------------------------------------------------
6.6. Troubles
------------

6.6.1. Listen on ::1:1720 failed: Address family not supported by protocol
-----------------------------------------------------------------------
IPv6 module is not loaded in the kernel.
#modprobe ipv6



6.6.2. SimpleOpal	TCP Could not open H.323 listener port on 1720
--------------------------------------------------------------
Add some traces: -t on the command line. 



6.6.3. SimpleOpal	TCP Listen on fe80::2b0:d0ff:fedf:d6bf:1720 failed: Invalid argument
------------------------------------------------------------------------------------
This address is a local scope address. As the scope_id field is always set to 0,
its value is invalid.

Use address with global scope.




================================================================================

7. Platform Specific Issues
---------------------------

See the Wiki at http://www.opalvoip.org/wiki/index.php?n=Main.HomePage

================================================================================

8. Conclusion
-------------

This package is far from a "product". There is very limited documentation and
support will be on an ad-hoc basis, send us an e-mail and we will probably
answer your question if it isn't too difficult.

It is supplied mainly to support the Opal project, but that shouldn't stop
you from using it in whatever project you have in mind if you so desire. We like
it and use it all the time, and we don't want to get into any religious wars of
this class library over that one.




================================================================================

9. Licensing                 
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


The in-band DTMF decoding code was taken from FreeBSD's dtmfdecode.c
application written by Poul-Henning Kamp. It has the following
license:
 * ----------------------------------------------------------------------------
 * "THE BEER-WARE LICENSE" (Revision 42):
 * <phk@FreeBSD.org> wrote this file.  As long as you retain this notice you
 * can do whatever you want with this stuff. If we meet some day, and you think
 * this stuff is worth it, you can buy me a beer in return.   Poul-Henning Kamp
 * ----------------------------------------------------------------------------



================================================================================
