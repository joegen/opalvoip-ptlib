/*
 * Test for PTLib integration of JavaScript
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
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/jscript.h>

// Sample command line:  -ttttodebugstream "myString='after';"

#if P_V8

class MyProcess : public PProcess
{
    PCLASSINFO(MyProcess, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(MyProcess)


class MyClass : public PObject {
  public:
    MyClass(PJavaScript & /*jscript*/, const char * str)
      : m_instance(str)
    {
    }

  protected:
    const PString m_instance;
};


void MyProcess::Main()
{
  cout << "JavaScript Test Utility" << endl;

  PArgList & args = GetArguments();
  args.Parse("h-help Display usage\n" PTRACE_ARGLIST);
  if (!args.IsParsed() || args.HasOption('h')) {
    args.Usage(cerr);
    return;
  }

  PTRACE_INITIALISE(args);

  PJavaScript jscript;

  jscript.SetString ("myString", "before");
  jscript.SetInteger("myInt",    -1234);
  jscript.SetNumber ("myNumber", -3.14);
  jscript.SetBoolean("myBool", true);

  for (PINDEX arg = 0; arg < args.GetCount(); ++arg) {
    if (jscript.Run(args[arg]))
      cout << "Executed '" << args[arg] << "'" << endl;
    else
      cerr << jscript.GetLastErrorText() << " executing '" << args[arg] << "'" << endl;
  }

  PVarType var;
  jscript.GetVar("myArray[0].var", var);

  cout << "myString             = '" << jscript.GetString("myString") << "'" << endl
       << "myInt                = "  << jscript.GetInteger("myInt") << endl
       << "myNumber             = "  << jscript.GetNumber("myNumber") << endl
       << "myBool               = "  << jscript.GetBoolean("myBool") << endl
       << "myObject.pi          = "  << jscript.GetNumber("myObject.pi") << endl
       << "myObject.subObject.e = "  << jscript.GetNumber("myObject.subObject.e") << endl
       << "myArray[0].number    = "  << jscript.GetNumber("myArray[0].number") << endl
       << "myArray[0].string    = "  << jscript.GetString("myArray[0].string") << endl
       << "myArray[0].var       = "  << var << endl
       << endl;
       ;
}

#else
#error Cannot compile JavaScript test program without JavaScript support!
#endif // P_V8
