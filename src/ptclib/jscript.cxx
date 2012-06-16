/*
 * jscript.cxx
 *
 * Interface library for JavaScript interpreter
 *
 * Portable Tools Library
 *
 * Copyright (C) 2012 by Post Increment
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "jscript.h"
#endif

#include <ptlib.h>

#include <ptbuildopts.h>

#if P_V8

#include <ptclib/jscript.h>

#ifdef _MSC_VER
  #pragma comment(lib, P_V8_LIBRARY)
  #pragma message("JavaScript support enabled")
#endif


#define PTraceModule() "JavaScript"

#define new PNEW


#if PTRACING
#endif

///////////////////////////////////////////////////////////////////////////////

PJavaScript::PJavaScript()
{
}


PJavaScript::~PJavaScript()
{
}


bool PJavaScript::LoadFile(const PFilePath & filename)
{
  return false;
}


bool PJavaScript::LoadText(const PString & text)
{
  return false;
}


bool PJavaScript::Run(const char * script)
{
  return false;
}


bool PJavaScript::GetBoolean(const PString & name)
{
  return false;
}


bool PJavaScript::SetBoolean(const PString & name, bool value)
{
  return false;
}


int PJavaScript::GetInteger(const PString & name)
{
  return false;
}


bool PJavaScript::SetInteger(const PString & name, int value)
{
  return false;
}


double PJavaScript::GetNumber(const PString & name)
{
  return false;
}


bool PJavaScript::SetNumber(const PString & name, double value)
{
  return false;
}


PString PJavaScript::GetString(const PString & name)
{
  return false;
}


bool PJavaScript::SetString(const PString & name, const char * value)
{
  return false;
}


bool PJavaScript::Call(const PString & name, const char * signature, ...)
{
  return false;
}


bool PJavaScript::Call(const PString & name, Signature & signature)
{
  return false;
}


bool PJavaScript::SetFunction(const PString & name, const FunctionNotifier & func)
{
  return false;
}


bool PJavaScript::OnError(int code, const PString & str, int pop)
{
  return false;
}


#else // P_V8

  #ifdef _MSC_VER
    #pragma message("JavaScript support DISABLED")
  #endif

#endif // P_V8

