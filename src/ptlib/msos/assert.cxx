/*
 * $Id: assert.cxx,v 1.8 1996/03/04 12:39:35 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
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
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include <ptlib.h>
#include <svcproc.h>

#include <errno.h>

#if defined(_WIN32)
#include <conio.h>
#ifdef _MSC_VER
#define GETCHAR _getch
#else
#define GETCHAR getch
#endif
#else
#define GETCHAR getchar
#endif


///////////////////////////////////////////////////////////////////////////////
// PProcess

#if defined(_WIN32)

static BOOL CALLBACK EnumWindowsProc(HWND hWnd, LPARAM thisProcess)
{
  PString wndClassName;
  GetClassName(hWnd, wndClassName.GetPointer(100), 100);
  if (wndClassName != "ConsoleWindowClass" && wndClassName != "tty")
    return TRUE;

  DWORD wndProcess;
  GetWindowThreadProcessId(hWnd, &wndProcess);
  if (wndProcess != (DWORD)thisProcess)
    return TRUE;

  fputs("\nPress any key to continue . . .\n", stderr);
  GETCHAR();
  return FALSE;
}


void PWaitOnExitConsoleWindow()
{
  EnumWindows(EnumWindowsProc, GetCurrentProcessId());
}

#endif


void PAssertFunc(const char * file, int line, const char * msg)
{
#if defined(_WIN32)
  DWORD err = GetLastError();
  static HANDLE mutex = CreateSemaphore(NULL, 1, 1, NULL);
  WaitForSingleObject(mutex, INFINITE);
#else
  int err = errno;
#endif

  if (PProcess::Current()->IsDescendant(PServiceProcess::Class()) &&
                              !((PServiceProcess*)PProcess::Current())->debugMode) {
    ((PServiceProcess*)PProcess::Current())->SystemLog(
              PServiceProcess::LogFatal,
              "Assertion fail in file %s, line %u %s - code = %lu",
              file, line, msg != NULL ? msg : "", err);
    _exit(100);
  }

  for (;;) {
    fprintf(stderr, "Assertion fail: File %s, Line %u\n", file, line);
    if (msg != NULL)
      fprintf(stderr, "%s - Error code=%u\n", msg, err);
    fputs("<A>bort, <B>reak, <I>gnore? ", stderr);
    switch (GETCHAR()) {
      case 'A' :
      case 'a' :
      case EOF :
        fputs("Aborted\n", stderr);
        _exit(100);
        
      case 'B' :
      case 'b' :
        fputs("Break\n", stderr);
#if defined(_WIN32)
        ReleaseSemaphore(mutex, 1, NULL);
#endif
#ifdef _MSC_VER
        __asm int 3;
#endif

      case 'I' :
      case 'i' :
        fputs("Ignored\n", stderr);
#if defined(_WIN32)
        ReleaseSemaphore(mutex, 1, NULL);
#endif
        return;
    }
  }
}


// End Of File ///////////////////////////////////////////////////////////////
