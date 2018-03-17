/*
 * mutex.h
 *
 * Mutual exclusion thread synchonisation class.
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
 */

#ifndef PTLIB_MUTEX_H
#define PTLIB_MUTEX_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/atomic.h>
#include <ptlib/semaphor.h>


class PMutexExcessiveLockInfo
{
  protected:
    PDebugLocation m_location;
    unsigned       m_excessiveLockTimeout;
    mutable bool   m_excessiveLockActive;
    uint64_t       m_startHeldSamplePoint;

    PMutexExcessiveLockInfo();
    PMutexExcessiveLockInfo(
      const PDebugLocation & location,
      unsigned timeout
    );
    PMutexExcessiveLockInfo(const PMutexExcessiveLockInfo & other);
    virtual ~PMutexExcessiveLockInfo() { }
    void Construct(unsigned timeout);

#if PTRACING
    void Constructed(const PObject & mutex) const;
    void Destroyed(const PObject & mutex) const;
    #define PMUTEX_CONSTRUCTED() Constructed(*this)
    #define PMUTEX_DESTROYED() Destroyed(*this)
#else
    #define PMUTEX_CONSTRUCTED()
    #define PMUTEX_DESTROYED()
#endif

    void PrintOn(ostream &strm) const;
    void ExcessiveLockPhantom(const PObject & mutex) const;
    virtual void AcquiredLock(uint64_t startWaitCycle, bool readOnly, const PDebugLocation & location);
    virtual void ReleasedLock(const PObject & mutex, uint64_t startHeldSamplePoint, bool readOnly, const PDebugLocation & location);

    static unsigned MinDeadlockTime(unsigned waitTime);

  public:
    void SetLocationName(const char * name) { m_location.m_extra = name; }
};


/**This class defines a thread mutual exclusion object. A mutex is where a
   piece of code or data cannot be accessed by more than one thread at a time.
   To prevent this the PMutex is used in the following manner:
<pre><code>
      PMutex mutex;

      ...

      mutex.Wait();

      ... critical section - only one thread at a time here.

      mutex.Signal();

      ...
</code></pre>
    The first thread will pass through the <code>Wait()</code> function, a second
    thread will block on that function until the first calls the
    <code>Signal()</code> function, releasing the second thread.
 */

class PTimedMutex : public PSync, public PMutexExcessiveLockInfo
{
    PCLASSINFO(PTimedMutex, PSync)
  public:
    /* Create a new mutex.
       Initially the mutex will not be "set", so the first call to Wait() will
       never wait.

       The name/line parameters are used for deadlock detection debugging.
     */
    explicit PTimedMutex();
    explicit PTimedMutex(
      const PDebugLocation & location, ///< Source file/line of mutex definition
      unsigned timeout = 0             ///< Timeout in ms, before declaring a possible deadlock. Zero uses default.
    );

    /**Copy constructor is allowed but does not copy, allocating a new mutex.
       Two copies of the same mutex information would be very bad.
      */
    PTimedMutex(const PTimedMutex & mutex);

    /**Assignment operator is allowed but does nothing.
       Overwriting the old mutex information would be very bad.
      */
    PTimedMutex & operator=(const PTimedMutex &) { return *this; }

    /// Destruction
    ~PTimedMutex();

    /**Block until the synchronisation object is available.
     */
    virtual void Wait();

    /**Block, for a time, until the synchronisation object is available.

       @return
       true if lock is acquired, false if timed out
     */
    virtual PBoolean Wait(
      const PTimeInterval & timeout // Amount of time to wait.
    );

    /**Signal that the synchronisation object is available.
     */
    virtual void Signal();

    /** Try to enter the critical section for exlusive access. Does not wait.
        @return true if cirical section entered, leave/Signal must be called.
      */
    PINLINE bool Try() { return Wait(0); }

    virtual void PrintOn(ostream &strm) const;

    static unsigned ExcessiveLockWaitTime;
    enum DeadlockStackWalkModes
    {
        DeadlockStackWalkDisabled,
        DeadlockStackWalkEnabled,
        DeadlockStackWalkOnPhantomRelease,
        DeadlockStackWalkNoSymbols
    };
    static DeadlockStackWalkModes DeadlockStackWalkMode;
#if PTRACING
    static unsigned CtorDtorLogLevel;
#endif

  protected:
    PThreadIdentifier       m_lockerId;
    PThreadIdentifier       m_lastLockerId;
    PUniqueThreadIdentifier m_lastUniqueId;
    unsigned                m_lockCount;

    void Construct();
    void PlatformConstruct();
    bool PlatformWait(const PTimeInterval & timeout);
    void PlatformSignal(const PDebugLocation * location);
    void InternalWait(const PDebugLocation * location);
    void InternalWaitComplete(uint64_t startWaitCycle, const PDebugLocation * location);
    bool InternalSignal(const PDebugLocation * location);

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/mutex.h"
#else
#include "unix/ptlib/mutex.h"
#endif
};

typedef PTimedMutex PMutex;

/// Declare a PReadWriteMutex with compiled file/line for deadlock debugging

#define PDECLARE_MUTEX_ARG_1(var)        struct PTimedMutex_##var : PTimedMutex { PTimedMutex_##var() : PTimedMutex(P_DEBUG_LOCATION) { } } var
#define PDECLARE_MUTEX_ARG_2(var,nam)    struct PTimedMutex_##var : PTimedMutex { PTimedMutex_##var() : PTimedMutex(#nam            ) { } } var
#define PDECLARE_MUTEX_ARG_3(var,nam,to) struct PTimedMutex_##var : PTimedMutex { PTimedMutex_##var() : PTimedMutex(#nam,to         ) { } } var

#define PDECLARE_MUTEX_PART1(narg, args) PDECLARE_MUTEX_PART2(narg, args)
#define PDECLARE_MUTEX_PART2(narg, args) PDECLARE_MUTEX_ARG_##narg args

#define PDECLARE_MUTEX(...) PDECLARE_MUTEX_PART1(PARG_COUNT(__VA_ARGS__), (__VA_ARGS__))

#if PTRACING
  class PInstrumentedMutex : PDebugLocation, public PTimedMutex
  {
    public:
      PInstrumentedMutex(
        const char * baseName,
        const char * file,
        unsigned line,
        const char * waitName,
        const char * heldName,
        unsigned waitTime,
        unsigned heldTime,
        unsigned throttleTime = 10000,    ///< Time between PTRACE outpout in milliseconds
        unsigned throttledLogLevel = 2,   ///< PTRACE level to use if enough samples are above thresholdTime
        unsigned unthrottledLogLevel = 6, ///< PTRACE level to use otherwise
        unsigned thresholdPercent = 5,    ///< Percentage of samples above thresholdTime to trigger throttledLogLevel
        unsigned maxHistory = 0           ///< Optional number of samples above thresholdTime to display sincle last PTRACE()
      ) : PDebugLocation(file, line, baseName)
        , PTimedMutex(this, MinDeadlockTime(waitTime))
        , m_timeWaitContext(PDebugLocation(file, line, waitName), waitTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
        , m_timeHeldContext(PDebugLocation(file, line, heldName), heldTime, throttleTime, throttledLogLevel, unthrottledLogLevel, thresholdPercent, maxHistory)
      { }

      void SetWaitThrottleTime(unsigned throttleTime) { m_timeWaitContext.SetThrottleTime(throttleTime); }
      void SetWaitThrottledLogLevel(unsigned throttledLogLevel) { m_timeWaitContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetWaitUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeWaitContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetWaitThresholdPercent(unsigned thresholdPercent) { m_timeWaitContext.SetThresholdPercent(thresholdPercent); }
      void SetWaitMaxHistory(unsigned maxHistory) { m_timeWaitContext.SetMaxHistory(maxHistory); }

      void SetHeldThrottleTime(unsigned throttleTime) { m_timeHeldContext.SetThrottleTime(throttleTime); }
      void SetHeldThrottledLogLevel(unsigned throttledLogLevel) { m_timeHeldContext.SetThrottledLogLevel(throttledLogLevel); }
      void SetHeldUnthrottledLogLevel(unsigned unthrottledLogLevel) { m_timeHeldContext.SetUnthrottledLogLevel(unthrottledLogLevel); }
      void SetHeldThresholdPercent(unsigned thresholdPercent) { m_timeHeldContext.SetThresholdPercent(thresholdPercent); }
      void SetHeldMaxHistory(unsigned maxHistory) { m_timeHeldContext.SetMaxHistory(maxHistory); }

      virtual bool InstrumentedWait(const PTimeInterval & timeout, const PDebugLocation & location);
      virtual void InstrumentedSignal(const PDebugLocation & location) { PlatformSignal(&location); }

    protected:
      virtual void AcquiredLock(uint64_t startWaitCycle, bool readOnly, const PDebugLocation & location);
      virtual void ReleasedLock(const PObject & mutex, uint64_t startHeldSamplePoint, bool readOnly, const PDebugLocation & location);

      PProfiling::TimeScope m_timeWaitContext;
      PProfiling::TimeScope m_timeHeldContext;
  };

  #define PDECLARE_INSTRUMENTED_MUTEX(var, name, ...)           \
    struct PInstrumentedMutex_##name : PInstrumentedMutex {     \
      PInstrumentedMutex_##name(const char * mutexName = #name) \
            : PInstrumentedMutex(mutexName, __FILE__, __LINE__, \
                                 "Wait " #name, "Held " #name,  \
                                 __VA_ARGS__) { }               \
    } var

#else
  #define PDECLARE_INSTRUMENTED_MUTEX(var, name, waitTime, heldTime, ...) \
                       PDECLARE_MUTEX(var, name, MinDeadlockTime(waitTime))
#endif


/** This class implements critical section mutexes using the most efficient
    mechanism available on the host platform.
    For example in Windows, a CRITICAL_SECTION is used.

    Note: There is no deadlock detection available on this version of a mutex,
    so it should only be used where a deadlock is clearly impossible, that is,
    there are never mopre than one mutex in the region being locked.
  */
class PCriticalSection : public PSync
{
  PCLASSINFO(PCriticalSection, PSync);

  public:
  /**@name Construction */
  //@{
    /**Create a new critical section object .
     */
    PCriticalSection();

    /**Allow copy constructor, but it actually does not copy the critical section,
       it creates a brand new one as they cannot be shared in that way.
     */
    PCriticalSection(const PCriticalSection &);

    /**Destroy the critical section object
     */
    ~PCriticalSection();

    /**Assignment operator is allowed but does nothing. Overwriting the old critical
       section information would be very bad.
      */
    PCriticalSection & operator=(const PCriticalSection &) { return *this; }
  //@}

  /**@name Operations */
  //@{
    /** Create a new PCriticalSection
      */
    PObject * Clone() const
    {
      return new PCriticalSection();
    }

    /** Enter the critical section by waiting for exclusive access.
     */
    virtual void Wait();
    inline void Enter() { Wait(); }

    /**Block, for a time, until the synchronisation object is available.

       @return
       true if lock is acquired, false if timed out
     */
    virtual PBoolean Wait(
      const PTimeInterval & timeout // Amount of time to wait.
    );

    /** Leave the critical section by unlocking the mutex
     */
    virtual void Signal();
    inline void Leave() { Signal(); }

    /** Try to enter the critical section for exlusive access. Does not wait.
        @return true if cirical section entered, leave/Signal must be called.
      */
    bool Try();
  //@}

#if _WIN32
    mutable CRITICAL_SECTION criticalSection;
#elif defined(P_PTHREADS) || defined(VX_TASKS)
    mutable pthread_mutex_t m_mutex;
#endif
};


#endif // PTLIB_MUTEX_H


// End Of File ///////////////////////////////////////////////////////////////
