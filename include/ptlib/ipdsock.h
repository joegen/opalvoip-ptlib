/*
 * ipdsock.h
 *
 * IP Datagram socket I/O channel class.
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
 * $Log: ipdsock.h,v $
 * Revision 1.4  1998/11/14 06:28:09  robertj
 * Fixed error in documentation
 *
 * Revision 1.3  1998/09/23 06:20:43  robertj
 * Added open source copyright license.
 *
 * Revision 1.2  1996/09/14 13:09:20  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.1  1996/05/15 21:11:16  robertj
 * Initial revision
 *
 */

#define _PIPDATAGRAMSOCKET

#ifdef __GNUC__
#pragma interface
#endif

PDECLARE_CLASS(PIPDatagramSocket, PIPSocket)
  protected:
    PIPDatagramSocket();
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


  public:
  // New functions for class
    virtual BOOL ReadFrom(
      void * buf,     // Data to be written as URGENT TCP data.
      PINDEX len,     // Number of bytes pointed to by <CODE>buf</CODE>.
      Address & addr, // Address from which the datagram was received.
      WORD & port     // Port from which the datagram was received.
    );
    /* Read a datagram from a remote computer.
       
       <H2>Returns:</H2>
       TRUE if any bytes were sucessfully read.
     */

    virtual BOOL WriteTo(
      const void * buf,   // Data to be written as URGENT TCP data.
      PINDEX len,         // Number of bytes pointed to by <CODE>buf</CODE>.
      const Address & addr, // Address to which the datagram is sent.
      WORD port           // Port to which the datagram is sent.
    );
    /* Write a datagram to a remote computer.

       <H2>Returns:</H2>
       TRUE if all the bytes were sucessfully written.
     */


// Class declaration continued in platform specific header file ///////////////
