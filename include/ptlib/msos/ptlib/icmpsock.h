/*
 * $Id: icmpsock.h,v 1.3 1996/10/29 13:28:30 robertj Exp $
 *
 * Portable Windows Library
 *
 * IP Datagram Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: icmpsock.h,v $
 * Revision 1.3  1996/10/29 13:28:30  robertj
 * Change ICMP to use DLL rather than Winsock
 *
 *
 */

#ifndef _PICMPSOCKET

#include "../../common/ptlib/icmpsock.h"

  public:
    BOOL Close();
    BOOL IsOpen() const;

  protected:
    HANDLE icmpHandle;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
