/*
 * $Id: socket.h,v 1.10 1996/11/03 04:36:25 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.10  1996/11/03 04:36:25  craigs
 * Added Read override to avoid problem with recv/read
 *
 * Revision 1.9  1996/08/09 12:16:09  craigs
 * *** empty log message ***
 *
 * Revision 1.8  1996/08/03 12:09:51  craigs
 * Changed for new common directories
 *
 * Revision 1.7  1996/05/03 13:12:07  craigs
 * More Sun4 fixes
 *
 * Revision 1.6  1996/05/02 13:34:22  craigs
 * More Sun4 fixes
 *
 * Revision 1.5  1996/05/02 12:28:03  craigs
 * More Sun4 fixes
 *
 * Revision 1.4  1996/01/26 11:06:31  craigs
 * Added destructor
 *
 * Revision 1.3  1995/12/08 13:15:21  craigs
 * Added new header file
 *
 * Revision 1.2  1995/01/23 22:59:51  craigs
 * Changes for HPUX and Sun 4
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PSOCKET

#define	P_HAS_BERKELEY_SOCKETS

#pragma interface

#include <netinet/in.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/time.h>

///////////////////////////////////////////////////////////////////////////////
// PSocket

#include "../../common/ptlib/socket.h"
  public:
    BOOL Read(void * ptr, PINDEX len);
    ~PSocket();
};

#endif
