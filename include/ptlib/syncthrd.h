/*
 * syncthrd.h
 *
 * Various thread synchronisation classes.
 *
 * Portable Tools Library
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_SYNCTHRD_H
#define PTLIB_SYNCTHRD_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/mutex.h>
#include <ptlib/semaphor.h>
#include <ptlib/syncpoint.h>
#include <map>
#include <queue>


/** This class defines a thread synchronisation object.

   This may be used to send signals to a thread and await an acknowldegement
   that the signal was processed. This can be be used to initiate an action in
   another thread and wait for the action to be completed.
<pre><code>
    ... thread one
    while (condition) {
      sync.Wait();
      do_something();
      sync.Acknowledge();
    }

    ... thread 2
    do_something_else();
    sync.Signal();    // At this point thread 1 wake up and does something.
    do_yet_more();    // However, this does not get done until Acknowledge()
                      // is called in the other thread.

</code></pre>
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

       The <code>waitTime</code> parameter is used as a maximum amount of time
       to wait for the achnowledgement to be returned from the other thread.
     */
    virtual void Signal();
    void Signal(const PTimeInterval & waitTime);

    /** This indicates that the thread that was blocked in a Wait() on this
       synchronisation object has completed the operation the signal was
       intended to initiate. This unblocks the thread that had called the
       Signal() function to initiate the action.
     */
    void Acknowledge();

  protected:
    PSyncPoint ack;
};


/**This class defines a thread synchronisation object.

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
    virtual PBoolean Condition() = 0;

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
      int value = 0,            ///< initial value if the integer
      int target = 0,           ///< target vaue which causes the mutex to unlock
      Operation operation = LE  ///< comparison operator
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

       @return true if condition is met.
     */
    virtual PBoolean Condition();

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


/** This class defines a thread synchronisation object.

   This is a special type of mutual exclusion, where the excluded area may
   have multiple read threads but only one write thread and the read threads
   are blocked on write as well.

   The original algorithm for this was described in 'Communications of the ACM:
   Concurrent Control with "Readers" and "Writers" P.J. Courtois,* F. H, 1971'
   http://cs.nyu.edu/~lerner/spring10/MCP-S10-Read04-ReadersWriters.pdf which
   can be changed via #define to an alternate algorithm 'Faster Fair Solution
   for the Reader-Writer Problem. V.Popov, O.Mazonka 2013'
   http://arxiv.org/ftp/arxiv/papers/1309/1309.4507.pdf to improve efficiency.
 */

class PReadWriteMutex : public PObject, protected PMutexExcessiveLockInfo
{
  PCLASSINFO(PReadWriteMutex, PObject);
  public:
  /**@name Construction */
  //@{
    explicit PReadWriteMutex(
      const char * name = NULL,  ///< Arbitrary name, or filename of mutex variable declaration
      unsigned line = 0,         ///< Line number, if non zero, name is assumed to be a filename
      unsigned timeout = 0       ///< Timeout in ms, before declaring a possible deadlock. Zero uses default.
    );
    ~PReadWriteMutex();
  //@}

  /**@name Operations */
  //@{
    /** This function attempts to acquire the mutex for reading.
        This call may be nested and must have an equal number of EndRead()
        calls for the mutex to be released.
     */
    void StartRead();

    /** This function attempts to release the mutex for reading.
     */
    void EndRead();

    /** This function attempts to acquire the mutex for writing.
        This call may be nested and must have an equal number of EndWrite()
        calls for the mutex to be released.

        Note, if the same thread had a read lock previous to this call then
        the read lock is automatically released and reacquired when EndWrite()
        is called, unless an EndRead() is called. The EndRead() and EndWrite()
        calls do not have to be strictly nested.

        It should also be noted that a consequence of this is that another
        thread may acquire the write lock before the thread that previously
        had the read lock. Thus it is impossibly to go straight from a read
        lock to write lock without the possiblility of the object being
        changed and application logic should take this into account.
     */
    void StartWrite();

    /** This function attempts to release the mutex for writing.
        Note, if the same thread had a read lock when the StartWrite() was
        called which has not yet been released by an EndRead() call then this
        will reacquire the read lock.

        It should also be noted that a consequence of this is that another
        thread may acquire the write lock before the thread that regains the
        read lock. Thus it is impossibly to go straight from a write lock to
        read lock without the possiblility of the object being changed and
        application logic should take this into account.
     */
    void EndWrite();
  //@}

    virtual void PrintOn(ostream &strm) const;

  protected:
#if P_READ_WRITE_ALGO2
    PSemaphore  m_inSemaphore;
    unsigned    m_inCount;
    PSemaphore  m_outSemaphore;
    unsigned    m_outCount;
    PSemaphore  m_writeSemaphore;
    bool        m_wait;
#else
    PSemaphore  m_readerSemaphore;
    PTimedMutex m_readerMutex;
    unsigned    m_readerCount;
    PTimedMutex m_starvationPreventer;

    PSemaphore  m_writerSemaphore;
    PTimedMutex m_writerMutex;
    unsigned    m_writerCount;
#endif
    struct Nest
    {
      unsigned m_readerCount;
      unsigned m_writerCount;
      bool     m_waiting;
      PUniqueThreadIdentifier m_uniqueId;

#if PTRACING
    uint64_t m_startHeldCycle;
#endif

      Nest()
        : m_readerCount(0)
        , m_writerCount(0)
        , m_waiting(false)
        , m_uniqueId(PThread::GetCurrentUniqueIdentifier())
      { }
    };
    typedef std::map<PThreadIdentifier, Nest> NestMap;
    NestMap          m_nestedThreads;
    PCriticalSection m_nestingMutex;

    Nest * GetNest();
    Nest & StartNest();
    void EndNest();
    void InternalStartRead(Nest & nest);
    void InternalEndRead(Nest & nest);
    void InternalStartWrite(Nest & nest);
    void InternalEndWrite(Nest & nest);
    void InternalWait(Nest & nest, PSync & sync) const;

  friend class PSafeObject;
};

/// Declare a PReadWriteMutex with compiled file/line for deadlock debugging
#define PDECLARE_READ_WRITE_MUTEX_ARG_1(var)              struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(__FILE__,__LINE__) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_2(var,nam)          struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam             ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_3(var,nam,to)       struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,0,to        ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_4(var,nam,to,tw)    struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,0,to,tw     ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_5(var,nam,to,tw,th) struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,0,to,tw,th  ) { } } var

#define PDECLARE_READ_WRITE_MUTEX_PART1(narg, args) PDECLARE_READ_WRITE_MUTEX_PART2(narg, args)
#define PDECLARE_READ_WRITE_MUTEX_PART2(narg, args) PDECLARE_READ_WRITE_MUTEX_ARG_##narg args

#define PDECLARE_READ_WRITE_MUTEX(...) PDECLARE_READ_WRITE_MUTEX_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))

#if PTRACING
  class PInstrumentedReadWriteMutex : public PReadWriteMutex
  {
    public:
      PInstrumentedReadWriteMutex(
        const char * name,
        const char * file,
        unsigned line,
        const char * waitReadOnlyName,
        const char * heldReadOnlyName,
        const char * waitReadWriteName,
        const char * heldReadWriteName,
        unsigned waitTime,
        unsigned heldTime,
        unsigned throttleTime = 10000,    ///< Time between PTRACE outpout in milliseconds
        unsigned throttledLogLevel = 2,   ///< PTRACE level to use if enough samples are above thresholdTime
        unsigned unthrottledLogLevel = 6, ///< PTRACE level to use otherwise
        unsigned thresholdPercent = 5,    ///< Percentage of samples above thresholdTime to trigger throttledLogLevel
        unsigned maxHistory = 0           ///< Optional number of samples above thresholdTime to display sincle last PTRACE()
      ) : PReadWriteMutex(name, 0, std::max((int)1000, (int)waitTime*2))
        , m_timeWaitReadOnlyContext(waitReadOnlyName, file, line, waitTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeHeldReadOnlyContext(heldReadOnlyName, file, line, heldTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeWaitReadWriteContext(waitReadWriteName, file, line, waitTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeHeldReadWriteContext(heldReadWriteName, file, line, heldTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
      { }

    protected:
      virtual void AcquiredLock(uint64_t startWaitCycle, bool readOnly);
      virtual void ReleasedLock(const PObject & mutex, uint64_t startHeldSamplePoint, bool readOnly);

      PProfiling::TimeScope m_timeWaitReadOnlyContext;
      PProfiling::TimeScope m_timeHeldReadOnlyContext;
      PProfiling::TimeScope m_timeWaitReadWriteContext;
      PProfiling::TimeScope m_timeHeldReadWriteContext;
  };

  #define PDECLARE_INSTRUMENTED_READ_WRITE_MUTEX(var, name, ...) \
    struct PInstrumentedReadWriteMutex_##name : PInstrumentedReadWriteMutex { \
      PInstrumentedReadWriteMutex_##name() : PInstrumentedReadWriteMutex(#name, __FILE__,__LINE__, \
               "Wait R/O " #name, "Held R/O " #name, "Wait R/W " #name, "Held R/W " #name, __VA_ARGS__) { } \
    } var
#else
  #define PDECLARE_INSTRUMENTED_READ_WRITE_MUTEX(var, name, waitTime, heldTime, ...) \
                       PDECLARE_READ_WRITE_MUTEX(var, name, std::max((int)1000, (int)waitTime*2))
#endif

/**This class starts a read operation for the PReadWriteMutex on construction
   and automatically ends the read operation on destruction.

  This is very usefull for constructs such as:
<pre><code>
    void func()
    {
      PReadWaitAndSignal mutexWait(myMutex);
      if (condition)
        return;
      do_something();
      if (other_condition)
        return;
      do_something_else();
    }
</code></pre>
 */
class PReadWaitAndSignal {
  public:
    /**Create the PReadWaitAndSignal wait instance.
       This will wait on the specified PReadWriteMutex using the StartRead()
       function before returning.
      */
    PReadWaitAndSignal(
      const PReadWriteMutex & rw,   ///< PReadWriteMutex descendent to wait/signal.
      PBoolean start = true    ///< Start read operation on PReadWriteMutex before returning.
    );
    /** End read operation on the PReadWriteMutex.
        This will execute the EndRead() function on the PReadWriteMutex that
        was used in the construction of this instance.
     */
    ~PReadWaitAndSignal();

  protected:
    PReadWriteMutex & mutex;
};


/**This class starts a write operation for the PReadWriteMutex on construction
   and automatically ends the write operation on destruction.

  This is very useful for constructs such as:
<pre><code>
    void func()
    {
      PWriteWaitAndSignal mutexWait(myMutex);
      if (condition)
        return;
      do_something();
      if (other_condition)
        return;
      do_something_else();
    }
</code></pre>
 */
class PWriteWaitAndSignal {
  public:
    /**Create the PWriteWaitAndSignal wait instance.
       This will wait on the specified PReadWriteMutex using the StartWrite()
       function before returning.
      */
    PWriteWaitAndSignal(
      const PReadWriteMutex & rw,   ///< PReadWriteMutex descendent to wait/signal.
      PBoolean start = true    ///< Start write operation on PReadWriteMutex before returning.
    );
    /** End write operation on the PReadWriteMutex.
        This will execute the EndWrite() function on the PReadWriteMutex that
        was used in the construction of this instance.
     */
    ~PWriteWaitAndSignal();

  protected:
    PReadWriteMutex & mutex;
};


/** A synchronous queue of objects.
    This implements a queue of objects between threads. The dequeue action will
    always block until an object is placed into the queue.
  */
template <class T> class PSyncQueue : public PObject, public std::queue<T>
{
    PCLASSINFO(PSyncQueue, PObject);
  public:
    typedef std::queue<T> BaseQueue;

    enum State
    {
      e_Open,
      e_Blocked,
      e_Closed
    };

    /// Construct synchronous queue
    PSyncQueue()
      : m_state(e_Open)
      , m_available(0, INT_MAX)
    {
    }

    /// Destroy synchronous queue
    ~PSyncQueue()
    {
      Close(true);
    }

    /// Enqueue an object to the synchronous queue.
    bool Enqueue(const T & obj)
    {
      PWaitAndSignal lock(m_mutex);
      if (m_state == e_Closed)
        return false;
      BaseQueue::push(obj);
      m_available.Signal();
      return true;
    }

    /** Dequeue an object from the synchronous queue.
        @return false if a timeout occurs, or the queue was flushed.
      */
    bool Dequeue(T & value, const PTimeInterval & timeout = PMaxTimeInterval)
    {
      bool dequeued = false;

      m_mutex.Wait();

      switch (m_state) {
        case e_Blocked :
          PAssertAlways("Multiple threads in PSyncQueue::pop()");
          break;

        case e_Open :
          m_state = e_Blocked;

          {
            m_mutex.Signal();
            bool available = m_available.Wait(timeout);
            m_mutex.Wait();

            if (available && !BaseQueue::empty()) {
              value = BaseQueue::front();
              BaseQueue::pop();
              dequeued = true;
            }
          }

          if (m_state != e_Closed) {
            m_state = e_Open;
            break;
          }
          // Do closed case

        case e_Closed :
          m_closed.Signal();
      }

      m_mutex.Signal();

      return dequeued;
    }

    /** Close the queue and break block in Dequeue() function.
        This may optionally wait for Dequeue() to exit before returning.
      */
    void Close(bool wait)
    {
      m_mutex.Wait();

      bool blocked = m_state == e_Blocked;
      m_state = e_Closed;

      while (!BaseQueue::empty())
        BaseQueue::pop();

      m_available.Signal();
      m_mutex.Signal();

      if (blocked && wait)
        m_closed.Wait();
    }

    // Indicate queue is in use.
    bool IsOpen() const
    {
        PWaitAndSignal mutex(m_mutex);
        return m_state != e_Closed;
    }

    /// Restart the queue after it has been closed.
    void Restart()
    {
      m_mutex.Wait();
      if (m_state == e_Closed) {
        while (!BaseQueue::empty())
          BaseQueue::pop();
        while (m_available.Wait(0))
          ;
        m_state = e_Open;
      }
      m_mutex.Signal();
    }


    __inline const PMutex & GetMutex() const { return m_mutex; }
    __inline       PMutex & GetMutex()       { return m_mutex; }

  protected:
    State          m_state;
    PSemaphore     m_available;
    PMutex         m_mutex;
    PSyncPoint     m_closed;

  private:
    __inline PSyncQueue(const PSyncQueue & other) : PObject(other) { }
    __inline void operator=(const PSyncQueue &) { }
    __inline void push(T * obj) { BaseQueue::push(obj); }
    __inline void pop() { BaseQueue::pop(); }
};


#endif // PTLIB_SYNCTHRD_H


// End Of File ///////////////////////////////////////////////////////////////
