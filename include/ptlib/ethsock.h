/*
 * $Id: ethsock.h,v 1.2 1998/08/21 05:26:34 robertj Exp $
 *
 * Portable Windows Library
 *
 * Ethernet Socket Class Declarations
 *
 * Copyright 1993 Equivalence
 *
 * $Log: ethsock.h,v $
 * Revision 1.2  1998/08/21 05:26:34  robertj
 * Fine tuning of interface.
 *
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

    ~PEthSocket();
      // Close the socket


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
      } ls;

      Address();
      Address(const BYTE * addr);
      Address(const Address & addr);
      Address(const PString & str);
      Address & operator=(const Address & addr);
      Address & operator=(const PString & str);
      operator PString() const;
      BOOL operator==(const BYTE * eth) const    { return memcmp(b, eth, sizeof(b)) == 0; }
      BOOL operator!=(const BYTE * eth) const    { return memcmp(b, eth, sizeof(b)) != 0; }
      BOOL operator==(const Address & eth) const { return ls.l == eth.ls.l && ls.s == eth.ls.s; }
      BOOL operator!=(const Address & eth) const { return ls.l != eth.ls.l || ls.s != eth.ls.s; }
      friend ostream & operator<<(ostream & s, Address & a)
        { return s << (PString)a; }
    };

  struct Frame {
    BYTE dst_addr[6];
    BYTE src_addr[6];
    union {
      struct {
        WORD type;
        BYTE payload[1500];
      } ether;
      struct {
        WORD length;
        BYTE dsap;
        BYTE ssap;
        BYTE ctrl;
        BYTE oui[3];
        WORD type;
        BYTE payload[1492];
      } snap;
    };
  };
#ifdef _MSC_VER
#pragma warning(default:4201)
#pragma pack()
#endif

  // Overrides from class PChannel
    virtual BOOL Close();
    /* Close the channel, shutting down the link to the data source.

       <H2>Returns:</H2>
       TRUE if the channel successfully closed.
     */

    virtual BOOL Read(
      void * buf,   // Pointer to a block of memory to receive the read bytes.
      PINDEX len    // Maximum number of bytes to read into the buffer.
    );
    /* Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
     */

    virtual BOOL Write(
      const void * buf, // Pointer to a block of memory to write.
      PINDEX len        // Number of bytes to write.
    );
    /* Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       FALSE to determine what caused the failure.

       <H2>Returns:</H2>
       TRUE if at least len bytes were written to the channel.
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

    enum EthTypes {
      TypeAll = 3,          // All frames
      TypeIP  = 0x800,      // Internet Protocol
      TypeX25 = 0x805,      // X.25
      TypeARP = 0x806,      // Address Resolution Protocol
      TypeAtalk = 0x809B,   // Appletalk DDP
      TypeAARP = 0x80F3,    // Appletalk AARP
      TypeIPX = 0x8137,     // IPX
      TypeIPv6 = 0x86DD     // Bluebook IPv6
    };
    enum FilterMask {
      FilterDirected     = 0x01,    // Packets directed at the interface.
      FilterMulticast    = 0x02,    // Multicast packets directed at the interface.
      FilterAllMulticast = 0x04,    // All multicast packets.
      FilterBroadcast    = 0x08,    // Packets with a broadcast address.
      FilterPromiscuous  = 0x10     // All packets.
    };
    BOOL GetFilter(
      unsigned & mask,  // Bits for filtering on address
      WORD & type       // Code for filtering on type.
    );
    /* Get the current filtering criteria for receiving packets.

       A bit-wise OR of the FilterMask values will filter packets so that
       they do not appear in the Read() function at all.

       The type is be the specific frame type to accept. A value of TypeAll
       may be used to match all frame types.

       <H2>Returns:</H2>
       A bit mask is returned, a value of 0 indicates an error.
     */

    BOOL SetFilter(
      unsigned mask,       // Bits for filtering on address
      WORD type = TypeAll  // Code for filtering on type.
    );
    /* Set the current filtering criteria for receiving packets. A bit-wise OR
       of the FilterMask values will filter packets so that they do not appear
       in the Read() function at all.

       The type is be the specific frame type to accept. A value of TypeAll
       may be used to match all frame types.

       A value of zero for the filter mask is useless and will assert.

       <H2>Returns:</H2>
       TRUE if the address is returned, FALSE on error.
     */


    enum MediumTypes {
      MediumLoop,     // A Loopback Network
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

    BOOL ReadPacket(
      PBYTEArray & buffer,  // Buffer to receive the raw packet
      Address & dest,       // Destination address of packet
      Address & src,        // Source address of packet
      WORD & type,          // Packet frame type ID
      PINDEX & len,         // Length of payload
      BYTE * & payload      // Pointer into <CODE>buffer</CODE> of payload.
    );
    /* Read a packet from the interface and parse out the information
       specified by the parameters. This will automatically adjust for 802.2
       and 802.3 ethernet frames.

       <H2>Returns:</H2>
       TRUE if the packet read, FALSE on error.
     */

  protected:
    virtual BOOL OpenSocket();
    virtual const char * GetProtocolName() const;


    WORD filterType;  // Remember the set filter frame type


// Class declaration continued in platform specific header file ///////////////
