/*
 * $Id: telnet.h,v 1.11 1995/03/18 06:27:50 robertj Exp $
 *
 * Portable Windows Library
 *
 * Telnet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: telnet.h,v $
 * Revision 1.11  1995/03/18 06:27:50  robertj
 * Rewrite of telnet socket protocol according to RFC1143.
 *
 * Revision 1.10  1995/03/14  12:42:47  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.9  1995/02/21  11:25:33  robertj
 * Further implementation of telnet socket, feature complete now.
 *
 * Revision 1.8  1995/01/03  09:36:23  robertj
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
    PTelnetSocket(
      WORD port = 0             // Port number to use for the connection.
    );
    // Create an unopened TELNET socket.

    PTelnetSocket(
      const PString & address,  // Address of remote machine to connect to.
      WORD port = 0             // Port number to use for the connection.
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

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
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


    virtual BOOL Open(
      const PString & address,  // Address of remote machine to connect to.
      WORD port = 0             // Port number to use for the connection.
    );
    /* Open a socket to a remote host on the specified port number. If the
       <CODE>port</CODE> parameter is zero then the port number as defined by
       the object instance construction or the <A><CODE>SetPort()</CODE></A>
       function is used.

       <H2>Returns:</H2>
       TRUE if the checnnel was successfully opened.
     */


    virtual void OnOutOfBand(
      const void * buf,   // Data to be received as URGENT TCP data.
      PINDEX len          // Number of bytes pointed to by <CODE>buf</CODE>.
    );
    /* This is callback function called by the system whenever out of band data
       from the TCP/IP stream is received. A descendent class may interpret
       this data according to the semantics of the high level protocol.

       The TELNET socket uses this for sychronisation.
     */

  // New functions
    enum Command {
      IAC           = 255,    // Interpret As Command - escape character.
      DONT          = 254,    // You are not to use option.
      DO            = 253,    // Request to use option.
      WONT          = 252,    // Refuse use of option.
      WILL          = 251,    // Accept the use of option.
      SB            = 250,    // Subnegotiation begin.
      GoAhead       = 249,    // Function GA, you may reverse the line.
      EraseLine     = 248,    // Function EL, erase the current line.
      EraseChar     = 247,    // Function EC, erase the current character.
      AreYouThere   = 246,    // Function AYT, are you there?
      AbortOutput   = 245,    // Function AO, abort output stream.
      InterruptProcess = 244, // Function IP, interrupt process, permanently.
      Break         = 243,    // NVT character break.
      DataMark      = 242,    // Marker for connection cleaning.
      NOP           = 241,    // No operation.
      SE            = 240,    // Subnegotiation end.
      EndOfReccord  = 239,    // End of record for transparent mode.
      AbortProcess  = 238,    // Abort the entire process
      SuspendProcess= 237,    // Suspend the process.
      EndOfFile     = 236     // End of file marker.
    };
    // Defined telnet commands codes

    void SendCommand(
      Command cmd,  // Command code to send
      int opt = 0  // Option for command code.
    );
    /* Send an escaped IAC command. The <CODE>opt</CODE> parameters meaning
       depends on the command being sent:

       <DL>
       <DT>DO, DONT, WILL, WONT    <DD><CODE>opt</CODE> is Options code.

       <DT>AbortOutput             <DD>TRUE is flush buffer.

       <DT>InterruptProcess,
          Break, AbortProcess,
          SuspendProcess           <DD>TRUE is synchronise.
       </DL>

       Synchronises the TELNET streams, inserts the data mark into outgoing
       data stream and sends an out of band data to the remote to flush all
       data in the stream up until the syncronisation command.
     */


    enum Options {
      TransmitBinary      = 0,    // Assume binary 8 bit data is transferred.
      EchoOption          = 1,    // Automatically echo characters sent.
      ReconnectOption     = 2,    // Prepare to reconnect
      SuppressGoAhead     = 3,    // Do not use the GA protocol.
      MessageSizeOption   = 4,    // Negatiate approximate message size
      StatusOption        = 5,    // Status packets are understood.
      TimingMark          = 6,    // Marker for synchronisation.
      RCTEOption          = 7,    // Remote controlled transmission and echo.
      OutputLineWidth     = 8,    // Negotiate about output line width.
      OutputPageSize      = 9,    // Negotiate about output page size.
      CRDisposition       = 10,   // Negotiate about CR disposition.
      HorizontalTabsStops = 11,   // Negotiate about horizontal tabstops.
      HorizTabDisposition = 12,   // Negotiate about horizontal tab disposition
      FormFeedDisposition = 13,   // Negotiate about formfeed disposition.
      VerticalTabStops    = 14,   // Negotiate about vertical tab stops.
      VertTabDisposition  = 15,   // Negotiate about vertical tab disposition.
      LineFeedDisposition = 16,   // Negotiate about output LF disposition.
      ExtendedASCII       = 17,   // Extended ascic character set.
      ForceLogout         = 18,   // Force logout.
      ByteMacroOption     = 19,   // Byte macro.
      DataEntryTerminal   = 20,   // data entry terminal.
      SupDupProtocol      = 21,   // supdup protocol.
      SupDupOutput        = 22,   // supdup output.
      SendLocation        = 23,   // Send location.
      TerminalType        = 24,   // Provide terminal type information.
      EndOfRecordOption   = 25,   // Record boundary marker.
      TACACSUID           = 26,   // TACACS user identification.
      OutputMark          = 27,   // Output marker or banner text.
      TerminalLocation    = 28,   // Terminals physical location infromation.
      Use3270RegimeOption = 29,   // 3270 regime.
      UseX3PADOption      = 30,   // X.3 PAD
      WindowSize          = 31,   // NAWS - Negotiate About Window Size.
      TerminalSpeed       = 32,   // Provide terminal speed information.
      FlowControl         = 33,   // Remote flow control.
      LineModeOption      = 34,   // Terminal in line mode option.
      XDisplayLocation    = 35,   // X Display location.
      EnvironmentOption   = 36,   // Provide environment information.
      AuthenticateOption  = 37,   // Authenticate option.
      EncriptionOption    = 38,	  // Encryption option.
      ExtendedOptionsList = 255,  // Code for extended options.
      MaxOptions
    };
    // Defined TELNET options.


    virtual BOOL SendDo(
      BYTE option    // Option to DO
    );
    // Send DO request.

    virtual BOOL SendDont(
      BYTE option    // Option to DONT
    );
    // Send DONT command.

    virtual BOOL SendWill(
      BYTE option    // Option to WILL
    );
    // Send WILL request.

    virtual BOOL SendWont(
      BYTE option    // Option to WONT
    );
    // Send WONT command.

    void SendSubOption(
      BYTE code,       // Suboptions option code.
      const BYTE * info,  // Information to send.
      PINDEX len          // Length of information.
    );
    // Send a sub-option with the information given.


    void SetOurOption(
      BYTE code,          // Option to check.
      BOOL state = TRUE   // New state for for option.
    ) { option[code].weCan = state; }
    /* Set if the option on our side is possible, this does not mean it is set
       it only means that in response to a DO we WILL rather than WONT.
     */

    void SetTheirOption(
      BYTE code,          // Option to check.
      BOOL state = TRUE  // New state for for option.
    ) { option[code].theyShould = state; }
    /* Set if the option on their side is desired, this does not mean it is set
       it only means that in response to a WILL we DO rather than DONT.
     */

    BOOL IsOurOption(
      BYTE code    // Option to check.
    ) const { return option[code].ourState == OptionInfo::IsYes; }
    /* Determine if the option on our side is enabled.

       <H2>Returns:</H2>
       TRUE if option is enabled.
     */

    BOOL IsTheirOption(
      BYTE code    // Option to check.
    ) const { return option[code].theirState == OptionInfo::IsYes; }
    /* Determine if the option on their side is enabled.

       <H2>Returns:</H2>
       TRUE if option is enabled.
     */

    void SetTerminalType(
      const PString & newType   // New terminal type description string.
    );
    // Set the terminal type description string for TELNET protocol.

    const PString & GetTerminalType() const { return terminalType; }
    // Get the terminal type description string for TELNET protocol.

    void SetWindowSize(
      WORD width,   // New window width.
      WORD height   // New window height.
    );
    // Set the width and height of the Network Virtual Terminal window.

    void GetWindowSize(
      WORD & width,   // Old window width.
      WORD & height   // Old window height.
    ) const;
    // Get the width and height of the Network Virtual Terminal window.


  protected:
    void Construct();
    // Common construct code for TELNET socket channel.

    virtual void OnDo(
      BYTE option   // Option to DO
    );
    /* This callback function is called by the system when it receives a DO
       request from the remote system.
       
       The default action is to send a WILL for options that are understood by
       the standard TELNET class and a WONT for all others.

       <H2>Returns:</H2>
       TRUE if option is accepted.
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

    enum {
      SubOptionIs       = 0,  // Sub-option is...
      SubOptionSend     = 1,  // Request to send option.
    };
    // Codes for sub option negotiation.

    virtual void OnSubOption(
      BYTE code,          // Option code for sub-option data.
      const BYTE * info,  // Extra information being sent in the sub-option.
      PINDEX len          // Number of extra bytes.
    );
    /* This callback function is called by the system when it receives a
       sub-option command from the remote system.
     */


    virtual BOOL OnCommand(
      BYTE code  // Code received that could not be precessed.
    );
    /* This callback function is called by the system when it receives an
       telnet command that it does not do anything with.

       The default action displays a message to the <A>PError</A> stream
       (when <CODE>debug</CODE> is TRUE) and returns TRUE;

       <H2>Returns:</H2>
       TRUE if next byte is not part of the command.
     */


  // Member variables.
    struct OptionInfo {
      enum {
        IsNo, IsYes, WantNo, WantNoQueued, WantYes, WantYesQueued
      };
      unsigned weCan:1;      // We can do the option if they want us to do.
      unsigned ourState:3;
      unsigned theyShould:1; // They should if they will.
      unsigned theirState:3;
    };
    
    OptionInfo option[MaxOptions];
    // Information on protocol options.

    PString terminalType;
    // Type of terminal connected to telnet socket, defaults to "UNKNOWN"

    WORD windowWidth, windowHeight;
    // Size of the "window" used by the NVT.

    BOOL debug;
    // Debug socket, output messages to PError stream.


  private:
    enum State {
      StateNormal,
      StateCarriageReturn,
      StateIAC,
      StateDo,
      StateDont,
      StateWill,
      StateWont,
      StateSubNegotiations,
      StateEndNegotiations
    };
    // Internal states for the TELNET decoder

    State state;
    // Current state of incoming characters.

    PBYTEArray subOption;
    // Storage for sub-negotiated options

    unsigned synchronising;
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
