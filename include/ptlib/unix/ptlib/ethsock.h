/*
 * ethsock.h
 *
 * Direct ethernet socket I/O channel class.
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
 * $Log: ethsock.h,v $
 * Revision 1.5  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.4  1998/11/30 22:06:39  robertj
 * New directory structure.
 *
 * Revision 1.3  1998/09/24 04:11:34  robertj
 * Added open software license.
 *
 * Revision 1.2  1998/09/08 09:53:56  robertj
 * Fixed ppp and ippp compatibility.
 *
 * Revision 1.1  1998/08/21 05:30:13  robertj
 * Initial revision
 *
 */

#ifndef _PETHSOCKET

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PEthSocket

#define _PETHSOCKET_PLATFORM_INCLUDE
#include "../../ethsock.h"

#endif
#ifdef _PETHSOCKET_PLATFORM_INCLUDE
#undef _PETHSOCKET_PLATFORM_INCLUDE

  protected:
    Address     macAddress;
    MediumTypes medium;
    unsigned    filterMask;
    BOOL        fakeMacHeader;
    BOOL        ipppInterface;

#endif


// End Of File ////////////////////////////////////////////////////////////////
