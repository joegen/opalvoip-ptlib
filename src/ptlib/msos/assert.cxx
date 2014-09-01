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
 *
 * $Revision$
 * $Author$
 * $Date$
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

  PTRACE(4, "PTLib\tAwaiting key press on exit.");
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

#include <DbgHelp.h>

#if defined (_M_IX86)
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_I386
#elif _M_X64
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_AMD64
#elif _M_IA64
#define IMAGE_FILE_MACHINE IMAGE_FILE_MACHINE_IA64
#else
#error ( "Unknown machine!" )
#endif

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

    PFUNCTION_TABLE_ACCESS_ROUTINE64 m_SymFunctionTableAccess64;
    PGET_MODULE_BASE_ROUTINE64       m_SymGetModuleBase64;

    PDebugDLL()
      : PDynaLink("DBGHELP.DLL")
    {
    }

    ~PDebugDLL()
    {
      if (IsLoaded())
        m_SymCleanup(GetCurrentProcess());
    }

    void StackWalk(ostream & strm, PThreadIdentifier id)
    {
      if (!GetFunction("SymInitialize", (Function &)m_SymInitialize) ||
          !GetFunction("SymCleanup", (Function &)m_SymCleanup) ||
          !GetFunction("SymGetOptions", (Function &)m_SymGetOptions) ||
          !GetFunction("SymSetOptions", (Function &)m_SymSetOptions) ||
          !GetFunction("StackWalk64", (Function &)m_StackWalk64) ||
          !GetFunction("SymGetSymFromAddr64", (Function &)m_SymGetSymFromAddr64) ||
          !GetFunction("SymFunctionTableAccess64", (Function &)m_SymFunctionTableAccess64) ||
          !GetFunction("SymGetModuleBase64", (Function &)m_SymGetModuleBase64) ||
          !m_SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        DWORD err = ::GetLastError();
        strm << "\n    Invalid stack walk DLL: " << GetName() << " failed: error=" << err;
        return;
      }

      m_SymSetOptions(m_SymGetOptions()|SYMOPT_LOAD_LINES|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_NO_PROMPTS);

      // The thread information.
      HANDLE hThread;
      int framesToSkip = 0;
      int resumeCount = -1;

      CONTEXT threadContext;
      memset(&threadContext, 0, sizeof(threadContext));
      threadContext.ContextFlags = CONTEXT_FULL;

      if (id == PNullThreadIdentifier || id == GetCurrentThreadId()) {
        hThread = GetCurrentThread();
        RtlCaptureContext(&threadContext);
        framesToSkip = 2;
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

      int frameCount = 0;
      while (frameCount++ < PTrace::MaxStackWalk) {
        if (!m_StackWalk64(imageType,
                           GetCurrentProcess(),
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

        strm << "\n    " << hex << setfill('0');

        char buffer[sizeof(IMAGEHLP_SYMBOL64)+200];
        PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
        symbol->MaxNameLength = sizeof(buffer)-sizeof(IMAGEHLP_SYMBOL64);
        DWORD64 displacement = 0;
        if (m_SymGetSymFromAddr64(GetCurrentProcess(), frame.AddrPC.Offset, &displacement, symbol))
          strm << symbol->Name;
        else
          strm << setw(8) << frame.AddrPC.Offset;
        strm << '(';
        for (PINDEX i = 0; i < PARRAYSIZE(frame.Params); i++) {
          if (i > 0)
            strm << ", ";
          if (frame.Params[i] != 0)
            strm << "0x";
          strm << frame.Params[i];
        }
        strm << setfill(' ') << ')';
        if (displacement != 0)
          strm << " + 0x" << displacement;

        strm << dec << setfill(' ');

        if (frame.AddrReturn.Offset == 0)
          break;
      }

      if (resumeCount >= 0)
        ResumeThread(hThread);
    }
};


static PCriticalSection StackWalkMutex;

static void InternalStackWalk(ostream & strm, PThreadIdentifier id)
{
  StackWalkMutex.Wait();
  PDebugDLL debughelp;
  if (debughelp.IsLoaded())
    debughelp.StackWalk(strm, id);
  StackWalkMutex.Signal();
}

#endif // _WIN32_WCE


#if PTRACING
void PTrace::WalkStack(ostream & strm, PThreadIdentifier id)
{
  InternalStackWalk(strm, id);
}
#endif // PTRACING


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

#ifndef _WIN32_WCE
  if (PProcess::Current().IsServiceProcess()) {
    PSYSTEMLOG(Fatal, str);
    return false;
  }
#endif // !_WIN32_WCE

  PTRACE(0, str);

  PWaitAndSignal mutex(AssertMutex);

  if (PProcess::Current().IsGUIProcess()) {
    PVarString msg = str;
    PVarString name = PProcess::Current().GetName();
    switch (MessageBox(NULL, msg, name, MB_ABORTRETRYIGNORE|MB_ICONHAND|MB_TASKMODAL)) {
      case IDABORT :
	_exit(100); // Never returns

      case IDRETRY :
        PBreakToDebugger();
    }
    return false;
  }

  for (;;) {
    cerr << str << "\n<A>bort, <B>reak, <I>gnore? ";
    cerr.flush();
    switch (cin.get()) {
      case 'A' :
      case 'a' :
        cerr << "Aborted" << endl;
        _exit(100); // Never returns

      case 'B' :
      case 'b' :
        cerr << "Break" << endl;
        PBreakToDebugger();
        return false; // Then ignore it

      case 'I' :
      case 'i' :
        cerr << "Ignored" << endl;

      case EOF :
        return false;
    }
  }
}


// End Of File ///////////////////////////////////////////////////////////////
