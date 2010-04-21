//
// hello.cxx
//
// Equivalence Pty. Ltd.
//

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/lua.h>

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
};

#pragma comment(lib, P_LUA_LIBRARY)

#if 0

template <class InstanceType>
class LuaClassBindings
{
  public:
    typedef int (InstanceType::* MemberFunction)(lua_State *);

    LuaClassBindings(const char * name = NULL)
      : m_name(NULL)
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
class LuaClassInstance 
{
  public:
    typedef LuaClassBindings<InstanceType> BindingsType;
    LuaClassInstance(PLuaContext & lua, BindingsType & bindings, InstanceType * instance, const char * name = NULL)
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
      LuaClassInstance * luaClassInstance = (LuaClassInstance *)lua_touserdata(L, lua_upvalueindex(1));
      return (luaClassInstance == NULL) ? 0 : luaClassInstance->m_bindings.Invoke(luaClassInstance->m_instance, lua_tointeger(L, lua_upvalueindex(2)), L);
    }
};

#define PLUA_DECLARE_FUNCTION(fn_name) \
int fn_name(lua_State *) 

#define PLUA_BIND_FUNCTION(bindings, cls, fn_name) \
  bindings.Bind(#fn_name, &cls::fn_name)

class XLuaContext : public PLuaContext
{
  public:
    template<class InstanceType>
    LuaClassInstance<InstanceType> * Instantiate(LuaClassBindings<InstanceType> & bindings, InstanceType * instance, const char * name)
    { return new LuaClassInstance<InstanceType>(*this, bindings, instance, name);  }
};




/////////////////////////////////////////////////////////
#endif

class LuaProcess : public PProcess
{
  PCLASSINFO(LuaProcess, PProcess)
  public:
    void Main();
    PLUA_DECLARE_FUNCTION(SetTraceLevel)
    {
      cerr << "SetTraceLevel called" << endl;
      return 0;
    }
};

class MyClass {
  public:
    MyClass(const char * str)
      : m_str(str)
    { }
    const char * m_str;

    PLUA_DECLARE_FUNCTION(print)
    {
      cerr << m_str << endl;
      return 0;
    }
};


PCREATE_PROCESS(LuaProcess)

void LuaProcess::Main()
{
  PLua::ClassBindings<MyClass> myClassBinding("MyClass");
  PLUA_BIND_FUNCTION(myClassBinding, MyClass, print);

  MyClass class1("class1");
  MyClass class2("class2");

  PLua lua;
  lua.Instantiate<MyClass>(myClassBinding, &class1, "class1");
  lua.Instantiate<MyClass>(myClassBinding, &class2, "class2");

  lua.Run("class1.print()\nclass2.print()");
}

// End of hello.cxx
