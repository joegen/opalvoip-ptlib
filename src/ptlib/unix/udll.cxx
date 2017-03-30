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
  : m_dll(NULL)
{
}

PDynaLink::PDynaLink(const PString & names)
  : m_dll(NULL)
{
  Open(names);
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

PBoolean PDynaLink::Open(const PString & names)
{
  m_lastError.MakeEmpty();

  Close();

  PStringArray filenames = names.Lines();
  for (PINDEX i = 0; i < filenames.GetSize(); ++i) {
    m_name = filenames[i];
    PTRACE(4, "UDLL\tOpening " << m_name);

    LOCK_DLFCN();

#if defined(P_OPENBSD)
      m_dll = dlopen((char *)(const char *)m_name, RTLD_NOW);
#else
      m_dll = dlopen((const char *)m_name, RTLD_NOW);
#endif

    if (m_dll != NULL) {
      UNLOCK_DLFCN();
      return true;
    }
  }

  m_lastError = dlerror();
  UNLOCK_DLFCN();

  PTRACE(1, "DLL\tError loading DLL: " << m_lastError);
  return false;
}

void PDynaLink::Close()
{
// without the hack to force late destruction of the DLL mutex this may crash for static PDynaLink instances
  if (m_dll == NULL)
    return;

  PTRACE(4, "UDLL\tClosing " << m_name);
  m_name.MakeEmpty();

  LOCK_DLFCN();
  dlclose(m_dll);
  m_dll = NULL;
  UNLOCK_DLFCN();
}

PBoolean PDynaLink::IsLoaded() const
{
  return m_dll != NULL;
}

PString PDynaLink::GetName(PBoolean full) const
{
  if (!IsLoaded())
    return PString::Empty();

  if (full)
    return m_name;

  PString str = m_name;

  PINDEX pos = str.FindLast('/');
  if (pos != P_MAX_INDEX)
    str = str.Mid(pos+1);
  pos = str.FindLast(".so");
  if (pos != P_MAX_INDEX)
    str = str.Left(pos);

  return str;
}


PBoolean PDynaLink::GetFunction(PINDEX, Function &, bool)
{
  return false;
}

PBoolean PDynaLink::GetFunction(const PString & fn, Function & func, bool compulsory)
{
  m_lastError.MakeEmpty();
  func = NULL;

  if (m_dll == NULL)
    return false;

  LOCK_DLFCN();
#if defined(P_OPENBSD)
  func = (Function)dlsym(m_dll, (char *)(const char *)fn);
#else
  func = (Function)dlsym(m_dll, (const char *)fn);
#endif
  m_lastError = dlerror();
  UNLOCK_DLFCN();

  if (func != NULL)
    return true;

  if (compulsory)
    Close();

  return false;
}

#else // P_DYNALINK

#warning "No implementation for dynamic library functions"

#endif // P_DYNALINK

// End of file
