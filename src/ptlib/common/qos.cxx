/*
 * qos.cxx
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
 * $Log: qos.cxx,v $
 * Revision 1.1  2003/10/27 03:20:10  csoutheren
 * Initial version of QoS implementation
 *   Thanks to Henry Harrison of AliceStreet
 *
 *
 */


#include <ptlib.h>
#include <ptlib/qos.h>

char PQoS::bestEffortDSCP = 0;
char PQoS::controlledLoadDSCP = 26;
char PQoS::guaranteedDSCP = 46;


PQoS::PQoS() 
{
    ServiceType = SERVICETYPE_PNOTDEFINED;
    dscp = -1;
    TokenRate = QOS_NOT_SPECIFIED;
    TokenBucketSize = QOS_NOT_SPECIFIED;
    PeakBandwidth = QOS_NOT_SPECIFIED;
}

PQoS::PQoS(int DSCPvalue) 
{
    ServiceType = SERVICETYPE_PNOTDEFINED;
    dscp = DSCPvalue;
    TokenRate = QOS_NOT_SPECIFIED;
    TokenBucketSize = QOS_NOT_SPECIFIED;
    PeakBandwidth = QOS_NOT_SPECIFIED;
}

PQoS::PQoS(DWORD avgBytesPerSec, 
           DWORD winServiceType,
           int DSCPalternative, 
           DWORD maxFrameBytes, 
           DWORD peakBytesPerSec)
{
    TokenRate = avgBytesPerSec;
    ServiceType = winServiceType;
    dscp = DSCPalternative;
    TokenBucketSize = maxFrameBytes;
    PeakBandwidth = peakBytesPerSec;
}

void PQoS::SetWinServiceType(DWORD winServiceType)
{
    ServiceType = winServiceType;
}

void PQoS::SetAvgBytesPerSec(DWORD avgBytesPerSec)
{
    TokenRate = avgBytesPerSec;
}

void PQoS::SetDSCP(int DSCPvalue)
{
    if (DSCPvalue < 63)
        dscp = DSCPvalue;
}

void PQoS::SetMaxFrameBytes(DWORD maxFrameBytes)
{
    TokenBucketSize = maxFrameBytes;
}

void PQoS::SetPeakBytesPerSec(DWORD peakBytesPerSec)
{
    PeakBandwidth = peakBytesPerSec;
}


void PQoS::SetDSCPAlternative(DWORD winServiceType, UINT dscp)
{
    if (dscp > -1 &&
        dscp < 63 &&
        winServiceType != SERVICETYPE_PNOTDEFINED)
    {
        switch (winServiceType)
        {
        case SERVICETYPE_BESTEFFORT:
             BestEffortDSCP = (char)dscp;
            break;
        case SERVICETYPE_CONTROLLEDLOAD:
            ControlledLoadDSCP = (char)dscp;
            break;
        case SERVICETYPE_GUARANTEED:
            GuaranteedDSCP = (char)dscp;
            break;
        }
    }
}
