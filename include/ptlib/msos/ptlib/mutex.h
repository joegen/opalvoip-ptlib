/*
 * $Id: mutex.h,v 1.1 1998/03/23 02:42:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: mutex.h,v $
 * Revision 1.1  1998/03/23 02:42:02  robertj
 * Initial revision
 *
 */


#ifndef _PMUTEX


///////////////////////////////////////////////////////////////////////////////
// PMutex

#include "../../common/ptlib/mutex.h"
  public:
    virtual void Signal();
};


#endif
