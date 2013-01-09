/*
 * main.cxx
 *
 * PWLib application source file for PxmlTest
 *
 * Main program entry point.
 *
 * Copyright 2002 David Iodice.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include "main.h"

PCREATE_PROCESS(PxmlTest);

PxmlTest::PxmlTest()
  : PProcess("XML Testing Guru", "PxmlTest", 1, 0, AlphaCode, 1)
{
}

static const char * testXML =
  "<note><to>Tove</to><from>Jani</from><heading>Reminder</heading><body>Don't forget me this weekend!</body></note>"
  ;

static const char * billionLaughs = 
"<?xml version=\"1.0\"?>"
"<!DOCTYPE lolz ["
" <!ENTITY lol \"lol\">"
" <!ENTITY lol1 \"&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;&lol;\">"
" <!ENTITY lol2 \"&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;&lol1;\">"
" <!ENTITY lol3 \"&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;&lol2;\">"
" <!ENTITY lol4 \"&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;&lol3;\">"
" <!ENTITY lol5 \"&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;&lol4;\">"
" <!ENTITY lol6 \"&lol5;&lol5;&lol5;&lol5;&lol5;&lol5;&lol5;&lol5;&lol5;&lol5;\">"
" <!ENTITY lol7 \"&lol6;&lol6;&lol6;&lol6;&lol6;&lol6;&lol6;&lol6;&lol6;&lol6;\">"
" <!ENTITY lol8 \"&lol7;&lol7;&lol7;&lol7;&lol7;&lol7;&lol7;&lol7;&lol7;&lol7;\">"
" <!ENTITY lol9 \"&lol8;&lol8;&lol8;&lol8;&lol8;&lol8;&lol8;&lol8;&lol8;&lol8;\">"
"]>"
"<lolz>&lol9;</lolz>";

void PxmlTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("f:b");

  PXML xml;
 
  bool status;
  if (args.HasOption('f')) {
    status = xml.LoadFile(args.GetOptionString('f'));
  }
  else if (args.HasOption('b')) {
    status = xml.Load(billionLaughs); 
  }
  else {
    status = xml.Load(testXML); 
  }

  if (!status) 
    cerr << "parse error: line " << xml.GetErrorLine() << ", col " << xml.GetErrorColumn() << ", " << xml.GetErrorString() << endl;
  else
    xml.PrintOn(cout);
}

// End of File ///////////////////////////////////////////////////////////////
