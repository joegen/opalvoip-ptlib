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
 * Revision 1.12  2003/04/16 07:17:35  craigs
 * CHanged to use new #define
 *
 * Revision 1.11  2001/06/30 06:59:07  yurik
 * Jac Goudsmit from Be submit these changes 6/28. Implemented by Yuri Kiryanov
 *
 * Revision 1.10  2001/03/07 06:57:52  yurik
 * Changed email to current one
 *
 * Revision 1.9  2000/03/10 08:21:17  rogerh
 * Add correct OpenBSD support
 *
 * Revision 1.8  2000/03/09 18:41:53  rogerh
 * Workaround for OpenBSD. This breaks the functionality on OpenBSD but
 * gains us a clean compilation. We can return to this problem later.
 *
 * Revision 1.7  1999/02/22 13:26:54  robertj
 * BeOS port changes.
 *
 * Revision 1.6  1999/02/06 05:49:44  robertj
 * BeOS port effort by Yuri Kiryanov <openh323@kiryanov.com>
 *
 * Revision 1.5  1998/11/30 21:52:03  robertj
 * New directory structure.
 *
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

#ifndef	P_DYNALINK

#warning "No implementation for dynamic library functions"

#else

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

#if defined(P_OPENBSD)
  dllHandle = dlopen((char *)(const char *)name, RTLD_LAZY);
#else
  dllHandle = dlopen((const char *)name, RTLD_LAZY);
#endif

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

#if defined(P_OPENBSD)
  void * p = dlsym(dllHandle, (char *)(const char *)name);
#else
  void * p = dlsym(dllHandle, (const char *)name);
#endif

  if (p == NULL)
    return FALSE;

  func = (Function &)p;
  return TRUE;
}

#endif

// End of file

