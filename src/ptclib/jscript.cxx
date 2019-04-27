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
 */

#ifdef __GNUC__
#pragma implementation "jscript.h"
#endif

#include <ptlib.h>

#if P_V8

#include <ptlib/pprocess.h>
#include <ptlib/notifier_ext.h>


/*
Requires Google's V8 version 6 or later

For Unix variants, follow build isntructions for v8 or just use distro.

For Windows the following commands was used to build V8:

Install Visual Studio 2015, note you should do a full isntallation, in
    particular the "Windows SDK" component.
Install Windows 10 SDK (yes, in addition to the above) from
    https://developer.microsoft.com/en-us/windows/downloads/windows-10-sdk
Download https://storage.googleapis.com/chrome-infra/depot_tools.zip and
    unpack to somehere, e.g. C:\tools\depot_tools
set DEPOT_TOOLS_WIN_TOOLCHAIN=0
set GYP_MSVS_VERSION=2015
PATH=C:\tools\depot_tools;%PATH%
mkdir <v8-dir>   ;e.g. if PTLib is in C:\Work\ptlib, use C:\Work\v8 or C:\Work\external\v8
cd <v8-dir>
fetch v8
Wait a while, this step is notorious for failing in myserious ways
cd v8
Edit build\config\win\BUILD.gn and change ":static_crt" to ":dynamic_crt"

Then do any or all of the following
.\tools\dev\v8gen.py x64.release -- v8_static_library=true is_component_build=false
ninja -C out.gn\x64.release

.\tools\dev\v8gen.py x64.debug -- v8_static_library=true is_component_build=false
ninja -C out.gn\x64.debug

.\tools\dev\v8gen.py ia32.release -- v8_static_library=true is_component_build=false
ninja -C out.gn\ia32.release

.\tools\dev\v8gen.py ia32.debug -- v8_static_library=true is_component_build=false
ninja -C out.gn\ia32.debug

Then reconfigure PTLib.

Note that three files, icudtl.dat, natives_blob.bin & snapshot_blob.bin, must be
copied to the executable directory of any application that uses the V8 system.
They are usually in the output directory of the build, e.g. out.gn\x64.release
*/

#ifdef _MSC_VER
  #pragma warning(disable:4100 4127)
#endif

#include <ptclib/jscript.h>

#include <iomanip>

#define V8_DEPRECATION_WARNINGS 1
#include <v8.h>

#ifndef V8_MAJOR_VERSION
  #error "Unsupported V8 version!"
#endif

#include <libplatform/libplatform.h>


#ifdef _MSC_VER
  #pragma comment(lib, "winmm.lib")
  #pragma comment(lib, "dbghelp.lib")
  #pragma comment(lib, "shlwapi.lib")
  #if defined(_DEBUG)
    #if defined(P_64BIT)
      #pragma comment(lib, P_V8_BASE0_DEBUG64)
      #pragma comment(lib, P_V8_BASE1_DEBUG64)
      #pragma comment(lib, P_V8_LIBBASE_DEBUG64)
      #pragma comment(lib, P_V8_SNAPSHOT_DEBUG64)
      #pragma comment(lib, P_V8_LIBPLATFORM_DEBUG64)
      #pragma comment(lib, P_V8_LIBSAMPLER_DEBUG64)
      #pragma comment(lib, P_V8_ICUI18N_DEBUG64)
      #pragma comment(lib, P_V8_ICUUC_DEBUG64)
      #define V8_BLOBS_DIR V8_DIR "/out.gn/x64.debug"
    #else
      #pragma comment(lib, P_V8_BASE0_DEBUG32)
      #pragma comment(lib, P_V8_BASE1_DEBUG32)
      #pragma comment(lib, P_V8_LIBBASE_DEBUG32)
      #pragma comment(lib, P_V8_SNAPSHOT_DEBUG32)
      #pragma comment(lib, P_V8_LIBPLATFORM_DEBUG32)
      #pragma comment(lib, P_V8_LIBSAMPLER_DEBUG32)
      #pragma comment(lib, P_V8_ICUI18N_DEBUG32)
      #pragma comment(lib, P_V8_ICUUC_DEBUG32)
      #define V8_BLOBS_DIR V8_DIR "/out.gn/ia32.debug"
    #endif
  #else
    #if defined(P_64BIT)
      #pragma comment(lib, P_V8_BASE0_RELEASE64)
      #pragma comment(lib, P_V8_BASE1_RELEASE64)
      #pragma comment(lib, P_V8_LIBBASE_RELEASE64)
      #pragma comment(lib, P_V8_SNAPSHOT_RELEASE64)
      #pragma comment(lib, P_V8_LIBPLATFORM_RELEASE64)
      #pragma comment(lib, P_V8_LIBSAMPLER_RELEASE64)
      #pragma comment(lib, P_V8_ICUI18N_RELEASE64)
      #pragma comment(lib, P_V8_ICUUC_RELEASE64)
      #define V8_BLOBS_DIR V8_DIR "/out.gn/x64.release"
    #else
      #pragma comment(lib, P_V8_BASE0_RELEASE32)
      #pragma comment(lib, P_V8_BASE1_RELEASE32)
      #pragma comment(lib, P_V8_LIBBASE_RELEASE32)
      #pragma comment(lib, P_V8_SNAPSHOT_RELEASE32)
      #pragma comment(lib, P_V8_LIBPLATFORM_RELEASE32)
      #pragma comment(lib, P_V8_LIBSAMPLER_RELEASE32)
      #pragma comment(lib, P_V8_ICUI18N_RELEASE32)
      #pragma comment(lib, P_V8_ICUUC_RELEASE32)
      #define V8_BLOBS_DIR V8_DIR "/out.gn/ia32.release"
    #endif
  #endif
#endif


#define PTraceModule() "JavaScript"


static PConstString const JavaName("JavaScript");
PFACTORY_CREATE(PFactory<PScriptLanguage>, PJavaScript, JavaName, false);


#if PTRACING
    static void LogEventCallback(const char * PTRACE_PARAM(name), int PTRACE_PARAM(event))
    {
      PTRACE(4, "V8-Log", "Event=" << event << " - " << name);
    }

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
        v8::String::Utf8Value str(args.GetIsolate(), args[i]);
        if (*str)
          trace << *str;
      }

      trace << PTrace::End;
    }
#endif // PTRACING


#ifdef V8_BLOBS_DIR
static bool MyInitializeExternalStartupData(const PDirectory dir)
{
  /* For some exceptionally stupid reason, the initialisation function for the
     external start up files returns no success/failure, it just crashes later
     when you try and create an Isolate object */
  if (!PFile::Exists(dir + "snapshot_blob.bin"))
    return false;
  if (!PFile::Exists(dir + "natives_blob.bin"))
    return false;

  v8::V8::InitializeExternalStartupData(dir);
  return true;
}
#endif // V8_BLOBS_DIR


struct PJavaScript::Private : PObject
{
  PCLASSINFO(PJavaScript::Private, PObject);

  PJavaScript & m_owner;

  v8::Isolate::CreateParams   m_isolateParams;
  v8::Isolate               * m_isolate;
  v8::Persistent<v8::Context> m_context;

  v8::Local<v8::String> NewString(const char * str) const { return v8::String::NewFromUtf8(m_isolate, str); }
  template <class Type, typename Param> v8::Handle<v8::Value> NewObject(Param param) const { return Type::New(m_isolate, param); }
  v8::Local<v8::Context> GetContext() const { return v8::Local<v8::Context>::New(m_isolate, m_context); }
  template <typename T> PString ToPString(const T & value) { return *v8::String::Utf8Value(m_isolate, value);  }

  struct Intialisation
  {
    v8::Platform * m_platform;
    bool           m_initialised;

    Intialisation()
      : m_platform(NULL)
      , m_initialised(false)
    {
      PDirectory exeDir = PProcess::Current().GetFile().GetDirectory();

#ifdef V8_BLOBS_DIR
      // Initialise some basics
      if (!MyInitializeExternalStartupData(exeDir)) {
        const char * dir = getenv("V8_BLOBS_DIR");
        if (dir == NULL || !MyInitializeExternalStartupData(dir)) {
          if (!MyInitializeExternalStartupData(V8_BLOBS_DIR)) {
            PTRACE(1, NULL, PTraceModule(), "v8::V8::InitializeExternalStartupData() failed, not loaded.");
            return;
          }
        }
      }
#endif // V8_BLOBS_DIR

#if V8_MAJOR_VERSION < 6
      if (!v8::V8::InitializeICU(exeDir)) {
#else
      if (!v8::V8::InitializeICUDefaultLocation(exeDir)) {
#endif
        PTRACE(2, PTraceModule(), "v8::V8::InitializeICUDefaultLocation() failed, continuing.");
      }

      // Start it up!
      m_platform = v8::platform::CreateDefaultPlatform();
      v8::V8::InitializePlatform(m_platform);

      if (!v8::V8::Initialize()) {
        PTRACE(1, PTraceModule(), "v8::V8::Initialize() failed, not loaded.");
        return;
      }

      m_initialised = true;
      PTRACE(4, PTraceModule(), "V8 initialisation successful.");
    }

    ~Intialisation()
    {
      PTRACE(4, PTraceModule(), "V8 shutdown.");
      v8::V8::Dispose();
      v8::V8::ShutdownPlatform();
      delete m_platform;
    }
  };

#if V8_MAJOR_VERSION < 6
  class ArrayBufferAllocator : public v8::ArrayBuffer::Allocator
  {
  public:
      virtual void* Allocate(size_t length) { return calloc(length, 1); }
    virtual void* AllocateUninitialized(size_t length) { return malloc(length); }
    virtual void Free(void* data, size_t length) { free(data); }
  };
  __inline v8::ArrayBuffer::Allocator * NewDefaultAllocator() { return new ArrayBufferAllocator; }
#else
  __inline v8::ArrayBuffer::Allocator * NewDefaultAllocator() { return v8::ArrayBuffer::Allocator::NewDefaultAllocator(); }
#endif // V8_MAJOR_VERSION < 6


public:
  Private(PJavaScript & owner)
    : m_owner(owner)
    , m_isolate(NULL)
  {
    if (!PSafeSingleton<Intialisation>()->m_initialised)
      return;

    PTRACE_CONTEXT_ID_FROM(owner);

    m_isolateParams.array_buffer_allocator = NewDefaultAllocator();
    m_isolate = v8::Isolate::New(m_isolateParams);
    if (m_isolate == NULL) {
      PTRACE(1, "Could not create context.");
      return;
    }

    v8::Isolate::Scope isolateScope(m_isolate);
    v8::HandleScope handleScope(m_isolate);

  #if PTRACING
    m_isolate->SetEventLogger(LogEventCallback);

    // Bind the global 'PTRACE' function to the PTLib trace callback.
    v8::Handle<v8::ObjectTemplate> global = v8::ObjectTemplate::New(m_isolate);
    global->Set(NewString("PTRACE"), v8::FunctionTemplate::New(m_isolate, TraceFunction));

    m_context.Reset(m_isolate, v8::Context::New(m_isolate, NULL, global));
    PTRACE(5, "Created context.");
  #else
    m_context.Reset(m_isolate, v8::Context::New(m_isolate));
  #endif
  }


  ~Private()
  {
    PTRACE(5, "Destroying context.");

    if (m_isolate != NULL)
      m_isolate->Dispose();

    delete m_isolateParams.array_buffer_allocator;
  }


  v8::Handle<v8::Value> GetMember(v8::Handle<v8::Object> object, const PString & name)
  {
    if (object.IsEmpty())
      return v8::Handle<v8::Value>();

    return name[0] != '[' ? object->Get(NewString(name)) : object->Get(name.Mid(1).AsInteger());
  }
  
  
  bool SetMember(v8::Handle<v8::Object> object,
                 const PString & name,
                 v8::Handle<v8::Value> value
                 PTRACE_PARAM(, const PString & key))
  {
    if (name[0] != '[' ? object->Set(NewString(name), value)
                       : object->Set(name.Mid(1).AsUnsigned(), value))
    {
      PTRACE(5, "Set \"" << key << "\" to " << ToPString(value).Left(100).ToLiteral());
      return true;
    }

    PTRACE(3, "Could not set \"" << key << '"');
    return false;
  }


  v8::Handle<v8::Object> GetObjectHandle(const v8::Local<v8::Context> & context, const PString & key, PString & var)
  {
    PStringArray tokens = key.Tokenise('.', false);
    if (tokens.GetSize() < 1) {
      PTRACE(3, "SetVar \"" << key << "\" is too short");
      return v8::Handle<v8::Object>();
    }

    for (PINDEX i = 0; i < tokens.GetSize(); ++i) {
      PString element = tokens[i];
      PINDEX start = element.Find('[');
      if (start != P_MAX_INDEX) {
        PINDEX end = element.Find(']', start + 1);
        if (end != P_MAX_INDEX) {
          tokens[i] = element(0, start - 1);
          tokens.InsertAt(++i, new PString(element(start, end - 1)));
          if (end < element.GetLength() - 1) {
            i++;
            tokens.InsertAt(i, new PString(element(end + 1, P_MAX_INDEX)));
          }
        }
      }
    }

    v8::Handle<v8::Object> object = context->Global();

    PINDEX i;
    for (i = 0; i < tokens.GetSize()-1; ++i) {
      // get the member variable
      v8::Handle<v8::Value> value = GetMember(object, tokens[i]);
      if (value.IsEmpty()) {
        PTRACE(3, "Cannot get element \"" << tokens[i] << '"');
        object.Clear();
        break;
      }

      // if path has ended, return error
      object = value->ToObject();
      if (object.IsEmpty()) {
        PTRACE(3, "Intermediate element \"" << tokens[i] << "\" not found");
        object.Clear();
        break;
      }

      // terminals must not be composites, internal nodes must be composites
      bool isObject = value->IsObject();
      if (!isObject) {
        PTRACE(3, "Intermediate element \"" << tokens[i] << "\" is not a composite");
        object.Clear();
        break;
      }
    }

    var = tokens[i];
    return object;
  }


  bool GetVarValue(v8::Handle<v8::Value> value, PVarType & var PTRACE_PARAM(, const PString & key))
  {
    if (value.IsEmpty()) {
      PTRACE(3, "Cannot get value for \"" << key << '"');
      return false;
    }

    if (value->IsInt32()) {
      var = PVarType(value->Int32Value());
      PTRACE(5, "Got Int32 \"" << key << "\" = " << var);
      return true;
    }

    if (value->IsUint32()) {
      var = PVarType(value->Uint32Value());
      PTRACE(5, "Got Uint32 \"" << key << "\" = " << var);
      return true;
    }

    if (value->IsNumber()) {
      var = PVarType(value->NumberValue());
      PTRACE(5, "Got Number \"" << key << "\" = " << var);
      return true;
    }

    if (value->IsBoolean()) {
      var = PVarType(value->BooleanValue());
      PTRACE(5, "Got Boolean \"" << key << "\" = " << var);
      return true;
    }

    if (value->IsString()) {
      var = PVarType(ToPString(value->ToString()));
      PTRACE(5, "Got String \"" << key << "\" = " << var.AsString().Left(100).ToLiteral());
      return true;
    }

    PTRACE(3, "Unable to determine type of \"" << key << "\" = \"" << ToPString(value) << '"');
    var.SetType(PVarType::VarNULL);
    return false;
  }


  bool GetVar(const PString & key, PVarType & var)
  {
    if (m_isolate == NULL)
      return false;

    v8::Isolate::Scope isolateScope(m_isolate);
    v8::HandleScope handleScope(m_isolate);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    PString varName;
    v8::Handle<v8::Object> object = GetObjectHandle(context, key, varName);
    return !object.IsEmpty() && GetVarValue(GetMember(object, varName), var PTRACE_PARAM(, key));
  }


  bool NewObject(const PString & name)
  {
    return false;
  }


  v8::Handle<v8::Value> SetVarValue(const PVarType & var)
  {
    switch (var.GetType()) {
      case PVarType::VarNULL:
        return v8::Handle<v8::Value>();

      case PVarType::VarBoolean:
        return NewObject<v8::Boolean>(var.AsBoolean());

      case PVarType::VarChar:
      case PVarType::VarStaticString:
      case PVarType::VarFixedString:
      case PVarType::VarDynamicString:
      case PVarType::VarGUID:
        return NewString(var.AsString());

      case PVarType::VarInt8:
      case PVarType::VarInt16:
      case PVarType::VarInt32:
        return NewObject<v8::Int32>(var.AsInteger());

      case PVarType::VarUInt8:
      case PVarType::VarUInt16:
      case PVarType::VarUInt32:
        return NewObject<v8::Uint32>(var.AsUnsigned());

      case PVarType::VarInt64:
      case PVarType::VarUInt64:
        // Until V8 suppies a 64 bit integer, we use double

      case PVarType::VarFloatSingle:
      case PVarType::VarFloatDouble:
      case PVarType::VarFloatExtended:
        return NewObject<v8::Number>(var.AsFloat());

      case PVarType::VarTime:
      case PVarType::VarStaticBinary:
      case PVarType::VarDynamicBinary:
      default:
        return v8::Object::New(m_isolate);
    }
  }


  bool SetVar(const PString & key, const PVarType & var)
  {
    if (m_isolate == NULL)
      return false;

    v8::Isolate::Scope isolateScope(m_isolate);
    v8::HandleScope handleScope(m_isolate);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    PString varName;
    v8::Handle<v8::Object> object = GetObjectHandle(context, key, varName);
    return !object.IsEmpty() && SetMember(object, varName, SetVarValue(var) PTRACE_PARAM(, key));
  }


  struct NotifierInfo {
    PJavaScript    & m_owner;
    PString          m_key;
    PNotifierListTemplate<Signature &> m_notifiers;
    NotifierInfo(PJavaScript & owner, const PString & key)
      : m_owner(owner)
      , m_key(key)
    { }
  };
  std::map<PString, NotifierInfo> m_notifiers;

  void FunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& callbackInfo, NotifierInfo & notifierInfo)
  {
    Signature signature;

    int nargs = callbackInfo.Length();
    signature.m_arguments.resize(nargs);
    for (int arg = 0; arg < nargs; ++arg)
      GetVarValue(callbackInfo[arg], signature.m_arguments[arg] PTRACE_PARAM(, notifierInfo.m_key));

    notifierInfo.m_notifiers(notifierInfo.m_owner, signature);

    if (!signature.m_results.empty())
      callbackInfo.GetReturnValue().Set(SetVarValue(signature.m_results[0]));
  }

  static void StaticFunctionCallback(const v8::FunctionCallbackInfo<v8::Value>& callbackInfo)
  {
    v8::Local<v8::Value> callbackData = callbackInfo.Data();
    if (callbackData.IsEmpty() || !callbackData->IsExternal())
      return;

    NotifierInfo & notifierInfo = *reinterpret_cast<NotifierInfo *>(v8::Local<v8::External>::Cast(callbackData)->Value());
    notifierInfo.m_owner.m_private->FunctionCallback(callbackInfo, notifierInfo);
  }

  bool SetFunction(const PString & key, const FunctionNotifier & notifier)
  {
    if (m_isolate == NULL)
      return false;

    v8::Isolate::Scope isolateScope(m_isolate);
    v8::HandleScope handleScope(m_isolate);
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    PString varName;
    v8::Handle<v8::Object> object = GetObjectHandle(context, key, varName);
    if (object.IsEmpty())
      return false;

    std::map<PString, NotifierInfo>::iterator it = m_notifiers.find(key);
    if (it == m_notifiers.end())
      it = m_notifiers.insert(make_pair(key, NotifierInfo(m_owner, key))).first;
    NotifierInfo & info = it->second;
    info.m_notifiers.Add(notifier);

    v8::Local<v8::External> data = v8::External::New(m_isolate, &info);
    v8::Handle<v8::Value> value = v8::Function::New(m_isolate, StaticFunctionCallback, data);

    if (object->Set(NewString(varName), value)) {
      PTRACE(5, "Set function \"" << varName << '"');
      return true;
    }

    PTRACE(3, "Could not set function \"" << varName << '"');
    return false;
  }


  bool Run(const PString & text, PString & resultText)
  {
    if (m_isolate == NULL)
      return false;

    // create a V8 scopes
    v8::Isolate::Scope isolateScope(m_isolate);
    v8::HandleScope handleScope(m_isolate);

    // make context scope availabke
    v8::Local<v8::Context> context = GetContext();
    v8::Context::Scope contextScope(context);

    // compile the source 
    v8::Local<v8::Script> script;
    if (!v8::Script::Compile(context, NewString(text)).ToLocal(&script)) {
      PTRACE(3, "Could not compile source " << text.Left(100).ToLiteral());
      return false;
    }

    // run the code
    v8::Handle<v8::Value> result;
    if (!script->Run(context).ToLocal(&result)) {
      PTRACE(3, "Script execution did not return a result.");
      return false;
    }

    resultText = ToPString(result);
    PTRACE(4, "Script execution returned " << resultText.Left(100).ToLiteral());

    return true;
  }
};


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PJavaScript::PJavaScript()
  : m_private(new Private(*this))
{
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


bool PJavaScript::IsInitialised() const
{
  return m_private->m_isolate != NULL;
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
  PString script(text);

  PTextFile file(text, PFile::ReadOnly);
  if (file.IsOpen()) {
    PTRACE(4, "Reading script file: " << file.GetFilePath());
    script = file.ReadString(P_MAX_INDEX);
  }

  return m_private->Run(script, m_resultText);
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


bool PJavaScript::SetFunction(const PString & name, const FunctionNotifier & func)
{
  return m_private->SetFunction(name, func);
}
#endif // P_V8

