/*
 * $Id: process.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: process.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PPROCESS


///////////////////////////////////////////////////////////////////////////////
// PProcess

#include "::common:process.h"
};


extern PProcess * PSTATIC PProcessInstance;

inline PProcess * PProcess::Current()
  { return PProcessInstance; }


#endif
