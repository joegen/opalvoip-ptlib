/*
 * $Id: textfile.h,v 1.3 1996/08/08 10:09:18 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: textfile.h,v $
 * Revision 1.3  1996/08/08 10:09:18  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.2  1995/03/12 05:00:01  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTEXTFILE

///////////////////////////////////////////////////////////////////////////////
// PTextFile

#include "../../common/ptlib/textfile.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required

  private:
    char characterAfterCarriageReturn;
};


#endif
