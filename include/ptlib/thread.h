/*
 * $Id: thread.h,v 1.7 1994/08/23 11:32:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.7  1994/08/23 11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/08/21  23:43:02  robertj
 * Added SuspendBlock state to cooperative multi-threading to fix logic fault.
 *
 * Revision 1.4  1994/08/04  12:32:22  robertj
 * Better name of thread block check function.
 *
 * Revision 1.3  1994/07/21  12:33:49  robertj
 * Moved cooperative threads to common.
 *
 * Revision 1.2  1994/07/02  03:03:49  robertj
 * Added restartable threads.
 *
 * Revision 1.1  1994/06/25  11:55:15  robertj
 * Initial revision
 *
 */


#define _PTHREAD

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PThread

PDECLARE_CLASS(PThread, PObject)
  public:
    enum Priority {
      LowestPriority,   // Will only run if all other threads are blocked
      LowPriority,      // Runs approx. half as often as normal
      NormalPriority,
      HighPriority,     // Runs approx. twice as often as normal
      HighestPriority,  // Is only thread that will run, unless blocked
      NumPriorities
    };

    PThread(PINDEX stackSize,
            BOOL startSuspended = FALSE,
            Priority priorityLevel = NormalPriority);
      // Create a new thread instance.

    ~PThread();
      // Destroy the thread, this simply calls Terminate(). See that function
      // for notes on terminating threads.


    // New functions for class
    static PThread * Current();
      // Get the currently running thread object

    virtual void Main() = 0;
      // User override function for the main execution routine of the thread.

    virtual void Restart();
      // Restart a terminated thread using the same stack priority etc that
      // was current when the thread terminated.

    virtual void Terminate();
      // Terminate the thread. It is highly recommended that this is not used
      // except in abnormal abort situations as not all clean up of resources
      // allocated to the thread will be executed. This is especially true in
      // C++ as the destructors of objects that are automatic variables are not
      // called causing at the very least the possiblity of memory leaks. A
      // thread should ideally terminate by returning from the Main() function,
      // or self terminate by calling Terminate() within the context of the 
      // thread which can then assure that all resources are cleaned up.

    virtual BOOL IsTerminated() const;
      // Return TRUE if the thread has been terminated.

    virtual void Suspend(BOOL susp = TRUE);
      // Suspend the thread, this (if TRUE) increments an internal count of
      // suspensions that must be matched by an equal number of resumes before
      // the thread actually resumes execution.

    virtual void Resume();
      // Resume thread execution, this decrements an internal count of
      // suspensions. If the count is <= 0 then the thread will run. Note that
      // the thread will not be suspended until an equal number of Suspend()
      // calls are made.

    virtual BOOL IsSuspended() const;
      // Return TRUE if the thread is currently suspended.

    virtual void Sleep(const PTimeInterval & delay);
      // Suspend the current thread for an amount of time

    virtual void SetPriority(Priority priorityLevel);
      // Set the priority of the thread relative to other threads in the
      // current process.

    virtual Priority GetPriority() const;
      // Get the current priority of the thread.

    static void Yield();
      // Yield to another thread. If there are no other threads then this
      // function does nothing. Note that on most platforms the threading is
      // cooperative and this function must be called for other threads to run
      // at all.


  protected:
    PThread();
      // Create a new thread instance as part of a process

    void InitialiseProcessThread();
      // Initialialise the primordial thread (the one in the PProcess)


  private:
    PThread(const PThread &) { }
    PThread & operator=(const PThread &) { return *this; }

#ifndef P_PLATFORM_HAS_THREADS
    void AllocateStack(PINDEX stackSize);
      // Allocate the stack for the thread.

    void ClearBlock();
      // Clear the blocked thread.

    BOOL IsNoLongerBlocked();
      // Check if the thread is no longer blocked.

    void BeginThread();
      // Function to start Main() and exit when completed.

    virtual void SwitchContext(PThread * from);
      // Do the machinations needed to jump to the current thread


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

    PThread * link;
      // Link to next thread in circular list

    enum {
      Starting,
      Running,
      Waiting,
      Sleeping,
      Suspended,
      Blocked,
      SuspendedBlock,
      Terminating,
      Terminated
    } status;
      // Thread status for scheduler handling

    jmp_buf context;
      // Buffer for context switching

    char PSTATIC * stackBase;
      // Base of stack allocated for the thread, PSTATIC is for DOS-Windows.

    char PSTATIC * stackTop;
      // Top of stack allocated for the thread

#endif


// Class declaration continued in platform specific header file ///////////////
