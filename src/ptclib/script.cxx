/*
 * script.cxx
 *
 * Abstract class for script language interfaces
 *
 * Portable Tools Library
 *
 * Copyright (C) 2010-2012 by Post Increment
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
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): Craig Southeren
 *                 Robert Jongbloed
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "script.h"
#endif

#include <ptlib.h>

#include <ptclib/script.h>

#define new PNEW

PScriptLanguage::PScriptLanguage()
  : m_loaded(false)
  , m_lastErrorCode(0)
{
}


PScriptLanguage::~PScriptLanguage()
{
}


bool PScriptLanguage::Load(const PString & script)
{
  PFilePath filename = script;
  if (PFile::Exists(filename)) {
    if (!LoadFile(filename))
      return false;
  }
  else {
    if (!LoadText(script))
      return false;
  }

  return true;
}


void PScriptLanguage::OnError(int code, const PString & str)
{
  m_mutex.Wait();
  m_lastErrorCode = code;
  m_lastErrorText = str;
  m_mutex.Signal();

  PTRACE(2, GetClass(), "Error " << code << ": " << str);
}


bool PScriptLanguage::InternalSetFunction(const PString & name, const FunctionNotifier & func)
{
  FunctionMap::iterator it = m_functions.find(name);
  if (it == m_functions.end())
    return func.IsNULL();

  if (func.IsNULL())
    m_functions.erase(it);
  else
    it->second = func;
  return true;
}


void PScriptLanguage::InternalRemoveFunction(const PString & prefix)
{
  FunctionMap::iterator it = m_functions.lower_bound(prefix);
  while (it != m_functions.end() && it->first.NumCompare(prefix) == EqualTo) {
    if (isalnum(it->first[prefix.GetLength()]))
      ++it;
    else
      m_functions.erase(it++);
  }
}
