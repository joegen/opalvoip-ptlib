/*
 * pdns.h
 *
 * PWLib library for DNS lookup services
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
 * $Log: pdns.h,v $
 * Revision 1.5  2003/07/22 23:52:20  dereksmithies
 * Fix from Fabrizio Ammollo to cope with when P_DNS is disabled. Thanks!
 *
 * Revision 1.4  2003/04/16 07:02:55  robertj
 * Cleaned up source.
 *
 * Revision 1.3  2003/04/15 08:14:06  craigs
 * Added single string form of GetSRVRecords
 *
 * Revision 1.2  2003/04/15 08:06:24  craigs
 * Added Unix implementation
 *
 * Revision 1.1  2003/04/15 04:06:56  craigs
 * Initial version
 *
 */

#if P_DNS
#ifndef _PDNS_H
#define _PDNS_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptlib/sockets.h>


// implement DNS lookup for MX and SRV records

class PDNS : public PObject
{
  public:
    class Record : public PObject
    {
        PCLASSINFO(Record, PObject);
      public:
        Record();

        PString            hostName;
        PIPSocket::Address hostAddress;
        BOOL               used;
    };

    class SRVRecord : public Record
    {
        PCLASSINFO(SRVRecord, Record);
      public:
        Comparison Compare(const PObject & obj) const;
        void PrintOn(ostream & strm) const;

        WORD port;
        WORD priority;
        WORD weight;
    };

    class MXRecord : public Record
    {
        PCLASSINFO(MXRecord, Record);
      public:
        Comparison Compare(const PObject & obj) const;
        void PrintOn(ostream & strm) const;

        WORD preference;
    };

    PDECLARE_SORTED_LIST(SRVRecordList, SRVRecord)
      public:
        void PrintOn(ostream & strm) const;

        SRVRecord * GetFirst();
        SRVRecord * GetNext();

      protected:
        PINDEX     priPos;
        PWORDArray priList;
    };

    PDECLARE_SORTED_LIST(MXRecordList, MXRecord)
      public:
        void PrintOn(ostream & strm) const;

        MXRecord * GetFirst();
        MXRecord * GetNext();

      protected:
        PINDEX lastIndex;
    };

    static BOOL GetSRVRecords(
      const PString & service,
      const PString & type,
      const PString & domain,
      SRVRecordList & serviceList
    );

    static BOOL GetSRVRecords(
      const PString & service,
      SRVRecordList & serviceList
    );

    static BOOL GetMXRecords(
      const PString & domain,
      MXRecordList & serviceList
    );
};


#endif // _PDNS_H
#endif // P_DNS

// End Of File ///////////////////////////////////////////////////////////////
