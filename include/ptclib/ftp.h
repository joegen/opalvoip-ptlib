/*
 * $Id: ftp.h,v 1.2 1996/03/18 13:33:10 robertj Exp $
 *
 * Portable Windows Library
 *
 * FTP Server/Client Socket Class Declarations
 *  As per RFC 959 and RFC 1123
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ftp.h,v $
 * Revision 1.2  1996/03/18 13:33:10  robertj
 * FireDoorV10
 *
 * Revision 1.1  1996/03/04 12:14:18  robertj
 * Initial revision
 *
 */

#ifndef _PFTPSOCKET
#define _PFTPSOCKET

#ifdef __GNUC__
#pragma interface
#endif

#ifndef _PAPPLICATIONSOCKET
#include <appsock.h>
#endif


PDECLARE_CLASS(PFTPSocket, PApplicationSocket)
  public:
    enum { DefaultFTPPort = 21 };
    enum { MaxIllegalPasswords = 3 };

    PFTPSocket(
      WORD port = DefaultFTPPort  // Port number to use for the connection.
    );
    // declare a client socket - use Connect later

    PFTPSocket(
      const PString & address,  // Address of remote machine to connect to.
      WORD port = DefaultFTPPort  // Port number to use for the connection.
    );
    // declare a client socket - automatically Connects

    PFTPSocket(
      PSocket & socket          // Listening socket making the connection.
    );
    PFTPSocket(
      PSocket & socket,         // Listening socket making the connection.
      const PString & readyString   // Sign on string on connection ready.
    );
    // declare a server socket - automatically Accepts

    ~PFTPSocket();
    // Delete and close the socket.


    //
    // client routines
    //

    BOOL Connect(
      const PString & address     // Remote address to connect to.
    );
    /* Connect a socket to a remote host for FTP.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the remote host.
     */


    //
    // server routines
    //

    virtual PString GetHelloString(const PString & user) const;
      // return the string printed when a user logs in
      // default value is a string giving the user name

    virtual PString GetGoodbyeString(const PString & user) const;
      // return the string printed just before exiting

    virtual PString GetSystemTypeString() const;
      // return the string to be returned by the SYST command


    enum Commands { 
      USER, PASS, ACCT, CWD, CDUP, SMNT, QUIT, REIN, PORT, PASV, TYPE,
      STRU, MODE, RETR, STOR, STOU, APPE, ALLO, REST, RNFR, RNTO, ABOR,
      DELE, RMD, MKD, PWD, LIST, NLST, SITE, SYST, STAT, HELP, NOOP,
      NumCommands
    };

    BOOL ProcessCommand();
    /* Process commands, dispatching to the appropriate virtual function. This
       is used when the socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the QUIT command was
       received or the <A>OnUnknown()</A> function returns FALSE.
     */

    virtual BOOL DispatchCommand(
      PINDEX code,          // Parsed command code.
      const PString & args  // Arguments to command.
    );
    /* Dispatching to the appropriate virtual function. This is used when the
       socket is acting as a server.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the QUIT command was
       received or the <A>OnUnknown()</A> function returns FALSE.
     */


    virtual BOOL CheckLoginRequired(
      PINDEX cmd    // Command to check if log in required.
    );
    /* Check to see if the command requires the server to be logged in before
       it may be processed.

       <H2>Returns:</H2>
       TRUE if the command required the user to be logged in.
     */

    virtual BOOL AuthoriseUser(
      const PString & user,     // User name to authorise.
      const PString & password, // Password supplied for the user.
      BOOL & replied            // Indication that a reply was sent to client.
    );
    /* Validate the user name and password for access. After three invalid
       attempts, the socket will close and FALSE is returned.

       Default implementation returns TRUE for all strings.

       <H2>Returns:</H2>
       TRUE if user can access, otherwise FALSE
     */

    virtual BOOL OnUnknown(
      const PCaselessString & command  // Complete command line received.
    );
    /* Handle an unknown command.

       <H2>Returns:</H2>
       TRUE if more processing may be done, FALSE if the
       <A>ProcessCommand()</A> function is to return FALSE.
     */

    virtual void OnSyntaxError(
      PINDEX cmdNum   // Command that had the syntax error.
    );
    // Called for syntax errors in commands.

    virtual void OnNotImplemented(
      PINDEX cmdNum   // Command that was not implemented.
    );
    // Called for unimplemented commands.

    virtual void OnCommandSuccessful(
      PINDEX cmdNum   // Command that had was successful.
    );
    // Called for successful commands.


    // the following commands must be implemented by all servers
    // and can be performed without logging in
    virtual BOOL OnUSER(const PCaselessString & args);
    virtual BOOL OnPASS(const PCaselessString & args);  // officially optional, but should be done
    virtual BOOL OnQUIT(const PCaselessString & args);
    virtual BOOL OnPORT(const PCaselessString & args);
    virtual BOOL OnSTRU(const PCaselessString & args);
    virtual BOOL OnMODE(const PCaselessString & args);
    virtual BOOL OnTYPE(const PCaselessString & args);
    virtual BOOL OnNOOP(const PCaselessString & args);
    virtual BOOL OnSYST(const PCaselessString & args);
    virtual BOOL OnSTAT(const PCaselessString & args);

    // the following commands must be implemented by all servers
    // and cannot be performed without logging in
    virtual BOOL OnRETR(const PCaselessString & args);
    virtual BOOL OnSTOR(const PCaselessString & args);
    virtual BOOL OnACCT(const PCaselessString & args);
    virtual BOOL OnAPPE(const PCaselessString & args);
    virtual BOOL OnRNFR(const PCaselessString & args);
    virtual BOOL OnRNTO(const PCaselessString & args);
    virtual BOOL OnDELE(const PCaselessString & args);
    virtual BOOL OnCWD(const PCaselessString & args);
    virtual BOOL OnCDUP(const PCaselessString & args);
    virtual BOOL OnRMD(const PCaselessString & args);
    virtual BOOL OnMKD(const PCaselessString & args);
    virtual BOOL OnPWD(const PCaselessString & args);
    virtual BOOL OnLIST(const PCaselessString & args);
    virtual BOOL OnNLST(const PCaselessString & args);
    virtual BOOL OnPASV(const PCaselessString & args);

    // the following commands are optional and can be performed without
    // logging in
    virtual BOOL OnHELP(const PCaselessString & args);
    virtual BOOL OnSITE(const PCaselessString & args);
    virtual BOOL OnABOR(const PCaselessString & args);

    // the following commands are optional and cannot be performed
    // without logging in
    virtual BOOL OnSMNT(const PCaselessString & args);
    virtual BOOL OnREIN(const PCaselessString & args);
    virtual BOOL OnSTOU(const PCaselessString & args);
    virtual BOOL OnALLO(const PCaselessString & args);
    virtual BOOL OnREST(const PCaselessString & args);


    void SendToClient(
      const PFilePath & filename    // File name to send.
    );
    // Send the specified file to the client.

    BOOL SendPORT(
      const PIPSocket::Address & addr,  // Address for PORT connection.
      WORD port                         // Port number for PORT connection.
    );
    BOOL SendPORT(
      const PIPSocket::Address & addr,  // Address for PORT connection.
      WORD port,                        // Port number for PORT connection.
      int & code,                       // Return code for PORT command.
      PString & info                    // Return info for PORT command.
    );
    // Send the PORT command for a transfer.


  protected:
    void Construct();
    void ConstructServerSocket(const PString & readyString);

    enum { NotConnected, NeedUser, NeedPassword, Connected } state;

    PIPSocket::Address remoteHost;
    WORD remotePort;

    PTCPSocket * passiveSocket;

    char    type;
    char    structure;
    char    mode;
    PString userName;
    int     illegalPasswordCount;
};


#endif


// End of File ///////////////////////////////////////////////////////////////
