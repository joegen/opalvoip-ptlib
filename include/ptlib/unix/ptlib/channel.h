/*
 * $Id: channel.h,v 1.6 1996/05/02 11:55:28 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: channel.h,v $
 * Revision 1.6  1996/05/02 11:55:28  craigs
 * Added ioctl definition for Sun4
 *
 * Revision 1.5  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
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

#ifdef P_SUN4
extern "C" int ioctl(int, int, void *);
#endif

#include "../../common/channel.h"
  public:
    enum {
      PXReadBlock,
      PXWriteBlock,
      PXAcceptBlock,
      PXConnectBlock
    };
  protected:
    BOOL PXSetIOBlock(int type, PTimeInterval timeout = PMaxTimeInterval);
    BOOL PXSetIOBlock(int type, int blockHandle, PTimeInterval timeout = PMaxTimeInterval);
    PString channelName;
};

#endif
