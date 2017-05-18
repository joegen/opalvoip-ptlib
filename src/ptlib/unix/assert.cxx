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
 */


#include <ptlib.h>

#include <ctype.h>
#include <signal.h>
#include <stdexcept>
#include <ptlib/pprocess.h>


#if P_HAS_BACKTRACE && PTRACING

  #define DEBUG_CERR(arg) // std::cerr << arg << std::endl

  #include <execinfo.h>
  #if P_HAS_DEMANGLE
    #include <cxxabi.h>
  #endif

  #if PTRACING
    #define InternalMaxStackWalk PTrace::MaxStackWalk
  #else
    #define InternalMaxStackWalk 20
  #endif // PTRACING


  static bool fgetstr(std::string & str, FILE * fp)
  {
    for (;;) {
      struct pollfd pfd;
      pfd.fd = fileno(fp);
      pfd.events = POLLIN;
      pfd.revents = 0;

      int pollStatus = ::poll(&pfd, 1, 10000);

      if (pollStatus < 0) {
        if (errno == EINTR)
          continue;
        str = strerror(errno);
        return false;
      }

      if (pollStatus == 0) {
        str = "Timed out";
        return false;
      }

      char c;
      if (read(pfd.fd, &c, 1) <= 0) {
        str += " - fgetc - ";
        str += strerror(errno);
        return false;
      }

      if (c == '\n')
        return true;

      str += (char)c;
    }
  }


  static void InternalWalkStack(ostream & strm, unsigned framesToSkip, void * const * addresses, unsigned addressCount)
  {
    DEBUG_CERR("InternalWalkStack: count=" << addressCount << ", framesToSkip=" << framesToSkip);
    if (addressCount <= framesToSkip) {
      strm << "\n\tStack back trace empty, possibly corrupt.";
      return;
    }

    std::vector<std::string> lines(addressCount);

    {
      FILE * pipe;
      {
        std::stringstream cmd;
        cmd << "addr2line -e \"" << PProcess::Current().GetFile() << '"';
        for (unsigned i = framesToSkip; i < addressCount; ++i)
          cmd << ' ' << addresses[i];
        std::string cmdstr = cmd.str();
        DEBUG_CERR("InternalWalkStack: before " << cmdstr);
        pipe = popen(cmdstr.c_str(), "r");
      }
      DEBUG_CERR("InternalWalkStack: addr2line pipe=" << pipe);
      if (pipe != NULL) {
        for (unsigned i = framesToSkip; i < addressCount; ++i) {
          std::string & line = lines[i];
          if (!fgetstr(line, pipe)) {
            DEBUG_CERR("InternalWalkStack: fgetstr error: " << line);
            if (!line.empty())
              strm << "\n\tStack trace error with addr2line: " << line;
            break;
          }
          DEBUG_CERR("InternalWalkStack: line=\"" << line << '"');
          if (line == "??:0")
            line.clear();
        }
        DEBUG_CERR("InternalWalkStack: closing pipe=" << pipe);
        fclose(pipe);
      }
    }


    DEBUG_CERR("InternalWalkStack: before backtrace_symbols");
    char ** symbols = backtrace_symbols(addresses, addressCount);
    DEBUG_CERR("InternalWalkStack: after backtrace_symbols");
    for (unsigned i = framesToSkip; i < addressCount; ++i) {
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

    DEBUG_CERR("InternalWalkStack: done");
  }


    #if P_PTHREADS
      struct PWalkStackInfo
      {
        enum { OtherThreadSkip = 6 };
        pthread_mutex_t   m_mainMutex;
        PThreadIdentifier m_threadId;
        PUniqueThreadIdentifier m_uniqueId;
        vector<void *>    m_addresses;
        int               m_addressCount;
        pthread_mutex_t   m_condMutex;
        pthread_cond_t    m_condVar;
        PTime             m_signalSentTime;

        PWalkStackInfo()
          : m_threadId(PNullThreadIdentifier)
          , m_uniqueId(0)
          , m_addressCount(-1)
          , m_signalSentTime(0)
        {
          pthread_mutex_init(&m_mainMutex, NULL);
          pthread_mutex_init(&m_condMutex, NULL);
          pthread_cond_init(&m_condVar, NULL);
        }

        void WalkOther(ostream & strm, PThreadIdentifier tid, PUniqueThreadIdentifier uid)
        {
          DEBUG_CERR("WalkOther: " << PThread::GetIdentifiersAsString(tid, uid));

          if (tid == 0) {
            strm << "\n\tStack trace WalkOther with zero thread ID.";
            return;
          }

          // Needs to all be done within X seconds
          struct timespec absTime;
          clock_gettime(CLOCK_REALTIME, &absTime);
          absTime.tv_sec += 2;

#if P_PTHREADS_XPG6
          bool failed = pthread_mutex_timedlock(&m_mainMutex, &absTime) != 0;
#else
          bool failed;
          for (;;) {
            failed = pthread_mutex_trylock(&m_mainMutex) != 0;
            if (!failed)
              break;

            struct timespec now;
            clock_gettime(CLOCK_REALTIME, &now);
            if (now.tv_sec > absTime.tv_sec && now.tv_nsec > absTime.tv_nsec)
              break;

            usleep(10000);
          }
#endif
          if (failed) {
            strm << "\n\tStack trace system is too busy to WalkOther";
            DEBUG_CERR("WalkOther: mutex timeout");
            return;
          }

          m_threadId = tid;
          m_uniqueId = uid;
          m_addressCount = -1;
          m_addresses.resize(InternalMaxStackWalk+OtherThreadSkip);
          m_signalSentTime.SetCurrentTime();
          if (!PThread::PX_kill(tid, uid, PProcess::WalkStackSignal)) {
            strm << "\n\tThread " << PThread::GetIdentifiersAsString(tid, uid) << " is no longer running";
            pthread_mutex_unlock(&m_mainMutex);
            return;
          }

          int err = 0;

          pthread_mutex_lock(&m_condMutex);
          while (m_addressCount < 0) {
            if ((err = pthread_cond_timedwait(&m_condVar, &m_condMutex, &absTime)) != 0)
              break;
          }
          pthread_mutex_unlock(&m_condMutex);

          if (err == ETIMEDOUT)
            strm << "\n\tNo response getting stack trace for " << PThread::GetIdentifiersAsString(tid, uid);
          else if (err != 0)
            strm << "\n\tError " << err << " getting stack trace for " << PThread::GetIdentifiersAsString(tid, uid);
          else
            InternalWalkStack(strm, OtherThreadSkip, m_addresses.data(), m_addressCount);

          m_threadId = PNullThreadIdentifier;
          m_uniqueId = 0;

          pthread_mutex_unlock(&m_mainMutex);
          DEBUG_CERR("WalkOther: done");
        }

        void OthersWalk()
        {
          PThreadIdentifier tid = PThread::GetCurrentThreadId();
          PUniqueThreadIdentifier uid = PThread::GetCurrentUniqueIdentifier();

          if (!m_signalSentTime.IsValid()) {
            PTRACE(0, "StackWalk", "No stack walk in operation, unexpected signal " << PProcess::WalkStackSignal);
            return;
          }

          if (m_threadId == PNullThreadIdentifier) {
            PTRACE(0, "StackWalk", "Thread took too long (" << m_signalSentTime.GetElapsed() << "s) to respond to signal.");
            return;
          }

          if (m_threadId != tid || (m_uniqueId != 0 && m_uniqueId != uid)) {
            PTRACE(0, "StackWalk", "Signal received on " << PThread::GetIdentifiersAsString(tid, uid) <<
                                   " but expected " << PThread::GetIdentifiersAsString(m_threadId, m_uniqueId));
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


    void PPlatformWalkStack(ostream & strm, PThreadIdentifier id, PUniqueThreadIdentifier uid, unsigned framesToSkip)
    {
      if (!PProcess::IsInitialised())
        return;

      if (id != PNullThreadIdentifier && id != PThread::GetCurrentThreadId())
        s_otherThreadStack.WalkOther(strm, id, uid);
      else {
        DEBUG_CERR("PPlatformWalkStack: id=0x" << hex << id << dec);
        // Allow for some bizarre optimisation when called from PTrace::WalkStack()
        if (framesToSkip == 1)
          framesToSkip = 0;
        const size_t maxStackWalk = InternalMaxStackWalk + framesToSkip;
        vector<void *> addresses(maxStackWalk);
        InternalWalkStack(strm, framesToSkip, addresses.data(), backtrace(addresses.data(), maxStackWalk));
      }
    }

    void PProcess::InternalWalkStackSignaled()
    {
      if (IsInitialised())
        s_otherThreadStack.OthersWalk();
    }

#else

  void PPlatformWalkStack(ostream & strm, PThreadIdentifier id, PUniqueThreadIdentifier uid, unsigned skip)
  {
  }

#endif // P_HAS_BACKTRACE


#if PTRACING
  #define OUTPUT_MESSAGE(msg) \
    PTRACE(0, NULL, "PTLib", msg); \
    if (PTrace::GetStream() != &PError) \
      PError << msg << endl
#else
  #define OUTPUT_MESSAGE(msg) PError << msg << endl
#endif


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
          if (system((const char *)cmd) < 0)
            PError << "Could not execute debugger!" << endl;
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


void PPlatformAssertFunc(const char * msg, char defaultAction)
{
  OUTPUT_MESSAGE(msg);

  // DO default action is specified
  if (defaultAction != '\0') {
    AssertAction(defaultAction, msg);
    return;
  }

  // Check for if stdin is not a TTY and just ignore the assert if so.
  if (isatty(STDIN_FILENO) != 1) {
    AssertAction('i', msg);
    return;
  }

  // Keep asking for action till we get a legal one
  do {
    PError << '\n' << ActionMessage << "? " << flush;
  } while (AssertAction(getchar(), msg));
}


// End Of File ///////////////////////////////////////////////////////////////
