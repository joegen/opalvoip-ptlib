/*
 * $Id: thread.h,v 1.1 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.1  1994/06/25 12:13:01  robertj
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

class PProcess;

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
    virtual void SwitchContext(PThread * from);
      // Do the machinations needed to jump to the current thread


  private:
    // Member fields
    Priority basePriority;
      // Threads priority level, this is fixed unless changed by application.

    int dynamicPriority;
      // Threads current relative priority. If 0 then can run on next schedule,
      // bit if >0 means must wait. It is decremented each time the thread has
      // a turn at being scheduled.

    int suspendCount;
      // Threads count of calls to Suspend() or Resume(). If <=0 then can run,
      // if >0 means suspended and is not to be scheduled.

    PTimeInterval wakeUpTime;
      // Time to wake up after a Sleep() call. If <PTimer::Tick() then can run,
      // otherwise is not scheduled.

    PThreadBlockFunction isBlocked;
      // Callback function to determine if the thread is blocked on I/O.

    PObject * blocker;
      // When thread is blocked on I/O this is the object to pass to isBlocked.

    PThread * link;
      // Next thread in schedule list

    enum { Starting, Running, Terminating, Terminated } status;
      // Thread status for scheduler handling

    char NEAR * stackBase;
      // Base of stack allocated for the thread

    char NEAR * stackTop;
      // Top of stack allocated for the thread

    unsigned stackUsed;
      // High water mark for stack allocated for the thread

    jmp_buf context;
      // Buffer for context switching
};



#ifndef _WINDLL

inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().

#endif


#endif
