/*
 * udll.cxx
 *
 * Dynamic Link Library implementation.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: udll.cxx,v $
 * Revision 1.4  1998/09/24 04:12:26  robertj
 * Added open software license.
 *
 * Revision 1.3  1998/01/04 08:11:41  craigs
 * Remove Solarisism and made platform independent
 *
 * Revision 1.2  1997/10/30 12:41:22  craigs
 * Added GetExtension command
 *
 * Revision 1.1  1997/04/22 10:58:17  craigs
 * Initial revision
 *
 *
 */

#pragma implementation "dynalink.h"

#include <ptlib.h>
#include <dynalink.h>

#ifndef RTLD_LAZY
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

PString PDynaLink::GetExtension()
{
  return PString(".so");
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
