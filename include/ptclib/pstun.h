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
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PSTUN_H
#define PTLIB_PSTUN_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptclib/pnat.h>
#include <ptlib/sockets.h>

////////////////////////////////////////////////////////////////////////////////

class PSTUN {
  public:
    enum {
      DefaultPort = 3478
    };

    enum NatTypes {
      UnknownNat,
      OpenNat,
      ConeNat,
      RestrictedNat,
      PortRestrictedNat,
      SymmetricNat,
      SymmetricFirewall,
      BlockedNat,
      PartialBlockedNat,
      NumNatTypes
    };

    /**Get NatTypes enumeration as an English string for the type.
      */
    static PString GetNatTypeString(
      NatTypes type   ///< NAT Type to get name of
    );
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
    PASSWORD            = 0x0007,   // RFC 3489
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

    PADDING             = 0x0026,   // RFC 5389 (added in RFC 5780)
    RESPONSE_PORT       = 0x0027,   // RFC 5389 (added in RFC 5780)

    RESPONSE_ORIGIN     = 0x802b,   // RFC 5389 (added in RFC 5780)
    OTHER_ADDRESS       = 0x802c    // RFC 5389 (added in RFC 5780)
  };
  
  PUInt16b type;
  PUInt16b length;
  
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
    void InitAddrAttr(Types newType)
    {
      pad    = 0;
      family = 1;
      type   = (WORD)newType;
      length = SizeofAddressAttribute;
    }

    WORD GetPort() const;
    PIPSocket::Address GetIP() const;
    void SetIPAndPort(const PIPSocketAddressAndPort & addrAndPort);
    void GetIPAndPort(PIPSocketAddressAndPort & addrAndPort);

  protected:
    enum { SizeofAddressAttribute = sizeof(BYTE)+sizeof(BYTE)+sizeof(WORD)+4 };
    bool IsValidAddrAttr(Types checkType) const
    {
      return type == checkType && length == SizeofAddressAttribute;
    }
};


class PSTUNStringAttribute : public PSTUNAttribute
{
  public:
    char m_string[763];   // not actually 763, but this will do

    PSTUNStringAttribute(Types newType, const PString & str)
    { InitStringAttr(newType, str); }

    PString GetString() const { return PString(m_string, length); }

    void InitStringAttr(Types newType, const PString & str)
    {
      type   = (WORD)newType;
      length = (WORD)str.GetLength();
      memcpy(m_string, (const char *)str, length);
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
    
    PSTUNChangeRequest() { }

    PSTUNChangeRequest(bool changeIP, bool changePort)
    {
      Initialise();
      SetChangeIP(changeIP);
      SetChangePort(changePort);
    }

    void Initialise()
    {
      type = CHANGE_REQUEST;
      length = sizeof(flags);
      memset(flags, 0, sizeof(flags));
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
    BYTE hmac[20];

    PSTUNMessageIntegrity(const BYTE * data = NULL)
    { Initialise(data); }
    
    void Initialise(const BYTE * data = NULL)
    {
      type = MESSAGE_INTEGRITY;
      length = sizeof(hmac);
      if (data == NULL)
        memset(hmac, 0, sizeof(hmac));
      else
        memcpy(hmac, data, sizeof(hmac));
    }
    bool IsValid() const { return type == MESSAGE_INTEGRITY && length == sizeof(hmac); }
};

class PSTUNErrorCode : public PSTUNAttribute
{
  public:
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

struct PSTUNMessageHeader
{
  PUInt16b       msgType;
  PUInt16b       msgLength;
  BYTE           transactionId[16];
};

#pragma pack()

////////////////////////////////////////////////////////////////////////////////

/**UDP socket that has been created by the STUN client.
  */
class PSTUNUDPSocket : public PUDPSocket
{
  PCLASSINFO(PSTUNUDPSocket, PUDPSocket);
  public:
    PSTUNUDPSocket();

    virtual PBoolean GetLocalAddress(
      Address & addr    ///< Variable to receive hosts IP address
    );
    virtual PBoolean GetLocalAddress(
      Address & addr,    ///< Variable to receive peer hosts IP address
      WORD & port        ///< Variable to receive peer hosts port number
    );

  protected:
    PIPSocket::Address externalIP;

  friend class PSTUNClient;
};

////////////////////////////////////////////////////////////////////////////////

class PSTUNMessage : public PBYTEArray
{
  public:
    enum MsgType {
      BindingRequest        = 0x0001,
      BindingResponse       = 0x0101,
      BindingError          = 0x0111,
        
      SharedSecretRequest   = 0x0002,
      SharedSecretResponse  = 0x0102,
      SharedSecretError     = 0x0112,

      Allocate              = 0x003,  // RFC 5766
      AllocateResponse      = 0x103,  // RFC 5766
      AllocateError         = 0x113,  // RFC 5766

      Refresh               = 0x004,  // RFC 5766
      Send                  = 0x006,  // RFC 5766
      Data                  = 0x007,  // RFC 5766
      CreatePermission      = 0x008,  // RFC 5766
      ChannelBind           = 0x009,  // RFC 5766

    };
    
    PSTUNMessage();
    PSTUNMessage(MsgType newType, const BYTE * id = NULL);

    void SetType(MsgType newType, const BYTE * id = NULL);
    MsgType GetType() const;

    const BYTE * GetTransactionID() const;

    const PSTUNMessageHeader * operator->() const { return (const PSTUNMessageHeader *)theArray; }

    PSTUNAttribute * GetFirstAttribute() const;

    bool Validate();
    bool Validate(const PSTUNMessage & request);

    PSTUNAttribute * AddAttribute(const PSTUNAttribute & attribute);
    PSTUNAttribute * SetAttribute(const PSTUNAttribute & attribute);
    PSTUNAttribute * FindAttribute(PSTUNAttribute::Types type) const;

    template <class Type> Type * FindAttributeOfType(PSTUNAttribute::Types type) const
    { return static_cast<Type *>(FindAttribute(type)); }

    bool Read(PUDPSocket & socket);
    bool Write(PUDPSocket & socket) const;
    bool Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries);

    bool IsRFC5389() const { return m_isRFC5389; }

    const PIPSocketAddressAndPort GetSourceAddressAndPort() const { return m_sourceAddressAndPort; }

    void InsertMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen);
    void InsertMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi);

    bool CheckMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen);
    void CalculateMessageIntegrity(BYTE * credentialsHash, PINDEX credentialsHashLen, PSTUNMessageIntegrity * mi, BYTE * hmac);

  protected:
    PIPSocketAddressAndPort m_sourceAddressAndPort;
    bool m_isRFC5389;
};

////////////////////////////////////////////////////////////////////////////////

/**STUN client.
  */
class PSTUNClient : public PNatMethod, public PSTUN
{
  PCLASSINFO(PSTUNClient, PNatMethod);
  public:
    PSTUNClient();

    PSTUNClient(
      const PString & server,
      WORD portBase = 0,
      WORD portMax = 0,
      WORD portPairBase = 0,
      WORD portPairMax = 0
    );
    PSTUNClient(
      const PIPSocket::Address & serverAddress,
      WORD serverPort = DefaultPort,
      WORD portBase = 0,
      WORD portMax = 0,
      WORD portPairBase = 0,
      WORD portPairMax = 0
    );


    void Initialise(
      const PString & server,
      WORD portBase = 0, 
      WORD portMax = 0,
      WORD portPairBase = 0, 
      WORD portPairMax = 0
    );

    /**Get the NAT Method Name
     */
    static PStringList GetNatMethodName() { return PStringList("STUN"); }

    /** Get the NAT traversal method name
    */
    virtual PString GetName() const { return "STUN"; }

    /**Get the current server address and port being used.
      */
    virtual bool GetServerAddress(
      PIPSocket::Address & address,   ///< Address of server
      WORD & port                     ///< Port server is using.
    ) const;

    /**Set the STUN server to use.
       The server string may be of the form host:port. If :port is absent
       then the default port 3478 is used. The substring port can also be
       a service name as found in /etc/services. The host substring may be
       a DNS name or explicit IP address.
      */
    PBoolean SetServer(
      const PString & server
    );

    /**Set the STUN server to use by IP address and port.
       If serverPort is zero then the default port of 3478 is used.
      */
    PBoolean SetServer(
      const PIPSocket::Address & serverAddress,
      WORD serverPort = 0
    );

    /**Determine via the STUN protocol the NAT type for the router.
       This will cache the last determine NAT type. Use the force variable to
       guarantee an up to date value.
      */
    NatTypes GetNatType(
      PBoolean force = false    ///< Force a new check
    );

    /**Determine via the STUN protocol the NAT type for the router.
       As for GetNatType() but returns an English string for the type.
      */
    PString GetNatTypeName(
      PBoolean force = false    ///< Force a new check
    ) { return GetNatTypeString(GetNatType(force)); }

    /**Return an indication if the current STUN type supports RTP
      Use the force variable to guarantee an up to date test
      */
    RTPSupportTypes GetRTPSupport(
      PBoolean force = false    ///< Force a new check
    );

    /**Determine the external router address.
       This will send UDP packets out using the STUN protocol to determine
       the intervening routers external IP address.

       A cached address is returned provided it is no older than the time
       specified.
      */
    virtual PBoolean GetExternalAddress(
      PIPSocket::Address & externalAddress, ///< External address of router
      const PTimeInterval & maxAge = 1000   ///< Maximum age for caching
    );

    /**Return the interface NAT router is using.
      */
    virtual bool GetInterfaceAddress(
      PIPSocket::Address & internalAddress
    ) const;

    /**Invalidates the cached addresses and modes.
       This allows to lazily update the external address cache at the next 
       attempt to get the external address.
      */
    void InvalidateCache();

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
    PBoolean CreateSocket(
      PUDPSocket * & socket,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      WORD localPort = 0
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
    virtual PBoolean CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny()
    );

    /**Get the timeout for responses from STUN server.
      */
    const PTimeInterval GetTimeout() const { return replyTimeout; }

    /**Set the timeout for responses from STUN server.
      */
    void SetTimeout(
      const PTimeInterval & timeout   ///< New timeout in milliseconds
    ) { replyTimeout = timeout; }

    /**Get the number of retries for responses from STUN server.
      */
    PINDEX GetRetries() const { return pollRetries; }

    /**Set the number of retries for responses from STUN server.
      */
    void SetRetries(
      PINDEX retries    ///< Number of retries
    ) { pollRetries = retries; }

    /**Get the number of sockets to create in attempt to get a port pair.
       RTP requires a pair of consecutive ports. To get this several sockets
       must be opened and fired through the NAT firewall to get a pair. The
       busier the firewall the more sockets will be required.
      */
    PINDEX GetSocketsForPairing() const { return numSocketsForPairing; }

    /**Set the number of sockets to create in attempt to get a port pair.
       RTP requires a pair of consecutive ports. To get this several sockets
       must be opened and fired through the NAT firewall to get a pair. The
       busier the firewall the more sockets will be required.
      */
    void SetSocketsForPairing(
      PINDEX numSockets   ///< Number opf sockets to create
    ) { numSocketsForPairing = numSockets; }

    /**Returns whether the Nat Method is ready and available in
       assisting in NAT Traversal. The principal is this function is
       to allow the EP to detect various methods and if a method
       is detected then this method is available for NAT traversal.
       The availablity of the STUN Method is dependant on the Type
       of NAT being used.
     */
    virtual bool IsAvailable(
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny()  ///< Interface to see if NAT is available on
    );

    void SetCredentials(const PString & username, const PString & password, const PString & realm);
    void SetCredentialsHash(const BYTE * ptr, PINDEX len);

  protected:
    PString            serverHost;
    WORD               serverPort;
    PTimeInterval      replyTimeout;
    PINDEX             pollRetries;
    PINDEX             numSocketsForPairing;

    bool OpenSocket(PUDPSocket & socket, PortInfo & portInfo, const PIPSocket::Address & binding);

    NatTypes           natType;
    PIPSocket::Address cachedServerAddress;
    PIPSocket::Address cachedExternalAddress;
    PIPSocket::Address interfaceAddress;
    PTime              timeAddressObtained;

    PBYTEArray m_credentialsHash;
};


inline ostream & operator<<(ostream & strm, PSTUNClient::NatTypes type) { return strm << PSTUN::GetNatTypeString(type); }


////////////////////////////////////////////////////////////////////////////////

/**STUN client.
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


class PTURNClient : public PSTUNClient
{
  PCLASSINFO(PTURNClient, PSTUNClient);
  public:
    PTURNClient(const PString & server);
    PTURNClient(
      const PIPSocket::Address & serverAddress,
      WORD serverPort = DefaultPort
    );

    virtual bool Open(const PString & username, const PString & password, const PString & realm);
    virtual bool Close();
    virtual bool IsOpen() const;

    virtual int CreateAllocation(bool evenPort = false);
    virtual bool DeleteAllocation(int allocation);
    virtual bool RefreshAllocation(int allocation, DWORD lifetime = 600);

    bool MakeAuthenticatedRequest(PSTUNMessage & request, PSTUNMessage & response);

    struct Allocation {
      int m_protocol;
      PIPSocketAddressAndPort m_clientAddress;
      PIPSocketAddressAndPort m_relayedTransportAddress;
      DWORD m_lifeTime;

      bool m_authenticated;
    };

  protected:
    PUDPSocket * m_socket;
    PString m_nonce;
    PString m_realm;
    PString m_username;
    PString m_password;

    PMutex m_allocationMutex;
    int m_allocationNumber;
    typedef std::map<int, Allocation> AllocationsMap;
    AllocationsMap m_allocations;
};

#endif


// End of file ////////////////////////////////////////////////////////////////
