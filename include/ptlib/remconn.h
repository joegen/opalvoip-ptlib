/*
 * $Id: remconn.h,v 1.1 1995/12/10 13:04:46 robertj Exp $
 *
 * Portable Windows Library
 *
 * Internet Protocol Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: remconn.h,v $
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
    PRemoteConnection(const PString & name);
    ~PRemoteConnection();

    virtual Comparison Compare(const PObject & obj) const;
    virtual PINDEX HashFunction() const;

    BOOL Open();
    BOOL Open(const PString & name);
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
    
  protected:
    PString  remoteName;

  private:
    PRemoteConnection(const PRemoteConnection &) { }
    void operator=(const PRemoteConnection &) { }
    void Construct();


// Class declaration continued in platform specific header file ///////////////
