/*
 * pdns.cxx
 *
 * PWLib library for DNS lookup services
 *
 * Copyright 2003 Equivalence
 *
 * $Log: pdns.h,v $
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

#ifndef _PDNS

#define _PDNS

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptlib/sockets.h>

// implement DNS lookup for MX and SRV records

class PDNS : public PObject
{
  public:
    class Record : public PObject
    {
      public:
        Record();
        Comparison Compare(const PObject & obj) const = 0;
        void PrintOn(ostream & strm) const = 0;

        PString hostName;
        PIPSocket::Address hostAddress;
        BOOL used;
    };

    class SRVRecord : public Record
    {
      public:
        Comparison Compare(const PObject & obj) const;
        void PrintOn(ostream & strm) const;

        WORD port;
        WORD priority;
        WORD weight;
    };

    class MXRecord : public Record
    {
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
        PINDEX priPos;
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

    static BOOL GetSRVRecords(const PString & _service,
                              const PString & type,
		                          const PString & domain,
		                    PDNS::SRVRecordList & serviceList);

    static BOOL GetSRVRecords(const PString & service,
		                    PDNS::SRVRecordList & serviceList);


    static BOOL GetMXRecords(const PString & domain,
		                    PDNS::MXRecordList & serviceList);
};

#endif
