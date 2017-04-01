/*
 * url_test.cxx
 *
 * Test program for URL loader.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2011 Vox Lucida Pty. Ltd.
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
 * The Initial Developer of the Original Code is Vox Lucida Pty. Ltd.
 *
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/url.h>

// -p 100000 /index.html
// http://someone:something@somewhere.com:54321/first/second#tag;param1=value%201;param2=value%202?query1=arg1&query+2=arg+2&query3=arg+3


class Test : public PProcess
{
  PCLASSINFO(Test, PProcess)
  public:
    void Main();
};


PCREATE_PROCESS(Test)

void Test::Main()
{
  cout << "URL Test Utility" << endl;

  PArgList & args = GetArguments();
  if (!args.Parse("v-verbose. Verbose output\n"
                  "T-time: time PURL parsing for number of iterations\n"
                  PTRACE_ARGLIST)) {
    args.Usage(cerr);
    return;
  }

  PTRACE_INITIALISE(args);

  PURL url;

  unsigned total = args.GetOptionString('T').AsUnsigned();
  if (total == 0) {
    if (!url.Parse(args[0], "http")) {
      cerr << "Could not parse URL \"" << args[0] << '"' << endl;
      return;
    }
  }
  else {
    unsigned count = total;
    PTime start;

    do {
      if (!url.Parse(args[0], "http")) {
        cerr << "Could not parse URL \"" << args[0] << '"' << endl;
        return;
      }
    } while (count-- > 0);

    cout << "UTL Parsing time is " << (1000.0*(PTime() - start).GetMilliSeconds()/total) << "us" << endl;
  }

  if (args.HasOption('v')) {
    static char const sep[] = ": ";
    static char const sepq[] = ": \"";
    static char const qlf[] = "\"\n";
    static int const width = 15;
    cout << "\nParsed URL Components\n"
         << setw(width) << "Scheme" << sepq << url.GetScheme() << qlf
         << setw(width) << "Username" << sepq << url.GetUserName() << qlf
         << setw(width) << "Password" << sepq << url.GetPassword() << qlf
         << setw(width) << "Hostname" << sepq << url.GetHostName() << qlf
         << setw(width) << "Port" << sep << url.GetPort() << " (" << (url.GetPortSupplied() ? "supplied" : "default") << ")\n"
         << setw(width) << "Fragment" << sepq << url.GetFragment() << qlf
         << setw(width) << "Path" << sep << url.GetPath().GetSize() << " elements (" << (url.GetRelativePath() ? "relative" : "absolute") << ")\n";
    for (PINDEX i = 0; i < url.GetPath().GetSize(); ++i)
      cout << setw(width) << i << sepq << url.GetPath()[i] << qlf;
    cout << setw(width) << "Query" << sep << url.GetQueryVars().GetSize() << " elements\n";
    for (PStringOptions::const_iterator it = url.GetQueryVars().begin(); it != url.GetQueryVars().end(); ++it)
      cout << setw(width+3) << '"' << it->first << "\" = \"" << it->second << qlf;
    cout << setw(width) << "Parameters" << sep << url.GetParamVars().GetSize() << " elements\n";
    for (PStringOptions::const_iterator it = url.GetParamVars().begin(); it != url.GetParamVars().end(); ++it)
      cout << setw(width+3) << '"' << it->first << "\" = \"" << it->second << "\"\n";
    cout << setw(width) << "Contents" << sepq << url.GetContents() << qlf
         << "\nReconstructed URL:\n" << url
         << endl;
  }

  if (args.GetCount() == 1) {
    PString str;
    if (url.LoadResource(str))
      cout << str << endl;
    else
      cerr << "\nCould not load text URL \"" << url << '"' << endl;
  }
  else {
    PBYTEArray data;
    if (url.LoadResource(data)) {
      PFile out;
      if (out.Open(args[1], PFile::WriteOnly))
        out.Write((const BYTE *)data, data.GetSize());
      else
        cerr << "\nCould not open file \"" << args[1] << '"' << endl;
    }
    else
      cerr << "\nCould not load binary URL \"" << url << '"' << endl;
  }
}


// End of file
