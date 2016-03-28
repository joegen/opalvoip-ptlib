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

/*
  Requires V8 version with SVN tag http://v8.googlecode.com/svn/tags/3.26.11

  Need to build V8 DLL's for compatibility across compilers.

  For Windows the following commands were used to build the VisualStudio solution:
    svn co http://v8.googlecode.com/svn/tags/3.26.11 where/ever
    cd where/ever
    svn co http://gyp.googlecode.com/svn/trunk build/gyp
    svn co http://src.chromium.org/svn/trunk/deps/third_party/cygwin@66844 third_party/cygwin
    svn co http://src.chromium.org/svn/trunk/tools/third_party/python_26@89111 third_party/python_26
    .\third_party\cygwin\bin\bash.exe -c "./third_party/python_26/python26.exe ./build/gyp_v8 -Dtarget_arch=ia32 -Dcomponent=shared_library"

  then use .\build\all.sln
*/

#ifdef _MSC_VER
  #pragma warning(disable:4100 4127)
#endif

#include <ptclib/jscript.h>

#include <iomanip>

#include <v8.h>

#ifdef _MSC_VER
  #if defined(_DEBUG)
    #pragma comment(lib, P_V8_DEBUG_LIB)
  #else
    #pragma comment(lib, P_V8_RELEASE_LIB)
  #endif
#endif


#define PTraceModule() "JavaScript"


static PConstString const JavaName("Java");
PFACTORY_CREATE(PFactory<PScriptLanguage>, PJavaScript, JavaName, false);

static atomic<bool> V8_initialised;

#ifndef P_V8_API
  #define P_V8_API 1
#endif

#if PTRACING
  #if P_V8_API > 2
    static void LogEventCallback(const char * PTRACE_PARAM(name), int PTRACE_PARAM(event))
    {
      PTRACE(4, "V8-Log", "Event=" << event << " - " << name);
    }
  #endif

  #if P_V8_API > 1
    static void TraceFunction(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
      if (args.Length() < 2)
        return;

      if (!args[0]->IsUint32())
        return;

      unsigned level = args[0]->Uint32Value();
      if (!PTrace::CanTrace(level))
        return;

      ostream & trace = PTRACE_BEGIN(level);
      for (int i = 1; i < args.Length(); i++) {
        v8::HandleScope handle_scope(args.GetIsolate());
        if (i > 0)
          trace << ' ';
        v8::String::Utf8Value str(args[i]);
        if (*str)
          trace << *str;
      }

      trace << PTrace::End;
    }
  #endif // P_V8_API
#endif // PTRACING


PINDEX ParseKey(const PString & name, PStringArray & tokens)
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
      PINDEX end = element.Find(']', start + 1);
      if (end != P_MAX_INDEX) {
        tokens[i] = element(0, start - 1);
        ++i;
        tokens.InsertAt(i, new PString(element(start, end - 1)));
        if (end < element.GetLength() - 1) {
          i++;
          tokens.InsertAt(i, new PString(element(end + 1, P_MAX_INDEX)));
        }
        else {
        }
      }
      ++i;
    }
  }

  return tokens.GetSize();
}


struct PJavaScript::Private : PObject
{
  PCLASSINFO(PJavaScript::Private, PObject);
private:
  v8::Isolate * m_isolate;
  v8::Persistent<v8::Context> m_context;

#if P_V8_API > 1
  struct HandleScope : v8::HandleScope
  {
    HandleScope(Private * prvt) : v8::HandleScope(prvt->m_isolate) { }
  };

  struct EscapableHandleScope : v8::EscapableHandleScope
  {
    EscapableHandleScope(Private * prvt) : v8::EscapableHandleScope(prvt->m_isolate) { }
  };

  v8::Local<v8::String> NewString(const char * str) const { return v8::String::NewFromUtf8(m_isolate, str); }
  template <class Type, typename Param> v8::Handle<v8::Value> NewObject(Param param) const { return Type::New(m_isolate, param); }
  v8::Local<v8::Context> GetContext() const { return v8::Local<v8::Context>::New(m_isolate, m_context); }
#else
  struct HandleScope : v8::HandleScope
  {
    HandleScope(Private *) { }
  };

  struct EscapableHandleScope : v8::HandleScope
  {
    EscapableHandleScope(Private *) { }
    template <class T> v8::Local<T> Escape(v8::Local<T> value) { return v8::HandleScope::Close(value); }
  };

  v8::Local<v8::String> NewString(const char * str) const { return v8::String::New(str); }
  template <class CLS, typename Param> v8::Handle<v8::Value> NewObject(Param param) const { return CLS::New(param); }
  v8::Local<v8::Context> GetContext() const { return v8::Local<v8::Context>::New(m_context); }
#endif


public:
  Private()
  {
    if (!V8_initialised.exchange(true)) {
#if P_V8_API > 1
      v8::V8::InitializeICU();
#endif
    }

    m_isolate = v8::Isolate::New();

    v8::Isolate::Scope isolateScope(m_isolate);
    HandleScope handleScope(this);

#if P_V8_API > 1
  #if PTRACING
    #if P_V8_API > 2
      m_isolate->SetEventLogger(LogEventCallback);
    #endif

    // Bind the global 'PTRACE' function to the PTLib trace callback.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_isolate);
    global->Set(NewString("PTRACE"), v8::FunctionTemplate::New(m_isolate, TraceFunction));

    m_context.Reset(m_isolate, v8::Context::New(m_isolate, NULL, global));
  #else
    m_context.Reset(m_isolate, v8::Context::New(m_isolate));
  #endif
#else
    m_context = v8::Context::New();
#endif
  }


  ~Private()
  {
    if (m_isolate != NULL)
      m_isolate->Dispose();
  }

  v8::Handle<v8::Value> GetMember(v8::Handle<v8::Object> object, const PString & name)
  {
    v8::Local<v8::Value> value;

    if (m_isolate == NULL)
      return value;

    // set flags if array access
    if (name[0] == '[')
      value = object->Get(name.Mid(1).AsInteger());
    else
      value = object->Get(NewString(name));

    return value;
  }
  
  
  bool SetMember(v8::Handle<v8::Object> object, const PString & name, v8::Handle<v8::Value> value)
  {
    if (m_isolate == NULL)
      return false;

    // set flags if array access
    if (name[0] == '[')
      return object->Set(name.Mid(1).AsInteger(), value);
    else
      return object->Set(NewString(name), value);
  }


  bool GetVar(const PString & key, PVarType & var)
  {
    if (m_isolate == NULL)
      return false;

    v8::Isolate::Scope isolateScope(m_isolate);
    HandleScope handleScope(this);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    PStringArray tokens;
    if (ParseKey(key, tokens) < 1) {
      PTRACE(5, "GetVar '" << key << " is too short");
      return false;
    }

    v8::Handle<v8::Value> value;
    v8::Handle<v8::Object> object = context->Global();

    int i = 0;

    for (;;) {

      // get the member variable
      value = GetMember(object, tokens[i]);
      if (value.IsEmpty()) {
        PTRACE(5, "Cannot get element '" << tokens[i] << "'");
        return false;
      }

      // see if end of path
      if (i == (tokens.GetSize() - 1))
        break;

      // terminals must not be composites, internal nodes must be composites
      bool isObject = value->IsObject();
      if (!isObject) {
        tokens.SetSize(i + 1);
        PTRACE(5, "GetVar intermediate node '" << setfill('.') << tokens << "' is not a composite");
        return false;
      }

      // if path has ended, return error
      object = value->ToObject();
      if (object->IsNull()) {
        tokens.SetSize(i + 1);
        PTRACE(5, "GetVar intermediate node '" << setfill('.') << tokens << " not found");
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
      var = PVarType(PString(*v8::String::Utf8Value(value->ToString())));
      return true;
    }

    PTRACE(5, "Unable to determine type of '" << key << "' = " << *v8::String::Utf8Value(value));
    return false;
  }


  bool NewObject(const PString & name)
  {
    return false;
  }


  bool SetVar(const PString & key, const PVarType & var)
  {
    if (m_isolate == NULL)
      return false;

    v8::Isolate::Scope isolateScope(m_isolate);
    HandleScope handleScope(this);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    PStringArray tokens;
    if (ParseKey(key, tokens) < 1) {
      PTRACE(3, "SetVar \"" << key << "\" is too short");
      return false;
    }

    v8::Handle<v8::Object> object = context->Global();

    PINDEX i;
    for (i = 0; i < tokens.GetSize()-1; ++i) {
      // get the member variable
      v8::Handle<v8::Value> value = GetMember(object, tokens[i]);
      if (value.IsEmpty()) {
        PTRACE(3, "Cannot get element \"" << tokens[i] << '"');
        return false;
      }

      // if path has ended, return error
      object = value->ToObject();
      if (object.IsEmpty()) {
        PTRACE(3, "SetVar intermediate element \"" << tokens[i] << "\" not found");
        return false;
      }

      // terminals must not be composites, internal nodes must be composites
      bool isObject = value->IsObject();
      if (!isObject) {
        PTRACE(3, "SetVar intermediate element \"" << tokens[i] << "\" is not a composite");
        return false;
      }
    }

    v8::Handle<v8::Value> value;

    switch (var.GetType()) {
      case PVarType::VarNULL:
        //return object->Set(strKey, v8::Null::New());
        break;

      case PVarType::VarBoolean:
        value = NewObject<v8::Boolean>(var.AsBoolean());
        break;

      case PVarType::VarChar:
      case PVarType::VarStaticString:
      case PVarType::VarFixedString:
      case PVarType::VarDynamicString:
      case PVarType::VarGUID:
        value = NewString(var.AsString());
        break;

      case PVarType::VarInt8:
      case PVarType::VarInt16:
      case PVarType::VarInt32:
        value = NewObject<v8::Int32>(var.AsInteger());
        break;

      case PVarType::VarUInt8:
      case PVarType::VarUInt16:
      case PVarType::VarUInt32:
        value = NewObject<v8::Uint32>(var.AsUnsigned());
        break;

      case PVarType::VarInt64:
      case PVarType::VarUInt64:
        // Until V8 suppies a 64 bit integer, we use double

      case PVarType::VarFloatSingle:
      case PVarType::VarFloatDouble:
      case PVarType::VarFloatExtended:
        value = NewObject<v8::Number>(var.AsFloat());
        break;

      case PVarType::VarTime:
      case PVarType::VarStaticBinary:
      case PVarType::VarDynamicBinary:
      default:
#if P_V8_API > 1
        value = v8::Object::New(m_isolate);
#else
        value = v8::Object::New();
#endif
        break;
    }

    return SetMember(object, tokens[i], value);
  }


  bool Run(const char * text, PString & resultText)
  {
    if (m_isolate == NULL)
      return false;

    // create a V8 scopes
    v8::Isolate::Scope isolateScope(m_isolate);
    HandleScope handleScope(this);

    // make context scope availabke
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    // create V8 string to hold the source
    v8::Handle<v8::String> source = NewString(text);

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
    resultText = *v8::String::Utf8Value(result);
    PTRACE(1, "Returned '" << resultText << "'");

    return true;
  }
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PJavaScript::PJavaScript()
  : m_private(new Private)
{
  PTRACE_CONTEXT_ID_TO(m_private);
}


PJavaScript::~PJavaScript()
{
  delete m_private;
}


PString PJavaScript::LanguageName()
{
  return JavaName;
}


PString PJavaScript::GetLanguageName() const
{
  return JavaName;
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
  return m_private->Run(text, m_resultText);
}


bool PJavaScript::CreateComposite(const PString & name)
{
  PVarType dummy;
  dummy.SetType(PVarType::VarStaticBinary);
  return m_private->SetVar(name, dummy);
}


bool PJavaScript::GetVar(const PString & key, PVarType & var)
{
  return m_private->GetVar(key, var);
}

bool PJavaScript::SetVar(const PString & key, const PVarType & var)
{
  return m_private->SetVar(key, var);
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
#endif // P_V8

