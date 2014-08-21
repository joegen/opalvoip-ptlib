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

#define PTRACE_WITH_STDERR(msg) \
    PTRACE_IF(0, PTrace::GetStream() != &PError, NULL, "PTLib", msg); \
    PError << msg << endl


#if PTRACING
void PTrace::WalkStack(ostream &, PThreadIdentifier)
{
}
#endif // PTRACING


#if defined(P_ANDROID)

#include <android/log.h>

bool PAssertFunc(const char * msg)
{
  PTRACE_WITH_STDERR(msg);
  __android_log_assert("", PProcess::Current().GetName(), "%s", msg);
  return false;
}

#elif defined(P_BEOS)

bool PAssertFunc(const char * msg)
{
  // Print location in Eddie-compatible format
  PTRACE_WITH_STDERR(msg);

  // Pop up the debugger dialog that gives the user the necessary choices
  // "Ignore" is not supported on BeOS but you can instruct the
  // debugger to continue executing.
  // Note: if you choose "debug" you get a debug prompt. Type bdb to
  // start the Be Debugger.
  debugger(msg);

  return false;
}

#elif defined(P_VXWORKS)

bool PAssertFunc(const char * msg)
{
  PTRACE_WITH_STDERR(msg);

  PThread::Trace(); // Get debugging dump
  exit(1);
  kill(taskIdSelf(), SIGABRT);

  return false;
}

#else

static PBoolean PAssertAction(int c, const char * msg)
{
  switch (c) {
    case 'a' :
    case 'A' :
      PError << "\nAborting.\n";
      _exit(1);
      break;

#if P_EXCEPTIONS
    case 't' :
    case 'T' :
      PError << "\nThrowing exception\n";
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
        PError << "\nStarting debugger \"" << cmd << "\"\n";
        system((const char *)cmd);
      }
      break;
#endif

    case 'c' :
    case 'C' :
      PError << "\nDumping core.\n";
      raise(SIGABRT);

    case 'i' :
    case 'I' :
    case EOF :
      PError << "\nIgnoring.\n";
      return true;
  }
  return false;
}


bool PAssertFunc(const char * msg)
{
  static PBoolean inAssert;
  if (inAssert)
    return false;
  inAssert = true;

  PTRACE_WITH_STDERR(msg);

  char *env;

#if P_EXCEPTIONS
  //Throw a runtime exception if the environment variable PWLIB_ASSERT_EXCEPTION is set
  env = ::getenv("PTLIB_ASSERT_EXCEPTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_EXCEPTION");
  if (env != NULL){
    throw std::runtime_error(msg);
  }
#endif
  
  env = ::getenv("PTLIB_ASSERT_ACTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_ACTION");
  if (env != NULL && *env != EOF && PAssertAction(*env, msg)) {
    inAssert = false;
    return false;
  }

  // Check for if stdin is not a TTY and just ignore the assert if so.
  if (isatty(STDIN_FILENO) != 1) {
    inAssert = false;
    return false;
  }

  for(;;) {
    PError << "\n<A>bort, <C>ore dump"
#if P_EXCEPTIONS
           << ", <T>hrow exception"
#endif
#ifdef _DEBUG
           << ", <D>ebug"
#endif
           << ", <I>gnore? " << flush;

    if (PAssertAction(getchar(), msg))
      break;
   }
   inAssert = false;

  return false;
}

#endif // P_ANDROID || P_VXWORKS || P_BEOS


// End Of File ///////////////////////////////////////////////////////////////
