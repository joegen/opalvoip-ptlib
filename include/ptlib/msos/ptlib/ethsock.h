/*
 * $Id: ethsock.h,v 1.1 1998/08/20 06:04:29 robertj Exp $
 *
 * Portable Windows Library
 *
 * Ethernet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ethsock.h,v $
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
    ~PEthSocket();
      // close a socket

    virtual BOOL Read(void * buf, PINDEX len);
    virtual BOOL Write(const void * buf, PINDEX len);
    virtual BOOL Close();

  protected:
    PWin32PacketDriver * driver;
    PWin32SnmpLibrary  * snmp;
    PString              interfaceName;
    PINDEX               numBuffers;
    PWin32PacketBuffer * buffers;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
