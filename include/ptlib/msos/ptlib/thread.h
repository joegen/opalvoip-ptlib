/*
 * $Id: thread.h,v 1.14 1996/08/17 10:00:36 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.14  1996/08/17 10:00:36  robertj
 * Changes for Windows DLL support.
 *
 * Revision 1.13  1996/08/08 10:09:19  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.12  1996/07/27 04:08:34  robertj
 * Changed thread creation to use C library function instead of direct WIN32.
 *
 * Revision 1.11  1996/06/13 13:32:12  robertj
 * Rewrite of auto-delete threads, fixes Windows95 total crash.
 *
 * Revision 1.10  1996/03/31 09:08:42  robertj
 * Added mutex to thread dictionary access.
 *
 * Revision 1.9  1995/12/10 11:48:54  robertj
 * Fixed bug in application shutdown of child threads.
 *
 * Revision 1.8  1995/08/24 12:38:36  robertj
 * Added extra conditional compile for WIN32 code.
 *
 * Revision 1.7  1995/07/02 01:23:51  robertj
 * Allowed access to thread info to descendents.
 *
 * Revision 1.6  1995/04/25 11:19:53  robertj
 * Fixes for DLL use in WIN32.
 *
 * Revision 1.5  1995/03/12 05:00:02  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.4  1994/07/27  06:00:10  robertj
 * Backup
 *
 * Revision 1.3  1994/07/21  12:35:18  robertj
 * *** empty log message ***
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
 * Multi-threading implementation.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 */


#ifndef _PTHREAD

#if defined(_WIN32)

#define P_PLATFORM_HAS_THREADS
#undef Yield

#else

#include <malloc.h>
#include <setjmp.h>

#if defined(_MSC_VER) && !defined(_JMP_BUF_DEFINED)

typedef int jmp_buf[9];
#define setjmp _setjmp
extern "C" int  __cdecl _setjmp(jmp_buf);
extern "C" void __cdecl longjmp(jmp_buf, int);

#endif

#endif


///////////////////////////////////////////////////////////////////////////////
// PThread

#include "../../common/ptlib/thread.h"
#if defined(P_PLATFORM_HAS_THREADS)
  public:
    HANDLE GetHandle() const { return threadHandle; }
    DWORD CleanUpOnTerminated();

  protected:
    HANDLE threadHandle;
    UINT   threadId;

  private:
    PINDEX originalStackSize;

    static UINT __stdcall MainFunction(void * thread);
#else
  public:
    typedef BOOL (__far *BlockFunction)(PObject *);
    void Block(BlockFunction isBlocked, PObject * obj);
      // Flag the thread as blocked. The scheduler will call the specified
      // function with the obj parameter to determine if the thread is to be
      // unblocked.

  protected:
    BOOL IsOnlyThread() const;
      // Return TRUE if is only thread in process


  private:
    // Member fields
    BlockFunction isBlocked;
      // Callback function to determine if the thread is blocked on I/O.

    PObject * blocker;
      // When thread is blocked on I/O this is the object to pass to isBlocked.

#ifdef _WINDOWS
    unsigned stackUsed;
      // High water mark for stack allocated for the thread
#endif
#endif
};


inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().


#endif
