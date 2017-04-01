/*
 * main.cxx
 *
 * PWLib application source file for stunclient
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
 */

#include <ptlib.h>
#include "main.h"
#include "version.h"

#include <ptclib/pstun.h>


PCREATE_PROCESS(StunClient);



StunClient::StunClient()
  : PProcess("Equivalence", "stunclient", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void StunClient::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("-turn. Use TURN instead of STUN\n"
                  "-username: TURN Authorisation username\n"
                  "-password: TURN Authorisation password\n"
                  "-realm: TURN Authorisation realm\n"
                  "-interface: Local network interface to use\n"
                  "-portbase: Port base for sockets\n"
                  "-portmax: Port maximum for sockets\n"
                  "-pairbase: Port base for socket pairs\n"
                  "-pairmax: Port maximum for socket pairs\n"
                  PTRACE_ARGLIST
                  "V-version. Output version\n")) {
    args.Usage(cerr, "[ <options> ... ] <stun-server>");
    return;
  }

  PTRACE_INITIALISE(args);

  if (args.HasOption('V')) {
    cout << GetName() << " version " << GetVersion() << endl;
    return;
  }

  PString methodName = args.HasOption("turn") ? "TURN" : "STUN";
  PNatMethod * nat = PNatMethod::Create(methodName);
  if (nat == NULL) {
    cerr << "Could not create NAT method for " << methodName << endl;
    return;
  }

  nat->SetCredentials(args.GetOptionString("username", "toto"),
                      args.GetOptionString("password", "password"),
                      args.GetOptionString("realm", "opalvoip.org"));
  nat->SetPortRanges((WORD)args.GetOptionString("portbase").AsUnsigned(),
                     (WORD)args.GetOptionString("portmax").AsUnsigned(),
                     (WORD)args.GetOptionString("pairbase").AsUnsigned(),
                     (WORD)args.GetOptionString("pairmax").AsUnsigned());

  if (!nat->SetServer(args[0])) {
    cerr << "Cannot set server \"" << args[0] << "\" for " << methodName << endl;
    return;
  }

  if (!nat->Open(PIPAddress(args.GetOptionString("interface", "0.0.0.0")))) {
    cerr << "Cannot open server \"" << args[0] << "\" for " << methodName << endl;
    return;
  }

  cout << "NAT type: " << nat->GetNatTypeName() << endl;

  PIPSocket::Address router;
  if (!nat->GetExternalAddress(router)) {
    cout << "Could not get router address!" << endl;
    return;
  }
  cout << "Router address: " << router << endl;

  PUDPSocket * udp;
  if (!nat->CreateSocket(udp)) {
    cout << "Cannot create a socket!" << endl;
    return;
  }

  PIPSocket::Address addr;
  WORD port;
  udp->GetLocalAddress(addr, port);
  cout << "Socket local address reported as " << addr << ":" << port << endl;

  delete udp;

  PUDPSocket * udp1, * udp2;
  if (!nat->CreateSocketPair(udp1, udp2)) {
    cout << "Cannot create socket pair" << endl;
    return;
  }

  udp1->GetLocalAddress(addr, port);
  cout << "Socket 1 local address reported as " << addr << ":" << port << endl;
  udp2->GetLocalAddress(addr, port);
  cout << "Socket 2 local address reported as " << addr << ":" << port << endl;

  delete udp1;
  delete udp2;
}


// End of File ///////////////////////////////////////////////////////////////
