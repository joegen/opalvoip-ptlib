/*
 * $Id: assert.cxx,v 1.17 1997/02/05 11:49:40 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: assert.cxx,v $
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
// Revision 1.1  1994/04/01  14:39:35  robertj
// Initial revision
//
 */

#include <ptlib.h>
#include <svcproc.h>

#include <errno.h>


///////////////////////////////////////////////////////////////////////////////
// PProcess

#if defined(_WIN32)

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

#endif


void PAssertFunc(const char * file, int line, const char * msg)
{
#if defined(_WIN32)
  DWORD err = GetLastError();
#else
  int err = errno;
#endif

  if (PProcess::Current().IsDescendant(PServiceProcess::Class())) {
    if (msg == NULL)
      msg = "";
    static const char fmt[] = "Assertion fail in file %s, line %u %s - code = %lu";
    char * buf = new char[sizeof(fmt)+strlen(file)+strlen(msg)+20];
    sprintf(buf, "Assertion fail in file %s, line %u %s - code = %lu",
                 file, line, msg != NULL ? msg : "", err);
    PSystemLog::Output(PSystemLog::Fatal, buf);
    delete [] buf;
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
    cerr << "Assertion fail: File " << file << ", Line " << line << endl;
    if (msg != NULL)
      cerr << msg << " - Error code=" << err << endl;
    cerr << "<A>bort, <B>reak, <I>gnore? ";
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
