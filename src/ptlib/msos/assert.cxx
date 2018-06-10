/*
 * assert.cxx
 *
 * Function to implement assert clauses.
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

#define P_DISABLE_FACTORY_INSTANCES

#include <ptlib.h>
#include <ptlib/svcproc.h>


///////////////////////////////////////////////////////////////////////////////
// PProcess

#ifdef _WIN32_WCE

  void PProcess::WaitOnExitConsoleWindow()
  {
  }

  #define InternalStackWalk(strm, id)

  #elif defined(__MINGW32__)

  void PProcess::WaitOnExitConsoleWindow()
  {
  }

  #define InternalStackWalk(strm, id)

  #if PTRACING
    void PTrace::WalkStack(ostream &, PThreadIdentifier)
    {
    }
  #endif // PTRACING

#else

  static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM thisProcess)
  {
    char wndClassName[100];
    GetClassName(hWnd, wndClassName, sizeof(wndClassName));
    if (strcmp(wndClassName, "ConsoleWindowClass") != 0)
      return true;

    DWORD wndProcess;
    GetWindowThreadProcessId(hWnd, &wndProcess);
    if (wndProcess != (DWORD)thisProcess)
      return true;

    PTRACE(2, "PTLib", "Awaiting key press on exit.");
    cerr << "\nPress a key to exit . . .";
    cerr.flush();

    HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
    SetConsoleMode(in, ENABLE_PROCESSED_INPUT);
    FlushConsoleInputBuffer(in);
    char dummy;
    DWORD readBytes;
    ReadConsole(in, &dummy, 1, &readBytes, NULL);
    return false;
  }


  void PProcess::WaitOnExitConsoleWindow()
  {
    if (m_waitOnExitConsoleWindow && !m_library)
      EnumWindows(EnumWindowsProc, GetCurrentProcessId());
  }

  #pragma warning(disable:4091)
  #include <DbgHelp.h>
  #include <Psapi.h>
  #pragma warning(default:4091)

  #if PTRACING
    #define InternalMaxStackWalk PTrace::MaxStackWalk
  #else
    #define InternalMaxStackWalk 20
  #endif // PTRACING

  class PDebugDLL : public PDynaLink
  {
    PCLASSINFO(PDebugDLL, PDynaLink)
    public:
      BOOL (__stdcall *m_SymInitialize)(
        __in HANDLE hProcess,
        __in_opt PCSTR UserSearchPath,
        __in BOOL fInvadeProcess
      );
      BOOL (__stdcall *m_SymCleanup)(
        __in HANDLE hProcess
      );
      DWORD (__stdcall *m_SymGetOptions)(
        VOID
      );
      DWORD (__stdcall *m_SymSetOptions)(
        __in DWORD   SymOptions
      );
      BOOL (__stdcall *m_StackWalk64)(
        __in DWORD MachineType,
        __in HANDLE hProcess,
        __in HANDLE hThread,
        __inout LPSTACKFRAME64 StackFrame,
        __inout PVOID ContextRecord,
        __in_opt PREAD_PROCESS_MEMORY_ROUTINE64 ReadMemoryRoutine,
        __in_opt PFUNCTION_TABLE_ACCESS_ROUTINE64 FunctionTableAccessRoutine,
        __in_opt PGET_MODULE_BASE_ROUTINE64 GetModuleBaseRoutine,
        __in_opt PTRANSLATE_ADDRESS_ROUTINE64 TranslateAddress
      );
      BOOL (__stdcall *m_SymGetSymFromAddr64)(
        __in HANDLE hProcess,
        __in DWORD64 qwAddr,
        __out_opt PDWORD64 pdwDisplacement,
        __inout PIMAGEHLP_SYMBOL64  Symbol
      );
      BOOL (__stdcall *m_SymGetLineFromAddr64)(
        _In_  HANDLE           hProcess,
        _In_  DWORD64          dwAddr,
        _Out_ PDWORD           pdwDisplacement,
        _Out_ PIMAGEHLP_LINE64 Line
      );

      PFUNCTION_TABLE_ACCESS_ROUTINE64 m_SymFunctionTableAccess64;
      PGET_MODULE_BASE_ROUTINE64       m_SymGetModuleBase64;

      HANDLE m_hProcess;


      PDebugDLL()
        : PDynaLink("DBGHELP.DLL")
        , m_hProcess(GetCurrentProcess())
      {
      }

      ~PDebugDLL()
      {
        if (IsLoaded())
          m_SymCleanup(GetCurrentProcess());
      }

      bool Initialise(ostream & strm)
      {
        if (!GetFunction("SymInitialize", (Function &)m_SymInitialize) ||
            !GetFunction("SymCleanup", (Function &)m_SymCleanup) ||
            !GetFunction("SymGetOptions", (Function &)m_SymGetOptions) ||
            !GetFunction("SymSetOptions", (Function &)m_SymSetOptions) ||
            !GetFunction("StackWalk64", (Function &)m_StackWalk64) ||
            !GetFunction("SymGetSymFromAddr64", (Function &)m_SymGetSymFromAddr64) ||
            !GetFunction("SymFunctionTableAccess64", (Function &)m_SymFunctionTableAccess64) ||
            !GetFunction("SymGetModuleBase64", (Function &)m_SymGetModuleBase64)) {
          strm << "\n    Invalid stack walk DLL: " << GetName() << " not all functions present.";
          return false;
        }

        if (!GetFunction("SymGetLineFromAddr64", (Function &)m_SymGetLineFromAddr64))
          m_SymGetLineFromAddr64 = NULL;

        // Get the directory the .exe file is in
        char filename[_MAX_PATH];
        if (GetModuleFileNameEx(m_hProcess, NULL, filename, sizeof(filename)) == 0) {
          DWORD err = ::GetLastError();
          strm << "\n    GetModuleFileNameEx failed, error=" << err;
          return false;
        }

        ostringstream path;

        // Always look in same place as the .exe file for .pdb file
        char * ptr = strrchr(filename, '\\');
        if (ptr == NULL)
          path << filename;
        else {
          *ptr = '\0';
          path << filename;
          *ptr = '\\';
        }

        /* Add in the environment variables as per default, but do not add in
           current directory (as default does), as if that is C:\, it searches
           the entire disk. Slooooow. */
        const char * env;
        if ((env = getenv("_NT_SYMBOL_PATH")) != NULL)
          path << ';' << env;
        if ((env = getenv("_NT_ALTERNATE_SYMBOL_PATH")) != NULL)
          path << ';' << env;

        // Initialise the symbols with path for PDB files.
        if (!m_SymInitialize(m_hProcess, path.str().c_str(), TRUE)) {
          DWORD err = ::GetLastError();
          strm << "\n    SymInitialize failed, error=" << err;
          return false;
        }

        strm << "\n    Stack walk symbols initialised, path=\"" << path.str() << '"';

        // See if PDB file exists
        ptr = strrchr(filename, '.');
        if (ptr == NULL)
          strcat(filename, ".pdb");
        else
          strcpy(ptr, ".pdb");

        if (_access(filename, 4) != 0)
          strm << "\n    Stack walk could not find symbols file \"" << filename << '"';

        m_SymSetOptions(m_SymGetOptions()|SYMOPT_LOAD_LINES|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_NO_PROMPTS);
        return true;
      }

      void OutputSymbol(ostream & strm, DWORD64 addrPC, DWORD64 * params = NULL)
      {
        strm << "\n    " << hex << setfill('0');

        char buffer[sizeof(IMAGEHLP_SYMBOL64) + 200];
        PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
        symbol->MaxNameLength = sizeof(buffer) - sizeof(IMAGEHLP_SYMBOL64);
        DWORD64 displacement = 0;
        DWORD error = 0;
        if (m_SymGetSymFromAddr64(m_hProcess, addrPC, &displacement, symbol))
          strm << symbol->Name;
        else {
          error = ::GetLastError();
          strm << setw(8) << addrPC;
        }

        if (params) {
          strm << '(';
          for (PINDEX i = 0; i < 4; i++) {
            if (i > 0)
              strm << ", ";
            if (params[i] != 0)
              strm << "0x";
            strm << params[i];
          }
          strm << setfill(' ') << ')';
        }

        if (displacement != 0)
          strm << " + 0x" << displacement;

        strm << dec << setfill(' ');

        IMAGEHLP_LINE64 line;
        line.SizeOfStruct = sizeof(line);
        DWORD dwDisplacement;
        if (m_SymGetLineFromAddr64 != NULL && m_SymGetLineFromAddr64(m_hProcess, addrPC, &dwDisplacement, &line))
          strm << ' ' << line.FileName << '(' << line.LineNumber << ')';

        if (error != 0)
          strm << " - symbol lookup error=" << error;
      }

      void WalkStack(ostream & strm, PThreadIdentifier id, unsigned framesToSkip)
      {
        if (!Initialise(strm))
          return;

        // The thread information.
        HANDLE hThread;
        int resumeCount = -1;

        CONTEXT threadContext;
        memset(&threadContext, 0, sizeof(threadContext));
        threadContext.ContextFlags = CONTEXT_FULL;

        if (id == PNullThreadIdentifier || id == GetCurrentThreadId()) {
          hThread = GetCurrentThread();
          RtlCaptureContext(&threadContext);
#if P_64BIT
          framesToSkip += 2;
#else
          ++framesToSkip;
#endif
        }
        else {
          hThread = OpenThread(THREAD_QUERY_INFORMATION|THREAD_GET_CONTEXT|THREAD_SUSPEND_RESUME, FALSE, id);
          if (hThread == NULL) {
            DWORD err = ::GetLastError();
            strm << "\n    No thread: id=" << id << " (0x" << std::hex << id << std::dec << "), error=" << err;
            return;
          }
          resumeCount = SuspendThread(hThread);
          if (!GetThreadContext(hThread, &threadContext)) {
            if (resumeCount >= 0)
              ResumeThread(hThread);
            DWORD err = ::GetLastError();
            strm << "\n    No context for thread: id=" << id << " (0x" << std::hex << id << std::dec << "), error=" << err;
            return;
          }
        }

        STACKFRAME64 frame;
        memset(&frame, 0, sizeof(frame));
        frame.AddrPC.Mode    = AddrModeFlat;
        frame.AddrStack.Mode = AddrModeFlat;
        frame.AddrFrame.Mode = AddrModeFlat;
        #ifdef _M_IX86
          // normally, call ImageNtHeader() and use machine info from PE header
          DWORD imageType = IMAGE_FILE_MACHINE_I386;
          frame.AddrPC.Offset = threadContext.Eip;
          frame.AddrFrame.Offset = frame.AddrStack.Offset = threadContext.Esp;
        #elif _M_X64
          DWORD imageType = IMAGE_FILE_MACHINE_AMD64;
          frame.AddrPC.Offset = threadContext.Rip;
          frame.AddrFrame.Offset = frame.AddrStack.Offset = threadContext.Rsp;
        #elif _M_IA64
          DWORD imageType = IMAGE_FILE_MACHINE_IA64;
          frame.AddrPC.Offset = threadContext.StIIP;
          frame.AddrFrame.Offset = threadContext.IntSp;
          frame.AddrBStore.Offset = threadContext.RsBSP;
          frame.AddrBStore.Mode = AddrModeFlat;
          frame.AddrStack.Offset = threadContext.IntSp;
        #else
          #error "Platform not supported!"
        #endif

        unsigned frameCount = 0;
        while (frameCount++ < InternalMaxStackWalk) {
          if (!m_StackWalk64(imageType,
                             m_hProcess,
                             hThread,
                             &frame,
                             &threadContext,
                             NULL,
                             m_SymFunctionTableAccess64,
                             m_SymGetModuleBase64,
                             NULL)) {
            DWORD err = ::GetLastError();
            strm << "\n    StackWalk64 failed: error=" << err;
            break;
          }

          if (frame.AddrPC.Offset == frame.AddrReturn.Offset) {
            strm << "\n    StackWalk64 returned recursive stack! PC=0x" << hex << frame.AddrPC.Offset << dec;
            break;
          }

          if (frameCount <= framesToSkip || frame.AddrPC.Offset == 0)
            continue;

          OutputSymbol(strm, frame.AddrReturn.Offset, frame.Params);

          if (frame.AddrReturn.Offset == 0)
            break;
        }

        if (resumeCount >= 0)
          ResumeThread(hThread);
      }
  };


  void PPlatformWalkStack(ostream & strm, PThreadIdentifier id, PUniqueThreadIdentifier, unsigned framesToSkip, bool)
  {
    PDebugDLL debughelp;
    if (debughelp.IsLoaded())
      debughelp.WalkStack(strm, id, framesToSkip);
  }

#endif // _WIN32_WCE


  static const char ActionMessage[] = "<A>bort, <B>reak, "
  #if P_EXCEPTIONS
                                      "<T>hrow exception, "
  #endif
                                      "<I>gnore";

static bool AssertAction(int c, const char * msg)
{
  switch (c) {
    case 'A':
    case 'a':
      cerr << "Aborted" << endl;
    case IDABORT :
      PTRACE(0, "Assert abort, application exiting immediately");
      abort(); // Never returns

    case 'B':
    case 'b':
      cerr << "Break" << endl;
    case IDRETRY :
      PBreakToDebugger();
      return false; // Then ignore it

  #if P_EXCEPTIONS
      case 't' :
      case 'T' :
        PError << "\nThrowing exception.\n";
        throw std::runtime_error(msg);
  #endif

    case 'I':
    case 'i':
      cerr << "Ignored" << endl;

    case IDIGNORE :
    case EOF:
      return false;
  }

  return true;
}


void PPlatformAssertFunc(const PDebugLocation & PTRACE_PARAM(location), const char * msg, char defaultAction)
{
#ifndef _WIN32_WCE
  if (PProcess::Current().IsServiceProcess()) {
    PSYSTEMLOG(Fatal, msg);
    return;
  }
#endif // !_WIN32_WCE

#if PTRACING
  PTrace::Begin(0, location.m_file, location.m_line, NULL, "PAssert") << msg << PTrace::End;
#endif

  if (defaultAction != '\0' && !AssertAction(defaultAction, msg))
      return;

  else if (PProcess::Current().IsGUIProcess()) {
    PVarString boxMsg = msg;
    PVarString boxTitle = PProcess::Current().GetName();
    AssertAction(MessageBox(NULL, boxMsg, boxTitle, MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_TASKMODAL), msg);
  }
  else {
    do {
      cerr << msg << '\n' << ActionMessage << "? " << flush;
    } while (AssertAction(cin.get(), msg));
  }
}


LONG WINAPI PExceptionHandler(PEXCEPTION_POINTERS info)
{
  ostringstream strm;
  switch (info->ExceptionRecord->ExceptionCode) {
    case EXCEPTION_ACCESS_VIOLATION:
      strm << "Access violation: " << (info->ExceptionRecord->ExceptionInformation[0] ? "Write" : "Read")
           << " at " << (void *)(info->ExceptionRecord->ExceptionInformation[1]);
      break;
    case EXCEPTION_IN_PAGE_ERROR:
      strm << "In page error (" << info->ExceptionRecord->ExceptionInformation[2] << ")"
              ": " << (info->ExceptionRecord->ExceptionInformation[0] ? "Write" : "Read")
           << " at " << (void *)(info->ExceptionRecord->ExceptionInformation[1]);
      break;
    case EXCEPTION_STACK_OVERFLOW :
      strm << "Stack overflow";
      break;
    case EXCEPTION_FLT_DIVIDE_BY_ZERO :
    case EXCEPTION_INT_DIVIDE_BY_ZERO :
      strm << "Divide by zero";
      break;
    default:
      strm << "Unhandled exception: code=0x" << hex << info->ExceptionRecord->ExceptionCode << dec;
  }

  strm << ", when=" << PTime().AsString(PTime::LoggingFormat);

  PDebugDLL debughelp;
  if (debughelp.Initialise(strm)) {
#ifdef _M_IX86
    // normally, call ImageNtHeader() and use machine info from PE header
    debughelp.OutputSymbol(strm, info->ContextRecord->Eip);
#elif _M_X64
    debughelp.OutputSymbol(strm, info->ContextRecord->Rip);
#elif _M_IA64
    debughelp.OutputSymbol(strm, info->ContextRecord->StIIP);
#endif
  }

  string msg = strm.str();

  if (PProcess::Current().IsServiceProcess())
    PSYSTEMLOG(Fatal, msg);
  else {
    if (PProcess::Current().IsGUIProcess()) {
      PVarString boxMsg = msg;
      PVarString boxTitle = PProcess::Current().GetName();
      MessageBox(NULL, boxMsg, "Exception", MB_OK | MB_ICONHAND | MB_TASKMODAL);
    }
    else {
      cerr << msg << endl;
#if PTRACING
      if (PTrace::GetStream() != &PError)
        PTrace::Begin(0, NULL, 0, NULL, "PException") << msg << PTrace::End;
#endif
      PProcess::Current().WaitOnExitConsoleWindow();
    }
    ExitProcess(1);
  }

  return EXCEPTION_CONTINUE_SEARCH;
}


// End Of File ///////////////////////////////////////////////////////////////
