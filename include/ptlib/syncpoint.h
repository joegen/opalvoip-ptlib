/*
 * $Id: syncpoint.h,v 1.1 1998/03/23 02:41:34 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: syncpoint.h,v $
 * Revision 1.1  1998/03/23 02:41:34  robertj
 * Initial revision
 *
 */


#define _PSYNCPOINT

#ifdef __GNUC__
#pragma interface
#endif

#include <semaphor.h>


PDECLARE_CLASS(PSyncPoint, PSemaphore)
/* This class defines a thread synchonisation object.
 */

  public:
    PSyncPoint();
    /* Create a new sync point.
     */

// Class declaration continued in platform specific header file ///////////////
