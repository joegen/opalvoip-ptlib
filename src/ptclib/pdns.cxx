/*
 * pdns.cxx
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * Copyright 2003 Equivalence Pty. Ltd.
 *
 * $Log: pdns.cxx,v $
 * Revision 1.12  2004/02/23 23:52:19  csoutheren
 * Added pragmas to avoid every Windows application needing to include libs explicitly
 *
 * Revision 1.11  2004/01/03 03:37:53  csoutheren
 * Fixed compile problem on Linux
 *
 * Revision 1.10  2004/01/03 03:10:42  csoutheren
 * Fixed more problems with looking up SRV records, especially on Windows
 *
 * Revision 1.9  2004/01/02 13:22:04  csoutheren
 * Fixed problem with extracting SRV records from DNS
 *
 * Revision 1.8  2003/11/02 15:52:58  shawn
 * added arpa/nameser_compat.h for Mac OS X 10.3 (Panther)
 *
 * Revision 1.7  2003/04/28 23:57:40  robertj
 * Fixed Solaris compatibility
 *
 * Revision 1.6  2003/04/22 23:21:37  craigs
 * Swapped includes at request of Shawn Hsiao for compatiobility with MacOSX
 *
 * Revision 1.5  2003/04/16 14:21:12  craigs
 * Added set of T_SRV for MacOS
 *
 * Revision 1.4  2003/04/16 08:00:19  robertj
 * Windoes psuedo autoconf support
 *
 * Revision 1.3  2003/04/15 08:14:32  craigs
 * Added single string form of GetSRVRecords
 *
 * Revision 1.2  2003/04/15 08:05:19  craigs
 * Added Unix implementation
 *
 * Revision 1.1  2003/04/15 04:06:35  craigs
 * Initial version
 *
 */

#ifdef __GNUC__
#pragma implementation "pdns.h"
#endif

#include <ptlib.h>

#include <ptclib/pdns.h>

#if P_DNS

#include <ptclib/random.h>

#if defined(_WIN32)

#include <windns.h>

#pragma comment(lib,    P_DNS_LIBRARY)
#pragma comment(linker, "/delayload:dnsapi.dll")
#pragma comment(lib,    "Delayimp.lib")

#else

#define	P_HAS_RESOLVER	1

#if P_HAS_RESOLVER
#include <arpa/nameser.h>
#include <resolv.h>
#if defined(P_MACOSX) && (P_MACOSX >= 700)
#include <arpa/nameser_compat.h>
#endif
#endif

#endif

#ifndef	T_SRV
#define	T_SRV	33
#endif


/////////////////////////////////////////////////

PDNS::Record::Record()
{ 
  used = FALSE; 
}


/////////////////////////////////////////////////


#ifdef P_HAS_RESOLVER

#define	DNS_STATUS		int
#define	DNS_TYPE_SRV		T_SRV
#define	DNS_TYPE_MX		T_MX
#define	DNS_TYPE_A		T_A
#define	DnsFreeRecordList	0
#define	DNS_QUERY_STANDARD	0
#define	DNS_QUERY_BYPASS_CACHE	0

typedef struct _DnsAData {
  DWORD IpAddress;
} DNS_A_DATA;

typedef struct {
  char   pNameExchange[MAXDNAME];
  WORD   wPreference;

} DNS_MX_DATA;

typedef struct {
  char pNameHost[MAXDNAME];
} DNS_PTR_DATA;

typedef struct _DnsSRVData {
  public:
    char   pNameTarget[MAXDNAME];
    WORD   wPriority;
    WORD   wWeight;
    WORD   wPort;

} DNS_SRV_DATA;

typedef struct _DnsRecordFlags
{
    unsigned   Section     : 2;
    unsigned   Delete      : 1;
    unsigned   CharSet     : 2;
    unsigned   Unused      : 3;
    unsigned   Reserved    : 24;
} DNS_RECORD_FLAGS;

typedef enum _DnsSection
{
  DnsSectionQuestion,
  DnsSectionAnswer,
  DnsSectionAuthority,
  DnsSectionAddtional,
} DNS_SECTION;


class DnsRecord {
  public:
    DnsRecord * pNext;
    char        pName[MAXDNAME];
    WORD        wType;
    WORD        wDataLength;

    union {
      DWORD               DW;     // flags as DWORD
      DNS_RECORD_FLAGS    S;      // flags as structure
    } Flags;

    union {
      DNS_A_DATA   A;
      DNS_MX_DATA  MX;
      DNS_PTR_DATA NS;
      DNS_SRV_DATA SRV;
    } Data;
};

typedef DnsRecord * PDNS_RECORD;

static BOOL GetDN(const BYTE * reply, const BYTE * replyEnd, BYTE * & cp, char * buff)
{
  int len = dn_expand(reply, replyEnd, cp, buff, MAXDNAME);
  if (len < 0)
    return FALSE;
  cp += len;
  return TRUE;
}

static BOOL ProcessDNSRecords(
		const BYTE * reply,
	        const BYTE * replyEnd,
		      BYTE * cp,
		    PINDEX anCount,
		    PINDEX nsCount,
		    PINDEX arCount,
	       PDNS_RECORD * results)
{
  PDNS_RECORD lastRecord = NULL;
  PDNS_RECORD newRecord  = NULL;

  PINDEX rrCount = anCount + nsCount + arCount;
  nsCount += anCount;
  arCount += nsCount;

  PINDEX i;
  for (i = 0; i < rrCount; i++) {

    if (newRecord == NULL) 
      newRecord = new DnsRecord;

    memset(newRecord, 0, sizeof(DnsRecord));

    if (i < anCount)
      newRecord->Flags.S.Section = DnsSectionAnswer;
    else if (i < nsCount)
      newRecord->Flags.S.Section = DnsSectionAuthority;
    else if (i < arCount)
      newRecord->Flags.S.Section = DnsSectionAddtional;

    // get the name
    if (!GetDN(reply, replyEnd, cp, newRecord->pName)) {
      delete newRecord;
      return FALSE;
    }

    // get other common parts of the record
    WORD  type;
    WORD  dnsClass;
    DWORD ttl;
    WORD  dlen;

    GETSHORT(type,     cp);
    GETSHORT(dnsClass, cp);
    GETLONG (ttl,      cp);
    GETSHORT(dlen,     cp);

    newRecord->wType = type;

    BYTE * data = cp;
    cp += dlen;
    BOOL ok = TRUE;

    switch (type) {
      case T_SRV:
        GETSHORT(newRecord->Data.SRV.wPriority, data);
        GETSHORT(newRecord->Data.SRV.wWeight, data);
        GETSHORT(newRecord->Data.SRV.wPort, data);
        if (!GetDN(reply, replyEnd, data, newRecord->Data.SRV.pNameTarget)) {
          delete newRecord;
          return FALSE;
        }
        break;

      case T_MX:
        GETSHORT(newRecord->Data.MX.wPreference,  data);
        if (!GetDN(reply, replyEnd, data, newRecord->Data.MX.pNameExchange)) {
          delete newRecord;
          return FALSE;
        }
        break;

      case T_A:
        GETLONG(newRecord->Data.A.IpAddress, data);
        break;

      case T_NS:
        if (!GetDN(reply, replyEnd, data, newRecord->Data.NS.pNameHost)) {
          delete newRecord;
          return FALSE;
        }
        break;

      default:
        ok = FALSE;
        break;
    }

    if (ok) {
      if (*results == NULL)
        *results = newRecord;

      newRecord->pNext = NULL;

      if (lastRecord != NULL)
        lastRecord->pNext = newRecord;

      lastRecord = newRecord;
      newRecord = NULL;
    }
  }

  delete newRecord;

  return TRUE;
}

void DnsRecordListFree(PDNS_RECORD rec, int /* FreeType */)
{
  while (rec != NULL) {
    PDNS_RECORD next = rec->pNext;
    delete rec;
    rec = next;
  }
}

DNS_STATUS DnsQuery_A(const char * service,
		      WORD requestType,
		      DWORD options,
		      void *,
		      PDNS_RECORD * results,
		      void *)
{
  if (results == NULL)
    return -1;

  *results = NULL;

  res_init();

  union {
    HEADER hdr;
    BYTE buf[PACKETSZ];
  } reply;

  int replyLen = res_search(service, C_IN, requestType, (BYTE *)&reply, sizeof(reply));

  if (replyLen < 1)
    return -1;

  BYTE * replyStart = reply.buf;
  BYTE * replyEnd   = reply.buf + replyLen;
  BYTE * cp         = reply.buf + sizeof(HEADER);

  // ignore questions in response
  unsigned i;
  for (i = 0; i < ntohs(reply.hdr.qdcount); i++) {
    char qName[MAXDNAME];
    if (!GetDN(replyStart, replyEnd, cp, qName))
      return -1;
    cp += QFIXEDSZ;
  }

  if (!ProcessDNSRecords(replyStart,
		         replyEnd,
			 cp,
		 	 ntohs(reply.hdr.ancount),
			 ntohs(reply.hdr.nscount),
			 ntohs(reply.hdr.arcount),
	 		 results)) {
    DnsRecordListFree(*results, 0);
    return -1;
  }

  return 0;
}


#endif // P_HAS_RESOLVER


/////////////////////////////////////////////////

PObject::Comparison PDNS::SRVRecord::Compare(const PObject & obj) const
{
  PDNS::SRVRecord & other = (PDNS::SRVRecord &)obj;
  if (priority < other.priority)
    return LessThan;
  else if (priority > other.priority)
    return GreaterThan;
  if (weight < other.weight)
    return LessThan;
  else if (weight > other.weight)
    return GreaterThan;
  return EqualTo;
}

void PDNS::SRVRecord::PrintOn(ostream & strm) const
{
  strm << "host=" << hostName << ":" << port << "(" << hostAddress << "), "
       << "priority=" << priority << ", "
       << "weight=" << weight;
}

/////////////////////////////////////////////////

void PDNS::SRVRecordList::PrintOn(ostream & strm) const
{
  PINDEX i;
  for (i = 0; i < GetSize(); i++) 
    strm << (*this)[i] << endl;
}

PDNS::SRVRecord * PDNS::SRVRecordList::GetFirst()
{
  if (GetSize() == 0)
    return NULL;

  // create a list of all prioities, to save time
  priPos = 0;
  priList.SetSize(0);

  PINDEX i;
  if (GetSize() > 0) {
    priList.SetSize(1);
    WORD lastPri = (*this)[0].priority;
    priList[0] = lastPri;
    (*this)[0].used = FALSE;
    for (i = 1; i < GetSize(); i++) {
      (*this)[i].used = FALSE;
      if ((*this)[i].priority != lastPri) {
        priList.SetSize(priPos+1);
        lastPri = (*this)[i].priority;
        priList[priPos] = lastPri;
      }
    }
  }
  
  priPos = 0;
  return GetNext();
}

PDNS::SRVRecord * PDNS::SRVRecordList::GetNext()
{
  if (priList.GetSize() == 0)
    return NULL;

  while (priPos < priList.GetSize()) {

    WORD currentPri = priList[priPos];

    // find first record at current priority
    PINDEX firstPos;
    for (firstPos = 0; (firstPos < GetSize()) && ((*this)[firstPos].priority != currentPri); firstPos++) 
      ;
    if (firstPos == GetSize())
      return NULL;

    // calculate total of all unused weights at this priority
    unsigned totalWeight = (*this)[firstPos].weight;
    PINDEX i = firstPos + 1;
    PINDEX count = 1;
    while (i < GetSize() && ((*this)[i].priority == currentPri)) {
      if (!(*this)[i].used) {
        totalWeight += (*this)[i].weight;
        count ++;
      }
    }

    // if no matches found, go to the next priority level
    if (count == 0) {
      priPos++;
      continue;
    }

    // selected the correct item
    if (totalWeight > 0) {
      unsigned targetWeight = PRandom::Number() % (totalWeight+1);
      totalWeight = 0;
      for (i = 0; i < GetSize() && ((*this)[i].priority == currentPri); i++) {
        if (!(*this)[i].used) {
          totalWeight += (*this)[i].weight;
          if (totalWeight >= targetWeight) {
            (*this)[i].used = TRUE;
            return &(*this)[i];
  	  }
        }
      }
    }

    // pick a random item at this priority
    PINDEX j = firstPos + ((count == 0) ? 0 : (PRandom::Number() % count) );
    count = 0;
    for (i = 0; i < GetSize() && ((*this)[i].priority == currentPri); i++) {
      if (!(*this)[i].used) {
        if (count == j) {
          (*this)[i].used = TRUE;
          return &(*this)[i];
	      }
        count++;
      }
    }

    // go to the next priority level
    priPos++;
  }

  return NULL;
}

///////////////////////////////////////////////////////

BOOL PDNS::GetSRVRecords(const PString & _service,
                         const PString & type,
		                     const PString & domain,
		                     SRVRecordList & recordList)
{

  if (_service.IsEmpty())
    return FALSE;

  PString service;
  if (_service[0] != '_')
    service = PString("_") + _service;
  else
    service = _service;

  service += PString("._") + type + "." + domain;

  return GetSRVRecords(service, recordList);
}

BOOL PDNS::GetSRVRecords(const PString & service, PDNS::SRVRecordList & recordList)
{
  recordList.RemoveAll();

  PDNS_RECORD results = NULL;
  DNS_STATUS status = DnsQuery_A((const char *)service, 
                                 DNS_TYPE_SRV, 
                                 DNS_QUERY_STANDARD | DNS_QUERY_BYPASS_CACHE, 
                                 NULL, 
                                 &results, 
                                 NULL);
  if (status != 0)
    return FALSE;

  // find SRV records
  PDNS_RECORD dnsRecord = results;
  while (dnsRecord != NULL) {
    if (
        (dnsRecord->Flags.S.Section == DnsSectionAnswer) && 
        (dnsRecord->wType == DNS_TYPE_SRV) &&
        (strcmp(dnsRecord->Data.SRV.pNameTarget, ".") != 0)
       ) {
      SRVRecord * record = new SRVRecord();
      record->hostName = PString(dnsRecord->Data.SRV.pNameTarget);
      record->port     = results->Data.SRV.wPort;
      record->priority = results->Data.SRV.wPriority;
      record->weight   = results->Data.SRV.wWeight;

      // see if any A records match this hostname
      PDNS_RECORD aRecord = results;
      while (aRecord != NULL) {
        if ((dnsRecord->Flags.S.Section == DnsSectionAddtional) && (dnsRecord->wType == DNS_TYPE_A)) {
          record->hostAddress = PIPSocket::Address(dnsRecord->Data.A.IpAddress);
          break;
        }
        aRecord = aRecord->pNext;
      }

      // if no A record found, then get address the hard way
      if (aRecord == NULL)
        PIPSocket::GetHostAddress(record->hostName, record->hostAddress);

      // add record to the list
      recordList.Append(record);
    }
    dnsRecord = dnsRecord->pNext;
  }

  if (results != NULL)
    DnsRecordListFree(results, DnsFreeRecordList);

  return recordList.GetSize() != 0;
}

///////////////////////////////////////////////////////

PObject::Comparison PDNS::MXRecord::Compare(const PObject & obj) const
{
  PDNS::MXRecord & other = (PDNS::MXRecord &)obj;
  if (preference < other.preference)
    return LessThan;
  else if (preference > other.preference)
    return GreaterThan;
  return EqualTo;
}

void PDNS::MXRecord::PrintOn(ostream & strm) const
{
  strm << "host=" << hostName << "(" << hostAddress << "), "
       << "preference=" << preference;
}

///////////////////////////////////////////////////////

BOOL PDNS::GetMXRecords(const PString & domain, MXRecordList & recordList)
{
  if (domain.IsEmpty())
    return FALSE;

  recordList.RemoveAll();

  PDNS_RECORD results = NULL;
  DNS_STATUS status = DnsQuery_A((const char *)domain, 
                                 DNS_TYPE_MX, 
                                 DNS_QUERY_STANDARD, 
                                 NULL, 
                                 &results, 
                                 NULL);
  if (status != 0)
    return FALSE;

  // find MX records
  PDNS_RECORD dnsRecord = results;
  while (dnsRecord != NULL) {
    if ((dnsRecord->Flags.S.Section == DnsSectionAnswer) && (dnsRecord->wType == DNS_TYPE_MX)) {
      MXRecord * record = new MXRecord();
      record->hostName   = PString(dnsRecord->Data.MX.pNameExchange);
      record->preference = dnsRecord->Data.MX.wPreference;

      // see if any A records match this hostname
      PDNS_RECORD aRecord = results;
      while (aRecord != NULL) {
        if ((dnsRecord->Flags.S.Section == DnsSectionAddtional) && (dnsRecord->wType == DNS_TYPE_A)) {
          record->hostAddress = PIPSocket::Address(dnsRecord->Data.A.IpAddress);
          break;
        }
        aRecord = aRecord->pNext;
      }

      // if no A record found, then get address the hard way
      if (aRecord == NULL)
        PIPSocket::GetHostAddress(record->hostName, record->hostAddress);

      // add record to the list
      recordList.Append(record);
    }
    dnsRecord = dnsRecord->pNext;
  }

  if (results != NULL)
    DnsRecordListFree(results, DnsFreeRecordList);

  return recordList.GetSize() != 0;
}

///////////////////////////////////////////////////////

void PDNS::MXRecordList::PrintOn(ostream & strm) const
{
  PINDEX i;
  for (i = 0; i < GetSize(); i++) 
    strm << (*this)[i] << endl;
}

PDNS::MXRecord * PDNS::MXRecordList::GetFirst()
{
  PINDEX i;
  for (i = 0; i < GetSize(); i++) 
    (*this)[i].used = FALSE;

  lastIndex = 0;

  return GetNext();
}

PDNS::MXRecord * PDNS::MXRecordList::GetNext()
{
  if (GetSize() == 0)
    return NULL;

  if (lastIndex >= GetSize())
    return NULL;

  return (PDNS::MXRecord *)GetAt(lastIndex++);
}


#endif // P_DNS


// End Of File ///////////////////////////////////////////////////////////////
