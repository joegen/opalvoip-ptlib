////////////////////////////////////////////////////////
//
//  PTelnetSocket
//
////////////////////////////////////////////////////////

#ifndef _PTELNETSOCKET

#define _PTELNETSOCKET

PDECLARE_CLASS(PTelnetSocket, PTCPSocket)

  public:
    // the default port number
    enum {
      DefaultPort = 23
    };

    PTelnetSocket();
      // create an unopened socket

    PTelnetSocket(const PString address, int port = DefaultPort);
      // create an opened socket

    // Overrides from class PTCPSocket
    BOOL Open (const PString address, int port = DefaultPort);
      // connect to a telnet server

    // Overrides from class PChannel
    BOOL Read (void * data, PINDEX len);
      // read data from a telnet port

    int ReadChar();
      // read a character from a telnet port

    // New functions
    virtual BOOL OnUnknownCommand(BYTE code);
      // Received unknown telnet command. Return TRUE if next byte
      // is not part of the unknown command

    virtual void OnDo (BYTE code);
      // Received DO request

    virtual void OnWill (BYTE code);
      // Received WILL request

    virtual void SendWill(BYTE code);
      // Send WILL request

    virtual void SendWont(BYTE code);
      // Send WONT request

    virtual void SendDo(BYTE code);
      // Send DO request

    virtual void SendDont(BYTE code);
      // Send DONT request

  protected:
    // defined telnet commands
    enum Command {
      SE   	= 240,		// subnegotiation end
      NOP  	= 241,		// no operation
      DataMark 	= 242,		// data stream portion of a Synch
      Break	= 243,		// NVT character break
      Interrupt = 244,          // The function IP
      Abort     = 245,          // The function AO
      AreYouThere = 246,        // The function AYT
      EraseChar = 247,          // The function EC
      EraseLine = 248,		// The function EL
      GoAhead   = 249,          // The function GA
      SB        = 250,          // subnegotiation start
      WILL      = 251,          // begin option
      WONT      = 252,          // refuse option
      DO        = 253,          // request option
      DONT      = 254,		// stop option
      IAC       = 255           // Escape
    };

    // internal states for the Telnet decoder
    enum State {
      StateNormal,
      StateIAC,
      StateDo,
      StateWill,
      StateUnknownCommand,
    };

    void Construct ();
      // common construct code for telnet socket

    // internal storage for the Telnet decoder
    State  state;

    BOOL   debug;

};

#endif
