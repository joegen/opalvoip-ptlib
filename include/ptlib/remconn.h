/*
 * $Id: remconn.h,v 1.3 1996/04/23 11:33:04 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
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
      InProgress,
      LineBusy,
      NoDialTone,
      NoAnswer,
      PortInUse,
      NoNameOrNumber,
      GeneralFailure,
      Connected,
      NumStatuses
    };
    Status GetStatus() const;


    static PStringArray GetAvailableNames();
    /* Get an array of names for all of the available remote connections on
       this system.

       <H2>Returns:</H2>
       Array of strings for remote connection names.
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
