/*
 * $Id: udpsock.h,v 1.3 1994/08/22 00:46:48 robertj Exp $
 *
 * Portable Windows Library
 *
 * UDP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: udpsock.h,v $
 * Revision 1.3  1994/08/22 00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#define _PUDPSOCKET

#ifdef __GNU__
#pragma interface
#endif


PDECLARE_CLASS(PUDPSocket, PIPSocket)

// Class declaration continued in platform specific header file ///////////////
