/*
 * $Id: file.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PFILE

#include <unix.h>

#define _lseek __lseek
#define _mkdir __mkdir
#define _chdir __chdir
#define _open __open
inline int _close(int f) { return __close(f); }
inline int _read(int f, void * b, int s) { return __read(f, (unsigned char *)b, s); }
inline int _write(int f, const void * b, int s) { return __write(f, (unsigned char *)b, s); }
#define _O_BINARY O_BINARY
#define _O_TEXT 0
#define EBADF 100


///////////////////////////////////////////////////////////////////////////////
// PFile

#include "::common:file.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required
};


#endif

