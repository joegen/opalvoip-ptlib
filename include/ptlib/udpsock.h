/*
 * $Id: udpsock.h,v 1.5 1995/01/03 09:36:24 robertj Exp $
 *
 * Portable Windows Library
 *
 * UDP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: udpsock.h,v $
 * Revision 1.5  1995/01/03 09:36:24  robertj
 * Documentation.
 *
 * Revision 1.4  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.3  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PUDPSOCKET

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PUDPSocket, PIPSocket)
/* Create a socket checnnel that uses the UDP transport on the Internal
   Protocol.
 */


// Class declaration continued in platform specific header file ///////////////
