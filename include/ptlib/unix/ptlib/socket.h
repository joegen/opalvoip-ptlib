/*
 * $Id: socket.h,v 1.1 1995/01/23 18:43:27 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
 * Revision 1.1  1995/01/23 18:43:27  craigs
 * Initial revision
 *
 * Revision 1.1  1994/04/12  08:31:05  robertj
 * Initial revision
 *
 */

#ifndef _PSOCKET

#define	P_HAS_BERKELEY_SOCKETS

#pragma interface

#include <sys/ioctl.h>
#include <fcntl.h>
#include <termio.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

///////////////////////////////////////////////////////////////////////////////
// PSocket

#include "../../common/socket.h"
};

#endif
