/*
 * main.cxx
 *
 * PWLib application source file for PxmlTest
 *
 * Main program entry point.
 *
 * Copyright 2002 David Iodice.
 *
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


static void TestXML(const PArgList & args, const PString & str)
{
  PXML xml(PXML::Indent, NULL, args.GetOptionString('e'));
  if (xml.Load(str))
    PConsoleChannel(PConsoleChannel::StandardOutput) << xml << endl; // Use this so presents UTF-8 correctly
  else
    cerr << "Parse error: line " << xml.GetErrorLine() << ", col " << xml.GetErrorColumn() << ", " << xml.GetErrorString() << endl;
}


void PxmlTest::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("s-simple.         Simple test\n"
                  "b-billion-laughs. Billion laugh test\n"
                  "e-encoding:       Set encoding character set\n"
                  PTRACE_ARGLIST
  ))
    cerr << args.Usage("[ -e ] -s | -b | { file ... }") << endl;
  else if (args.HasOption('s'))
    TestXML(args, testXML); 
  else if (args.HasOption('b'))
    TestXML(args, billionLaughs); 
  else {
    for (PINDEX i = 0; i < args.GetCount(); ++i) {
      PTextFile file;
      if (!file.Open(args[i], PFile::ReadOnly))
        cerr << "Could not open file: " << args[i] << " - " << file.GetErrorText() << endl;
      else
        TestXML(args, file.ReadString(P_MAX_INDEX));
    }
  }
}

// End of File ///////////////////////////////////////////////////////////////
