/*
 * $Id: ethsock.h,v 1.1 1998/08/21 05:30:13 robertj Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1996 Equivalence Pty. Ltd.
 *
 * $Log: ethsock.h,v $
 * Revision 1.1  1998/08/21 05:30:13  robertj
 * Initial revision
 *
 */

#ifndef _PETHSOCKET

#pragma interface


///////////////////////////////////////////////////////////////////////////////
// PEthSocket

#include "../../common/ptlib/ethsock.h"
  protected:
    Address     macAddress;
    MediumTypes medium;
    unsigned    filterMask;
    size_t      skipBytes;
    size_t      padBytes;
};

#endif
