/*
 * pdirect.h
 *
 * File system directory class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pdirect.h,v $
 * Revision 1.5  1998/09/24 03:30:14  robertj
 * Added open software license.
 *
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
