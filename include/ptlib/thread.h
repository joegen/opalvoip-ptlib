/*
 * $Id: thread.h,v 1.1 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.1  1994/06/25 11:55:15  robertj
 * Initial revision
 *
 */


#define _PTHREAD


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


// Class declaration continued in platform specific header file ///////////////
