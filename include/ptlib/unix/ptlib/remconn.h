/*
 * $Id: remconn.h,v 1.2 1996/04/15 10:50:48 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1996/01/26 11:06:31  craigs
 * Initial revision
 *
 */

#ifndef _PREMOTECONNECTION

#pragma interface

class PXRemoteThread;

#include "../../common/remconn.h"
  protected:
    PString          pppDeviceName;
    PXRemoteThread * remoteThread;
    BOOL             wasConnected;
};

#endif
