/*
 * object.h
 *
 * Mother of all ancestor classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: object.h,v $
 * Revision 1.42  1999/03/09 02:59:50  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.41  1999/02/23 07:11:26  robertj
 * Improved trace facility adding trace levels and #define to remove all trace code.
 *
 * Revision 1.40  1999/02/22 10:48:14  robertj
 * Fixed delete operator prototypes for MSVC6 and GNU compatibility.
 *
 * Revision 1.39  1999/02/19 11:33:02  robertj
 * Fixed compatibility problems with GNU/MSVC6
 *
 * Revision 1.38  1999/02/16 08:12:22  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.37  1999/01/07 03:35:35  robertj
 * Added default for PCHAR8 to ANSI, removes need for compiler option.
 *
 * Revision 1.36  1998/12/15 09:00:29  robertj
 * Fixed 8 byte alignment problem in memory leak check code for sparc.
 *
 * Revision 1.35  1998/11/03 00:57:19  robertj
 * Added allocation breakpoint variable.
 *
 * Revision 1.34  1998/10/26 11:05:26  robertj
 * Added raw free for things allocated within the runtime library.
 *
 * Revision 1.33  1998/10/18 14:26:55  robertj
 * Improved tracing functions.
 *
 * Revision 1.32  1998/10/15 07:47:21  robertj
 * Added ability to ignore G++lib memory leaks.
 *
 * Revision 1.31  1998/10/15 01:53:58  robertj
 * GNU compatibility.
 *
 * Revision 1.30  1998/10/13 14:23:29  robertj
 * Complete rewrite of memory leak detection.
 *
 * Revision 1.29  1998/09/23 06:20:57  robertj
 * Added open source copyright license.
 *
 * Revision 1.28  1998/09/14 12:29:11  robertj
 * Fixed memory leak dump under windows to not include static globals.
 * Fixed problem with notifier declaration not allowing implementation inline after macro.
 *
 * Revision 1.27  1997/07/08 13:13:45  robertj
 * DLL support.
 *
 * Revision 1.26  1997/04/27 05:50:11  robertj
 * DLL support.
 *
 * Revision 1.25  1997/02/05 11:54:10  robertj
 * Fixed problems with memory check and leak detection.
 *
 * Revision 1.24  1996/09/16 12:57:23  robertj
 * DLL support
 *
 * Revision 1.23  1996/08/17 10:00:23  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.22  1996/07/15 10:27:51  robertj
 * Changed endian classes to be memory mapped.
 *
 * Revision 1.21  1996/05/09 12:14:48  robertj
 * Fixed up 64 bit integer class for Mac platform.
 *
 * Revision 1.20  1996/02/24 14:19:29  robertj
 * Fixed bug in endian independent integer code for memory transfers.
 *
 * Revision 1.19  1996/01/28 02:46:43  robertj
 * Removal of MemoryPointer classes as usage didn't work for GNU.
 * Added missing bit shift operators to 64 bit integer class.
 *
 * Revision 1.18  1996/01/23 13:14:32  robertj
 * Added const version of PMemoryPointer.
 * Added constructor to endian classes for the base type.
 *
 * Revision 1.17  1996/01/02 11:54:11  robertj
 * Mac OS compatibility changes.
 *
 * Revision 1.16  1995/11/09 12:17:10  robertj
 * Added platform independent base type access classes.
 *
 * Revision 1.15  1995/06/17 11:12:47  robertj
 * Documentation update.
 *
 * Revision 1.14  1995/06/04 12:34:19  robertj
 * Added trace functions.
 *
 * Revision 1.13  1995/04/25 12:04:35  robertj
 * Fixed borland compatibility.
 * Fixed function hiding ancestor virtuals.
 *
 * Revision 1.12  1995/03/14 12:41:54  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.11  1995/03/12  04:40:55  robertj
 * Changed standard error code for not open from file to channel.
 *
 * Revision 1.10  1995/02/19  04:19:14  robertj
 * Added dynamically linked command processing.
 *
 * Revision 1.9  1995/02/05  00:48:07  robertj
 * Fixed template version.
 *
 * Revision 1.8  1995/01/15  04:51:31  robertj
 * Mac compatibility.
 * Added levels of memory checking.
 *
 * Revision 1.7  1995/01/09  12:38:31  robertj
 * Changed variable names around during documentation run.
 * Fixed smart pointer comparison.
 * Fixed serialisation stuff.
 * Documentation.
 *
 * Revision 1.6  1995/01/03  09:39:06  robertj
 * Put standard malloc style memory allocation etc into memory check system.
 *
 * Revision 1.5  1994/12/12  10:08:30  robertj
 * Renamed PWrapper to PSmartPointer..
 *
 * Revision 1.4  1994/12/05  11:23:28  robertj
 * Fixed PWrapper macros.
 *
 * Revision 1.3  1994/11/19  00:22:55  robertj
 * Changed PInteger to be INT, ie standard type like BOOL/WORD etc.
 * Moved null object check in notifier to construction rather than use.
 * Added virtual to the callback function in notifier destination class.
 *
 * Revision 1.2  1994/11/03  09:25:30  robertj
 * Made notifier destination object not to be descendent of PObject.
 *
 * Revision 1.1  1994/10/30  12:01:37  robertj
 * Initial revision
 *
 */

#ifdef __GNUC__
#pragma interface
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <iostream.h>
#ifndef __MWERKS__
#include <iomanip.h>
#endif



///////////////////////////////////////////////////////////////////////////////
// Disable inlines when debugging for faster compiles (the compiler doesn't
// actually inline the function with debug on any way).

#ifdef P_USE_INLINES
#define PINLINE inline
#else
#define PINLINE
#endif


///////////////////////////////////////////////////////////////////////////////
// Declare the debugging support

/// Standard assert messages for the PAssert macro.
enum PStandardAssertMessage {
  PLogicError,              // A logic error occurred.
  POutOfMemory,             // A new or malloc failed.
  PNullPointerReference,    // A reference was made through a NULL pointer.
  PInvalidCast,             // An invalid cast to descendant is required.
  PInvalidArrayIndex,       // An index into an array was negative.
  PInvalidArrayElement,     // A NULL array element object was accessed.
  PStackEmpty,              // A Pop() was made of a stack with no elements.
  PUnimplementedFunction,   // Funtion is not implemented.
  PInvalidParameter,        // Invalid parameter was passed to a function.
  POperatingSystemError,    // Error was returned by Operating System.
  PChannelNotOpen,          // Operation attempted when channel not open.
  PUnsupportedFeature,      // Feature is not supported.
  PInvalidWindow,           // Access through invalid window.
  PMaxStandardAssertMessage
};

/** This macro is used to assert that a condition must be TRUE.
If the condition is FALSE then an assert function is called with the source
file and line number the macro was instantiated on, plus the message described
by the #msg# parameter. This parameter may be either a standard value
from the #PStandardAssertMessage# enum or a literal string.
*/
#define PAssert(b, m) if(b);else PAssertFunc(__FILE__, __LINE__, (m))

/** This macro is used to assert that an operating system call succeeds.
If the condition is FALSE then an assert function is called with the source
file and line number the macro was instantiated on, plus the message
described by the #POperatingSystemError# value in the #PStandardAssertMessage#
enum.
 */
#define PAssertOS(b) \
              if(b);else PAssertFunc(__FILE__, __LINE__, POperatingSystemError)

/** This macro is used to assert that a pointer must be non-null.
If the pointer is NULL then an assert function is called with the source file
and line number the macro was instantiated on, plus the message described by
the PNullPointerReference value in the #PStandardAssertMessage# enum.

Note that this evaluates the expression defined by #ptr# twice. To
prevent incorrect behaviour with this, the macro will assume that the
#ptr# parameter is an L-Value.
 */
#define PAssertNULL(p) ((&(p)&&(p)!=NULL)?(p):(PAssertFunc(__FILE__, \
                                        __LINE__, PNullPointerReference), (p)))

/** This macro is used to assert immediately.
The assert function is called with the source file and line number the macro
was instantiated on, plus the message described by the #msg# parameter. This
parameter may be either a standard value from the #PStandardAssertMessage#
enum or a literal string.
*/
#define PAssertAlways(m) PAssertFunc(__FILE__, __LINE__, (m))


void PAssertFunc(const char * file, int line, PStandardAssertMessage msg);
void PAssertFunc(const char * file, int line, const char * msg);


/** Get the stream being used for error output.
This stream is used for all trace output using the various trace functions
and macros.
*/
ostream & PGetErrorStream();

/** Set the stream to be used for error output.
This stream is used for all error output using the #PError# macro.
*/
void PSetErrorStream(ostream * strm /** New stream for error output */ );

/** This macro is used to access the platform specific error output stream.
This is to be used in preference to assuming #cerr# is always available. On
Unix platforms this {\bfis} #cerr# but for MS-Windows this is another stream
that uses the OutputDebugString() Windows API function. Note that a MS-DOS or
Windows NT console application would still use #cerr#.

The #PError# stream would normally only be used for debugging information as
a suitable display is not always available in windowed environments.
   
The macro is a wrapper for a global variable #PErrorStream# which is a pointer
to an #ostream#. The variable is initialised to #cerr# for all but MS-Windows
and NT GUI applications. An application could change this pointer to a
#ofstream# variable of #PError# output is wished to be redirected to a file.
*/
#define PError (PGetErrorStream())



///////////////////////////////////////////////////////////////////////////////
// Debug and tracing

#if !defined(PTRACING) && defined(_DEBUG)
#define PTRACING
#endif

/** Set the stream to be used for trace output.
This stream is used for all trace output using the various trace functions
and macros.
*/
void PSetTraceStream(ostream * out /** New output stream from trace. */ );

/** Set the PTraceBlock output flag.
Set the internal flag for PTraceBlock classes to output to the trace stream.
If this is FALSE, all PTraceBlock output is inhibited regardless of the trace
level. If TRUE the PTraceBlock may occur provided the trace level is greater
than zero.
*/
void PSetTraceBlock(BOOL enable /** Flag for enabling all PTraceBlock output. */ );

/** Set the trace level.
The PTRACE() macro checks to see if its level is equal to or lower then the
level set by this function. If so then the trace text is output to the trace
stream.
*/
void PSetTraceLevel(unsigned level /** New level for trace */ );

/** Determine if the level may cause trace output.
This checks against the current global trace level set by #PSetTraceLevel#
for if the trace output may be emitted. This is used by the PTRACE macro.
*/
BOOL PCanTrace(unsigned level /** New level */);

/** Begin a trace output.
If the trace stream output is used outside of the provided macros, it
should be noted that a mutex is obtained on the call to #PBeginTrace# which
will prevent any other threads from using the trace stream until the
#PEndTrace# function is called.

So a typical usage would be:
\begin{verbatim}
  ostream & s = PBegintrace();
  s << "hello";
  if (want_there)
    s << " there";
  s << '!' << PEndTrace();
\end{verbatim}
*/
ostream & PBeginTrace(
  const char * fileName,  /// Filename of source file being traced
  int lineNum             /// Line number of source file being traced.
);

/** End a trace output.
If the trace stream output is used outside of the provided macros, the
#PEndTrace# function must be used at the end of the section of trace
output. A mutex is obtained on the call to #PBeginTrace# which will prevent
any other threads from using the trace stream until the PEndTrace. The
#PEndTrace# is used in a similar manner to #::endl# or #::flush#.

So a typical usage would be:
\begin{verbatim}
  ostream & s = PBegintrace();
  s << "hello";
  if (want_there)
    s << " there";
  s << '!' << PEndTrace();
\end{verbatim}
*/
ostream & PEndTrace(ostream & strm /** Trace output stream being completed */);


/** Class to trace Execution blocks.
This class is used for tracing the entry and exit of program blocks. Upon
construction it outputs an entry trace message and on destruction outputs an
exit trace message. This is normally only used from in the PTRACE_BLOCK macro.
*/
class PTraceBlock {
  public:
    /** Output entry trace message. */
    PTraceBlock(
      const char * fileName, /// Filename of source file being traced
      int lineNum,           /// Line number of source file being traced.
      const char * traceName
        /// String to be output with trace, typically it is the function name.
     );
    /// Output exit trace message.
    ~PTraceBlock();
  private:
    const char * file;
    int          line;
    const char * name;
};


#ifndef PTRACING

#define PTRACE_BLOCK(n)
#define PTRACE_LINE()
#define PTRACE(level, arg)

#else

/** Trace an execution block.
This macro creates a trace variable for tracking the entry and exit of program
blocks. It creates an instance of the PTraceBlock class that will output a
trace message at the line PTRACE_BLOCK is called and then on exit from the
scope it is defined in.
*/
#define PTRACE_BLOCK(name) PTraceBlock __trace_block_instance(__FILE__, __LINE__, name)

/** Trace the execution of a line.
This macro outputs a trace of a source file line execution.
*/
#define PTRACE_LINE() \
    if (!PCanTrace(1)) ; else PBeginTrace(__FILE__, __LINE__) << PEndTrace

/** Output trace.
This macro outputs a trace of any information needed, using standard stream
output operators. The output is only made if the trace level set by the
#PSetTraceLevel# function is greater than or equal to the #level# argument.
*/
#define PTRACE(level, args) \
    if (!PCanTrace(level)) ; else PBeginTrace(__FILE__, __LINE__) << args << PEndTrace

#endif


#ifdef PMEMORY_CHECK

/** Memory heap chacking class.
This class implements the memory heap checking and validation functions. It
maintains lists of allocated block so that memory leaks can be detected. It
also initialises memory on allocation and deallocation to help catch errors
involving the use of dangling pointers.
*/
class PMemoryHeap {
  protected:
    /// Initialise the memory checking subsystem.
    PMemoryHeap();

  public:
    // Clear up the memory checking subsystem, dumping memory leaks.
    ~PMemoryHeap();

    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the #allocFillChar# member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t nSize,           /// Number of bytes to allocate.
      const char * file,      /// Source file name for allocating function.
      int line,               /// Source file line for allocating function.
      const char * className  /// Class name for allocating function.
    );
    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the #allocFillChar# member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t count,       /// Number of items to allocate.
      size_t iSize,       /// Size in bytes of each item.
      const char * file,  /// Source file name for allocating function.
      int line            /// Source file line for allocating function.
    );

    /** Change the size of an allocated memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the #allocFillChar# member variable
       to help detect uninitialised structures.
      @return pointer to reallocated memory block. Note this may
      {\em not} be the same as the pointer passed into the function.
     */
    static void * Reallocate(
      void * ptr,         /// Pointer to memory block to reallocate.
      size_t nSize,       /// New number of bytes to allocate.
      const char * file,  /// Source file name for allocating function.
      int line            /// Source file line for allocating function.
    );

    /** Free a memory block.
      The memory is deallocated, a warning is displayed if it was never
      allocated. The block of memory is filled with the value in the
      #freeFillChar# member variable.
     */
    static void Deallocate(
      void * ptr,             /// Pointer to memory block to deallocate.
      const char * className  /// Class name for deallocating function.
    );

    /** Validation result.
     */
    enum Validation {
      Ok, Bad, Trashed
    };
    /** Validate the memory pointer.
        The #ptr# parameter is validated as a currently allocated heap
        variable.
        @return Ok for pointer is in heap, Bad for pointer is not in the heap
        or Trashed if the pointer is in the heap but has overwritten the guard
        bytes before or after the actual data part of the memory block.
     */
    static Validation Validate(
      void * ptr,             /// Pointer to memory block to check
      const char * className, /// Class name it should be.
      ostream * error         /// Stream to receive error message (may be NULL)
    );

    /** Ignore/Monitor allocations.
       Set internal flag so that allocations are not included in the memory
       leak check on program termination.
     */
    static void SetIgnoreAllocations(
      BOOL ignore  /// New flag for allocation ignoring.
    );

    /** Get memory check system statistics.
        Dump statistics output to the default stream.
     */
    static void DumpStatistics();
    /** Get memory check system statistics.
        Dump statistics output to the specified stream.
     */
    static void DumpStatistics(ostream & strm /** Stream to output to */);

    /* Get number of allocation.
      Each allocation is counted and if desired the next allocation request
      number may be obtained via this function.
      @return Allocation request number.
     */
    static DWORD GetAllocationRequest();

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the #GetAllocationRequest()#
       function.

       Output is to the default stream.
     */
    static void DumpObjectsSince(
      DWORD objectNumber    /// Memory object to begin dump from.
    );

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the #GetAllocationRequest()#
       function.
     */
    static void DumpObjectsSince(
      DWORD objectNumber,   /// Memory object to begin dump from.
      ostream & strm        /// Stream to output dump
    );

    /** Set break point allocation number.
      Set the allocation request number to cause an assert. This allows a
      developer to cause a halt in a debugger on a certain allocation allowing
      them to determine memory leaks allocation point.
     */
    static void SetAllocationBreakpoint(
      DWORD point   /// Allocation number to stop at.
    );

  protected:
    void * InternalAllocate(
      size_t nSize,           // Number of bytes to allocate.
      const char * file,      // Source file name for allocating function.
      int line,               // Source file line for allocating function.
      const char * className  // Class name for allocating function.
    );
    Validation InternalValidate(
      void * ptr,             // Pointer to memory block to check
      const char * className, // Class name it should be.
      ostream * error         // Stream to receive error message (may be NULL)
    );
    void InternalDumpStatistics(ostream & strm);
    void InternalDumpObjectsSince(DWORD objectNumber, ostream & strm);

    class Wrapper {
      public:
        Wrapper();
        ~Wrapper();
        PMemoryHeap * operator->() const { return instance; }
      private:
        PMemoryHeap * instance;
    };
    friend class Wrapper;

    enum Flags {
      NoLeakPrint = 1
    };

#pragma pack(1)
    struct Header {
      Header     * prev;
      Header     * next;
      size_t       size;
      DWORD        request;
      const char * className;
      const char * fileName;
      WORD         line;
      BYTE         flags;
      static const char GuardBytes[5];
      char         guard[sizeof(GuardBytes)];
    };
#pragma pack()

    Header * listHead;
    Header * listTail;

    static DWORD allocationBreakpoint;
    DWORD allocationRequest;
    DWORD firstRealObject;
    BYTE  flags;

    char  allocFillChar;
    char  freeFillChar;

    DWORD currentMemoryUsage;
    DWORD peakMemoryUsage;
    DWORD currentObjects;
    DWORD peakObjects;
    DWORD totalObjects;

    ostream * leakDumpStream;

#ifdef _WIN32
    CRITICAL_SECTION mutex;
#endif
};


/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the #PMEMORY_CHECK# compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define malloc(s) PMemoryHeap::Allocate(s, __FILE__, __LINE__, NULL)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the #PMEMORY_CHECK# compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define calloc(n,s) PMemoryHeap::Allocate(n, s, __FILE__, __LINE__)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the #PMEMORY_CHECK# compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define realloc(p,s) PMemoryHeap::Reallocate(p, s, __FILE__, __LINE__)


/** Free memory allocated by run time library.
This version of free is used for data that is not allocated using the
memory check system, ie was malloc'ed inside the C run time library.
*/
inline void runtime_free(void * ptr /** Memory block to free */ ) { free(ptr); }

/** Override of system call for memory check system.
This macro is used to deallocate memory via the memory check system selected
with the #PMEMORY_CHECK# compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define free(p) PMemoryHeap::Deallocate(p, NULL)


/** Macro for overriding system default #new# operator.
This macro is used to allocate memory via the memory check system selected
with the PMEMORY_CHECK compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.

This macro could be used instead of the system #new# operator. Or you can place
the line
\begin{verbatim}
  #define new PNEW
\end{verbatim}
at the begining of the source file, after all declarations that use the
PCLASSINFO macro.
*/
#define PNEW  new (__FILE__, __LINE__)

#ifdef __GNUC__
#define PSPECIAL_DELETE_FUNCTION
#else
#define PSPECIAL_DELETE_FUNCTION \
    void operator delete(void * ptr, const char *, int) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    void operator delete[](void * ptr, const char *, int) \
      { PMemoryHeap::Deallocate(ptr, Class()); }
#endif

#define PNEW_AND_DELETE_FUNCTIONS \
    void * operator new(size_t nSize, const char * file, int line) \
      { return PMemoryHeap::Allocate(nSize, file, line, Class()); } \
    void * operator new(size_t nSize) \
      { return PMemoryHeap::Allocate(nSize, NULL, 0, Class()); } \
    void operator delete(void * ptr) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    void * operator new[](size_t nSize, const char * file, int line) \
      { return PMemoryHeap::Allocate(nSize, file, line, Class()); } \
    void * operator new[](size_t nSize) \
      { return PMemoryHeap::Allocate(nSize, NULL, 0, Class()); } \
    void operator delete[](void * ptr) \
      { PMemoryHeap::Deallocate(ptr, Class()); } \
    PSPECIAL_DELETE_FUNCTION


inline void * operator new(size_t nSize, const char * file, int line)
  { return PMemoryHeap::Allocate(nSize, file, line, NULL); }

inline void * operator new[](size_t nSize, const char * file, int line)
  { return PMemoryHeap::Allocate(nSize, file, line, NULL); }

#ifndef __GNUC__
void operator delete(void * ptr);
void operator delete[](void * ptr);

inline void operator delete(void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }

inline void operator delete[](void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }
#endif


#else // _DEBUG

#define PNEW new
#define PNEW_AND_DELETE_FUNCTIONS
#define runtime_free(p) free(p)

#endif // _DEBUG


/** Declare all the standard PWlib class information.
This macro is used to provide the basic run-time typing capability needed
by the library. All descendent classes from the #PObject# class require
these functions for correct operation. Either use this macro or the
#PDECLARE_CLASS# macro.

The use of the #PDECLARE_CLASS# macro is no longer recommended for reasons
of compatibility with documentation systems.
*/
#define PCLASSINFO(cls, par) \
  public: \
    static const char * Class() \
      { return #cls; } \
    virtual const char * GetClass(unsigned ancestor = 0) const \
      { return ancestor > 0 ? par::GetClass(ancestor-1) : cls::Class(); } \
    virtual BOOL IsClass(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0; } \
    virtual BOOL IsDescendant(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0 || \
                                               par::IsDescendant(clsName); } \
    virtual Comparison CompareObjectMemoryDirect(const PObject & obj) const \
      { return (Comparison)memcmp(this, &obj, sizeof(cls)); } \
    PNEW_AND_DELETE_FUNCTIONS

/** Declare a class with PWLib class information.
This macro is used to declare a new class with a single public ancestor. It
starts the class declaration and then uses the #PCLASSINFO# macro to
get all the run-time type functions.

The use of this macro is no longer recommended for reasons of compatibility
with documentation systems.
*/
#define PDECLARE_CLASS(cls, par) class cls : public par { PCLASSINFO(cls, par)


class PSerialiser;
class PUnSerialiser;


///////////////////////////////////////////////////////////////////////////////
// The root of all evil ... umm classes

/** Ultimate parent class for all objects in the class library.
This provides functionality provided to all classes, eg run-time types,
default comparison operations, simple stream I/O and serialisation support.
*/
class PObject {

  protected:
    /** Constructor for PObject, make protected so cannot ever create one on
       its own.
     */
    PObject() { }

  public:
    /* Destructor required to get the "virtual". A PObject really has nothing
       to destroy.
     */
    virtual ~PObject() { }

  /**@name Run Time Type functions */
  //@{
    /** Get the name of the class as a C string. This is a static function which
       returns the type of a specific class. It is primarily used as an
       argument to the #IsClass()# or #IsDescendant()# functions.
       
       When comparing class names, always use the #strcmp()#
       function rather than comparing pointers. The pointers are not
       necessarily the same over compilation units depending on the compiler,
       platform etc.

       The #PCLASSINFO# macro declares a version of this function for the
       particular class.

       @return pointer to C string literal.
     */      
    static const char * Class() { return "PObject"; }

    /** Get the current dynamic type of the object instance.

       When comparing class names, always use the #strcmp()#
       function rather than comparing pointers. The pointers are not
       necessarily the same over compilation units depending on the compiler,
       platform etc.

       The #PCLASSINFO# macro declares an override of this function for
       the particular class. The user need not implement it.

       @return pointer to C string literal.
     */
    virtual const char * GetClass(
      unsigned ancestor = 0
      /** Level of ancestor to get the class name for. A value of zero is the
         instances class name, one is its ancestor, two for the ancestors
         ancestor etc.
       */
    ) const;

    /** Determine if the dynamic type of the current instance is of the
       specified class. The class name is usually provided by the
       #Class()# static function of the desired class.
    
       The #PCLASSINFO# macro declares an override of this function for
       the particular class. The user need not implement it.

       @return TRUE if object is of the class.
     */
    virtual BOOL IsClass(
      const char * clsName    // Class name to compare against.
    ) const;

    /** Determine if the dynamic type of the current instance is a descendent of
       the specified class. The class name is usually provided by the
       #Class()# static function of the desired class.
    
       The #PCLASSINFO# macro declares an override of this function for
       the particular class. The user need not implement it.

       @return TRUE if object is descended from the class.
     */
    virtual BOOL IsDescendant(
      const char * clsName    // Ancestor class name to compare against.
    ) const;
  //@}

  /**@name Comparison functions */
  //@{
    /** Result of the comparison operation performed by the #Compare()#
       function.
      */
    enum Comparison {
      LessThan = -1,
      EqualTo = 0,
      GreaterThan = 1
    };

    /** Compare the two objects and return their relative rank. This function is
       usually overridden by descendent classes to yield the ranking according
       to the semantics of the object.
       
       The default function is to use the #CompareObjectMemoryDirect()#
       function to do a byte wise memory comparison of the two objects.

       @return
       #LessThan#, #EqualTo# or #GreaterThan#
       according to the relative rank of the objects.
     */
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    
    /** Determine the byte wise comparison of two objects. This is the default
       comparison operation for objects that do not explicitly override the
       #Compare()# function.
    
       The #PCLASSINFO# macro declares an override of this function for
       the particular class. The user need not implement it.

       @return
       #LessThan#, #EqualTo# or #GreaterThan#
       according to the result #memcpy()# function.
     */
    virtual Comparison CompareObjectMemoryDirect(
      const PObject & obj   // Object to compare against.
    ) const;

    /** Compare the two objects.
    
       @return
       TRUE if objects are equal.
     */
    BOOL operator==(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == EqualTo; }

    /** Compare the two objects.
    
       @return
       TRUE if objects are not equal.
     */
    BOOL operator!=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != EqualTo; }

    /** Compare the two objects.
    
       @return
       TRUE if objects are less than.
     */
    BOOL operator<(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == LessThan; }

    /** Compare the two objects.
    
       @return
       TRUE if objects are greater than.
     */
    BOOL operator>(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == GreaterThan; }

    /** Compare the two objects.
    
       @return
       TRUE if objects are less than or equal.
     */
    BOOL operator<=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != GreaterThan; }

    /** Compare the two objects.
    
       @return
       TRUE if objects are greater than or equal.
     */
    BOOL operator>=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != LessThan; }
  //@}

  /**@name I/O functions */
  //@{
    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard #operator<<# function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;

    /** Input the contents of the object from the stream. The exact input is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard #operator>># function.

       The default behaviour is to do nothing.
     */
    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );


    /** Global function for using the standard << operator on objects descended
       from PObject. This simply calls the objects #PrintOn()# function.
       
       @return the #strm# parameter.
     */
    inline friend ostream & operator<<(
      ostream &strm,       // Stream to print the object into.
      const PObject & obj  // Object to print to the stream.
    ) { obj.PrintOn(strm); return strm; }

    /** Global function for using the standard >> operator on objects descended
       from PObject. This simply calls the objects #ReadFrom()# function.

       @return the #strm# parameter.
     */
    inline friend istream & operator>>(
      istream &strm,   // Stream to read the objects contents from.
      PObject & obj    // Object to read inormation into.
    ) { obj.ReadFrom(strm); return strm; }


    /** This function is used to determine the size of the object and all other
       objects it contains. The actual size is dependent on the exact semantics
       of the descendent object. For example the #PString# class would
       return the length of the string plus one, while the #PList# class
       would return the sum of the sizes of all of the objects in the list
       plus the size of an integer for the number of objects.

       This in only required by the #PBinarySerialiser# class which
       serialises the objects into a binary file. The #PTextSerialiser#
       class which serialises into a text stream does not use this function.

       Note serialisation requires the use of the #PDECLARE_SERIAL# and
       #PIMPLEMENT_SERIAL# macros.

       @return size in bytes of object.
     */
    virtual PINDEX PreSerialise(
      PSerialiser & strm   // Serialiser stream to serialise object into.
    );

    /** Serialise the object into the specified stream. This is similar to the
       #PrintOn()# function that outputs the contents of the object to a
       stream, but where #PrintOn()# usually produces a human readable
       form of the object, this function outputs enough data so that it can be
       reconstructed by the #PUnSerialiser# class.
       
       When the user implements this function they will usually be doing it for
       one of either the text of binary output versions. In some circumstances,
       eg libraries, both need be supported so the #IsDscendent()#
       function should be used on the #strm# parameter to determine
       whether it is a #PBinarySerialiser# class or a
       #PTextSerialiser# class and do the appropriate output.

       To a large extent, if only the #<<# operator is used on the
       #PSerialiser# instance, the text and binary version can be made
       identical.
     */
    virtual void Serialise(
      PSerialiser & strm   // Serialiser stream to serialise object into.
    );

    /** Un-serialise the object from the specified stream. This is similar to
       the #ReadFrom()# function that inputs the contents of the object
       from a stream, but where #ReadFrom()# usually intrerprets a human
       readable form of the object, this function inputs enough data so that
       it can be reconstructed from the data provided by the #Serialise()#
       function.

       When the user implements this function they will usually be doing it for
       one of either the text of binary input versions. In some circumstances,
       eg libraries, both need be supported so the #IsDscendent()#
       function should be used on the #strm# parameter to determine
       whether it is a #PBinarySerialiser# class or a
       #PTextSerialiser# class and do the appropriate input.

       To a large extent, if only the >> operator is used on the
       #PUnSerialiser# instance, the text and binary version can be made
       identical.
     */
    virtual void UnSerialise(
      PUnSerialiser & strm   // Serialiser stream to serialise object into.
    );
  //@}

  /**@name Miscellaneous functions */
  //@{
    /** Create a copy of the class on the heap. The exact semantics of the
       descendent class determine what is required to make a duplicate of the
       instance. Not all classes can even {\bf do} a clone operation.
       
       The main user of the clone function is the #PDictionary# class as
       it requires copies of the dictionary keys.

       The default behaviour is for this function to assert.

       @return
       pointer to new copy of the class instance.
     */
    virtual PObject * Clone() const;

    /** This function yields a hash value required by the #PDictionary#
       class. A descendent class that is required to be the key of a dictionary
       should override this function. The precise values returned is dependent
       on the semantics of the class. For example, the #PString# class
       overrides it to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       @return
       hash function value for class instance.
     */
    virtual PINDEX HashFunction() const;
  //@}
};



///////////////////////////////////////////////////////////////////////////////
// Serialisation

class PUnSerialiser;

/** Registration class for persistent object serialisation/unserialisation.
This class is for registration of object classes that will be serialised and
un-serialised.

As objects are un-serialised, the objects need to be constructed. For the
#PUnSerialiser# instance to know what constructor to call, a
registration of functions that call the appropriate constructor.

The #PDECLARE_SERIAL# macro creates a single instance of this class to
register the class with the serialiser.

Even though this class implements a hash table it does {\bf not} use the
standard #PHashTable# or #PDictionary# classes due to recursive
definition problems. Those classes need to register themselves with this
class before they can be used!
*/
class PSerialRegistration {

  public:
    /** This type is a pointer to a function to create objects during the
       un-serialisation operation.
     */
    typedef PObject * (*CreatorFunction)(PUnSerialiser * serial);

    /** Create a serialiser class registration. This is unversally called by
       static member variables in the #PDECLARE_SERIAL# and
       #PIMPLEMENT_SERIAL# macros.
     */
    PSerialRegistration(
      const char * clsNam,    // Name of class to register.
      CreatorFunction func    // Constructor function for the class.
    );

    /** Get the creator function for the class name specified.

       @return function to construct objects.
     */
    static CreatorFunction GetCreator(
      const char * clsNam   // Name of class to get the creator function for.
    );

    // Constant for size of hash table.
    enum { HashTableSize = 41 };

  protected:
    /** Calculate the bucket for the hash table lookup.
      @return Has index for class.
     */
    static PINDEX HashFunction(
      const char * className    // Class name to calculate hash function for.
    );

    /// This serialiser registrations class
    const char * className;

    /** This serialiser registrations creator function - the function that will
       make a new object of the classes type and construct it with an instance
       of the #PSerialiser# class.
     */
    CreatorFunction creator;

    /// Pointer to next registration when a hash clash occurs.
    PSerialRegistration * clash;

    /// A static dictionary of class names to creator functions.
    static PSerialRegistration * creatorHashTable[HashTableSize];
};


/** This class allows the serialisation of objects to an output stream. This
   packages up objects so that they can be reconstructed by an instance of the
   #PUnSerialiser# class. The stream they are sent to can be any stream;
   file, string, pipe, socket etc.

   Serialisation can be done in two manners: binary or text. This depends on
   the serialiser instance that was constructed. Each objects
   #PObject::Serialise()# function is called and it is up to that
   function to output in binary or text.

   To a large extent, if only the << operator is used on the
   #PSerialser# instance, the text and binary versions of the
   #PObject::Serialise()# function can be made identical.

   This class is an abstract class and descendents of #PTextSerialiser# or
   #PBinarySerialiser# should be created.
 */
class PSerialiser : public PObject
{
  PCLASSINFO(PSerialiser, PObject);

  public:
    /// Construct a serialiser.
    PSerialiser(
      ostream & strm  // Stream to output serialisation to.
    );

    /// Output char to serial stream.
    virtual PSerialiser & operator<<(char) = 0;
    /// Output unsigned char to serial stream.
    virtual PSerialiser & operator<<(unsigned char) = 0;
    /// Output signed char to serial stream.
    virtual PSerialiser & operator<<(signed char) = 0;
    /// Output short to serial stream.
    virtual PSerialiser & operator<<(short) = 0;
    /// Output unsigned short to serial stream.
    virtual PSerialiser & operator<<(unsigned short) = 0;
    /// Output int to serial stream.
    virtual PSerialiser & operator<<(int) = 0;
    /// Output unsigned int to serial stream.
    virtual PSerialiser & operator<<(unsigned int) = 0;
    /// Output long to serial stream.
    virtual PSerialiser & operator<<(long) = 0;
    /// Output unsigned long to serial stream.
    virtual PSerialiser & operator<<(unsigned long) = 0;
    /// Output float to serial stream.
    virtual PSerialiser & operator<<(float) = 0;
    /// Output double to serial stream.
    virtual PSerialiser & operator<<(double) = 0;
    /// Output long double to serial stream.
    virtual PSerialiser & operator<<(long double) = 0;
    /// Output C string to serial stream.
    virtual PSerialiser & operator<<(const char *) = 0;
    /// Output C string to serial stream.
    virtual PSerialiser & operator<<(const unsigned char *) = 0;
    /// Output C string to serial stream.
    virtual PSerialiser & operator<<(const signed char *) = 0;
    /** Output the data to the serialiser object.
      When the operator is executed on a #PObject# descendent then that objects
      #PObject::Serialise()# function is called.
     */
    virtual PSerialiser & operator<<(PObject &);

  protected:
    /// Stream to output serial data to.
    ostream & stream;
};


/** This class allows the un-serialisation of objects from an input stream. This
   reconstruct objects that where packaged earlier by an instance of the
   #PSerialise# class. The stream they are received from can be any
   stream; file, string, pipe, socket etc.

   Serialisation can be done in two manners: binary or text. This depends on
   the serialiser instance that was constructed. Each objects
   #PObject::Serialise()# function is called and it is up to that
   function to output in binary or text.

   To a large extent, if only the #<<# operator is used on the
   #PSerialser# instance, the text and binary versions of the
   #PObject::Serialise()# function can be made identical.

   This class is an abstract class and descendents of #PTextSerialiser# or
   #PBinarySerialiser# should be created.
 */
class PUnSerialiser : public PObject
{
  PCLASSINFO(PUnSerialiser, PObject);

  public:
    /// Construct an un-serialiser.
    PUnSerialiser(
      istream & strm    // Stream to read the serialised objects from.
    );

    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(char &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(unsigned char &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(signed char &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(short &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(unsigned short &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(int &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(unsigned int &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(long &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(unsigned long &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(float &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(double &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(long double &) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(char *) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(unsigned char *) = 0;
    /// Input primitive from stream.
    virtual PUnSerialiser & operator>>(signed char *) = 0;
    /** Input the data from the un-serialiser object.
      When the operator is executed on a #PObject# descendent then
      that objects #PObject::UnSerialise()# function is called.
     */
    virtual PUnSerialiser & operator>>(PObject &) = 0;

  protected:
    /// Stream the read un-serialiser data from.
    istream & stream;
};


/**Declare information in a class for serialisation of objects.
   This macro is used to declare functions required by the serialisation
   system. It is used in conjunction with the #PIMPLEMENT_SERIAL# macro.

   This declares the #PObject::PreSerialise()# and
   #PObject::Serialise()# functions which must be imeplemented by the
   user. The un-serialisation and registration is declared and implemented by
   these two functions automatically.
 */
#define PSERIALINFO(cls) \
  public: \
    virtual PINDEX PreSerialise(PSerialiser & strm); \
    virtual void Serialise(PSerialiser & serial); \
    virtual void UnSerialise(PUnSerialiser & serial); \
  protected: \
    cls(PUnSerialiser & serial); \
    static cls * UnSerialiseNew(PUnSerialiser & serial); \
  private: \
    PINDEX serialisedLength; \
    static PSerialRegistration pRegisterSerial; \

/**
   This macro is used to implement functions required by the serialisation
   system. It is used in conjunction with the #PDECLARE_SERIAL# macro.
 */
#define PIMPLEMENT_SERIAL(cls) \
  cls * cls::UnSerialiseNew(PUnSerialiser & serial) \
    { return new cls(serial); } \
  PSerialRegistration cls::pRegisterSerial(cls::Class(), cls::UnSerialise); \


/** This serialiser class serialises each object using ASCII text. This gives
  the highest level of portability for streams and platforms at the expense
  if larger amounts of data.
 */
class PTextSerialiser : public PSerialiser
{
  PCLASSINFO(PTextSerialiser, PSerialiser);

  public:
    /// Create a text serialiser.
    PTextSerialiser(
      ostream & strm,   // Stream to serialise to.
      PObject & data    // First object to serialise.
    );

    /// Output primitive to stream.
    PSerialiser & operator<<(char);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned char);
    /// Output primitive to stream.
    PSerialiser & operator<<(signed char);
    /// Output primitive to stream.
    PSerialiser & operator<<(short);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned short);
    /// Output primitive to stream.
    PSerialiser & operator<<(int);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned int);
    /// Output primitive to stream.
    PSerialiser & operator<<(long);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned long);
    /// Output primitive to stream.
    PSerialiser & operator<<(float);
    /// Output primitive to stream.
    PSerialiser & operator<<(double);
    /// Output primitive to stream.
    PSerialiser & operator<<(long double);
    /// Output primitive to stream.
    PSerialiser & operator<<(const char *);
    /// Output primitive to stream.
    PSerialiser & operator<<(const unsigned char *);
    /// Output primitive to stream.
    PSerialiser & operator<<(const signed char *);
    /** Output the data to the serialiser object.
      When the operator is executed on a #PObject# descendent then that objects
      #PObject::Serialise()# function is called.
     */
    virtual PSerialiser & operator<<(PObject & obj);
};


class PSortedStringList;

/** This serialiser class serialises each object using binary data. This gives
   the highest level data density at the expense of some portability and
   possibly the speed of execution.
   
   This is because two passes through the objects is made, the first to
   determine the classes and sizes and the second to actually output the data.
   A table of classes must also be output to set the correspondence between
   the class codes used in the output and the class names that are required by
   the unserialiser to construct instances of those classes.
 */
class PBinarySerialiser : public PSerialiser
{
  PCLASSINFO(PBinarySerialiser, PSerialiser);

  public:
    /// Create a binary serialiser.
    PBinarySerialiser(
      ostream & strm,   // Stream to serialise to.
      PObject & data    // First object to serialise.
    );

    /// Destroy the serialiser and its class table.
    ~PBinarySerialiser();

    /// Output primitive to stream.
    PSerialiser & operator<<(char);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned char);
    /// Output primitive to stream.
    PSerialiser & operator<<(signed char);
    /// Output primitive to stream.
    PSerialiser & operator<<(short);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned short);
    /// Output primitive to stream.
    PSerialiser & operator<<(int);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned int);
    /// Output primitive to stream.
    PSerialiser & operator<<(long);
    /// Output primitive to stream.
    PSerialiser & operator<<(unsigned long);
    /// Output primitive to stream.
    PSerialiser & operator<<(float);
    /// Output primitive to stream.
    PSerialiser & operator<<(double);
    /// Output primitive to stream.
    PSerialiser & operator<<(long double);
    /// Output primitive to stream.
    PSerialiser & operator<<(const char *);
    /// Output primitive to stream.
    PSerialiser & operator<<(const unsigned char *);
    /// Output primitive to stream.
    PSerialiser & operator<<(const signed char *);
    /** Output the data to the serialiser object.
      When the operator is executed on a #PObject# descendent then that objects
      #PObject::Serialise()# function is called.
     */
    virtual PSerialiser & operator<<(PObject & obj);

  protected:
    /// List of classes used during serialisation.
    PSortedStringList * classesUsed;
};


/** This un-serialiser class reconstructs each object using ASCII text. This
   gives the highest level of portability for streams and platforms at the
   expense if larger amounts of data.
 */
class PTextUnSerialiser : public PUnSerialiser
{
  PCLASSINFO(PTextUnSerialiser, PUnSerialiser);

  public:
    /// Create a text un-serialiser.
    PTextUnSerialiser(
      istream & strm    // Stream to read serialised objects from.
    );

    /// Input primitive from stream.
    PUnSerialiser & operator>>(char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(signed char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(short &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned short &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(int &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned int &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(long &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned long &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(float &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(double &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(long double &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(char *);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned char *);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(signed char *);
    /** Input the data from the un-serialiser object.
      When the operator is executed on a #PObject# descendent then
      that objects #PObject::UnSerialise()# function is called.
     */
    PUnSerialiser & operator>>(PObject &);
};


class PStringArray;

/** This un-serialiser class reconstructs each object using binary data. This
   gives the highest level data density at the expense of some portability and
   possibly the speed of execution.
   
   A table of classes must also be output
   to set the correspondence between the class codes used in the output and
   the class names that are required by the unserialiser to construct instances
   of those classes.
 */
class PBinaryUnSerialiser : public PUnSerialiser
{
  PCLASSINFO(PBinaryUnSerialiser, PUnSerialiser);
  public:
    /// Create a binary un-serialiser.
    PBinaryUnSerialiser(
      istream & strm    // Stream to read serialised objects from.
    );

    /// Destroy the un-serialiser and its class table.
    ~PBinaryUnSerialiser();

    /// Input primitive from stream.
    PUnSerialiser & operator>>(char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(signed char &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(short &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned short &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(int &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned int &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(long &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned long &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(float &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(double &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(long double &);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(char *);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(unsigned char *);
    /// Input primitive from stream.
    PUnSerialiser & operator>>(signed char *);
    /** Input the data from the un-serialiser object.
      When the operator is executed on a #PObject# descendent then
      that objects #PObject::UnSerialise()# function is called.
     */
    PUnSerialiser & operator>>(PObject &);

  protected:
    /// Class table used by the serialiser.
    PStringArray * classesUsed;
};


///////////////////////////////////////////////////////////////////////////////
// "Smart" pointers.

/** This is the base class for objects that use the {\it smart pointer} system.
   In conjunction with the #PSmartPointer# class, this class creates
   objects that can have the automatic deletion of the object instance when
   there are no more smart pointer instances pointing to it.

   A #PSmartObject# carries the reference count that the #PSmartPointer# 
   requires to determine if the pointer is needed any more and should be
   deleted.
 */
class PSmartObject : public PObject
{
  PCLASSINFO(PSmartObject, PObject);

  public:
    /** Construct a new smart object, subject to a #PSmartPointer# instance
       referencing it.
     */
    PSmartObject() { referenceCount = 1; }

  protected:
    /** Count of number of instances of #PSmartPointer# that currently
       reference the object instance.
     */
    unsigned referenceCount;


  friend class PSmartPointer;
};


/** This is the class for pointers to objects that use the {\it smart pointer}
   system. In conjunction with the #PSmartObject# class, this class
   references objects that can have the automatic deletion of the object
   instance when there are no more smart pointer instances pointing to it.

   A PSmartPointer carries the pointer to a #PSmartObject# instance which
   contains a reference count. Assigning or copying instances of smart pointers
   will automatically increment and decrement the reference count. When the
   last instance that references a #PSmartObject# instance is destroyed or
   overwritten, the #PSmartObject# is deleted.

   A NULL value is possible for a smart pointer. It can be detected via the
   #IsNULL()# function.
 */
class PSmartPointer : public PObject
{
  PCLASSINFO(PSmartPointer, PObject);

  public:
  /**@name Construction */
  //@{
    /** Create a new smart pointer instance and have it point to the specified
       #PSmartObject# instance.
     */
    PSmartPointer(
      PSmartObject * obj = NULL   /// Smart object to point to.
    ) { object = obj; }

    /** Create a new smart pointer and point it at the data pointed to by the
       #ptr# parameter. The reference count for the object being
       pointed at is incremented.
     */
    PSmartPointer(
      const PSmartPointer & ptr  /// Smart pointer to make a copy of.
    );

    /** Destroy the smart pointer and decrement the reference count on the
       object being pointed to. If there are no more references then the
       object is deleted.
     */
    virtual ~PSmartPointer();

    /** Assign this pointer to the value specified in the #ptr#
       parameter.

       The previous object being pointed to has its reference count
       decremented as this will no longer point to it. If there are no more
       references then the object is deleted.

       The new object being pointed to after the assignment has its reference
       count incremented.
     */
    PSmartPointer & operator=(
      const PSmartPointer & ptr  /// Smart pointer to assign.
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Determine the relative rank of the pointers. This is identical to
       determining the relative rank of the integer values represented by the
       memory pointers.

       @return
       #EqualTo# if objects point to the same object instance,
       otherwise #LessThan# and #GreaterThan# may be
       returned depending on the relative values of the memory pointers.
     */
    virtual Comparison Compare(
      const PObject & obj   // Other smart pointer to compare against.
    ) const;
  //@}

  /**@name Pointer access functions */
  //@{
    /** Determine if the smart pointer has been set to point to an actual
       object instance.

       @return
       TRUE if the pointer is NULL.
     */
    BOOL IsNULL() const { return object == NULL; }

    /** Get the current value if the internal smart object pointer.

       @return
       pointer to object instance.
     */
    PSmartObject * GetObject() const { return object; }
  //@}

  protected:
    // Member variables
    /// Object the smart pointer points to.
    PSmartObject * object;
};


/** This macro is used to declare a smart pointer class.
The class is not closed off allowing customisation of the new class being
declared.

The class #cls# is declared as a smart pointer, descended from
the #par# class, to the #type# class.

If no customisations for the clase are required use the
#PDECLARE_POINTER_CLASS# macro instead.

The class declares the following functions:
\begin{verbatim}
      cls(type * obj);
        Constructor creating the smart pointer given the memory pointer.

      type * operator->() const;
        Access to the members of the smart object in the smart pointer.

      type & operator*() const;
        Access to the value of the smart object in the smart pointer.
\end{verbatim}

Note if this macro is used then the #PIMPLEMENT_POINTER# macro must be
used to implement some inline functions that the pointer class declares.
The separate declaration and definition is sometimes required due to the
order in which the #PSmartPointer# class and the #PSmartObject#
class are declared. They may require a circular reference under some
circumstances.
*/
#define PDECLARE_POINTER_CLASS(cls, par, type) \
  PDECLARE_CLASS(cls, par) \
    public: \
      cls(type * obj); \
      type * operator->() const; \
      type & operator*() const; \


/** This macro is used to declare a smart pointer.
Unlike the #PDECLARE_POINTER_CLASS# macro this closes off the class declaration.

One additional constructor is created in this class declaration which
will create a NULL smart pointer when no parameters are provided to the
constructor.

The class #cls# is declared as a smart pointer, descended from
the #par# class, to the #type# class.

Note if this macro is used then the #PIMPLEMENT_POINTER# macro must be
used to implement some inline functions that the pointer class declares.
The separate declaration and definition is sometimes required due to the
order in which the #PSmartPointer# class and the #PSmartObject#
class are declared. They may require a circular reference under some
circumstances.
*/
#define PDECLARE_POINTER(cls, par, type) \
  PDECLARE_POINTER_CLASS(cls, par, type) \
    public: \
      cls() { } \
  }

/**This macro is used to implement a smart pointer.
  This macro implements the optionally inline functions, using the
  #PINLINE# macro.
 */
#define PIMPLEMENT_POINTER(cls, par, type) \
  PINLINE cls::cls(type * obj) : par(obj) { } \
  PINLINE type * cls::operator->() const \
    { return (type *)PAssertNULL(object); } \
  PINLINE type & cls::operator*() const \
    { return *(type *)PAssertNULL(object); }


/** This macro is used to declare a smart pointer.
Unlike the #PDECLARE_POINTER_CLASS# macro this closes off the class declaration
and assumes that it is descended directly off the PSmartPointer class. Also
the member functions are implemented as inlines directly in the declaration.

If the order in which the #PSmartPointer# and the #PSmartObject#
classes are declared causes problems, use the separate
#PDECLARE_POINTER_CLASS# and #PIMPLEMENT_POINTER# macros.
*/
#define PSMART_POINTER(cls, type) \
  PDECLARE_CLASS(cls, par) \
    public: \
      cls(type * obj = NULL) : par(obj) { } \
      type * operator->() const \
        { return (type *)PAssertNULL(object); } \
      type & operator*() const \
        { return *(type *)PAssertNULL(object); } \


///////////////////////////////////////////////////////////////////////////////
// General notification mechanism from one object to another

/** This class is the #PSmartObject# contents of the #PNotifier#
   class.

   This is an abstract class for which a descendent is declared for every
   function that may be called. The #PDECLARE_NOTIFIER# macro makes this
   declaration.

   The #PNotifier# and PNotifierFunction classes build a completely type
   safe mechanism for calling arbitrary member functions on classes. The
   "pointer to a member function" capability built into C++ makes the
   assumption that the function name exists in an ancestor class. If you wish
   to call a member function name that does {\bf not} exist in any ancestor
   class, very type unsafe casting of the member functions must be made. Some
   compilers will even refuse to do it at all!

   To overcome this problem, as this mechanism is highly desirable for callback
   functions in the GUI part of the PWLib library, these classes and a macro
   are used to create all the classes and declarations to use polymorphism as
   the link between the caller, which has no knowledege of the function, and
   the receiver object and member function.

   This is most often used as the notification of actions being take by
   interactors in the PWLib library.
 */
class PNotifierFunction : public PSmartObject
{
  PCLASSINFO(PNotifierFunction, PSmartObject);

  public:
    /// Create a notification function instance.
    PNotifierFunction(
      void * obj    /// Object instance that the function will be called on.
    ) { object = PAssertNULL(obj); }

    /** Execute the call to the actual notification function on the object
       instance contained in this object.
     */
    virtual void Call(
      PObject & notifier,  /// Object that is making the notification.
      INT extra            /// Extra information that may be passed to function.
    ) const = 0;

  protected:
    // Member variables
    /** Object instance to receive the notification function call. */
    void * object;
};


/** This class is the #PSmartPointer# to the #PNotifierFunction#
   class.

   The PNotifier and #PNotifierFunction# classes build a completely type
   safe mechanism for calling arbitrary member functions on classes. The
   "pointer to a member function" capability built into C++ makes the
   assumption that the function name exists in an ancestor class. If you wish
   to call a member function name that does {\bf not} exist in any ancestor
   class, very type unsafe casting of the member functions must be made. Some
   compilers will even refuse to do it at all!

   To overcome this problem, as this mechanism is highly desirable for callback
   functions in the GUI part of the PWLib library, these classes and a macro
   are used to create all the classes and declarations to use polymorphism as
   the link between the caller, which has no knowledege of the function, and
   the receiver object and member function.

   This is most often used as the notification of actions being take by
   interactors in the PWLib library.
 */
class PNotifier : public PSmartPointer
{
  PCLASSINFO(PNotifier, PSmartPointer);

  public:
    /** Create a new notification function smart pointer. */
    PNotifier(
      PNotifierFunction * func = NULL   /// Notifier function to call.
    ) : PSmartPointer(func) { }

    /**Execute the call to the actual notification function on the object
       instance contained in this object. This will make a polymorphic call to
       the function declared by the #PDECLARE_NOTIFIER# macro which in
       turn calls the required function in the destination object.
     */
    virtual void operator()(
      PObject & notifier,  /// Object that is making the notification.
      INT extra            /// Extra information that may be passed to function.
    ) const {((PNotifierFunction*)PAssertNULL(object))->Call(notifier,extra);}
};


/** Declare a notifier object class.
  This macro declares the descendent class of #PNotifierFunction# that
  will be used in instances of #PNotifier# created by the
  #PCREATE_NOTIFIER# or #PCREATE_NOTIFIER2# macros.

  The macro is expected to be used inside a class declaration. The class it
  declares will therefore be a nested class within the class being declared.
  The name of the new nested class is derived from the member function name
  which should guarentee the class names are unique.

  The #notifier# parameter is the class of the function that will be
  calling the notification function. The #notifiee# parameter is the
  class to which the called member function belongs. Finally the
  #func# parameter is the name of the member function to be
  declared.

  This macro will also declare the member function itself. This will be:
\begin{verbatim}
      void func(notifier & n, INT extra)
\end{verbatim}

  The implementation of the function is left for the user.
 */
#define PDECLARE_NOTIFIER(notifier, notifiee, func) \
  class func##_PNotifier : public PNotifierFunction { \
    public: \
      func##_PNotifier(notifiee * obj) : PNotifierFunction(obj) { } \
      virtual void Call(PObject & note, INT extra) const \
        { ((notifiee*)object)->func((notifier &)note, extra); } \
  }; \
  friend class func##_PNotifier; \
  virtual void func(notifier & note, INT extra)

/** Create a notifier object instance.
  This macro creates an instance of the particular #PNotifier# class using
  the #func# parameter as the member function to call.

  The #obj# parameter is the instance to call the function against.
  If the instance to be called is the current instance, ie #obj# is
  to #this# the the #PCREATE_NOTIFIER# macro should be used.
 */
#define PCREATE_NOTIFIER2(obj, func) PNotifier(new func##_PNotifier(obj))

/** Create a notifier object instance.
  This macro creates an instance of the particular #PNotifier# class using
  the #func# parameter as the member function to call.

  The #this# object is used as the instance to call the function
  against. The #PCREATE_NOTIFIER2# macro may be used if the instance to be
  called is not the current object instance.
 */
#define PCREATE_NOTIFIER(func) PCREATE_NOTIFIER2(this, func)


///////////////////////////////////////////////////////////////////////////////
// Really big integer class for architectures without

#ifndef P_HAS_INT64

class PInt64__ {
  public:
    operator long()  const { return (long)low; }
    operator int()   const { return (int)low; }
    operator short() const { return (short)low; }
    operator char()  const { return (char)low; }

    operator unsigned long()  const { return (unsigned long)low; }
    operator unsigned int()   const { return (unsigned int)low; }
    operator unsigned short() const { return (unsigned short)low; }
    operator unsigned char()  const { return (unsigned char)low; }

  protected:
    PInt64__() { }
    PInt64__(unsigned long l) : low(l), high(0) { }
    PInt64__(unsigned long l, unsigned long h) : low(l), high(h) { }

    void operator=(const PInt64__ & v) { low = v.low; high = v.high; }

    void Inc() { if (++low == 0) ++high; }
    void Dec() { if (--low == 0) --high; }

    void Or (long v) { low |= v; }
    void And(long v) { low &= v; }
    void Xor(long v) { low ^= v; }

    void Add(const PInt64__ & v);
    void Sub(const PInt64__ & v);
    void Mul(const PInt64__ & v);
    void Div(const PInt64__ & v);
    void Mod(const PInt64__ & v);
    void Or (const PInt64__ & v) { low |= v.low; high |= v.high; }
    void And(const PInt64__ & v) { low &= v.low; high &= v.high; }
    void Xor(const PInt64__ & v) { low ^= v.low; high ^= v.high; }
    void ShiftLeft(int bits);
    void ShiftRight(int bits);

    BOOL Eq(unsigned long v) const { return low == v && high == 0; }
    BOOL Ne(unsigned long v) const { return low != v || high != 0; }

    BOOL Eq(const PInt64__ & v) const { return low == v.low && high == v.high; }
    BOOL Ne(const PInt64__ & v) const { return low != v.low || high != v.high; }

    unsigned long low, high;
};


#define DECL_OPS(cls, type) \
    const cls & operator=(type v) { PInt64__::operator=(cls(v)); return *this; } \
    cls operator+(type v) const { cls t = *this; t.Add(v); return t; } \
    cls operator-(type v) const { cls t = *this; t.Sub(v); return t; } \
    cls operator*(type v) const { cls t = *this; t.Mul(v); return t; } \
    cls operator/(type v) const { cls t = *this; t.Div(v); return t; } \
    cls operator%(type v) const { cls t = *this; t.Mod(v); return t; } \
    cls operator|(type v) const { cls t = *this; t.Or (v); return t; } \
    cls operator&(type v) const { cls t = *this; t.And(v); return t; } \
    cls operator^(type v) const { cls t = *this; t.Xor(v); return t; } \
    cls operator<<(type v) const { cls t = *this; t.ShiftLeft((int)v); return t; } \
    cls operator>>(type v) const { cls t = *this; t.ShiftRight((int)v); return t; } \
    const cls & operator+=(type v) { Add(v); return *this; } \
    const cls & operator-=(type v) { Sub(v); return *this; } \
    const cls & operator*=(type v) { Mul(v); return *this; } \
    const cls & operator/=(type v) { Div(v); return *this; } \
    const cls & operator|=(type v) { Or (v); return *this; } \
    const cls & operator&=(type v) { And(v); return *this; } \
    const cls & operator^=(type v) { Xor(v); return *this; } \
    const cls & operator<<=(type v) { ShiftLeft((int)v); return *this; } \
    const cls & operator>>=(type v) { ShiftRight((int)v); return *this; } \
    BOOL operator==(type v) const { return Eq(v); } \
    BOOL operator!=(type v) const { return Ne(v); } \
    BOOL operator< (type v) const { return Lt(v); } \
    BOOL operator> (type v) const { return Gt(v); } \
    BOOL operator>=(type v) const { return !Gt(v); } \
    BOOL operator<=(type v) const { return !Lt(v); } \


class PInt64 : public PInt64__ {
  public:
    PInt64() { }
    PInt64(long l) : PInt64__(l, l < 0 ? -1 : 0) { }
    PInt64(unsigned long l, long h) : PInt64__(l, h) { }
    PInt64(const PInt64__ & v) : PInt64__(v) { }

    PInt64 operator~() const { return PInt64(~low, ~high); }
    PInt64 operator-() const { return operator~()+1; }

    PInt64 operator++() { Inc(); return *this; }
    PInt64 operator--() { Dec(); return *this; }

    PInt64 operator++(int) { PInt64 t = *this; Inc(); return t; }
    PInt64 operator--(int) { PInt64 t = *this; Dec(); return t; }

    DECL_OPS(PInt64, char)
    DECL_OPS(PInt64, unsigned char)
    DECL_OPS(PInt64, short)
    DECL_OPS(PInt64, unsigned short)
    DECL_OPS(PInt64, int)
    DECL_OPS(PInt64, unsigned int)
    DECL_OPS(PInt64, long)
    DECL_OPS(PInt64, unsigned long)
    DECL_OPS(PInt64, const PInt64 &)

    friend ostream & operator<<(ostream &, const PInt64 &);
    friend istream & operator>>(istream &, PInt64 &);

  protected:
    void Add(long v) { Add(PInt64(v)); }
    void Sub(long v) { Sub(PInt64(v)); }
    void Mul(long v) { Mul(PInt64(v)); }
    void Div(long v) { Div(PInt64(v)); }
    void Mod(long v) { Mod(PInt64(v)); }
    BOOL Lt(long v) const { return Lt(PInt64(v)); }
    BOOL Gt(long v) const { return Gt(PInt64(v)); }
    BOOL Lt(const PInt64 &) const;
    BOOL Gt(const PInt64 &) const;
};


class PUInt64 : public PInt64__ {
  public:
    PUInt64() { }
    PUInt64(unsigned long l) : PInt64__(l, 0) { }
    PUInt64(unsigned long l, unsigned long h) : PInt64__(l, h) { }
    PUInt64(const PInt64__ & v) : PInt64__(v) { }

    PUInt64 operator~() const { return PUInt64(~low, ~high); }

    const PUInt64 & operator++() { Inc(); return *this; }
    const PUInt64 & operator--() { Dec(); return *this; }

    PUInt64 operator++(int) { PUInt64 t = *this; Inc(); return t; }
    PUInt64 operator--(int) { PUInt64 t = *this; Dec(); return t; }

    DECL_OPS(PUInt64, char)
    DECL_OPS(PUInt64, unsigned char)
    DECL_OPS(PUInt64, short)
    DECL_OPS(PUInt64, unsigned short)
    DECL_OPS(PUInt64, int)
    DECL_OPS(PUInt64, unsigned int)
    DECL_OPS(PUInt64, long)
    DECL_OPS(PUInt64, unsigned long)
    DECL_OPS(PUInt64, const PUInt64 &)

    friend ostream & operator<<(ostream &, const PUInt64 &);
    friend istream & operator>>(istream &, PUInt64 &);

  protected:
    void Add(long v) { Add(PUInt64(v)); }
    void Sub(long v) { Sub(PUInt64(v)); }
    void Mul(long v) { Mul(PUInt64(v)); }
    void Div(long v) { Div(PUInt64(v)); }
    void Mod(long v) { Mod(PUInt64(v)); }
    BOOL Lt(long v) const { return Lt(PUInt64(v)); }
    BOOL Gt(long v) const { return Gt(PUInt64(v)); }
    BOOL Lt(const PUInt64 &) const;
    BOOL Gt(const PUInt64 &) const;
};

#undef DECL_OPS

#endif


///////////////////////////////////////////////////////////////////////////////
// Platform independent types

// All these classes encapsulate primitive types such that they may be
// transfered in a platform independent manner. In particular it is used to
// do byte swapping for little endien and big endien processor architectures
// as well as accommodating structure packing rules for memory structures.

#define PANSI_CHAR 1
#define PLITTLE_ENDIAN 2
#define PBIG_ENDIAN 3


#if 0
class PStandardType
/* Encapsulate a standard 8 bit character into a portable format. This would
   rarely need to do translation, only if the target platform uses EBCDIC
   would it do anything.

   The platform independent form here is always 8 bit ANSI.
 */
{
  public:
    PStandardType(
      type newVal   // Value to initialise data in platform dependent form.
    ) { data = newVal; }
    /* Create a new instance of the platform independent type using platform
       dependent data, or platform independent streams.
     */

    operator type() { return data; }
    /* Get the platform dependent value for the type.

       @return
       data for instance.
     */

    friend ostream & operator<<(ostream & strm, const PStandardType & val)
      { return strm << (type)val; }
    /* Output the platform dependent value for the type to the stream.

       @return
       the stream output was made to.
     */

    friend istream & operator>>(istream & strm, PStandardType & val)
      { type data; strm >> data; val = PStandardType(data); return strm; }
    /* Input the platform dependent value for the type from the stream.

       @return
       the stream input was made from.
     */


  private:
    type data;
};
#endif


#define PI_SAME(name, type) \
  struct name { \
    name() { } \
    name(type value) { data = value; } \
    name(const name & value) { data = value.data; } \
    name & operator =(type value) { data = value; return *this; } \
    name & operator =(const name & value) { data = value.data; return *this; } \
    operator type() const { return data; } \
    friend ostream & operator<<(ostream & s, const name & v) { return s << v.data; } \
    friend istream & operator>>(istream & s, name & v) { return s >> v.data; } \
    private: type data; \
  }

#define PI_LOOP(src, dst) \
    BYTE *s = ((BYTE *)&src)+sizeof(src); BYTE *d = (BYTE *)&dst; \
    while (s != (BYTE *)&src) *d++ = *--s;

#define PI_DIFF(name, type) \
  struct name { \
    name() { } \
    name(type value) { operator=(value); } \
    name(const name & value) { data = value.data; } \
    name & operator =(type value) { PI_LOOP(value, data); return *this; } \
    name & operator =(const name & value) { data = value.data; return *this; } \
    operator type() const { type value; PI_LOOP(data, value); return value; } \
    friend ostream & operator<<(ostream & s, const name & value) { return s << (type)value; } \
    friend istream & operator>>(istream & s, name & v) { type val; s >> val; v = val; return s; } \
    private: type data; \
  }

#ifndef PCHAR8
#define PCHAR8 PANSI_CHAR
#endif

#if PCHAR8==PANSI_CHAR
PI_SAME(PChar8, char);
#endif

PI_SAME(PInt8, signed char);

PI_SAME(PUInt8, unsigned char);

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PInt16l, PInt16);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PInt16l, PInt16);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PInt16b, PInt16);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PInt16b, PInt16);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PUInt16l, WORD);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PUInt16l, WORD);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PUInt16b, WORD);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PUInt16b, WORD);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PInt32l, PInt32);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PInt32l, PInt32);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PInt32b, PInt32);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PInt32b, PInt32);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PUInt32l, DWORD);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PUInt32l, DWORD);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PUInt32b, DWORD);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PUInt32b, DWORD);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PInt64l, PInt64);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PInt64l, PInt64);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PInt64b, PInt64);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PInt64b, PInt64);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PUInt64l, PUInt64);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PUInt64l, PUInt64);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PUInt64b, PUInt64);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PUInt64b, PUInt64);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PFloat32l, float);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PFloat32l, float);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PFloat32b, float);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PFloat32b, float);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PFloat64l, double);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PFloat64l, double);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PFloat64b, double);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PFloat64b, double);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_SAME(PFloat80l, long double);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_DIFF(PFloat80l, long double);
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
PI_DIFF(PFloat80b, long double);
#elif PBYTE_ORDER==PBIG_ENDIAN
PI_SAME(PFloat80b, long double);
#endif

#undef PI_LOOP
#undef PI_SAME
#undef PI_DIFF


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

/*$MACRO PARRAYSIZE(array)
   This macro is used to calculate the number of array elements in a static
   array.
 */
#define PARRAYSIZE(array) ((PINDEX)(sizeof(array)/sizeof(array[0])))

/*$MACRO PMIN(v1, v2)
   This macro is used to calculate the minimum of two values. As this is a
   macro the expression in #v1# or #v2# is executed
   twice so extreme care should be made in its use.
 */
#define PMIN(v1, v2) ((v1) < (v2) ? (v1) : (v2))

/*$MACRO PMAX(v1, v2)
   This macro is used to calculate the maximum of two values. As this is a
   macro the expression in #v1# or #v2# is executed
   twice so extreme care should be made in its use.
 */
#define PMAX(v1, v2) ((v1) > (v2) ? (v1) : (v2))

/*$MACRO PABS(val)
   This macro is used to calculate an absolute value. As this is a macro the
   expression in #val# is executed twice so extreme care should be
   made in its use.
 */
#define PABS(v) ((v) < 0 ? -(v) : (v))



// End Of File ///////////////////////////////////////////////////////////////
