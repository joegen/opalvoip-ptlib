/*
 * pnat.h
 *
 * NAT Strategy support for Portable Windows Library.
 *
 * Virteos is a Trade Mark of ISVO (Asia) Pte Ltd.
 *
 * Copyright (c) 2004 ISVO (Asia) Pte Ltd. All Rights Reserved.
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
 *
 * The Original Code is derived from and used in conjunction with the 
 * OpenH323 Project (www.openh323.org/)
 *
 * The Initial Developer of the Original Code is ISVO (Asia) Pte Ltd.
 *
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PNAT_H
#define PTLIB_PNAT_H

#include <ptlib.h>

#if P_NAT

#include <ptlib/sockets.h>

#include <ptlib/plugin.h>
#include <ptlib/pluginmgr.h>

class PNATUDPSocket;


/** PNatMethod
    Base Network Address Traversal Method class
    All NAT Traversal Methods are derived off this class. 
    There are quite a few methods of NAT Traversal. The 
    only purpose of this class is to provide a common 
    interface. It is intentionally minimalistic.
*/
class PNatMethod  : public PObject
{
    PCLASSINFO(PNatMethod,PObject);

  public:
    enum {
      NatPortStart = 49152,
      NatPortEnd   = 65535
    };

    enum Component {
      eComponent_RTP  = 1,
      eComponent_RTCP = 2,
      eComponent_Unknown = 255,
    };

    P_DECLARE_STREAMABLE_ENUM(NatTypes,
      UnknownNat,         ///< NAT was not determined
      OpenNat,            ///< No NAT was detected
      ConeNat,            /**< Full cone NAT.
                               Once an internal address (iAddr:iPort) is mapped
                               to an external address (eAddr:ePort), any
                               packets from iAddr:iPort will be sent through
                               eAddr:ePort.
                               
                               Any external host can send packets to
                               iAddr:iPort by sending packets to eAddr:ePort.
                            */
      RestrictedNat,      /**< Restricted cone NAT.
                               Once an internal address (iAddr:iPort) is mapped
                               to an external address (eAddr:ePort), any
                               packets from iAddr:iPort will be sent through
                               eAddr:ePort.

                               An external host (hAddr:any) can send packets to
                               iAddr:iPort by sending packets to eAddr:ePort
                               only if iAddr:iPort has previously sent a packet
                               to hAddr:any. "Any" means the port number
                               doesn't matter.
                            */
      PortRestrictedNat,  /**< Port-restricted cone NAT.
                               Like an address restricted cone NAT, but the
                               restriction includes port numbers.

                               Once an internal address (iAddr:iPort) is mapped
                               to an external address (eAddr:ePort), any
                               packets from iAddr:iPort will be sent through
                               eAddr:ePort.

                               An external host (hAddr:hPort) can send packets
                               to iAddr:iPort by sending packets to eAddr:ePort
                               only if iAddr:iPort has previously sent a packet
                               to hAddr:hPort.
                            */
      SymmetricNat,       /**< Symmetric NAT
                               Each request from the same internal IP address
                               and port to a specific destination IP address
                               and port is mapped to a unique external source
                               IP address and port, if the same internal host
                               sends a packet even with the same source address
                               and port but to a different destination, a
                               different mapping is used.

                               Only an external host that receives a packet
                               from an internal host can send a packet back.
                            */
      PartiallyBlocked,   /**< Partially blocked.
                               A pathological condition where some packets get
                               through and some do not causing confusion to the
                               STUN protocol in particular. Usually indicates a
                               badly configured firewall rules.
                            */
      BlockedNat          /**< Completely blocked.
                               Packets cannot pass through the router on one
                               or the other direction.
                            */
    );

  /**@name Construction */
  //@{
    /** Default Contructor
    */
    PNatMethod();

    /** Deconstructor
    */
    ~PNatMethod();
  //@}


  /**@name Overrides from PObject */
  //@{
    virtual void PrintOn(
      ostream & strm
    ) const;
  //@}


  /**@name General Functions */
  //@{
    /** Factory Create
    */
    static PNatMethod * Create(
      const PString & name,             ///< Feature Name Expression
      PPluginManager * pluginMgr = NULL ///< Plugin Manager
    );

    /**Get NatTypes enumeration as an English string for the type.
      */
    static PString GetNatTypeString(
      NatTypes type   ///< NAT Type to get name of
    );

    /** Get the NAT traversal method Name
    */
    virtual PString GetName() const = 0;

    /**Get the current server address name.
      */
    virtual PString GetServer() const = 0;

    /**Get the current server address and port being used.
      */
    virtual bool GetServerAddress(
      PIPSocket::Address & address,   ///< Address of server
      WORD & port                     ///< Port server is using.
    ) const;
    virtual bool GetServerAddress(
      PIPSocketAddressAndPort & externalAddressAndPort 
    ) const;

    __inline NatTypes GetNatType(
      const PTimeInterval & maxAge
    ) { return InternalGetNatType(false, maxAge); }

    __inline NatTypes GetNatType(
      bool force = false    ///< Force a new check
    ) { return InternalGetNatType(force, PMaxTimeInterval); }

    /**Determine via the STUN protocol the NAT type for the router.
       As for GetNatType() but returns an English string for the type.
      */
    PString GetNatTypeName(
      bool force = false    ///< Force a new check
    ) { return GetNatTypeString(GetNatType(force)); }

    /**Set the current server address name.
       Defaults to be "address:port" string form.
      */
    virtual bool SetServer(const PString & server);

    /**Set the authentication credientials.
      */
    virtual void SetCredentials(
      const PString & username, 
      const PString & password, 
      const PString & realm
    );

    /** Get the acquired External IP Address.
    */
    virtual bool GetExternalAddress(
      PIPSocket::Address & externalAddress, ///< External address of router
      const PTimeInterval & maxAge = 1000   ///< Maximum age for caching
    );

    /**Return the interface NAT router is using.
      */
    virtual bool GetInterfaceAddress(
      PIPSocket::Address & internalAddress  ///< NAT router internal address returned.
    ) const;

    /**Open the NAT method.
      */
    virtual bool Open(
      const PIPSocket::Address & ifaceAddr
    );

    /**Close the NAT method
      */
    virtual void Close() { }

    /**Create a single socket.
       The NAT traversal protocol is used to create a socket for which the
       external IP address and port numbers are known. A PUDPSocket descendant
       is returned which will, in response to GetLocalAddress() return the
       externally visible IP and port rather than the local machines IP and
       socket.

       The will create a new socket pointer. It is up to the caller to make
       sure the socket is deleted to avoid memory leaks.

       The socket pointer is set to NULL if the function fails and returns
       false.
      */
    virtual PBoolean CreateSocket(
      PUDPSocket * & socket,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      WORD localPort = 0
    ) { return CreateSocket(eComponent_Unknown, socket, binding, localPort); }
    virtual bool CreateSocket(
      Component component,
      PUDPSocket * & socket,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      WORD localPort = 0
    );

    /**Create a socket pair.
       The NAT traversal protocol is used to create a pair of sockets with
       adjacent port numbers for which the external IP address and port
       numbers are known. PUDPSocket descendants are returned which will, in
       response to GetLocalAddress() return the externally visible IP and port
       rather than the local machines IP and socket.

       The will create new socket pointers. It is up to the caller to make
       sure the sockets are deleted to avoid memory leaks.

       The socket pointers are set to NULL if the function fails and returns
       false.
      */
    virtual bool CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny()
    );

    /**Create a socket pair.
       The NAT traversal protocol is used to create a pair of sockets with
       adjacent port numbers for which the external IP address and port
       numbers are known. PUDPSocket descendants are returned which will, in
       response to GetLocalAddress() return the externally visible IP and port
       rather than the local machines IP and socket.

       The will create new socket pointers. It is up to the caller to make
       sure the sockets are deleted to avoid memory leaks.

       The socket pointers are set to NULL if the function fails and returns
       false.
      */
    virtual bool CreateSocketPair(
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding,
      void * userData
    );

    /**Create a socket pair asynchronously.
       This is similar to CreateSocketPair but returns immediately, initiating
       the creating of a socket pair in the background. Use GetSocketPairAsync
       at some later time to get the resultant sockets.

       Default behaviour does nothing.
      */
    virtual bool CreateSocketPairAsync(
      const PString & token
    );

    /**Get a socket pair asynchronously created.
       This function will get the socket pair initated by the
       CreateSocketPairAsync function. If the sockets are not ready yet it
       will block until they are.

       Default behaviour calls CreateSocketPair synchronously
      */
    virtual bool GetSocketPairAsync(
      const PString & token,
      PUDPSocket * & socket1,
      PUDPSocket * & socket2,
      const PIPSocket::Address & binding = PIPSocket::GetDefaultIpAny(),
      void * userData = NULL
    );

    /**Returns whether the Nat Method is ready and available in
    assisting in NAT Traversal on the specified interface.
    */
    virtual bool IsAvailable(
      const PIPSocket::Address & binding
    );

    /**Activate/DeActivate the NAT Method on a call by call basis
     Default does nothing
      */
    virtual void Activate(bool active);

    /**Set Alternate Candidate (ICE) or probe (H.460.24A) addresses.
       Default does nothing.
      */
    virtual void SetAlternateAddresses(
      const PStringArray & addresses,   ///< List of probe/candidates
      void * userData = NULL            ///< User data to link to NAT handler.
    );

    P_DECLARE_ENUM(RTPSupportTypes,
      RTPSupported,
      RTPIfSendMedia,
      RTPUnsupported,
      RTPUnknown
    );

    /**Return an indication if the current STUN type supports RTP
    Use the force variable to guarantee an up to date test
    */
    virtual RTPSupportTypes GetRTPSupport(
      bool force = false    ///< Force a new check
    );

    /**Set the port ranges to be used on local machine.
    Note that the ports used on the NAT router may not be the same unless
    some form of port forwarding is present.

    If the port base is zero then standard operating system port allocation
    method is used.

    If the max port is zero then it will be automatically set to the port
    base + 99.
    */
    virtual void SetPortRanges(
      WORD portBase,          ///< Single socket port number base
      WORD portMax = 0,       ///< Single socket port number max
      WORD portPairBase = 0,  ///< Socket pair port number base
      WORD portPairMax = 0    ///< Socket pair port number max
    );
  //@}

    struct PortInfo {
      PortInfo(WORD port = 0)
        : basePort(port)
        , maxPort(port)
        , currentPort(port)
      {
      }

      void SetPorts(
        WORD start,
        WORD end
      );

      WORD GetNext(
        unsigned increment
      );

      /** RandomPortPair
	      This function returns a random port pair base number in the specified range for
	      the creation of the RTP port pairs (this used to avoid issues with multiple
	      NATed devices opening the same local ports and experiencing issues with
	      the behaviour of the NAT device Refer: draft-jennings-behave-test-results-04 sect 3
      */
      WORD GetRandomPair();

      PMutex mutex;
      WORD   basePort;
      WORD   maxPort;
      WORD   currentPort;
    } ;

  protected:
    virtual NatTypes InternalGetNatType(bool forced, const PTimeInterval & maxAge) = 0;

    PortInfo singlePortInfo, pairedPortInfo;
};

/////////////////////////////////////////////////////////////

PLIST(PNatList, PNatMethod);

////////////////////////////////////////////////////////////////////////////////

class PNatCandidate : public PObject
{
  PCLASSINFO(PNatCandidate, PObject);
  public:
    enum {
      eType_Unknown,
      eType_Host,
      eType_ServerReflexive,
      eType_PeerReflexive,
      eType_Relay
    };

    PNatCandidate();
    PNatCandidate(int type, PNatMethod::Component component);

    virtual PString AsString() const;

    PIPSocketAddressAndPort m_baseAddress;
    PIPSocketAddressAndPort m_transport;
    int                     m_type;
    PNatMethod::Component   m_component;
};


/**UDP socket that has been created by a NAT method.
  */

class PNATUDPSocket : public PUDPSocket
{
  PCLASSINFO(PNATUDPSocket, PUDPSocket);

  public:
    PNATUDPSocket(
      PNatMethod::Component component = PNatMethod::eComponent_Unknown
    );

    virtual PNatCandidate GetCandidateInfo();

    virtual PString GetBaseAddress();
    virtual bool GetBaseAddress(PIPSocketAddressAndPort & addrAndPort);

    PNatMethod::Component GetComponent() const
    { return m_component; }

    void SetComponent(PNatMethod::Component component)
    { m_component = component; }

  protected:
    virtual bool InternalGetBaseAddress(PIPSocketAddressAndPort & addrAndPort);

    PNatMethod::Component m_component;
};


//////////////////////////////////////////////////////////////////////////

/** Fixed NAT support class.
    This can be used in some specific circumstances where you may have no
    NAT at all, or a "symmetric" port forwarding arrangement where STUN is
    unecessary and the appropriate addressess and modes are "hard coded".

    If no NAT is present, then you should use Open() with the interface
    address being used, and not use SetServer(), or use
    SetServer(PString::Empty()). This will create a "null" NAT support
    handler.

    If a NAT system is in use, and you know a priori it's type and addresses
    then use SetServer(extern + '/' + type) to indicate the
    parameters to use. The extern string is the address that will be
    substituted for the interface address set in the Open() function.
    The '/' is optional and if present is following by the numeric value of
    that NatTypes enumeration. If absent SymmetricNat is used.

    If you do not know the NAT type, then PSTUNClient should be used.
  */
class PNatMethod_Fixed  : public PNatMethod
{
  PCLASSINFO(PNatMethod_Fixed, PNatMethod);
  public:
    PNatMethod_Fixed();

    static PString GetNatMethodName();
    virtual PString GetName() const;

    virtual PString GetServer() const;
    virtual bool SetServer(const PString & str);
    virtual bool GetExternalAddress(PIPSocket::Address & addr ,const PTimeInterval &);
    virtual bool GetInterfaceAddress(PIPSocket::Address & addr) const;
    virtual bool Open(const PIPSocket::Address & addr);
    virtual bool IsAvailable(const PIPSocket::Address &);

  protected:
    virtual NatTypes InternalGetNatType(bool forced, const PTimeInterval & maxAge);

    NatTypes           m_type;
    PIPSocket::Address m_interfaceAddress;
    PIPSocket::Address m_externalAddress;
};

/////////////////////////////////////////////////////////////

/** PNatStrategy
  The main container for all
  NAT traversal Strategies. 
*/

class PNatStrategy : public PObject
{
  PCLASSINFO(PNatStrategy,PObject);

public :

  /**@name Construction */
  //@{
  /** Default Contructor
  */
  PNatStrategy();

  /** Deconstructor
  */
  ~PNatStrategy();
  //@}

  /**@name Method Handling */
  //@{
  /** AddMethod
    This function is used to add the required NAT
    Traversal Method. The Order of Loading is important
    The first added has the highest priority.
  */
  void AddMethod(PNatMethod * method);

  /** GetMethod
    This function retrieves the first available NAT
    Traversal Method. If no available NAT Method is found
    then NULL is returned. 
  */
  PNatMethod * GetMethod(const PIPSocket::Address & address = PIPSocket::GetDefaultIpAny());

  /** GetMethodByName
    This function retrieves the NAT Traversal Method with the given name. 
    If it is not found then NULL is returned. 
  */
  PNatMethod * GetMethodByName(const PString & name);

  /** RemoveMethod
    This function removes a NAT method from the NATlist matching the supplied method name
   */
  bool RemoveMethod(const PString & meth);

    /**Set the port ranges to be used on local machine.
       Note that the ports used on the NAT router may not be the same unless
       some form of port forwarding is present.

       If the port base is zero then standard operating system port allocation
       method is used.

       If the max port is zero then it will be automatically set to the port
       base + 99.
      */
    void SetPortRanges(
      WORD portBase,          ///< Single socket port number base
      WORD portMax = 0,       ///< Single socket port number max
      WORD portPairBase = 0,  ///< Socket pair port number base
      WORD portPairMax = 0    ///< Socket pair port number max
    );

    /** Get Loaded NAT Method List
     */
    PNatList & GetNATList() {  return natlist; };

	PNatMethod * LoadNatMethod(const PString & name);

    static PStringArray GetRegisteredList();

  //@}

private:
  PNatList natlist;
  PPluginManager * pluginMgr;
};

////////////////////////////////////////////////////////
//
// declare macros and structures needed for NAT plugins
//

template <class SVC> class PNatMethodServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject *    CreateInstance(int /*userData*/) const { return new SVC; }
    virtual PStringArray GetDeviceNames(int /*userData*/) const { return SVC::GetNatMethodName(); }
    virtual bool  ValidateDeviceName(const PString & deviceName, int /*userData*/) const { return SVC::GetNatMethodName() *= deviceName; }
};

#define PCREATE_NAT_PLUGIN(name) \
  static PNatMethodServiceDescriptor<PNatMethod_##name> PNatMethod_##name##_descriptor; \
  PCREATE_PLUGIN(name, PNatMethod, &PNatMethod_##name##_descriptor) \


#define P_NAT_PARAM(...) ,__VA_ARGS__


#if P_STUN
PPLUGIN_STATIC_LOAD(STUN, PNatMethod);
#endif

#if P_TURN
PPLUGIN_STATIC_LOAD(TURN, PNatMethod);
#endif

#else  // P_NAT

#define P_NAT_PARAM(...)

#endif // P_NAT

#endif // PTLIB_PNAT_H


// End Of File ///////////////////////////////////////////////////////////////
