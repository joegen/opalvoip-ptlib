/*
 * $Id: timer.h,v 1.6 1994/07/02 03:03:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timer.h,v $
 * Revision 1.6  1994/07/02 03:03:49  robertj
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

class PTimer;


PDECLARE_CLASS(PTimerList, PAbstractList)
  public:
    PTimerList();
      // Create a new timer list

    PTimeInterval Process();
      // Check all the created timers against the Tick() function value (since
      // some arbitrary base time) and despatch to their callback functions if
      // they have expired. The return value is the number of milliseconds
      // until the next timer needs to be despatched. The function need not be
      // called again for this amount of time, though it can (and usually is).

  private:
    PTimeInterval lastSample;
      // The last system timer tick value that was used to process timers.
};


PDECLARE_CLASS(PTimer, PTimeInterval)
  // A class representing a system timer. The PTimeInterval ancestor value is
  // the amount of time left in the timer.

  public:
    PTimer(long milliseconds = 0,
                int seconds = 0, int minutes = 0, int hours = 0, int days = 0);
      // Create a new timer object and start it in one shot mode for the
      // specified amount of time
 
    PTimer(const PTimeInterval & time);
    PTimer & operator=(const PTimeInterval & time);
      // Restart the timer in one shot mode using the specified time value.

    virtual ~PTimer();
      // Destroy the timer object


    // New functions for class
    void RunContinuous(const PTimeInterval & time);
      // Start a timerin continous cycle mode. It calls the notification
      // function every time interval.

    void Stop();
      // Stop a running timer.

    BOOL IsRunning() const;
      // Return TRUE if the timer is currently running. This really is only
      // useful for one shot timers as repeating timers are always running.

    void Pause();
      // Pause a running timer.

    void Resume();
      // Restart a paused timer continuing at the time it was paused.

    BOOL IsPaused() const;
      // Return TRUE if the timer is currently paused.

    static PTimeInterval Tick();
      // Return the number of milliseconds since some arbtrary point in time.

    static unsigned Resolution();
      // Return the smallest number of milliseconds that the timer can be set
      // to. All actual timing events will be rounded up to the next value.
      // This is typically the real tick unit used in the Tick() function
      // above that all system timing is derived from.


  protected:
    // System callback functions.
    virtual void OnTimeout();
      // This function is called.


  private:
    void StartRunning(BOOL once);
      // Start the timer from the resetTime variable.

    BOOL Process(const PTimeInterval & delta, PTimeInterval & minTimeLeft);
      // Process the timer decrementing it by the delta amount and calling
      // the OnTimeout() when zero. Returns TRUE if is no longer running.

    // Member variables
    PTimeInterval resetTime;
      // The time to reset a timer to when RunContinuous() is called.

    BOOL oneshot;
      // Timer operates once then stops.

    enum { Stopped, Running, Paused } state;
      // Timer state

    BOOL inTimeout;
      // Timer is currently executing its OnTimeout() function. This is to
      // prevent recursive calls when timer is in free running mode.


  friend class PTimerList;
  
  
// Class declaration continued in platform specific header file ///////////////
