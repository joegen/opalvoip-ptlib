/*
 * semaphor.h
 *
 * Thread synchronisation semaphore class.
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
 * $Log: semaphor.h,v $
 * Revision 1.11  2001/04/23 00:34:29  robertj
 * Added ability for PWaitAndSignal to not wait on semaphore.
 *
 * Revision 1.10  2001/01/27 23:40:09  yurik
 * WinCE port-related - CreateEvent used instead of CreateSemaphore
 *
 * Revision 1.9  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.8  1999/03/09 02:59:50  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.7  1999/02/16 08:11:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.6  1998/11/19 05:17:37  robertj
 * Added PWaitAndSignal class for easier mutexing.
 *
 * Revision 1.5  1998/09/23 06:21:19  robertj
 * Added open source copyright license.
 *
 * Revision 1.4  1998/03/20 03:16:11  robertj
 * Added special classes for specific sepahores, PMutex and PSyncPoint.
 *
 * Revision 1.3  1995/12/10 11:34:50  robertj
 * Fixed incorrect order of parameters in semaphore constructor.
 *
 * Revision 1.2  1995/11/21 11:49:42  robertj
 * Added timeout on semaphore wait.
 *
 * Revision 1.1  1995/08/01 21:41:24  robertj
 * Initial revision
 *
 */


#define _PSEMAPHORE

#ifdef __GNUC__
#pragma interface
#endif

#include <limits.h>


/**This class waits for the semaphore on construction and automatically
   signals the semaphore on destruction. Any descendent of PSemaphore
   may be used.

  This is very usefull for constructs such as:
\begin{verbatim}
    void func()
    {
      PWaitAndSignal mutexWait(myMutex);
      if (condition)
        return;
      do_something();
      if (other_condition)
        return;
      do_something_else();
    }
\end{verbatim}
 */
class PWaitAndSignal {
  public:
    /**Create the semaphore wait instance.
       This will wait on the specified semaphore using the #Wait()# function
       before returning.
      */
    PWaitAndSignal(
      PSemaphore & sem,   /// Semaphore descendent to wait/signal.
      BOOL wait = TRUE    /// Wait for semaphore before returning.
    );
    /** Signal the semaphore.
        This will execute the Signal() function on the semaphore that was used
        in the construction of this instance.
     */
    ~PWaitAndSignal();

  protected:
    PSemaphore & semaphore;
};


/**This class defines a thread synchonisation object. This is in the form of a
   integer semaphore. The semaphore has a count and a maximum value. The
   various combinations of count and usage of the #Wait()# and
   #Signal()# functions determine the type of synchronisation mechanism
   to be employed.

   The #Wait()# operation is that if the semaphore count is > 0,
   decrement the semaphore and return. If it is = 0 then wait (block).

   The #Signal()# operation is that if there are waiting threads then
   unblock the first one that was blocked. If no waiting threads and the count
   is less than the maximum then increment the semaphore.

   The most common is to create a mutual exclusion zone. A mutex is where a
   piece of code or data cannot be accessed by more than one thread at a time.
   To prevent this the PSemaphore is used in the following manner:
\begin{verbatim}
      PSemaphore mutex(1, 1);  // Maximum value of 1 and initial value of 1.

      ...

      mutex.Wait();

      ... critical section - only one thread at a time here.

      mutex.Signal();

      ...
\end{verbatim}
    The first thread will pass through the #Wait()# function, a second
    thread will block on that function until the first calls the
    #Signal()# function, releasing the second thread.
 */
class PSemaphore : public PObject
{
  PCLASSINFO(PSemaphore, PObject);

  public:
  /**@name Construction */
  //@{
    /**Create a new semaphore with maximum count and initial value specified.
       If the initial value is larger than the maximum value then is is set to
       the maximum value.
     */
    PSemaphore(
      unsigned initial, /// Initial value for semaphore count.
      unsigned maximum  /// Maximum value for semaphore count.
    );

    /**Destroy the semaphore. This will assert if there are still waiting
       threads on the semaphore.
     */
    ~PSemaphore();
  //@}

  /**@name Operations */
  //@{
    /**If the semaphore count is > 0, decrement the semaphore and return. If
       if is = 0 then wait (block).
     */
    virtual void Wait();

    /**If the semaphore count is > 0, decrement the semaphore and return. If
       if is = 0 then wait (block) for the specified amount of time.

       @return
       TRUE if semaphore was signalled, FALSE if timed out.
     */
    virtual BOOL Wait(
      const PTimeInterval & timeout // Amount of time to wait for semaphore.
    );

    /**If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.
     */
    virtual void Signal();

    /**Determine if the semaphore would block if the #Wait()# function
       were called.

       @return
       TRUE if semaphore will block when Wait() is called.
     */
    virtual BOOL WillBlock() const;
  //@}

#ifndef P_PLATFORM_HAS_THREADS
  protected:
    unsigned currentCount;
    unsigned maximumCount;
    PTimer   timeout;
    PQUEUE(BlockedThreadsQueue, PThread);
    BlockedThreadsQueue blockedThreads;
    friend void PThread::Yield();
#else
#ifdef _WIN32_WCE
    unsigned currentCount;
#endif  
#endif


  private:
    PSemaphore(const PSemaphore &) { }
    PSemaphore & operator=(const PSemaphore &) { return *this; }

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
