/*
 * $Id: channel.h,v 1.2 1995/01/23 22:59:47 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.2  1995/01/23 22:59:47  craigs
 * Changes for HPUX and Sun 4
 *
 */

#ifndef _PCHANNEL

#pragma interface

#if defined(P_SUN4)
#include <errno.h>
#include <sys/errno.h>
#endif

#include "../../common/channel.h"
  protected:
    BOOL SetIOBlock(BOOL isRead);

    PString channelName;
};

#endif
