
/*
 * $Id: tcpsock.h,v 1.2 1996/08/03 12:09:51 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: tcpsock.h,v $
 * Revision 1.2  1996/08/03 12:09:51  craigs
 * Changed for new common directories
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PTCPSOCKET

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#include "../../common/ptlib/tcpsock.h"
  public:
    virtual BOOL Read(void * buf, PINDEX len);

};

#endif
