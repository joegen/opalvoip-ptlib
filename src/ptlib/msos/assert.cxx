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

#define StackWalk(strm)

#else

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM thisProcess)
{
  char wndClassName[100];
  GetClassName(hWnd, wndClassName, sizeof(wndClassName));
  if (strcmp(wndClassName, "ConsoleWindowClass") != 0)
    return PTrue;

  DWORD wndProcess;
  GetWindowThreadProcessId(hWnd, &wndProcess);
  if (wndProcess != (DWORD)thisProcess)
    return PTrue;

  PTRACE(4, "PTLib\tAwaiting key press on exit.");
  cerr << "\nPress a key to continue . . .";
  cerr.flush();

  HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(in, ENABLE_PROCESSED_INPUT);
  FlushConsoleInputBuffer(in);
  char dummy;
  DWORD readBytes;
  ReadConsole(in, &dummy, 1, &readBytes, NULL);
  return PFalse;
}


void PProcess::WaitOnExitConsoleWindow()
{
  if (!m_library)
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
    BOOL (__stdcall *SymInitialize)(
      __in HANDLE hProcess,
      __in_opt PCSTR UserSearchPath,
      __in BOOL fInvadeProcess
    );
    BOOL (__stdcall *SymCleanup)(
      __in HANDLE hProcess
    );
    DWORD (__stdcall *SymGetOptions)(
      VOID
    );
    DWORD (__stdcall *SymSetOptions)(
      __in DWORD   SymOptions
    );
    BOOL (__stdcall *StackWalk64)(
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
    BOOL (__stdcall *SymGetSymFromAddr64)(
      __in HANDLE hProcess,
      __in DWORD64 qwAddr,
      __out_opt PDWORD64 pdwDisplacement,
      __inout PIMAGEHLP_SYMBOL64  Symbol
    );

    PFUNCTION_TABLE_ACCESS_ROUTINE64 SymFunctionTableAccess64;
    PGET_MODULE_BASE_ROUTINE64       SymGetModuleBase64;

    PDebugDLL()
      : PDynaLink("DBGHELP.DLL")
    {
    }

    ~PDebugDLL()
    {
      if (IsLoaded())
        SymCleanup(GetCurrentProcess());
    }

    void StackWalk(ostream & strm)
    {
      if (!GetFunction("SymInitialize", (Function &)SymInitialize) ||
          !GetFunction("SymCleanup", (Function &)SymCleanup) ||
          !GetFunction("SymGetOptions", (Function &)SymGetOptions) ||
          !GetFunction("SymSetOptions", (Function &)SymSetOptions) ||
          !GetFunction("StackWalk64", (Function &)StackWalk64) ||
          !GetFunction("SymGetSymFromAddr64", (Function &)SymGetSymFromAddr64) ||
          !GetFunction("SymFunctionTableAccess64", (Function &)SymFunctionTableAccess64) ||
          !GetFunction("SymGetModuleBase64", (Function &)SymGetModuleBase64) ||
          !SymInitialize(GetCurrentProcess(), NULL, TRUE)) {
        DWORD err = ::GetLastError();
        Close();
        strm << "\n    No stack dump: " << GetName() << " failed: error=" << err;
        return;
      }

      SymSetOptions(SymGetOptions()|SYMOPT_LOAD_LINES|SYMOPT_FAIL_CRITICAL_ERRORS|SYMOPT_NO_PROMPTS);

      // The thread information.
      CONTEXT threadContext;
      memset(&threadContext, 0, sizeof(threadContext));
      RtlCaptureContext(&threadContext);

      STACKFRAME64 frame;
      memset(&frame, 0, sizeof(frame));
      frame.AddrPC.Mode    = AddrModeFlat;
      frame.AddrStack.Mode = AddrModeFlat;
      frame.AddrFrame.Mode = AddrModeFlat;

      int frameCount = 0;
      while (frameCount++ < 16 &&
                        StackWalk64(IMAGE_FILE_MACHINE,
                                    GetCurrentProcess(),
                                    GetCurrentThread(),
                                    &frame,
                                    &threadContext,
                                    NULL,
                                    SymFunctionTableAccess64,
                                    SymGetModuleBase64,
                                    NULL) && frame.AddrPC.Offset != 0) {
        if (frameCount <= 1)
          continue;

        char buffer[sizeof(IMAGEHLP_SYMBOL64)+200];
        PIMAGEHLP_SYMBOL64 symbol = (PIMAGEHLP_SYMBOL64)buffer;
        symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL64);
        symbol->MaxNameLength = sizeof(buffer)-sizeof(IMAGEHLP_SYMBOL64);
        DWORD64 displacement = 0;
        strm << "\n    ";
        if (SymGetSymFromAddr64(GetCurrentProcess(), frame.AddrPC.Offset, &displacement, symbol))
          strm << symbol->Name;
        else
          strm << hex << setfill('0') << setw(8) << frame.AddrPC.Offset << dec << setfill(' ');
        strm << '(' << hex << setfill('0');
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
      }

      if (frameCount <= 2) {
        DWORD err = ::GetLastError();
        strm << "\n    No stack dump: " << GetName() << " StackWalk64 failed: error=" << err;
      }
    }
};


static void StackWalk(ostream & strm)
{
  PDebugDLL debughelp;
  if (debughelp.IsLoaded())
    debughelp.StackWalk(strm);
}

#endif // _WIN32_WCE

static PCriticalSection AssertMutex;

bool PAssertFunc(const char * msg)
{
  std::string str;
  {
    ostringstream strm;
    strm << msg;
    StackWalk(strm);
    strm << ends;
    str = strm.str();
  }

#ifndef _WIN32_WCE
  if (PProcess::Current().IsServiceProcess()) {
    PSYSTEMLOG(Fatal, str);
    if (PServiceProcess::Current().debugMode)
      PBreakToDebugger();;
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
