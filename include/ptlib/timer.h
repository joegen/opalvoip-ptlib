/*
 * $Id: timer.h,v 1.10 1995/03/14 12:42:51 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timer.h,v $
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


class PTimer;


PDECLARE_CLASS(PTimerList, PAbstractList)
/* This class defines a list of <A>PTimer</A> objects. It is primarily used
   internally by the library and the user should never create an instance of
   it. The <A>PProcess</A> instance for the application maintains an instance
   of all of the timers created so that it may decrements them at regular
   intervals.
 */

  public:
    PTimerList();
    // Create a new timer list

    PTimeInterval Process();
    /* Decrement all the created timers and dispatch to their callback
       functions if they have expired. The <A>PTimer::Tick()</A> function
       value is used to determine the time elapsed since the last call to
       Process().

       The return value is the number of milliseconds until the next timer
       needs to be despatched. The function need not be called again for this
       amount of time, though it can (and usually is).
       
       <H2>Returns:</H2>
       maximum time interval before function should be called again.
     */

  private:
    PTimeInterval lastSample;
    // The last system timer tick value that was used to process timers.
};


PDECLARE_CLASS(PTimer, PTimeInterval)
/* A class representing a system timer. The time interval ancestor value is
   the amount of time left in the timer.

   A timer on completion calls the virtual function
   <A><CODE>OnTimeout()</CODE></A>. This will in turn call the callback
   function provided by the instance. The user may either override the virtual
   function or set a callback as desired.
   
   Note that only one timeout function can be executed at a time. The timeout
   function is also executed in the context of the <A>PProcess</A> instances
   thread of execution.
   
   A list of active timers is maintinaed by the applications <A>PProcess</A> 
   instance. This is used for sstealing the processor time to decrement the
   timers and call the timeout functions. A consequence of this is that no
   static timer instances can be running when the program terminates.
 */

  public:
    PTimer(
      long milliseconds = 0,  // Number of milliseconds for timer.
      int seconds = 0,        // Number of seconds for timer.
      int minutes = 0,        // Number of minutes for timer.
      int hours = 0,          // Number of hours for timer.
      int days = 0            // Number of days for timer.
    );
    PTimer(
      const PTimeInterval & time    // New time interval for timer.
    );
    /* Create a new timer object and start it in one shot mode for the
       specified amount of time. If the time was zero milliseconds then the
       timer is <EM>not</EM> started, ie the callback function is not executed
       immediately.
     */

    PTimer & operator=(
      const PTimeInterval & time    // New time interval for timer.
    );
    /* Restart the timer in one shot mode using the specified time value. If
       the timer was already running, the "time left" is simply reset.

       <H2>Returns:</H2>
       reference to the timer.
     */

    virtual ~PTimer();
    /* Destroy the timer object, removing it from the applications timer list
       if it was running.
     */


  // New functions for class
    void RunContinuous(
      const PTimeInterval & time    // New time interval for timer.
    );
    /* Start a timer in continous cycle mode. Whenever the timer runs out it
       is automatically reset to the time specified. Thus, it calls the
       notification function every time interval.
     */

    void Stop();
    /* Stop a running timer. The imer will not call the notification function
       and is reset back to the original timer value. Thus when the timer
       is restarted it begins again from the beginning.
     */

    BOOL IsRunning() const;
    /* Determine if the timer is currently running. This really is only useful
       for one shot timers as repeating timers are always running.
       
       <H2>Returns:</H2>
       TRUE if timer is still counting.
     */

    void Pause();
    /* Pause a running timer. This differs from the <A><CODE>Stop()</CODE></A>
       function in that the timer may be resumed at the point that it left off.
       That is time is "frozen" while the timer is paused.
     */

    void Resume();
    /* Restart a paused timer continuing at the time it was paused. The time
       left at the moment the timer was paused is the time until the next
       call to the notification function.
     */

    BOOL IsPaused() const;
    /* Determine if the timer is currently paused.

       <H2>Returns:</H2>
       TRUE if timer paused.
     */

    const PNotifier & GetNotifier() const;
    /* Get the current call back function that is called whenever the timer
       expires. This is called by the <A><CODE>OnTimeout()</CODE></A> function.

       <H2>Returns:</H2>
       current notifier for the timer.
     */

    void SetNotifier(
      const PNotifier & func  // New notifier function for the timer.
    );
    /* Set the call back function that is called whenever the timer expires.
       This is called by the <A><CODE>OnTimeout()</CODE></A> function.
     */

    static PTimeInterval Tick();
    /* Get the number of milliseconds since some arbtrary point in time. This
       is a platform dependent function that yields a real time counter.
       
       Note that even though this function returns milliseconds, the value may
       jump in minimum quanta according the platforms timer system, eg under
       MS-DOS and MS-Windows the values jump by 55 every 55 milliseconds. The
       <A><CODE>Resolution()</CODE></A> function may be used to determine what
       the minimum time interval is.
    
       <H2>Returns:</H2>
       millisecond counter.
     */

    static unsigned Resolution();
    /* Get the smallest number of milliseconds that the timer can be set to.
       All actual timing events will be rounded up to the next value. This is
       typically the platforms internal timing units used in the
       <A><CODE>Tick()</CODE></A> function.
       
       <H2>Returns:</H2>
       minimum number of milliseconds per timer "tick".
     */


  protected:
  // System callback functions.
    virtual void OnTimeout();
    /* This function is called on time out. That is when the system timer
       processing decrements the timer from a positive value to less than or
       equal to zero. The interval is then reset to zero and the function
       called.
       
       The default behaviour of this function is to call the <A>PNotifier</A> 
       callback function.
     */


  private:
    void StartRunning(
      BOOL once   // Flag for one shot or continuous.
    );
    /* Start or restart the timer from the <CODE>resetTime</CODE> variable.
       This is an internal function.
     */

    BOOL Process(
      const PTimeInterval & delta,    // Time interval since last call.
      PTimeInterval & minTimeLeft     // Minimum time left till next timeout.
    );
    /* Process the timer decrementing it by the delta amount and calling the
       <A><CODE>OnTimeout()</CODE></A> when zero. This is used by internally
       by the <A>PTimerList</A> calless <A><CODE>Process()</CODE></A> function.

       <H2>Returns:</H2>
       TRUE if is no longer running.
     */

  // Member variables
    PNotifier callback;
    // Callback function for expired timers.

    PTimeInterval resetTime;
    // The time to reset a timer to when RunContinuous() is called.

    BOOL oneshot;
    // Timer operates once then stops.

    enum { Stopped, Running, Paused } state;
    // Timer state.

    BOOL inTimeout;
    /* Timer is currently executing its OnTimeout() function. This is to
       prevent recursive calls when timer is in free running mode.
     */


  friend class PTimerList;
  
  
// Class declaration continued in platform specific header file ///////////////
