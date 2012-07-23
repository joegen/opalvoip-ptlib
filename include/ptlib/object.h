/*
 * object.h
 *
 * Mother of all ancestor classes.
 *
 * Portable Tools Library
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_OBJECT_H
#define PTLIB_OBJECT_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if defined(_WIN32)
#include "msos/ptlib/platform.h"
#else
#include "unix/ptlib/platform.h"
#endif

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <typeinfo>

using namespace std; // Not a good practice (name space polution), but will take too long to fix.

// Somewhere in C headers you get this, which blows up STL version
#ifdef min
#undef min
#endif
#ifdef max
#undef max
#endif


///////////////////////////////////////////////////////////////////////////////

#define P_REMOVE_VIRTUAL_INTERNAL_BASE(fn) __inline virtual struct ptlib_virtual_function_changed_or_removed ****** fn { return 0; }

#if defined(_MSC_VER)
  #if _MSC_VER < 1310
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #elif _MSC_VER < 1400
    #define P_DEPRECATED __declspec(deprecated)
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __inline virtual __declspec(deprecated) type fn body
  #else
    #define P_DEPRECATED __declspec(deprecated)
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __inline virtual __declspec(deprecated("Virtual function signature changed or function deprecated")) type fn body
  #endif
#elif defined(__GNUC__)
  #if __GNUC__ < 4
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #else
    #define P_DEPRECATED __attribute__((deprecated))
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) __attribute__((warn_unused_result)) __attribute__((deprecated)) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
  #endif
#else
    #define P_DEPRECATED
    #define P_REMOVE_VIRTUAL_INTERNAL(type, fn, body) P_REMOVE_VIRTUAL_INTERNAL_BASE(fn)
#endif

#define P_REMOVE_VIRTUAL_VOID(fn)       P_REMOVE_VIRTUAL_INTERNAL(void, fn, {})
#define P_REMOVE_VIRTUAL(type, fn, ret) P_REMOVE_VIRTUAL_INTERNAL(type, fn, { return ret; })


// P_USE_INTEGER_BOOL is the default and gives the old behaviour (it
// is also used for C translation units).
// without P_USE_INTEGER_BOOL, the ANSI C++ bool is used.

#ifndef FALSE
#define FALSE 0
#endif
#ifndef TRUE
#define TRUE 1
#endif

#if defined(P_USE_INTEGER_BOOL) || !defined(__cplusplus)
  typedef BOOL   PBoolean;
  #define PTrue  TRUE
  #define PFalse FALSE
#else
  typedef bool   PBoolean;
  #define PTrue  true
  #define PFalse false
#endif


///////////////////////////////////////////////////////////////////////////////
// Disable inlines when debugging for faster compiles (the compiler doesn't
// actually inline the function with debug on any way).

#ifndef P_USE_INLINES
#ifdef _DEBUG
#define P_USE_INLINES 0
#else
#define P_USE_INLINES 0
#endif
#endif

#if P_USE_INLINES
#define PINLINE inline
#else
#define PINLINE
#endif


///////////////////////////////////////////////////////////////////////////////
// Declare the debugging support

#ifndef P_USE_ASSERTS
#define P_USE_ASSERTS 1
#endif

#if !P_USE_ASSERTS

#define PAssert(b, m) (b)
#define PAssert2(b, c, m) (b)
#define PAssertOS(b) (b)
#define PAssertNULL(p) (p)
#define PAssertAlways(m) {}
#define PAssertAlways2(c, m) {}

#else // P_USE_ASSERTS

/// Standard assert messages for the PAssert macro.
enum PStandardAssertMessage {
  PLogicError,              ///< A logic error occurred.
  POutOfMemory,             ///< A new or malloc failed.
  PNullPointerReference,    ///< A reference was made through a NULL pointer.
  PInvalidCast,             ///< An invalid cast to descendant is required.
  PInvalidArrayIndex,       ///< An index into an array was negative.
  PInvalidArrayElement,     ///< A NULL array element object was accessed.
  PStackEmpty,              ///< A Pop() was made of a stack with no elements.
  PUnimplementedFunction,   ///< Funtion is not implemented.
  PInvalidParameter,        ///< Invalid parameter was passed to a function.
  POperatingSystemError,    ///< Error was returned by Operating System.
  PChannelNotOpen,          ///< Operation attempted when channel not open.
  PUnsupportedFeature,      ///< Feature is not supported.
  PInvalidWindow,           ///< Access through invalid window.
  PMaxStandardAssertMessage ///< Number of standard assert message.
};

#define __CLASS__ NULL

bool PAssertFunc(const char * file, int line, const char * className, PStandardAssertMessage msg);
bool PAssertFunc(const char * file, int line, const char * className, const char * msg);
bool PAssertFunc(const char * full_msg);


/** This macro is used to assert that a condition must be true.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message described
by the <code>msg</code> parameter. This parameter may be either a standard value
from the <code>PStandardAssertMessage</code> enum or a literal string.
*/
#define PAssert(b, msg) ((b)?true:PAssertFunc(__FILE__,__LINE__,__CLASS__,(msg)))

/** This macro is used to assert that a condition must be true.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message described
by the <code>msg</code> parameter. This parameter may be either a standard value
from the <code>PStandardAssertMessage</code> enum or a literal string.
The <code>cls</code> parameter specifies the class name that the error occurred in
*/
#define PAssert2(b, cls, msg) ((b)?true:PAssertFunc(__FILE__,__LINE__,(cls),(msg)))

/** This macro is used to assert that an operating system call succeeds.
If the condition is false then an assert function is called with the source
file and line number the macro was instantiated on, plus the message
described by the <code>POperatingSystemError</code> value in the <code>PStandardAssertMessage</code>
enum.
 */
#define PAssertOS(b) ((b)?true:PAssertFunc(__FILE__,__LINE__,__CLASS__,POperatingSystemError))

/** This macro is used to assert that a pointer must be non-null.
If the pointer is NULL then an assert function is called with the source file
and line number the macro was instantiated on, plus the message described by
the PNullPointerReference value in the <code>PStandardAssertMessage</code> enum.

Note that this evaluates the expression defined by <code>ptr</code> twice. To
prevent incorrect behaviour with this, the macro will assume that the
<code>ptr</code> parameter is an L-Value.
 */
#define PAssertNULL(ptr) (((ptr)!=NULL)?(ptr): \
                     (PAssertFunc(__FILE__,__LINE__, __CLASS__, PNullPointerReference),(ptr)))

/** This macro is used to assert immediately.
The assert function is called with the source file and line number the macro
was instantiated on, plus the message described by the <code>msg</code> parameter. This
parameter may be either a standard value from the <code>PStandardAssertMessage</code>
enum or a literal string.
*/
#define PAssertAlways(msg) PAssertFunc(__FILE__,__LINE__,__CLASS__,(msg))

/** This macro is used to assert immediately.
The assert function is called with the source file and line number the macro
was instantiated on, plus the message described by the <code>msg</code> parameter. This
parameter may be either a standard value from the <code>PStandardAssertMessage</code>
enum or a literal string.
*/
#define PAssertAlways2(cls, msg) PAssertFunc(__FILE__,__LINE__,(cls),(msg))

#endif // P_USE_ASSERTS


/** Get the stream being used for error output.
This stream is used for all trace output using the various trace functions
and macros.
*/
ostream & PGetErrorStream();

/** Set the stream to be used for error output.
This stream is used for all error output using the <code>PError</code> macro.
*/
void PSetErrorStream(ostream * strm /** New stream for error output */ );

/** This macro is used to access the platform specific error output stream.
This is to be used in preference to assuming <code>cerr</code> is always available. On
Unix platforms this {\b is} <code>cerr</code> but for MS-Windows this is another stream
that uses the OutputDebugString() Windows API function. Note that a MS-DOS or
Windows NT console application would still use <code>cerr</code>.

The <code>PError</code> stream would normally only be used for debugging information as
a suitable display is not always available in windowed environments.
   
The macro is a wrapper for a global variable error stream. The internal variable
is initialised to <i>std::cerr</i> for all but MS-Windows and NT GUI applications.
An application could change this pointer to a <i>std::ofstream</i> variable of
#PError output is wished to be redirected to a file.
*/
#define PError (PGetErrorStream())



///////////////////////////////////////////////////////////////////////////////
// Debug and tracing

#ifndef PTRACING
#define PTRACING 2
#endif

#if PTRACING

class PObject;

/**Class to encapsulate tracing functions.
   This class does not require any instances and is only being used as a
   method of grouping functions together in a name space.

   There are a number of macros for supporting tracing. These will all
   evaluate as empty in a "No Trace" build of the system:
     - PTRACE()
     - PTRACE_IF()
     - PTRACE_PARAM()
     - PTRACE_BLOCK()
     - PTRACE_LINE()
  */
class PTrace
{
public:
  /// Options for trace output.
  enum Options {
    /**Include PTrace::Block constructs in output
       If this is bit is clear, all PTrace::Block output is inhibited
       regardless of the trace level. If set, the PTrace::Block may occur
       provided the trace level is greater than zero.
    */
    Blocks = 1,
    /// Include date and time in all output
    DateAndTime = 2,
    /// Include (millisecond) timestamp in all output
    Timestamp = 4,
    /// Include identifier for thread trace is made from in all output
    Thread = 8,
    /// Include trace level in all output
    TraceLevel = 16,
    /// Include the file and line for the trace call in all output
    FileAndLine = 32,
    /// Include thread object pointer address in all trace output
    ThreadAddress = 64,
    /// Append to log file rather than resetting every time
    AppendToFile = 128,
    /// Output timestamps in GMT time rather than local time
    GMTTime = 256,
    /// If set, log file will be rotated daily
    RotateDaily = 512,
    /// If set, log file will be rotated hourly
    RotateHourly = 1024,
    /// If set, log file will be rotated every minute
    RotateMinutely = 2048,
    /// Mask for all the rotate bits
    RotateLogMask = RotateDaily + RotateHourly + RotateMinutely,
    /// Include object instance in all trace output
    ObjectInstance = 4096,
    /// Include context identifier in all trace output
    ContextIdentifier = 8192,
    /** SystemLog flag for tracing within a PServiceProcess application. Must
        be set in conjection with <code>#SetStream(new PSystemLog)</code>.
      */
    SystemLogStream = 32768
  };


  /**Set the most common trace options.
     If \p filename is not NULL then a PTextFile is created and attached the
     trace output stream. This object is never closed or deleted until the
     termination of the program.

     There are several special values for \p filename:
       <dl>
       <dt>"stderr"      <dd>Output to standard error
       <dt>"stdout"      <dd>Output to standard output
       <dt>"DEBUGSTREAM" <dd>Output to debugger (Windows only)
       </dl>
     A trace output of the program name version and OS is written as well.
    */
  static void Initialise(
    unsigned level,                               ///< Level for tracing
    const char * filename = NULL,                 ///< Filename for log output
    unsigned options = Timestamp | Thread | Blocks ///< #Options for tracing
  );

  /**Set the most common trace options.
     If \p filename is not NULL then a PTextFile is created and attached the
     trace output stream. This object is never closed or deleted until the
     termination of the program.

     If \p rolloverPatterm is not NULL it is used as the time format patterm
     appended to filename if the #RotateDaily is set. Default is "yyyy_MM_dd".

     A trace output of the program name version and OS is written as well.
    */
  static void Initialise(
    unsigned level,                                 ///< Level for tracing
    const char * filename,                          ///< Filename for log output
    const char * rolloverPattern,                   ///< Pattern for rolling over trace files
    unsigned options = Timestamp | Thread | Blocks  ///< #Options for tracing
  );

  /** Set the trace options.
  The PTRACE(), PTRACE_BLOCK() and PTRACE_LINE() macros output trace text that
  may contain assorted values. These are defined by the #Options enum.

  Note this function OR's the bits included in the options parameter.
  */
  static void SetOptions(
    unsigned options ///< New option bits for tracing
  );

  /** Clear the trace options.
  The <code>PTRACE()</code>, <code>PTRACE_BLOCK()</code> and
  <code>PTRACE_LINE()</code> macros output trace text that
  may contain assorted values. These are defined by the #Options enum.

  Note this function AND's the complement of the bits included in the options
  parameter.
  */
  static void ClearOptions(
    unsigned options ///< Option bits to turn off
  );

  /** Get the current trace options.
  The <code>PTRACE()</code>, <code>PTRACE_BLOCK()</code> and
  <code>PTRACE_LINE()</code> macros output trace text that
  may contain assorted values. These are defined by the #Options enum.
  */
  static unsigned GetOptions();

  /** Set the trace level.
  The <code>PTRACE()</code> macro checks to see if its level is equal to or lower then the
  level set by this function. If so then the trace text is output to the trace
  stream.
  */
  static void SetLevel(
    unsigned level ///< New level for tracing
  );

  /** Get the trace level.
  The <code>PTRACE()</code> macro checks to see if its level is equal to or lower then the
  level set by this function. If so then the trace text is output to the trace
  stream.
  */
  static unsigned GetLevel();

  /** Determine if the level may cause trace output.
  This checks against the current global trace level set by SetLevel()
  for if the trace output may be emitted. This is used by the PTRACE() macro.
  */
  static PBoolean CanTrace(
    unsigned level ///< Trace level to check
  );

  /** Set the stream to be used for trace output.
  This stream is used for all trace output using the various trace functions
  and macros.
  */
  static void SetStream(
    ostream * out ///< New output stream from trace.
  );

  /** Get the stream being used for trace output.
  */
  static ostream * GetStream();

  /** Begin a trace output.
  If the trace stream output is used outside of the provided macros, it
  should be noted that a mutex is obtained on the call to Begin() which
  will prevent any other threads from using the trace stream until the
  End() function is called.

  So a typical usage would be:
  <pre><code>
    ostream & s = PTrace::Begin(3, __FILE__, __LINE__);
    s << "hello";
    if (want_there)
      s << " there";
    s << '!' << PTrace::End;
  </code></pre>
  */
  static ostream & Begin(
    unsigned level,         ///< Log level for output
    const char * fileName,  ///< Filename of source file being traced
    int lineNum,            ///< Line number of source file being traced.
    const PObject * instance = NULL,  ///< Instance for object logging occurred in
    const char * module = NULL        ///< Module or subsection string
  );

  static ostream & Begin(
    unsigned level,           ///< Log level for output
    const char * fileName,    ///< Filename of source file being traced
    int lineNum,              ///< Line number of source file being traced.
    const char * module,      ///< Module or subsection string
    const PObject * instance, ///< Instance for object logging occurred in
    const char * defModule
  ) { return Begin(level, fileName, lineNum, instance, module != NULL ? module : defModule); }

  static ostream & Begin(
    unsigned level,           ///< Log level for output
    const char * fileName,    ///< Filename of source file being traced
    int lineNum,              ///< Line number of source file being traced.
    const PObject * instance, ///< Instance for object logging occurred in
    const PObject * defInstance,
    const char * module       ///< Module or subsection string
  ) { return Begin(level, fileName, lineNum, instance != NULL ? instance : defInstance, module); }

  /** End a trace output.
  If the trace stream output is used outside of the provided macros, the
  End() function must be used at the end of the section of trace
  output. A mutex is obtained on the call to Begin() which will prevent
  any other threads from using the trace stream until the End(). The
  End() is used in a similar manner to <code>std::endl</code> or
  <code>std::flush</code>.

  So a typical usage would be:
  <pre><code>
    ostream & s = PTrace::Begin();
    s << "hello";
    if (want_there)
      s << " there";
    s << '!' << PTrace::End;
  </code></pre>
  */
  static ostream & End(
    ostream & strm ///< Trace output stream being completed
  );

  /** Class to trace Execution blocks.
  This class is used for tracing the entry and exit of program blocks. Upon
  construction it outputs an entry trace message and on destruction outputs an
  exit trace message. This is normally only used from in the <code>PTRACE_BLOCK()</code> macro.
  */
  class Block {
    public:
      /** Output entry trace message. */
      Block(
        const char * fileName, ///< Filename of source file being traced
        int lineNum,           ///< Line number of source file being traced.
        const char * traceName
          ///< String to be output with trace, typically it is the function name.
       );
      Block(const Block & obj)
        : file(obj.file), line(obj.line), name(obj.name) { }
      /// Output exit trace message.
      ~Block();
    private:
      Block & operator=(const Block &)
      { return *this; }
      const char * file;
      int          line;
      const char * name;
  };

  static unsigned GetNextContextIdentifier();
};

/* Macro to conditionally declare a parameter to a function to avoid compiler
   warning due that parameter only being used in a <code>PTRACE()</code> */
#define PTRACE_PARAM(...) __VA_ARGS__

/** Trace an execution block.
This macro creates a trace variable for tracking the entry and exit of program
blocks. It creates an instance of the PTraceBlock class that will output a
trace message at the line <code>PTRACE_BLOCK()</code> is called and then on exit from the
scope it is defined in.
*/
#define PTRACE_BLOCK(name) PTrace::Block __trace_block_instance(__FILE__, __LINE__, name)

/** Trace the execution of a line.
This macro outputs a trace of a source file line execution.
*/
#define PTRACE_LINE() \
    if (PTrace::CanTrace(1)) \
      PTrace::Begin(1, __FILE__, __LINE__) << __FILE__ << '(' << __LINE__ << ')' << PTrace::End; \
    else (void)0



#define PTRACE_INTERNAL(level, condition, args, ...) \
    if (PTrace::CanTrace(level) condition) \
      PTrace::Begin(level, __FILE__, __LINE__, __VA_ARGS__) << args << PTrace::End; \
    else (void)0

#define PTRACE_NO_CONDITION


#define PTRACE_PART1(narg, args) PTRACE_PART2(narg, args)
#define PTRACE_PART2(narg, args) PTRACE_ARG_##narg args

#define PTRACE_ARG_4(level, object, module, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, object, module)

#define PTRACE_ARG_3(level, objectOrModule, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, objectOrModule, PTraceObjectInstance(), PTraceModule())

#define PTRACE_ARG_2(level, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, PTraceObjectInstance(), PTraceModule())


#define PTRACE_IF_PART1(narg, args) PTRACE_IF_PART2(narg, args)
#define PTRACE_IF_PART2(narg, args) PTRACE_IF_ARG_##narg args

#define PTRACE_IF_ARG_5(level, condition, object, module, args) \
        PTRACE_INTERNAL(level, && (condition), args, object, module)

#define PTRACE_IF_ARG_4(level, condition, objectOrModule, args) \
        PTRACE_INTERNAL(level, && (condition), args, objectOrModule, PTraceObjectInstance(), PTraceModule())

#define PTRACE_IF_ARG_3(level, condition, args) \
        PTRACE_INTERNAL(level, && (condition), args, PTraceObjectInstance(), PTraceModule())


#define PTRACE_BEGIN_PART1(narg, args) PTRACE_BEGIN_PART2(narg, args)
#define PTRACE_BEGIN_PART2(narg, args) PTRACE_BEGIN_ARG_##narg args

#define PTRACE_BEGIN_ARG_3(level, object, module) \
      PTrace::Begin(level, __FILE__, __LINE__, object, module)

#define PTRACE_BEGIN_ARG_2(level, objectOrModule) \
      PTrace::Begin(level, __FILE__, __LINE__, objectOrModule, PTraceObjectInstance(), PTraceModule())

#define PTRACE_BEGIN_ARG_1(level) \
      PTrace::Begin(level, __FILE__, __LINE__, PTraceObjectInstance(), PTraceModule())


#define PTRACE_NARG(...) PTRACE_NARG_PART1(PTRACE_NARG_PART2(__VA_ARGS__,4,3,2,1,0))
#define PTRACE_NARG_PART1(arg) arg
#define PTRACE_NARG_PART2(_1,_2,_3,_4,N,...) N

// Backward compatibility
#define PTRACE2(level, object, args) \
        PTRACE_INTERNAL(level, PTRACE_NO_CONDITION, args, object, PTraceModule())
#define PTRACE_IF2(level, condition, object, args) \
        PTRACE_INTERNAL(level, && (condition), args, object, PTraceModule())

/** Output trace.
This macro outputs a trace of any information needed, using standard stream
output operators. The output is only made if the trace level set by the
SetLevel() function is greater than or equal to the first argument.

There can be variable numbers of arguments to this macro. Its full form is:

  PTRACE(level, instance, module, stream)

The level is the level for this trace log, instance is a PObject instance to
generate a context for the trace, module is a string representing a subsytem
for filtering and stream is a standard C++ stream output expression.

Both instance and module can by NULL, or absent. If only one of instance or
module is present it is determined by its type (PObject * or char *) as to
which it is.

The stream is only evaluated if level is below the PTrace::GetLevel()
threshold, so do not expect any functions to always be executed.

Note: If this is used with a static function of a PObject descendant there
will be an issue with the default usage of PTraceObjectInstance(). To avoid
the issue you will need to make sure the PTRACE macro has four parameters, as
in the full form descibed above.
*/
#define PTRACE(...) PTRACE_PART1(PTRACE_NARG(__VA_ARGS__), (__VA_ARGS__))

/** Output trace on condition.
This macro conditionally outputs a trace of any information needed.

This macro has variable arguments, and is of the form:

  PTRACE_IF(level, condition, instance, module, stream)

Note condition is only evaluated if level is below the PTrace::GetLevel()
threshold, and stream is only evaluated if condition is evaluated to true.

See PTRACE() for more information on level, instance, module and stream.
*/
#define PTRACE_IF(...) PTRACE_IF_PART1(PTRACE_NARG(__VA_ARGS__), (__VA_ARGS__))

/** Begin output trace.
This macro returns a ostream & for trace output.

This macro has variable arguments, and is of the form:

  PTRACE_BEGIN(level, instance, module)

See PTRACE() for more information on level, instance, module.
*/
#define PTRACE_BEGIN(...) PTRACE_BEGIN_PART1(PTRACE_NARG(__VA_ARGS__), (__VA_ARGS__))


__inline const PObject * PTraceObjectInstance() { return NULL; }
__inline const char * PTraceModule() { return NULL; }


#if PTRACING==2

/**Propagate PTRACE context identifier in an object from another.
   The context identifier can group objects together with a single
   identifier which can aid in debugging highly threaded systems where
   the logs from various threads become very interleaved.
  */
#define PTRACE_CONTEXT_ID_NEW() SetTraceContextIdentifier(PTrace::GetNextContextIdentifier())
#define PTRACE_CONTEXT_ID_SET(to, from) (to).SetTraceContextIdentifier(from)
#define PTRACE_CONTEXT_ID_FROM(obj) SetTraceContextIdentifier(obj)
#define PTRACE_CONTEXT_ID_TO(obj) GetTraceContextIdentifier(obj)

class PTraceSaveContextIdentifier
{
  private:
    class PThread * m_currentThread;
    unsigned        m_savedContextIdentifier;
  public:
    PTraceSaveContextIdentifier(const PObject & obj);
    PTraceSaveContextIdentifier(const PObject * obj);
    ~PTraceSaveContextIdentifier();
};

#define PTRACE_CONTEXT_ID_PUSH_THREAD(obj) PTraceSaveContextIdentifier praceSavedContextIdentifier(obj)

#endif // PTRACING==2

#endif // PTRACING

#ifndef PTRACE_PARAM
#define PTRACE_PARAM(...)
#endif

#ifndef PTRACE_BLOCK
#define PTRACE_BLOCK(n)
#endif

#ifndef PTRACE_LINE
#define PTRACE_LINE()
#endif

#ifndef PTRACE
#define PTRACE(...)
#endif

#ifndef PTRACE_IF
#define PTRACE_IF(...)
#endif

#ifndef PTRACE_BEGIN
#define PTRACE_BEGIN(...)
#endif

#ifndef PTRACE2
#define PTRACE2(level, obj, arg)
#endif

#ifndef PTRACE_IF2
#define PTRACE_IF2(level, cond, obj, args)
#endif

#ifndef PTRACE_CONTEXT_ID_NEW
#define PTRACE_CONTEXT_ID_NEW()
#endif

#ifndef PTRACE_CONTEXT_ID_SET
#define PTRACE_CONTEXT_ID_SET(to, from)
#endif

#ifndef PTRACE_CONTEXT_ID_FROM
#define PTRACE_CONTEXT_ID_FROM(obj)
#endif

#ifndef PTRACE_CONTEXT_ID_TO
#define PTRACE_CONTEXT_ID_TO(obj)
#endif

#ifndef PTRACE_CONTEXT_ID_PUSH_THREAD
#define PTRACE_CONTEXT_ID_PUSH_THREAD(obj)
#endif


#if PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG) && !defined(_WIN32_WCE)) 

#define PMEMORY_HEAP 1

/** Memory heap checking class.
This class implements the memory heap checking and validation functions. It
maintains lists of allocated block so that memory leaks can be detected. It
also initialises memory on allocation and deallocation to help catch errors
involving the use of dangling pointers.
*/
class PMemoryHeap {
  public:
    /// Initialise the memory checking subsystem.
    PMemoryHeap();

    // Clear up the memory checking subsystem, dumping memory leaks.
    ~PMemoryHeap();

    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t nSize,           ///< Number of bytes to allocate.
      const char * file,      ///< Source file name for allocating function.
      int line,               ///< Source file line for allocating function.
      const char * className  ///< Class name for allocating function.
    );
    /** Allocate a memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
       @return pointer to newly allocated memory block.
     */
    static void * Allocate(
      size_t count,       ///< Number of items to allocate.
      size_t iSize,       ///< Size in bytes of each item.
      const char * file,  ///< Source file name for allocating function.
      int line            ///< Source file line for allocating function.
    );

    /** Change the size of an allocated memory block.
       This allocates a new memory block and keeps track of it. The memory
       block is filled with the value in the <code>allocFillChar</code> member variable
       to help detect uninitialised structures.
      @return pointer to reallocated memory block. Note this may
      {\em not} be the same as the pointer passed into the function.
     */
    static void * Reallocate(
      void * ptr,         ///< Pointer to memory block to reallocate.
      size_t nSize,       ///< New number of bytes to allocate.
      const char * file,  ///< Source file name for allocating function.
      int line            ///< Source file line for allocating function.
    );

    /** Free a memory block.
      The memory is deallocated, a warning is displayed if it was never
      allocated. The block of memory is filled with the value in the
      <code>freeFillChar</code> member variable.
     */
    static void Deallocate(
      void * ptr,             ///< Pointer to memory block to deallocate.
      const char * className  ///< Class name for deallocating function.
    );

    /** Validation result.
     */
    enum Validation {
      Ok, Bad, Trashed
    };
    /** Validate the memory pointer.
        The <code>ptr</code> parameter is validated as a currently allocated heap
        variable.
        @return Ok for pointer is in heap, Bad for pointer is not in the heap
        or Trashed if the pointer is in the heap but has overwritten the guard
        bytes before or after the actual data part of the memory block.
     */
    static Validation Validate(
      const void * ptr,       ///< Pointer to memory block to check
      const char * className, ///< Class name it should be.
      ostream * error         ///< Stream to receive error message (may be NULL)
    );

    /** Validate all objects in memory.
       This effectively calls Validate() on every object in the heap.
        @return true if every object in heap is Ok.
     */
    static PBoolean ValidateHeap(
      ostream * error = NULL  ///< Stream to output, use default if NULL
    );

    /** Ignore/Monitor allocations.
       Set internal flag so that allocations are not included in the memory
       leak check on program termination.
       Returns the previous state.
     */
    static PBoolean SetIgnoreAllocations(
      PBoolean ignore  ///< New flag for allocation ignoring.
    );

    /** Get memory check system statistics.
        Dump statistics output to the default stream.
     */
    static void DumpStatistics();
    /** Get memory check system statistics.
        Dump statistics output to the specified stream.
     */
    static void DumpStatistics(ostream & strm /** Stream to output to */);

#if PMEMORY_CHECK
    struct State {
      DWORD allocationNumber;
    };
#else
	typedef _CrtMemState State;
#endif

    /* Get memory state.
      This returns a state that may be used to determine where to start dumping
      objects from.
     */
    static void GetState(
      State & state  ///< Memory state
    );

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the <code>GetAllocationRequest()</code>
       function.

       Output is to the default stream.
     */
    static void DumpObjectsSince(
      const State & when    ///< Memory state to begin dump from.
    );

    /** Dump allocated objects.
       Dump ojects allocated and not deallocated since the specified object
       number. This would be a value returned by the <code>GetAllocationRequest()</code>
       function.
     */
    static void DumpObjectsSince(
      const State & when,   ///< Memory state to begin dump from.
      ostream & strm        ///< Stream to output dump
    );

    /** Set break point allocation number.
      Set the allocation request number to cause an assert. This allows a
      developer to cause a halt in a debugger on a certain allocation allowing
      them to determine memory leaks allocation point.
     */
    static void SetAllocationBreakpoint(
      DWORD point   ///< Allocation number to stop at.
    );

#if PMEMORY_CHECK

  protected:
    void * InternalAllocate(
      size_t nSize,           // Number of bytes to allocate.
      const char * file,      // Source file name for allocating function.
      int line,               // Source file line for allocating function.
      const char * className  // Class name for allocating function.
    );
    Validation InternalValidate(
      const void * ptr,       // Pointer to memory block to check
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
      enum {
        // Assure that the Header struct is aligned to 8 byte boundary
        NumGuardBytes = 16 - (sizeof(Header *) +
                              sizeof(Header *) +
                              sizeof(const char *) +
                              sizeof(const char *) +
                              sizeof(size_t) +
                              sizeof(DWORD) +
                              sizeof(WORD) +
                              sizeof(BYTE) +
                              sizeof(PThreadIdentifier)
                              )%8
      };

      Header     * prev;
      Header     * next;
      const char * className;
      const char * fileName;
      size_t       size;
      DWORD        request;
      WORD         line;
      BYTE         flags;
      PThreadIdentifier threadId;
      char         guard[NumGuardBytes];

      static char GuardBytes[NumGuardBytes];
    };
#pragma pack()

    PBoolean isDestroyed;

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

#if defined(_WIN32)
    CRITICAL_SECTION mutex;
#elif defined(P_PTHREADS)
    pthread_mutex_t mutex;
#elif defined(P_VXWORKS)
    void * mutex;
#endif

#else

#define P_CLIENT_BLOCK (_CLIENT_BLOCK|(0x61<<16)) // This identifies a PObject derived class
    _CrtMemState initialState;

#endif // PMEMORY_CHECK
};


/** Allocate memory for the run time library.
This version of free is used for data that is not to be allocated using the
memory check system, ie will be free'ed inside the C run time library.
*/
inline void * runtime_malloc(size_t bytes /** Size of block to allocate */ ) { return malloc(bytes); }

/** Free memory allocated by run time library.
This version of free is used for data that is not allocated using the
memory check system, ie was malloc'ed inside the C run time library.
*/
inline void runtime_free(void * ptr /** Memory block to free */ ) { free(ptr); }


/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define malloc(s) PMemoryHeap::Allocate(s, __FILE__, __LINE__, NULL)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define calloc(n,s) PMemoryHeap::Allocate(n, s, __FILE__, __LINE__)

/** Override of system call for memory check system.
This macro is used to allocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define realloc(p,s) PMemoryHeap::Reallocate(p, s, __FILE__, __LINE__)


/** Override of system call for memory check system.
This macro is used to deallocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define free(p) PMemoryHeap::Deallocate(p, NULL)


/** Override of system call for memory check system.
This macro is used to deallocate memory via the memory check system selected
with the <code>PMEMORY_CHECK</code> compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.
*/
#define cfree(p) PMemoryHeap::Deallocate(p, NULL)


/** Macro for overriding system default <code>new</code> operator.
This macro is used to allocate memory via the memory check system selected
with the PMEMORY_CHECK compile time option. It will include the source file
and line into the memory allocation to allow the PMemoryHeap class to keep
track of the memory block.

This macro could be used instead of the system <code>new</code> operator. Or you can place
the line
<pre><code>
  #define new PNEW
</code></pre>
at the begining of the source file, after all declarations that use the
<code>#PCLASSINFO</code> macro.
*/
#define PNEW  new (__FILE__, __LINE__)

#if !defined(_MSC_VER) || _MSC_VER<1200
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
    void * operator new(size_t, void * placement) \
      { return placement; } \
    void operator delete(void *, void *) \
      { } \
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
void * operator new(size_t nSize);
void * operator new[](size_t nSize);

void operator delete(void * ptr);
void operator delete[](void * ptr);

#if defined(_MSC_VER) && _MSC_VER>=1200
inline void operator delete(void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }

inline void operator delete[](void * ptr, const char *, int)
  { PMemoryHeap::Deallocate(ptr, NULL); }
#endif
#endif


class PMemoryHeapIgnoreAllocationsForScope {
public:
  PMemoryHeapIgnoreAllocationsForScope() : previousIgnoreAllocations(PMemoryHeap::SetIgnoreAllocations(true)) { }
  ~PMemoryHeapIgnoreAllocationsForScope() { PMemoryHeap::SetIgnoreAllocations(previousIgnoreAllocations); }
private:
  PBoolean previousIgnoreAllocations;
};

#define PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE PMemoryHeapIgnoreAllocationsForScope instance_PMemoryHeapIgnoreAllocationsForScope

class PMemoryAllocationBreakpoint {
public:
  PMemoryAllocationBreakpoint(DWORD point)
  {
    PMemoryHeap::SetAllocationBreakpoint(point);
  }
};

#define PMEMORY_ALLOCATION_BREAKPOINT(point) PMemoryAllocationBreakpoint PMemoryAllocationBreakpointInstance(point)


#else // PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG))

#define PMEMORY_HEAP 0

#define PNEW new

#define PNEW_AND_DELETE_FUNCTIONS

#define runtime_malloc(s) malloc(s)
#define runtime_free(p) free(p)

#define PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE
#define PMEMORY_ALLOCATION_BREAKPOINT(point)

#endif // PMEMORY_CHECK || (defined(_MSC_VER) && defined(_DEBUG))


/*
 *  Implement "construct on first use" paradigm
 */

template <class GnuAllocator, class Type>
struct PAllocatorTemplate
{
  Type * allocate(size_t v)  
  {
    return GetAllocator().allocate(v);
  }

  void deallocate(Type * p, size_t v)  
  {
    GetAllocator().deallocate(p, v);
  }

  private:
    static GnuAllocator & GetAllocator()
    {
      static GnuAllocator instance;
      return instance;
    }
};

#define GCC_VERSION (__GNUC__ * 10000 \
                   + __GNUC_MINOR__ * 100 \
                   + __GNUC_PATCHLEVEL__)

// Memory pooling allocators
#if defined(__GNUC__) && (GCC_VERSION > 40000) && !defined(P_MINGW) && !defined(P_MACOSX) 
#include <ext/mt_allocator.h>
template <class Type> struct PFixedPoolAllocator    : public PAllocatorTemplate<__gnu_cxx::__mt_alloc<Type>, Type> { };
template <class Type> struct PVariablePoolAllocator : public PAllocatorTemplate<__gnu_cxx::__mt_alloc<Type>, Type> { };

#else

template <class Type> struct PFixedPoolAllocator    : public PAllocatorTemplate<std::allocator<Type>, Type> { };
template <class Type> struct PVariablePoolAllocator : public PAllocatorTemplate<std::allocator<Type>, Type> { };
#endif

#define PDECLARE_POOL_ALLOCATOR() \
    void * operator new(size_t nSize); \
    void * operator new(size_t nSize, const char * file, int line); \
    void operator delete(void * ptr); \
    void operator delete(void * ptr, const char *, int)

#define PDEFINE_POOL_ALLOCATOR(cls) \
  static PFixedPoolAllocator<cls> cls##_allocator; \
  void * cls::operator new(size_t)                           { return cls##_allocator.allocate(1);               } \
  void * cls::operator new(size_t, const char *, int)        { return cls##_allocator.allocate(1);               } \
  void   cls::operator delete(void * ptr)                    {        cls##_allocator.deallocate((cls *)ptr, 1); } \
  void   cls::operator delete(void * ptr, const char *, int) {        cls##_allocator.deallocate((cls *)ptr, 1); }



/** Declare all the standard PTLib class information.
This macro is used to provide the basic run-time typing capability needed
by the library. All descendent classes from the <code>PObject</code> class require
these functions for correct operation. Either use this macro or the
<code>#PDECLARE_CLASS</code> macro.

The use of the <code>#PDECLARE_CLASS</code> macro is no longer recommended for reasons
of compatibility with documentation systems.
*/

#define PCLASSINFO(cls, par) \
  public: \
    typedef cls P_thisClass; \
    static inline const char * Class() \
      { return #cls; } \
    virtual PBoolean InternalIsDescendant(const char * clsName) const \
      { return strcmp(clsName, cls::Class()) == 0 || par::InternalIsDescendant(clsName); } \
    virtual const char * GetClass(unsigned ancestor = 0) const \
      { return ancestor > 0 ? par::GetClass(ancestor-1) : cls::Class(); } \
    virtual PObject::Comparison CompareObjectMemoryDirect(const PObject & obj) const \
      { return PObject::InternalCompareObjectMemoryDirect(this, dynamic_cast<const cls *>(&obj), sizeof(cls)); } \
    PNEW_AND_DELETE_FUNCTIONS


#define PIsDescendant(ptr, cls)    (dynamic_cast<const cls *>(ptr) != NULL) 
#define PIsDescendantStr(ptr, str) ((ptr)->InternalIsDescendant(str)) 

#define PRemoveConst(cls, ptr)  (const_cast<cls*>(ptr))

#if P_USE_ASSERTS
template<class BaseClass> inline BaseClass * PAssertCast(BaseClass * obj, const char * file, int line) 
  { if (obj == NULL) PAssertFunc(file, line, BaseClass::Class(), PInvalidCast); return obj; }
#define PDownCast(cls, ptr) PAssertCast<cls>(dynamic_cast<cls*>(ptr),__FILE__,__LINE__)
#else
#define PDownCast(cls, ptr) (dynamic_cast<cls*>(ptr))
#endif


/** Declare a class with PWLib class information.
This macro is used to declare a new class with a single public ancestor. It
starts the class declaration and then uses the <code>#PCLASSINFO</code> macro to
get all the run-time type functions.

The use of this macro is no longer recommended for reasons of compatibility
with documentation systems.
*/
#define PDECLARE_CLASS(cls, par) class cls : public par { PCLASSINFO(cls, par)
#ifdef DOC_PLUS_PLUS
} Match previous opening brace in doc++
#endif

///////////////////////////////////////////////////////////////////////////////
// The root of all evil ... umm classes

/** Ultimate parent class for all objects in the class library.
This provides functionality provided to all classes, eg run-time types,
default comparison operations, simple stream I/O and serialisation support.
*/
class PObject {
  protected:
    unsigned m_traceContextIdentifier;
#if PTRACING==2
  public:
    /**Get PTRACE context identifier
      */
    unsigned GetTraceContextIdentifier() const { return m_traceContextIdentifier; }
    void SetTraceContextIdentifier(unsigned id) { m_traceContextIdentifier = id; }
    void GetTraceContextIdentifier(PObject & obj) { obj.m_traceContextIdentifier = m_traceContextIdentifier; }
    void GetTraceContextIdentifier(PObject * obj) { if (obj != NULL) obj->m_traceContextIdentifier = m_traceContextIdentifier; }
    void SetTraceContextIdentifier(const PObject & obj) { m_traceContextIdentifier = obj.m_traceContextIdentifier; }
    void SetTraceContextIdentifier(const PObject * obj) { if (obj != NULL) m_traceContextIdentifier = obj->m_traceContextIdentifier; }
#endif // PTRACING==2

  protected:
    /** Constructor for PObject, made protected so cannot ever create one on
       its own.
     */
    PObject()
      : m_traceContextIdentifier(0)
    { }

  public:
    /* Destructor required to get the "virtual". A PObject really has nothing
       to destroy.
     */
    virtual ~PObject() { }

    /**@name Run Time Type functions */
  //@{
    /** Get the name of the class as a C string. This is a static function which
       returns the type of a specific class. 
       
       When comparing class names, always use the <code>strcmp()</code>
       function rather than comparing pointers. The pointers are not
       necessarily the same over compilation units depending on the compiler,
       platform etc.

       @return pointer to C string literal.
     */      
    static inline const char * Class()    { return "PObject"; }

    /** Get the current dynamic type of the object instance.

       When comparing class names, always use the <code>strcmp()</code>
       function rather than comparing pointers. The pointers are not
       necessarily the same over compilation units depending on the compiler,
       platform etc.

       The <code>#PCLASSINFO</code> macro declares an override of this function for
       the particular class. The user need not implement it.

       @return pointer to C string literal.
     */
    virtual const char * GetClass(unsigned ancestor = 0) const { return ancestor > 0 ? "" : Class(); }

    PBoolean IsClass(const char * cls) const 
    { return strcmp(cls, GetClass()) == 0; }

    /** Determine if the dynamic type of the current instance is a descendent of
       the specified class. The class name is usually provided by the
       <code>Class()</code> static function of the desired class.
    
       The <code>#PCLASSINFO</code> macro declares an override of this function for
       the particular class. The user need not implement it.

       @return true if object is descended from the class.
     */
    virtual PBoolean InternalIsDescendant(
      const char * clsName    // Ancestor class name to compare against.
    ) const
    { return IsClass(clsName); }

    __inline const PObject * PTraceObjectInstance() const { return this; }
  //@}

  /**@name Comparison functions */
  //@{
    /** Result of the comparison operation performed by the <code>Compare()</code>
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
       
       The default function is to use the <code>CompareObjectMemoryDirect()</code>
       function to do a byte wise memory comparison of the two objects.

       @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the relative rank of the objects.
     */
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;
    
    /** Determine the byte wise comparison of two objects. This is the default
       comparison operation for objects that do not explicitly override the
       <code>Compare()</code> function.
    
       The <code>#PCLASSINFO</code> macro declares an override of this function for
       the particular class. The user need not implement it.

       @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the result <code>memcpy()</code> function.
     */
    virtual Comparison CompareObjectMemoryDirect(
      const PObject & obj   // Object to compare against.
    ) const;

    /// Internal function caled from CompareObjectMemoryDirect()
    static Comparison InternalCompareObjectMemoryDirect(
      const PObject * obj1,
      const PObject * obj2,
      PINDEX size
    );

    /** Compare the two objects.
    
       @return
       true if objects are equal.
     */
    bool operator==(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == EqualTo; }

    /** Compare the two objects.
    
       @return
       true if objects are not equal.
     */
    bool operator!=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != EqualTo; }

    /** Compare the two objects.
    
       @return
       true if objects are less than.
     */
    bool operator<(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == LessThan; }

    /** Compare the two objects.
    
       @return
       true if objects are greater than.
     */
    bool operator>(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) == GreaterThan; }

    /** Compare the two objects.
    
       @return
       true if objects are less than or equal.
     */
    bool operator<=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != GreaterThan; }

    /** Compare the two objects.
    
       @return
       true if objects are greater than or equal.
     */
    bool operator>=(
      const PObject & obj   // Object to compare against.
    ) const { return Compare(obj) != LessThan; }
  //@}

  /**@name I/O functions */
  //@{
    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator<<</code> function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream &strm   // Stream to print the object into.
    ) const;

    /** Input the contents of the object from the stream. The exact input is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator>></code> function.

       The default behaviour is to do nothing.
     */
    virtual void ReadFrom(
      istream &strm   // Stream to read the objects contents from.
    );


    /** Global function for using the standard << operator on objects descended
       from PObject. This simply calls the objects <code>PrintOn()</code> function.
       
       @return the \p strm parameter.
     */
    inline friend ostream & operator<<(
      ostream &strm,       ///< Stream to print the object into.
      const PObject & obj  ///< Object to print to the stream.
    ) { obj.PrintOn(strm); return strm; }

    /** Global function for using the standard >> operator on objects descended
       from PObject. This simply calls the objects <code>ReadFrom()</code> function.

       @return the \p strm parameter.
     */
    inline friend istream & operator>>(
      istream &strm,   ///< Stream to read the objects contents from.
      PObject & obj    ///< Object to read inormation into.
    ) { obj.ReadFrom(strm); return strm; }


  /**@name Miscellaneous functions */
  //@{
    /** Create a copy of the class on the heap. The exact semantics of the
       descendent class determine what is required to make a duplicate of the
       instance. Not all classes can even \b do a clone operation.
       
       The main user of the clone function is the <code>PDictionary</code> class as
       it requires copies of the dictionary keys.

       The default behaviour is for this function to assert.

       @return
       pointer to new copy of the class instance.
     */
    virtual PObject * Clone() const;

    /** As for Clone() but converts to specified type.
      */
    template <class CLS>
    CLS * CloneAs() const
    {
      PObject * clone = Clone();
      CLS * obj = dynamic_cast<CLS *>(clone);
      if (obj != NULL)
        return obj;
      delete clone;
      return NULL;
    }

    /** This function yields a hash value required by the <code>PDictionary</code>
       class. A descendent class that is required to be the key of a dictionary
       should override this function. The precise values returned is dependent
       on the semantics of the class. For example, the <code>PString</code> class
       overrides it to provide a hash function for distinguishing text strings.

       The default behaviour is to return the value zero.

       @return
       hash function value for class instance.
     */
    virtual PINDEX HashFunction() const;
  //@}
};

///////////////////////////////////////////////////////////////////////////////
// Platform independent types

// All these classes encapsulate primitive types such that they may be
// transfered in a platform independent manner. In particular it is used to
// do byte swapping for little endien and big endien processor architectures
// as well as accommodating structure packing rules for memory structures.

#define PANSI_CHAR 1
#define PLITTLE_ENDIAN 2
#define PBIG_ENDIAN 3


template <typename type>
struct PIntSameOrder {
  __inline PIntSameOrder()                            : data(0)              { }
  __inline PIntSameOrder(type value)                  : data(value)          { }
  __inline PIntSameOrder(const PIntSameOrder & value) : data(value.data)     { }
  __inline PIntSameOrder & operator=(type value)                             { data = value; return *this; }
  __inline PIntSameOrder & operator=(const PIntSameOrder & value)            { data = value.data; return *this; }
  __inline operator type() const                                             { return data; }
  __inline friend ostream & operator<<(ostream & s, const PIntSameOrder & v) { return s << v.data; }
  __inline friend istream & operator>>(istream & s, PIntSameOrder & v)       { return s >> v.data; }

  private:
    type data;
};


template <typename type>
struct PIntReversedOrder {
  __inline PIntReversedOrder()                                : data(0)              { }
  __inline PIntReversedOrder(type value)                                             { ReverseBytes(value, data); }
  __inline PIntReversedOrder(const PIntReversedOrder & value) : data(value.data)     { }
  __inline PIntReversedOrder & operator=(type value)                                 { ReverseBytes(value, data); return *this; }
  __inline PIntReversedOrder & operator=(const PIntReversedOrder & value)            { data = value.data; return *this; }
  __inline operator type() const                                                     { type value; ReverseBytes(data, value); return value; }
  __inline friend ostream & operator<<(ostream & s, const PIntReversedOrder & value) { return s << (type)value; }
  __inline friend istream & operator>>(istream & s, PIntReversedOrder & v)           { type val; s >> val; v = val; return s; }

  private:
    type data;

  static __inline void ReverseBytes(const type & src, type & dst)
  {
    size_t s = sizeof(type)-1;
    for (size_t d = 0; d < sizeof(type); ++d,--s)
      ((BYTE *)&dst)[d] = ((const BYTE *)&src)[s];
  }
};

#ifndef PCHAR8
#define PCHAR8 PANSI_CHAR
#endif

#if PCHAR8==PANSI_CHAR
typedef PIntSameOrder<char> PChar8;
#endif

typedef PIntSameOrder<char> PInt8;

typedef PIntSameOrder<unsigned char> PUInt8;

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt16> PInt16l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt16> PInt16l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt16> PInt16b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt16> PInt16b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<WORD> PUInt16l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<WORD> PUInt16l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<WORD> PUInt16b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<WORD> PUInt16b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt32> PInt32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt32> PInt32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt32> PInt32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt32> PInt32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<DWORD> PUInt32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<DWORD> PUInt32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<DWORD> PUInt32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<DWORD> PUInt32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PInt64> PInt64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PInt64> PInt64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PInt64> PInt64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PInt64> PInt64b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<PUInt64> PUInt64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<PUInt64> PUInt64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<PUInt64> PUInt64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<PUInt64> PUInt64b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<float> PFloat32l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<float> PFloat32l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<float> PFloat32b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<float> PFloat32b;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<double> PFloat64l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<double> PFloat64l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<double> PFloat64b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<double> PFloat64b;
#endif

#ifndef NO_LONG_DOUBLE // stupid OSX compiler
#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntSameOrder<long double> PFloat80l;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntReversedOrder<long double> PFloat80l;
#endif

#if PBYTE_ORDER==PLITTLE_ENDIAN
typedef PIntReversedOrder<long double> PFloat80b;
#elif PBYTE_ORDER==PBIG_ENDIAN
typedef PIntSameOrder<long double> PFloat80b;
#endif
#endif


#ifdef _MSC_VER
#define P_PACK_FIELD(f) __declspec(align(1)) f
#else
#define P_PACK_FIELD(f) f __attribute__ ((packed))
#endif


///////////////////////////////////////////////////////////////////////////////
// Miscellaneous

/*$MACRO PARRAYSIZE(array)
   This macro is used to calculate the number of array elements in a static
   array.
 */
#define PARRAYSIZE(array) ((PINDEX)(sizeof(array)/sizeof(array[0])))

/*$MACRO PMIN(v1, v2)
   This macro is used to calculate the minimum of two values.
   Maps to std::min and is for backward compatibility only.
 */
#define PMIN(v1, v2) std::min(v1, v2)

/*$MACRO PMAX(v1, v2)
   This macro is used to calculate the maximum of two values.
   Maps to std::max and is for backward compatibility only.
 */
#define PMAX(v1, v2) std::max(v1, v2)

/*$MACRO PABS(val)
   This macro is used to calculate an absolute value.
   Maps to std::abs and is for backward compatibility only.
 */
#define PABS(v) std::abs(v)


#endif // PTLIB_OBJECT_H


// End Of File ///////////////////////////////////////////////////////////////
