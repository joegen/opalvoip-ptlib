/*
 * $Id: icmpsock.h,v 1.1 1996/05/15 21:11:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * ICMP Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: icmpsock.h,v $
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


    BOOL Ping(
      const PString & host,   // Host to send ping.
      PTimeInterval & delay   // Time for packet to make trip.
    );
    /* Send an ECHO_REPLY message to the specified host and wait for a reply
       to be sent back.

       <H2>Returns:</H2>
       FALSE if host not found or no response.
     */


    BOOL WritePing(
      const PString & host,   // Host to send ping.
      WORD sequence = 0       // Sequence number for multiple pinging.
    );
    /* Send an ECHO_REPLY message to the specified host.

       <H2>Returns:</H2>
       FALSE if host not found or no response.
     */

    BOOL ReadPing(
      Address & addr,
      WORD & sequenceNum,
      PTimeInterval & delay
    );
    /* Receive an ECHO_REPLY message from the host.

       <H2>Returns:</H2>
       FALSE if an error occurred.
     */



  private:
    WORD GetPortByService(const PString & serviceName) const;
    PString GetServiceByPort(WORD port) const;


// Class declaration continued in platform specific header file ///////////////
