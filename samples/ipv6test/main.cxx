/*
 * main.cxx
 *
 * PWLib application source file for dtmftest
 *
 * Main program entry point.
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: main.cxx,v $
 * Revision 1.1  2004/12/14 06:50:59  csoutheren
 * Initial version
 *
 *
 */

#include "precompile.h"
#include "main.h"
#include "version.h"

#include <ptlib/sockets.h>


PCREATE_PROCESS(IPV6Test);


IPV6Test::IPV6Test()
  : PProcess("Post Increment", "dtmftest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void IPV6Test::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
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

#if ! P_HAS_IPV6
  cout << "error: IPV6 not included in PWLib" << endl;
#else
  {
      // test #1 - check PIPSocket::IsIpAddressFamilyV6Supported
      cout << "test #1: PIPSocket::IsIpAddressFamilyV6Supported ";
      if (PIPSocket::IsIpAddressFamilyV6Supported())
          cout << "OK";
      else
          cout << "failed";
      cout << endl;
  }
#endif
}

// End of File ///////////////////////////////////////////////////////////////
