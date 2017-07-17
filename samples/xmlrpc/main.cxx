/*
 * main.cxx
 *
 * PWLib application source file for XMLRPCApp
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 */

/*

  Example command lines

    http://betty.userland.com/RPC2 examples.getStateName -i 1

    --test-struct http://xmlrpc.usefulinc.com/demo/server.php interopEchoTests.echoStruct
    -s http://10.0.2.13:6666/RPC2 Function1 key value


 */

#include <ptlib.h>
#include "main.h"

PCREATE_PROCESS(XMLRPCApp);

XMLRPCApp::XMLRPCApp()
  : PProcess("Equivalence", "XMLRPCApp", 1, 0, AlphaCode, 1)
{
}

#if P_EXPAT

#include <ptclib/pxmlrpc.h>

PXMLRPC_STRUCT_BEGIN(NestedStruct)
    PXMLRPC_STRING  (NestedStruct, PString, another_string);
    PXMLRPC_INTEGER (NestedStruct, int, another_integer);
PXMLRPC_STRUCT_END()

PXMLRPC_STRUCT_BEGIN     (TestStruct)
    PXMLRPC_STRING_INIT  (TestStruct, PString, a_string, "A string!");
    PXMLRPC_INTEGER_INIT (TestStruct, int, an_integer, 12);
    PXMLRPC_BOOLEAN_INIT (TestStruct, PBoolean, a_boolean, true);
    PXMLRPC_DOUBLE_INIT  (TestStruct, double, a_float, 3.14159);
    PXMLRPC_DATETIME     (TestStruct, PTime, a_date);
    PXMLRPC_BINARY       (TestStruct, PBYTEArray, a_binary);
    PXMLRPC_ARRAY_STRING (TestStruct, PStringArray, PCaselessString, a_string_array);
    PXMLRPC_ARRAY_INTEGER(TestStruct, int, an_integer_array);
    PXMLRPC_ARRAY_DOUBLE (TestStruct, float, a_float_array);
    PXMLRPC_STRUCT       (TestStruct, NestedStruct, nested_struct);
    PXMLRPC_ARRAY_STRUCT (TestStruct, NestedStruct, array_struct);
PXMLRPC_STRUCT_END()


bool AddParam(PXMLRPCBlock & request, PArgList & args,PXMLElement * params)
{
  if (!args.Parse(NULL))
    return false;

  PINDEX arg = 0;

  if (args.HasOption('a')) {
    PINDEX sz = args.GetOptionString('a').AsUnsigned();
    if (sz == 0)
      return true;

    const char * arrType;
    if (args.HasOption('i'))
      arrType = "int";
    else if (args.HasOption('d'))
      arrType = "double";
    else if (args.HasOption('s'))
      arrType = "struct";
    else
      arrType = "string";
    PXMLElement * d;
    PXMLElement * a = request.CreateArray(d);
    params->AddSubObject(a);
    return true;
  }

  if (args.HasOption('s')) {
    PXMLElement * d;
    PXMLElement * s = request.CreateStruct(d);
    params->AddSubObject(s);
    return true;
  }

  if (args.HasOption('i'))
    params->AddSubObject(request.CreateScalar(args[arg++].AsInteger()));
  else if (args.HasOption('f'))
    params->AddSubObject(request.CreateScalar(args[arg++].AsReal()));

  while (arg < args.GetCount())
    params->AddSubObject(request.CreateScalar(args[arg++]));

  return true;
}


/////////////////////////////////////////////////////////////////////////////

void XMLRPCApp::Main()
{
  PArgList & args = GetArguments();

  args.Parse("a-array:"
             "f-float."
             "i-integer."
             "s-struct."
#if PTRACING
             "t-trace."
             "o-output:"
#endif
             "v-verbose."
             "-test-struct."
             );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL);
#endif

  if (args.GetCount() < 2) {
    PError << "usage: xmlrpc [ -v -t ] url method [ <param> ... ]\n"
              "       xmlrpc --test-struct url method\n"
              "\n"
              "Options:\n"
              "  -v or --version              Verbose output\n"
#if PTRACING
              "  -t or --trace                Trace level\n"
              "  -o or --output file          Trace output file\n"
#endif
              "\n"
              "With <param> being one of:\n"
              "  -a N <param>                 array with N <param> elements\n"
              "  -s <param> [ <param> ... ]   structure\n"
              "  -i value                     integer value\n"
              "  -f value                     floating pont value\n"
              "  string                       string value\n"
           << endl;
    return;
  }

  PURL url = args[0];
  PString method = args[1];

  PXMLRPC rpc(url);

  PXMLRPCBlock request(method);
  PXMLRPCBlock response;

  if (args.HasOption("test-struct")) {
    TestStruct ts;
    ts.a_date -= PTimeInterval(0, 0, 0, 0, 5);

    ts.a_binary.SetSize(10);
    for (PINDEX i = 0; i < 10; i++)
      ts.a_binary[i] = (BYTE)(i+1);

    ts.a_string_array.SetSize(3);
    ts.a_string_array[0] = "first";
    ts.a_string_array[1] = "second";
    ts.a_string_array[2] = "third";

    ts.an_integer_array.SetSize(7);
    for (PINDEX i = 0; i < ts.an_integer_array.GetSize(); i++)
      ts.an_integer_array[i] = i+1;

    ts.a_float_array.SetSize(5);
    for (PINDEX i = 0; i < ts.a_float_array.GetSize(); i++)
      ts.a_float_array[i] = (float)(1.0/(i+2));

    ts.nested_struct.another_string = "Another string!";
    ts.nested_struct.another_integer = 345;

    ts.array_struct.SetSize(2);
    ts.array_struct.SetAt(0, new NestedStruct);
    ts.array_struct[0].another_string = "Structure one";
    ts.array_struct[0].another_integer = 11111;
    ts.array_struct.SetAt(1, new NestedStruct);
    ts.array_struct[1].another_string = "Structure two";
    ts.array_struct[1].another_integer = 22222;
    request.AddParam(ts);
  }
  else {
    while (AddParam(request, args, request.GetParams()))
      ;
  }

  if (args.HasOption('v'))
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
  for (PINDEX i = 0; i < response.GetParamCount(); i++) {
    cout << "  " << i << ": ";
    PString type;
    PString val;
    if (response.GetParam(i, type, val)) {
      cout << type << " = ";
      if (type == "struct") {
        PStringToString dict;
        response.GetParam(i, dict);
        cout << '\n' << dict;
      }
      else if (type == "array") {
        PStringArray array;
        response.GetParam(i, array);
        cout << '\n' << setfill('\n') << array << setfill(' ');
      }
      else
        cout << val;
    }
    else
      cout << "error: " << response.GetFaultText();
    cout << endl;
  }

  if (args.HasOption("test-struct")) {
    TestStruct ts;
    ts.a_date = PTime(0);
    if (response.GetParam(0, ts))
      cout << "Parsed response:\n" << ts;
    else
      cout << "Failed to parse resonse: " << response.GetFaultText();
    cout << endl;
  }
}

#else
#pragma message("Must have XML support for this application")

void XMLRPCApp::Main()
{
}

#endif


// End of File ///////////////////////////////////////////////////////////////
