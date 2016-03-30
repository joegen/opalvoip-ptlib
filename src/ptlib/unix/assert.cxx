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
    #define InternalMaxStackWalk PTrace::MaxStackWalk
  #else
    #define InternalMaxStackWalk 20
  #endif // PTRACING


  static bool fgets_nonl(char * buffer, size_t size, FILE * fp)
  {
    fd_set rd;
    FD_ZERO(&rd);
    FD_SET(fileno(fp), &rd);

    P_timeval tv(1);
    if (select(1, &rd, NULL, NULL, tv) != 1)
      return false;

    if (fgets(buffer, size, fp) == NULL)
      return false;

    size_t len = strlen(buffer);
    if (len == 0)
      return true;

    if (buffer[--len] == '\n')
      buffer[len] = '\0';
    return true;
}

  static std::string Locate_addr2line()
  {
    std::string addr2line;

    FILE * p = popen("which addr2line", "r");
    if (p != NULL) {
      char line[100];
      if (fgets_nonl(line, sizeof(line), p) && access(line, R_OK|X_OK) == 0)
        addr2line = line;
      fclose(p);
    }

    return addr2line;
  }


  static void InternalWalkStack(ostream & strm, int skip, void * const * addresses, int addressCount)
  {
    if (addressCount <= skip) {
      strm << "\n\tStack back trace empty, possibly corrupt.";
      return;
    }

    int i;

    std::vector<std::string> lines(addressCount);

    static std::string addr2line = Locate_addr2line();
    if (!addr2line.empty()) {
      std::stringstream cmd;
      cmd << addr2line << " -e \"" << PProcess::Current().GetFile() << '"';
      for (i = skip; i < addressCount; ++i)
        cmd << ' ' << addresses[i];
      FILE * p = popen(cmd.str().c_str(), "r");
      if (p != NULL) {
        char line[200];
        for (i = skip; i < addressCount && fgets_nonl(line, sizeof(line), p); ++i) {
          if (strcmp(line, "??:0") != 0)
            lines[i] = line;
        }
        fclose(p);
      }
    }


    char ** symbols = backtrace_symbols(addresses, addressCount);
    for (i = skip; i < addressCount; ++i) {
      strm << "\n\t";

      if (symbols[i] == NULL || symbols[i][0] == '\0') {
        strm << addresses[i] << ' ' << lines[i];
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
              strm << symbols[i] << demangled << separator << offset << ' ' << lines[i];
              runtime_free(demangled);
              continue;
            }
            if (demangled != NULL)
              runtime_free(demangled);
          }
        }
      #endif // P_HAS_DEMANGLE

      strm << symbols[i] << ' ' << lines[i];
    }
    runtime_free(symbols);
  }


  static void InternalWalkStack(ostream & strm, int skip)
  {
    const size_t maxStackWalk = InternalMaxStackWalk + skip;
    void * addresses[maxStackWalk];
    InternalWalkStack(strm, skip, addresses, backtrace(addresses, maxStackWalk));
  }


  #if PTRACING
    #if P_PTHREADS
      struct PWalkStackInfo
      {
        enum { OtherThreadSkip = 6 };
        pthread_mutex_t   m_mainMutex;
        PThreadIdentifier m_id;
        vector<void *>    m_addresses;
        int               m_addressCount;
        pthread_mutex_t   m_condMutex;
        pthread_cond_t    m_condVar;

        PWalkStackInfo()
          : m_id(PNullThreadIdentifier)
          , m_addressCount(-1)
        {
          pthread_mutex_init(&m_mainMutex, NULL);
          pthread_mutex_init(&m_condMutex, NULL);
          pthread_cond_init(&m_condVar, NULL);
        }

        void WalkOther(ostream & strm, PThreadIdentifier id)
        {
          pthread_mutex_lock(&m_mainMutex);

          m_id = id;
          m_addressCount = -1;
          m_addresses.resize(InternalMaxStackWalk+OtherThreadSkip);
          if (!PThread::PX_kill(id, PProcess::WalkStackSignal)) {
            strm << "\n    Thread id=" << id << " (0x" << hex << id << ") is no longer running";
            return;
          }

          int err = 0;
          struct timespec absTime;
          absTime.tv_sec = time(NULL) + 2;
          absTime.tv_nsec = 0;

          pthread_mutex_lock(&m_condMutex);
          while (m_addressCount < 0) {
            if ((err = pthread_cond_timedwait(&m_condVar, &m_condMutex, &absTime)) != 0)
              break;
          }
          pthread_mutex_unlock(&m_condMutex);

          if (err == ETIMEDOUT)
            strm << "\n    No response getting stack trace for id=" << id << " (0x" << hex << id << ')';
          else if (err != 0)
            strm << "\n    Error " << err << " getting stack trace for id=" << id << " (0x" << hex << id << ')';
          else
            InternalWalkStack(strm, OtherThreadSkip, m_addresses.data(), m_addressCount);

          m_id = PNullThreadIdentifier;

          pthread_mutex_unlock(&m_mainMutex);
        }

        void OthersWalk()
        {
          PThreadIdentifier id = PThread::GetCurrentThreadId();
          if (m_id != id) {
            if (id == PNullThreadIdentifier)
              PTRACE(0, "StackWalk", "Thread took too long to respond to signal");
            else
              PTRACE(0, "StackWalk", "Signal received on " << id << " (0x" << hex << id << ")"
                                     " but expected " << m_id << " (0x" << hex << m_id << ')');
            return;
          }

          int addressCount = backtrace(m_addresses.data(), m_addresses.size());

          pthread_mutex_lock(&m_condMutex);
          m_addressCount = addressCount < 0 ? 0 : addressCount;
          pthread_cond_signal(&m_condVar);
          pthread_mutex_unlock(&m_condMutex);
        }
      };
    #else // P_PTHREADS
      struct PWalkStackInfo
      {
        void WalkOther(ostream &, PThreadIdentifier) { }
        void OthersWalk() { }
      };
    #endif // P_PTHREADS

    static PWalkStackInfo s_otherThreadStack;


    void PTrace::WalkStack(ostream & strm, PThreadIdentifier id)
    {
      if (id == PNullThreadIdentifier || id == PThread::GetCurrentThreadId())
        InternalWalkStack(strm, 2);
      else if (PProcess::IsInitialised())
        s_otherThreadStack.WalkOther(strm, id);
    }

    void PProcess::InternalWalkStackSignaled()
    {
      if (IsInitialised())
        s_otherThreadStack.OthersWalk();
    }
  #endif // PTRACING

#else

  #define InternalWalkStack(...)

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
        abort();
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


static void InternalAssertFunc(const char * msg)
{
  std::string str;
  {
    ostringstream strm;
    strm << msg;
    InternalWalkStack(strm, 3);
    strm << ends;
    str = strm.str();
  }

  OUTPUT_MESSAGE(str);

  char *env;

#if P_EXCEPTIONS
  //Throw a runtime exception if the environment variable PWLIB_ASSERT_EXCEPTION is set
  env = ::getenv("PTLIB_ASSERT_EXCEPTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_EXCEPTION");
  if (env != NULL) {
    throw std::runtime_error(msg);
    return;
  }
#endif

  env = ::getenv("PTLIB_ASSERT_ACTION");
  if (env == NULL)
    env = ::getenv("PWLIB_ASSERT_ACTION");
  if (env != NULL && *env != EOF && AssertAction(*env, msg))
    return;

  // Check for if stdin is not a TTY and just ignore the assert if so.
  if (isatty(STDIN_FILENO) != 1) {
    AssertAction('i', msg);
    return;
  }

  do {
    PError << '\n' << ActionMessage << "? " << flush;
  } while (AssertAction(getchar(), msg));
}


static PCriticalSection s_AssertMutex;
static bool s_TopLevelAssert = true;

bool PAssertFunc(const char * msg)
{
  s_AssertMutex.Wait();
  if (s_TopLevelAssert) {
    s_TopLevelAssert = false;
    InternalAssertFunc(msg);
    s_TopLevelAssert = true;
  }
  s_AssertMutex.Signal();
  return false;
}


// End Of File ///////////////////////////////////////////////////////////////
