/*
 * $Id: inetmail.h,v 1.4 1996/07/27 04:14:49 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: inetmail.h,v $
 * Revision 1.4  1996/07/27 04:14:49  robertj
 * Redesign and reimplement of mail sockets.
 *
 * Revision 1.3  1996/06/28 13:16:32  robertj
 * Changed SMTP incoming message handler so can tell when started, processing or ended message.
 *
 * Revision 1.2  1996/03/16 04:38:24  robertj
 * Added ParseReponse() for splitting reponse line into code and info.
 *
 * Revision 1.1  1996/01/23 13:04:20  robertj
 * Initial revision
 *
 * Revision 1.3  1995/06/17 11:12:15  robertj
 * Documentation update.
 *
 * Revision 1.2  1995/06/17 00:39:53  robertj
 * More implementation.
 *
 * Revision 1.1  1995/06/04 13:17:16  robertj
 * Initial revision
 *
 */

#ifndef _PMAILSOCKETS
#define _PMAILSOCKETS

#ifdef __GNUC__
#pragma interface
#endif

#include <appsock.h>
#include <mime.h>


//////////////////////////////////////////////////////////////////////////////
// PPOP3Socket

PDECLARE_CLASS(PSMTPSocket, PApplicationSocket)
/* A TCP/IP socket for the Simple Mail Transfer Protocol.

   When acting as a client, the procedure is to make the connection to a
   remote server, then to send a message using the following procedure:
      <PRE><CODE>
      PSMTPSocket mail("mailserver");
      if (mail.IsOpen()) {
        mail.BeginMessage("Me@here.com.au", "Fred@somwhere.com");
        mail.Write(myMessage);
        if (!mail.EndMessage())
          PError << "Mail send failed." << endl;
      }
      else
         PError << "Mail conection failed." << endl;
      </PRE></CODE>

    When acting as a server, a descendant class would be created to override
    at least the <A>LookUpName()</A> and <A>HandleMessage()</A> functions.
    Other functions may be overridden for further enhancement to the sockets
    capabilities, but these two will give a basic SMTP server functionality.

    The server socket thread would continuously call the
    <A>ProcessMessage()</A> function until it returns FALSE. This will then
    call the appropriate virtual function on parsing the SMTP protocol.
*/

  public:
    PSMTPSocket();
    PSMTPSocket(
      const PString & address   // Address of remote machine to connect to.
    );
    PSMTPSocket(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Create a TCP/IP SMPTP protocol socket channel. The parameterless form
       creates an unopened socket, the form with the <CODE>address</CODE>
       parameter makes a connection to a remote system, opening the socket. The
       form with the <CODE>socket</CODE> parameter opens the socket to an
       incoming call from a "listening" socket.
     */

    ~PSMTPSocket();
    /* Destroy the socket object. This will close the socket and if used as a
       client, QUIT from remote SMTP server.
     */


  // Overrides from class PSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    /* Connect a socket to a remote host on the specified port number.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number.

       Note that this function will block until a remote system connects to the
       port number specified in the "listening" socket.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */

    virtual BOOL Close();
    /* Close the socket, and if connected as a client, QUITs from server.

       <H2>Returns:</H2>
       TRUE if the channel was closed and the QUIT accepted by the server.
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


    enum Commands {
      HELO, EHLO, QUIT, HELP, NOOP,
      TURN, RSET, VRFY, EXPN, RCPT,
      MAIL, SEND, SAML, SOML, DATA,
      NumCommands
    };

    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the QUIT command was
       received or the <A>OnUnknown()</A> function returns FALSE.
     */

    void ServerReset();
    // Reset the state of the SMTP server socket.

    enum ForwardResult {
      LocalDomain,    // User may be on local machine, do LookUpName().
      WillForward,    // User may be forwarded to another SMTP host.
      CannotForward   // User cannot be forwarded.
    };
    // Result of forward check

    virtual ForwardResult ForwardDomain(
      PCaselessString & userDomain,       // Domain for user
      PCaselessString & forwardDomainList // Domains forwarding to
    );
    /* Determine if a user for this domain may be on the local system, or
       should be forwarded.

       <H2>Returns:</H2>
       Result of forward check operation.
     */

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
      BOOL starting,        // This is the first call for the message.
      BOOL completed        // This is the last call for the message.
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
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */

    virtual void OnSendMail(
      const PCaselessString & sender  // Name of sender.
    );
    // Common code for OnMAIL(), OnSEND(), OnSOML() and OnSAML() funtions.

    virtual BOOL OnTextData(PCharArray & buffer, BOOL & completed);
    /* Read a standard text message that is being received by the socket. The
       text message is terminated by a line with a '.' character alone.

       The default behaviour is to read the data into the <CODE>buffer</CODE>
       parameter until either the end of the message or when the
       <CODE>messageBufferSize</CODE> bytes have been read.

       <H2>Returns:</H2>
       TRUE if partial message received, FALSE if the end of the data was
       received.
     */

    virtual BOOL OnMIMEData(PCharArray & buffer, BOOL & completed);
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
    PString     fromAddress;
    PString     fromPath;
    PStringList toNames;
    PStringList toDomains;
    PINDEX      messageBufferSize;
    enum { WasMAIL, WasSEND, WasSAML, WasSOML } sendCommand;
    StuffState  endMIMEDetectState;

  private:
    void Construct();
    BOOL _BeginMessage();
};


//////////////////////////////////////////////////////////////////////////////
// PPOP3Socket

PDECLARE_CLASS(PPOP3Socket, PApplicationSocket)
/* A TCP/IP socket for the Post Office Protocol version 3.

   When acting as a client, the procedure is to make the connection to a
   remote server, then to retrieve a message using the following procedure:
      <PRE><CODE>
      PPOP3Socket mail("popserver");
      if (mail.IsOpen()) {
        if (mail.LogIn("Me", "password")) {
          if (mail.GetMessageCount() > 0) {
            PUnsignedArray sizes = mail.GetMessageSizes();
            for (PINDEX i = 0; i < sizes.GetSize(); i++) {
              if (mail.BeginMessage(i+1))
                mail.Read(myMessage, sizes[i]);
              else
                PError << "Error getting mail message." << endl;
            }
          }
          else
            PError << "No mail messages." << endl;
        }
        else
           PError << "Mail log in failed." << endl;
      }
      else
         PError << "Mail conection failed." << endl;
      </PRE></CODE>

    When acting as a server, a descendant class would be created to override
    at least the <A>HandleOpenMailbox()</A>, <A>HandleSendMessage()</A> and
    <A>HandleDeleteMessage()</A> functions. Other functions may be overridden
    for further enhancement to the sockets capabilities, but these will give a
    basic POP3 server functionality.

    The server socket thread would continuously call the
    <A>ProcessMessage()</A> function until it returns FALSE. This will then
    call the appropriate virtual function on parsing the POP3 protocol.
 */

  public:
    PPOP3Socket();
    PPOP3Socket(
      const PString & address   // Address of remote machine to connect to.
    );
    PPOP3Socket(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Create a TCP/IP POP3 protocol socket channel. The parameterless form
       creates an unopened socket, the form with the <CODE>address</CODE>
       parameter makes a connection to a remote system, opening the socket. The
       form with the <CODE>socket</CODE> parameter opens the socket to an
       incoming call from a "listening" socket.
     */

    ~PPOP3Socket();
    /* Destroy the socket object. This will close the socket and if used as a
       client, QUIT from remote POP3 server.
     */


  // Overrides from class PSocket.
    virtual BOOL Connect(
      const PString & address   // Address of remote machine to connect to.
    );
    /* Connect a socket to a remote host on the specified port number. This is
       typically used by the client or initiator of a communications channel.
       This connects to a "listening" socket at the other end of the
       communications channel.

       The port number as defined by the object instance construction or the
       <A>PIPSocket::SetPort()</A> function.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    virtual BOOL Accept(
      PSocket & socket          // Listening socket making the connection.
    );
    /* Open a socket to a remote host on the specified port number. This is an
       "accepting" socket. When a "listening" socket has a pending connection
       to make, this will accept a connection made by the "connecting" socket
       created to establish a link.

       The port that the socket uses is the one used in the <A>Listen()</A>
       command of the <CODE>socket</CODE> parameter.

       Note that this function will block until a remote system connects to the
       port number specified in the "listening" socket.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */

    virtual BOOL Close();
    /* Close the socket, and if connected as a client, QUITs from server.

       <H2>Returns:</H2>
       TRUE if the channel was closed and the QUIT accepted by the server.
     */


  // New functions for class.
    BOOL LogIn(
      const PString & username,   // User name on remote system.
      const PString & password    // Password for user name.
    );
    /* Log into the POP server using the mailbox and access codes specified.

       <H2>Returns:</H2>
       TRUE if logged in.
     */

    int GetMessageCount();
    /* Get a count of the number of messages in the mail box.

       <H2>Returns:</H2>
       Number of messages in mailbox or -1 if an error occurred.
     */

    PUnsignedArray GetMessageSizes();
    /* Get an array of a integers representing the sizes of each of the
       messages in the mail box.

       <H2>Returns:</H2>
       Array of integers representing the size of each message.
     */

    PStringArray GetMessageHeaders();
    /* Get an array of a strings representing the standard internet message
       headers of each of the messages in the mail box.

       Note that the remote server may not support this function, in which
       case an empty array will be returned.

       <H2>Returns:</H2>
       Array of strings continaing message headers.
     */


    BOOL BeginMessage(
      PINDEX messageNumber
        /* Number of message to retrieve. This is an integer from 1 to the
           maximum number of messages available.
         */
    );
    /* Begin the retrieval of an entire message. The application may then use
       the <A>PApplicationSocket::ReadLine()</A> function with the
       <CODE>unstuffLine</CODE> parameter set to TRUE. Repeated calls until
       its return valus is FALSE will read the message headers and body.

       <H2>Returns:</H2>
       Array of strings continaing message headers.
     */

    BOOL DeleteMessage(
      PINDEX messageNumber
        /* Number of message to retrieve. This is an integer from 1 to the
           maximum number of messages available.
         */
    );
    /* Delete the message specified from the mail box.

       <H2>Returns:</H2>
       Array of strings continaing message headers.
     */


    enum Commands {
      USER, PASS, QUIT, RSET, NOOP, STAT,
      LIST, RETR, DELE, APOP, TOP,  UIDL,
      NumCommands
    };

    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the QUIT command was
       received or the <A>OnUnknown()</A> function returns FALSE.
     */

    virtual BOOL HandleOpenMailbox(
      const PString & username,  // User name for mail box
      const PString & password   // Password for user name
    );
    /* Log the specified user into the mail system and return sizes of each
       message in mail box.

       The user override of this function is expected to populate the protected
       member fields <CODE>messageSizes</CODE> and <CODE>messageIDs</CODE>.

       <H2>Returns:</H2>
       TRUE if user and password were valid.
     */

    virtual void HandleSendMessage(
      PINDEX messageNumber, // Number of message to send.
      const PString & id,   // Unique id of message to send.
      PINDEX lines          // Nuumber of lines in body of message to send.
    );
    /* Handle the sending of the specified message to the remote client. The
       data written to the socket will automatically have the '.' character
       stuffing enabled.

       <H2>Returns:</H2>
       TRUE if successfully sent message.
     */
    
    virtual void HandleDeleteMessage(
      PINDEX messageNumber, // Number of message to send.
      const PString & id    // Unique id of message to send.
    );
    /* Handle the deleting of the specified message from the mail box. This is
       called when the OnQUIT command is called for each message that was
       deleted using the DELE command.

       <H2>Returns:</H2>
       TRUE if successfully sent message.
     */
    

  protected:
    virtual PINDEX ParseResponse(
      const PString & line // Input response line to be parsed
    );
    /* Parse a response line string into a response code and any extra info
       on the line. Results are placed into the member variables
       <CODE>lastResponseCode</CODE> and <CODE>lastResponseInfo</CODE>.

       The default bahaviour looks for a space or a '-' and splits the code
       and info either side of that character, then returns FALSE.

       <H2>Returns:</H2>
       Position of continuation character in response, 0 if no continuation
       lines are possible.
     */

    virtual void OnUSER(
      const PString & name  // Name of user.
    );
    // Specify user name (mailbox).

    virtual void OnPASS(
      const PString & passwd  // Password for account.
    );
    // Specify password and log user in.

    virtual void OnQUIT();
    // End connection, saving all changes (delete messages).

    virtual void OnRSET();
    // Reset connection (undelete messages).

    virtual void OnNOOP();
    // Do nothing.

    virtual void OnSTAT();
    // Get number of messages in mailbox.

    virtual void OnLIST(
      PINDEX msg  // Number of message.
    );
    /* Get the size of a message in mailbox. If <CODE>msg</CODE> is 0 then get
       sizes of all messages in mailbox.
     */

    virtual void OnRETR(
      PINDEX msg  // Number of message.
    );
    // Retrieve a message from mailbox.

    virtual void OnDELE(
      PINDEX msg  // Number of message.
    );
    // Delete a message from mailbox.

    virtual void OnTOP(
      PINDEX msg,  // Number of message.
      PINDEX count // Count of messages
    );
    // Get the message header and top <CODE>count</CODE> lines of message.

    virtual void OnUIDL(
      PINDEX msg  // Number of message.
    );
    /* Get unique ID for message in mailbox. If <CODE>msg</CODE> is 0 then get
       all IDs for all messages in mailbox.
     */

    virtual BOOL OnUnknown(
      const PCaselessString & command  // Complete command line received.
    );
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */


  // Member variables
    BOOL loggedIn;

    PString        username;
    PUnsignedArray messageSizes;
    PStringArray   messageIDs;
    PBYTEArray     messageDeletions;

    static PString okResponse;
    static PString errResponse;


  private:
    void Construct();
};


#endif


// End Of File ///////////////////////////////////////////////////////////////
