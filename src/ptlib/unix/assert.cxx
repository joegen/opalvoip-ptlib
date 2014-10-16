/*
 * assert.cxx
 *
 * Assert function implementation.
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
 * $Revision$
 * $Author$
 * $Date$
 */


#include <ptlib.h>

#include <ctype.h>
#include <signal.h>
#include <stdexcept>
#include <ptlib/pprocess.h>


#if P_HAS_BACKTRACE

  #include <execinfo.h>
  #if P_HAS_DEMANGLE
    #include <cxxabi.h>
  #endif

  #if PTRACING
    #define InternalStackWalk    PTrace::WalkStack
    #define InternalMaxStackWalk PTrace::MaxStackWalk
  #else
    #define InternalMaxStackWalk 20
    static
  #endif // PTRACING
  void InternalStackWalk(ostream & strm, PThreadIdentifier id)
  {
    if (id != PNullThreadIdentifier && id != PThread::GetCurrentThreadId()) {
      strm << "\n    Cannot get stack trace for other thread\n";
      return;
    }

    void* addresses[InternalMaxStackWalk];
    int addressCount = backtrace(addresses, InternalMaxStackWalk);
    if (addressCount == 0) {
      strm << "\n    Stack back trace empty, possibly corrupt\n";
      return;
    }

    char ** symbols = backtrace_symbols(addresses, addressCount);
    for (int i = 1; i < addressCount; ++i) {
      strm << "\n    ";
      if (symbols[i] == NULL || symbols[i][0] == '\0') {
        strm << addresses[i];
        continue;
      }

      #if P_HAS_DEMANGLE
        char * mangled = strchr(symbols[i], '(');
        if (mangled != NULL) {
          ++mangled;
          char * offset = mangled + strcspn(mangled, "+)");
          if (offset != mangled) {
            char separator = *offset;
            *offset++ = '\0';

    	      int status = -1;
	          char * demangled = abi::__cxa_demangle(mangled, NULL, NULL, &status);
            if (status == 0) {
              *mangled = '\0';
              strm << symbols[i] << demangled << separator << offset;
              free(demangled);
              continue;
            }
            if (demangled != NULL)
              free(demangled);
          }
        }
      #endif // P_HAS_DEMANGLE

      strm << symbols[i];
    }
    free(symbols);
  }
#else

  #define InternalStackWalk(s, i)

  #if PTRACING
    void PTrace::WalkStack(ostream &, PThreadIdentifier)
    {
    }
  #endif // PTRACING

#endif // P_HAS_BACKTRACE


#define OUTPUT_MESSAGE(msg) \
    PTRACE_IF(0, PTrace::GetStream() != &PError, NULL, "PTLib", msg); \
    PError << msg << endl


#if defined(P_ANDROID)

  #include <android/log.h>

  #undef  OUTPUT_MESSAGE
  #define OUTPUT_MESSAGE(msg) \
      PTRACE(0, NULL, "PTLib", msg); \
      __android_log_assert("", PProcess::Current().GetName(), "%s", msg.c_str());

  static const char ActionMessage[] = "Ignoring";

  static bool AssertAction(int, const char *)
  {
    return false;
  }

#elif defined(P_BEOS)

  static const char ActionMessage[] = "Entering debugger";

  static bool AssertAction(int c, const char * msg)
  {
    // Pop up the debugger dialog that gives the user the necessary choices
    // "Ignore" is not supported on BeOS but you can instruct the
    // debugger to continue executing.
    // Note: if you choose "debug" you get a debug prompt. Type bdb to
    // start the Be Debugger.
    debugger(msg);

    return false;
  }

#elif defined(P_VXWORKS)

  static const char ActionMessage[] = "Aborting";

  static bool AssertAction(int c, const char *)
  {
    exit(1);
    kill(taskIdSelf(), SIGABRT);
    return false;
  }

#else

  static const char ActionMessage[] = "<A>bort, <C>ore dump, "
  #if P_EXCEPTIONS
                                      "<T>hrow exception, "
  #endif
  #ifdef _DEBUG
                                      "<D>ebug, "
  #endif
                                      "<I>gnore";

  static bool AssertAction(int c, const char * msg)
  {
    switch (c) {
      case 'a' :
      case 'A' :
        PError << "\nAborting.\n";
        _exit(1);
        return true;

  #if P_EXCEPTIONS
      case 't' :
      case 'T' :
        PError << "\nThrowing exception.\n";
        throw std::runtime_error(msg);
        return true;
  #endif
        
  #ifdef _DEBUG
      case 'd' :
      case 'D' :
        {
          PString cmd = ::getenv("PTLIB_ASSERT_DEBUGGER");
          if (cmd.IsEmpty())
            cmd = "gdb";
          cmd &= PProcess::Current().GetFile();
          cmd.sprintf(" %d", getpid());
          PError << "\nStarting debugger \"" << cmd << '"' << endl;
          system((const char *)cmd);
        }
        return false;
  #endif

      case 'c' :
      case 'C' :
        PError << "\nDumping core.\n";
        raise(SIGABRT);
        return false;

      case 'i' :
      case 'I' :
      case EOF :
        PError << "\nIgnoring.\n";
        return false;

      default :
        return true;
    }
  }

#endif


static PCriticalSection AssertMutex;

bool PAssertFunc(const char * msg)
{
  std::string str;
  {
    ostringstream strm;
    strm << msg;
    InternalStackWalk(strm, PNullThreadIdentifier);
    strm << ends;
    str = strm.str();
  }

  PWaitAndSignal mutex(AssertMutex);

  OUTPUT_MESSAGE(str);

  char *env;

#if P_EXCEPTIONS
  //Throw a runtime exception if the environment variable PWLIB_ASSERT_EXCEPTION is set
  env = ::getenv("PTLIB_ASSERT_EXCEPTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_EXCEPTION");
  if (env != NULL) {
    throw std::runtime_error(msg);
    return false;
  }
#endif

  env = ::getenv("PTLIB_ASSERT_ACTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_ACTION");
  if (env != NULL && *env != EOF && AssertAction(*env, msg))
    return false;

  // Check for if stdin is not a TTY and just ignore the assert if so.
  if (isatty(STDIN_FILENO) != 1) {
    AssertAction('i', msg);
    return false;
  }

  do {
    PError << '\n' << ActionMessage << "? " << flush;
  } while (AssertAction(getchar(), msg));

  return false;
}


// End Of File ///////////////////////////////////////////////////////////////
