/*
 * main.cxx
 *
 * PWLib application source file for DNSTest
 *
 * Main program entry point.
 *
 * Copyright 2003 Equivalence
 *
 * $Log: main.cxx,v $
 * Revision 1.4  2003/09/26 13:42:16  rjongbloed
 * Added special test to give more indicative error if try to compile without DNS support.
 *
 * Revision 1.3  2003/04/22 23:25:13  craigs
 * Changed help message for SRV records
 *
 * Revision 1.2  2003/04/15 08:15:16  craigs
 * Added single string form of GetSRVRecords
 *
 * Revision 1.1  2003/04/15 04:12:38  craigs
 * Initial version
 *
 */

#include <ptlib.h>
#include <ptclib/pdns.h>
#include "main.h"


#if !P_DNS
#error Must have DNS support for this application
#endif


PCREATE_PROCESS(DNSTest);

DNSTest::DNSTest()
  : PProcess("Equivalence", "DNSTest", 1, 0, AlphaCode, 1)
{
}

void Usage()
{
  PError << "usage: dnstest -t MX hostname\n"
            "       dnstest -t SRV service    (i.e. _ras._udp._example.com)\n"
  ;
}

void DNSTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse("t:");

  if (args.GetCount() < 1) {
    Usage();
    return;
  }

  PString type = args.GetOptionString('t');
  if (type *= "SRV") {
    PDNS::SRVRecordList srvRecords;
    if (!PDNS::GetSRVRecords(args[0], srvRecords))
      PError << "GetSRVRecords failed" << endl;
    else
      cout << "SRV Returned " << srvRecords << endl;
  }

  else if (type *= "MX") {
    PDNS::MXRecordList mxRecords;
    if (!PDNS::GetMXRecords(args[0], mxRecords)) {
      PError << "GetMXRecords failed";
    } else {
      cout << "MX returned " << mxRecords << endl;
    } 
  }
}

// End of File ///////////////////////////////////////////////////////////////
