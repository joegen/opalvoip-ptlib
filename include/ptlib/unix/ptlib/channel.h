/*
 * $Id: channel.h,v 1.4 1996/01/26 11:06:31 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.4  1996/01/26 11:06:31  craigs
 * Fixed problem with blocking Accept calls
 *
 * Revision 1.3  1995/07/09 00:34:58  craigs
 * Latest and greatest omnibus change
 *
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
  public:
    enum {
      PXReadBlock,
      PXWriteBlock,
      PXOtherBlock,
      PXAcceptBlock
    };
  protected:
    BOOL PXSetIOBlock(int type);
    BOOL PXSetIOBlock(int type, int blockHandle);
    PString channelName;
};

#endif
