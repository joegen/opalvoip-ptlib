/*
 * main.cxx
 *
 * PWLib application header file for stunserver
 *
 * Copyright (c) 2010 Post Increment
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
 */

#include <ptlib.h>
#include "main.h"
#include "version.h"

#include <ptclib/pstunsrvr.h>

PCREATE_PROCESS(StunServer);

StunServer::StunServer()
  : PProcess("Post Increment", "stunserver", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void StunServer::Main()
{
  PArgList & args = GetArguments();
  args.Parse(
            "-turnserver:"
#if PTRACING
            "t-trace."       "-no-trace."
            "o-output:"      "-no-output."
#endif
    );

#if PTRACING

  PTrace::Initialise(args.GetOptionCount('t'),
                   args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
                   PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  WORD port = PSTUNServer::DefaultPort;

  if (args.GetCount() > 0)
    port = (WORD)args[1].AsUnsigned();

  PSTUNServer server;
  if (!server.Open(port)) {
    PError << "error: cannot create STUN server on port " << (int)port << endl;
    return;
  }

  PIPSocketAddressAndPort turnServer(args.GetOptionString("turnserver"));

  while (server.IsOpen()) {
    PSTUNServer::SocketInfo info;
    PSTUNMessage message;
    if (!server.Read(message, info)) 
      cerr << "error: read failed" << endl;
    else if (message.GetSize() > 0) {
      if (!message.Validate())
        cerr << "error: invalid message received" << endl;
      else if (message.GetType() != PSTUNMessage::Allocate)
        server.Process(message, info);
      else if (turnServer.IsValid()) {
        cerr << "TURN allocate response received from " << info.m_socketAddress << " - redirecting to " << turnServer << endl;
        PSTUNMessage response;
        response.SetType((PSTUNMessage::MsgType)(message.GetType() | 0x110), message.GetTransactionID());
        {
          PSTUNErrorCode attr;
          attr.Initialise();
          attr.SetErrorCode(300, "TURN available on alternate server");
          response.AddAttribute(attr);
        }
        {
          PSTUNAddressAttribute attr;
          attr.InitAddrAttr(PSTUNAttribute::ALTERNATE_SERVER);
          attr.SetIPAndPort(turnServer);
          response.AddAttribute(attr);
          server.WriteTo(response, *info.m_socket, message.GetSourceAddressAndPort());
        }
      }
      else
        cerr << "Unknown command " << hex << message.GetType() << dec << " from " << message.GetSourceAddressAndPort() << endl;
    }
  }
}

// End of File ///////////////////////////////////////////////////////////////
