/*
 * $Id: svcproc.h,v 1.2 1996/04/15 10:50:48 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: svcproc.h,v $
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1996/01/26 11:06:31  craigs
 * Initial revision
 *
 */

#ifndef _PSERVICEPROCESS

#pragma interface

#include "../../common/svcproc.h"
  protected:
    void _PXShowSystemWarning(PINDEX num, const PString & str);
    BOOL consoleMessages;
};


#endif
