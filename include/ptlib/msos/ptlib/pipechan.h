/*
 * $Id: pipechan.h,v 1.2 1995/03/12 04:59:56 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.2  1995/03/12 04:59:56  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.1  1994/10/23  05:35:36  robertj
 * Initial revision
 *
 */


#ifndef _PPIPECHANNEL

#include "..\..\common\pipechan.h"
#if defined(_WIN32)
  protected:
    PROCESS_INFORMATION info;
    HANDLE hToChild, hFromChild;
#else
    int hToChild, hFromChild;
    BOOL hasRun;
#endif
};


#endif
