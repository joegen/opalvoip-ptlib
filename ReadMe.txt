			Portable Windows Libary
			=======================

Introduction
------------

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

Apologies (not)
---------------

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


Building PWLib
--------------

For Windows.

1.	Start MSVC v5. MSVC v6 should work though we have not confirmed this. If
	you have another compiler you are on your own!
	Go into the Tools menu, Options item, Directories tab and add to the
	Include files path:
		C:\PWLib\Include\PtLib\MSOS
		C:\PWLib\Include
	and add to the Lib Files path and the Executable Files path the
	following:
		C:\PWLib\Lib
2.	Open the pwlib.dsw file in the pwlib top directory. If you have the
	minimum library it will come up with several requests to find .dsp
	files, just cancel past these.
3.	If you have the NT or 95 DDK, find the ndis.h file and put it into the
	pwlib/include/ptlib/msos directory. If you do not, then delete the files
	ethsock.cxx and pethsock.cxx from the Console project.
4.	Build the target MergeSym, in release mode.
5.	Move the MergeSym.exe file to somewhere in your path, eg c:\pwlib\lib
6.	Build the target ASNParser, in release mode. Note you will need bison
	and flex to do this.
7.	Move the ASNParser.exe file to somewhere in your path, eg c:\pwlib\lib
8.	Build the pwtest target, if you have the full distribution.
9.	You are now on your own!


For unix.

1.	If you have not put pwlib it into your home directory (~/pwlib) then
	you will have to defined the environment variable PWLIBDIR to point to
	the correct directory.
2.	Build the debug and release versions of the PWLib library as follows:
		cd ~/pwlib
		make both
	This may take some time. Note, you will need bison and flex for this to
	compile.

	If you are getting errors during the compile, then it is likely your
	platform is not supported, or you have incorrectly set the OSTYPE and
	MACHTYPE variables.

3.	You are now on your own!



Using PWLib
-----------

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
pre-emptive on platforms that support it (Win32, pthreads) and cooperative on
those that don't (eg Linux at the time of writing).

GUI

There are a very large number of classes to support a GUI interface. This is not
very complete at this time. The Win32 implementation is quite usable, though it
doesn't include the latest and greatest out of Redmond. The pure xlib
implementation has quite a lot implemented but is by no means complete. A motif
implementation is in the works but has not progressed very far.


Conclusion
----------

This package is far from a "product". There is very limited documentation and
support will be on an ad-hoc basis, send us an e-mail and we will probably
answer your question if it isn't too difficult.

It is supplied mainly to support the open H323 project, but that shouldn't stop
you from using it in whatever project you have in mind if you so desire. We like
it and use it all the time, and we don't want to get into any religious wars of
this class library over that one.


================================================================================
Equivalence Pty. Ltd.
Home of FireDoor, MibMaster & PhonePatch

support@equival.com.au
http://www.equival.com.au (US Mirror - http://www.equival.com)
Fax: +61 2 4368 1395   Voice: +61 2 4368 2118
