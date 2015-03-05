/*
 * timer.h
 *
 * Real time down counting time interval class.
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

#ifndef PTLIB_TIMER_H
#define PTLIB_TIMER_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib_config.h>

#if P_TIMERS

#include <ptlib/notifier.h>
#include <ptlib/id_generator.h>
#include <ptclib/threadpool.h>

#include <queue>
#include <set>
#include <map>


// To avoid ambiguous operator error we need a one for every integer variant
#define PTIMER_OPERATORS(cls) \
    cls & operator=( int16_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=(uint16_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=( int32_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=(uint32_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=( int64_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=(uint64_t rhs)              { this->SetInterval(rhs); return *this; } \
    cls & operator=(const PTimeInterval & rhs) { this->SetInterval(rhs.GetMilliSeconds()); return *this; } \


/**A class represeting a simple timer.
   Unlike the PTimer class this does not support call back operations, nor is
   it startable and stoppable. It is intended for very simple real time
   operations where an elapsed time from a starting point is required. For
   example:

    <code>
     PSimpleTimer timeout(0, 10); // 10 seconds
     while (!timeout) {
       DoStuff();
     }
    </code>
  */
class PSimpleTimer : public PTimeInterval
{
  PCLASSINFO(PSimpleTimer, PTimeInterval);

  public:
  /**@name Construction */
  //@{
    /** Create a new timer object which will be expired the specified
        time interval after "now" in real time.
      */
    PSimpleTimer(
      long milliseconds = 0,  ///< Number of milliseconds for timer.
      int seconds = 0,        ///< Number of seconds for timer.
      int minutes = 0,        ///< Number of minutes for timer.
      int hours = 0,          ///< Number of hours for timer.
      int days = 0            ///< Number of days for timer.
    );
    PSimpleTimer(
      const PTimeInterval & time    ///< New time interval for timer.
    );
    PSimpleTimer(
      const PSimpleTimer & timer    ///< Timer to copy.
    );

    /** Restart the timer using the specified time value. It will be expired
        the specified time interval after "now" in real time.

       @return
       reference to the timer.
     */
    PSimpleTimer & operator=(
      DWORD milliseconds            ///< New time interval for timer.
    );
    PSimpleTimer & operator=(
      const PTimeInterval & time    ///< New time interval for timer.
    );
    PSimpleTimer & operator=(
      const PSimpleTimer & timer          ///< New time interval for timer.
    );
  //@}

  /**@name Control functions */
  //@{
    /** Set the value of the time interval. The time interval, in milliseconds,
       is the sum of all of the parameters. For example all of the following
       are equivalent:
<pre><code>
              SetInterval(120000)
              SetInterval(60000, 60)
              SetInterval(60000, 0, 1)
              SetInterval(0, 60, 1)
              SetInterval(0, 0, 2)
</code></pre>

       The timer will be expired the specified time interval after "now" in
       real time.
     */
    virtual void SetInterval(
      PInt64 milliseconds = 0,  ///< Number of milliseconds for interval.
      long seconds = 0,         ///< Number of seconds for interval.
      long minutes = 0,         ///< Number of minutes for interval.
      long hours = 0,           ///< Number of hours for interval.
      int days = 0              ///< Number of days for interval.
    );

    /**Stop the timer.
      */
    void Stop();

    /**Return the real time elapsed since instantiation.
      */
    PTimeInterval GetElapsed() const;

    /**Return the real time remaining before expiry.
       If timer has expired, then returns zero.
      */
    PTimeInterval GetRemaining() const;

    /**Indicate timer has not expired.
      */
    bool IsRunning() const;

    /**Indicate timer has expired.
      */
    bool HasExpired() const;

    /**Indicate timer has expired.
      */
    operator bool() const;
  //@}

  protected:
    PTimeInterval m_startTick;
};


/**
   A class representing a system timer. The time interval ancestor value is
   the amount of time left in the timer.

   A timer on completion calls the virtual function <code>OnTimeout()</code>. This
   will in turn call the callback function provided by the instance. The user
   may either override the virtual function or set a callback as desired.
   
   A list of active timers is maintained by the applications <code>PProcess</code> 
   instance and the timeout functions are executed in the context of a separate
   thread of execution.

   Also timers are not very accurate in sub-second delays, even though you can
   set the timer in milliseconds, its accuracy is only to -0/+250 ms. Even
   more (potentially MUCH more) if there are large delays in the user call-back
   functions and many of them exhausting the thread pool.

   When you subclass PTimer you MUST call Stop() in destructor. This guarantees
   that object not destroyed in OnTimeout execution.

   Finally, while the timers "system" threads are fully thread safe, the
   PTimeInterval on which it is based is not. So care is required for user
   threads on platforms where a 64 bit integer access is not an atomic
   operation.
 */
class PTimer : public PTimeInterval
{
  PCLASSINFO_WITH_CLONE(PTimer, PTimeInterval);

  public:

  /**@name Construction */
  //@{
    /** Create a new timer object and start it in one shot mode for the
       specified amount of time. If the time was zero milliseconds then the
       timer is {\b not} started, ie the callback function is not executed
       immediately.
      */
    PTimer(
      long milliseconds = 0,  ///< Number of milliseconds for timer.
      int seconds = 0,        ///< Number of seconds for timer.
      int minutes = 0,        ///< Number of minutes for timer.
      int hours = 0,          ///< Number of hours for timer.
      int days = 0            ///< Number of days for timer.
    );
    PTimer(
      const PTimeInterval & time    ///< New time interval for timer.
    );
    PTimer(
      const PTimer & timer    ///< Timer to copy.
    );

    /** Restart the timer in one shot mode using the specified time value. If
       the timer was already running, the "time left" is simply reset.

       @return
       reference to the timer.
     */
    PTimer & operator=(
      const PTimer & timer          ///< New time interval for timer.
    );

    PTIMER_OPERATORS(PTimer);

    /** Destroy the timer object, stops timer if it is running.
     */
    virtual ~PTimer();
  //@}

  /**@name Overrides from class PObject */
  //@{
    /** Output the time interval to the I/O stream. This outputs the number of
       milliseconds as a signed decimal integer number.
     */
    virtual void PrintOn(
      ostream & strm    ///< I/O stream to output the time interval.
    ) const;
  //@}

  /**@name Control functions */
  //@{
    /** Set the number of milliseconds for the time interval.
        Note, this will restart the timer in the current mode.
    */
    virtual void SetMilliSeconds(PInt64 msecs);

    /** Start a timer in continous cycle mode. Whenever the timer runs out it
       is automatically reset to the time specified. Thus, it calls the
       notification function every time interval.
     */
    void RunContinuous(
      const PTimeInterval & time    // New time interval for timer.
    );

    /** Stop a running timer. The timer will not call the notification function
       and is reset back to the original timer value. Thus when the timer
       is restarted it begins again from the beginning.

       While OnTimeout() is running this method will wait until OnTimeout()
       will finish.
       
       The wait flag is deprecated and not used in this implementation.
       */
    void Stop(
      bool wait = true
    );

    /** Determine if the timer is currently running. This really is only useful
       for one shot timers as repeating timers are always running.
       
       @return
       true if timer is still counting.
     */
    PBoolean IsRunning() const;

    /** Restart a timer continuing from the time it was initially.
     */
    void Reset();

    /** Get the time this timer was set to initially.
     */
    PTimeInterval GetResetTime() const;
  //@}

  /**@name Notification functions */
  //@{
    /**This function is called on time out. That is when the system timer
       processing decrements the timer from a positive value to less than or
       equal to zero. The interval is then reset to zero and the function
       called.

       Please note that the application should not execute large amounts of
       code in this call back or the accuracy of ALL timers can be severely
       impacted.

       The default behaviour of this function is to call the <code>PNotifier</code> 
       callback function.
     */
    virtual void OnTimeout();

    /** Get the current call back function that is called whenever the timer
       expires. This is called by the <code>OnTimeout()</code> function.

       @return
       current notifier for the timer.
     */
    const PNotifier & GetNotifier() const;

    /** Set the call back function that is called whenever the timer expires.
       This is called by the <code>OnTimeout()</code> function.
     */
    void SetNotifier(
      const PNotifier & func  // New notifier function for the timer.
    );
  //@}

  /**@name Global real time functions */
  //@{
    PPROFILE_EXCLUDE(
    /** Get the number of milliseconds since some arbtrary point in time. This
       is a platform dependent function that yields a real time counter.
       
       Note that even though this function returns milliseconds, the value may
       jump in minimum quanta according the platforms timer system, eg under
       MS-DOS and MS-Windows the values jump by 55 every 55 milliseconds. The
       <code>Resolution()</code> function may be used to determine what the minimum
       time interval is.
    
       @return
       millisecond counter.
     */
    static PTimeInterval Tick()
    );

    /** Get the smallest number of milliseconds that the timer can be set to.
       All actual timing events will be rounded up to the next value. This is
       typically the platforms internal timing units used in the <code>Tick()</code>
       function.
       
       @return
       minimum number of milliseconds per timer "tick".
     */
    static unsigned Resolution();
  //@}

  /**@name Member access */
  //@{
    /**Return number of milliseconds left in timer.
      */
    PInt64 GetMilliSeconds() const;
  //@}


    /* This class defines a list of <code>PTimer</code> objects. It is primarily used
       internally by the library and the user should never create an instance of
       it. The <code>PProcess</code> instance for the application maintains an instance
       of all of the timers created so that it may decrements them at regular
       intervals.
     */
    class List
    {
      public:
        // Create a new timer list
        List();

        /* Decrement all the created timers and dispatch to their callback
           functions if they have expired. The <code>PTimer::Tick()</code> function
           value is used to determine the time elapsed since the last call to
           Process().

           The return value is the number of milliseconds until the next timer
           needs to be dispatched. The function need not be called again for this
           amount of time, though it can (and usually is).
       
           @return
           maximum time interval before function should be called again.
         */
        PTimeInterval Process();

      private:
        bool OnTimeout(PIdGenerator::Handle handle);

        struct Timeout
        {
          PIdGenerator::Handle m_handle;
          Timeout(PIdGenerator::Handle handle) : m_handle(handle) { }
          virtual void Work();
        };
        PQueuedThreadPool<Timeout> m_threadPool;

        typedef std::map<PIdGenerator::Handle, PTimer *> TimerMap;
        TimerMap m_timers;
        PMutex   m_timersMutex;

      friend class PTimer;
    };

    static List * TimerList();


  private:
    void InternalStart(bool once, PTimeInterval resetTime); // Note, not "const PTimeInterval &" to avoid mutex issues

    // Member variables
    PNotifier            m_callback;     // Callback function for expired timers.
    bool                 m_oneshot;      // Timer operates once then stops.
    PIdGenerator::Handle m_handle;
    bool                 m_running;
    PTimeInterval        m_absoluteTime;
    mutable PTimedMutex  m_callbackMutex;

    friend class Emitter;

// Include platform dependent part of class
#ifdef _WIN32
#include "msos/ptlib/timer.h"
#else
#include "unix/ptlib/timer.h"
#endif
};


/**Template abstract class for a PTimer that queues a work item to a thread pool.
   This allows for load balancing of timer actions and, in complex muti-threading
   sytems, it is very easy for a deadlock to occur with the usual PNotifier
   method of executing a PTimer call back. This tries to largely avoid the
   issue by the housekeeper thread queuing a PThreadPool "work item" for later
   processing. The thread pooling system has a sophisticated system for
   avoiding deadlocks by making sure a particular target, indicated through a
   string "token", is always handled by the same thread.

   Usage:
     class MyWork1 : public MyWork
     {
       MyWork1(int value);
       virtual void Work();
     };
     PPoolTimerArg1<MyWork1, int, MyWork> timer;

     timer(threadPool, 27)

   Note if more than three parameters are required for the Work class, then you
   will need to use a struct and PPoolTimerArg1<WorkLotsParams, ParamsStruct>
  */
template <
  class Work_T,
  class Pool_T = PQueuedThreadPool<Work_T>
>
class PPoolTimer : public PTimer
{
    PCLASSINFO(PPoolTimer, PTimer);
  protected:
    Pool_T & m_pool;
  public:
    PPoolTimer(Pool_T & pool)
      : m_pool(pool)
    {
    }

    virtual void OnTimeout()
    {
      Work_T * work = CreateWork();
      if (work != NULL)
        m_pool.AddWork(work, GetGroup(*work));
    }

    virtual Work_T * CreateWork() = 0;
    virtual const char * GetGroup(const Work_T & /*work*/) const { return NULL; }

    PTIMER_OPERATORS(PPoolTimer);
};


/// Create a thread pooled timer execution with no parameters to work item.
template <
  class Work_T,
  class Base_T = Work_T,
  class Pool_T = PQueuedThreadPool<Base_T>
>
class PPoolTimerArg0 : public PPoolTimer<Base_T, Pool_T>
{
    typedef PPoolTimer<Base_T, Pool_T> BaseClass;
    PCLASSINFO(PPoolTimerArg0, BaseClass);
  public:
    PPoolTimerArg0(Pool_T & pool)
      : BaseClass(pool)
    {
    }

    virtual Work_T * CreateWork() { return new Work_T(); }

    PTIMER_OPERATORS(PPoolTimerArg0);
};


/// Create a thread pooled timer execution with one parameter to work item.
template <
  class Work_T,
  typename Arg1,
  class Base_T = Work_T,
  class Pool_T = PQueuedThreadPool<Base_T>
>
class PPoolTimerArg1: public PPoolTimer<Base_T, Pool_T>
{
    typedef PPoolTimer<Base_T, Pool_T> BaseClass;
    PCLASSINFO(PPoolTimerArg1, BaseClass);
  protected:
    Arg1 m_arg1;
  public:
    PPoolTimerArg1(Pool_T & pool, Arg1 arg1)
      : BaseClass(pool)
      , m_arg1(arg1)
    {
    }

    virtual Work_T * CreateWork() { return new Work_T(m_arg1); }

    PTIMER_OPERATORS(PPoolTimerArg1);
};


/// Create a thread pooled timer execution with two parameters to work item.
template <
  class Work_T,
  typename Arg1,
  typename Arg2,
  class Base_T = Work_T,
  class Pool_T = PQueuedThreadPool<Base_T>
>
class PPoolTimerArg2: public PPoolTimer<Base_T, Pool_T>
{
    typedef PPoolTimer<Base_T, Pool_T> BaseClass;
    PCLASSINFO(PPoolTimerArg2, BaseClass);
  protected:
    Arg1 m_arg1;
    Arg2 m_arg2;
  public:
    PPoolTimerArg2(Pool_T & pool, Arg1 arg1, Arg2 arg2)
      : BaseClass(pool)
      , m_arg1(arg1)
      , m_arg2(arg2)
    {
    }

    virtual Work_T * CreateWork() { return new Work_T(m_arg1, m_arg2); }

    PTIMER_OPERATORS(PPoolTimerArg2);
};


/// Create a thread pooled timer execution with three parameters to work item.
template <
  class Work_T,
  typename Arg1,
  typename Arg2,
  typename Arg3,
  class Base_T = Work_T,
  class Pool_T = PQueuedThreadPool<Base_T>
>
class PPoolTimerArg3: public PPoolTimer<Base_T, Pool_T>
{
    typedef PPoolTimer<Base_T, Pool_T> BaseClass;
    PCLASSINFO(PPoolTimerArg3, BaseClass);
  protected:
    Arg1 m_arg1;
    Arg2 m_arg2;
    Arg3 m_arg3;
  public:
    PPoolTimerArg3(Pool_T & pool, Arg1 arg1, Arg2 arg2, Arg3 arg3)
      : BaseClass(pool)
      , m_arg1(arg1)
      , m_arg2(arg2)
      , m_arg3(arg3)
    {
    }

    virtual Work_T * CreateWork() { return new Work_T(m_arg1, m_arg2, m_arg3); }

    PTIMER_OPERATORS(PPoolTimerArg3);
};


#else

namespace PTimer
{
  PTimeInterval Tick();
  unsigned Resolution();
};

#endif // P_TIMERS

#endif // PTLIB_TIMER_H


// End Of File ///////////////////////////////////////////////////////////////
