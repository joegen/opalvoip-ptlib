/*
 * $Id: ipsock.h,v 1.3 1997/01/10 13:15:39 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ipsock.h,v $
 * Revision 1.3  1997/01/10 13:15:39  robertj
 * Added unix style error codes for WinSock codes compatible with GetErrorNumber().
 *
 * Revision 1.2  1996/08/08 10:09:04  robertj
 * Directory structure changes for common files.
 *
 * Revision 1.1  1994/07/27 06:00:10  robertj
 * Initial revision
 *
 */

#ifndef _PIPSOCKET


#define EINPROGRESS             (WSAEINPROGRESS|0x40000000)
#define ENOTSOCK                (WSAENOTSOCK|0x40000000)
#define EMSGSIZE                (WSAEMSGSIZE|0x40000000)
#define ESOCKTNOSUPPORT         (WSAESOCKTNOSUPPORT|0x40000000)
#define EOPNOTSUPP              (WSAEOPNOTSUPP|0x40000000)
#define EPFNOSUPPORT            (WSAEPFNOSUPPORT|0x40000000)
#define EAFNOSUPPORT            (WSAEAFNOSUPPORT|0x40000000)
#define EADDRINUSE              (WSAEADDRINUSE|0x40000000)
#define EADDRNOTAVAIL           (WSAEADDRNOTAVAIL|0x40000000)
#define ENETDOWN                (WSAENETDOWN|0x40000000)
#define ENETUNREACH             (WSAENETUNREACH|0x40000000)
#define ENETRESET               (WSAENETRESET|0x40000000)
#define ECONNABORTED            (WSAECONNABORTED|0x40000000)
#define ECONNRESET              (WSAECONNRESET|0x40000000)
#define ENOBUFS                 (WSAENOBUFS|0x40000000)
#define EISCONN                 (WSAEISCONN|0x40000000)
#define ENOTCONN                (WSAENOTCONN|0x40000000)
#define ESHUTDOWN               (WSAESHUTDOWN|0x40000000)
#define ETOOMANYREFS            (WSAETOOMANYREFS|0x40000000)
#define ETIMEDOUT               (WSAETIMEDOUT|0x40000000)
#define ECONNREFUSED            (WSAECONNREFUSED|0x40000000)
#define EHOSTDOWN               (WSAEHOSTDOWN|0x40000000)
#define EHOSTUNREACH            (WSAEHOSTUNREACH|0x40000000)


#include "../../common/ptlib/ipsock.h"
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
