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
 * Revision 1.8  2002/04/30 06:21:54  robertj
 * Fixed PReadWriteMutex class to implement text book algorithm!
 *
 * Revision 1.7  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.6  1999/03/09 02:59:51  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.5  1999/02/16 08:11:17  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.4  1998/11/30 02:52:01  robertj
 * New directory structure
 *
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

#include <ptlib/mutex.h>
#include <ptlib/syncpoint.h>

/** This class defines a thread synchonisation object.

   This may be used to send signals to a thread and await an acknowldegement
   that the signal was processed. This can be be used to initate an action in
   another thread and wait for the action to be completed.
\begin{verbatim}
    ... thread one
    while (condition) {
      sync.Wait();
      do_something();
      sync.Acknowledge();
    }

    ... thread 2
    do_something_else();
    sync.Signal();    // At this point thread 1 wake up and does something.
    do_yet_more();    // However, this does not get done until Acknowldege()
                      // is called in the other thread.

\end{verbatim}
 */
class PSyncPointAck : public PSyncPoint
{
  PCLASSINFO(PSyncPointAck, PSyncPoint);

  public:
    /** If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.

       Unlike the PSyncPoint::Signal() this function will block until the
       target thread that was blocked by the Wait() function has resumed
       execution and called the Acknowledge() function.

       The #waitTime# parameter is used as a maximum amount of time
       to wait for the achnowledgement to be returned from the other thread.
     */
    virtual void Signal();
    void Signal(const PTimeInterval & waitTime);

    /** This indicates that the thread that was blocked in a Wait() on this
       synchonrisation object has completed the operation the signal was
       intended to initiate. This unblocks the thread that had called the
       Signal() function to initiate the action.
     */
    void Acknowledge();

  protected:
    PSyncPoint ack;
};


/**This class defines a thread synchonisation object.

   This is a special type of mutual exclusion, where a thread wishes to get
   exlusive use of a resource but only if a certain other condition is met.
 */
class PCondMutex : public PMutex
{
  PCLASSINFO(PCondMutex, PMutex);

  public:
    /** This function attempts to acquire the mutex, but will block not only
       until the mutex is free, but also that the condition returned by the
       Condition() function is also met.
     */
    virtual void WaitCondition();

    /** If there are waiting (blocked) threads then unblock the first one that
       was blocked. If no waiting threads and the count is less than the
       maximum then increment the semaphore.
     */
    virtual void Signal();

    /** This is the condition that must be met for the WaitCondition() function
       to acquire the mutex.
     */
    virtual BOOL Condition() = 0;

    /** This function is called immediately before blocking on the condition in
       the WaitCondition() function. This could get called multiple times
       before the condition is met and the WaitCondition() function returns.
     */
    virtual void OnWait();

  protected:
    PSyncPoint syncPoint;
};


/** This is a PCondMutex for which the conditional is the value of an integer.
 */
class PIntCondMutex : public PCondMutex
{
  PCLASSINFO(PIntCondMutex, PCondMutex);

  public:
  /**@name Construction */
  //@{
    /// defines possible operators on current value and target value
    enum Operation {
      /// Less than
      LT,
      /// Less than or equal to
      LE,
      /// Equal to
      EQ,
      /// Greater than or equal to
      GE,
      /// Greater than
      GT
    };

    /**
      Create a cond mutex using an integer
    */
    PIntCondMutex(
      int value = 0,            /// initial value if the integer
      int target = 0,           /// target vaue which causes the mutex to unlock
      Operation operation = LE  /// comparison operator
    );
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Print the object on the stream. This will be of the form
       #"(value < target)"#.
     */
    void PrintOn(ostream & strm) const;
  //@}

  /**@name Condition access functions */
  //@{
    /** This is the condition that must be met for the WaitCondition() function
       to acquire the mutex.

       @return TRUE if condition is met.
     */
    virtual BOOL Condition();

    /**Get the current value of the condition variable.
      @return Current condition variable value.
     */
    operator int() const { return value; }

    /**Assign new condition value.
       Use the Wait() function to acquire the mutex, modify the value, then
       release the mutex, possibly releasing the thread in the WaitCondition()
       function if the condition was met by the operation.

       @return The object reference for consecutive operations in the same statement.
     */
    PIntCondMutex & operator=(int newval);

    /**Increment condition value.
       Use the Wait() function to acquire the mutex, modify the value, then
       release the mutex, possibly releasing the thread in the WaitCondition()
       function if the condition was met by the operation.

       @return The object reference for consecutive operations in the same statement.
     */
    PIntCondMutex & operator++();

    /**Add to condition value.
       Use the Wait() function to acquire the mutex, modify the value, then
       release the mutex, possibly releasing the thread in the WaitCondition()
       function if the condition was met by the operation.

       @return The object reference for consecutive operations in the same statement.
     */
    PIntCondMutex & operator+=(int inc);

    /**Decrement condition value.
       Use the Wait() function to acquire the mutex, modify the value, then
       release the mutex, possibly releasing the thread in the WaitCondition()
       function if the condition was met by the operation.

       @return The object reference for consecutive operations in the same statement.
     */
    PIntCondMutex & operator--();

    /**Subtract from condition value.
       Use the Wait() function to acquire the mutex, modify the value, then
       release the mutex, possibly releasing the thread in the WaitCondition()
       function if the condition was met by the operation.

       @return The object reference for consecutive operations in the same statement.
     */
    PIntCondMutex & operator-=(int dec);
  //@}


  protected:
    int value, target;
    Operation operation;
};


/** This class defines a thread synchonisation object.

   This is a special type of mutual exclusion, where the excluded area may
   have multiple read threads but only one write thread and the read threads
   are blocked on write as well.
 */

class PReadWriteMutex : public PObject
{
  PCLASSINFO(PReadWriteMutex, PObject);
  public:
    /** This function attempts to acquire the mutex for reading.
     */
    void StartRead();

    /** This function attempts to release the mutex for reading.
     */
    void EndRead();

    /** This function attempts to acquire the mutex for writing.
     */
    void StartWrite();

    /** This function attempts to release the mutex for writing.
     */
    void EndWrite();

  protected:
    PMutex   readMutex;
    PMutex   readerCountMutex;
    unsigned readerCount;

    PMutex   writeMutex;
    PMutex   writerCountMutex;
    unsigned writerCount;

    PMutex   starvationPreventer;
};


// End Of File ///////////////////////////////////////////////////////////////
