/*
 * $Id: ethsock.h,v 1.1 1998/08/20 05:46:45 robertj Exp $
 *
 * Portable Windows Library
 *
 * Ethernet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ethsock.h,v $
 * Revision 1.1  1998/08/20 05:46:45  robertj
 * Initial revision
 *
 */

#define _PETHSOCKET

#ifdef __GNUC__
#pragma interface
#endif


#ifndef _PSOCKET
#include <socket.h>
#endif

PDECLARE_CLASS(PEthSocket, PSocket)
/* This class describes a type of socket that will communicate using
   raw ethernet packets.
 */

  public:
    PEthSocket(
      PINDEX nBuffers = 8,  // Number of buffers.
      PINDEX size = 1514    // Size of each buffer.
    );
    /* Create a new ethernet packet socket. Some platforms require a set of
       buffers to be allocated to avoid losing frequent packets.
     */


  public:
#ifdef _MSC_VER
#pragma pack(1)
#pragma warning(disable:4201)
#endif
    union Address {
      BYTE b[6];
      WORD w[3];
      struct {
        DWORD l;
        WORD  s;
      };

      Address();
      Address(const BYTE * addr);
      Address(const Address & addr);
      Address(const PString & str);
      Address & operator=(const Address & addr);
      Address & operator=(const PString & str);
      operator PString() const;
      BOOL operator==(const Address & eth) { return l == eth.l && s == eth.s; }
      BOOL operator!=(const Address & eth) { return l != eth.l || s != eth.s; }
      friend ostream & operator<<(ostream & s, Address & a)
        { return s << (PString)a; }
    };
#ifdef _MSC_VER
#pragma warning(default:4201)
#pragma pack()
#endif

  // Overrides from class PChannel
    virtual PString GetName() const;
    /* Get the platform and I/O channel type name of the channel. For an
       ethernet socket this returns the interface name, which is platform
       dependent.

       <H2>Returns:</H2>
       the name of the channel.
     */


  // Overrides from class PSocket
    virtual BOOL Connect(
      const PString & address   // Name of interface to connect to.
    );
    /* Connect a socket to an interface. The first form opens an interface by
       a name as returned by the EnumInterfaces() function. The second opens
       the interface that has the specified MAC address.

       <H2>Returns:</H2>
       TRUE if the channel was successfully connected to the interface.
     */

    virtual BOOL Listen(
      unsigned queueSize = 5,  // Number of pending accepts that may be queued.
      WORD port = 0,           // Port number to use for the connection.
      Reusability reuse = AddressIsExclusive // Can/Cant listen more than once.
    );
    /* This function is illegal and will assert if attempted. You must be
       connected to an interface using Connect() to do I/O on the socket.

       <H2>Returns:</H2>
       TRUE if the channel was successfully opened.
     */


  // New functions for class
    BOOL EnumInterfaces(
      PINDEX idx,      // Index of interface
      PString & name   // Interface name
    );
    /* Enumerate all the interfaces that are capable of being accessed at the
       ethernet level. Begin with index 0, and increment until the function
       returns FALSE. The name string returned can be passed, unchanged, to
       the Connect() function.

       Note that the driver does not need to be open for this function to work.

       <H2>Returns:</H2>
       TRUE if an interface has the index supplied.
     */

    PString GetGatewayInterface() const;
    /* Return the name for the interface that is being used as the gateway,
       that is, the interface that packets on the default route will be sent.

       The string returned may be used in the Connect() function to open that
       interface.

       Note that the driver does not need to be open for this function to work.

       <H2>Returns:</H2>
       String name of the gateway device, or empty string if there is none.
     */


    BOOL GetAddress(
      Address & addr   // Variable to receive the MAC address.
    );
    /* Get the low level MAC address of the open interface.

       <H2>Returns:</H2>
       TRUE if the address is returned, FALSE on error.
     */

    BOOL GetIpAddress(
      PIPSocket::Address & addr     // Variable to receive the IP address.
    );
    BOOL GetIpAddress(
      PIPSocket::Address & addr,    // Variable to receive the IP address.
      PIPSocket::Address & netMask  // Variable to receive the net mask.
    );
    /* Get the prime IP number bound to the open interface. The second form
       of the function also returns the net mask associated with the
       open interface.

       <H2>Returns:</H2>
       TRUE if the address is returned, FALSE on error.
     */

    BOOL EnumIpAddress(
      PINDEX idx,                   // Index 
      PIPSocket::Address & addr,    // Variable to receive the IP address.
      PIPSocket::Address & netMask  // Variable to receive the net mask.
    );
    /* Enumerate all of the IP addresses and net masks bound to the open
       interface. This allows all the addresses to be found on multi-homed
       hosts. Begin with index 0 and increment until the function returns
       FALSE to enumerate all the addresses.

       <H2>Returns:</H2>
       TRUE if the address is returned, FALSE on error or if there are no more
       addresses bound to the interface.
     */

    enum FilterMask {
      FilterDirected     = 0x01,    // Packets directed at the interface.
      FilterMulticast    = 0x02,    // Multicast packets directed at the interface.
      FilterAllMulticast = 0x04,    // All multicast packets.
      FilterBroadcast    = 0x08,    // Packets with a broadcast address.
      FilterPromiscuous  = 0x10     // All packets.
    };
    unsigned GetFilter();
    /* Get the current filtering criteria for receiving packets. A bit-wise OR
       of the FilterMask values will filter packets so that they do not appear
       in the Read() function at all.

       <H2>Returns:</H2>
       A bit mask is returned, a value of 0 indicates an error.
     */

    BOOL SetFilter(unsigned filter);
    /* Set the current filtering criteria for receiving packets. A bit-wise OR
       of the FilterMask values will filter packets so that they do not appear
       in the Read() function at all.

       A value of zero for the filter is useless and will assert.

       <H2>Returns:</H2>
       TRUE if the address is returned, FALSE on error.
     */


    enum MediumTypes {
      Medium802_3,    // An ethernet Network Interface Card (10base2, 10baseT etc)
      MediumWan,      // A Wide Area Network (modem etc)
      MediumUnknown,  // Something else
      NumMediumTypes
    };
    MediumTypes GetMedium();
    /* Return the type of the interface.

       <H2>Returns:</H2>
       Type enum for the interface, or NumMediumTypes if interface not open.
     */

    BOOL ResetAdaptor();
    /* Reset the interface.
     */


  protected:
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;


// Class declaration continued in platform specific header file ///////////////
