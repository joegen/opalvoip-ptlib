/*
 * ftp.h
 *
 * File Transfer Protocol Server/Client channel classes
 *  As per RFC 959 and RFC 1123
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
 * $Log: ftp.h,v $
 * Revision 1.10  1999/02/16 08:07:10  robertj
 * MSVC 6.0 compatibility changes.
 *
 * Revision 1.9  1998/11/30 02:50:45  robertj
 * New directory structure
 *
 * Revision 1.8  1998/09/23 06:19:26  robertj
 * Added open source copyright license.
 *
 * Revision 1.7  1996/10/26 01:39:41  robertj
 * Added check for security breach using 3 way FTP transfer or use of privileged PORT.
 *
 * Revision 1.6  1996/09/14 13:09:08  robertj
 * Major upgrade:
 *   rearranged sockets to help support IPX.
 *   added indirect channel class and moved all protocols to descend from it,
 *   separating the protocol from the low level byte transport.
 *
 * Revision 1.5  1996/05/23 09:56:24  robertj
 * Changed FTP so can do passive/active mode on all data transfers.
 *
 * Revision 1.4  1996/03/31 08:45:57  robertj
 * Added QUIT command sent on FTP socket close.
 *
 * Revision 1.3  1996/03/26 00:50:28  robertj
 * FTP Client Implementation.
 *
 * Revision 1.2  1996/03/18 13:33:10  robertj
 * Fixed incompatibilities to GNU compiler where PINDEX != int.
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

#include <ptclib/inetprot.h>
#include <ptlib/sockets.h>


class PFTP : public PInternetProtocol
{
  PCLASSINFO(PFTP, PInternetProtocol)
  public:
    // FTP commands
    enum Commands { 
      USER, PASS, ACCT, CWD, CDUP, SMNT, QUIT, REIN, PORT, PASV, TYPE,
      STRU, MODE, RETR, STOR, STOU, APPE, ALLO, REST, RNFR, RNTO, ABOR,
      DELE, RMD, MKD, PWD, LIST, NLST, SITE, SYST, STAT, HELP, NOOP,
      NumCommands
    };

    enum RepresentationType {
      ASCII,
      EBCDIC,
      Image
    };

    enum DataChannelType {
      NormalPort,
      Passive
    };

    enum NameTypes {
      ShortNames,
      DetailedNames
    };

    BOOL SendPORT(
      const PIPSocket::Address & addr,  // Address for PORT connection.
      WORD port                         // Port number for PORT connection.
    );
    // Send the PORT command for a transfer.


  protected:
    PFTP();
    // Construct an inetern File Ttransfer Protocol channel.
};


class PFTPClient : public PFTP
{
  PCLASSINFO(PFTPClient, PFTP)
  public:
    PFTPClient();
    // Declare an FTP client socket.

    ~PFTPClient();
    // Delete and close the socket.


  // Overrides from class PSocket.
    virtual BOOL Close();
    /* Close the socket, and if connected as a client, QUITs from server.

       <H2>Returns:</H2>
       TRUE if the channel was closed and the QUIT accepted by the server.
     */


  // New functions for class
    BOOL LogIn(
      const PString & username,   // User name for FTP log in.
      const PString & password    // Password for the specified user name.
    );
    /* Log in to the remote host for FTP.

       <H2>Returns:</H2>
       TRUE if the log in was successfull.
     */

    PString GetSystemType();
    /* Get the type of the remote FTP server system, eg Unix, WindowsNT etc.

       <H2>Returns:</H2>
       String for the type of system.
     */

    BOOL SetType(
      RepresentationType type   // RepresentationTypeof file to transfer
    );
    /* Set the transfer type.

       <H2>Returns:</H2>
       TRUE if transfer type set.
     */

    BOOL ChangeDirectory(
      const PString & dirPath     // New directory
    );
    /* Change the current directory on the remote FTP host.

       <H2>Returns:</H2>
       TRUE if the log in was successfull.
     */

    PString GetCurrentDirectory();
    /* Get the current working directory on the remote FTP host.

       <H2>Returns:</H2>
       String for the directory path, or empty string if an error occurred.
     */

    PStringArray GetDirectoryNames(
      NameTypes type = ShortNames,        // Detail level on a directory entry.
      DataChannelType channel = Passive   // Data channel type.
    );
    PStringArray GetDirectoryNames(
      const PString & path,               // Name to get details for.
      NameTypes type = ShortNames,        // Detail level on a directory entry.
      DataChannelType channel = Passive   // Data channel type.
    );
    /* Get a list of files from the current working directory on the remote
       FTP host.

       <H2>Returns:</H2>
       String array for the files in the directory.
     */

    PString GetFileStatus(
      const PString & path,                // Path to get status for.
      DataChannelType channel = Passive    // Data channel type.
    );
    /* Get status information for the file path specified.

       <H2>Returns:</H2>
       String giving file status.
     */

    PTCPSocket * GetFile(
      const PString & filename,            // Name of file to get
      DataChannelType channel = NormalPort // Data channel type.
    );
    /* Begin retreiving a file from the remote FTP server. The second
       parameter indicates that the transfer is on a normal or passive data
       channel. In short, a normal transfer the server connects to the
       client and in passive mode the client connects to the server.

       <H2>Returns:</H2>
       Socket to read data from, or NULL if an error occurred.
     */

    PTCPSocket * PutFile(
      const PString & filename,   // Name of file to get
      DataChannelType channel = NormalPort // Data channel type.
    );
    /* Begin storing a file to the remote FTP server. The second parameter
       indicates that the transfer is on a normal or passive data channel.
       In short, a normal transfer the server connects to the client and in
       passive mode the client connects to the server.

       <H2>Returns:</H2>
       Socket to write data to, or NULL if an error occurred.
     */


  protected:
    BOOL OnOpen();
    PTCPSocket * NormalClientTransfer(
      Commands cmd,
      const PString & args
    );
    PTCPSocket * PassiveClientTransfer(
      Commands cmd,
      const PString & args
    );

    WORD remotePort;
};


class PFTPServer : public PFTP
{
  PCLASSINFO(PFTPServer, PFTP)
  public:
    enum { MaxIllegalPasswords = 3 };

    PFTPServer();
    PFTPServer(
      const PString & readyString   // Sign on string on connection ready.
    );
    // declare a server socket

    ~PFTPServer();
    // Delete the server, cleaning up passive sockets.


  // New functions for class
    virtual PString GetHelloString(const PString & user) const;
      // return the string printed when a user logs in
      // default value is a string giving the user name

    virtual PString GetGoodbyeString(const PString & user) const;
      // return the string printed just before exiting

    virtual PString GetSystemTypeString() const;
      // return the string to be returned by the SYST command

    BOOL GetAllowThirdPartyPort() const { return thirdPartyPort; }
      // return the thirdPartyPort flag, allowing 3 host put and get.

    void SetAllowThirdPartyPort(BOOL state) { thirdPartyPort = state; }
      // Set the thirdPartyPort flag.

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

    virtual void OnError(
      PINDEX errorCode, // Error code to use
      PINDEX cmdNum,    // Command that had the error.
      const char * msg  // Error message.
    );
    /* Handle an error in command.

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


  protected:
    BOOL OnOpen();
    void Construct();

    PString readyString;
    BOOL    thirdPartyPort;

    enum {
      NotConnected,
      NeedUser,
      NeedPassword,
      Connected,
      ClientConnect
    } state;

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
