
/*
 * $Id: ipsock.h,v 1.2 1996/04/15 10:50:48 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.2  1996/04/15 10:50:48  craigs
 * Last revision prior to release of MibMaster
 *
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _IPSOCKET

#pragma interface

#include <arpa/inet.h>
#include <netinet/in.h>


///////////////////////////////////////////////////////////////////////////////
// PIPSocket

#include "../../common/ipsock.h"
};


ostream & operator << (ostream & strm, const PIPSocket::Address & addr);

#endif
