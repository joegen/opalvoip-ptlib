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
 * $Log: assert.cxx,v $
 * Revision 1.23  1998/12/04 10:10:45  robertj
 * Added virtual for determining if process is a service. Fixes linkage problem.
 *
 * Revision 1.22  1998/11/30 05:33:08  robertj
 * Fixed duplicate debug stream class, ther can be only one.
 *
 * Revision 1.21  1998/11/30 04:48:38  robertj
 * New directory structure
 *
 * Revision 1.20  1998/09/24 03:30:39  robertj
 * Added open software license.
 *
 * Revision 1.19  1997/03/18 21:22:31  robertj
 * Display error message if assert stack dump fails
 *
 * Revision 1.18  1997/02/09 01:27:18  robertj
 * Added stack dump under NT.
 *
 * Revision 1.17  1997/02/05 11:49:40  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.16  1997/01/04 06:52:04  robertj
 * Removed the press a key to continue under win  '95.
 *
 * Revision 1.15  1996/11/18 11:30:00  robertj
 * Removed int 3 on non-debug versions.
 *
 * Revision 1.14  1996/11/16 10:51:51  robertj
 * Changed assert to display message and break if in debug mode service.
 *
 * Revision 1.13  1996/11/10 21:02:08  robertj
 * Fixed bug in assertion when as a service, string buffer not big enough.
 *
 * Revision 1.12  1996/10/08 13:00:46  robertj
 * Changed default for assert to be ignore, not abort.
 *
 * Revision 1.11  1996/07/27 04:08:13  robertj
 * Changed SystemLog to be stream based rather than printf based.
 *
 * Revision 1.10  1996/05/30 11:48:28  robertj
 * Fixed press a key to continue to only require one key.
 *
 * Revision 1.9  1996/05/23 10:03:20  robertj
 * Windows 95 support.
 *
 * Revision 1.8  1996/03/04 12:39:35  robertj
 * Fixed Win95 support for console tasks.
 *
 * Revision 1.7  1996/01/28 14:13:04  robertj
 * Made PServiceProcess special case global not just WIN32.
 *
 * Revision 1.6  1995/12/10 11:55:09  robertj
 * Numerous fixes for WIN32 service processes.
 *
 * Revision 1.5  1995/04/25 11:32:34  robertj
 * Fixed Borland compiler warnings.
 *
 * Revision 1.4  1995/03/12 05:00:04  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.3  1994/10/30  11:25:09  robertj
 * Added error number to assert.
 *
 * Revision 1.2  1994/06/25  12:13:01  robertj
 * Synchronisation.
 *
 * Revision 1.1  1994/04/01  14:39:35  robertj
 * Initial revision
 */

#include <ptlib.h>
#include <ptlib/svcproc.h>
#include <ptlib/debstrm.h>

#include <errno.h>
#include <strstrea.h>


///////////////////////////////////////////////////////////////////////////////
// PProcess

#if defined(_WIN32)

#include <imagehlp.h>

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM thisProcess)
{
  char wndClassName[100];
  GetClassName(hWnd, wndClassName, sizeof(wndClassName));
  if (strcmp(wndClassName, "ConsoleWindowClass") != 0)
    return TRUE;

  DWORD wndProcess;
  GetWindowThreadProcessId(hWnd, &wndProcess);
  if (wndProcess != (DWORD)thisProcess)
    return TRUE;

  cerr << "\nPress a key to continue . . .";
  cerr.flush();

  HANDLE in = GetStdHandle(STD_INPUT_HANDLE);
  SetConsoleMode(in, ENABLE_PROCESSED_INPUT);
  FlushConsoleInputBuffer(in);
  char dummy;
  DWORD readBytes;
  ReadConsole(in, &dummy, 1, &readBytes, NULL);
  return FALSE;
}


void PWaitOnExitConsoleWindow()
{
  EnumWindows(EnumWindowsProc, GetCurrentProcessId());
}


PDECLARE_CLASS(PImageDLL, PDynaLink)
  public:
    PImageDLL();

  BOOL (__stdcall *SymInitialize)(
    IN HANDLE   hProcess,
    IN LPSTR    UserSearchPath,
    IN BOOL     fInvadeProcess
    );
  BOOL (__stdcall *SymCleanup)(
    IN HANDLE hProcess
    );
  BOOL (__stdcall *StackWalk)(
    DWORD                             MachineType,
    HANDLE                            hProcess,
    HANDLE                            hThread,
    LPSTACKFRAME                      StackFrame,
    LPVOID                            ContextRecord,
    PREAD_PROCESS_MEMORY_ROUTINE      ReadMemoryRoutine,
    PFUNCTION_TABLE_ACCESS_ROUTINE    FunctionTableAccessRoutine,
    PGET_MODULE_BASE_ROUTINE          GetModuleBaseRoutine,
    PTRANSLATE_ADDRESS_ROUTINE        TranslateAddress
    );
  BOOL (__stdcall *SymGetSymFromAddr)(
    IN  HANDLE              hProcess,
    IN  DWORD               dwAddr,
    OUT PDWORD              pdwDisplacement,
    OUT PIMAGEHLP_SYMBOL    Symbol
    );

  PFUNCTION_TABLE_ACCESS_ROUTINE SymFunctionTableAccess;
  PGET_MODULE_BASE_ROUTINE       SymGetModuleBase;
};


PImageDLL::PImageDLL()
  : PDynaLink("IMAGEHLP.DLL")
{
  if (!GetFunction("SymInitialize", (Function &)SymInitialize) ||
      !GetFunction("SymCleanup", (Function &)SymCleanup) ||
      !GetFunction("StackWalk", (Function &)StackWalk) ||
      !GetFunction("SymGetSymFromAddr", (Function &)SymGetSymFromAddr) ||
      !GetFunction("SymFunctionTableAccess", (Function &)SymFunctionTableAccess) ||
      !GetFunction("SymGetModuleBase", (Function &)SymGetModuleBase))
    Close();
}


///////////////////////////////////////////////////////////////////////////////
// PDebugStream

PDebugStream::PDebugStream()
  : ostream(&buffer)
{
}


PDebugStream::Buffer::Buffer()
{
  setb(buffer, &buffer[sizeof(buffer)-2]);
  unbuffered(FALSE);
  setp(base(), ebuf());
}


int PDebugStream::Buffer::overflow(int c)
{
  int bufSize = out_waiting();

  if (c != EOF) {
    *pptr() = (char)c;
    bufSize++;
  }

  if (bufSize != 0) {
    char * p = pbase();
    setp(p, epptr());
    p[bufSize] = '\0';
    OutputDebugString(p);
  }

  return 0;
}


int PDebugStream::Buffer::underflow()
{
  return EOF;
}


int PDebugStream::Buffer::sync()
{
  return overflow(EOF);
}


#endif


void PAssertFunc(const char * file, int line, const char * msg)
{
#if defined(_WIN32)
  DWORD err = GetLastError();
#else
  int err = errno;
#endif

  ostrstream str;
  str << "Assertion fail: ";
  if (msg != NULL)
    str << msg << ", ";
  str << "file " << file << ", line " << line;
  if (err != 0)
    str << ", Error=" << err;

#if defined(_WIN32) && defined(_M_IX86)
  PImageDLL imagehlp;
  if (imagehlp.IsLoaded()) {
    HANDLE hProcess = GetCurrentProcess();
    if (imagehlp.SymInitialize(hProcess, NULL, TRUE)) {
      STACKFRAME frame;
      memset(&frame, 0, sizeof(frame));
      frame.AddrPC.Mode = AddrModeFlat;
      frame.AddrFrame.Mode = AddrModeFlat;

      __asm {
        call $+5
        pop frame.AddrPC.Offset
        mov frame.AddrFrame.Offset,ebp
      }

      int frameCount = 0;
      while (frameCount++ < 16 &&
             imagehlp.StackWalk(IMAGE_FILE_MACHINE_I386,
                                hProcess,
                                GetCurrentThread(),
                                &frame,
                                NULL, // Context
                                NULL, // ReadMemoryRoutine
                                imagehlp.SymFunctionTableAccess,
                                imagehlp.SymGetModuleBase,
                                 NULL)) {
        if (frameCount > 1 && frame.AddrPC.Offset != 0) {
          char buffer[sizeof(IMAGEHLP_SYMBOL)+100];
          PIMAGEHLP_SYMBOL symbol = (PIMAGEHLP_SYMBOL)buffer;
          symbol->SizeOfStruct = sizeof(IMAGEHLP_SYMBOL);
          symbol->MaxNameLength = sizeof(buffer)-sizeof(IMAGEHLP_SYMBOL);
          DWORD displacement = 0;
          if (imagehlp.SymGetSymFromAddr(hProcess,
                                         frame.AddrPC.Offset,
                                         &displacement,
                                         symbol)) {
            str << "\n    " << symbol->Name;
          }
          else {
            str << "\n    0x"
                << hex << setfill('0')
                << setw(8) << frame.AddrPC.Offset
                << dec << setfill(' ');
          }
          str << '(' << hex << setfill('0');
          for (PINDEX i = 0; i < PARRAYSIZE(frame.Params); i++) {
            if (i > 0)
              str << ", ";
            if (frame.Params[i] != 0)
              str << "0x";
            str << frame.Params[i];
          }
          str << setfill(' ') << ')';
          if (displacement != 0)
            str << " + 0x" << displacement;
        }
      }

      imagehlp.SymCleanup(hProcess);
    }
    else
      str << "\n    No stack dump: IMAGEHLP.DLL SymInitialise failed.";
  }
  else
    str << "\n    No stack dump: IMAGEHLP.DLL could not be loaded.";
#endif

  str << ends;

  if (PProcess::Current().IsServiceProcess()) {
    PSystemLog::Output(PSystemLog::Fatal, str.str());
#if defined(_MSC_VER) && defined(_DEBUG)
    if (PServiceProcess::Current().debugMode)
      __asm int 3;
#endif
    return;
  }

#if defined(_WIN32)
  static HANDLE mutex = CreateSemaphore(NULL, 1, 1, NULL);
  WaitForSingleObject(mutex, INFINITE);
#endif

  for (;;) {
    cerr << str.str() << "\n<A>bort, <B>reak, <I>gnore? ";
    cerr.flush();
    switch (cin.get()) {
      case 'A' :
      case 'a' :
        cerr << "Aborted" << endl;
        _exit(100);
        
      case 'B' :
      case 'b' :
        cerr << "Break" << endl;
#if defined(_WIN32)
        ReleaseSemaphore(mutex, 1, NULL);
#endif
#ifdef _MSC_VER
        __asm int 3;
#endif

      case 'I' :
      case 'i' :
      case EOF :
        cerr << "Ignored" << endl;
#if defined(_WIN32)
        ReleaseSemaphore(mutex, 1, NULL);
#endif
        return;
    }
  }
}


// End Of File ///////////////////////////////////////////////////////////////
