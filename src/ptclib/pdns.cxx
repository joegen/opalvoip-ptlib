/*
 * pdns.cxx
 *
 * PWLib library for DNS lookup services
 *
 * Copyright 2003 Equivalence
 *
 * $Log: pdns.cxx,v $
 * Revision 1.1  2003/04/15 04:06:35  craigs
 * Initial version
 *
 */

#include <ptlib.h>

#include <ptclib/pdns.h>
#include <ptclib/random.h>

/////////////////////////////////////////////////

PDNS::Record::Record()
{ 
  used = FALSE; 
}

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
    PINDEX count = 0;
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
    PINDEX j = firstPos + ((count == 0) ? 0 : (PRandom::Number() % (count-1)) );
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

#ifdef WIN32
  PDNS_RECORD results = NULL;
  DNS_STATUS status = DnsQuery_A((const char *)service, 
                                 DNS_TYPE_SRV, 
                                 DNS_QUERY_STANDARD, 
                                 NULL, 
                                 &results, 
                                 NULL);
  if (status != 0)
    return FALSE;

  // find SRV records
  PDNS_RECORD dnsRecord = results;
  while (dnsRecord != NULL) {
    if ((dnsRecord->Flags.S.Section == DnsSectionAnswer) && (dnsRecord->wType == DNS_TYPE_SRV)) {
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

  return TRUE;

#else

#if 0

  PBYTEArray reply(1024);
  int replyLen = res_query(service, C_IN, T_SRV, (u_char *)(const BYTE *)reply, reply.GetSize());
  if (replyLen <= 0) {
    PError << "Cannot resolve service " << service << endl;
    return FALSE;
  }
  reply.SetSize(replyLen);

  cout << "Reply is " << endl << hex << reply << dec << endl;

  char buffer[1024];
  const BYTE * p = reply + sizeof(HEADER);

  // ignore first record - it returns what we passed in
  int len = dn_expand(reply, reply + replyLen, p, buffer, sizeof(buffer));
  if (len < 0) {
    PError << "SRV record is weird" << endl;
    return FALSE;
  }
  p += len;
  p += 4;

  while (p < reply + replyLen) {
    len = dn_expand(reply, reply + replyLen, p, buffer, sizeof(buffer));
    if (len < 0) {
      PError << "Failed to parse hostname at 0x" << hex << (p - reply) << dec << endl;
      break;
    }

    p += len;

    int type = (p[0] << 8) | p[1];
    p += 2;

    //int dnsClass = (p[0] << 8) | p[1];
    p += 2;

    //int ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += 4;

    int size = (p[0] << 8) | p[1];
    p += 2;

    PString hostName(buffer);

    if (type == T_SRV) {
      len = dn_expand(reply, reply + replyLen, p + 6, buffer, sizeof(buffer));
      if (len < 0) {
        PError << "Failed to parse T_SRV hostname at 0x" << hex << ((p + 6) - reply) << dec << endl;
        break;
      }

      SRVRecord * record = new SRVRecord();
      record->host     = PString(buffer);
      record->port     = (WORD)((p[4] << 8) | p[5]);
      record->priority = (WORD)((p[0] << 8) | p[1]);
      record->weight   = (WORD)((p[2] << 8) | p[3]);

      recordList.Append(record);
    }
  
    p += size;
  }

  return serviceList.GetSize() != 0;
#endif
  return NULL
#endif
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

#ifdef WIN32
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

  return TRUE;

#else

#if 0

  PBYTEArray reply(1024);
  int replyLen = res_query(service, C_IN, T_SRV, (u_char *)(const BYTE *)reply, reply.GetSize());
  if (replyLen <= 0) {
    PError << "Cannot resolve service " << service << endl;
    return FALSE;
  }
  reply.SetSize(replyLen);

  cout << "Reply is " << endl << hex << reply << dec << endl;

  char buffer[1024];
  const BYTE * p = reply + sizeof(HEADER);

  // ignore first record - it returns what we passed in
  int len = dn_expand(reply, reply + replyLen, p, buffer, sizeof(buffer));
  if (len < 0) {
    PError << "SRV record is weird" << endl;
    return FALSE;
  }
  p += len;
  p += 4;

  while (p < reply + replyLen) {
    len = dn_expand(reply, reply + replyLen, p, buffer, sizeof(buffer));
    if (len < 0) {
      PError << "Failed to parse hostname at 0x" << hex << (p - reply) << dec << endl;
      break;
    }

    p += len;

    int type = (p[0] << 8) | p[1];
    p += 2;

    //int dnsClass = (p[0] << 8) | p[1];
    p += 2;

    //int ttl = (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3];
    p += 4;

    int size = (p[0] << 8) | p[1];
    p += 2;

    PString hostName(buffer);

    if (type == T_SRV) {
      len = dn_expand(reply, reply + replyLen, p + 6, buffer, sizeof(buffer));
      if (len < 0) {
        PError << "Failed to parse T_SRV hostname at 0x" << hex << ((p + 6) - reply) << dec << endl;
        break;
      }

      Service * service = new Service();
      service->host     = PString(buffer);
      service->port     = (WORD)((p[4] << 8) | p[5]);
      service->priority = (WORD)((p[0] << 8) | p[1]);
      service->weight   = (WORD)((p[2] << 8) | p[3]);

      serviceList.Append(service);
    }
  
    p += size;
  }

  return serviceList.GetSize() != 0;
#endif

  return NULL;

#endif
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

///////////////////////////////////////////////////////
