/*
 * $Id: svcproc.h,v 1.1 1996/01/02 13:10:31 robertj Exp $
 *
 * Portable Windows Library
 *
 * Service Process for Windows NT platforms
 *
 * Copyright 1995 Equivalence
 *
 * $Log: svcproc.h,v $
 * Revision 1.1  1996/01/02 13:10:31  robertj
 * Initial revision
 *
 */


#ifndef _PSERVICEPROCESS


///////////////////////////////////////////////////////////////////////////////
// PServiceProcess

#include "::common:svcproc.h"
};


inline PServiceProcess * PServiceProcess::Current()
  { return (PServiceProcess *)PProcessInstance; }


#endif
