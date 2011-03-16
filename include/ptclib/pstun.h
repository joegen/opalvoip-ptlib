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

struct PSTUNAttribute
{
  enum Types {
    MAPPED_ADDRESS     = 0x0001,
    RESPONSE_ADDRESS   = 0x0002,
    CHANGE_REQUEST     = 0x0003,
    SOURCE_ADDRESS     = 0x0004,
    CHANGED_ADDRESS    = 0x0005,
    USERNAME           = 0x0006,
    PASSWORD           = 0x0007,
    MESSAGE_INTEGRITY  = 0x0008,
    ERROR_CODE         = 0x0009,
    UNKNOWN_ATTRIBUTES = 0x000a,
    REFLECTED_FROM     = 0x000b,
    MaxValidCode
  };
  
  PUInt16b type;
  PUInt16b length;
  
  PSTUNAttribute * GetNext() const { return (PSTUNAttribute *)(((const BYTE *)this)+length+4); }
};

////////////////////////////////////////////////////////////////////////////////
#pragma pack(1)

class PSTUNAddressAttribute : public PSTUNAttribute
{
  public:
    BYTE     pad;
    BYTE     family;
    PUInt16b port;
    BYTE     ip[4];

    PIPSocket::Address GetIP() const { return PIPSocket::Address(4, ip); }

  protected:
    enum { SizeofAddressAttribute = sizeof(BYTE)+sizeof(BYTE)+sizeof(WORD)+sizeof(PIPSocket::Address) };
    void InitAddrAttr(Types newType)
    {
      type = (WORD)newType;
      length = SizeofAddressAttribute;
      pad = 0;
      family = 1;
    }
    bool IsValidAddrAttr(Types checkType) const
    {
      return type == checkType && length == SizeofAddressAttribute;
    }
};

class PSTUNMappedAddress : public PSTUNAddressAttribute
{
  public:
    void Initialise() { InitAddrAttr(MAPPED_ADDRESS); }
    bool IsValid() const { return IsValidAddrAttr(MAPPED_ADDRESS); }
};

class PSTUNChangedAddress : public PSTUNAddressAttribute
{
  public:
    void Initialise() { InitAddrAttr(CHANGED_ADDRESS); }
    bool IsValid() const { return IsValidAddrAttr(CHANGED_ADDRESS); }
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
    
    void Initialise()
    {
      type = MESSAGE_INTEGRITY;
      length = sizeof(hmac);
      memset(hmac, 0, sizeof(hmac));
    }
    bool IsValid() const { return type == MESSAGE_INTEGRITY && length == sizeof(hmac); }
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
      BindingRequest  = 0x0001,
      BindingResponse = 0x0101,
      BindingError    = 0x0111,
        
      SharedSecretRequest  = 0x0002,
      SharedSecretResponse = 0x0102,
      SharedSecretError    = 0x0112,
    };
    
    PSTUNMessage();
    PSTUNMessage(MsgType newType, const BYTE * id = NULL);

    void SetType(MsgType newType, const BYTE * id = NULL);

    const PSTUNMessageHeader * operator->() const { return (PSTUNMessageHeader *)theArray; }

    PSTUNAttribute * GetFirstAttribute();

    bool Validate(const PSTUNMessage & request);

    void AddAttribute(const PSTUNAttribute & attribute);
    void SetAttribute(const PSTUNAttribute & attribute);
    PSTUNAttribute * FindAttribute(PSTUNAttribute::Types type);

    bool Read(PUDPSocket & socket);
    bool Write(PUDPSocket & socket) const;
    bool Poll(PUDPSocket & socket, const PSTUNMessage & request, PINDEX pollRetries);
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
};


inline ostream & operator<<(ostream & strm, PSTUNClient::NatTypes type) { return strm << PSTUN::GetNatTypeString(type); }


#endif // PTLIB_PSTUN_H


// End of file ////////////////////////////////////////////////////////////////
