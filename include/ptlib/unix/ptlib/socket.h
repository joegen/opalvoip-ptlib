/*
 * $Id: socket.h,v 1.6 1996/05/02 13:34:22 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
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

#if 0
#include <fcntl.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#endif

#if defined(P_SUN4)
#include <errno.h>
#endif

///////////////////////////////////////////////////////////////////////////////
// PSocket

#include "../../common/socket.h"
  public:
    ~PSocket();
};

#endif
