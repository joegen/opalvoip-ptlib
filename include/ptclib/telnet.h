/*
 * $Id: telnet.h,v 1.8 1995/01/03 09:36:23 robertj Exp $
 *
 * Portable Windows Library
 *
 * Telnet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: telnet.h,v $
 * Revision 1.8  1995/01/03 09:36:23  robertj
 * Documentation.
 *
 * Revision 1.7  1995/01/01  01:07:33  robertj
 * More implementation.
 *
 * Revision 1.6  1994/11/28  12:38:59  robertj
 * Added DONT and WONT states.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Changed type of socket port number for better portability.
 *
 * Revision 1.2  1994/07/25  03:36:03  robertj
 * Added sockets to common, normalising to same comment standard.
 *
 */

#ifndef _PTELNETSOCKET
#define _PTELNETSOCKET

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PTelnetSocket, PTCPSocket)
/* A TCP/IP socket for the TELNET high level protocol.
 */

  public:
    enum {
      DefaultPort = 23
    };
    // The default port number for a TELNET connection.

    PTelnetSocket(
      WORD port = DefaultPort   // Port number to use for the connection.
    );
    // Create an unopened TELNET socket.

    PTelnetSocket(
      const PString & address,  // Address of remote machine to connect to.
      WORD port = DefaultPort   // Port number to use for the connection.
    );
    // Create an opened TELNET socket.


  // Overrides from class PChannel
    BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       The TELNET channel intercepts and escapes commands in the data stream to
       implement the TELNET protocol.

       Returns: TRUE indicates that at least one character was read from the
                channel. FALSE means no bytes were read due to timeout or
                some other I/O error.
     */

    BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       The TELNET channel intercepts and escapes commands in the data stream to
       implement the TELNET protocol.

       Returns TRUE if at least len bytes were written to the channel.
     */


    virtual void OnOutOfBand(
      const void * buf,   // Data to be received as URGENT TCP data.
      PINDEX len          // Number of bytes pointed to by $B$buf$B$.
    );
    /* This is callback function called by the system whenever out of band data
       from the TCP/IP stream is received. A descendent class may interpret
       this data according to the semantics of the high level protocol.

       The TELNET socket uses this for sychronisation.
     */

  // New functions
    enum Options {
      ExtendedOptionsList = 255,  // Code for extended options.
      TransmitBinary      = 0,    // Assume binary 8 bit data is transferred.
      Echo                = 1,    // Automatically echo characters sent.
      SuppressGoAhead     = 3,    // Do not use the GA protocol.
      Status              = 5,    // Status packets are understood.
      TimingMark          = 6,    // Marker for synchronisation.
      MaxOptions
    };
    // Defined TELNET options.


    virtual BOOL OnUnknownCommand(
      BYTE code  // Code received that could not be precessed.
    );
    /* This callback function is called by the system when it receives an
       unknown telnet command.
       
       The default action displays a message to the $H$PError stream and
       returns TRUE;

       Returns: TRUE if next byte is not part of the unknown command.
     */

    virtual void OnDo(
      BYTE option   // Option to DO
    );
    /* This callback function is called by the system when it receives a DO
       request from the remote system.
       
       The default action is to send a WILL for options that are understood by
       the standard TELNET class and a WONT for all others.
     */

    virtual void OnDont(
      BYTE option   // Option to DONT
    );
    /* This callback function is called by the system when it receives a DONT
       request from the remote system.
       
       The default action is to disable options that are understood by the
       standard TELNET class. All others are ignored.
     */

    virtual void OnWill(
      BYTE option   // Option to WILL
    );
    /* This callback function is called by the system when it receives a WILL
       request from the remote system.
       
       The default action is to send a DO for options that are understood by
       the standard TELNET class and a DONT for all others.
     */

    virtual void OnWont(
      BYTE option   // Option to WONT
    );
    /* This callback function is called by the system when it receives a WONT
       request from the remote system.

       The default action is to disable options that are understood by the
       standard TELNET class. All others are ignored.
     */

    virtual void SendDo(
      BYTE option   // Option to DO
    );
    // Send DO request.

    virtual void SendDont(
      BYTE option   // Option to DONT
    );
    // Send DONT command.

    virtual void SendWill(
      BYTE option   // Option to WILL
    );
    // Send WILL request.

    virtual void SendWont(
      BYTE option   // Option to WONT
    );
    // Send WONT command.


    enum Command {
      SE        = 240,    // subnegotiation end
      NOP       = 241,    // no operation
      DataMark  = 242,    // data stream portion of a Synch
      Break     = 243,    // NVT character break
      Interrupt = 244,    // The function IP, Interrupt Process
      Abort     = 245,    // The function AO, Abort Output
      AreYouThere = 246,  // The function AYT, Are You There
      EraseChar = 247,    // The function EC, Erase Character
      EraseLine = 248,    // The function EL, Erase Line
      GoAhead   = 249,    // The function GA, Go Ahead
      SB        = 250,    // subnegotiation start
      WILL      = 251,    // begin option
      WONT      = 252,    // refuse option
      DO        = 253,    // request option
      DONT      = 254,    // stop option
      IAC       = 255     // Escape
    };
    // Defined telnet commands codes

    void SendDataMark(
      Command cmd   // Command to synchronise on.
    );
    /* Synchronises the TELNET streams, inserts the data mark into outgoing
       data stream and sends an out of band data to the remote to flush all
       data in the stream up until the syncronisation command.
     */


  protected:
    void Construct();
    // Common construct code for TELNET socket channel.


  // Member variables.
    enum State {
      StateNormal,
      StateIAC,
      StateDo,
      StateDont,
      StateWill,
      StateWont,
      StateSubNegotiations
    };
    // Internal states for the TELNET decoder

    State state;
    // Current state of incoming characters.

    BOOL willOptions[MaxOptions];
    // WILL options that are enabled (outgoing data).

    BOOL doOptions[MaxOptions];
    // DO options that are enabled (incoming data).


    BOOL debug;
    // Debug socket, output messages to PError stream.
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
