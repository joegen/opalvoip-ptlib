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

v8::Handle<v8::Object> PJavaScript::ParseKey(const PString & name, PString & lastName)
{
  PStringArray tokens = name.Tokenise('.', false);
  if (tokens.GetSize() < 1) {
    PTRACE(5, "V8\tParseKey:node '" << name << " is too short");
    return v8::Handle<v8::Object>();
  }
  PINDEX i = 0;
  while (i < tokens.GetSize()) {
    PString element = tokens[i];
PTRACE(5, "  Parsing element '" << element << "'");
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
PTRACE(5, "  Split '" << element << "' into '" << element(0, start-1) << "','" << element(start,end-1) << "','" << element(end+1, P_MAX_INDEX) << "'");
          i++;
          tokens.InsertAt(i, new PString(element(end+1, P_MAX_INDEX)));
        }
        else {
PTRACE(5, "  Split '" << element << "' into '" << element(0, start-1) << "','" << element(start,end-1) << "'");
        }
      }
      ++i;
    }
  }
  
  v8::Local<v8::Value> value = m_context->Global();
  v8::Local<v8::Object> object;

  PString soFar;
  i = 0;
  for (;;) {
    if (value->IsNull()) {
      PTRACE(5, "V8\tParseKey:node '" << soFar << " not found");
      return v8::Handle<v8::Object>();
    }
    if (i >= (tokens.GetSize()-1))
      break;
    if (value->IsObject() || value->IsArray()) {
      object = value->ToObject();
      if (tokens[i][0] == '[') {
        value = object->Get(tokens[i].Mid(1).AsInteger());
        PTRACE(5, "V8\tParseKey: array index = " << tokens[i]);
      }
      else {
        value = object->Get(v8::String::New((const char *)tokens[i]));
        PTRACE(5, "V8\tParseKey: object member = " << tokens[i]);
      }
    }
    else {
      PTRACE(5, "V8\tParseKey:node '" << soFar << "' is not a composite");
      return v8::Handle<v8::Object>();
    }
    //cerr << "  Getting node element " << i << " " << tokens[i] << endl;
    if (!soFar.IsEmpty())
      soFar += ".";
    soFar += tokens[i];
    ++i;
  } 

  lastName = tokens[i];

  return object;
}


bool PJavaScript::GetVar(const PString & name, PVarType & var)
{
  v8::Locker locker;
  v8::HandleScope handleScope;
  v8::Context::Scope contextScope(m_context);

  PString lastName;
  v8::Handle<v8::Object> object = ParseKey(name, lastName);
  if (object.IsEmpty()) {
    PTRACE(5, "V8\tGetVar:node '" << name << " is not an object");
    return false;
  }

  v8::Handle<v8::Value> value = object->Get(v8::String::New((const char *)lastName));

  {
    v8::String::AsciiValue ascii(value);
    std::string txt = std::string(*ascii);
    PTRACE(5, "V8\tExtracted '" << name << "' = " << txt);
  }

  if (value->IsInt32()) {
    var = PVarType(value->Int32Value());
    return true;
  }

  if (value->IsUint32()) {
    var = PVarType(value->Uint32Value());
    return true;
  }

  if (value->IsNumberObject()) {
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
  PTRACE(5, "V8\tUnable to determine type of '" << name << "' = " << txt);

  return false;
}
  
bool PJavaScript::SetVar(const PString & key, const PVarType & var)
{
  v8::Locker locker;
  v8::HandleScope handleScope;
  v8::Context::Scope contextScope(m_context);

  PString lastName;
  v8::Handle<v8::Object> object = ParseKey(key, lastName);
  if (object.IsEmpty()) {
    PTRACE(5, "V8\tSetVar:node '" << key << "' is not an object");
    return false;
  }

  v8::Handle<v8::String> strKey = v8::String::New((const char *)lastName);

  switch (var.GetType()) {
    case PVarType::VarNULL:
      //return object->Set(strKey, v8::Null::New());
      break;

    case PVarType::VarBoolean:
      return object->Set(strKey, v8::Boolean::New(var.AsBoolean()));

    case PVarType::VarChar:
    case PVarType::VarStaticString:
    case PVarType::VarFixedString:
    case PVarType::VarDynamicString:
    case PVarType::VarGUID:
      return object->Set(strKey, v8::String::New(var.AsString()));

    case PVarType::VarInt8:
    case PVarType::VarInt16:
    case PVarType::VarInt32:
      return object->Set(strKey, v8::Int32::New(var.AsInteger()));

    case PVarType::VarUInt8:
    case PVarType::VarUInt16:
    case PVarType::VarUInt32:
      return object->Set(strKey, v8::Uint32::New(var.AsUnsigned()));

    case PVarType::VarInt64:
      return object->Set(strKey, v8::NumberObject::New(var.AsInteger64()));

    case PVarType::VarUInt64:
      return object->Set(strKey, v8::NumberObject::New(var.AsUnsigned64()));

    case PVarType::VarFloatSingle:
    case PVarType::VarFloatDouble:
    case PVarType::VarFloatExtended:
  PTRACE(5, "V8\tSetting '" << key << "' to float '" << var << "'");
      return object->Set(strKey, v8::NumberObject::New(var.AsFloat()));
      break;

    case PVarType::VarTime:
    case PVarType::VarStaticBinary:
    case PVarType::VarDynamicBinary:
    default:
      break;
  }

  PTRACE(5, "V8\tSetting '" << key << "' to unknown type '" << var << "'");

  return false;
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

  #ifdef _MSC_VER
    #pragma message("JavaScript support DISABLED")
  #endif

#endif // P_V8

