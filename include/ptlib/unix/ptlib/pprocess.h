/*
 * $Id: pprocess.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: pprocess.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PPROCESS

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "../../common/process.h"
  public:
    PString GetHomeDir ();
    char ** GetEnvironment();

  protected:
    char **envp;
};

extern PProcess * PProcessInstance;

#endif
