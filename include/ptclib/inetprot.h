/*
 * $Id: inetprot.h,v 1.7 1996/03/16 04:35:32 robertj Exp $
 *
 * Portable Windows Library
 *
 * Application Socket Class Declarations
 *
 * Copyright 1995 Equivalence
 *
 * $Log: inetprot.h,v $
 * Revision 1.7  1996/03/16 04:35:32  robertj
 * Redesigned response codes to be more flexible.
 *
 * Revision 1.6  1996/02/13 12:57:05  robertj
 * Added access to the last response in an application socket.
 *
 * Revision 1.5  1996/02/03 11:33:16  robertj
 * Changed RadCmd() so can distinguish between I/O error and unknown command.
 *
 * Revision 1.4  1996/01/23 13:08:43  robertj
 * Major rewrite for HTTP support.
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

#ifndef _PAPPLICATIONSOCKET
#define _PAPPLICATIONSOCKET

#ifdef __GNUC__
#pragma interface
#endif

#include <sockets.h>


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


  // Overrides from class PChannel.
    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel.

       This override also supports the mechanism in the <A>UnRead()</A>
       function allowing characters to be be "put back" into the data stream.
       This allows a look-ahead required by the logic of some protocols. This
       is completely independent of the standard iostream mechanisms which do
       not support the level of timeout control required by the protocols.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel.

       This override assures that the sequence CR/LF/./CR/LF does not occur by
       byte stuffing an extra '.' character into the data stream, whenever a
       line begins with a '.' character.

       Note that this only occurs if the member variable
       <CODE>stuffingState</CODE> has been set to some value other than
       <CODE>DontStuff</CODE>, usually <CODE>StuffIdle</CODE>. Also, if the
       <CODE>newLineToCRLF</CODE> member variable is TRUE then all occurrences
       of a '\n' character will be translated to a CR/LF pair.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
     */


  // New functions for class.
    BOOL WriteLine(
      const PString & line // String to write as a command line.
    );
    /* Write a string to the socket channel followed by a CR/LF pair. If there
       are any lone CR or LF characters in the <CODE>line</CODE> parameter
       string, then these are translated into CR/LF pairs.

       <H2>Returns:</H2>
       TRUE if the string and CR/LF were completely written.
     */

    BOOL ReadLine(
      PString & line,             // String to receive a CR/LF terminated line.
      BOOL allowContinuation = FALSE  // Flag to handle continued lines.
    );
    /* Read a string from the socket channel up to a CR/LF pair.
    
       If the <CODE>unstuffLine</CODE> parameter is set then the function will
       remove the '.' character from the start of any line that begins with
       two consecutive '.' characters. A line that has is exclusively a '.'
       character will make the function return FALSE.

       Note this function will block for the time specified by the
       <A>PChannel::SetReadTimeout()</A> function for only the first character
       in the line. The rest of the characters must each arrive within the time
       set by the <CODE>readLineTimeout</CODE> member variable. The timeout is
       set back to the original setting when the function returns.

       <H2>Returns:</H2>
       TRUE if a CR/LF pair was received, FALSE if a timeout or error occurred.
     */

    void UnRead(
      int ch                // Individual character to be returned.
    );
    void UnRead(
      const PString & str   // String to be put back into data stream.
    );
    void UnRead(
      const void * buffer,  // Characters to be put back into data stream.
      PINDEX len            // Number of characters to be returned.
    );
    /* Put back the characters into the data stream so that the next
       <A>Read()</A> function call will return them first.
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

    BOOL ReadCommand(
      PINDEX & num,
       // Number of the command parsed from the command line, or P_MAX_INDEX
       // if no match.
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
       TRUE if something was read, otherwise an I/O error occurred.
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
      int & code,      // Response code for command response.
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

    int ExecuteCommand(
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

    int GetLastResponseCode() const;
    /* Return the code associated with the last response received by the
       socket.

       <H2>Returns:</H2>
       Response code
    */

    PString GetLastResponseInfo() const;
    /* Return the last response received by the socket.

       <H2>Returns:</H2>
       Response as a string
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


    PStringArray commandNames;
    // Names of each of the command codes.

    PCharArray unReadBuffer;
    // Buffer for characters put back into the data stream.

    PTimeInterval readLineTimeout;
    // Time for characters in a line to be received.

    enum StuffState {
      DontStuff, StuffIdle, StuffCR, StuffCRLF, StuffCRLFdot, StuffCRLFdotCR
    } stuffingState;
    // Do byte stuffing of '.' characters in output to the socket channel.

    BOOL newLineToCRLF;
    // Translate \n characters to CR/LF pairs.

    int     lastResponseCode;
    PString lastResponseInfo;
    // Responses


  private:
    void Construct();
};



#endif


// End Of File ///////////////////////////////////////////////////////////////
