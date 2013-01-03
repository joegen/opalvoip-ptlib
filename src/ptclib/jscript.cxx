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
  #pragma comment(lib, P_V8_LIBRARY1)
  #pragma comment(lib, P_V8_LIBRARY2)
  #pragma comment(lib, "winmm.lib")
  #pragma message("JavaScript support enabled")
#endif


#define PTraceModule() "JavaScript"

PFACTORY_CREATE(PFactory<PScriptLanguage>, PJavaScript, "Java", false);

#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PJavaScript::PJavaScript()
{
  // V8 is full of globals, so we have to lock it. Sigh....
  v8::Locker locker;

  // create a V8 handle scope
  v8::HandleScope handleScope;

  // create a V8 context
  m_context = v8::Context::New();

  // make context scope availabke
  v8::Context::Scope contextScope(m_context);
}


PJavaScript::~PJavaScript()
{
  // V8 is full of globals, so we have to lock it. Sigh....
  v8::Locker locker;

  // dispose of the global context
  m_context.Dispose();
}


bool PJavaScript::LoadFile(const PFilePath & /*filename*/)
{
  return false;
}


bool PJavaScript::LoadText(const PString & /*text*/)
{
  return false;
}


bool PJavaScript::Run(const char * text)
{
  // V8 is full of globals, so we have to lock it. Sigh....
  v8::Locker locker;

  // create a V8 handle scope
  v8::HandleScope handleScope;

  // make context scope availabke
  v8::Context::Scope contextScope(m_context);

  // create V8 string to hold the source
  v8::Handle<v8::String> source = v8::String::New(text);

  // compile the source 
  v8::Handle<v8::Script> script = v8::Script::Compile(source);
  if (*script == NULL)
    return false;

  // run the code
  v8::Handle<v8::Value> result = script->Run();

  // return error if no result
  if (*result == NULL)
    return false;

  // save return value
  v8::String::AsciiValue ascii(result);
  m_resultText = std::string(*ascii);
  PTRACE(1, "V8", "Returned '" << m_resultText << "'");

  return true;
}


bool PJavaScript::CreateComposite(const PString & /*name*/)
{
  return false;
}


bool PJavaScript::GetVar(const PString & name, PVarType & var)
{
  v8::Locker locker;
  v8::HandleScope handleScope;
  v8::Context::Scope contextScope(m_context);
  v8::Local<v8::Value> value = m_context->Global()->Get(v8::String::New(name));

  if (value->IsString()) {
    v8::String::AsciiValue ascii(value); 
    var = PVarType(*ascii); 
  }

  else if (value->IsInt32())
    var = PVarType((static_cast<v8::Int32 *>(*value))->Value()); 

  else if (value->IsUint32())
    var = PVarType((static_cast<v8::Uint32 *>(*value))->Value()); 

  else if (value->IsBoolean())
    var = PVarType((static_cast<v8::Boolean *>(*value))->Value()); 

  else if (value->IsNumber())
    var = PVarType((static_cast<v8::Number *>(*value))->Value()); 

  else if (value->IsNull())
    var = PVarType();

/*
  else if (value->IsDate()) {
    v8::Date date(value); 
    //??? unclear how to converter to date
  }
*/

  else
    return false;

  return true;
}

bool PJavaScript::SetVar(const PString & name, const PVarType & var)
{
  v8::Locker locker;
  v8::HandleScope handleScope;
  v8::Context::Scope contextScope(m_context);

  v8::Local<v8::Object> global = m_context->Global();
  v8::Handle<v8::String> key = v8::String::New(name);

  switch (var.GetType()) {
    case PVarType::VarNULL:
      global->Set(key, v8::Null());
      break;

    case PVarType::VarBoolean:
      global->Set(key, v8::Boolean::New(var.AsBoolean()));
      break;

    case PVarType::VarChar:
    case PVarType::VarStaticString:
    case PVarType::VarFixedString:
    case PVarType::VarDynamicString:
    case PVarType::VarGUID:
      global->Set(key, v8::String::New(var.AsString()));
      break;

    case PVarType::VarInt8:
    case PVarType::VarInt16:
    case PVarType::VarInt32:
      global->Set(key, v8::Int32::New(var.AsInteger()));
      break;

    case PVarType::VarUInt8:
    case PVarType::VarUInt16:
    case PVarType::VarUInt32:
      global->Set(key, v8::Uint32::New(var.AsUnsigned()));
      break;

    case PVarType::VarInt64:
    case PVarType::VarUInt64:
      // Until V8 has 64 bit integer type, use doube float
    case PVarType::VarFloatSingle:
    case PVarType::VarFloatDouble:
    case PVarType::VarFloatExtended:
      global->Set(key, v8::Number::New(var.AsFloat()));
      break;

    case PVarType::VarTime:
    case PVarType::VarStaticBinary:
    case PVarType::VarDynamicBinary:
    default:
      return false;
  }
  return true;
}

#define GET_VALUE_INTERNAL(v8Type, cppType, isFunc) \
  v8::Locker locker; \
  v8::HandleScope handleScope; \
  v8::Context::Scope contextScope(m_context); \
  v8::Local<v8::Value> value = m_context->Global()->Get(v8::String::New(name)); \
  if (!value->isFunc()) \
    return false; \

#define GET_VALUE(v8Type, cppType, isFunc) \
  GET_VALUE_INTERNAL(v8Type, cppType, isFunc) \
  return static_cast<cppType>((static_cast<v8::v8Type *>(*value))->Value()); \


bool PJavaScript::GetBoolean(const PString & name)
{
  GET_VALUE(Boolean, bool, IsBoolean);
}


bool PJavaScript::SetBoolean(const PString & name, bool value)
{
  return SetValue<v8::Boolean, bool>(name, value);
}


int PJavaScript::GetInteger(const PString & name)
{
  GET_VALUE(Integer, int, IsInt32);
}


bool PJavaScript::SetInteger(const PString & name, int value)
{
  return SetValue<v8::Integer, int>(name, value);
}


double PJavaScript::GetNumber(const PString & name)
{
  GET_VALUE(Number, double, IsNumber);
}


bool PJavaScript::SetNumber(const PString & name, double value)
{
  return SetValue<v8::Number, double>(name, value);
}


PString PJavaScript::GetString(const PString & name)
{
  GET_VALUE_INTERNAL(String, PString, IsString);
  v8::String::AsciiValue ascii(value); 
  return PString(*ascii); 
}


bool PJavaScript::SetString(const PString & name, const char * value)
{
  return SetValue<v8::String, PString>(name, value);
}


bool PJavaScript::ReleaseVariable(const PString & /*name*/)
{
  return false;
}


bool PJavaScript::Call(const PString & /*name*/, const char * /*signature*/, ...)
{
  return false;
}


bool PJavaScript::Call(const PString & /*name*/, Signature & /*signature*/)
{
  return false;
}


bool PJavaScript::SetFunction(const PString & /*name*/, const FunctionNotifier & /*func*/)
{
  return false;
}


#else // P_V8

  #ifdef _MSC_VER
    #pragma message("JavaScript support DISABLED")
  #endif

#endif // P_V8

