/*
 * $Id: thread.h,v 1.4 1996/01/26 11:08:45 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.4  1996/01/26 11:08:45  craigs
 * Fixed problem with blocking Accept calls
 *
 * Revision 1.3  1995/12/08 13:16:38  craigs
 * Added semaphore include and friend class
 *
 * Revision 1.2  1995/07/09 00:35:00  craigs
 * Latest and greatest omnibus change
 *
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
class PSemaphore;

///////////////////////////////////////////////////////////////////////////////
// PThread

#include "../../common/thread.h"

  public:
    BOOL PXBlockOnIO(int handle, int type);
    BOOL PXBlockOnIO(int handle, int type, const PTimeInterval & timeout);

  protected:
    PTimer ioTimer;
    BOOL   hasIOTimer;
    int    blockHandle;
    int    blockType;
    BOOL   dataAvailable;
};


#endif
