/*
 * $Id: pdirect.h,v 1.4 1996/08/08 10:09:06 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.4  1996/08/08 10:09:06  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.3  1995/03/12 04:59:55  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.2  1994/10/24  00:15:21  robertj
 * Changed PFilePath and PDirectory so descends from either PString or
 *     PCaselessString depending on the platform.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
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
#if !defined(_WIN32)
#include <dos.h>
#endif

const char PDIR_SEPARATOR = '\\';

const PINDEX P_MAX_PATH = _MAX_PATH;

#define PFILE_PATH_STRING PCaselessString


#include "../../common/ptlib/pdirect.h"
  protected:
#if defined(_WIN32)
    HANDLE hFindFile;
    WIN32_FIND_DATA fileinfo;
#else
    struct find_t fileinfo;
#endif

    BOOL Filtered();

  public:
    static PString CreateFullPath(const PString & path, BOOL isDirectory);
};


#endif
