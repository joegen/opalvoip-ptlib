/*
 * $Id: ethsock.h,v 1.2 1998/08/21 05:27:01 robertj Exp $
 *
 * Portable Windows Library
 *
 * Ethernet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ethsock.h,v $
 * Revision 1.2  1998/08/21 05:27:01  robertj
 * Fine tuning of interface.
 *
 * Revision 1.1  1998/08/20 06:04:29  robertj
 * Initial revision
 *
 */

#ifndef _PETHSOCKET

class PWin32PacketDriver;
class PWin32SnmpLibrary;
class PWin32PacketBuffer;


#include "../../common/ptlib/ethsock.h"
  public:
  // Overrides from class PChannel
    virtual PString GetName() const;

  protected:
    PWin32PacketDriver * driver;
    PWin32SnmpLibrary  * snmp;
    PString              interfaceName;
    PINDEX               numBuffers;
    PWin32PacketBuffer * buffers;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
