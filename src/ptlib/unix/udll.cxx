/*
 * $Id: udll.cxx,v 1.1 1997/04/22 10:58:17 craigs Exp $
 *
 * Portable Windows Library
 *
 * Dynamic Link Library Declarations
 *
 * Copyright 1997 Equivalence
 *
 * $Log: udll.cxx,v $
 * Revision 1.1  1997/04/22 10:58:17  craigs
 * Initial revision
 *
 *
 */

#pragma implementation "dynalink.h"

#include <ptlib.h>
#include <dynalink.h>

#ifdef P_SOLARIS
#include <dlfcn.h>
#else
#message "No implementation for dynamic library functions"
#endif

PDynaLink::PDynaLink()
{
  dllHandle = NULL;
}

PDynaLink::PDynaLink(const PString & name)
{
  dllHandle = NULL;
  Open(name);
}

PDynaLink::~PDynaLink()
{
  Close();
}

BOOL PDynaLink::Open(const PString & name)
{
  Close();
  dllHandle = dlopen((const char *)name, RTLD_LAZY);
  return IsLoaded();
}

void PDynaLink::Close()
{
  if (dllHandle != NULL)
    dlclose(dllHandle);
}

BOOL PDynaLink::IsLoaded() const
{
  return dllHandle != NULL;
}


BOOL PDynaLink::GetFunction(PINDEX, Function &)
{
  return FALSE;
}

BOOL PDynaLink::GetFunction(const PString & name, Function & func)
{
  if (dllHandle == NULL)
    return FALSE;

  void * p = dlsym(dllHandle, (const char *)name);
  if (p == NULL)
    return FALSE;

  func = (Function &)p;
  return TRUE;
}
