/*
 * $Id: remconn.h,v 1.3 1996/08/03 12:08:19 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
 * Revision 1.3  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
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

#include "../../common/ptlib/remconn.h"
  protected:
    PString          pppDeviceName;
    PXRemoteThread * remoteThread;
    BOOL             wasConnected;
};

#endif
