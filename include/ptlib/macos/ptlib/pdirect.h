/*
 * $Id: pdirect.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pdirect.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PDIRECTORY

///////////////////////////////////////////////////////////////////////////////
// PDirectory

const char PDIR_SEPARATOR = ':';

const PINDEX P_MAX_PATH = 255;

#define PFILE_PATH_STRING PCaselessString


#include "::common:pdirect.h"
  protected:
   BOOL Filtered();

  public:
    static PString CreateFullPath(const PString & path, BOOL isDirectory);
};


#endif
