/*
 * $Id: psnmp.h,v 1.2 1996/09/20 12:19:36 robertj Exp $
 *
 * SNMP Interface
 *
 * Copyright 1996 Equivalence
 *
 * $Log: psnmp.h,v $
 * Revision 1.2  1996/09/20 12:19:36  robertj
 * Used read timeout instead of member variable.
 *
 * Revision 1.1  1996/09/14 12:58:57  robertj
 * Initial revision
 *
 * Revision 1.6  1996/05/09 13:23:49  craigs
 * Added trap functions
 *
 * Revision 1.5  1996/04/23 12:12:46  craigs
 * Changed to use GetErrorText function
 *
 * Revision 1.4  1996/04/16 13:20:43  craigs
 * Final version prior to beta1 release
 *
 * Revision 1.3  1996/04/15 09:05:30  craigs
 * Latest version prior to integration with Robert's changes
 *
 * Revision 1.2  1996/04/01 12:36:12  craigs
 * Fixed RCS header, added IPAddress functions
 *
 * Revision 1.1  1996/03/02 06:49:51  craigs
 * Initial revision
 *
 */

#ifndef _PSNMP_H
#define _PSNMP_H

#ifdef __GNUC__
#pragma interface
#endif

#include <sockets.h>
#include "pasn.h"


//////////////////////////////////////////////////////////////////////////
//
//  PSNMPVarBindingList
//     A list of object IDs and their values
//

PDECLARE_CLASS (PSNMPVarBindingList, PObject)
  public:

    void Append(const PString & objectID);
    void Append(const PString & objectID, PASNObject * obj);
    void AppendString(const PString & objectID, const PString & str);

    void RemoveAll();

    PINDEX GetSize() const;

    PString GetObjectID(PINDEX idx) const;
    PASNObject & operator[](PINDEX idx) const;

    void PrintOn(ostream & strm) const;

  protected:
    PStringList     objectIds;
    PASNObjectList  values;
};

//////////////////////////////////////////////////////////////////////////
//
//  PSNMP
//     A descendant of PUDPSocket which can perform SNMP calls
//

PDECLARE_CLASS(PSNMP, PIndirectChannel)
  public:
    enum ErrorType {
       // Standard RFC1157 errors
       NoError        = 0,
       TooBig         = 1,
       NoSuchName     = 2,
       BadValue       = 3,
       ReadOnly       = 4,
       GenErr         = 5,

       // Additional errors
       NoResponse,
       MalformedResponse,
       SendFailed,
       NumErrors
    };

    enum RequestType {
       GetRequest     = 0,
       GetNextRequest = 1,
       GetResponse    = 2,
       SetRequest     = 3,
       Trap           = 4,
    };

    enum { TrapPort = 162 };

    enum TrapType {
      ColdStart             = 0,
      WarmStart             = 1,
      LinkDown              = 2,
      LinkUp                = 3,
      AuthenticationFailure = 4,
      EGPNeighbourLoss      = 5,
      EnterpriseSpecific    = 6,
      NumTrapTypes
    };

    static PString GetErrorText(ErrorType err);

    static PString GetTrapTypeText(PINDEX code);

    static void SendEnterpriseTrap (
                 const PIPSocket::Address & addr,
                            const PString & community,
                            const PString & enterprise,
                                     PINDEX specificTrap,
                               PASNUnsigned timeTicks,
                                       WORD sendPort = TrapPort);

    static void SendEnterpriseTrap (
                 const PIPSocket::Address & addr,
                            const PString & community,
                            const PString & enterprise,
                                     PINDEX specificTrap,
                               PASNUnsigned timeTicks,
                const PSNMPVarBindingList & vars,
                                       WORD sendPort = TrapPort);

    static void SendTrap (
                       const PIPSocket::Address & addr,
                                  PSNMP::TrapType trapType,
                                  const PString & community,
                                  const PString & enterprise,
                                           PINDEX specificTrap,
                                     PASNUnsigned timeTicks,
                      const PSNMPVarBindingList & vars,
                                             WORD sendPort = TrapPort);

    static void SendTrap (
                      const PIPSocket::Address & addr,
                                  PSNMP::TrapType trapType,
                                  const PString & community,
                                  const PString & enterprise,
                                           PINDEX specificTrap,
                                     PASNUnsigned timeTicks,
                      const PSNMPVarBindingList & vars,
                       const PIPSocket::Address & agentAddress,
                                             WORD sendPort = TrapPort);
                            
    static void WriteTrap (           PChannel & channel,
                                  PSNMP::TrapType trapType,
                                  const PString & community,
                                  const PString & enterprise,
                                           PINDEX specificTrap,
                                     PASNUnsigned timeTicks,
                      const PSNMPVarBindingList & vars,
                       const PIPSocket::Address & agentAddress);

    static BOOL DecodeTrap(const PBYTEArray & readBuffer,
                                       PINDEX & version,
                                      PString & community,
                                      PString & enterprise,
                           PIPSocket::Address & address,
                                       PINDEX & genericTrapType,
                                      PINDEX  & specificTrapType,
                                 PASNUnsigned & timeTicks,
                          PSNMPVarBindingList & varsOut);
};


//////////////////////////////////////////////////////////////////////////
//
//  PSNMPClient
//

PDECLARE_CLASS(PSNMPClient, PSNMP)
  public:
    PSNMPClient(const PString & host,
                PINDEX retryMax = 5,
                PINDEX timeoutMax = 5);

    PSNMPClient(PINDEX retryMax = 5,
                PINDEX timeoutMax = 5);

    void SetVersion(PASNInt version);
    PASNInt GetVersion() const;

    void SetCommunity(const PString & str);
    PString GetCommunity() const;

    void SetRequestID(PASNInt requestID);
    PASNInt GetRequestID() const;

    BOOL WriteGetRequest (PSNMPVarBindingList & varsIn,
                          PSNMPVarBindingList & varsOut);

    BOOL WriteGetNextRequest (PSNMPVarBindingList & varsIn,
                              PSNMPVarBindingList & varsOut);

    BOOL WriteSetRequest (PSNMPVarBindingList & varsIn,
                          PSNMPVarBindingList & varsOut);

    ErrorType GetLastErrorCode() const;
    PINDEX    GetLastErrorIndex() const;
    PString   GetLastErrorText() const;

  protected:
    BOOL WriteRequest (PASNInt requestCode,
                       PSNMPVarBindingList & varsIn,
                       PSNMPVarBindingList & varsOut);

    PString   hostName;
    PString   community;
    PASNInt   requestId;
    PASNInt   version;
    PINDEX    retryMax;
    PINDEX    lastErrorIndex;
    ErrorType lastErrorCode;
};


//////////////////////////////////////////////////////////////////////////
//
//  PSNMPServer
//

PDECLARE_CLASS(PSNMPServer, PSNMP)
  public:

    virtual void OnGetRequest     (PSNMPVarBindingList & vars);
    virtual void OnGetNextRequest (PSNMPVarBindingList & vars);
    virtual void OnSetRequest     (PSNMPVarBindingList & vars);

    BOOL SendGetResponse          (PSNMPVarBindingList & vars);
};

#endif
