/*
 * $Id: thread.h,v 1.3 1994/07/21 12:35:18 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.3  1994/07/21 12:35:18  robertj
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

#include <setjmp.h>

#if defined(_MSC_VER) && !defined(_JMP_BUF_DEFINED)

typedef int jmp_buf[9];
#define setjmp _setjmp
extern "C" int  __cdecl _setjmp(jmp_buf);
extern "C" void __cdecl longjmp(jmp_buf, int);

#endif


typedef BOOL (__far *PThreadBlockFunction)(PObject *);


///////////////////////////////////////////////////////////////////////////////
// PThread

#include "../../common/thread.h"
  public:
    void Block(PThreadBlockFunction isBlocked, PObject * obj);
      // Flag the thread as blocked. The scheduler will call the specified
      // function with the obj parameter to determine if the thread is to be
      // unblocked.

  protected:
    BOOL IsOnlyThread() const;
      // Return TRUE if is only thread in process


  private:
    // Member fields
    PThreadBlockFunction isBlocked;
      // Callback function to determine if the thread is blocked on I/O.

    PObject * blocker;
      // When thread is blocked on I/O this is the object to pass to isBlocked.

#ifdef _WINDOWS
    unsigned stackUsed;
      // High water mark for stack allocated for the thread
#endif
};


#ifndef _WINDLL

inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().

#endif


#endif
