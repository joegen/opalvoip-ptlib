/*
 * $Id: mail.h,v 1.3 1995/06/17 00:42:22 robertj Exp $
 *
 * Portable Windows Library
 *
 * Mail Interface
 *
 * Copyright 1993 Equivalence
 *
 * $Log: mail.h,v $
 * Revision 1.3  1995/06/17 00:42:22  robertj
 * Added mail reading interface.
 * Changed name to simply PMail
 *
 * Revision 1.2  1995/04/01 08:27:57  robertj
 * Added GUI support.
 *
 * Revision 1.1  1995/03/14  12:44:11  robertj
 * Initial revision
 *
 */

#define _PMAIL

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PMail, PObject)
/* This class establishes a mail session with the platforms mail system.
*/

  public:
    PMail();
    PMail(
      const PString & username,  // User withing mail system to use.
      const PString & password   // Password for user in mail system.
    );
    PMail(
      const PString & username,  // User withing mail system to use.
      const PString & password,  // Password for user in mail system.
      const PString & service
      /* A platform dependent string indicating the location of the underlying
         messaging service, eg the path to a message store or node name of the
         mail server.
       */
    );
    /* Create a mail session. The parameterless form does not log into the
       mail system. The second form attempts to log in using the parameters
       provided.
     */


    virtual ~PMail();
    /* Destroy the mail session, logging off the mail system if necessary.
     */


  // New functions for class
    BOOL LogOn(
      const PString & username,  // User withing mail system to use.
      const PString & password   // Password for user in mail system.
    );
    BOOL LogOn(
      const PString & username,  // User withing mail system to use.
      const PString & password,  // Password for user in mail system.
      const PString & service
      /* A platform dependent string indicating the location of the underlying
         messaging service, eg the path to a message store or node name of the
         mail server.
       */
    );
    /* Attempt to log on to the mail system using the parameters provided.

       <H2>Returns:</H2>
       TRUE if successfully logged on.
     */

    virtual BOOL LogOff();
    /* Log off from the mail system.

       <H2>Returns:</H2>
       TRUE if successfully logged off.
     */

    BOOL IsLoggedOn() const;
    /* Determine if the mail session is active and logged into the mail system.

       <H2>Returns:</H2>
       TRUE if logged into the mail system.
     */


    BOOL SendNote(
      const PString & recipient,  // Name of recipient of the mail message.
      const PString & subject,    // Subject name for the mail message.
      const char * body           // Text body of the mail message.
    );
    BOOL SendNote(
      const PStringList & recipients, // Name of recipient of the mail message.
      const PString & subject,        // Subject name for the mail message.
      const char * body               // Text body of the mail message.
    );
    /* Send a new simple mail message.

       <H2>Returns:</H2>
       TRUE if the mail message was successfully queued. Note that this does
       <EM>not</EM> mean that it has been delivered.
     */


    PStringArray GetMessageIDs(
      BOOL unreadOnly = TRUE    // Only get the IDs for unread messages.
    );
    /* Get a list of ID strings for all messages in the mail box.

       <H2>Returns:</H2>
       An array of ID strings.
     */

    struct Header {
      PString  subject;           // Subject for message.
      PString  originatorName;    // Full name of message originator.
      PString  originatorAddress; // Return address of message originator.
      PTime    received;          // Time message received.
      unsigned attachments;       // Number of attachment files.
    };

    BOOL GetMessageHeader(
      const PString & id,      // Identifier of message to get header.
      Header & hdrInfo,        // Header info for the message.
      BOOL markAsRead = FALSE  // Mark the message as read
    );
    /* Get the header information for a message.

       <H2>Returns:</H2>
       TRUE if header information was successfully obtained.
     */

    PString GetMessageBody(
      const PString & id,      // Identifier of message to get body.
      BOOL markAsRead = FALSE  // Mark the message as read
    );
    /* Get the body text for a message.

       Note that to tell between an error getting the message body and simply
       having an empty message body the <A>GetErrorCode()</A> function must
       be used.

       <H2>Returns:</H2>
       The body text was successfully or an empty string if an error occurred.
     */

    BOOL GetMessageAttachments(
      const PString & id,       // Identifier of message to get attachments.
      PStringArray & filenames, // Body text for message.
      BOOL includeBody = FALSE, // Include the message body as first attachment
      BOOL markAsRead = FALSE   // Mark the message as read
    );
    /* Get all of the attachments for a message as disk files.

       <H2>Returns:</H2>
       TRUE if attachments were successfully obtained.
     */

    BOOL MarkMessageRead(
      const PString & id      // Identifier of message to get header.
    );
    /* Mark the message as read.

       <H2>Returns:</H2>
       TRUE if message was successfully marked as read.
     */

    BOOL DeleteMessage(
      const PString & id      // Identifier of message to get header.
    );
    /* Delete the message from the system.

       <H2>Returns:</H2>
       TRUE if message was successfully deleted.
     */


    enum LookUpResult {
      UnknownUser,    // User name is unknown in mail system.
      AmbiguousUser,  // User is ambiguous in mail system.
      ValidUser,      // User is a vlid, unique name in mail system.
      LookUpError     // An error occurred during the look up
    };

    LookUpResult LookUp(
      const PString & name,  // Name to look up.
      PString * fullName = NULL
      /* String to receive full name of user passed in <CODE>name</CODE>. If
         NULL then the full name is <EM>not</EM> returned.
       */
    );
    /* Look up the specified name and verify that they are a valid address in
       the mail system.

       <H2>Returns:</H2>
       result of the name lookup.
     */


    int GetErrorCode() const;
    /* Get the internal error code for the last error by a function in this
       mail session.

       <H2>Returns:</H2>
       integer error code for last operation.
     */

    PString GetErrorText() const;
    /* Get the internal error description for the last error by a function in
       this mail session.

       <H2>Returns:</H2>
       string error text for last operation.
     */


  protected:
    void Construct();
    // Common construction code.

    BOOL loggedOn;
    // Flag indicating the session is active.


// Class declaration continued in platform specific header file ///////////////
