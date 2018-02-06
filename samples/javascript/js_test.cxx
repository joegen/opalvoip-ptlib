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

/*
Sample command line:  -ttttodebugstream "myString='after';"

Note that three files, icudtl.dat, natives_blob.bin & snapshot_blob.bin, must be
copied to the executable directory of any application that uses the V8 system.
They are usually in the output directory of the build, e.g. out.gn\x64.release
*/

class MyProcess : public PProcess
{
    PCLASSINFO(MyProcess, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(MyProcess)


#if P_V8

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

  PConstString const myString("before");
  jscript.SetString ("myString", myString);
  cout << "myString = \"" << jscript.GetString("myString") << "\" expected \"" << myString << '"' << endl;

  static int const myInt = -1234;
  jscript.SetInteger("myInt", myInt);
  cout << "myInt = " << jscript.GetInteger("myInt") << " expected " << myInt << endl;

  static double const myNumber = -3.141;
  jscript.SetNumber ("myNumber", myNumber);

  cout << "myNumber = " << jscript.GetNumber("myNumber") << " expected " << myNumber << endl;

  jscript.SetBoolean("myBool", true);
  cout << "myBool = " << jscript.GetBoolean("myBool") << " expected true" << endl;

  jscript.SetBoolean("myBool", false);
  cout << "myBool = " << jscript.GetBoolean("myBool") << " expected false" << endl;

  jscript.CreateComposite("Top");
  jscript.CreateComposite("Top.Middle");
  jscript.CreateComposite("Top.Middle.Bottom");
  jscript.SetNumber("Top.Middle.Bottom.Number", myNumber);
  cout << "Top.Middle.Bottom.Number = " << jscript.GetNumber("Top.Middle.Bottom.Number") << " expected " << myNumber  << endl;

  for (PINDEX arg = 0; arg < args.GetCount(); ++arg) {
    if (jscript.Run(args[arg]))
      cout << "Executed '" << args[arg] << "'" << endl;
    else
      cerr << jscript.GetLastErrorText() << " executing '" << args[arg] << "'" << endl;
  }
}

#else
#pragma message("Cannot compile test program without JavaScript support!")

void MyProcess::Main()
{
}
#endif // P_V8
