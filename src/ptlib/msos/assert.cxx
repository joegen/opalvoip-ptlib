/*
 * $Id: assert.cxx,v 1.4 1995/03/12 05:00:04 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
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

#include "ptlib.h"
#include <errno.h>

#if defined(_WIN32)
#include <conio.h>
#define GETCHAR _getch
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
  if (wndClassName != "ConsoleWindowClass")
    return TRUE;

  DWORD wndProcess;
  GetWindowThreadProcessId(hWnd, &wndProcess);
  if (wndProcess != (DWORD)thisProcess)
    return TRUE;

  cerr << "\nPress any key to continue . . .\n";
  _getch();
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
#else
  int err = errno;
#endif
  for (;;) {
    cerr << "Assertion fail: File " << file << ", Line " << line << endl;
    if (msg != NULL)
      cerr << msg << " - Error code=" << err << endl;
    cerr << "<A>bort, <B>reak, <I>gnore? ";
    cerr.flush();
    switch (GETCHAR()) {
      case 'A' :
      case 'a' :
      case EOF :
        cerr << "Aborted\n";
        _exit(100);
        
      case 'B' :
      case 'b' :
        cerr << "Break\n";
        __asm int 3;
        
      case 'I' :
      case 'i' :
        cerr << "Ignored\n";
        return;
    }
  }
}


// End Of File ///////////////////////////////////////////////////////////////
