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


class PWaitAndSignal {
/* This class waits for the semaphore on construction and automatically
   signals the semaphore on destruction. Any descendent of PSemaphore
   may be used.
 */
  public:
    PWaitAndSignal(PSemaphore & sem);
    ~PWaitAndSignal();
  protected:
    PSemaphore & semaphore;
};


PDECLARE_CLASS(PSemaphore, PObject)
/* This class defines a thread synchonisation object. This is in the form of a
   integer semaphore. The semaphore has a count and a maximum value. The
   various combinations of count and usage of the <A>Wait()</A> and
   <A>Signal()</A> functions determine the type of synchronisation mechanism
   to be employed.

   The <A>Wait()</A> operation is that if the semaphore count is > 0,
   decrement the semaphore and return. If it is = 0 then wait (block).

   The <A>Signal()</A> operation is that if there are waiting threads then
   unblock the first one that was blocked. If no waiting threads and the count
   is less than the maximum then increment the semaphore.

   The most common is to create a mutual exclusion zone. A mutex is where a
   piece of code or data cannot be accessed by more than one thread at a time.
   To prevent this the PSemaphore is used in the following manner:
      <CODE>
      PSemaphore mutex(1, 1);  // Maximum value of 1 and initial value of 1.

      ...

      mutex.Wait();

      ... critical section - only one thread at a time here.

      mutex.Signal();

      ...
      </CODE>
    The first thread will pass through the <A>Wait()</A> function, a second
    thread will block on that function until the first calls the
    <A>Signal()</A> function, releasing the second thread.
 */

  public:
    PSemaphore(
      unsigned initial, // Initial value for semaphore count.
      unsigned maximum  // Maximum value for semaphore count.
    );
    /* Create a new semaphore with maximum count and initial value specified.
       If the initial value is larger than the maximum value then is is set to
       the maximum value.
     */

    ~PSemaphore();
    /* Destroy the semaphore. This will assert if there are still waiting
       threads on the semaphore.
     */


  // New functions for class.
    virtual void Wait();
    virtual BOOL Wait(
      const PTimeInterval & timeout // Amount of time to wait for semaphore.
    );
    /* If the semaphore count is > 0, decrement the semaphore and return. If
       if is = 0 then wait (block).

       <H2>Returns:</H2>
       TRUE if semaphore was signalled, FALSE if timed out.
     */

    virtual void Signal();
    /* If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.
     */

    virtual BOOL WillBlock() const;
    /* Determine if the semaphore would block if the <A>Wait()</A> function
       were called.
     */

#ifndef P_PLATFORM_HAS_THREADS
  protected:
    unsigned currentCount;
    unsigned maximumCount;
    PTimer   timeout;
    PQUEUE(BlockedThreadsQueue, PThread);
    BlockedThreadsQueue blockedThreads;
    friend void PThread::Yield();
#endif


  private:
    PSemaphore(const PSemaphore &) { }
    PSemaphore & operator=(const PSemaphore &) { return *this; }


// Class declaration continued in platform specific header file ///////////////
