/*
 * $Id: sockets.h,v 1.3 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: sockets.h,v $
 * Revision 1.3  1994/08/22 00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.2  1994/08/21  23:43:02  robertj
 * Added telnet.
 *
 * Revision 1.1  1994/07/25  03:36:03  robertj
 * Initial revision
 *
 * Revision 1.3  1994/07/21  12:17:41  robertj
 * Sockets.
 *
 * Revision 1.2  1994/06/25  12:27:39  robertj
 * *** empty log message ***
 *
 * Revision 1.1  1994/04/01  14:38:42  robertj
 * Initial revision
 *
 */

#ifndef _SOCKETS_H
#define _SOCKETS_H

#ifdef __GNU__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PSocket

#include <socket.h>


///////////////////////////////////////////////////////////////////////////////
// PIPSocket

#include <ipsock.h>


///////////////////////////////////////////////////////////////////////////////
// PUDPSocket

#include <udpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#include <tcpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PTelnetSocket

#include <telnet.h>


#endif // _SOCKETS_H


// End Of File ///////////////////////////////////////////////////////////////
