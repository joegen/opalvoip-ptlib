/*
 * main.cxx
 *
 * PWLib application source file for threadex
 *
 * Main program entry point.
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: main.cxx,v $
 * Revision 1.1  2004/09/13 01:13:26  dereksmithies
 * Initial release of VERY simple program to test PThread::WaitForTermination
 *
 * Revision 1.3  2004/09/10 22:33:31  dereksmithies
 * Calculate time required to do the decoding of the dtmf symbol.
 *
 * Revision 1.2  2004/09/10 04:31:57  dereksmithies
 * Add code to calculate the detection rate.
 *
 * Revision 1.1  2004/09/10 01:59:35  dereksmithies
 * Initial release of program to test Dtmf creation and detection.
 *
 *
 */

#include "precompile.h"
#include "main.h"
#include "version.h"


PCREATE_PROCESS(Threadex);

#include  <ptclib/dtmf.h>
#include  <ptclib/random.h>



Threadex::Threadex()
  : PProcess("Equivalence", "threadex", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void Threadex::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
             "h-help."               "-no-help."
#if PTRACING
             "o-output:"             "-no-output."
             "t-trace."              "-no-trace."
#endif
	     "v-version."
	     );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  if (args.HasOption('v')) {
    cout << "Product Name: " << GetName() << endl
	 << "Manufacturer: " << GetManufacturer() << endl
	 << "Version     : " << GetVersion(TRUE) << endl
	 << "System      : " << GetOSName() << '-'
	 << GetOSHardware() << ' '
	 << GetOSVersion() << endl;
    return;
  }

  if (args.HasOption('h')) {
    PError << "Available options are: " << endl         
	   << "-h  or --help  :print this help" << endl
	   << "-v  or --version print version info" << endl
#if PTRACING
	   << "o-output   output file name for trace" << endl
	   << "t-trace.    trace level to use." << endl
#endif
	   << endl
	   << endl << endl;
    return;
  }
 
  ExampleThread messager("example Thread");

  messager.WaitForTermination();
}


void ExampleThread::Main()
{
  cout << "Start of thread.    User message is \"" << userName << "\"" <<  endl;

  PThread::Sleep(20 * 1000);   //20 second sleep

  cout << "End of 20 second sleep" << endl;

  cout << "End of thread.    User message is \"" << userName << "\"" <<  endl;
}

// End of File ///////////////////////////////////////////////////////////////
