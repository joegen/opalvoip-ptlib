/*
 * $Id: file.h,v 1.2 1995/06/04 12:46:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.2  1995/06/04 12:46:49  robertj
 * Borland C++ compatibility.
 *
 * Revision 1.1  1994/06/25 12:13:01  robertj
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */


#ifndef _PFILE

#include <sys\types.h>
#include <errno.h>
#include <io.h>

#ifdef __BORLANDC__
#define _open ::open
#define _close ::close
#define _read ::read
#define _write ::write
#define _lseek ::lseek
#define _chsize ::chsize
#define _access ::access
#define _chmod ::chmod
#define _mkdir ::mkdir
#define _rmdir ::rmdir
#define _chdir ::chdir
#define _mktemp ::mktemp
#define _S_IWRITE S_IWRITE
#define _S_IREAD S_IREAD
#define _O_TEXT O_TEXT
#define _O_BINARY O_BINARY
#endif


///////////////////////////////////////////////////////////////////////////////
// PFile

#include "../../common/file.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required
};


#endif
