/*
 * $Id: pipechan.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PPIPECHANNEL

#pragma interface

#include "../../common/pipechan.h"
  protected:
    int toChildPipe[2];
    int fromChildPipe[2];
    int childPid;
};

#endif
