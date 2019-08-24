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

class PReadWriteMutex : public PObject, public PMutexExcessiveLockInfo
{
  PCLASSINFO(PReadWriteMutex, PObject);
  public:
  /**@name Construction */
  //@{
    explicit PReadWriteMutex();
    explicit PReadWriteMutex(
      const PDebugLocation & location, ///< Source file/line of mutex definition
      unsigned timeout = 0             ///< Timeout in ms, before declaring a possible deadlock. Zero uses default.
    );
    ~PReadWriteMutex();
  //@}

  /**@name Operations */
  //@{
    /** This function attempts to acquire the mutex for reading.
        This call may be nested and must have an equal number of EndRead()
        calls for the mutex to be released.
     */
    __inline void StartRead() { InternalStartRead(NULL); }
    __inline void StartRead(const PDebugLocation & location) { InternalStartRead(&location); }

    /** This function attempts to release the mutex for reading.
     */
    __inline void EndRead() { InternalEndRead(NULL); }
    __inline void EndRead(const PDebugLocation & location) { InternalEndRead(&location); }

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
    __inline void StartWrite() { InternalStartWrite(NULL); }
    __inline void StartWrite(const PDebugLocation & location) { InternalStartWrite(&location); }

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
    void EndWrite() { InternalEndWrite(NULL); }
    void EndWrite(const PDebugLocation & location) { InternalEndWrite(&location); }
  //@}

    virtual void PrintOn(ostream &strm) const;

  protected:
    void InternalStartRead(const PDebugLocation * location);
    void InternalEndRead(const PDebugLocation * location);
    void InternalStartWrite(const PDebugLocation * location);
    void InternalEndWrite(const PDebugLocation * location);

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
      uint64_t m_startHeldCycle;
      PUniqueThreadIdentifier m_uniqueId;

      Nest()
        : m_readerCount(0)
        , m_writerCount(0)
        , m_waiting(false)
        , m_startHeldCycle(0)
        , m_uniqueId(PThread::GetCurrentUniqueIdentifier())
      { }
    };
    typedef std::map<PThreadIdentifier, Nest> NestMap;
    NestMap          m_nestedThreads;
    PCriticalSection m_nestingMutex;

    Nest * GetNest();
    Nest & StartNest();
    void EndNest();
    void InternalStartReadWithNest(Nest & nest, const PDebugLocation & location);
    void InternalEndReadWithNest(Nest & nest, const PDebugLocation & location);
    void InternalStartWriteWithNest(Nest & nest, const PDebugLocation & location);
    void InternalEndWriteWithNest(Nest & nest, const PDebugLocation & location);
    void InternalWait(Nest & nest, PSync & sync, const PDebugLocation & location) const;

  private:
    PReadWriteMutex(const PReadWriteMutex & other) : PObject(other) { }
    void operator=(const PReadWriteMutex &) { }

  friend class PSafeObject;
  friend class PReadWaitAndSignal;
  friend class PWriteWaitAndSignal;
  friend class PInstrumentedReadWaitAndSignal;
  friend class PInstrumentedWriteWaitAndSignal;
};

/// Declare a PReadWriteMutex with compiled file/line for deadlock debugging
#define PDECLARE_READ_WRITE_MUTEX_ARG_1(var)              struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(P_DEBUG_LOCATION) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_2(var,nam)          struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,           ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_3(var,nam,to)       struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,to         ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_4(var,nam,to,tw)    struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,to,tw      ) { } } var
#define PDECLARE_READ_WRITE_MUTEX_ARG_5(var,nam,to,tw,th) struct PReadWriteMutex_##var : PReadWriteMutex { PReadWriteMutex_##var() : PReadWriteMutex(#nam,to,tw,th   ) { } } var

#define PDECLARE_READ_WRITE_MUTEX_PART1(narg, args) PDECLARE_READ_WRITE_MUTEX_PART2(narg, args)
#define PDECLARE_READ_WRITE_MUTEX_PART2(narg, args) PDECLARE_READ_WRITE_MUTEX_ARG_##narg args

#define PDECLARE_READ_WRITE_MUTEX(...) PDECLARE_READ_WRITE_MUTEX_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))

class PReadWriteWaitAndSignalBase
{
  protected:
    typedef void (PReadWriteMutex:: * StartFn)(const PDebugLocation * location);
    typedef void (PReadWriteMutex:: * EndFn)(const PDebugLocation * location);

    PReadWriteWaitAndSignalBase(const PReadWriteMutex & mutex, const PDebugLocation * location, StartFn start, EndFn end)
      : m_mutex(const_cast<PReadWriteMutex &>(mutex))
      , m_location(location)
      , m_end(end)
    {
      if (start)
        (m_mutex.*start)(&m_location);
    }

  public:
    ~PReadWriteWaitAndSignalBase()
    {
      (m_mutex.*m_end)(&m_location);
    }

  protected:
    PReadWriteMutex & m_mutex;
    PDebugLocation    m_location;
    EndFn             m_end;
};



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
class PReadWaitAndSignal : public PReadWriteWaitAndSignalBase
{
  public:
    /**Create the PReadWaitAndSignal wait instance.
       This will wait on the specified PReadWriteMutex using the StartRead()
       function before returning.
      */
    PReadWaitAndSignal(
      const PReadWriteMutex & mutex,  ///< PReadWriteMutex descendent to wait/signal.
      bool start = true               ///< Start read operation on PReadWriteMutex before returning.
    ) : PReadWriteWaitAndSignalBase(mutex, NULL, start ? &PReadWriteMutex::InternalStartRead : NULL, &PReadWriteMutex::InternalEndRead) { }
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
class PWriteWaitAndSignal : public PReadWriteWaitAndSignalBase
{
  public:
    /**Create the PWriteWaitAndSignal wait instance.
       This will wait on the specified PReadWriteMutex using the StartWrite()
       function before returning.
      */
    PWriteWaitAndSignal(
      const PReadWriteMutex & mutex,  ///< PReadWriteMutex descendent to wait/signal.
      PBoolean start = true           ///< Start write operation on PReadWriteMutex before returning.
    ) : PReadWriteWaitAndSignalBase(mutex, NULL, start ? &PReadWriteMutex::InternalStartWrite : NULL, &PReadWriteMutex::InternalEndWrite) { }
};


#if PTRACING
  class PInstrumentedReadWriteMutex : public PReadWriteMutex
  {
    public:
      PInstrumentedReadWriteMutex(
        const char * baseName,
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
      ) : PReadWriteMutex           (PDebugLocation(file, line, baseName         ), MinDeadlockTime(waitTime))
        , m_timeWaitReadOnlyContext (PDebugLocation(file, line, waitReadOnlyName ), waitTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeHeldReadOnlyContext (PDebugLocation(file, line, heldReadOnlyName ), heldTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeWaitReadWriteContext(PDebugLocation(file, line, waitReadWriteName), waitTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeHeldReadWriteContext(PDebugLocation(file, line, heldReadWriteName), heldTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
      { }

      void SetWaitReadOnlyThrottleTime(unsigned throttleTime) { m_timeWaitReadOnlyContext.SetThrottleTime(throttleTime); }
      void SetWaitReadOnlyThrottledLogLevel(unsigned throttledLogLevel) { m_timeWaitReadOnlyContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetWaitReadOnlyUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeWaitReadOnlyContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetWaitReadOnlyThresholdPercent(unsigned thresholdPercent) { m_timeWaitReadOnlyContext.SetThresholdPercent(thresholdPercent); }
      void SetWaitReadOnlyMaxHistory(unsigned maxHistory) { m_timeWaitReadOnlyContext.SetMaxHistory(maxHistory); }

      void SetHeldReadOnlyThrottleTime(unsigned throttleTime) { m_timeHeldReadOnlyContext.SetThrottleTime(throttleTime); }
      void SetHeldReadOnlyThrottledLogLevel(unsigned throttledLogLevel) { m_timeHeldReadOnlyContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetHeldReadOnlyUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeHeldReadOnlyContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetHeldReadOnlyThresholdPercent(unsigned thresholdPercent) { m_timeHeldReadOnlyContext.SetThresholdPercent(thresholdPercent); }
      void SetHeldReadOnlyMaxHistory(unsigned maxHistory) { m_timeHeldReadOnlyContext.SetMaxHistory(maxHistory); }

      void SetWaitReadWriteThrottleTime(unsigned throttleTime) { m_timeWaitReadWriteContext.SetThrottleTime(throttleTime); }
      void SetWaitReadWriteThrottledLogLevel(unsigned throttledLogLevel) { m_timeWaitReadWriteContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetWaitReadWriteUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeWaitReadWriteContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetWaitReadWriteThresholdPercent(unsigned thresholdPercent) { m_timeWaitReadWriteContext.SetThresholdPercent(thresholdPercent); }
      void SetWaitReadWriteMaxHistory(unsigned maxHistory) { m_timeWaitReadWriteContext.SetMaxHistory(maxHistory); }

      void SetHeldReadWriteThrottleTime(unsigned throttleTime) { m_timeHeldReadWriteContext.SetThrottleTime(throttleTime); }
      void SetHeldReadWriteThrottledLogLevel(unsigned throttledLogLevel) { m_timeHeldReadWriteContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetHeldReadWriteUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeHeldReadWriteContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetHeldReadWriteThresholdPercent(unsigned thresholdPercent) { m_timeHeldReadWriteContext.SetThresholdPercent(thresholdPercent); }
      void SetHeldReadWriteMaxHistory(unsigned maxHistory) { m_timeHeldReadWriteContext.SetMaxHistory(maxHistory); }

    protected:
      virtual void AcquiredLock(uint64_t startWaitCycle, bool readOnly, const PDebugLocation & location);
      virtual void ReleasedLock(const PObject & mutex, uint64_t startHeldSamplePoint, bool readOnly, const PDebugLocation & location);

      PProfiling::TimeScope m_timeWaitReadOnlyContext;
      PProfiling::TimeScope m_timeHeldReadOnlyContext;
      PProfiling::TimeScope m_timeWaitReadWriteContext;
      PProfiling::TimeScope m_timeHeldReadWriteContext;
  };

  class PInstrumentedReadWaitAndSignal : public PReadWriteWaitAndSignalBase
  {
    public:
      PInstrumentedReadWaitAndSignal(
        const PReadWriteMutex & mutex,
        const PDebugLocation & location,
        bool start = true
      ) : PReadWriteWaitAndSignalBase(mutex, &location, start ? &PReadWriteMutex::InternalStartRead : NULL, &PReadWriteMutex::InternalEndRead) { }
  };

  class PInstrumentedWriteWaitAndSignal : public PReadWriteWaitAndSignalBase
  {
    public:
      PInstrumentedWriteWaitAndSignal(
        const PReadWriteMutex & mutex,
        const PDebugLocation & location,
        PBoolean start = true
      ) : PReadWriteWaitAndSignalBase(mutex, &location, start ? &PReadWriteMutex::InternalStartWrite : NULL, &PReadWriteMutex::InternalEndWrite) { }
  };

  #define PDECLARE_INSTRUMENTED_READ_WRITE_MUTEX(var, name, ...)                \
    struct PInstrumentedReadWriteMutex_##name : PInstrumentedReadWriteMutex {   \
      PInstrumentedReadWriteMutex_##name(const char * mutexName = #name)        \
            : PInstrumentedReadWriteMutex(mutexName, __FILE__, __LINE__,        \
                                          "Wait R/O " #name, "Held R/O " #name, \
                                          "Wait R/W " #name, "Held R/W " #name, \
                                          __VA_ARGS__) { }                      \
    } var
#else
  #define PDECLARE_INSTRUMENTED_READ_WRITE_MUTEX(var, name, waitTime, heldTime, ...) \
                       PDECLARE_READ_WRITE_MUTEX(var, name, MinDeadlockTime(waitTime))
#endif


/** A synchronous queue of objects.
    This implements a queue of objects between threads. The dequeue action will
    always block until an object is placed into the queue.
  */
template <class T> class PSyncQueue : public PObject
{
    PCLASSINFO(PSyncQueue, PObject);
  public:
    typedef std::queue<T> BaseQueue;

    enum State
    {
      e_Open,
      e_Blocked,
      e_Draining,
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
      if (m_state == e_Closed || m_state == e_Draining)
        return false;
      m_queue.push(obj);
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
          PAssertAlways("Multiple threads in PSyncQueue::Dequeue()");
          break;

        case e_Open :
          m_state = e_Blocked;

          {
            m_mutex.Signal();
            bool available = m_available.Wait(timeout);
            m_mutex.Wait();

            if (available && !m_queue.empty()) {
              value = m_queue.front();
              m_queue.pop();
              dequeued = true;
            }
          }

          if (m_state == e_Blocked) {
            m_state = e_Open;
            break;
          }
          if (m_state == e_Draining && m_queue.empty())
            m_state = e_Closed;     // Just popped the last item
          if (m_state == e_Closed)
            m_closed.Signal();
          break;

        case e_Draining:
          if (!m_queue.empty()) {
            // Continue draining
            value = m_queue.front();
            m_queue.pop();
            dequeued = true;
            break;  
          }
          m_state = e_Closed;
          // Do closed case

        case e_Closed :
          m_closed.Signal();
      }

      m_mutex.Signal();

      return dequeued;
    }

    /** Begin graceful draining of the queue. No further Enqueues will be
        accepted, and the queue will close automatically once empty.
        This may optionally wait for Dequeue() to exit before returning. */
    void Drain(bool wait)
    {
      bool blocked;
      {
        PWaitAndSignal mutex(m_mutex);
        if (m_state == e_Closed || m_state == e_Draining)
          return;

        blocked = m_state == e_Blocked;
        m_state = m_queue.empty() ? e_Closed : e_Draining;
        m_available.Signal();
      }

      if (blocked && wait)
        m_closed.Wait();
    }

    /** Close the queue and break block in Dequeue() function.
        This may optionally wait for Dequeue() to exit before returning.
      */
    void Close(bool wait)
    {
      m_mutex.Wait();

      bool blocked = m_state == e_Blocked;
      m_state = e_Closed;

      while (!m_queue.empty())
        m_queue.pop();

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
        while (!m_queue.empty())
          m_queue.pop();
        while (m_available.Wait(0))
          ;
        m_state = e_Open;
      }
      m_mutex.Signal();
    }


    /// Get the current size of the queue
    size_t size() const
    {
      PWaitAndSignal lock(m_mutex);
      return m_queue.size();
    }


    /// Determine if queue is empty
    bool empty() const
    {
      PWaitAndSignal lock(m_mutex);
      return m_queue.empty();
    }


    __inline const PMutex & GetMutex() const { return m_mutex; }
    __inline       PMutex & GetMutex()       { return m_mutex; }

  protected:
    BaseQueue      m_queue;
    State          m_state;
    PSemaphore     m_available;
    PDECLARE_MUTEX(m_mutex);
    PSyncPoint     m_closed;

  private:
    __inline PSyncQueue(const PSyncQueue & other) : PObject(other) { }
    __inline void operator=(const PSyncQueue &) { }
};


#endif // PTLIB_SYNCTHRD_H


// End Of File ///////////////////////////////////////////////////////////////
