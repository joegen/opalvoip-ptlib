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
 * Revision 1.5  2004/05/31 13:57:00  csoutheren
 * Added tests for ENUM resolution
 *
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
#include <ptclib/enum.h>
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
            "       dnstest -t SRV service            (i.e. _ras._udp._example.com)\n"
            "       dnstest -t NAPTR resource         (i.e. 2.1.2.1.5.5.5.0.0.8.1.e164.org)\n"
            "       dnstest -t NAPTR resource service (i.e. 2.1.2.1.5.5.5.0.0.8.1.e164.org E2U+SIP)\n"
            "       dnstest -t ENUM service           (i.e. +18005551212 E2U+SIP)\n"
  ;
}

template <class RecordListType>
void GetAndDisplayRecords(const PString & name)
{
  RecordListType records;
  if (!PDNS::GetRecords(name, records))
    PError << "Lookup for " << name << " failed" << endl;
  else
    cout << "Lookup for " << name << " returned" << endl << records << endl;
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
  if (type *= "SRV") 
    GetAndDisplayRecords<PDNS::SRVRecordList>(args[0]);

  else if (type *= "MX")
    GetAndDisplayRecords<PDNS::MXRecordList>(args[0]);

  else if (type *= "NAPTR") {
    if (args.GetCount() == 1)
      GetAndDisplayRecords<PDNS::NAPTRRecordList>(args[0]);
    else {
      PDNS::NAPTRRecordList records;
      if (!PDNS::GetRecords(args[0], records))
        PError << "Lookup for " << args[0] << " failed" << endl;
      else {
        cout << "Returned " << endl;
        PDNS::NAPTRRecord * rec = records.GetFirst(args[1]);
        while (rec != NULL) {
          cout << *rec;
          rec = records.GetNext(args[1]);
        }
      }
    }
  }

  else if (type *= "enum") {
    if (args.GetCount() < 2)
      Usage();
    else {
      PString e164    = args[0];
      PString service = args[1];
      PString str;
      if (!PDNS::ENUMLookup(e164, service, str))
        cout << "Could not resolve E164 number " << e164 << " with service " << service << endl;
      else
        cout << "E164 number " << e164 << " with service " << service << " resolved to " << str << endl;
    }
  }
}

// End of File ///////////////////////////////////////////////////////////////
