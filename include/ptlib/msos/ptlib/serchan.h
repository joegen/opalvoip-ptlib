/*
 * $Id: serchan.h,v 1.2 1994/07/02 03:18:09 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.2  1994/07/02 03:18:09  robertj
 * Using system timers for serial channel timeouts.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 */


#ifndef _PSERIALCHANNEL

#include "..\..\common\serchan.h"
  protected:
    BOOL SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow);

    // Meber variables
    int commsId;

  private:
    static BOOL IsReadBlocked(PObject * obj);
    static BOOL IsWriteBlocked(PObject * obj);

    PTimer readTimer;
    PTimer writeTimer;
#ifdef _WINDOWS
    enum { InputQueueSize = 2048, OutputQueueSize = 1024 };
#else
    BYTE biosParm;
#endif
};


#endif
