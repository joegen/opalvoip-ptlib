/*
 * $Id: pipechan.h,v 1.5 1996/11/16 10:53:30 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pipechan.h,v $
 * Revision 1.5  1996/11/16 10:53:30  robertj
 * Fixed bug in PPipeChannel test for open channel, win95 support.
 *
 * Revision 1.4  1996/08/08 10:09:07  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.3  1995/03/14 13:31:35  robertj
 * Implemented DOS pipe channel.
 *
 * Revision 1.2  1995/03/12  04:59:56  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.1  1994/10/23  05:35:36  robertj
 * Initial revision
 *
 */


#ifndef _PPIPECHANNEL

#include "..\..\common\ptlib/pipechan.h"
  public:
    virtual BOOL IsOpen() const;
  protected:
#if defined(_WIN32)
    PROCESS_INFORMATION info;
    HANDLE hToChild, hFromChild;
#else
    BOOL hasRun;
    PFilePath toChild, fromChild;
#endif
};


#endif
