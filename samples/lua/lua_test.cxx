/*
 * lua_test.h
 *
 * Test for PTLib integration of Lua interpreter
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
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/lua.h>

#if P_LUA

class MyProcess : public PProcess
{
    PCLASSINFO(MyProcess, PProcess)
  public:
    void Main();
    PDECLARE_LuaFunctionNotifier(MyProcess, LuaTestFunction);
};

PCREATE_PROCESS(MyProcess)


#define LUA_TO_C_FUNCTION "lua_to_c_test"
#define C_TO_LUA_FUNCTION "c_to_lua_test"
#define LUA_VARIABLE      "lua_variable"
#define LUA_VAR_INIT_VALUE 12
#define LUA_VAR_NEW_VALUE  "27"
const PConstString StringIndex("Some \"string\" index");

const char TestScript[] =
#if PTRACING
  "PTRACE(2, 'Starting test script: ', ' setting " LUA_VARIABLE " to ', " LUA_VAR_NEW_VALUE ")\n"
#endif
  LUA_TO_C_FUNCTION "('main')\n"
  "class1." LUA_TO_C_FUNCTION "('first', 1)\n"
  "class2." LUA_TO_C_FUNCTION "('second', 2)\n"
  LUA_VARIABLE "=" LUA_VAR_NEW_VALUE "\n"
  "function " C_TO_LUA_FUNCTION "(a, b)\n"
  "  return 'a+b=' .. (a + b)\n"
  "end\n";


class MyClass : public PObject {
  public:
    MyClass(PLua & lua, const char * str)
      : m_instance(str)
    {
      if (!lua.CreateTable(m_instance) ||
          !lua.SetFunction(m_instance + "." LUA_TO_C_FUNCTION, PCREATE_NOTIFIER(LuaTestFunction)))
        cerr << lua.GetLastErrorText() << endl;
    }

    PDECLARE_LuaFunctionNotifier(MyClass, LuaTestFunction);

  protected:
    const PString m_instance;
};


void MyProcess::Main()
{
  cout << "Lua Test Utility" << endl;

  PArgList & args = GetArguments();
  args.Parse("T-test:"
#if PTRACING
             "o-output:"
             "t-trace."
#endif
    );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  PLua lua;
  if (!lua.SetFunction(LUA_TO_C_FUNCTION, PCREATE_NOTIFIER(LuaTestFunction))) {
    cerr << lua.GetLastErrorText() << endl;
    return;
  }

  // populate the table
  MyClass class1(lua, "class1");
  MyClass class2(lua, "class2");

  lua.SetInteger(LUA_VARIABLE, LUA_VAR_INIT_VALUE);

  lua.CreateTable("TopTable");
  lua.CreateTable("TopTable.MiddleTable");
  lua.CreateTable("TopTable.MiddleTable.BottomTable");
  lua.SetNumber("TopTable.MiddleTable.BottomTable.Number", 1234.56789);
  cout << "TopTable.MiddleTable.BottomTable.Number="
       << setprecision(10) << lua.GetNumber("TopTable.MiddleTable.BottomTable.Number") << endl;

  lua.DeleteTable("TopTable.MiddleTable");
  if (lua.GetNumber("TopTable.MiddleTable.BottomTable.Number") != 0)
    cerr << "Failed to delete table TopTable.MiddleTable" << endl;
  else
    cout << "Deleteed table TopTable.MiddleTable\n";

  PString strTableName = "TopTable[" + StringIndex.ToLiteral() + ']';
  if (!lua.CreateTable(strTableName))
    cerr << lua.GetLastErrorText() << " creating string indexed table" << endl;

  if (!lua.SetString(strTableName + ".a_string", "A value"))
    cerr << lua.GetLastErrorText() << " setting string field of indexed table" << endl;

  if (lua.GetString(strTableName + ".a_string") != "A value")
    cerr << "Did not actually set string field of indexed table" << endl;
  else
    cout << "Set string field: " << strTableName << ".a_string" << endl;

  if (args.GetCount() == 0) {
    if (lua.Run(TestScript)) {
      cout << "Variable " LUA_VARIABLE " changed from " << LUA_VAR_INIT_VALUE
           << " to " << lua.GetInteger(LUA_VARIABLE) << " (expected " LUA_VAR_NEW_VALUE ")" << endl;
      char * str = NULL;
      if (lua.Call(C_TO_LUA_FUNCTION, "in>s", 3, 0.14159, &str))
        cout << "Function returned \"" << str << '"' << endl;
      else
        cerr << lua.GetLastErrorText() << " executing " C_TO_LUA_FUNCTION << endl;
      delete[] str;
    }
    else
      cerr << lua.GetLastErrorText() << " executing script" << endl;
  }
  else {
    for (PINDEX arg = 0; arg < args.GetCount(); ++arg) {
      if (lua.Run(args[arg]))
        cout << "Executed " << args[arg] << endl;
      else
        cerr << lua.GetLastErrorText() << " executing " << args[arg] << endl;
    }
  }
}


static void TestOutput(const PLua::Signature & sig)
{
  cout << " nargs=" << sig.m_arguments.size() << ", ";

  for (size_t i = 0; i < sig.m_arguments.size(); ++i) {
    if (i != 0)
      cout << ", ";
    cout << i << '=' << sig.m_arguments[i];
  }

  cout << endl; 
}


void MyProcess::LuaTestFunction(PLua&, PLua::Signature & sig)
{
  cout << "Global:";
  TestOutput(sig);
}


void MyClass::LuaTestFunction(PLua&, PLua::Signature & sig)
{
  cout << "Member: instance=" << m_instance << ',';
  TestOutput(sig); 
}


#else
#error Cannot compile Lua test program without Lua support!
#endif // P_LUA
