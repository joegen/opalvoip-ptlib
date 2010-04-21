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

class LuaProcess : public PProcess
{
  PCLASSINFO(LuaProcess, PProcess)
  public:
    void Main();
};

class MyClass {
  public:
    MyClass(const char * str)
      : m_str(str)
    { }
    const char * m_str;

    int print(lua_State *)
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
