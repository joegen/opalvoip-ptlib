/*
 * $Id: thread.h,v 1.2 1994/07/02 03:18:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.2  1994/07/02 03:18:09  robertj
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
    BOOL IsOnlyThread() const;
      // Return TRUE if is only thread in process

    virtual void SwitchContext(PThread * from);
      // Do the machinations needed to jump to the current thread


  private:
    void BeginThread();
      // Function to start Main() and exit when completed.


    // Member fields
    Priority basePriority;
      // Threads priority level, realtive to other threads.

    int dynamicPriority;
      // Threads priority during this scheduled slice.

    int suspendCount;
      // Threads count of calls to Suspend() or Resume(). If <=0 then can run,
      // if >0 means suspended and is not to be scheduled.

    PTimer sleepTimer;
      // Time for thread to remain asleep. Thread is not scheduled while this
      // is running after a Sleep() call.

    PThreadBlockFunction isBlocked;
      // Callback function to determine if the thread is blocked on I/O.

    PObject * blocker;
      // When thread is blocked on I/O this is the object to pass to isBlocked.

    PThread * link;
      // Link to next thread in circular list

    enum {
      Starting,
      Running,
      Waiting,
      Sleeping,
      Suspended,
      Blocked,
      Terminating,
      Terminated
    } status;
      // Thread status for scheduler handling

    jmp_buf context;
      // Buffer for context switching

    char NEAR * stackBase;
      // Base of stack allocated for the thread

    char NEAR * stackTop;
      // Top of stack allocated for the thread

    unsigned stackUsed;
      // High water mark for stack allocated for the thread
};


#ifndef _WINDLL

inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().

#endif


#endif
