/*
 * $Id: channel.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 */

#ifndef _PCHANNEL

#pragma interface

#include "../../common/channel.h"
  protected:
    BOOL SetIOBlock(BOOL isRead);

    PString channelName;
};

#endif
