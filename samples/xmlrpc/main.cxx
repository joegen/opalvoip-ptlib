/*
 * main.cxx
 *
 * PWLib application source file for XMLRPCApp
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 * $Log: main.cxx,v $
 * Revision 1.1  2002/03/26 07:05:28  craigs
 * Initial version
 *
 */

/*

  Example command lines

    http://time.xmlrpc.com/RPC2 currentTime.getCurrentTime 

    http://www.mirrorproject.com/xmlrpc mirror.Random


 */

#include <ptlib.h>
#include "main.h"

#include <ptclib/pxmlrpc.h>

PCREATE_PROCESS(XMLRPCApp);

XMLRPCApp::XMLRPCApp()
  : PProcess("Equivalence", "XMLRPCApp", 1, 0, AlphaCode, 1)
{
}

void XMLRPCApp::Main()
{
  PArgList & args = GetArguments();

  args.Parse("t.o:");

  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL);

  if (args.GetCount() < 2) {
    PError << "usage: xmlrpc url method [parms...]" << endl;
    return;
  }

  PString url    = args[0];
  PString method = args[1];

  PXMLRPC rpc(url);

  PXMLRPCResponse response;
  if (!rpc.MakeRequest(method, response)) {
    PError << "Error in request (" 
           << rpc.GetFaultCode() 
           << ") : "
           << rpc.GetFaultText()
           << endl;
    return;
  }

  // scan through the response and print it out
  cout << "Response" << endl;
  PINDEX i;
  for (i = 0; i < response.GetParamCount(); i++) {
    cout << "  " << i << ": ";
    PString type;
    PString val;
    if (response.GetParam(i, type, val)) {
      cout << type << " = " << val << endl;
    } else {
      cout << "error" << endl;
    }
  }
}

// End of File ///////////////////////////////////////////////////////////////
