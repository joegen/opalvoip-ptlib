/*
 * $Id: thread.h,v 1.13 1995/11/21 11:49:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.13  1995/11/21 11:49:44  robertj
 * Added timeout on semaphore wait.
 *
 * Revision 1.12  1995/07/31 12:10:40  robertj
 * Added semaphore class.
 *
 * Revision 1.11  1995/06/17 11:13:35  robertj
 * Documentation update.
 *
 * Revision 1.10  1995/03/14 12:42:49  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.9  1995/01/16  09:42:13  robertj
 * Documentation.
 *
 * Revision 1.8  1994/09/25  10:45:22  robertj
 * Virtualised IsNoLongerBlocked for unix platform.
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
/* This class defines a thread of execution in the system. A <I>thread</I> is
   an independent flow of processor instructions. This differs from a
   <I>process</I> which also embodies a program address space and resource
   allocation. So threads can share memory and resources as they run in the
   context of a given process. A process always contains at least one thread.
   This is reflected in this library by the <A>PProcess</A> class being
   descended from the PThread class.

   The implementation of a thread is platform dependent. Not all platforms
   support concurrent threads within a process or even concurrent processes!
   For example, MS-DOS has no form of multi-threading or multi-processing,
   Microsoft Windows has a cooperative multi-processing but no multi-threading.
   Unix has full pre-emptive multi-processing but most cannot do multiple
   threads within that process while some Unix systems and Windows NT have
   full preemptive proceses and threads.

   If a platform does not directly support multiple threads, the library will
   them using a cooperative co-routine technique. This requires that each
   thread of execution within a process, voluntarily yields control to other
   threads. This will occur if the thread is blocked inside an I/O function
   on a <A>PChannel</A> or when the <A>PThread::Yield()</A> function is
   explicitly called.
   
   Note that this is <EM>cooperative</EM>. An endless loop will stop all
   threads in a process, possibly all processes on some platforms. If a
   lengthy operation is to take place that does not involve blocking I/O,
   eg pure computation or disk file I/O, then it is the responsiblity of the
   programmer to assure enough yielding for background threads to execute.
 */

  public:
    enum Priority {
      LowestPriority,   // Will only run if all other threads are blocked.
      LowPriority,      // Runs approximately half as often as normal.
      NormalPriority,   // Normal priority for a thread.
      HighPriority,     // Runs approximately twice as often as normal.
      HighestPriority,  // Is only thread that will run, unless blocked.
      NumPriorities
    };
    // Codes for thread priorities.

    PThread(
      PINDEX stackSize,                 // Size of stack to use for thread.
      BOOL startSuspended = TRUE,       // Thread does not execute immediately.
      Priority priorityLevel = NormalPriority  // Initial priority of thread.
    );
    /* Create a new thread instance. Unless the <CODE>startSuspended</CODE>
       parameter is TRUE, the threads <A>Main()</A> function is called to
       execute the code for the thread.
       
       Note that the exact timing of the execution of code in threads can
       never be predicted. Thus never assume that on return from this
       constructor that the thread has executed any code at all. The only
       guarentee is that it will <EM>not</EM> be executed if the thread is
       started suspended.

       If synchronisation is required between threads then the use of
       semaphores is essential.
       
       The stack size specified is <EM>not</EM> simply in bytes. It is a value
       that is multiplied by a factor into bytes depending on the target
       platform. For example a Unix system with a RISC processor may use
       significantly more stack than an MS-DOS platform. These sizes are
       normalised to the "stack factor" provided here. For some platforms, eg
       Windows NT, the stack size is only an initial size and the stack will
       automatically be increased as required.
     */

    ~PThread();
    /* Destroy the thread, this simply calls the <A>Terminate()</A> function
       with all its restrictions and penalties. See that function for more
       information.

       Note that the correct way for a thread to terminate is to return from
       the <A>Main()</A> function.
     */


  // New functions for class
    static PThread * Current();
    /* Get the currently running thread object instance. It is possible, even
       likely, that the smae code may be executed in the context of differenct
       threads. Under some circumstances it may be necessary to know what the
       current codes thread is and this static function provides that
       information.

       <H2>Returns:</H2>
       pointer to current thread.
     */

    virtual void Main() = 0;
    /* User override function for the main execution routine of the thread. A
       descendent class must provide the code that will be executed in the
       thread within this function.
       
       Note that the correct way for a thread to terminate is to return from
       this function.
     */

    virtual void Restart();
    /* Restart a terminated thread using the same stack priority etc that
       was current when the thread terminated.
       
       If the thread is still running then this function is ignored.
     */

    virtual void Terminate();
    /* Terminate the thread. It is highly recommended that this is not used
       except in abnormal abort situations as not all clean up of resources
       allocated to the thread will be executed. This is especially true in
       C++ as the destructors of objects that are automatic variables are not
       called causing at the very least the possiblity of memory leaks.

       Note that the correct way for a thread to terminate is to return from
       the <A>Main()</A> function or self terminate by calling
       <A>Terminate()</A> within the context of the thread which can then
       assure that all resources are cleaned up.
     */

    virtual BOOL IsTerminated() const;
    /* Determine if the thread has been terminated or ran to completion.

       <H2>Returns:</H2>
       TRUE if the thread has been terminated.
     */

    virtual void Suspend(
      BOOL susp = TRUE    // Flag to suspend or resume a thread.
    );
    /* Suspend or resume the thread.
    
       If <CODE>susp</CODE> is TRUE this increments an internal count of
       suspensions that must be matched by an equal number of calls to
       <A>Resume()</A> or <CODE>Suspend(FALSE)</CODE> before the
       thread actually executes again.

       If <CODE>susp</CODE> is FALSE then this decrements the internal count of
       suspensions. If the count is <= 0 then the thread will run. Note that
       the thread will not be suspended until an equal number of
       <CODE>Suspend(TRUE)</CODE> calls are made.
     */

    virtual void Resume();
    /* Resume thread execution, this is identical to
       <CODE>Suspend(FALSE)</CODE>.
     */

    virtual BOOL IsSuspended() const;
    /* Determine if the thread is currently suspended. This checks the
       suspension count and if greater than zero returns TRUE for a suspended
       thread.

       <H2>Returns:</H2>
       TRUE if thread is suspended.
     */

    virtual void Sleep(
      const PTimeInterval & delay   // Time interval to sleep for.
    );
    // Suspend the current thread for the specified amount of time.

    virtual void SetPriority(
      Priority priorityLevel    // New priority for thread.
    );
    /* Set the priority of the thread relative to other threads in the current
       process.
     */

    virtual Priority GetPriority() const;
    /* Get the current priority of the thread in the current process.

       <H2>Returns:</H2>
       current thread priority.
     */

    static void Yield();
    /* Yield to another thread. If there are no other threads then this
       function does nothing. Note that on most platforms the threading is
       cooperative and this function must be called for other threads to run
       at all. There may be an implicit call to Yield within the I/O functions
       of <A>PChannel</A> classes. This is so when a thread is I/O blocked then
       other threads can, as far as possible, continue to run.
       
       If the platform directly supports multiple threads then this function
       will do nothing.
     */


  protected:
    void InitialiseProcessThread();
    /* Initialialise the primordial thread, the one in the PProcess. This is
       required due to the bootstrap logic of processes and threads.
     */

#ifndef P_PLATFORM_HAS_THREADS
    virtual BOOL IsNoLongerBlocked();
    /* Check if the condition that has blocked a thread in an I/O function,
       for example, has ceased. This is required by the internal cooperative
       thread scheduler.

       This function is not present for platforms that support threads.
       
       <H2>Returns:</H2>
       TRUE if the thread is no longer blocked.
     */
#endif

  private:
    PThread();
    // Create a new thread instance as part of a PProcess class.

    friend class PProcess;
    // So a PProcess can get at PThread() constructor but nothing else.

    PThread(const PThread &) { }
    // Empty constructor to prevent copying of thread instances.

    PThread & operator=(const PThread &) { return *this; }
    // Empty assignment operator to prevent copying of thread instances.

#ifndef P_PLATFORM_HAS_THREADS
    void AllocateStack(
      PINDEX stackSize  // Size of the stack to allocate.
    );
    /* Allocate the stack for the thread.

       The stack size specified is <EM>not</EM> simply in bytes. It is a value
       that is multiplied by a factor into bytes depending on the target
       platform. For example a Unix system with a RISC processor may use
       significantly more stack than an MS-DOS platform. These sizes are
       normalised to the "stack factor" provided here. For some platforms, eg
       Windows NT, the stack size is only an initial size and the stack will
       automatically be increased as required.

       This function is not present for platforms that support threads.
     */

    void ClearBlock();
    /* Clear the blocked thread. This is used by platform dependent code to
       signal to the common code scheduler that the thread is no longer
       blocked.

       This function is not present for platforms that support threads.
     */

    void BeginThread();
    /* Function to start <A>Main()</A> and exit when completed.

       This function is not present for platforms that support threads.
     */

    virtual void SwitchContext(
      PThread * from    // Thread being switched from.
    );
    /* Do the machinations needed to jump to the current thread. This is a
       platform dependent function that utilises the standard C
       <CODE>setjmp()</CODE> and <CODE>longjmp()</CODE> functions to implement
       the co-routines.
    
       This function is not present for platforms that support threads.
     */


    // Member fields
    Priority basePriority;
    /* The threads priority level, relative to other threads.

       This variable is not present for platforms that support threads.
     */

    int dynamicPriority;
    /* The threads priority during this scheduled slice. A thread that has not
       been scheduled has its dynamic priority increased so that next time
       the scheduler is looking for a thread to run it has a better chance of
       executing. Once a thread is executed the dynamic priority is set back
       to the base priority as set by <A>SetPriority()</A>.

       This variable is not present for platforms that support threads.
     */

    int suspendCount;
    /* The threads count of calls to <A>Suspend()</A> or <A>Resume()</A>.
       If <=0 then can run, if >0 means suspended and is not to be scheduled.

       This variable is not present for platforms that support threads.
     */

    PTimer sleepTimer;
    /* Time for thread to remain asleep. Thread is not scheduled while this
       is running after a <A>Sleep()</A> call.

       This variable is not present for platforms that support threads.
     */

    PSemaphore * blockingSemaphore;
    /* Semaphore that is blocking this thread.

       This variable is not present for platforms that support threads.
     */

    PThread * link;
    /* Link to next thread in circular list. The use of a list rather than
       priority queues or other sophisticated task queuing technique is to
       simplify the scheduler. It is expected that any given process will not
       have a large number of threads, ie less than approximately ten, so the
       overhead of the list search is small in comparison to the overhead in
       complex data structures.

       This variable is not present for platforms that support threads.
     */

    enum {
      Starting,         // Thread is starting up.
      Running,          // Thread is the currently executing context.
      Waiting,          // Thread is waiting to be scheduled.
      Sleeping,         // Thread is sleeping until sleepTimer is up.
      Suspended,        // Thread is currently suspended.
      BlockedIO,        // Thread is currently blocked in I/O.
      SuspendedBlockIO, // Thread is blocked <EM>and</EM> suspended.
      BlockedSem,       // Thread is currently blocked by a semaphore.
      SuspendedBlockSem,// Thread is blocked <EM>and</EM> suspended.
      Terminating,      // Thread is terminating but has not died yet.
      Terminated        // Thread has terminated.
    } status;
    /* Thread status for scheduler handling.

       This variable is not present for platforms that support threads.
     */

    jmp_buf context;
    /* Buffer for context switching.

       This variable is not present for platforms that support threads.
     */

    char PSTATIC * stackBase;
    /* Base of stack allocated for the thread. The PSTATIC is for DOS-Windows.

       This variable is not present for platforms that support threads.
     */

    char PSTATIC * stackTop;
    /* Top of stack allocated for the thread.

       This variable is not present for platforms that support threads.
     */


    friend class PSemaphore;
#endif


// Class declaration continued in platform specific header file ///////////////
