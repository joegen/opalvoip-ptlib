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
 * $Log: main.cxx,v $
 * Revision 1.3  2003/02/04 07:02:55  robertj
 * Removed ports in test, added delete of created udp socket.
 *
 * Revision 1.2  2003/02/04 05:23:59  craigs
 * Added new functions
 *
 * Revision 1.1  2003/02/04 03:31:04  robertj
 * Added STUN
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

  PSTUNClient stun(args[0]);
  cout << stun.GetNatTypeName() << endl;

  PUDPSocket * udp;
  if (!stun.CreateSocket(udp)) {
    cout << "Cannot create a socket" << endl;
    return;
  }

  PIPSocket::Address addr;
  WORD port;
  udp->GetLocalAddress(addr, port);
  cout << "local address is " << addr << ":" << port << endl;

  delete udp;
}


// End of File ///////////////////////////////////////////////////////////////
