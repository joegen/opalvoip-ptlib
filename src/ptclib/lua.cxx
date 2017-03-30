/*
 * lua.cxx
 *
 * Interface library for Lua interpreter
 *
 * Portable Tools Library
 *
 * Copyright (C) 2010 by Post Increment
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
 */

#ifdef __GNUC__
#pragma implementation "lua.h"
#endif

#include <ptlib.h>

#if P_LUA

#include <ptclib/lua.h>

extern "C" {
  #include <lua.h>
  #include <lualib.h>
  #include <lauxlib.h>
}


#ifdef _MSC_VER
  #pragma comment(lib, P_LUA_LIBRARY)
#endif

#if LUA_VERSION_NUM > 501 && !defined(lua_objlen)
  #define lua_objlen lua_rawlen
#endif

#define PTraceModule() "Lua"

static PConstString const LuaName("Lua");
PFACTORY_CREATE(PFactory<PScriptLanguage>, PLua, LuaName, false);

#define new PNEW


#if PTRACING
static int TraceFunction(lua_State * lua)
{
  int argCount = lua_gettop(lua);
  if (argCount < 2) {
    PTRACE(1, "Too few arguments for PTRACE.");
    return 0;
  }

  if (!lua_isnumber(lua, -argCount)) {
    PTRACE(1, "First PTRACE argument must be number.");
    return 0;
  }

  unsigned level = (unsigned)lua_tointeger(lua, -argCount);
  if (!PTrace::CanTrace(level))
    return 0;

  ostream & strm = PTRACE_BEGIN(level);
  for (int arg = -(argCount-1); arg < 0; ++arg)
    strm << lua_tostring(lua, arg);
  strm << PTrace::End;

  lua_pop(lua, argCount);
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////

PLua::PLua()
  : m_lua(luaL_newstate())
{
  luaL_openlibs(m_lua);

#if PTRACING
  lua_pushcfunction(m_lua, TraceFunction);
  lua_setglobal(m_lua, "PTRACE");
#endif
}


PLua::~PLua()
{
  lua_close(m_lua);
}


PString PLua::LanguageName()
{
  return LuaName;
}


PString PLua::GetLanguageName() const
{
  return LuaName;
}


bool PLua::LoadFile(const PFilePath & filename)
{
  PWaitAndSignal mutex(m_mutex);

  int err = luaL_loadfile(m_lua, filename);
  m_loaded = err == 0;
  if (m_loaded)
    return true;

  if (err != LUA_ERRFILE)
    return OnLuaError(err);

  return OnLuaError(err, PSTRSTRM("Cannot load/open file \"" << filename << '"'), 1);
}


bool PLua::LoadText(const PString & text)
{
  PWaitAndSignal mutex(m_mutex);

  m_loaded = OnLuaError(luaL_loadstring(m_lua, text));
  return m_loaded;
}


bool PLua::Run(const char * script)
{
  if (script != NULL && !LoadText(script))
    return false;

  if (IsLoaded())
    return OnLuaError(lua_pcall(m_lua, 0, 0, 0));

  return OnLuaError(LUA_ERRRUN, "Script not loaded");
}


bool PLua::CreateTable(const PString & name, const PString & metatable)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  int type = lua_type(m_lua, -1);
  lua_pop(m_lua, 1);

  switch (type) {
    case LUA_TNIL :
      break;

    case LUA_TTABLE :
      return true;

    default :
      return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Already using name \"" << name << "\" of type " << lua_typename(m_lua, type)));
  }

  lua_newtable(m_lua);

  if (!metatable.IsEmpty()) {
    luaL_newmetatable(m_lua, metatable);
    lua_setmetatable(m_lua, -2);
  }

  return InternalSetVariable(name);
}


bool PLua::DeleteTable(const PString & name, bool metaTable)
{
  PWaitAndSignal mutex(m_mutex);

  if (metaTable) {
    lua_pushnil(m_lua);
    lua_setfield(m_lua, LUA_REGISTRYINDEX, name);
  }

  if (!InternalGetVariable(name))
    return false;

  int type = lua_type(m_lua, -1);
  lua_pop(m_lua, 1);

  switch (type) {
    case LUA_TNIL :
      return true;

    case LUA_TTABLE :
      InternalRemoveFunction(name);
      lua_pushnil(m_lua);
      return InternalSetVariable(name);

    default :
      return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Not a table: \"" << name << "\" is " << lua_typename(m_lua, type)));
  }

}


bool PLua::CreateComposite(const PString & name)
{
  return CreateTable(name);
}


bool PLua::ReleaseVariable(const PString & name)
{
  return DeleteTable(name);
}


bool PLua::GetVar(const PString & name, PVarType & var)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  bool result = true;
  switch (lua_type(m_lua, -1)) {
    case LUA_TNONE:
      return false;

    case LUA_TNIL:
      var = PVarType();
      break;

    case LUA_TBOOLEAN:
      var = PVarType(lua_toboolean(m_lua, -1));
      break;

    case LUA_TNUMBER:
      var = PVarType(lua_tonumber(m_lua, -1));
      break;

    case LUA_TSTRING:
      var = PVarType(lua_tostring(m_lua, -1));
      break;

    case LUA_TLIGHTUSERDATA:
    case LUA_TTABLE:
    case LUA_TFUNCTION:
    case LUA_TUSERDATA:
    case LUA_TTHREAD:
      result = false;
  }

  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetVar(const PString & name, const PVarType & var)
{
  PWaitAndSignal mutex(m_mutex);

  switch (var.GetType()) {
    case PVarType::VarNULL:
      lua_pushnil(m_lua);
      break;

    case PVarType::VarBoolean:
      lua_pushboolean(m_lua, var.AsBoolean() ? 1 : 0);
      break;

    case PVarType::VarInt8:
    case PVarType::VarInt16:
    case PVarType::VarInt32:
    case PVarType::VarInt64:
    case PVarType::VarUInt8:
    case PVarType::VarUInt16:
    case PVarType::VarUInt32:
    case PVarType::VarUInt64:
      lua_pushinteger(m_lua, var.AsInteger());
      break;

    case PVarType::VarFloatSingle:
    case PVarType::VarFloatDouble:
    case PVarType::VarFloatExtended:
      lua_pushnumber(m_lua, var.AsFloat());
      break;

    case PVarType::VarChar:
    case PVarType::VarGUID:
    case PVarType::VarTime:
    case PVarType::VarStaticString:
    case PVarType::VarFixedString:
    case PVarType::VarDynamicString:
      lua_pushstring(m_lua, var.AsString());
      break;

    case PVarType::VarStaticBinary:
    case PVarType::VarDynamicBinary:
    default:
      return false;
  }
  return InternalSetVariable(name);
}


bool PLua::GetBoolean(const PString & name)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  bool result = lua_toboolean(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetBoolean(const PString & name, bool value)
{
  PWaitAndSignal mutex(m_mutex);

  lua_pushboolean(m_lua, value);
  return InternalSetVariable(name);
}


int PLua::GetInteger(const PString & name)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  int result = lua_tointeger(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetInteger(const PString & name, int value)
{
  PWaitAndSignal mutex(m_mutex);

  lua_pushinteger(m_lua, value);
  return InternalSetVariable(name);
}


double PLua::GetNumber(const PString & name)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  double result = lua_tonumber(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetNumber(const PString & name, double value)
{
  PWaitAndSignal mutex(m_mutex);

  lua_pushnumber(m_lua, value);
  return InternalSetVariable(name);
}


PString PLua::GetString(const PString & name)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  PString result = lua_tostring(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetString(const PString & name, const char * value)
{
  PWaitAndSignal mutex(m_mutex);

  lua_pushstring(m_lua, value);
  return InternalSetVariable(name);
}


static char * my_lua_tostring(lua_State * lua, int index)
{
  size_t len;
  const char * str = lua_tolstring(lua, index, &len);
  if (str == NULL)
    return NULL;

  char * buf = new char[len+1];
  strcpy(buf, str);
  return buf;
}

bool PLua::Call(const PString & name, const char * signature, ...)
{
  PWaitAndSignal mutex(m_mutex);

  va_list args;
  va_start(args, signature);

  if (!InternalGetVariable(name))
    return false;

  if (!lua_isfunction(m_lua, -1))
    return OnLuaError(LUA_ERRRUN, PSTRSTRM("No such function as \"" << name << '"'), 1);

  int nargs = 0, nresults = 0;
  const char * resultSignature = NULL;
  if (signature != NULL) {
    while (*signature != '\0') {
      switch (*signature++) {
        case 'B':
        case 'b':
          if (resultSignature != NULL)
            ++nresults;
          else {
            lua_pushboolean(m_lua, va_arg(args, int));
            ++nargs;
          }
          break;

        case 'I':
        case 'i':
          if (resultSignature != NULL)
            ++nresults;
          else {
            lua_pushinteger(m_lua, va_arg(args, int));
            ++nargs;
          }
          break;

        case 'N':
        case 'n':
          if (resultSignature != NULL)
            ++nresults;
          else {
            lua_pushnumber(m_lua, va_arg(args, double));
            ++nargs;
          }
          break;

        case 'S':
        case 's':
          if (resultSignature != NULL)
            ++nresults;
          else {
            lua_pushstring(m_lua, va_arg(args, const char *));
            ++nargs;
          }
          break;

        case 'U':
        case 'u':
          if (resultSignature != NULL)
            ++nresults;
          else {
            lua_pushlightuserdata(m_lua, va_arg(args, void *));
            ++nargs;
          }
          break;

        case '>':
          if (resultSignature == NULL)
            resultSignature = signature;
          break;
      }
    }
  }

  if (!OnLuaError(lua_pcall(m_lua, nargs, nresults, 0)))
    return false;

  if (resultSignature != NULL) {
    int result = -nresults;

    while (*resultSignature) {
      switch (*resultSignature++) {
        case 'B':
        case 'b':
          *va_arg(args, bool *) = lua_toboolean(m_lua, result++);
          break;

        case 'I':
        case 'i':
          *va_arg(args, int *) = lua_tointeger(m_lua, result++);
          break;

        case 'N':
        case 'n':
          *va_arg(args, double *) = lua_tonumber(m_lua, result++);
          break;

        case 'S':
        case 's':
          *va_arg(args, char **) = my_lua_tostring(m_lua, result++);
          break;

        case 'U':
        case 'u':
          *va_arg(args, void **) = lua_touserdata(m_lua, result++);
          break;
      }
    }
    lua_pop(m_lua, nresults);
  }

  va_end(args);
  return true;
}


bool PLua::Call(const PString & name, Signature & signature)
{
  PWaitAndSignal mutex(m_mutex);

  if (!InternalGetVariable(name))
    return false;

  if (!lua_isfunction(m_lua, -1))
    return OnLuaError(LUA_ERRRUN, PSTRSTRM("No such function as \"" << name << '"'), 1);

  signature.m_arguments.Push(m_lua);

  if (!OnLuaError(lua_pcall(m_lua, signature.m_arguments.size(), LUA_MULTRET, 0)))
    return false;

  signature.m_results.Pop(m_lua);

  return true;
}


int PLua::InternalCallback(lua_State * state)
{
  PLua * lua = reinterpret_cast<PLua *>(lua_touserdata(state, lua_upvalueindex(2)));
  return lua != NULL ? lua->InternalCallback() : 0;
}


int PLua::InternalCallback()
{
  PLua::FunctionNotifier * func = reinterpret_cast<PLua::FunctionNotifier *>(lua_touserdata(m_lua, lua_upvalueindex(1)));
  if (func == NULL || func->IsNULL())
    return 0;

  PLua::Signature signature;

  signature.m_arguments.Pop(m_lua);

  (*func)(*this, signature);

  signature.m_results.Push(m_lua);
  PTRACE(6, "InternalCallback stack=" << lua_gettop(m_lua) << ", results::size=" << signature.m_results.size());

  return signature.m_results.size();
}


bool PLua::SetFunction(const PString & name, const FunctionNotifier & func)
{
  PWaitAndSignal mutex(m_mutex);

  if (InternalSetFunction(name, func))
    return true;

  if (!InternalGetVariable(name))
    return false;

  int type = lua_type(m_lua, -1);
  lua_pop(m_lua, 1);

  if (type != LUA_TNIL)
    return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Already using name \"" << name << "\", is " << lua_typename(m_lua, type)));

  m_functions[name] = func;
  lua_pushlightuserdata(m_lua, &m_functions[name]);
  lua_pushlightuserdata(m_lua, this);
  lua_pushcclosure(m_lua, InternalCallback, 2);
  return InternalSetVariable(name);
}


bool PLua::OnLuaError(int code, const PString & str, int pop)
{
  if (code == 0)
    return true;

  if (!str.IsEmpty())
    OnError(code, str);
  else {
    ++pop;
    PString luaError = lua_tostring(m_lua, -1);
    if (luaError.IsEmpty())
      OnError(code, psprintf("Error code %i", code));
    else
      OnError(code, luaError);
  }

  if (pop > 0)
    lua_pop(m_lua, pop);

  return false;
}


static bool ValidIdentifer(const PString & identifier)
{
  return !isdigit(identifier[0]) &&
          identifier.FindSpan("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_") == P_MAX_INDEX;
}


bool PLua::ParseVariableName(const PString & name, PStringArray & elements)
{
  if (name.IsEmpty())
    return OnLuaError(LUA_ERRSYNTAX, "Empty name");

  // First element must be a straight variable
  PINDEX elementPos = name.FindOneOf(".[");
  PString element = name.Left(elementPos);
  if (!ValidIdentifer(element))
    return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Illegal variable name: \"" << name << '"'));

  while (elementPos != P_MAX_INDEX) {
    elements.AppendString(element);

    if (name[elementPos++] == '.') {
      PINDEX lastElementPos = elementPos;
      elementPos = name.FindOneOf(".[", lastElementPos);
      element = name(lastElementPos, elementPos-1);
      if (!ValidIdentifer(element))
        return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Illegal variable name: \"" << name << '"'));
    }
    else {
      element = name.FromLiteral(elementPos);
      if (name[elementPos++] != ']')
        return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Illegal variable name: \"" << name << '"'));

      if (elementPos >= name.GetLength())
        elementPos = P_MAX_INDEX;
      else {
        if (name[elementPos] != '.' && name[elementPos] != '[')
          return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("Illegal variable name: \"" << name << '"'));
      }
    }
  }

  elements.AppendString(element);
  return true;
}


bool PLua::InternalGetVariable(const PString & name)
{
  PStringArray elements;
  if (!ParseVariableName(name, elements))
    return false;

  if (elements.GetSize() < 2) {
    lua_getglobal(m_lua, name);
    return true;
  }

  lua_getglobal(m_lua, elements[0]);
  for (PINDEX var = 1; var < elements.GetSize(); ++var) {
    int type = lua_type(m_lua, -1);
    if (type != LUA_TTABLE)
      return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("No such table as \"" << elements[var-1] << "\", is " << lua_typename(m_lua, type)), 1);

    lua_getfield(m_lua, -1, elements[var]);
    lua_remove(m_lua, -2); // Remove the table from underneath
  }

  PTRACE(6, "InternalGetVariable stack=" << lua_gettop(m_lua));
  return true;
}


bool PLua::InternalSetVariable(const PString & name)
{
  PStringArray elements;
  if (!ParseVariableName(name, elements)) {
    lua_pop(m_lua, 1);
    return false;
  }

  if (elements.GetSize() < 2) {
    lua_setglobal(m_lua, name);
    return true;
  }

  lua_getglobal(m_lua, elements[0]);

  PINDEX var = 1;
  for (;;) {
    int type = lua_type(m_lua, -1);
    if (type != LUA_TTABLE)
      return OnLuaError(LUA_ERRSYNTAX, PSTRSTRM("No such table as \"" << elements[var-1] << "\", is " << lua_typename(m_lua, type)), 2);

    if (var >= elements.GetSize()-1)
      break;

    lua_getfield(m_lua, -1, elements[var++]);
    lua_remove(m_lua, -2); // Remove the table from underneath
  }

  lua_insert(m_lua, -2); // Swap table (top of stack) and next below (value)

  lua_pushstring(m_lua, elements[var]);
  lua_insert(m_lua, -2); // Swap field name (top of stack) and next below (value)

  lua_settable(m_lua, -3);  // Set table key to value

  lua_pop(m_lua, 1); // Pop the table as above doesn't

  PTRACE(6, "InternalSetVariable stack=" << lua_gettop(m_lua));
  return true;
}


void PLua::ParamVector::Push(void * data)
{
  lua_State * lua = static_cast<lua_State *>(data);
  for (iterator it = begin(); it != end(); ++it) {
    switch (it->GetType()) {
      case PVarType::VarNULL :
        lua_pushnil(lua);
        break;

      case PVarType::VarBoolean :
        lua_pushboolean(lua, it->AsBoolean());
        break;

      case PVarType::VarInt8 :
      case PVarType::VarInt16 :
      case PVarType::VarInt32 :
      case PVarType::VarInt64 :
      case PVarType::VarUInt8 :
      case PVarType::VarUInt16 :
      case PVarType::VarUInt32 :
      case PVarType::VarUInt64 :
        lua_pushinteger(lua, it->AsInteger());
        break;

      case PVarType::VarFloatSingle :
      case PVarType::VarFloatDouble :
      case PVarType::VarFloatExtended :
        lua_pushnumber(lua, it->AsFloat());
        break;

      case PVarType::VarStaticString :
      case PVarType::VarDynamicString :
        lua_pushstring(lua, it->AsString());
        break;

      case PVarType::VarStaticBinary :
      case PVarType::VarDynamicBinary :
        lua_pushlightuserdata(lua, (void *)it->GetPointer());
        break;

      default :
        PAssertAlways("Unsupport PVarType for Lua");
    }
  }
}


void PLua::ParamVector::Pop(void * data)
{
  lua_State * lua = static_cast<lua_State *>(data);

  resize(lua_gettop(lua));
  PTRACE(6, PTraceModule(), "ParamVector::Pop stack=" << size());

  for (reverse_iterator it = rbegin(); it != rend(); ++it) {
    switch (lua_type(lua, -1)) {
      case LUA_TBOOLEAN :
        *it = (bool)lua_toboolean(lua, -1);
        break;

      case LUA_TLIGHTUSERDATA :
        *it = PVarType(lua_touserdata(lua, -1), lua_objlen(lua, -1), true);
        break;

      case LUA_TNUMBER :
        *it = lua_tonumber(lua, -1);
        break;

      case LUA_TSTRING :
        *it = my_lua_tostring(lua, -1);
        break;
    }

    lua_pop(lua, 1);
  }
}
#endif // P_LUA
