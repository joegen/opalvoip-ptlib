/*
 * $Id: textfile.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: textfile.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */

#ifndef _PTEXTFILE

///////////////////////////////////////////////////////////////////////////////
// PTextFile

#include "::common:textfile.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required
};


#endif
