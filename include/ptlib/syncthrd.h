/*
 * $Id: syncthrd.h,v 1.1 1998/05/30 13:26:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: syncthrd.h,v $
 * Revision 1.1  1998/05/30 13:26:15  robertj
 * Initial revision
 *
 */


#define _PSYNCPOINTACK

#ifdef __GNUC__
#pragma interface
#endif

#include <syncpoint.h>


PDECLARE_CLASS(PSyncPointAck, PSyncPoint)
/* This class defines a thread synchonisation object.

   This may be used to send signals to a thread and await an acknowldegement
   that the signal was processed.
 */

  public:
    virtual void Signal();
    void Signal(const PTimeInterval & waitTime);

    void Acknowledge();

  protected:
    PSyncPoint ack;
};

////////////////////////////////////////////////////////////////////////////////

