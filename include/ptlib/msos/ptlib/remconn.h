/*
 * $Id: remconn.h,v 1.1 1995/12/10 13:04:08 robertj Exp $
 *
 * Portable Windows Library
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
 * Revision 1.1  1995/12/10 13:04:08  robertj
 * Initial revision
 *
 */

#ifndef _PREMOTECONNECTION

#include <ras.h>
#include <raserror.h>


#include "../../common/remconn.h"
  private:
    // Win32 specific stuff
    HRASCONN rasConnection;
    DWORD    rasError;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
