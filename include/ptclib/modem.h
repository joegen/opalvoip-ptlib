/*
 * $Id: modem.h,v 1.6 1995/01/06 10:31:02 robertj Exp $
 *
 * Portable Windows Library
 *
 * User Interface Classes Interface Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: modem.h,v $
 * Revision 1.6  1995/01/06 10:31:02  robertj
 * Documentation.
 *
 * Revision 1.5  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.4  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.3  1994/08/21  23:43:02  robertj
 * Moved meta-string transmitter from PModem to PChannel.
 *
 * Revision 1.2  1994/07/25  03:32:29  robertj
 * Fixed bug in GCC with enums.
 *
 * Revision 1.1  1994/06/25  11:55:15  robertj
 * Initial revision
 *
 */


#define _PMODEM

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PModem, PSerialChannel)
/* A class representing a modem attached to a serial port. This adds the usual
   modem operations to the basic serial port.
   
   A modem object is always in a particular state. This state determines what
   operations are allowed which then move the object to other states. The
   operations are the exchange of strings in "chat" script.
   
   The following defaults are used for command strings:
       initialise         ATZ\r\w2sOK\w100m
       deinitialise       \d2s+++\d2sATH0\r
       pre-dial           ATDT
       post-dial          \r
       busy reply         BUSY
       no carrier reply   NO CARRIER
       connect reply      CONNECT
       hang up            \d2s+++\d2sATH0\r

 */

  public:
    PModem();
    PModem(
      const PString & port,   // Serial port name to open.
      DWORD speed = 0,        // Speed of serial port.
      BYTE data = 0,          // Number of data bits for serial port.
      Parity parity = DefaultParity,  // Parity for serial port.
      BYTE stop = 0,          // Number of stop bits for serial port.
      FlowControl inputFlow = DefaultFlowControl,   // Input flow control.
      FlowControl outputFlow = DefaultFlowControl   // Output flow control.
    );
    /* Create a modem object on the serial port specified. If no port was
       specified do not open it. It does not initially have a valid port name.
       
       See the $H$PSerialChannel class for more information on the parameters.
     */

    PModem(
      PConfig & cfg   // Configuration file to read parameters from.
    );
    /* Open the modem serial channel obtaining the parameters from standard
       variables in the configuration file. Note that it assumed that the
       correct configuration file section is already set.
     */


  // Overrides from class PChannel
    virtual BOOL Close();
    // Close the modem serial port channel.


  // Overrides from class PSerialChannel
    virtual BOOL Open(
      const PString & port,   // Serial port name to open.
      DWORD speed = 0,        // Speed of serial port.
      BYTE data = 0,          // Number of data bits for serial port.
      Parity parity = DefaultParity,  // Parity for serial port.
      BYTE stop = 0,          // Number of stop bits for serial port.
      FlowControl inputFlow = DefaultFlowControl,   // Input flow control.
      FlowControl outputFlow = DefaultFlowControl   // Output flow control.
    );
    /* Open the modem serial channel on the specified port.
       
       See the $H$PSerialChannel class for more information on the parameters.
       
       Returns: TRUE if the modem serial port was successfully opened.
     */

    virtual BOOL Open(
      PConfig & cfg   // Configuration file to read parameters from.
    );
    /* Open the modem serial port obtaining the parameters from standard
       variables in the configuration file. Note that it assumed that the
       correct configuration file section is already set.

       Returns: TRUE if the modem serial port was successfully opened.
     */

    virtual void SaveSettings(
      PConfig & cfg   // Configuration file to write parameters to.
    );
    // Save the current modem serial port settings into the configuration file.


  // New member functions
    void SetInitString(
      const PString & str   // New initialisation command string.
    );
    /* Set the modem initialisation meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \s before the string.
     */

    PString GetInitString() const;
    /* Get the modem initialisation meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for initialisation command.
     */

    BOOL CanInitialise() const;
    /* The modem is in a state that allows the initialise to start.
    
       Returns: TRUE if the $B$Initialise()$B$ function may proceeed.
     */

    BOOL Initialise();
    /* Send the initialisation meta-command string to the modem. The return
       value indicates that the conditions for the operation to start were met,
       ie the serial port was open etc and the command was successfully
       sent with all replies met.

       Returns: TRUE if command string sent successfully and the objects state
                has changed.
     */

    void SetDeinitString(
      const PString & str   // New de-initialisation command string.
    );
    /* Set the modem de-initialisation meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \s before the string.
     */

    PString GetDeinitString() const;
    /* Get the modem de-initialisation meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for de-initialisation command.
     */

    BOOL CanDeinitialise() const;
    /* The modem is in a state that allows the de-initialise to start.
    
       Returns: TRUE if the $B$Deinitialise()$B$ function may proceeed.
     */

    BOOL Deinitialise();
    /* Send the de-initialisation meta-command string to the modem. The return
       value indicates that the conditions for the operation to start were met,
       ie the serial port was open etc and the command was successfully
       sent with all replies met.

       Returns: TRUE if command string sent successfully and the objects state
                has changed.
     */

    void SetPreDialString(
      const PString & str   // New pre-dial command string.
    );
    /* Set the modem pre-dial meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \s before the string.
     */

    PString GetPreDialString() const;
    /* Get the modem pre-dial meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for pre-dial command.
     */

    void SetPostDialString(
      const PString & str   // New post-dial command string.
    );
    /* Set the modem post-dial meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is $U$not$U$ an implied \s before the string, unlike the
       pre-dial string.
     */

    PString GetPostDialString() const;
    /* Get the modem post-dial meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for post-dial command.
     */

    void SetBusyString(
      const PString & str   // New busy response command string.
    );
    /* Set the modem busy response meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \w120s before the string. Also the \s and \d
       commands do not operate and will simply terminate the string match.
     */

    PString GetBusyString() const;
    /* Get the modem busy response meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for busy response command.
     */

    void SetNoCarrierString(
      const PString & str   // New no carrier response command string.
    );
    /* Set the modem no carrier response meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \w120s before the string. Also the \s and \d
       commands do not operate and will simply terminate the string match.
     */

    PString GetNoCarrierString() const;
    /* Get the modem no carrier response meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for no carrier response command.
     */

    void SetConnectString(
      const PString & str   // New connect response command string.
    );
    /* Set the modem connect response meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \w120s before the string. Also the \s and \d
       commands do not operate and will simply terminate the string match.
     */

    PString GetConnectString() const;
    /* Get the modem connect response meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for connect response command.
     */

    BOOL CanDial() const;
    /* The modem is in a state that allows the dial to start.
    
       Returns: TRUE if the $B$Dial()$B$ function may proceeed.
     */

    BOOL Dial(const PString & number);
    /* Send the dial meta-command strings to the modem. The return
       value indicates that the conditions for the operation to start were met,
       ie the serial port was open etc and the command was successfully
       sent with all replies met.

       The string sent to the modem is the concatenation of the pre-dial
       string, a \s, the $B$number$B$ parameter and the post-dial string.

       Returns: TRUE if command string sent successfully and the objects state
                has changed.
     */

    void SetHangUpString(
      const PString & str   // New hang up command string.
    );
    /* Set the modem hang up meta-command string.

       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Note there is an implied \s before the string.
     */

    PString GetHangUpString() const;
    /* Get the modem hang up meta-command string.
    
       See the $H$PChannel::SendCommandString() function for more information
       on the format of the command string.

       Returns: string for hang up command.
     */

    BOOL CanHangUp() const;
    /* The modem is in a state that allows the hang up to start.
    
       Returns: TRUE if the $B$HangUp()$B$ function may proceeed.
     */

    BOOL HangUp();
    /* Send the hang up meta-command string to the modem. The return
       value indicates that the conditions for the operation to start were met,
       ie the serial port was open etc and the command was successfully
       sent with all replies met.

       Returns: TRUE if command string sent successfully and the objects state
                has changed.
     */

    BOOL CanSendUser() const;
    /* The modem is in a state that allows the user command to start.
    
       Returns: TRUE if the $B$SendUser()$B$ function may proceeed.
     */

    BOOL SendUser(
      const PString & str   // User command string to send.
    );
    /* Send an arbitrary user meta-command string to the modem. The return
       value indicates that the conditions for the operation to start were met,
       ie the serial port was open etc and the command was successfully
       sent with all replies met.

       Returns: TRUE if command string sent successfully.
     */

    void Abort();
    // Abort the current meta-string command operation eg dial, hang up etc.

    BOOL CanRead() const;
    /* The modem is in a state that allows the user application to read from
       the channel. Reading while this is TRUE can interfere with the operation
       of the meta-string processing. This function is only usefull when
       multi-threading is used.

       Returns: TRUE if Read() operations are "safe".
     */

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
    };
    // Modem object states.

    Status GetStatus() const;
    /* Get the modem objects current state.

       Returns: modem status.
     */


  protected:
    // Member variables
    PString initCmd, deinitCmd, preDialCmd, postDialCmd,
            busyReply, noCarrierReply, connectReply, hangUpCmd;
      // Modem command meta-strings.

    Status status;
      // Current modem status
};


// End Of File ///////////////////////////////////////////////////////////////
