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
 * Revision 1.7  1998/09/23 06:20:39  robertj
 * Added open source copyright license.
 *
 * Revision 1.6  1998/01/26 00:30:41  robertj
 * Added error codes, TTL and data buffer to Ping.
 *
 * Revision 1.5  1997/02/05 11:52:07  robertj
 * Changed current process function to return reference and validate objects descendancy.
 *
 * Revision 1.4  1996/11/04 03:57:16  robertj
 * Rewrite of ping for Win32 support.
 *
 * Revision 1.3  1996/09/14 13:09:19  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.2  1996/06/03 10:03:22  robertj
 * Changed ping to return more parameters.
 *
 * Revision 1.1  1996/05/15 21:11:16  robertj
 * Initial revision
 *
 */

#define _PICMPSOCKET

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PICMPSocket, PIPDatagramSocket)
/* Create a socket channel that uses allows ICMP commands in the Internal
   Protocol.
 */

  public:
    PICMPSocket();
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


    enum PingStatus {
      Success,
      NetworkUnreachable,
      HostUnreachable,
      PacketTooBig,
      RequestTimedOut,
      BadRoute,
      TtlExpiredTransmit,
      TtlExpiredReassembly,
      SourceQuench,
      MtuChange,
      GeneralError,
      NumStatuses
    };

    class PingInfo {
      public:
        PingInfo(WORD id = (WORD)PProcess::Current().GetProcessID());

        // Supplied data
        WORD identifier;         // Arbitrary identifier for the ping.
        WORD sequenceNum;        // Sequence number for ping packet.
        BYTE ttl;                // Time To Live for packet.
        const BYTE * buffer;     // Send buffer (if NULL, defaults to 32 bytes).
        PINDEX bufferSize;       // Size of buffer (< 64k).

        // Returned data
        PTimeInterval delay;     // Time for packet to make trip.
        Address remoteAddr;      // Source address of reply packet.
        Address localAddr;       // Destination address of reply packet.
        PingStatus status;       // Status of the last ping operation
    };

    BOOL Ping(
      const PString & host   // Host to send ping.
    );
    BOOL Ping(
      const PString & host,   // Host to send ping.
      PingInfo & info         // Information on the ping and reply.
    );
    /* Send an ECHO_REPLY message to the specified host and wait for a reply
       to be sent back.

       <H2>Returns:</H2>
       FALSE if host not found or no response.
     */


  protected:
    const char * GetProtocolName() const;
    virtual BOOL OpenSocket();


// Class declaration continued in platform specific header file ///////////////





