/*
 * $Id: mutex.h,v 1.1 1998/03/23 02:41:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: mutex.h,v $
 * Revision 1.1  1998/03/23 02:41:31  robertj
 * Initial revision
 *
 */


#define _PMUTEX

#ifdef __GNUC__
#pragma interface
#endif

#include <semaphor.h>


PDECLARE_CLASS(PMutex, PSemaphore)
/* This class defines a thread mutual exclusion object. A mutex is where a
   piece of code or data cannot be accessed by more than one thread at a time.
   To prevent this the PMutex is used in the following manner:
      <CODE>
      PMutex mutex;

      ...

      mutex.Wait();

      ... critical section - only one thread at a time here.

      mutex.Signal();

      ...
      </CODE>
    The first thread will pass through the <A>Wait()</A> function, a second
    thread will block on that function until the first calls the
    <A>Signal()</A> function, releasing the second thread.
 */

  public:
    PMutex();
    /* Create a new mutex.
     */


// Class declaration continued in platform specific header file ///////////////
