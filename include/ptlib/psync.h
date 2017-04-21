/*
 * psync.h
 *
 * Abstract synchronisation semaphore class.
 *
 * Portable Tools Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 * Copyright (c) 2005 Post Increment
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

#ifndef PTLIB_SYNC_H
#define PTLIB_SYNC_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/contain.h>
#include <ptlib/object.h>


class PTimeInterval;


class PSync : public PObject
{
  public:
    PSync() { }

  /**@name Operations */
  //@{
    /**Block until the synchronisation object is available.
     */
    virtual void Wait() = 0;

    /**Block, for a time, until the synchronisation object is available.

       @return
       true if lock is acquired, false if timed out
     */
    virtual PBoolean Wait(
      const PTimeInterval & timeout // Amount of time to wait.
    ) = 0;

    /// As for Wait() but with location of call for instrumentation. Mostly used internally.
    virtual bool InstrumentedWait(const PTimeInterval & timeout, const PDebugLocation & /*location*/) { return Wait(timeout); }

     /**Signal that the synchronisation object is available.
     */
    virtual void Signal() = 0;

    /// As for Signal() but with location of call for instrumentation. Mostly used internally.
    virtual void InstrumentedSignal(const PDebugLocation & /*location*/) { Signal(); }
  //@}

  private:
    PSync(const PSync &) : PObject() { }
    void operator=(const PSync &) { }
};


/// Synchronisation without really synchronising.
class PSyncNULL : public PSync
{
  public:
    PSyncNULL() { }
    virtual void Wait() { }
    virtual PBoolean Wait(const PTimeInterval &) { return true; }
    virtual void Signal() { }

  private:
    PSyncNULL(const PSyncNULL &) : PSync() { }
    void operator=(const PSyncNULL &) { }
};


/**This class waits for the semaphore on construction and automatically
   signals the semaphore on destruction. Any descendent of PSemaphore
   may be used.

  This is very useful for constructs such as:
<pre><code>
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
</code></pre>
 */
class PWaitAndSignal
{
  public:
    /**Create the semaphore wait instance.
       This will wait on the specified semaphore using the Wait() function
       before returning.
      */
    __inline explicit PWaitAndSignal(
      const PSync & sem   ///< Semaphore descendent to wait/signal.
    ) : sync(const_cast<PSync &>(sem))
    {
      sync.Wait();
    }

    __inline PWaitAndSignal(
      const PSync & sem,  ///< Semaphore descendent to wait/signal.
      bool wait           ///< Wait for semaphore before returning.
    ) : sync(const_cast<PSync &>(sem))
    {
      if (wait)
        sync.Wait();
    }

    /** Signal the semaphore.
        This will execute the Signal() function on the semaphore that was used
        in the construction of this instance.
     */
    __inline ~PWaitAndSignal()
    {
      sync.Signal();
    }

  protected:
    PSync & sync;
};

#if PTRACING
class PInstrumentedWaitAndSignal
{
  public:
    /**Create the semaphore wait instance.
       This will wait on the specified semaphore using the Wait() function
       before returning.
      */
    __inline explicit PInstrumentedWaitAndSignal(
      const PDebugLocation & location, ///< Source file/line for instance
      const PSync & sem   ///< Semaphore descendent to wait/signal.
    ) : m_location(location)
      , sync(const_cast<PSync &>(sem))
    {
      sync.InstrumentedWait(PMaxTimeInterval, m_location);
    }

    __inline PInstrumentedWaitAndSignal(
      const PDebugLocation & location, ///< Source file/line for instance
      const PSync & sem,  ///< Semaphore descendent to wait/signal.
      bool wait           ///< Wait for semaphore before returning.
    ) : m_location(location)
      , sync(const_cast<PSync &>(sem))
    {
      if (wait)
        sync.InstrumentedWait(PMaxTimeInterval, m_location);
    }

    /** Signal the semaphore.
        This will execute the Signal() function on the semaphore that was used
        in the construction of this instance.
     */
    __inline ~PInstrumentedWaitAndSignal()
    {
      sync.InstrumentedSignal(m_location);
    }

  protected:
    PDebugLocation const m_location;
    PSync & sync;
};

#define P_INSTRUMENTED_WAIT_AND_SIGNAL2(var, mutex) PInstrumentedWaitAndSignal var(P_DEBUG_LOCATION, mutex)
#else // P_TRACING
#define P_INSTRUMENTED_WAIT_AND_SIGNAL2(var, mutex) PWaitAndSignal var(mutex)
#endif // P_TRACING
#define P_INSTRUMENTED_WAIT_AND_SIGNAL(mutex) P_INSTRUMENTED_WAIT_AND_SIGNAL2(lock,mutex)


#endif // PTLIB_SYNC_H


// End Of File ///////////////////////////////////////////////////////////////
