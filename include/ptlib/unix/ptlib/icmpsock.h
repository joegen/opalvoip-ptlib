
/*
 * $Id: icmpsock.h,v 1.3 1996/10/31 10:27:42 craigs Exp $
 *
 * Portable Windows Library
 *
 * Operating System Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: icmpsock.h,v $
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
