/*
 * $Id: pdirect.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PDIRECTORY

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PDirectory

#include <dirent.h>

#define PDIR_SEPARATOR '/'

#define P_MAX_PATH    (_POSIX_PATH_MAX)

#define	PFILE_PATH_STRING	PString

#include "../../common/pdirect.h"
  protected:
    DIR           * directory;
    struct dirent * entry;
    PFileInfo     * entryInfo;
};

#endif
