/*
 * $Id: file.h,v 1.2 1996/05/09 12:23:00 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.2  1996/05/09 12:23:00  robertj
 * Further implementation.
 *
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PFILE

#include <unix.h>

#define _lseek lseek
#define _mkdir mkdir
#define _chdir chdir
inline int _open(const char * n, int m) { return open(n, m); }
inline int _close(int fd) { return close(fd); }
inline int _read(int fd, void * p, unsigned s) { return read(fd, (char *)p, s); }
inline int _write(int fd, const void * p, unsigned s) { return write(fd, (char *)p, s); }
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

