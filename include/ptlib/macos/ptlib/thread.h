/*
 * $Id: thread.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PTHREAD

#include <setjmp.h>


///////////////////////////////////////////////////////////////////////////////
// PThread

#include "::common:thread.h"
  public:
    typedef BOOL (*BlockFunction)(PObject *);
    void Block(BlockFunction isBlocked, PObject * obj);
      // Flag the thread as blocked. The scheduler will call the specified
      // function with the obj parameter to determine if the thread is to be
      // unblocked.

  protected:
    BOOL IsOnlyThread() const;
      // Return TRUE if is only thread in process


  private:
    // Member fields
    BlockFunction isBlocked;
      // Callback function to determine if the thread is blocked on I/O.

    PObject * blocker;
      // When thread is blocked on I/O this is the object to pass to isBlocked.
};


inline PThread::PThread()
  { }   // Is mostly initialised by InitialiseProcessThread().

#endif
