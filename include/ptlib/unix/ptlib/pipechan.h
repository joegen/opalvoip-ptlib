/*
 * $Id: pipechan.h,v 1.4 1996/12/30 03:23:52 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.4  1996/12/30 03:23:52  robertj
 * Commonised kill and wait functions.
 *
 * Revision 1.3  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PPIPECHANNEL

#pragma interface

#include <signal.h>

#include "../../common/ptlib/pipechan.h"
  protected:
    int toChildPipe[2];
    int fromChildPipe[2];
    int childPid;
    int retVal;
};

#ifdef P_USE_INLINES
#include "../../common/pipechan.inl"
#endif

#endif
