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
 * Revision 1.3  2004/12/14 14:24:20  csoutheren
 * Added PIPSocket::Address::operator*= to compare IPV4 addresses
 * to IPV4-compatible IPV6 addresses. More documentation needed
 * once this is tested as working
 *
 * Revision 1.2  2004/12/14 07:49:49  csoutheren
 * added some tests
 *
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


  // this is an IPV6 compatibility address
  const BYTE ipv6Data[] = { 0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0x00, 0x00,
                            0x00, 0x00, 0xff, 0xff,
                            220,  244,  81,   10 };
  const PIPSocket::Address ipv6Addr1(sizeof(ipv6Data), ipv6Data);
  {
    // test #2 - check V6 constructor
    cout << "test #2: PIPSocket::Address(int, const BYTE *) ";
    if (ipv6Addr1.GetVersion() == 6) 
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }

  PIPSocket::Address ipv6Addr2("::ffff:220.244.81.10");
  {
    // test #3 - check V6 constructor
    cout << "test #3: PIPSocket::Address(const PString & str) ";
    if (ipv6Addr1.GetVersion() == 6) 
        cout << "OK";
    else
        cout << "failed - " << ipv6Addr1 << " <-> " << ipv6Addr2;
    cout << endl;
  }
  
  {
    // test #4 - check comparison for equality
    cout << "test #4: PIPSocket::operator == ";
    if (ipv6Addr1 == ipv6Addr2)
        cout << "OK";
    else
        cout << "failed - " << ipv6Addr1 << " <-> " << ipv6Addr2;
    cout << endl;
  }
  
  PIPSocket::Address ipv6Addr3("::ffff:220.244.81.09");
  {
    // test #5 - check comparison for non-equality
    cout << "test #5: PIPSocket::operator != ";
    if (ipv6Addr1 != ipv6Addr3)
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }

  PIPSocket::Address ipv4Addr("220.244.81.10");
  {
    // test #6 - check IPV6 comparison to IPV4
    cout << "test #6: PIPSocket::operator == with IPV4 (should fail) ";
    if (ipv6Addr1 == ipv4Addr)
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }
  {
    // test #6a - check IPV6 comparison to IPV4
    cout << "test #6a: PIPSocket::operator == with IPV4 (should fail) ";
    if (ipv6Addr3 == ipv4Addr)
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }

  {
    // test #7 - check IPV6 comparison to IPV4
    cout << "test #7: PIPSocket::operator *= with IPV4 ";
    if (ipv6Addr1 *= ipv4Addr)
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }
  {
    // test #7a - check IPV6 comparison to IPV4
    cout << "test #7a: PIPSocket::operator *= with IPV4 (should fail) ";
    if (ipv6Addr3 *= ipv4Addr)
        cout << "OK";
    else
        cout << "failed";
    cout << endl;
  }
#endif
}

// End of File ///////////////////////////////////////////////////////////////
