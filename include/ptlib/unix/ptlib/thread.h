/*
 * $Id: thread.h,v 1.8 1997/04/22 11:00:44 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: thread.h,v $
 * Revision 1.8  1997/04/22 11:00:44  craigs
 * Added FreeStack function
 *
 * Revision 1.7  1996/12/30 03:23:52  robertj
 * Added timeout to block on child process function.
 *
 * Revision 1.6  1996/08/03 12:09:51  craigs
 * Changed for new common directories
 *
 * Revision 1.5  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
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

#include "../../common/ptlib/thread.h"

  public:
    int PXBlockOnIO(int handle,
                    int type,
                   const PTimeInterval & timeout);

    int PXBlockOnIO(int maxHandle,
               fd_set & readBits,
               fd_set & writeBits,
               fd_set & execptionBits,
               const PTimeInterval & timeout,
               const PIntArray & osHandles);

    int PXBlockOnChildTerminate(int pid,
                                const PTimeInterval & timeout);
                     
  protected:
    void FreeStack();
    void PXSetOSHandleBlock  (int fd, int type);
    void PXClearOSHandleBlock(int fd, int type);

    PTimer ioTimer;
    BOOL   hasIOTimer;
    int    waitPid;

    fd_set * read_fds;
    fd_set * write_fds;
    fd_set * exception_fds;
    int    handleWidth;

    int    selectReturnVal;
    int    selectErrno;
};


#endif
