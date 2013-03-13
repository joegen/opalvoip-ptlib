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
 * $Revision$
 * $Author$
 * $Date$
 */

#pragma implementation "dynalink.h"

#include <ptlib.h>

#if P_DYNALINK

#include <dlfcn.h>


#ifdef P_PTHREADS
static pthread_mutex_t g_DLLMutex = PTHREAD_MUTEX_INITIALIZER;
#define LOCK_DLFCN() pthread_mutex_lock(&g_DLLMutex)
#define UNLOCK_DLFCN() pthread_mutex_unlock(&g_DLLMutex)
#else
#define LOCK_DLFCN()
#define UNLOCK_DLFCN()
#endif


PDynaLink::PDynaLink()
  : dllHandle(NULL)
{
}

PDynaLink::PDynaLink(const PString & _name)
  : dllHandle(NULL)
{
  Open(_name);
}

PDynaLink::~PDynaLink()
{
  Close();
}

PString PDynaLink::GetExtension()
{
#ifdef P_MACOSX
  return PString(".dylib");
#else
  return PString(".so");
#endif
}

PBoolean PDynaLink::Open(const PString & _name)
{
  m_lastError.MakeEmpty();

  Close();

  if (_name.IsEmpty())
    return false;

  PTRACE(4, "UDLL\topening " << _name);

  name = _name;

  LOCK_DLFCN();

#if defined(P_OPENBSD)
    dllHandle = dlopen((char *)(const char *)name, RTLD_NOW);
#else
    dllHandle = dlopen((const char *)name, RTLD_NOW);
#endif

  if (dllHandle == NULL) {
    m_lastError = dlerror();
    PTRACE(1, "DLL\tError loading DLL: " << m_lastError);
  }

  UNLOCK_DLFCN();

  return IsLoaded();
}

void PDynaLink::Close()
{
// without the hack to force late destruction of the DLL mutex this may crash for static PDynaLink instances
  if (dllHandle == NULL)
    return;

  PTRACE(4, "UDLL\tClosing " << name);
  name.MakeEmpty();

  LOCK_DLFCN();
  dlclose(dllHandle);
  dllHandle = NULL;
  UNLOCK_DLFCN();
}

PBoolean PDynaLink::IsLoaded() const
{
  return dllHandle != NULL;
}

PString PDynaLink::GetName(PBoolean full) const
{
  if (!IsLoaded())
    return "";

  if (full)
    return name;

  PString str = name;
  if (!full) {
    PINDEX pos = str.FindLast('/');
    if (pos != P_MAX_INDEX)
      str = str.Mid(pos+1);
    pos = str.FindLast(".so");
    if (pos != P_MAX_INDEX)
      str = str.Left(pos);
  }

  return str;
}


PBoolean PDynaLink::GetFunction(PINDEX, Function &)
{
  return false;
}

PBoolean PDynaLink::GetFunction(const PString & fn, Function & func)
{
  m_lastError.MakeEmpty();

  if (dllHandle == NULL)
    return false;

  LOCK_DLFCN();
#if defined(P_OPENBSD)
  func = (Function)dlsym(dllHandle, (char *)(const char *)fn);
#else
  func = (Function)dlsym(dllHandle, (const char *)fn);
#endif
  m_lastError = dlerror();
  UNLOCK_DLFCN();

  return func != NULL;
}

#else // P_DYNALINK

#warning "No implementation for dynamic library functions"

#endif // P_DYNALINK

// End of file
