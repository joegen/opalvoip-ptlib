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
 * Revision 1.5  2002/12/04 02:09:17  robertj
 * Changed macro name prefix to PXMLRPC
 *
 * Revision 1.4  2002/12/04 00:16:18  robertj
 * Large enhancement to create automatically encoding and decoding structures
 *   using macros to build a class.
 *
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

    http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoString "A test!"
    -i http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoInteger 12
    -f http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoFloat 3.121
    -s http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoStruct first 1st second 2nd third 3rd
    --echo-struct http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoStruct
    -s http://10.0.2.13:6666/RPC2 Function1 key value


 */

#include <ptlib.h>
#include "main.h"

#include <ptclib/pxmlrpc.h>


PXMLRPC_STRUCT_BEGIN(NestedStruct)
    PXMLRPC_STRING  (NestedStruct, PString, another_string);
    PXMLRPC_INTEGER (NestedStruct, int, another_integer);
PXMLRPC_STRUCT_END()

PXMLRPC_STRUCT_BEGIN(TestStruct)
    PXMLRPC_STRING  (TestStruct, PString, a_string);
    PXMLRPC_INTEGER (TestStruct, int, an_integer);
    PXMLRPC_BOOLEAN (TestStruct, BOOL, a_boolean);
    PXMLRPC_DOUBLE  (TestStruct, double, a_float);
    PXMLRPC_DATETIME(TestStruct, PTime, a_date);
    PXMLRPC_BINARY  (TestStruct, PBYTEArray, a_binary);
    PXMLRPC_STRUCT  (TestStruct, NestedStruct, nested_struct);
PXMLRPC_STRUCT_END()
 

PCREATE_PROCESS(XMLRPCApp);


/////////////////////////////////////////////////////////////////////////////

XMLRPCApp::XMLRPCApp()
  : PProcess("Equivalence", "XMLRPCApp", 1, 0, AlphaCode, 1)
{
}

void XMLRPCApp::Main()
{
  PINDEX i;
  PArgList & args = GetArguments();

  args.Parse("d-debug."
             "f-float."
             "i-integer."
             "o-output:"
             "s-struct."
             "t-trace."
             "-echo-struct."
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
  PXMLRPCBlock response;

  if (args.HasOption("echo-struct")) {
    TestStruct ts;
    ts.a_string = "A string!";
    ts.an_integer = 12;
    ts.a_float = 3.14159;
    ts.a_boolean = TRUE;
    ts.a_date = PTime() - PTimeInterval(0, 0, 0, 0, 5);
    ts.a_binary.SetSize(10);
    for (i = 0; i < 10; i++)
      ts.a_binary[i] = (BYTE)(i+1);
    ts.nested_struct.another_string = "Another string!";
    ts.nested_struct.another_integer = 345;
    request.AddParam(ts);
  }
  else {
    if (args.HasOption('s')) {
      PStringToString dict;
      for (i = 2; (i+1) < args.GetCount(); i += 2) {
        PString key   = args[i];
        PString value = args[i+1];
        dict.SetAt(key, value);
      }

      request.AddStruct(dict);
    }
    else {
      for (i = 2; i < args.GetCount(); i++) {
        if (args.HasOption('i'))
          request.AddParam(args[i].AsInteger());
        else if (args.HasOption('f'))
          request.AddParam(args[i].AsReal());
        else
          request.AddParam(args[i]);
      }
    }
  }

  if (args.HasOption('d'))
    cout << "Request = " << request << endl;

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
  for (i = 0; i < response.GetParamCount(); i++) {
    cout << "  " << i << ": ";
    PString type;
    PString val;
    if (response.GetParam(i, type, val)) {
      cout << type << " = ";
      if (type != "struct")
        cout << val;
      else {
        PStringToString dict;
        response.GetParam(i, dict);
        cout << '\n' << dict;
      }
    }
    else
      cout << "error: " << response.GetFaultText();
    cout << endl;
  }

  if (args.HasOption("echo-struct")) {
    TestStruct ts;
    ts.a_date = PTime(0);
    if (response.GetParam(0, ts))
      cout << "Parsed response:\n" << ts;
    else
      cout << "Failed to parse resonse: " << response.GetFaultText();
    cout << endl;
  }
}

// End of File ///////////////////////////////////////////////////////////////
