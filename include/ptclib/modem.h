/*
 * $Id: modem.h,v 1.1 1994/06/25 11:55:15 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: modem.h,v $
 * Revision 1.1  1994/06/25 11:55:15  robertj
 * Initial revision
 *
 */


#define _PMODEM


PDECLARE_CLASS(PModem, PSerialChannel)
  // A class representing a modem attached to a serial port.

  public:
    PModem();
      // Create a modem object but do not open it. It does not
      // initially have a valid port name.

    PModem(const PString & port,
           DWORD speed = 0,
           BYTE data = 0,
           Parity parity = DefaultParity,
           BYTE stop = 0,
           FlowControl inputFlow = DefaultFlowControl,
           FlowControl outputFlow = DefaultFlowControl);
      // Open the modem on the serial channel as specified.

    PModem(PConfig & cfg);
      // Open the modem serial channel obtaining the parameters from standard
      // variables in the configuration file. Note that it assumed that the
      // correct configuration file section is already set.


    // Overrides from class PChannel
    virtual BOOL Close();
      // Close the channel.


    // Overrides from class PSerialChannel
    virtual BOOL Open(const PString & port,
                      DWORD speed = 0,
                      BYTE data = 0,
                      Parity parity = DefaultParity,
                      BYTE stop = 0,
                      FlowControl inputFlow = DefaultFlowControl,
                      FlowControl outputFlow = DefaultFlowControl);
      // Open the serial channel on the specified port.

    virtual BOOL Open(PConfig & cfg);
      // Open the modem serial port obtaining the parameters from standard
      // variables in the configuration file. Note that it assumed that the
      // correct configuration file section is already set.

    virtual void SaveSettings(PConfig & cfg);
      // Save the current port settings into the configuration file


    // New member functions
    void SetInitString(const PString & str);
      // Set the modem initialisation meta-command string. Note there is an
      // implied \s before the string.

    PString GetInitString() const;
      // Get the modem initialisation meta-command string.

    BOOL CanInitialise() const;
      // The modem is in a state that allows the initialise to start.

    BOOL Initialise();
      // Send the initialisation meta-command string to the modem. The
      // return value indicates that the conditions for the operation to
      // start were met i.e. the serial port was open etc. and the command
      // was successfully sent with all replies met.

    void SetDeinitString(const PString & str);
      // Set the modem de-initialisation meta-command string. Note there is an
      // implied \s before the string.

    PString GetDeinitString() const;
      // Get the modem de-initialisation meta-command string.

    BOOL CanDeinitialise() const;
      // The modem is in a state that allows the deinitialise to start.

    BOOL Deinitialise();
      // Send the de-initialisation meta-command string to the modem. The
      // return value indicates that the conditions for the operation to
      // start were met i.e. the serial port was open etc. and the command
      // was successfully sent with all replies met.

    void SetPreDialString(const PString & str);
      // Set the modem pre-dial meta-command string. Note there is an implied
      // \s before the string.

    PString GetPreDialString() const;
      // Get the modem pre-dial meta-command string.

    void SetPostDialString(const PString & str);
      // Set the modem post-dial meta-command string. Note there is not an
      // implied \s before the string, unlike the pre-dial string.

    PString GetPostDialString() const;
      // Get the modem post-dial meta-command string.

    void SetBusyString(const PString & str);
      // Set the modem busy response meta-command string. Note there is an
      // implied \w120s before the string. Also the \s and \d commands do not
      // operate and will simply terminate the string match.

    PString GetBusyString() const;
      // Get the modem busy response meta-command string.

    void SetNoCarrierString(const PString & str);
      // Set the modem no carrier response meta-command string. Note there is
      // an implied \w120s before the string. Also the \s and \d commands do
      // not operate and will simply terminate the string match.

    PString GetNoCarrierString() const;
      // Get the modem no carrier response meta-command string.

    void SetConnectString(const PString & str);
      // Set the modem connect response meta-command string. Note there is
      // an implied \w120s before the string. Also the \s and \d commands do
      // not operate and will simply terminate the string match.

    PString GetConnectString() const;
      // Get the modem connect response meta-command string.

    BOOL CanDial() const;
      // The modem is in a state that allows the dial to start.

    BOOL Dial(const PString & number);
      // Dial the specified number and wait for the reply. The return value
      // indicates that the conditions for the operation to start were met
      // i.e. the serial port was open and the modem initialised etc. The
      // string sent to the modem is the concatenation of the pre-dial, a \s,
      // number and post-dial strings.

    void SetHangUpString(const PString & str);
      // Set the modem hang up meta-command string. Note there is an implied
      // \s before the string.

    PString GetHangUpString() const;
      // Get the modem hang up meta-command string.

    BOOL CanHangUp() const;
      // The modem is in a state that allows the hang up to start.

    BOOL HangUp();
      // Hang up the modem. The return value indicates that the conditions for
      // the operation to start were met i.e. the serial port was open, the
      // modem was connected and the command was successfully sent with all
      // replies met.

    BOOL CanSendUser() const;
      // The modem is in a state that allows a user command to start.

    BOOL SendUser(const PString & str);
      // Send an arbitrary user meta-string. The return value indicates that
      // the conditions for the operation to start were met i.e. the serial
      // port was open, and the command was successfully sent with all
      // replies met.

    void Abort();
      // Abort the current operation eg hang up etc.

    BOOL CanRead() const;
      // The modem is in a state that allows the user application to read
      // from the channel. Reading while this is TRUE can interfere with the
      // operation of the meta-string processing.

    enum Status {
      Unopened,           // Has not been opened yet
      Uninitialised,      // Is open but has not yet been initialised
      Initialising,       // Is currently initialising the modem
      Initialised,        // Has been initialised but is not connected
      InitialiseFailed,   // Initialisation sequence failed
      Dialling,           // Is currently dialling
      DialFailed,         // Dial failed
      AwaitingResponse,   // Dialling in progress, awaiting connection
      LineBusy,           // Dial failed due to line busy
      NoCarrier,          // Dial failed due to no carrier
      Connected,          // Dial was successful and modem has connected
      HangingUp,          // Is currently hanging up the modem
      HangUpFailed,       // The hang up failed
      Deinitialising,     // is currently de-initialising the modem
      DeinitialiseFailed, // The de-initialisation failed
      SendingUserCommand, // Is currently sending a user command
      NumStatuses
    } GetStatus() const;
      // Return the modem status


  protected:
    BOOL SendString(const PString & command);
      // Send the specified meta-string.

    // Member variables
    PString initCmd, deinitCmd, preDialCmd, postDialCmd,
            busyReply, noCarrierReply, connectReply, hangUpCmd;
      // Modem command meta-strings. A meta-string is a string of characters
      // that may contain escaped commands. The escape command is the \ as in
      // the C language. The escape commands are:
      //    \a    alert (ascii value 7)
      //    \b    backspace (ascii value 8)
      //    \f    formfeed (ascii value 12)
      //    \n    newline (ascii value 10)
      //    \r    return (ascii value 13)
      //    \t    horizontal tab (ascii value 9)
      //    \v    vertical tab (ascii value 11)
      //    \\    backslash
      //    \ooo  where ooo is octal number (ascii value ooo)
      //    \xhh  where hh is hex number (ascii value 0xhh)
      //    \0    null character (ascii zero)
      //    \dns  delay for n seconds
      //    \dnm  delay for n milliseconds
      //    \s    characters following this, up to a \w command or the end of
      //          string, are to be sent to modem
      //    \wns  characters following this, up to a \s, \d or another \w or
      //          the end of the string are expected back from the modem. If
      //          the string is not received within n seconds, a failed command
      //          is registered. The exception to this is if the command is at
      //          the end of the string or the next character in the string is
      //          the \s, \d or \w in which case all characters are ignored
      //          from the modem until n seconds of no data.
      //    \wnm  as for above but timeout is in milliseconds

    Status status;
      // Current modem status
};


// End Of File ///////////////////////////////////////////////////////////////
