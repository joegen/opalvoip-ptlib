/*
 * $Id: socket.h,v 1.2 1995/01/23 22:59:51 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: socket.h,v $
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

#include <fcntl.h>
#include <termio.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>

#if defined (P_SUN4)
extern "C" int socket(int, int, int);
extern "C" int connect(int s, struct sockaddr *name, int namelen);
extern "C" int ioctl(int, int, void *);
extern "C" int send(int s, const void *buf, int len, int flags);
extern "C" int recv(int s, void *buf, int len, int flags);
#endif


///////////////////////////////////////////////////////////////////////////////
// PSocket

#include "../../common/socket.h"
};

#endif
