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

#if P_V8

#ifndef __clang__
  #pragma message("JavaScript support enabled")
#endif

#ifdef _MSC_VER
  #pragma warning(disable:4100 4127)
#endif

#include <ptclib/jscript.h>

#include <iomanip>

#include <v8.h>

#ifdef _MSC_VER
  #pragma comment(lib, P_V8_LIBRARY1)
  #pragma comment(lib, P_V8_LIBRARY2)
  #pragma comment(lib, "winmm.lib")
#endif


#define PTraceModule() "JavaScript"


PFACTORY_CREATE(PFactory<PScriptLanguage>, PJavaScript, "Java", false);


struct PJavaScript::Private {
  v8::Isolate * m_isolate;
  v8::Handle<v8::Context> m_context;

  
  v8::Handle<v8::Value> GetMember(v8::Handle<v8::Object> object, const PString & name)
  {
    v8::HandleScope handleScope(m_isolate);
    v8::Local<v8::Value> value;
    
    // set flags if array access
    if (name[0] == '[')
      value = object->Get(name.Mid(1).AsInteger());
    else
      value = object->Get(v8::String::New((const char *)name));
    
    return handleScope.Close(value);
  }
  
  
  void SetMember(v8::Handle<v8::Object> object, const PString & name, v8::Handle<v8::Value> value)
  {
    v8::HandleScope handleScope(m_isolate);
    
    // set flags if array access
    if (name[0] == '[')
      object->Set(name.Mid(1).AsInteger(), value);
    else
      object->Set(v8::String::New((const char *)name), value);
  }
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PJavaScript::PJavaScript()
  : m_private(new Private)
{
  v8::V8::InitializeICU();
  m_private->m_isolate = v8::Isolate::GetCurrent();

  // V8 is full of globals, so we have to lock it. Sigh....
  v8::Locker locker(m_private->m_isolate);

  // create a V8 handle scope
  v8::HandleScope handleScope(m_private->m_isolate);

  // create a V8 context
  m_private->m_context = v8::Context::New(m_private->m_isolate);

  // make context scope available
  v8::Context::Scope contextScope(m_private->m_context);
}


PJavaScript::~PJavaScript()
{
  // V8 is full of globals, so we have to lock it. Sigh....
  v8::Locker locker(m_private->m_isolate);

  delete m_private;
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
  v8::Locker locker(m_private->m_isolate);

  // create a V8 handle scope
  v8::HandleScope handleScope(m_private->m_isolate);

  // make context scope availabke
  v8::Context::Scope contextScope(m_private->m_context);

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

PINDEX PJavaScript::ParseKey(const PString & name, PStringArray & tokens)
{
  tokens = name.Tokenise('.', false);
  if (tokens.GetSize() < 1) {
    PTRACE(5, "V8\tParseKey:node '" << name << " is too short");
    return 0;
  }
  PINDEX i = 0;
  while (i < tokens.GetSize()) {
    PString element = tokens[i];
    PINDEX start = element.Find('[');
    if (start == P_MAX_INDEX)
      ++i;
    else {
      PINDEX end = element.Find(']', start+1);
      if (end != P_MAX_INDEX) {
        tokens[i] = element(0, start-1);
        ++i;
        tokens.InsertAt(i, new PString(element(start, end-1)));
        if (end < element.GetLength()-1) {
          i++;
          tokens.InsertAt(i, new PString(element(end+1, P_MAX_INDEX)));
        }
        else {
        }
      }
      ++i;
    }
  }

  return tokens.GetSize();
}


bool PJavaScript::GetVar(const PString & key, PVarType & var)
{
  v8::Locker locker(m_private->m_isolate);
  v8::HandleScope handleScope(m_private->m_isolate);
  v8::Context::Scope contextScope(m_private->m_context);

  PStringArray tokens;
  if (ParseKey(key, tokens) < 1) {
    PTRACE(5, "V8\tGetVar '" << key << " is too short");
    return false;
  }

  v8::Handle<v8::Value> value;
  v8::Handle<v8::Object> object = m_private->m_context->Global();

  int i = 0;

  for (;;)  {

    // get the member variable
    value = m_private->GetMember(object, tokens[i]);
    if (value.IsEmpty()) {
      PTRACE(5, "V8", "Cannot get element '" << tokens[i] << "'");
      return false;
    }

    // see if end of path
    if (i == (tokens.GetSize()-1)) 
      break;

    // terminals must not be composites, internal nodes must be composites
    bool isObject = value->IsObject();
    if (!isObject) {
      tokens.SetSize(i+1);
      PTRACE(5, "V8\tGetVar intermediate node '" << setfill('.') << tokens << "' is not a composite");
      return false;
    }

    // if path has ended, return error
    object = value->ToObject();
    if (object->IsNull()) {
      tokens.SetSize(i+1);
      PTRACE(5, "V8\tGetVar intermediate node '" << setfill('.') << tokens << " not found");
      return false;
    }

    i++;
  } 

  if (value->IsInt32()) {
    var = PVarType(value->Int32Value());
    return true;
  }

  if (value->IsUint32()) {
    var = PVarType(value->Uint32Value());
    return true;
  }

  if (value->IsNumber()) {
    var = PVarType(value->NumberValue());
    return true;
  }

  if (value->IsBoolean()) {
    var = PVarType(value->BooleanValue());
    return true;
  }

  if (value->IsString()) {
    v8::String::AsciiValue ascii(value->ToString()); 
    var = PVarType(PString(*ascii)); 
    return true;
  }

  v8::String::AsciiValue ascii(value);
  std::string txt = std::string(*ascii);
  PTRACE(5, "V8\tUnable to determine type of '" << key << "' = " << txt);

  return false;
}

bool PJavaScript::SetVar(const PString & key, const PVarType & var)
{
  v8::Locker locker(m_private->m_isolate);
  v8::HandleScope handleScope(m_private->m_isolate);
  v8::Context::Scope contextScope(m_private->m_context);

  PStringArray tokens;
  if (ParseKey(key, tokens) < 1) {
    PTRACE(5, "V8\tSetVar '" << key << " is too short");
    return false;
  }

  v8::Handle<v8::Object> object = m_private->m_context->Global();

  int i = 0;

  while (i > 0) {

    // get the member variable
    v8::Handle<v8::Value> value = m_private->GetMember(object, tokens[i]);
    if (value.IsEmpty()) {
      PTRACE(5, "V8", "Cannot get element '" << tokens[i] << "'");
      return false;
    }

    // terminals must not be composites, internal nodes must be composites
    bool isObject = value->IsObject();
    if (!isObject) {
      tokens.SetSize(i+1);
      PTRACE(5, "V8\tGetVar intermediate node '" << setfill('.') << tokens << "' is not a composite");
      return false;
    }

    // if path has ended, return error
    object = value->ToObject();
    if (object->IsNull()) {
      tokens.SetSize(i+1);
      PTRACE(5, "V8\tGetVar intermediate node '" << setfill('.') << tokens << " not found");
      return false;
    }

    i++;
  } 

  v8::Handle<v8::Value> value;

  switch (var.GetType()) {
    case PVarType::VarNULL:
      //return object->Set(strKey, v8::Null::New());
      break;

    case PVarType::VarBoolean:
      value = v8::Boolean::New(var.AsBoolean());
      break;

    case PVarType::VarChar:
    case PVarType::VarStaticString:
    case PVarType::VarFixedString:
    case PVarType::VarDynamicString:
    case PVarType::VarGUID:
      value = v8::String::New(var.AsString());
      break;

    case PVarType::VarInt8:
    case PVarType::VarInt16:
    case PVarType::VarInt32:
      value = v8::Int32::New(var.AsInteger());
      break;

    case PVarType::VarUInt8:
    case PVarType::VarUInt16:
    case PVarType::VarUInt32:
      value = v8::Uint32::New(var.AsUnsigned());
      break;

    case PVarType::VarInt64:
    case PVarType::VarUInt64:
      // Until V8 suppies a 64 bit integer, we use double

    case PVarType::VarFloatSingle:
    case PVarType::VarFloatDouble:
    case PVarType::VarFloatExtended:
      value = v8::Number::New(var.AsFloat());
      break;

    case PVarType::VarTime:
    case PVarType::VarStaticBinary:
    case PVarType::VarDynamicBinary:
    default:
      break;
  }

  m_private->SetMember(object, tokens[i], value); 
  return true;
}


bool PJavaScript::GetBoolean(const PString & name)
{
  PVarType var;
  if (!GetVar(name, var))
    return false;
  return var.AsBoolean();
}


bool PJavaScript::SetBoolean(const PString & name, bool value)
{
  PVarType var(value);
  return SetVar(name, value);
}


int PJavaScript::GetInteger(const PString & name)
{
  PVarType var;
  if (!GetVar(name, var))
    return 0;
  return var.AsInteger();
}


bool PJavaScript::SetInteger(const PString & name, int value)
{
  PVarType var(value);
  return SetVar(name, value);
}


double PJavaScript::GetNumber(const PString & name)
{
  PVarType var;
  if (!GetVar(name, var))
    return 0.0;
  return var.AsFloat();
}


bool PJavaScript::SetNumber(const PString & name, double value)
{
  PVarType var(value);
  return SetVar(name, value);
}


PString PJavaScript::GetString(const PString & name)
{
  PVarType var;
  if (!GetVar(name, var))
    return PString::Empty();
  return var.AsString();
}


bool PJavaScript::SetString(const PString & name, const char * value)
{
  PVarType var(value);
  return SetVar(name, value);
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
  #pragma message("JavaScript support DISABLED")
#endif // P_V8

