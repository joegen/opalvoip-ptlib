/*
 * syncthrd.h
 *
 * Various thread synchronisation classes.
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
 * $Log: syncthrd.h,v $
 * Revision 1.3  1998/10/31 12:46:45  robertj
 * Renamed file for having general thread synchronisation objects.
 * Added conditional mutex and read/write mutex thread synchronisation objects.
 *
 * Revision 1.2  1998/09/23 06:21:35  robertj
 * Added open source copyright license.
 *
 * Revision 1.1  1998/05/30 13:26:15  robertj
 * Initial revision
 *
 */


#define _PSYNCPOINTACK

#ifdef __GNUC__
#pragma interface
#endif

#include <mutex.h>
#include <syncpoint.h>


PDECLARE_CLASS(PSyncPointAck, PSyncPoint)
/* This class defines a thread synchonisation object.

   This may be used to send signals to a thread and await an acknowldegement
   that the signal was processed. This can be be used to initate an action in
   another thread and wait for the action to be completed.
 */

  public:
    virtual void Signal();
    void Signal(const PTimeInterval & waitTime);
    /* If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.

       Unlike the PSyncPoint::Signal() this function will block until the
       target thread that was blocked by the Wait() function has resumed
       execution and called the Acknowledge() function.

       The <CODE>waitTime</CODE> parameter is used as a maximum amount of time
       to wait for the achnowledgement to be returned from the other thread.
     */

    void Acknowledge();
    /* This indicates that the thread that was blocked in a Wait() on this
       synchonrisation object has completed the operation the signal was
       intended to initiate. This unblocks the thread that had called the
       Signal() function to initiate the action.
     */

  protected:
    PSyncPoint ack;
};


PDECLARE_CLASS(PCondMutex, PMutex)
/* This class defines a thread synchonisation object.

   This is a special type of mutual exclusion, where a thread wishes to get
   exlusive use of a resource but only if a certain other condition is met.
 */

  public:
    virtual void WaitCondition();
    /* This function attempts to acquire the mutex, but will block not only
       until the mutex is free, but also that the condition returned by the
       Condition() function is also met.
     */

    virtual void Signal();
    /* If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.
     */

    virtual BOOL Condition() = 0;
    /* This is the condition that must be met for the WaitCondition() function
       to acquire the mutex.
     */

    virtual void OnWait();
    /* This function is called immediately before blocking on the condition in
       the WaitCondition() function. This could get called multiple times
       before the condition is met and the WaitCondition() function returns.
     */

  protected:
    PSyncPoint syncPoint;
};


PDECLARE_CLASS(PIntCondMutex, PCondMutex)
/* This class defines a thread synchonisation object.

   This is a PCondMutex for which the conditional is the value of an integer.
 */

  public:
    enum Operation {
      LT, LE, EQ, GE, GT
    };
    PIntCondMutex(
      int value = 0,
      int target = 0,
      Operation operation = LE
    );

    void PrintOn(ostream & strm) const;
    /* Print the object on the stream. This will be of the form
       "(value < target)".
     */

    virtual BOOL Condition();
    /* This is the condition that must be met for the WaitCondition() function
       to acquire the mutex. The 
     */

    operator int() const { return value; }
    /* Return the current value of the condition variable.
     */

    PIntCondMutex & operator=(int newval);
    PIntCondMutex & operator++();
    PIntCondMutex & operator+=(int inc);
    PIntCondMutex & operator--();
    PIntCondMutex & operator-=(int dec);
    /* Each of the above operators will use the Wait() function to acquire the
       mutex, modify the value, then release the mutex, possibly releasing the
       thread in the WaitCondition() function if the condition was met by the
       operation.

       <H2>Returns:</H2>
       The object reference for consecutive operations in the same statement.
     */


  protected:
    int value, target;
    Operation operation;
};


PDECLARE_CLASS(PReadWriteMutex, PObject)
/* This class defines a thread synchonisation object.

   This is a special type of mutual exclusion, where the excluded area may
   have multiple read threads but only one write thread and the read threads
   are blocked on write as well.
 */

  public:
    void StartRead();
    /* This function attempts to acquire the mutex for reading.
     */

    void EndRead();
    /* This function attempts to release the mutex for reading.
     */

    void StartWrite();
    /* This function attempts to acquire the mutex for writing.
     */

    void EndWrite();
    /* This function attempts to release the mutex for writing.
     */

  protected:
    PMutex starvationPreventer;
    PIntCondMutex readers;
};


////////////////////////////////////////////////////////////////////////////////
