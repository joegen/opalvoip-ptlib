/*
 * $Id: textfile.h,v 1.1 1994/06/25 12:13:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: textfile.h,v $
 * Revision 1.1  1994/06/25 12:13:01  robertj
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTEXTFILE

///////////////////////////////////////////////////////////////////////////////
// PTextFile

#include "../../common/textfile.h"
  protected:
    virtual BOOL IsTextFile() const;
      // Return TRUE if text file translation is required

    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

  private:
    char characterAfterCarriageReturn;
};


#endif
