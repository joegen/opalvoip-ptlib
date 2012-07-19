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
 * $Revision: 27552 $
 * $Author: rjongbloed $
 * $Date: 2012-05-02 16:04:44 +1000 (Wed, 02 May 2012) $
 */

#ifdef __GNUC__
#pragma implementation "script.h"
#endif

#include <ptlib.h>

#include <ptbuildopts.h>

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
