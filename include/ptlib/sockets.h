/*
 * $Id: sockets.h,v 1.9 1996/10/08 13:05:38 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 by Robert Jongbloed and Craig Southeren
 *
 * $Log: sockets.h,v $
 * Revision 1.9  1996/10/08 13:05:38  robertj
 * More IPX support.
 *
 * Revision 1.8  1996/09/14 13:09:44  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.7  1996/08/08 10:08:53  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.6  1996/05/15 10:13:15  robertj
 * Added ICMP protocol socket, getting common ancestor to UDP.
 *
 * Revision 1.5  1995/06/04 12:36:55  robertj
 * Added application layer protocol sockets.
 *
 * Revision 1.4  1994/08/23 11:32:52  robertj
 * Oops
 *
 * Revision 1.3  1994/08/22  00:46:48  robertj
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

#ifdef __GNUC__
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PSocket

#include <socket.h>


///////////////////////////////////////////////////////////////////////////////
// PIPSocket

#include <ipsock.h>


///////////////////////////////////////////////////////////////////////////////
// PIPDatagramSocket

#include <ipdsock.h>


///////////////////////////////////////////////////////////////////////////////
// PUDPSocket

#include <udpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PICMPSocket

#include <icmpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#include <tcpsock.h>


#ifdef PIPX

///////////////////////////////////////////////////////////////////////////////
// PIPXSocket

#include <ipxsock.h>


///////////////////////////////////////////////////////////////////////////////
// PSPXSocket

#include <spxsock.h>

#endif // PIPX

#endif // _SOCKETS_H


// End Of File ///////////////////////////////////////////////////////////////
