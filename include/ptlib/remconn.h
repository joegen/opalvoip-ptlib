/*
 * $Id: remconn.h,v 1.6 1997/01/12 04:15:11 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
 * Revision 1.6  1997/01/12 04:15:11  robertj
 * Added ability to add/change new connections.
 *
 * Revision 1.5  1996/11/04 03:40:43  robertj
 * Added more debugging for remote drop outs.
 *
 * Revision 1.4  1996/08/11 07:03:45  robertj
 * Changed remote connection to late bind DLL.
 *
 * Revision 1.3  1996/04/23 11:33:04  robertj
 * Added username and password.
 *
 * Revision 1.2  1996/03/02 03:09:48  robertj
 * Added function to get all possible remote access connection names.
 *
 * Revision 1.1  1995/12/10 13:04:46  robertj
 * Initial revision
 *
 */

#define _PREMOTECONNECTION

#ifdef __GNUC__
#pragma interface
#endif


PDECLARE_CLASS(PRemoteConnection, PObject)
  public:
    PRemoteConnection();
    PRemoteConnection(
      const PString & name
    );
    // Create a new remote connection.

    ~PRemoteConnection();

    virtual Comparison Compare(const PObject & obj) const;
    virtual PINDEX HashFunction() const;

    BOOL Open();
    BOOL Open(
      const PString & name
    );
    BOOL Open(
      const PString & name,
      const PString & username,
      const PString & password
    );
    void Close();

    const PString & GetName() const { return remoteName; }

    enum Status {
      Idle,
      Connected,
      InProgress,
      LineBusy,
      NoDialTone,
      NoAnswer,
      PortInUse,
      NoNameOrNumber,
      GeneralFailure,
      ConnectionLost,
      NotInstalled,
      NumStatuses
    };
    Status GetStatus() const;


    static PStringArray GetAvailableNames();
    /* Get an array of names for all of the available remote connections on
       this system.

       <H2>Returns:</H2>
       Array of strings for remote connection names.
     */

    enum MultiChannelDialMode {

    };

    struct Configuration {
      PString device;
      PString phoneNumber;
      PString ipAddress;
      PString dnsAddress;
      PString script;
      PINDEX  subEntries;
      BOOL    dialAllSubEntries;
    };

    Status GetConfiguration(
      Configuration & config  // Configuration of remote connection
    );
    static Status GetConfiguration(
      const PString & name,   // Remote connection name to get configuration
      Configuration & config  // Configuration of remote connection
    );
    /* Get the configuration of the specified remote access connection.

       <H2>Returns:</H2>
       Connected if the configuration information was obtained,
       NoNameOrNumber if the particular RAS name does not exist,
       NotInstalled if there is no RAS support in the operating system,
       GeneralFailure on any other error.
     */

    Status SetConfiguration(
      const Configuration & config,  // Configuration of remote connection
      BOOL create = FALSE            // Flag to create connection if not present
    );
    static Status SetConfiguration(
      const PString & name,          // Remote connection name to configure
      const Configuration & config,  // Configuration of remote connection
      BOOL create = FALSE            // Flag to create connection if not present
    );
    /* Set the configuration of the specified remote access connection.

       <H2>Returns:</H2>
       Connected if the configuration information was obtained,
       NoNameOrNumber if the particular RAS name does not exist,
       NotInstalled if there is no RAS support in the operating system,
       GeneralFailure on any other error.
     */

    
  protected:
    PString remoteName;
    PString userName;
    PString password;

  private:
    PRemoteConnection(const PRemoteConnection &) { }
    void operator=(const PRemoteConnection &) { }
    void Construct();


// Class declaration continued in platform specific header file ///////////////
