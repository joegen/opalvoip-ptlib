/*
 * $Id: remconn.h,v 1.3 1998/01/26 00:53:13 robertj Exp $
 *
 * Portable Windows Library
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
 * Revision 1.3  1998/01/26 00:53:13  robertj
 * Moved to common.
 *
 * Revision 1.2  1996/08/08 10:09:10  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1995/12/10 13:04:08  robertj
 * Initial revision
 *
 */

#ifndef _PREMOTECONNECTION

#include <ras.h>
#include <raserror.h>


#include "../../common/ptlib/remconn.h"
  private:
    // Win32 specific stuff
    HRASCONN rasConnection;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
