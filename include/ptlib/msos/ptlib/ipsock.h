/*
 * ipsock.h
 *
 * Internet Protocol socket I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: ipsock.h,v $
 * Revision 1.4  1998/09/24 03:30:09  robertj
 * Added open software license.
 *
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
