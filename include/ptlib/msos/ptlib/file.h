/*
 * $Id: file.h,v 1.1 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: file.h,v $
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


///////////////////////////////////////////////////////////////////////////////
// PFile

#include "../../common/file.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required
};


#endif
