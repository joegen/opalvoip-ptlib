/*
 * $Id: serchan.h,v 1.5 1995/03/12 04:59:59 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: serchan.h,v $
 * Revision 1.5  1995/03/12 04:59:59  robertj
 * Re-organisation of DOS/WIN16 and WIN32 platforms to maximise common code.
 * Used built-in equate for WIN32 API (_WIN32).
 *
 * Revision 1.4  1994/08/04  13:08:43  robertj
 * Added DCB so can set parameters on closed channel.
 *
 * Revision 1.3  1994/07/17  11:01:04  robertj
 * Ehancements, implementation, bug fixes etc.
 *
 * Revision 1.2  1994/07/02  03:18:09  robertj
 * Using system timers for serial channel timeouts.
 *
 * Revision 1.1  1994/06/25  12:13:01  robertj
 * Initial revision
 *
 */


#ifndef _PSERIALCHANNEL

#include "..\..\common\serchan.h"
  public:
    // Overrides from class PChannel
    virtual PString GetName() const;
      // Return the name of the channel.

      
    virtual BOOL Read(void * buf, PINDEX len);
      // Low level read from the channel. This function will block until the
      // requested number of characters were read.

    virtual BOOL Write(const void * buf, PINDEX len);
      // Low level write to the channel. This function will block until the
      // requested number of characters were written.

    virtual BOOL Close();
      // Close the channel.


  private:
    BOOL SetCommsParam(DWORD speed, BYTE data, Parity parity,
                     BYTE stop, FlowControl inputFlow, FlowControl outputFlow);


  // Member variables
#if defined(_WIN32)
    HANDLE  commsResource;
    PString portName;
#else
    static BOOL IsReadBlocked(PObject * obj);
    static BOOL IsWriteBlocked(PObject * obj);

    PTimer readTimer;
    PTimer writeTimer;
#endif

#if defined(_WINDOWS) || defined(_WIN32)
    enum { InputQueueSize = 2048, OutputQueueSize = 1024 };
    DCB deviceControlBlock;
#else
    BYTE biosParm;
#endif
};


#endif
