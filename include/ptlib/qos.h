/*
 * qos.h
 *
 * QOS class used by PWLIB dscp or Windows GQOS implementation.
 *
 * Copyright (c) 2003 AliceStreet Ltd
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
 * Contributor(s): ______________________________________.
 *
 * $Log: qos.h,v $
 * Revision 1.1  2003/10/27 03:20:10  csoutheren
 * Initial version of QoS implementation
 *   Thanks to Henry Harrison of AliceStreet
 *
 *
 */

#ifndef	_PQOS
#define _PQOS

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#ifdef _WIN32
#include <qos.h>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <qossp.h>
#endif

#ifndef QOS_NOT_SPECIFIED
#define QOS_NOT_SPECIFIED 0xFFFFFFFF
#endif

#ifndef SERVICETYPE
#define SERVICETYPE DWORD
#endif

#ifndef SERVICETYPE_GUARANTEED
#define SERVICETYPE_GUARANTEED 0x00000003
#endif

#ifndef SERVICETYPE_CONTROLLEDLOAD
#define SERVICETYPE_CONTROLLEDLOAD 0x00000002
#endif

#ifndef SERVICETYPE_BESTEFFORT
#define SERVICETYPE_BESTEFFORT 0x00000001
#endif

#define SERVICETYPE_PNOTDEFINED 0xFFFFFFFF

class PQoS : public PObject
{
    PCLASSINFO(PQoS, PObject);

public:
    PQoS();
    PQoS(DWORD avgBytesPerSec,
         DWORD winServiceType,
         int DSCPalternative = -1,
         DWORD maxFrameBytes = 1500,
         DWORD peakBytesPerSec = QOS_NOT_SPECIFIED);
    PQoS(int DSCPvalue);

    void SetAvgBytesPerSec(DWORD avgBytesPerSec);
    void SetWinServiceType(DWORD winServiceType);
    void SetDSCP(int DSCPvalue);
    void SetMaxFrameBytes(DWORD maxFrameBytes);
    void SetPeakBytesPerSec(DWORD peakBytesPerSec);

    DWORD GetTokenRate() const       { return tokenRate;}
    DWORD GetTokenBucketSize() const { return tokenBucketSize;}
    DWORD GetPeakBandwidth() const   { return peakBandwidth;}
    DWORD GetServiceType() const     { return serviceType;}
    int GetDSCP() const              { return dscp;}

    static void SetDSCPAlternative(DWORD winServiceType,
                                   UINT dscp);
    static char bestEffortDSCP;
    static char controlledLoadDSCP;
    static char guaranteedDSCP;

  protected:
    int dscp;
    DWORD tokenRate;
    DWORD tokenBucketSize;
    DWORD peakBandwidth;
    DWORD serviceType;

};

#endif // _PQOS
