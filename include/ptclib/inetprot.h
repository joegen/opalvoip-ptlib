/*
 * $Id: inetprot.h,v 1.1 1995/06/04 13:17:16 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: inetprot.h,v $
 * Revision 1.1  1995/06/04 13:17:16  robertj
 * Initial revision
 *
 */

#ifndef _PAPPLICATIONSOCKET
#define _PAPPLICATIONSOCKET

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PApplicationSocket, PTCPSocket)
/* A TCP/IP socket for process/application layer high level protocols. All of
   these protocols execute commands and responses in a standard manner.

   A command consists of a line starting with a short, case insensitive command
   string terminated by a space or the end of the line. This may be followed
   by optional arguments.

   A response to a command is usually a number and/or a short string eg "OK".
   The response may be followed by additional information about the response
   but this is not typically used by the protocol. It is only for user
   information and may be tracked in log files etc.

   All command and reponse lines of the protocol are terminated by a CR/LF
   pair. A command or response line may be followed by additional data as
   determined by the protocol, but this data is "outside" the protocol
   specification as defined by this class.
 */

  public:
    PApplicationSocket(
      PINDEX cmdCount,               // Number of command strings.
      char const * const * cmdNames, // Strings for each command.
      WORD port = 0                  // Port number to use for the connection.
    );
    PApplicationSocket(
      PINDEX cmdCount,               // Number of command strings.
      char const * const * cmdNames, // Strings for each command.
      const PString & service   // Service name to use for the connection.
    );
    // Create an unopened TCP/IP protocol socket channel.

    PApplicationSocket(
      PINDEX cmdCount,               // Number of command strings.
      char const * const * cmdNames, // Strings for each command.
      const PString & address,  // Address of remote machine to connect to.
      WORD port                 // Port number to use for the connection.
    );
    PApplicationSocket(
      PINDEX cmdCount,               // Number of command strings.
      char const * const * cmdNames, // Strings for each command.
      const PString & address,  // Address of remote machine to connect to.
      const PString & service   // Service name to use for the connection.
    );
    PApplicationSocket(
      PINDEX cmdCount,               // Number of command strings.
      char const * const * cmdNames, // Strings for each command.
      PSocket & socket          // Listening socket making the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


  // New functions for class.
    BOOL WriteLine(
      const PString & line // String to write as a command line.
    );
    /* Write a string to the socket channel followed by a CR/LF pair. If there
       are any lone CR or LF characters in the line, then these are translated
       into CR/LF pairs.

       <H2>Returns:</H2>
       TRUE if the string & CR/LF were completely written.
     */

    BOOL ReadLine(
      PString & line // String to receive a CR/LF terminated line.
    );
    /* Read a string from the socket channel up to a CR/LF pair.
    
       Note this function will block for the time specified by the
       <A>PChannel::SetReadTimeout()</A> function.

       <H2>Returns:</H2>
       TRUE if a CR/LF pair was received.
     */

    BOOL WriteCommand(
      PINDEX cmdNumber,      // Number of command to write.
      const PString & param  // Extra parameters required by the command.
    );
    /* Write a single line for a command. The command name for the command
       number is output, then a space, the the <CODE>param</CODE> string
       followed at the end with a CR/LF pair.

       If the <CODE>cmdNumber</CODE> parameter is outside of the range of
       valid command names, then the function does not send anything and
       returns FALSE.

       This function is typically used by client forms of the socket.

       <H2>Returns:</H2>
       TRUE if the command was completely written.
     */

    PINDEX ReadCommand(
      PString & args  // String to receive the arguments to the command.
    );
    /* Read a single line of a command which ends with a CR/LF pair. The
       command number for the command name is parsed from the input, then the
       remaining text on the line is returned in the <CODE>args</CODE>
       parameter.

       If the command does not match any of the command names then the entire
       line is placed in the <CODE>args</CODE> parameter and a value of
       P_MAX_INDEX is returned.

       Note this function will block for the time specified by the
       <A>PChannel::SetReadTimeout()</A> function.

       This function is typically used by server forms of the socket.

       <H2>Returns:</H2>
       Number of the command parsed from the command line, or P_MAX_INDEX if
       no match.
     */

    BOOL WriteResponse(
      unsigned numericCode, // Response code for command response.
      const PString & info  // Extra information available after response code.
    );
    BOOL WriteResponse(
      const PString & code, // Response code for command response.
      const PString & info  // Extra information available after response code.
    );
    /* Write a response code followed by a text string describing the response
       to a command. The form of the response is to place the code string,
       then the info string.
       
       If the <CODE>info</CODE> parameter has multiple lines then each line
       has the response code at the start. A '-' character separates the code
       from the text on all lines but the last where a ' ' character is used.

       The first form assumes that the response code is a 3 digit numerical
       code. The second form allows for any arbitrary string to be the code.

       This function is typically used by server forms of the socket.

       <H2>Returns:</H2>
       TRUE if the response was completely written.
     */

    BOOL ReadResponse();
    BOOL ReadResponse(
      PString & code,  // Response code for command response.
      PString & info   // Extra information available after response code.
    );
    /* Read a response code followed by a text string describing the response
       to a command. The form of the response is to have the code string,
       then the info string.
       
       The response may have multiple lines in it. A '-' character separates
       the code from the text on all lines but the last where a ' ' character
       is used. The <CODE>info</CODE> parameter will have placed in it all of
       the response lines separated by a single '\n' character.

       The first form places the response code and info into the protected
       member variables <CODE>lastResponseCode</CODE> and
       <CODE>lastResponseInfo</CODE>.

       This function is typically used by client forms of the socket.

       <H2>Returns:</H2>
       TRUE if the response was completely read without a socket error.
     */

    char ExecuteCommand(
      PINDEX cmdNumber,      // Number of command to write.
      const PString & param  // Extra parameters required by the command.
    );
    /* Write a command to the socket, using <CODE>WriteCommand()</CODE> and
       await a response using <CODE>ReadResponse()</CODE>. The first character
       of the response is returned, as well as the entire response being saved
       into the protected member variables <CODE>lastResponseCode</CODE> and
       <CODE>lastResponseInfo</CODE>.

       This function is typically used by client forms of the socket.

       <H2>Returns:</H2>
       First character of response string or '\0' if a socket error occurred.
     */


  protected:
    PStringArray commandNames;
    // Names of each of the command codes.

    PString lastResponseCode;
    PString lastResponseInfo;
    // Response 
};


PDECLARE_CLASS(PSMTPSocket, PApplicationSocket)
/* A TCP/IP socket for Simple Mail Transfer Protocol.
 */

  public:
    PSMTPSocket();
    // Create an unopened TCP/IP SMPTP protocol socket channel.

    PSMTPSocket(
      const PString & address   // Address of remote machine to connect to.
    );
    PSMTPSocket(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Create a TCP/IP protocol socket channel. If a remote machine address or
       a "listening" socket is specified then the channel is also opened.
     */


  // Overrides from class PChannel.
    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       This override assures that the sequence CR/LF/./CR/LF does not occur by
       byte stuffing an extra '.' character into the data stream, whenever a
       line begins with a '.' character.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */


  // New functions for class.
    BOOL BeginMessage(
      const PString & from,        // User name of sender.
      const PString & to,          // User name of recipient.
      BOOL eightBitMIME = FALSE    // Mesage will be 8 bit MIME.
    );
    BOOL BeginMessage(
      const PString & from,        // User name of sender.
      const PStringList & toList,  // List of user names of recipients.
      BOOL eightBitMIME = FALSE    // Mesage will be 8 bit MIME.
    );
    /* Begin transmission of a message using the SMTP socket as a client. This
       negotiates with the remote server and establishes the protocol state
       for data transmission. The usual Write() or stream commands may then
       be used to transmit the data itself.

       <H2>Returns:</H2>
       TRUE if message was handled, FALSE if an error occurs.
     */

    BOOL EndMessage();
    /* End transmission of a message using the SMTP socket as a client.

       <H2>Returns:</H2>
       TRUE if message was accepted by remote server, FALSE if an error occurs.
     */


    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more precessing may be done, FALSE if the QUIT command was
       received or the <A>OnUnknown()</A> function returns FALSE.
     */

    enum Commands {
      HELO, EHLO, QUIT, HELP, NOOP,
      TURN, RSET, VRFY, EXPN, RCPT,
      MAIL, SEND, SAML, SOML, DATA,
      NumCommands
    };

    void Reset();
    // Reset the state of the SMTP socket.

    enum LookUpResult {
      ValidUser,      // User name was valid and unique.
      AmbiguousUser,  // User name was valid but ambiguous.
      UnknownUser,    // User name was invalid.
      LookUpError     // Some other error occurred in look up.
    };
    // Result of user name look up

    virtual LookUpResult LookUpName(
      const PCaselessString & name,    // Name to look up.
      PString & expandedName           // Expanded form of name (if found).
    );
    /* Look up a name in the context of the SMTP server.

       The default bahaviour simply returns FALSE.

       <H2>Returns:</H2>
       Result of name look up operation.
     */

    virtual BOOL HandleMessage(
      PCharArray & buffer,  // Buffer containing message data received.
      BOOL completed
      // Indication that the entire message has been received.
    );
    /* Handle a received message. The <CODE>buffer</CODE> parameter contains
       the partial or complete message received, depending on the
       <CODE>completed</CODE> parameter.

       The default behaviour is to simply return FALSE;

       <H2>Returns:</H2>
       TRUE if message was handled, FALSE if an error occurs.
     */


  protected:
    virtual void OnHELO(
      const PCaselessString & remoteHost  // Name of remote host.
    );
    // Start connection.

    virtual void OnEHLO(
      const PCaselessString & remoteHost  // Name of remote host.
    );
    // Start extended SMTP connection.

    virtual void OnQUIT();
    // close connection and die.

    virtual void OnHELP();
    // get help.

    virtual void OnNOOP();
    // do nothing
    
    virtual void OnTURN();
    // switch places
    
    virtual void OnRSET();
    // Reset state.

    virtual void OnVRFY(
      const PCaselessString & name    // Name to verify.
    );
    // Verify address.

    virtual void OnEXPN(
      const PCaselessString & name    // Name to expand.
    );
    // Expand alias.

    virtual void OnRCPT(
      const PCaselessString & recipient   // Name of recipient.
    );
    // Designate recipient

    virtual void OnMAIL(
      const PCaselessString & sender  // Name of sender.
    );
    // Designate sender
    
    virtual void OnSEND(
      const PCaselessString & sender  // Name of sender.
    );
    // send message to screen

    virtual void OnSAML(
      const PCaselessString & sender  // Name of sender.
    );
    // send AND mail
    
    virtual void OnSOML(
      const PCaselessString & sender  // Name of sender.
    );
    // send OR mail

    virtual void OnDATA();
    // Message text.

    virtual BOOL OnUnknown(
      const PCaselessString & command  // Complete command line received.
    );
    /* handle an unknown command.

       <H2>Returns:</H2>
       TRUE if more precessing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */

    virtual void OnSendMail(
      const PCaselessString & sender  // Name of sender.
    );
    // Common code for OnMAIL(), OnSEND(), OnSOML() and OnSAML() funtions.

    virtual BOOL OnTextData(PCharArray & buffer);
    /* Read a standard text message that is being received by the socket. The
       text message is terminated by a line with a '.' character alone.

       The default behaviour is to read the data into the <CODE>buffer</CODE>
       parameter until either the end of the message or when the
       <CODE>messageBufferSize</CODE> bytes have been read.

       <H2>Returns:</H2>
       TRUE if partial message received, FALSE if the end of the data was
       received.
     */

    virtual BOOL OnMimeData(PCharArray & buffer);
    /* Read an eight bit MIME message that is being received by the socket. The
       MIME message is terminated by the CR/LF/./CR/LF sequence.

       The default behaviour is to read the data into the <CODE>buffer</CODE>
       parameter until either the end of the message or when the
       <CODE>messageBufferSize</CODE> bytes have been read.

       <H2>Returns:</H2>
       TRUE if partial message received, FALSE if the end of the data was
       received.
     */


  // Member variables
    BOOL        haveHello;
    BOOL        extendedHello;
    BOOL        eightBitMIME;
    BOOL        sendingData;
    PString     fromName;
    PStringList toNames;
    PINDEX      messageBufferSize;
    enum { WasMAIL, WasSEND, WasSAML, WasSOML } sendCommand;
    enum { GotNothing, GotCR, GotCRLF, GotCRLFdot, GotCRLFdotCR } stuffingState;

  private:
    BOOL _BeginMessage();
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
