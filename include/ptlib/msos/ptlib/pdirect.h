/*
 * $Id: pdirect.h,v 1.1 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.1  1994/06/25 12:13:01  robertj
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */


#ifndef _PDIRECTORY

///////////////////////////////////////////////////////////////////////////////
// PDirectory

#include <direct.h>
#include <dos.h>

#define PDIR_SEPARATOR '\\'

#define P_MAX_PATH    (_MAX_PATH)

#include "../../common/pdirect.h"
  protected:
    struct find_t  fileinfo;

    BOOL Filtered();
};


#endif
