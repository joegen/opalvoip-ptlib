/*
 * sockets.h
 *
 * Berkley Sockets classes.
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
 * $Id$
 */

#ifndef _SOCKETS_H
#define _SOCKETS_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


///////////////////////////////////////////////////////////////////////////////
// PSocket

#include <ptlib/socket.h>


///////////////////////////////////////////////////////////////////////////////
// PIPSocket

#include <ptlib/ipsock.h>


///////////////////////////////////////////////////////////////////////////////
// PIPDatagramSocket

#include <ptlib/ipdsock.h>


///////////////////////////////////////////////////////////////////////////////
// PUDPSocket

#include <ptlib/udpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PICMPSocket

#include <ptlib/icmpsock.h>


///////////////////////////////////////////////////////////////////////////////
// PTCPSocket

#include <ptlib/tcpsock.h>


#ifdef PIPX

///////////////////////////////////////////////////////////////////////////////
// PIPXSocket

#include <ptlib/ipxsock.h>


///////////////////////////////////////////////////////////////////////////////
// PSPXSocket

#include <ptlib/spxsock.h>

#endif // PIPX


///////////////////////////////////////////////////////////////////////////////
// PEthSocket

#include <ptlib/ethsock.h>


#endif // _SOCKETS_H


// End Of File ///////////////////////////////////////////////////////////////
