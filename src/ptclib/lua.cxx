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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "lua.h"
#endif

#include <ptlib.h>

#include <ptbuildopts.h>

#if P_LUA

#include <ptclib/lua.h>
#include <lua.hpp>


#ifdef _MSC_VER
  #pragma comment(lib, P_LUA_LIBRARY)
  #pragma message("Lua scripting support enabled")
#endif


#if PTRACING
static int TraceFunction(lua_State * L)
{
  if (lua_isnumber(L, -2) && lua_isstring(L, -1))
    PTRACE(lua_tointeger(L, -2), "LuaScript\t" << lua_tostring(L, -1));
  return 0;
}
#endif

///////////////////////////////////////////////////////////////////////////////

PLua::PLua()
  : m_lua(luaL_newstate())
  , m_loaded(false)
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


bool PLua::LoadFile(const PFilePath & filename)
{
  int err = luaL_loadfile(m_lua, filename);
  m_loaded = err == 0;
  if (m_loaded)
    return true;

  if (err != LUA_ERRFILE)
    return OnError(err);

  return OnError(err, "Cannot load/open file " + filename, 1);
}


bool PLua::LoadText(const PString & text)
{
  m_loaded = OnError(luaL_loadstring(m_lua, text));
  return m_loaded;
}


bool PLua::Run(const char * script)
{
  if (script != NULL) {
    PFilePath filename = script;
    if (PFile::Exists(filename)) {
      if (!LoadFile(filename))
        return false;
    }
    else {
      if (!LoadText(script))
        return false;
    }
  }

  if (IsLoaded())
    return OnError(lua_pcall(m_lua, 0, 0, 0));

  return OnError(LUA_ERRRUN, "Script not loaded");
}


bool PLua::CreateTable(const PString & name, bool withMeta)
{
  if (name.Find('.') != P_MAX_INDEX)
    return OnError(LUA_ERRSYNTAX, "Illegal table name " + name);

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
      return OnError(LUA_ERRSYNTAX, "Already using name " + name + ", type " + lua_typename(m_lua, type));
  }

  lua_newtable(m_lua);

  if (withMeta) {
    luaL_newmetatable(m_lua, name);
    lua_setmetatable(m_lua, -2);
  }

  lua_setglobal(m_lua, name);

  return true;
}


bool PLua::GetBoolean(const PString & name)
{
  if (!InternalGetVariable(name))
    return false;

  bool result = lua_toboolean(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetBoolean(const PString & name, bool value)
{
  lua_pushboolean(m_lua, value);
  return InternalSetVariable(name);
}


int PLua::GetInteger(const PString & name)
{
  if (!InternalGetVariable(name))
    return false;

  int result = lua_tointeger(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetInteger(const PString & name, int value)
{
  lua_pushinteger(m_lua, value);
  return InternalSetVariable(name);
}


double PLua::GetNumber(const PString & name)
{
  if (!InternalGetVariable(name))
    return false;

  double result = lua_tonumber(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetNumber(const PString & name, double value)
{
  lua_pushnumber(m_lua, value);
  return InternalSetVariable(name);
}


PString PLua::GetString(const PString & name)
{
  if (!InternalGetVariable(name))
    return false;

  PString result = lua_tostring(m_lua, -1);
  lua_pop(m_lua, 1);
  return result;
}


bool PLua::SetString(const PString & name, const char * value)
{
  lua_pushstring(m_lua, value);
  return InternalSetVariable(name);
}


static char * my_lua_tostring(lua_State * lua, int index)
{
  size_t len;
  const char * str = lua_tolstring(lua, index, &len);
  char * buf = new char[len+1];
  strcpy(buf, str);
  return buf;
}

bool PLua::Call(const PString & name, const char * signature, ...)
{
  va_list args;
  va_start(args, signature);

  if (!InternalGetVariable(name))
    return false;

  if (!lua_isfunction(m_lua, -1))
    return OnError(LUA_ERRRUN, "No such function as " + name, 1);

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
            lua_pushboolean(m_lua, va_arg(args, bool));
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

  if (!OnError(lua_pcall(m_lua, nargs, nresults, 0)))
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
  if (!InternalGetVariable(name))
    return false;

  if (!lua_isfunction(m_lua, -1))
    return OnError(LUA_ERRRUN, "No such function as " + name, 1);

  signature.m_arguments.Push(m_lua);

  if (!OnError(lua_pcall(m_lua, signature.m_arguments.size(), signature.m_results.size(), 0)))
    return false;

  signature.m_results.Pop(m_lua);

  return true;
}


int PLua::InternalCallback(lua_State * state)
{
  PLua * lua = reinterpret_cast<PLua *>(lua_touserdata(state, lua_upvalueindex(2)));
  return lua != NULL ? lua->InternalCallback() : NULL;
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

  return signature.m_results.size();
}


bool PLua::SetFunction(const PString & name, const FunctionNotifier & func)
{
  map<PString, FunctionNotifier>::iterator it = m_functions.find(name);
  if (it == m_functions.end()) {
    if (func.IsNULL())
      return true;

    if (!InternalGetVariable(name))
      return false;

    int type = lua_type(m_lua, -1);
    lua_pop(m_lua, 1);

    if (type != LUA_TNIL)
      return OnError(LUA_ERRSYNTAX, "Already using name " + name + ", type " + lua_typename(m_lua, type));
  }
  else {
    if (it->second == func)
      return true;
  }

  m_functions[name] = func;
  lua_pushlightuserdata(m_lua, &m_functions[name]);
  lua_pushlightuserdata(m_lua, this);
  lua_pushcclosure(m_lua, InternalCallback, 2);
  return InternalSetVariable(name);
}


bool PLua::OnError(int code, const PString & str, unsigned pop)
{
  if (code == 0)
    return true;

  m_lastErrorText = str;
  if (str.IsEmpty()) {
    m_lastErrorText = lua_tostring(m_lua, -1);
    ++pop;

    if (m_lastErrorText.IsEmpty())
      m_lastErrorText.sprintf("Error code %i", code);
  }

  if (pop > 0)
    lua_pop(m_lua, pop);

  PTRACE(2, "Lua\tError " << code << ": " << m_lastErrorText);
  return false;
}


bool PLua::InternalGetVariable(const PString & name)
{
  if (name.IsEmpty())
    return OnError(LUA_ERRSYNTAX, "Empty name");

  PString table, field;
  if (!name.Split('.', table, field))
    lua_getglobal(m_lua, name);
  else {
    lua_getglobal(m_lua, table);
    if (!lua_istable(m_lua, -1))
      return OnError(LUA_ERRSYNTAX, "No such table as " + table, 1);

    lua_getfield(m_lua, -1, field);
  }

  return true;
}


bool PLua::InternalSetVariable(const PString & name)
{
  if (name.IsEmpty())
    return OnError(LUA_ERRSYNTAX, "Empty name", 1);

  PString table, field;
  if (!name.Split('.', table, field))
    lua_setglobal(m_lua, name);
  else {
    lua_getglobal(m_lua, table);
    if (!lua_istable(m_lua, -3))
      return OnError(LUA_ERRSYNTAX, "No such table as " + table, 2);

    lua_insert(m_lua, -2); // Swap table (top of stack) and next below (value)

    lua_pushstring(m_lua, field);
    lua_insert(m_lua, -2); // Swap field name (top of stack) and next below (value)

    lua_settable(m_lua, -3);  // Set table key to value

    lua_pop(m_lua, 1); // Pop the table as above doesn't
  }

  return true;
}


void PLua::ParamVector::Push(lua_State * lua)
{
  for (iterator it = begin(); it != end(); ++it) {
    switch (it->m_type) {
      case PLua::ParamNIL :
        lua_pushnil(lua);
        break;

      case PLua::ParamBoolean :
        lua_pushboolean(lua, it->m_boolean);
        break;

      case PLua::ParamInteger :
        lua_pushinteger(lua, it->m_integer);
        break;

      case PLua::ParamNumber :
        lua_pushnumber(lua, it->m_number);
        break;

      case PLua::ParamStaticString :
        lua_pushstring(lua, it->m_staticString);
        break;

      case PLua::ParamDynamicString :
        lua_pushstring(lua, it->m_dynamicString);
    }
  }
}


void PLua::ParamVector::Pop(lua_State * lua)
{
  resize(lua_gettop(lua));

  for (reverse_iterator it = rbegin(); it != rend(); ++it) {
    switch (lua_type(lua, -1)) {
      case LUA_TBOOLEAN :
        it->m_type = ParamBoolean;
        it->m_boolean = lua_toboolean(lua, -1);
        break;

      case LUA_TLIGHTUSERDATA :
        it->m_type = ParamUserData;
        it->m_userData = lua_touserdata(lua, -1);
        break;

      case LUA_TNUMBER :
        it->m_type = ParamNumber;
        it->m_number = lua_tonumber(lua, -1);
        break;

      case LUA_TSTRING :
        it->m_type = ParamDynamicString;
        it->m_dynamicString = my_lua_tostring(lua, -1);
        break;
    }

    lua_pop(lua, 1);
  }
}


ostream& operator<<(ostream& strm, const PLua::Parameter& param)
{
  switch (param.m_type) {
    case PLua::ParamNIL :
      strm << "(nil)";
      break;

    case PLua::ParamBoolean :
      strm << param.m_boolean;
      break;

    case PLua::ParamInteger :
      strm << param.m_integer;
      break;

    case PLua::ParamNumber :
      strm << param.m_number;
      break;

    case PLua::ParamStaticString :
    case PLua::ParamDynamicString :
      strm << param.m_staticString;
      break;

    case PLua::ParamUserData :
      strm << param.m_userData;
  }
  return strm;
}


#else // P_LUA

  #ifdef _MSC_VER
    #pragma message("Lua scripting support DISABLED")
  #endif

#endif // P_LUA
