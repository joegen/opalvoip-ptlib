/*
 * $Id: mail.h,v 1.2 1995/04/01 08:27:57 robertj Exp $
 *
 * Portable Windows Library
 *
 * Mail Interface
 *
 * Copyright 1993 Equivalence
 *
 * $Log: mail.h,v $
 * Revision 1.2  1995/04/01 08:27:57  robertj
 * Added GUI support.
 *
 * Revision 1.1  1995/03/14  12:44:11  robertj
 * Initial revision
 *
 */

#define _PMAILSESSION

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PMailSession, PObject)
/* This class establishes a mail session with the platforms mail system.
*/

  public:
    PMailSession();
    PMailSession(
      const PString & username,  // User withing mail system to use.
      const PString & password   // Password for user in mail system.
    );
    PMailSession(
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


    virtual ~PMailSession();
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
