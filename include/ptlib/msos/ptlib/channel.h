/*
 * $Id: channel.h,v 1.3 1998/08/20 06:03:44 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.3  1998/08/20 06:03:44  robertj
 * Allowed Win32 class to be used in other compilation modules
 *
 * Revision 1.2  1996/08/08 10:08:56  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1994/07/02 03:18:09  robertj
 * Initial revision
 *
 */


#ifndef _PCHANNEL

///////////////////////////////////////////////////////////////////////////////
// PChannel

#include "../../common/ptlib/channel.h"
};


class PWin32Overlapped : public OVERLAPPED
{
  // Support class for overlapped I/O in Win32.
  public:
    PWin32Overlapped();
    ~PWin32Overlapped();
    void Reset();
};



#endif
