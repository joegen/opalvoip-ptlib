/*
 * $Id: timer.h,v 1.2 1993/08/31 03:38:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: timer.h,v $
 * Revision 1.2  1993/08/31 03:38:02  robertj
 * Added missing virtual on destructor.
 *
 * Revision 1.1  1993/08/27  18:17:47  robertj
 * Initial revision
 *
 */


#define _PTIMER

class PTimer;

DECLARE_CLASS(PTimerList, PAbstractSortedList)
  public:
    PTimerList();
      // Create a new timer list

    PMilliseconds Process();
      // Check all the created timers against the Tick() function value (since
      // some arbitrary base time) and despatch to their callback functions if
      // they have expired. The return value is the number of milliseconds
      // until the next timer needs to be despatched. The function need not be
      // called again for this amount of time, though it can (and usually is).
};


DECLARE_CLASS(PTimer, PTimeInterval)
  // A class representing a system timer.

  public:
    PTimer(PApplication * app, long milliseconds = 0,
                   int seconds = 0,int minutes = 0,int hours = 0,int days = 0);
      // Create a new timer object
 
    PTimer(const PTimer & timer);
    PTimer & operator=(const PTimer & timer);
      // Make a duplicate of the specified timer

    virtual ~PTimer();
      // Destroy the timer object


    // Overrides from class PObject
    virtual Comparison Compare(const PObject & obj) const;


    // New functions for class
    void Start(BOOL once = TRUE);
      // Start a timer, if once is TRUE then the timer stops after one 
      // interval, otherwise it calls the notification function every time 
      // interval.

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

    static PMilliseconds Tick();
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


    // Member variables
    PApplication * owner;
      // The application that the timer is running in.

    PMilliseconds targetTime;
      // The system timer tick value that the next timeout will occur.

    BOOL oneshot;
      // Timer operates once then stops.

    enum { stopped, running, paused } state;
      // Timer state

    PMilliseconds pauseLeft;
      // The amount of time left when the Pause() function was called, used
      // when Resume() is subsequently called.


  friend class PTimerList;
  
  
// Class declaration continued in platform specific header file ///////////////
