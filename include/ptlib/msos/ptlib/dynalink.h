/*
 * $Id: dynalink.h,v 1.2 1996/08/08 10:08:59 robertj Exp $
 *
 * Portable Windows Library
 *
 * Dynamic Link Library Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: dynalink.h,v $
 * Revision 1.2  1996/08/08 10:08:59  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1995/03/14 12:44:49  robertj
 * Initial revision
 *
 */

#ifndef _PDYNALINK


#include "../../common/ptlib/dynalink.h"
  protected:
#if defined(_WINDOWS) || defined(_WIN32)
    HINSTANCE _hDLL;
#endif
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
