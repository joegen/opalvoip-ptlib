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
 * $Log: timer.h,v $
 * Revision 1.18  2000/01/06 14:09:42  robertj
 * Fixed problems with starting up timers,losing up to 10 seconds
 *
 * Revision 1.17  1999/03/09 02:59:51  robertj
 * Changed comments to doc++ compatible documentation.
 *
 * Revision 1.16  1999/02/16 08:11:17  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.15  1998/09/23 06:21:45  robertj
 * Added open source copyright license.
 *
 * Revision 1.14  1996/12/21 07:57:22  robertj
 * Fixed possible deadlock in timers.
 *
 * Revision 1.13  1996/05/18 09:18:37  robertj
 * Added mutex to timer list.
 *
 * Revision 1.12  1995/06/17 11:13:36  robertj
 * Documentation update.
 *
 * Revision 1.11  1995/04/02 09:27:34  robertj
 * Added "balloon" help.
 *
 * Revision 1.10  1995/03/14 12:42:51  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.9  1995/01/18  09:01:06  robertj
 * Added notifiers to timers.
 * Documentation.
 *
 * Revision 1.8  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.7  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.6  1994/07/02  03:03:49  robertj
 * Redesign of timers.
 *
 * Revision 1.5  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.4  1994/03/07  07:38:19  robertj
 * Major enhancementsacross the board.
 *
 * Revision 1.3  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.2  1993/08/31  03:38:02  robertj
 * Added missing virtual on destructor.
 *
 * Revision 1.1  1993/08/27  18:17:47  robertj
 * Initial revision
 *
 */


#define _PTIMER

#ifdef __GNUC__
#pragma interface
#endif


class PThread;

/**
   A class representing a system timer. The time interval ancestor value is
   the amount of time left in the timer.

   A timer on completion calls the virtual function #OnTimeout()#. This
   will in turn call the callback function provided by the instance. The user
   may either override the virtual function or set a callback as desired.
   
   Note that only one timeout function can be executed at a time. The timeout
   function is also executed in the context of the #PProcess# instances
   thread of execution.
   
   A list of active timers is maintained by the applications #PProcess# 
   instance. This is used for sstealing the processor time to decrement the
   timers and call the timeout functions. A consequence of this is that no
   static timer instances can be running when the program terminates.
 */
class PTimer : public PTimeInterval
{
  PCLASSINFO(PTimer, PTimeInterval);

  public:
  /**@name Construction */
  //@{
    /** Create a new timer object and start it in one shot mode for the
       specified amount of time. If the time was zero milliseconds then the
       timer is {\bf not} started, ie the callback function is not executed
       immediately.
      */
    PTimer(
      long milliseconds = 0,  /// Number of milliseconds for timer.
      int seconds = 0,        /// Number of seconds for timer.
      int minutes = 0,        /// Number of minutes for timer.
      int hours = 0,          /// Number of hours for timer.
      int days = 0            /// Number of days for timer.
    );
    PTimer(
      const PTimeInterval & time    /// New time interval for timer.
    );

    /** Restart the timer in one shot mode using the specified time value. If
       the timer was already running, the "time left" is simply reset.

       @return
       reference to the timer.
     */
    PTimer & operator=(
      DWORD milliseconds            /// New time interval for timer.
    );
    PTimer & operator=(
      const PTimeInterval & time    /// New time interval for timer.
    );

    /** Destroy the timer object, removing it from the applications timer list
       if it was running.
     */
    virtual ~PTimer();
  //@}

  /**@name Control functions */
  //@{
    /** Start a timer in continous cycle mode. Whenever the timer runs out it
       is automatically reset to the time specified. Thus, it calls the
       notification function every time interval.
     */
    void RunContinuous(
      const PTimeInterval & time    // New time interval for timer.
    );

    /** Stop a running timer. The imer will not call the notification function
       and is reset back to the original timer value. Thus when the timer
       is restarted it begins again from the beginning.
     */
    void Stop();

    /** Determine if the timer is currently running. This really is only useful
       for one shot timers as repeating timers are always running.
       
       @return
       TRUE if timer is still counting.
     */
    BOOL IsRunning() const;

    /** Pause a running timer. This differs from the #Stop()# function in
       that the timer may be resumed at the point that it left off. That is
       time is "frozen" while the timer is paused.
     */
    void Pause();

    /** Restart a paused timer continuing at the time it was paused. The time
       left at the moment the timer was paused is the time until the next
       call to the notification function.
     */
    void Resume();

    /** Determine if the timer is currently paused.

       @return
       TRUE if timer paused.
     */
    BOOL IsPaused() const;
  //@}

  /**@name Notification functions */
  //@{
    /**This function is called on time out. That is when the system timer
       processing decrements the timer from a positive value to less than or
       equal to zero. The interval is then reset to zero and the function
       called.
       
       The default behaviour of this function is to call the #PNotifier# 
       callback function.
     */
    virtual void OnTimeout();

    /** Get the current call back function that is called whenever the timer
       expires. This is called by the #OnTimeout()# function.

       @return
       current notifier for the timer.
     */
    const PNotifier & GetNotifier() const;

    /** Set the call back function that is called whenever the timer expires.
       This is called by the #OnTimeout()# function.
     */
    void SetNotifier(
      const PNotifier & func  // New notifier function for the timer.
    );
  //@}

  /**@name Global real time functions */
  //@{
    /** Get the number of milliseconds since some arbtrary point in time. This
       is a platform dependent function that yields a real time counter.
       
       Note that even though this function returns milliseconds, the value may
       jump in minimum quanta according the platforms timer system, eg under
       MS-DOS and MS-Windows the values jump by 55 every 55 milliseconds. The
       #Resolution()# function may be used to determine what the minimum
       time interval is.
    
       @return
       millisecond counter.
     */
    static PTimeInterval Tick();

    /** Get the smallest number of milliseconds that the timer can be set to.
       All actual timing events will be rounded up to the next value. This is
       typically the platforms internal timing units used in the #Tick()#
       function.
       
       @return
       minimum number of milliseconds per timer "tick".
     */
    static unsigned Resolution();
  //@}

  private:
    void StartRunning(
      BOOL once   // Flag for one shot or continuous.
    );
    /* Start or restart the timer from the #resetTime# variable.
       This is an internal function.
     */

    BOOL Process(
      const PTimeInterval & delta,    // Time interval since last call.
      PTimeInterval & minTimeLeft     // Minimum time left till next timeout.
    );
    /* Process the timer decrementing it by the delta amount and calling the
       #OnTimeout()# when zero. This is used internally by the
       #PTimerList::Process()# function.

       @return
       TRUE if is no longer running.
     */

  // Member variables
    PNotifier callback;
    // Callback function for expired timers.

    PTimeInterval resetTime;
    // The time to reset a timer to when RunContinuous() is called.

    BOOL oneshot;
    // Timer operates once then stops.

    enum { Stopped, Starting, Running, Paused } state;
    // Timer state.

    PThread * timeoutThread;
    /* Timer is currently executing its OnTimeout() function. This is to
       prevent recursive calls when timer is in free running mode.
     */


  friend class PTimerList;
  
#ifdef DOC_PLUS_PLUS
};
#endif
  
// Class declaration continued in platform specific header file ///////////////
