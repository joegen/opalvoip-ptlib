/*
 * $Id: thread.h,v 1.5 1995/03/12 05:00:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
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

#include "../../common/thread.h"
#if defined(P_PLATFORM_HAS_THREADS)
  private:
    HANDLE handle;
    DWORD  id;
    PINDEX originalStackSize;

    PDICTIONARY(ThreadDict, POrdinalKey, PThread);
    static ThreadDict threads;

    static DWORD EXPORTED MainFunction(LPVOID thread);
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


#ifndef _WINDLL

inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().

#endif


#endif
