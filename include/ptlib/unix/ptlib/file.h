/*
 * $Id: file.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PFILE

#pragma interface

#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////
// PFile

#include "../../common/file.h"
};

#define	_read(fd,vp,st)		::read(fd, vp, (size_t)st)
#define	_write(fd,vp,st)	::write(fd, vp, (size_t)st)
#define	_fdopen			::fdopen
#define	_lseek(fd,off,w)	::lseek(fd, (off_t)off, w)
#define _close(fd)		::close(fd)

#endif
