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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_MUTEX_H
#define PTLIB_MUTEX_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/atomic.h>
#include <ptlib/semaphor.h>

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

class PTimedMutex : public PSync
{
    PCLASSINFO(PTimedMutex, PSync)
  public:
    /* Create a new mutex.
       Initially the mutex will not be "set", so the first call to Wait() will
       never wait.

       The name/line parameters are used for deadlock detection debugging.
     */
    PTimedMutex(
        const char * name = NULL,  ///< Arbitrary name, or filename of mutex variable declaration
        unsigned line = 0          ///< Line number, if non zero, name is assumed to be a filename
    );

    /**Copy constructor is allowed but does not copy, allocating a new mutex.
       Two copies of the same mutex information would be very bad.
      */
    PTimedMutex(const PTimedMutex & mutex);

    /**Assignment operator is allowed but does nothing.
       Overwriting the old mutex information would be very bad.
      */
    PTimedMutex & operator=(const PTimedMutex &) { return *this; }

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

  protected:
    PThreadIdentifier       m_lockerId;
    PThreadIdentifier       m_lastLockerId;
    atomic<uint32_t>        m_lockCount;
    PUniqueThreadIdentifier m_uniqueId;
    bool                    m_excessiveLockTime;
    PString                 m_name;
    unsigned                m_line;

    void ExcessiveLockWait();
    void CommonSignal();

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/mutex.h"
#else
#include "unix/ptlib/mutex.h"
#endif
};

typedef PTimedMutex PMutex;

/// Declare a PReadWriteMutex with compiled file/line for deadlock debugging
#define PDECLARE_MUTEX(var) struct PTimedMutex_##var : PTimedMutex { PTimedMutex_##var() : PTimedMutex(__FILE__,__LINE__) { } } var
#define PDECLARE_MUTEX2(var, name) struct PTimedMutex_##var : PTimedMutex { PTimedMutex_##var() : PTimedMutex(name) { } } var


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
