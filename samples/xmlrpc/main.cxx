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
 * Revision 1.3  2002/10/04 05:16:44  craigs
 * Changed for new XMLRPC code
 *
 * Revision 1.2  2002/03/27 01:54:40  craigs
 * Added ability to send random struct as request
 * Added ability to preview request without sending
 *
 * Revision 1.1  2002/03/26 07:05:28  craigs
 * Initial version
 *
 */

/*

  Example command lines

    http://time.xmlrpc.com/RPC2 currentTime.getCurrentTime 

    http://www.mirrorproject.com/xmlrpc mirror.Random

    http://www.newsisfree.com/xmlrpc.php

    -s http://10.0.2.13:6666/RPC2 Function1 key value


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

  args.Parse("t."
             "o:"
             "s."
             "d."
             );

  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL);

  if (args.GetCount() < 2) {
    PError << "usage: xmlrpc url method [parms...]" << endl;
    return;
  }

  PString url    = args[0];
  PString method = args[1];

  PXMLRPC rpc(url);

  PXMLRPCBlock request(method);

  if (args.HasOption('s')) {
    PStringToString dict;
    PINDEX i;
    for (i = 2; (i+1) < args.GetCount(); i += 2) {
      PString key   = args[i];
      PString value = args[i+1];
      dict.SetAt(key, value);
    }

    request.AddStruct(dict);
  }

  if (args.HasOption('d')) {
    cout << "Request = " << request;
    return;
  }

  PXMLRPCBlock response;

  if (!rpc.MakeRequest(request, response)) {
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
