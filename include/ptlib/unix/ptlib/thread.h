/*
 * $Id: thread.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTHREAD

#pragma interface

#include <setjmp.h>

class PProcess;

///////////////////////////////////////////////////////////////////////////////
// PThread

#include "../../common/thread.h"

  public:
    friend class PProcess;
    BOOL PXBlockOnIO(int handle, BOOL isRead);
    BOOL PXBlockOnIO(int handle, BOOL isRead, PTimeInterval timeout);

  private:
    PTimer ioTimer;
    BOOL   hasIOTimer;
    int    blockHandle;
    BOOL   blockRead;
};


#endif
