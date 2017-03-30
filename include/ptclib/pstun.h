/*
 * pstun.h
 *
 * STUN client
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_PSTUN_H
#define PTLIB_PSTUN_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_STUN

#include <ptclib/pnat.h>
#include <ptlib/sockets.h>
#include <ptclib/cypher.h>


////////////////////////////////////////////////////////////////////////////////

class PSTUNMessage;
class PSTUNUDPSocket;

class PSTUN {
  public:
    enum {
      DefaultPort = 3478
    };

    enum {
      MinChannelNumber = 0x4000,
      MaxChannelNumber = 0x7ffe,
    };

    PSTUN();
    virtual ~PSTUN() { }

    /** Determine the NAT type using RFC3489 discovery method
      */
    virtual PNatMethod::NatTypes DoRFC3489Discovery(
      PSTUNUDPSocket * socket, 
      const PIPSocketAddressAndPort & serverAddress, 
      PIPSocketAddressAndPort & baseAddressAndPort, 
      PIPSocketAddressAndPort & externalAddressAndPort
    );

    virtual PNatMethod::NatTypes FinishRFC3489Discovery(
      PSTUNMessage & responseI,
      PSTUNUDPSocket * socket, 
      PIPSocketAddressAndPort & externalAddressAndPort
    );

    virtual int MakeAuthenticatedRequest(
      PSTUNUDPSocket * socket, 
      PSTUNMessage & request, 
      PSTUNMessage & response
    );

    virtual bool GetFromBindingResponse(
      const PSTUNMessage & response,
      PIPSocketAddressAndPort & externalAddress
    );

    virtual void AppendMessageIntegrity(
      PSTUNMessage & message
    );

    virtual bool ValidateMessageIntegrity(
      const PSTUNMessage & message
    );

    virtual void SetCredentials(
      const PString & username, 
      const PString & password, 
      const PString & realm       /// If empty the using short term credentials
    );

    /**Get the timeout for responses from STUN server.
      */
    virtual const PTimeInterval GetTimeout() const { return replyTimeout; }

    /**Set the timeout for responses from STUN server.
      */
    virtual void SetTimeout(
      const PTimeInterval & timeout   ///< New timeout in milliseconds
    ) { replyTimeout = timeout; }

    /**Get the number of retries for responses from STUN server.
      */
    virtual PINDEX GetRetries() const { return m_pollRetries; }

    /**Set the number of retries for responses from STUN server.
      */
    virtual void SetRetries(
      PINDEX retries    ///< Number of retries
    ) { m_pollRetries = retries; }

    PINDEX                  m_pollRetries;
    PSASLString             m_userName;
    PSASLString             m_realm;
    PSASLString             m_nonce;
    PBYTEArray              m_password; // text for short term, MD5 for long term
    PIPSocket::Address      m_interface;
    PIPSocketAddressAndPort m_serverAddress;
    PTimeInterval           replyTimeout;
};

////////////////////////////////////////////////////////////////////////////////

#pragma pack(1)

struct PSTUNAttribute
{
  enum Types {
    MAPPED_ADDRESS      = 0x0001,   // RFC 3489 & RFC 5389
    RESPONSE_ADDRESS    = 0x0002,   // RFC 3489 & RFC 5389
    CHANGE_REQUEST      = 0x0003,   // RFC 3489 & RFC 5389 (added in RFC 5780)
    SOURCE_ADDRESS      = 0x0004,   // RFC 3489
    CHANGED_ADDRESS     = 0x0005,   // RFC 3489
    USERNAME            = 0x0006,   // RFC 3489 & RFC 5389
    PASSWORD            = 0x0007,   // RFC 3489 (deprecated)
    MESSAGE_INTEGRITY   = 0x0008,   // RFC 3489 & RFC 5389
    ERROR_CODE          = 0x0009,   // RFC 3489 & RFC 5389
    UNKNOWN_ATTRIBUTES  = 0x000a,   // RFC 3489 & RFC 5389  
    REFLECTED_FROM      = 0x000b,   // RFC 3489  

    CHANNEL_NUMBER      = 0x000C,   // RFC 5766 
    LIFETIME            = 0x000D,   // RFC 5766 
    //BANDWIDTH         = 0x0010,
    XOR_PEER_ADDRESS    = 0x0012,   // RFC 5766 
    DATA                = 0x0013,   // RFC 5766 

    REALM               = 0x0014,   // RFC 5389  
    NONCE               = 0x0015,   // RFC 5389  

    XOR_RELAYED_ADDRESS = 0x0016,   // RFC 5766 
    EVEN_PORT           = 0x0018,   // RFC 5766 
    REQUESTED_TRANSPORT = 0x0019,   // RFC 5766 
    DONT_FRAGMENT       = 0x001A,   // RFC 5766
    XOR_MAPPED_ADDRESS  = 0x0020,   // RFC 5389  

    // TIMER-VAL        =  0x0021,
    RESERVATION_TOKEN   =  0x0022,  // RFC 5766

    PRIORITY            = 0x0024,   // RFC 5245 (ICE)
    USE_CANDIDATE       = 0x0025,   // RFC 5245 (ICE)
    ICE_CONTROLLED      = 0x8029,   // RFC 5245 (ICE)
    ICE_CONTROLLING     = 0x802A,   // RFC 5245 (ICE)

    PADDING             = 0x0026,   // RFC 5389 (added in RFC 5780)
    RESPONSE_PORT       = 0x0027,   // RFC 5389 (added in RFC 5780)

    FINGERPRINT         = 0x8028,   // RFC 5389

    ALTERNATE_SERVER    = 0x8023,   // RFC 5389

    RESPONSE_ORIGIN     = 0x802b,   // RFC 5389 (added in RFC 5780)
    OTHER_ADDRESS       = 0x802c    // RFC 5389 (added in RFC 5780)
  };
  
  PUInt16b type;
  PUInt16b length;

  PSTUNAttribute(Types newType = ERROR_CODE, size_t len = 0)
    : type((WORD)newType)
    , length((WORD)len)
  {
  }

  PSTUNAttribute * GetNext() const;
};

////////////////////////////////////////////////////////////////////////////////

class PSTUNAddressAttribute : public PSTUNAttribute
{
  protected:
    BYTE     pad;
    BYTE     family;
    PUInt16b port;
    BYTE     ip[4];

  public:
    PSTUNAddressAttribute(Types newType, const PIPSocketAddressAndPort & addrAndPort)
      : PSTUNAttribute(newType, SizeofAddressAttribute)
      , pad(0)
      , family(1)
    {
      SetIPAndPort(addrAndPort);
    }

    WORD GetPort() const;
    PIPSocket::Address GetIP() const;
    void SetIPAndPort(const PIPSocketAddressAndPort & addrAndPort);
    void GetIPAndPort(PIPSocketAddressAndPort & addrAndPort);

  protected:
    enum { SizeofAddressAttribute = sizeof(BYTE)+sizeof(BYTE)+sizeof(WORD)+4 };
    bool IsValidAddrAttr(Types checkType) const { return type == checkType && length == SizeofAddressAttribute; }
};


class PSTUNStringAttribute : public PSTUNAttribute
{
  public:
    char m_string[763];   // not actually 763, but this will do

    PSTUNStringAttribute(Types newType, const PString & str)
      : PSTUNAttribute(newType, str.GetLength())
    {
      memcpy(m_string, (const char *)str, length);
    }

    PString GetString() const
    {
      return PString(m_string, length);
    }

    bool IsValidStringAttr(Types checkType) const
    {
      return (type == checkType) && (length == strlen(m_string));
    }
};


class PSTUNChangeRequest : public PSTUNAttribute
{
  public:
    BYTE flags[4];
    
    PSTUNChangeRequest(bool changeIP = false, bool changePort = false)
      : PSTUNAttribute(CHANGE_REQUEST, sizeof(flags))
    {
      memset(flags, 0, sizeof(flags));
      SetChangeIP(changeIP);
      SetChangePort(changePort);
    }

    bool IsValid() const { return type == CHANGE_REQUEST && length == sizeof(flags); }
    
    bool GetChangeIP() const { return (flags[3]&4) != 0; }
    void SetChangeIP(bool on) { if (on) flags[3] |= 4; else flags[3] &= ~4; }
    
    bool GetChangePort() const { return (flags[3]&2) != 0; }
    void SetChangePort(bool on) { if (on) flags[3] |= 2; else flags[3] &= ~2; }
};


class PSTUNMessageIntegrity : public PSTUNAttribute
{
  public:
    BYTE m_hmac[PHMAC::KeyLength];

    PSTUNMessageIntegrity(const BYTE * hmac = NULL)
      : PSTUNAttribute(MESSAGE_INTEGRITY, sizeof(m_hmac))
    {
      if (hmac == NULL)
        memset(m_hmac, 0, sizeof(m_hmac));
      else
        memcpy(m_hmac, hmac, sizeof(m_hmac));
    }

    bool IsValid() const { return type == MESSAGE_INTEGRITY && length == sizeof(m_hmac); }
};


class PSTUNFingerprint : public PSTUNAttribute
{
  public:
    PUInt32b m_crc;

    PSTUNFingerprint(DWORD crc = 0)
      : PSTUNAttribute(FINGERPRINT, sizeof(m_crc))
      , m_crc(crc)
    {
    }

    bool IsValid() const { return type == FINGERPRINT && length == sizeof(m_crc); }
};


class PSTUNErrorCode : public PSTUNAttribute
{
  public:
    PSTUNErrorCode()
    {
      Initialise();
    }

    PSTUNErrorCode(int code, const PString & reason = PString::Empty())
    {
      Initialise();
      SetErrorCode(code, reason);
    }

    BYTE m_zero1;
    BYTE m_zero2;
    BYTE m_hundreds;
    BYTE m_units;
    char m_reason[256];   // not actually 256, but this will do
   
    void Initialise();
    void SetErrorCode(int code, const PString & reason);
    int GetErrorCode() const { return (m_hundreds * 100) + m_units; }
    PString GetReason() { return PString(m_reason); }
    bool IsValid() const { return (type == ERROR_CODE) && (length == 4 + strlen(m_reason) + 1); }
};

struct PSTUNChannelNumber : public PSTUNAttribute
{
  PSTUNChannelNumber()
  { Initialise(); }

  PUInt16b       m_channelNumber;
  PUInt16b       m_rffu;

  void Initialise();
};

struct PSTUNMessageHeader
{
  PUInt16b       msgType;
  PUInt16b       msgLength;
  BYTE           transactionId[16];
};

#pragma pack()

////////////////////////////////////////////////////////////////////////////////


class PSTUNClient;

/**UDP socket that has been created by the STUN client.
  */
class PSTUNUDPSocket : public PNATUDPSocket
{
  PCLASSINFO(PSTUNUDPSocket, PNATUDPSocket);
  public:
    PSTUNUDPSocket(
      PNatMethod::Component component
    );

    virtual const char * GetNatName() const;

    bool OpenSTUN(PSTUNClient & client);
    virtual void GetCandidateInfo(PNatCandidate & candidate);

    bool BaseWriteTo(const void * buf, PINDEX len, const PIPSocketAddressAndPort & ap)
    { Slice slice((void *)buf, len); return PUDPSocket::InternalWriteTo(&slice, 1, ap); }

    bool BaseReadFrom(void * buf, PINDEX len, PIPSocketAddressAndPort & ap)
    { Slice slice(buf, len); return PUDPSocket::InternalReadFrom(&slice, 1, ap); }

  protected:
    friend class PSTUN;
    friend class PSTUNClient;

    PIPSocketAddressAndPort m_serverReflexiveAddress;
    PIPSocketAddressAndPort m_baseAddressAndPort;

    bool InternalGetLocalAddress(PIPSocketAddressAndPort & addr);

  private:
    PNatMethod::NatTypes m_natType;
};

////////////////////////////////////////////////////////////////////////////////

class PSTUNMessage : public PBYTEArray
{
  public:
    enum MsgType {
      InvalidMessage,

      BindingRequest        = 0x0001,
      BindingResponse       = 0x0101,
      BindingError          = 0x0111,
        
      SharedSecretRequest   = 0x0002,
      SharedSecretResponse  = 0x0102,
      SharedSecretError     = 0x0112,

      // RFC 5766
      Allocate              = 0x0003,  
      AllocateResponse      = 0x0103,
      AllocateError         = 0x0113,

      Refresh               = 0x0004,
      RefreshResponse       = 0x0104,
      RefreshError          = 0x0114,

      Send                  = 0x0016,
      Data                  = 0x0017,

      CreatePermission      = 0x0008,
      CreatePermResponse    = 0x0108,
      CreatePermError       = 0x0118,

      ChannelBind           = 0x0009,
      ChannelBindResponse   = 0x0109,
      ChannelBindError      = 0x0119,

      // RFC6062
      Connect               = 0x000a,
      ConnectResponse       = 0x010a,
      ConnectError          = 0x011a,

      ConnectionBind        = 0x000b,
      ConnectionBindResponse= 0x010b,
      ConnectionBindError   = 0x011b,

      ConnectionAttempt     = 0x001c,
    };
    
    PSTUNMessage();
    PSTUNMessage(MsgType newType, const BYTE * id = NULL);
    PSTUNMessage(const BYTE * data, PINDEX size, const PIPSocketAddressAndPort & srcAddr);

#if PTRACING
    void PrintOn(ostream & strm) const;
#endif

    bool IsValid() const;
    bool IsValidFor(const PSTUNMessage & request) const;

    PBYTEArray GetTransactionID() const;

    bool IsRFC5389() const;

    const PSTUNMessageHeader * operator->() const { return (const PSTUNMessageHeader *)theArray; }

    MsgType GetType() const;
    void SetType(MsgType newType, const BYTE * id = NULL);
    void SetErrorType(int code, const BYTE * id, const char * reason = NULL);

    bool IsRequest()         const { return (GetType() & 0x0110) == 0x0000; }
    bool IsIndication()      const { return (GetType() & 0x0110) == 0x0010; }
    bool IsSuccessResponse() const { return (GetType() & 0x0110) == 0x0100; }
    bool IsErrorResponse()   const { return (GetType() & 0x0110) == 0x0110; }

    PSTUNAttribute * AddAttribute(const PSTUNAttribute & attribute);
    PSTUNAttribute * SetAttribute(const PSTUNAttribute & attribute);
    PSTUNAttribute * FindAttribute(PSTUNAttribute::Types type) const;

    template <class Type> Type * FindAttributeAs(PSTUNAttribute::Types type) const
    { return static_cast<Type *>(FindAttribute(type)); }

    PString FindAttributeString(PSTUNAttribute::Types type, const char * dflt = NULL) const;

    bool Read(PUDPSocket & socket);
    bool Write(PUDPSocket & socket) const;
    bool Write(PUDPSocket & socket, const PIPSocketAddressAndPort & ap) const;
    bool Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries);

    const PIPSocketAddressAndPort GetSourceAddressAndPort() const { return m_sourceAddressAndPort; }

    void AddMessageIntegrity(const PBYTEArray & credentialsHash) { AddMessageIntegrity(credentialsHash, credentialsHash.GetSize()); }
    void AddMessageIntegrity(const BYTE * credentialsHashPtr, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi = NULL);

    bool CheckMessageIntegrity(const PBYTEArray & credentialsHash) const { return CheckMessageIntegrity(credentialsHash, credentialsHash.GetSize()); }
    bool CheckMessageIntegrity(const BYTE * credentialsHashPtr, PINDEX credentialsHashLen) const;

    void AddFingerprint(PSTUNFingerprint * fp = NULL);
    bool CheckFingerprint(bool required) const;

  protected:
    PSTUNAttribute * GetFirstAttribute() const;
    void CalculateMessageIntegrity(const BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi, BYTE * hmac) const;
    DWORD CalculateFingerprint(PSTUNFingerprint * fp) const;

    PIPSocketAddressAndPort m_sourceAddressAndPort;
};

////////////////////////////////////////////////////////////////////////////////

/**STUN client.
  */
class PSTUNClient : public PNatMethod, public PSTUN
{
    PCLASSINFO(PSTUNClient, PNatMethod);
  public:
    enum { DefaultPriority = 40 };
    PSTUNClient(unsigned priority = DefaultPriority);
    ~PSTUNClient();

    /**Get the NAT Method Name
     */
    static const char * MethodName();
    virtual PCaselessString GetMethodName() const;

    /**Set the STUN server to use.
       The server string may be of the form host:port. If :port is absent
       then the default port 3478 is used. The substring port can also be
       a service name as found in /etc/services. The host substring may be
       a DNS name or explicit IP address.
      */
    bool SetServer(
      const PString & server
    );

    /**Set the authentication credentials.
      */
    virtual void SetCredentials(
      const PString & username, 
      const PString & password, 
      const PString & realm
    );

    /**Get the current server address name.
       Defaults to be "address:port" string form.
      */
    virtual PString GetServer() const;

    virtual bool GetServerAddress(
      PIPSocketAddressAndPort & serverAddressAndPort 
    ) const;

    virtual bool GetInterfaceAddress(
      PIPSocket::Address & internalAddress
    ) const;

    virtual bool Open(
      const PIPSocket::Address & ifaceAddr
    );

    virtual bool IsAvailable(
      const PIPSocket::Address & binding,
      PObject * userData
    );

    virtual void Close();

    /**Create a single socket.
       The STUN protocol is used to create a socket for which the external IP
       address and port numbers are known. A PUDPSocket descendant is returned
       which will, in response to GetLocalAddress() return the externally
       visible IP and port rather than the local machines IP and socket.

       The will create a new socket pointer. It is up to the caller to make
       sure the socket is deleted to avoid memory leaks.

       The socket pointer is set to NULL if the function fails and returns
       false.
      */
    virtual bool CreateSocket(
      PUDPSocket * & socket,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      WORD localPort = 0,
      PObject * context = NULL,
      Component component = eComponent_Unknown
    );

    /**Create a socket pair.
       The STUN protocol is used to create a pair of sockets with adjacent
       port numbers for which the external IP address and port numbers are
       known. PUDPSocket descendants are returned which will, in response
       to GetLocalAddress() return the externally visible IP and port rather
       than the local machines IP and socket.

       The will create new socket pointers. It is up to the caller to make
       sure the sockets are deleted to avoid memory leaks.

       The socket pointers are set to NULL if the function fails and returns
       false.
      */
    virtual bool CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      PObject * context = NULL
    );

  protected:
    virtual PNATUDPSocket * InternalCreateSocket(Component component, PObject * context);
    virtual void InternalUpdate();
    bool InternalSetServer(const PIPSocketAddressAndPort & addr);

    PSTUNUDPSocket * m_socket;

  private:
    PString m_serverName;
    PINDEX  m_numSocketsForPairing;
};


////////////////////////////////////////////////////////////////////////////////

/**TURN client.
  */

class PTURNRequestedTransport : public PSTUNAttribute
{
  public:
    enum {
      ProtocolUDP = IPPROTO_UDP,
      ProtocolTCP = IPPROTO_TCP
    };
    BYTE m_protocol;
    BYTE m_rffu1;
    BYTE m_rffu2;
    BYTE m_rffu3;

    PTURNRequestedTransport(BYTE protocol = ProtocolUDP)
    { Initialise(protocol); }
   
    void Initialise(BYTE protocol = ProtocolUDP);
    bool IsValid() const { return (type == REQUESTED_TRANSPORT) && (length == 4); }
};


class PTURNLifetime : public PSTUNAttribute
{
  public:
    PUInt32b m_lifetime;

    PTURNLifetime(DWORD lifetime = 600)
      : m_lifetime(lifetime)
    { type = LIFETIME; length = 4; }
   
    bool IsValid() const { return (type == LIFETIME) && (length == 8); }

    DWORD GetLifetime() const { return m_lifetime; }
};


class PTURNEvenPort : public PSTUNAttribute
{
  public:
    BYTE m_bits;

    PTURNEvenPort(bool evenPort = true)
    { type = EVEN_PORT; m_bits = evenPort ? 1 : 0; length = 1; }
   
    bool IsValid() const { return (type == EVEN_PORT) && (length == 1); }

    bool IsEven() const { return (m_bits & 1) != 0; }
};

///////////////////////////////////////////////////////

class PTURNClient;

#pragma pack(1)

struct PTURNChannelHeader {
  PUInt16b m_channelNumber;
  PUInt16b m_length;
};

#pragma pack()

class PTURNUDPSocket : public PSTUNUDPSocket, public PSTUN
{
  public:

    friend class PTURNClient;

    PTURNUDPSocket(
      PNatMethod::Component component
    );
    ~PTURNUDPSocket();

    virtual const char * GetNatName() const;

    virtual bool Close();

    virtual void GetCandidateInfo(PNatCandidate & candidate);

    int OpenTURN(PTURNClient & client);

  protected:
    bool InternalGetLocalAddress(PIPSocketAddressAndPort & addr);
    bool InternalWriteTo(const Slice * slices, size_t sliceCount, const PIPSocketAddressAndPort & ipAndPort);
    bool InternalReadFrom(Slice * slices, size_t sliceCount, PIPSocketAddressAndPort & ipAndPort);
    void InternalSetSendAddress(const PIPSocketAddressAndPort & addr);
    void InternalGetSendAddress(PIPSocketAddressAndPort & addr);

    bool m_allocationMade;
    int m_channelNumber;
    bool m_usingTURN;

    BYTE m_protocol;
    PIPSocketAddressAndPort m_relayedAddress;
    DWORD m_lifeTime;
    PIPSocketAddressAndPort m_peerIpAndPort;

    std::vector<Slice> m_txVect;
    PTURNChannelHeader m_txHeader;
    BYTE m_txPadding[4];

    std::vector<Slice> m_rxVect;
    PTURNChannelHeader m_rxHeader;
    BYTE m_rxPadding[4];
};

///////////////////////////////////////////////////////

class PTURNClient : public PSTUNClient
{
    PCLASSINFO(PTURNClient, PSTUNClient);
  public:

    friend class PTURNUDPSocket;

    /**Get the NAT Method Name
     */
    static const char * MethodName();
    virtual PCaselessString GetMethodName() const;

    enum { DefaultPriority = 30 };
    PTURNClient(unsigned priority = DefaultPriority);

    // overrides from PNatMethod
    bool CreateSocket(
      PUDPSocket * & socket,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      WORD localPort = 0,
      PObject * context = NULL,
      Component component = eComponent_Unknown
    );

    bool CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      PObject * context = NULL
    );

    /**Return an indication if the current STUN type supports RTP
      Use the force variable to guarantee an up to date test
      */
    virtual RTPSupportTypes GetRTPSupport(
      bool force = false    ///< Force a new check
    );

  protected:
    virtual PNATUDPSocket * InternalCreateSocket(Component component, PObject * context);
    // New functions
    virtual bool RefreshAllocation(DWORD lifetime = 600);
};


#endif // P_STUN

#endif // PTLIB_PSTUN_H
