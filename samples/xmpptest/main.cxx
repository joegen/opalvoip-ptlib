/*
 * main.cxx
 *
 * PWLib application source file for XMPPTest
 *
 * Main program entry point.
 *
 * Copyright 2004 Reitek S.p.A.
 *
 * $Log: main.cxx,v $
 * Revision 1.1  2004/04/26 01:51:58  rjongbloed
 * More implementation of XMPP, thanks a lot to Federico Pinna & Reitek S.p.A.
 *
 *
 */

#include <ptlib.h>
#include <ptclib/xmpp_c2s.h>
#include <ptclib/xmpp_roster.h>
#include "main.h"


#if !P_EXPAT
#error Must have XML support for this application
#endif


PCREATE_PROCESS(XMPPTest);

XMPPTest::XMPPTest()
  : PProcess("Reitek S.p.A.", "XMPPTest", 1, 0, AlphaCode, 1)
{
}


void Usage()
{
  PError << "usage: xmpptest -j JID -p password\n";
}


XMPP::C2S::StreamHandler * c = 0;
XMPP::Roster * r = 0;

void XMPPTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse("j:p:");

  if (!args.HasOption('j') || !args.HasOption('p')) {
    Usage();
    return;
  }

  PString jid = args.GetOptionString('j');
  PString pwd = args.GetOptionString('p');

  PTrace::SetLevel(5);

//  PSASLClient::SetPath("c:\\appl\\reitek\\lib"); // path to the SASL mechanisms dlls (plugins)

  c = new XMPP::C2S::StreamHandler(jid, pwd);
//  c->SetVersion(0, 9); // Old jabber protocol
  c->MessageHandlers().Add(new PCREATE_NOTIFIER(OnMessage));

  r = new XMPP::Roster(c);

  c->Start();

  c->WaitForTermination();
  cout << "Terminated" << endl;
}


void XMPPTest::OnMessage(XMPP::Message& msg, INT)
{
  cout << "Message from " << msg.GetFrom() << ", subject " << msg.GetSubject() << ":\n"
       << msg.GetBody() << '\n' << endl;
}

// End of File ///////////////////////////////////////////////////////////////
