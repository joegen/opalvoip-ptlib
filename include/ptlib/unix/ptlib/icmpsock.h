/*
 * icmpsock.h
 *
 * Internet Control Message Protocol socket I/O channel class.
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
 * $Log: icmpsock.h,v $
 * Revision 1.4  1998/09/24 04:11:38  robertj
 * Added open software license.
 *
 * Revision 1.3  1996/10/31 10:27:42  craigs
 * New platform dependent socket implementation
 *
 * Revision 1.2  1996/08/03 12:08:19  craigs
 * Changed for new common directories
 *
 * Revision 1.1  1996/05/25 06:07:23  craigs
 * Initial revision
 *
 */

#ifndef _PICMPSOCKET

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// PICMPSocket

#include "../../common/ptlib/icmpsock.h"
  protected:
    BOOL WritePing(
      const PString & host,   // Host to send ping.
      PingInfo & info         // Information on the ping and reply.
    );
    /* Send an ECHO_REPLY message to the specified host.

       <H2>Returns:</H2>
       FALSE if host not found or no response.
     */

    BOOL ReadPing(
      PingInfo & info         // Information on the ping and reply.
    );
    /* Receive an ECHO_REPLY message from the host.

       <H2>Returns:</H2>
       FALSE if an error occurred.
     */
};

#endif
