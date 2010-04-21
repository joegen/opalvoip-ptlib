/*
 * lua.h
 *
 * Interface library for Lua interpreter
 *
 * Portable Tools Library]
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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_LUA_H
#define PTLIB_LUA_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptbuildopts.h>

#if P_LUA

#include <lua.hpp>

//////////////////////////////////////////////////////////////

class PLua
{
  public:
    PLua();
    ~PLua();

    virtual bool LoadString(const char * text);

    virtual bool LoadFile(const char * filename);

    virtual bool Run(const char * program = NULL);

    virtual void OnError(int code, const PString & str);

    operator lua_State * () { return m_lua; }

    virtual void SetValue(const char * name, const char * value);

    virtual void SetFunction(const char * name, lua_CFunction func);

    bool CallLuaFunction(const char * name);
    bool CallLuaFunction(const char * name, const char * sig, ...);

    static int TraceFunction(lua_State * L);

    template <class InstanceType>
    class ClassBindings
    {
      public:
        typedef int (InstanceType::* MemberFunction)(lua_State *);

        ClassBindings(const char * name = NULL)
          : m_name(name)
        { }

        bool Bind(const char * name, MemberFunction func)
        {
          MemberBinding binding;
          binding.m_name = name;
          binding.m_func = func;
          m_memberBindings.push_back(binding);
          return true;
        }

        virtual int Invoke(InstanceType * instance, int function, lua_State * L)
        {
          return m_memberBindings[function].Invoke(instance, L); 
        }

        class Creator 
        {
          public:
            Creator(const char * name)
            {
              Creator::SetName(name);
            }

            static const char * SetName(const char * name_ = NULL)
            {
              static const char * name = name_;
              return name;
            }

            static ClassBindings * GetBindings()
            { 
              static PMutex mutex;
              PWaitAndSignal m(mutex);
              static ClassBindings m_bindings(SetName());
              static bool init = false;
              if (!init) {
                OnCreate(m_bindings); 
                init = true;
              }
              return &m_bindings;
            }

            static void OnCreate(ClassBindings & bindings);
        };

        struct MemberBinding {
          const char *   m_name;
          MemberFunction m_func;

          virtual int Invoke(InstanceType * instance, lua_State * L)
          {
            return (instance->*m_func)(L); 
          }
        };

        const char * m_name;
        std::vector<MemberBinding> m_memberBindings;
    };

    template <class InstanceType>
    class ClassInstance 
    {
      public:
        typedef ClassBindings<InstanceType> BindingsType;
        typedef typename BindingsType::Creator BindingsCreatorType;

        ClassInstance(PLua & lua, BindingsType & bindings, InstanceType * instance, const char * name = NULL)
          : m_instance(instance)
          , m_bindings(bindings)
          , m_name(name)
        {
          // using binding name if no instance name specified
          if (m_name == NULL)
            m_name = bindings.m_name;

          // populate the table
          lua_newtable(lua);
          for (size_t i = 0; i < m_bindings.m_memberBindings.size(); i++) {
            lua_pushlightuserdata(lua, (void *)this);
            lua_pushinteger      (lua, i);
            lua_pushcclosure     (lua, &CallBack, 2);
            lua_setfield(lua, -2, m_bindings.m_memberBindings[i].m_name);
          }

          // register the table
          lua_setglobal(lua, m_name);
        }

        BindingsType & m_bindings;
        InstanceType * m_instance;
        const char * m_name;

        static int CallBack(lua_State * L)
        {
          ClassInstance * luaClassInstance = (ClassInstance *)lua_touserdata(L, lua_upvalueindex(1));
          return (luaClassInstance == NULL) ? 0 : luaClassInstance->m_bindings.Invoke(luaClassInstance->m_instance, lua_tointeger(L, lua_upvalueindex(2)), L);
        }
    };

    template<class InstanceType>
    ClassInstance<InstanceType> * Instantiate(ClassBindings<InstanceType> & bindings, InstanceType * instance, const char * name)
    { return new ClassInstance<InstanceType>(*this, bindings, instance, name);  }

    template<class InstanceType>
    ClassInstance<InstanceType> * Instantiate(InstanceType * instance, const char * name)
    { return new ClassInstance<InstanceType>(*this, *ClassBindings<InstanceType>::Creator::GetBindings(), instance, name);  }

  protected:
    lua_State * m_lua;
};

#define PLUA_DECLARE_FUNCTION(fn_name); \
  int fn_name(lua_State *);

#define PLUA_BIND_FUNCTION(bindings, cls, fn_name) \
  (bindings).Bind(#fn_name, &cls::fn_name);


//////////////////////////////////////////////////////////////

#endif // P_LUA

#endif  // PTLIB_LUA_H

