/*
 * thread.h
 *
 * Executable thread encapsulation class (pre-emptive if OS allows).
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: thread.h,v $
 * Revision 1.27  2001/09/10 02:51:22  robertj
 * Major change to fix problem with error codes being corrupted in a
 *   PChannel when have simultaneous reads and writes in threads.
 *
 * Revision 1.26  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.25  2000/11/28 12:55:36  robertj
 * Added static function to create a new thread class and automatically
 *   run a function on another class in the context of that thread.
 *
 * Revision 1.24  2000/10/20 05:31:09  robertj
 * Added function to change auto delete flag on a thread.
 *
 * Revision 1.23  2000/06/26 11:17:19  robertj
 * Nucleus++ port (incomplete).
 *
 * Revision 1.22  2000/02/29 12:26:14  robertj
 * Added named threads to tracing, thanks to Dave Harvey
 *
 * Revision 1.21  1999/06/06 05:07:17  robertj
 * Fixed documentation error.
 *
 * Revision 1.20  1999/03/09 02:59:51  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.19  1999/02/16 08:11:17  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.18  1998/11/20 03:18:33  robertj
 * Added thread WaitForTermination() function.
 *
 * Revision 1.17  1998/10/31 12:47:59  robertj
 * Removed ability to start threads immediately, race condition with vtable (Main() function).
 *
 * Revision 1.16  1998/09/23 06:21:41  robertj
 * Added open source copyright license.
 *
 * Revision 1.15  1996/03/02 03:15:51  robertj
 * Added automatic deletion of thread object instances on thread completion.
 *
 * Revision 1.14  1995/12/10 11:44:32  robertj
 * Fixed bug in non-platform threads and semaphore timeouts.
 *
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


#ifdef __GNUC__
#pragma interface
#endif

#ifdef Priority
#undef Priority
#endif

class PSemaphore;


///////////////////////////////////////////////////////////////////////////////
// PThread

/** This class defines a thread of execution in the system. A {\it thread} is
   an independent flow of processor instructions. This differs from a
   {\it process} which also embodies a program address space and resource
   allocation. So threads can share memory and resources as they run in the
   context of a given process. A process always contains at least one thread.
   This is reflected in this library by the #PProcess# class being
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
   on a #PChannel# or when the #PThread::Yield()# function is
   explicitly called.
   
   Note that this is {\bf cooperative}. An endless loop will stop all
   threads in a process, possibly all processes on some platforms. If a
   lengthy operation is to take place that does not involve blocking I/O,
   eg pure computation or disk file I/O, then it is the responsiblity of the
   programmer to assure enough yielding for background threads to execute.
 */
class PThread : public PObject
{
  PCLASSINFO(PThread, PObject);

  public:
  /**@name Construction */
  //@{
    /// Codes for thread priorities.
    enum Priority {
      /// Will only run if all other threads are blocked.
      LowestPriority,   

      /// Runs approximately half as often as normal.
      LowPriority,      

      /// Normal priority for a thread.
      NormalPriority,   

      /// Runs approximately twice as often as normal.
      HighPriority,     

      /// Is only thread that will run, unless blocked.
      HighestPriority,  

      NumPriorities
    };

    /// Codes for thread autodelete flag
    enum AutoDeleteFlag {
      /// Automatically delete thread object on termination.
      AutoDeleteThread,   

      /// Don't delete thread as it may not be on heap.
      NoAutoDeleteThread  
    };

    /** Create a new thread instance. Unless the #startSuspended#
       parameter is TRUE, the threads #Main()# function is called to
       execute the code for the thread.
       
       Note that the exact timing of the execution of code in threads can
       never be predicted. Thus you you can get a race condition on
       intialising a descendent class. To avoid this problem a thread is
       always started suspended. You must call the Resume() function after
       your descendent class construction is complete.

       If synchronisation is required between threads then the use of
       semaphores is essential.

       If the #deletion# is set to #AutoDeleteThread#
       then the PThread is assumed to be allocated with the new operator and
       may be freed using the delete operator as soon as the thread is
       terminated or executes to completion (usually the latter).

       The stack size specified is {\bf not} simply in bytes. It is a value
       that is multiplied by a factor into bytes depending on the target
       platform. For example a Unix system with a RISC processor may use
       significantly more stack than an MS-DOS platform. These sizes are
       normalised to the "stack factor" provided here. For some platforms, eg
       Windows NT, the stack size is only an initial size and the stack will
       automatically be increased as required.
     */
    PThread(
      PINDEX stackSize,                 /// Size of stack to use for thread.
      AutoDeleteFlag deletion = AutoDeleteThread,
        /// Automatically delete PThread instance on termination of thread.
      Priority priorityLevel = NormalPriority,  /// Initial priority of thread.
      const PString & threadName = "" /// The name of the thread (for Debug/Trace)
    );

    /** Destroy the thread, this simply calls the #Terminate()# function
       with all its restrictions and penalties. See that function for more
       information.

       Note that the correct way for a thread to terminate is to return from
       the #Main()# function.
     */
    ~PThread();
  //@}

  /**@name Overrides from PObject */
  //@{
    /**Standard stream print function.
       The PObject class has a << operator defined that calls this function
       polymorphically.
      */
    void PrintOn(
      ostream & strm    /// Stream to output text representation
    ) const;
  //@}

  /**@name Control functions */
  //@{
    /** Restart a terminated thread using the same stack priority etc that
       was current when the thread terminated.
       
       If the thread is still running then this function is ignored.
     */
    virtual void Restart();

    /** Terminate the thread. It is highly recommended that this is not used
       except in abnormal abort situations as not all clean up of resources
       allocated to the thread will be executed. This is especially true in
       C++ as the destructors of objects that are automatic variables are not
       called causing at the very least the possiblity of memory leaks.

       Note that the correct way for a thread to terminate is to return from
       the #Main()# function or self terminate by calling
       #Terminate()# within the context of the thread which can then
       assure that all resources are cleaned up.
     */
    virtual void Terminate();

    /** Determine if the thread has been terminated or ran to completion.

       @return
       TRUE if the thread has been terminated.
     */
    virtual BOOL IsTerminated() const;

    /** Block and wait for the thread to terminate.

       @return
       FALSE if the thread has not terminated and the timeout has expired.
     */
    void WaitForTermination() const;
    BOOL WaitForTermination(
      const PTimeInterval & maxWait  /// Maximum time to wait for termination.
    ) const;

    /** Suspend or resume the thread.
    
       If #susp# is TRUE this increments an internal count of
       suspensions that must be matched by an equal number of calls to
       #Resume()# or #Suspend(FALSE)# before the
       thread actually executes again.

       If #susp# is FALSE then this decrements the internal count of
       suspensions. If the count is <= 0 then the thread will run. Note that
       the thread will not be suspended until an equal number of
       #Suspend(TRUE)# calls are made.
     */
    virtual void Suspend(
      BOOL susp = TRUE    /// Flag to suspend or resume a thread.
    );

    /** Resume thread execution, this is identical to
       #Suspend(FALSE)#.
     */
    virtual void Resume();

    /** Determine if the thread is currently suspended. This checks the
       suspension count and if greater than zero returns TRUE for a suspended
       thread.

       @return
       TRUE if thread is suspended.
     */
    virtual BOOL IsSuspended() const;

    /// Suspend the current thread for the specified amount of time.
    static void Sleep(
      const PTimeInterval & delay   /// Time interval to sleep for.
    );

    /** Set the priority of the thread relative to other threads in the current
       process.
     */
    virtual void SetPriority(
      Priority priorityLevel    /// New priority for thread.
    );

    /** Get the current priority of the thread in the current process.

       @return
       current thread priority.
     */
    virtual Priority GetPriority() const;

    /** Set the flag indicating thread object is to be automatically deleted
        when the thread ends.
     */
    virtual void SetAutoDelete(
      AutoDeleteFlag deletion = AutoDeleteThread  /// New auto delete setting.
    );

    /** Reet the flag indicating thread object is to be automatically deleted
        when the thread ends.
     */
    void SetNoAutoDelete() { SetAutoDelete(NoAutoDeleteThread); }

    /** Get the name of the thread. Thread names are a optional debugging aid.

       @return
       current thread name.
     */
    virtual PString GetThreadName() const;

    /** Change the name of the thread. Thread names are a optional debugging aid.

       @return
       current thread name.
     */
    virtual void SetThreadName(
      const PString & name        /// New name for the thread.
    );
  //@}

  /**@name Miscellaneous */
  //@{
    /** User override function for the main execution routine of the thread. A
       descendent class must provide the code that will be executed in the
       thread within this function.
       
       Note that the correct way for a thread to terminate is to return from
       this function.
     */
    virtual void Main() = 0;

    /** Get the currently running thread object instance. It is possible, even
       likely, that the smae code may be executed in the context of differenct
       threads. Under some circumstances it may be necessary to know what the
       current codes thread is and this static function provides that
       information.

       @return
       pointer to current thread.
     */
    static PThread * Current();

    /** Yield to another thread. If there are no other threads then this
       function does nothing. Note that on most platforms the threading is
       cooperative and this function must be called for other threads to run
       at all. There may be an implicit call to Yield within the I/O functions
       of #PChannel# classes. This is so when a thread is I/O blocked then
       other threads can, as far as possible, continue to run.
       
       If the platform directly supports multiple threads then this function
       will do nothing.
     */
    static void Yield();

    /**Create a simple thread executing the specified notifier.
       This creates a simple PThread class that automatically executes the
       function defined by the PNotifier in the context of a new thread.
      */
    static PThread * Create(
      const PNotifier & notifier,     /// Function to execute in thread.
      INT parameter = 0,              /// Parameter value to pass to notifier.
      AutoDeleteFlag deletion = AutoDeleteThread,
        /// Automatically delete PThread instance on termination of thread.
      Priority priorityLevel = NormalPriority,  /// Initial priority of thread.
      const PString & threadName = "", /// The name of the thread (for Debug/Trace)
      PINDEX stackSize = 10000         /// Stack size on some platforms
    );
  //@}

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

    BOOL autoDelete;
    // Automatically delete the thread on completion.

    // Give the thread a name for debugging purposes.
    PString threadName;

#ifndef P_PLATFORM_HAS_THREADS
    void AllocateStack(
      PINDEX stackSize  // Size of the stack to allocate.
    );
    /* Allocate the stack for the thread.

       The stack size specified is {\bf not} simply in bytes. It is a value
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
    /* Function to start #Main()# and exit when completed.

       This function is not present for platforms that support threads.
     */

    virtual void SwitchContext(
      PThread * from    // Thread being switched from.
    );
    /* Do the machinations needed to jump to the current thread. This is a
       platform dependent function that utilises the standard C
       #setjmp()# and #longjmp()# functions to implement
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
       to the base priority as set by #SetPriority()#.

       This variable is not present for platforms that support threads.
     */

    int suspendCount;
    /* The threads count of calls to #Suspend()# or #Resume()#.
       If <=0 then can run, if >0 means suspended and is not to be scheduled.

       This variable is not present for platforms that support threads.
     */

    PTimer sleepTimer;
    /* Time for thread to remain asleep. Thread is not scheduled while this
       is running after a #Sleep()# call.

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
      SuspendedBlockIO, // Thread is blocked {\bf and} suspended.
      BlockedSem,       // Thread is currently blocked by a semaphore.
      SuspendedBlockSem,// Thread is blocked {\bf and} suspended.
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


// Include platform dependent part of class
#include <ptlib/thread.h>
};


// End Of File ///////////////////////////////////////////////////////////////
