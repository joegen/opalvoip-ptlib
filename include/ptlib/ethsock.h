/*
 * ethsock.h
 *
 * Direct Ethernet socket I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_ETHSOCKET_H
#define PTLIB_ETHSOCKET_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/ipsock.h>


/**This class describes a type of socket that will communicate using
   raw ethernet packets.
 */
class PEthSocket : public PSocket
{
    PCLASSINFO(PEthSocket, PSocket);
  public:
  /**@name Constructor */
  //@{
    /**Create a new ethernet packet socket.
     */
    PEthSocket(
      bool promiscuous =  true,     /**< Indicates all packets to be received,
                                         not just ones directed to this interface address */
      unsigned snapLength = 65536   ///< Maximum data size for each apcket capture.
    );

      /// Close the socket
    ~PEthSocket();
  //@}


  public:
    /// Medium types for the open interface.
    P_DECLARE_STREAMABLE_ENUM(MediumType,
      MediumLoop,      ///< A Loopback Network
      Medium802_3,     ///< An ethernet Network Interface Card (10base2, 10baseT etc)
      MediumWan,       ///< A Wide Area Network (modem etc)
      MediumLinuxSLL,  ///< Linux "cooked" capture encapsulation
      MediumUnknown   ///< Something else
    );

#pragma pack(1)
    /** An ethernet MAC Address specification.
     */
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

      bool operator==(const BYTE * eth) const;
      bool operator!=(const BYTE * eth) const;
      bool operator==(const Address & eth) const { return ls.l == eth.ls.l && ls.s == eth.ls.s; }
      bool operator!=(const Address & eth) const { return ls.l != eth.ls.l || ls.s != eth.ls.s; }

      operator PString() const;

      friend ostream & operator<<(ostream & s, const Address & a)
        { return s << (PString)a; }
    };
#pragma pack()

  /**@name Overrides from class PChannel */
  //@{
    /** Get the platform and I/O channel type name of the channel. For example,
       it would return the filename in <code>PFile</code> type channels.

       @return the name of the channel.
     */
    virtual PString GetName() const { return m_channelName; }

    /**Close the channel, shutting down the link to the data source.

       @return
       true if the channel successfully closed.
     */
    virtual PBoolean Close();

    /**Low level read from the channel. This function may block until the
       requested number of characters were read or the read timeout was
       reached. The GetLastReadCount() function returns the actual number
       of bytes read.

       The GetErrorCode() function should be consulted after Read() returns
       false to determine what caused the failure.

       @return
       true indicates that at least one character was read from the channel.
       false means no bytes were read due to timeout or some other I/O error.
     */
    virtual PBoolean Read(
      void * buf,   ///< Pointer to a block of memory to receive the read bytes.
      PINDEX len    ///< Maximum number of bytes to read into the buffer.
    );

    /**Low level write to the channel. This function will block until the
       requested number of characters are written or the write timeout is
       reached. The GetLastWriteCount() function returns the actual number
       of bytes written.

       The GetErrorCode() function should be consulted after Write() returns
       false to determine what caused the failure.

       @return
       true if at least len bytes were written to the channel.
     */
    virtual PBoolean Write(
      const void * buf, ///< Pointer to a block of memory to write.
      PINDEX len        ///< Number of bytes to write.
    );
  //@}

  /**@name Overrides from class PSocket */
  //@{
    /**Connect a socket to an interface. The first form opens an interface by
       a name as returned by the EnumInterfaces() function. The second opens
       the interface that has the specified MAC address.

       @return
       true if the channel was successfully connected to the interface.
     */
    virtual PBoolean Connect(
      const PString & address   ///< Name of interface to connect to.
    );

    /**This function is illegal and will assert if attempted. You must be
       connected to an interface using Connect() to do I/O on the socket.

       @return
       true if the channel was successfully opened.
     */
    virtual PBoolean Listen(
      unsigned queueSize = 5,  ///< Number of pending accepts that may be queued.
      WORD port = 0,           ///< Port number to use for the connection.
      Reusability reuse = AddressIsExclusive ///< Can/Cant listen more than once.
    );
  //@}


  /**@name Information functions */
  //@{
    /**Enumerate all the interfaces that are capable of being accessed at the
       ethernet level. The name string(s) returned can be passed, unchanged, to
       the Connect() function.

       Note that the driver does not need to be open for this function to work.

       @return
       Empty array if no devices are available.
     */
    static PStringArray EnumInterfaces(
      bool detailed = true
    );


    /**Return the data link of the interface.

       @return
       Type enum for the interface, or NumMediumTypes if interface not open.
     */
    MediumType GetMedium();

    /**Return the capture time of the last read packet.
      */
    const PTime & GetLastPacketTime() const { return m_lastPacketTime; }
  //@}


  /**@name Filtering functions */
  //@{
    /**Get the current filtering criteria for receiving packets.
     */
    const PString & GetFilter() const { return m_filter; }

    /**Set the current filtering criteria for receiving packets.
       See http://www.tcpdump.org for the expression syntax.

       @return
       true if the filter expression is valid.
     */
    bool SetFilter(
      const PString & filter   ///< Bits for filtering on address
    );
  //@}


  /**@name I/O functions */
  //@{
    /** An ethernet MAC frame.
     */
    class Frame : public PObject
    {
        PCLASSINFO(Frame, PObject);
      public:
        Frame(
          PINDEX maxSize = 65536
        );
        Frame(
          const Frame & frame
        );

        virtual bool Write(
          PChannel & channel
        ) const;

        virtual bool Read(
          PChannel & channel,
          PINDEX packetSize = P_MAX_INDEX
        );

        void PreRead();

        /** Extract the data link payload.
            @return the protocol identifier for the payload.
         */
        int GetDataLink(
          PBYTEArray & payload
        );
        int GetDataLink(
          PBYTEArray & payload,
          Address & src,
          Address & dst
        );
        BYTE * CreateDataLink(
          const Address & src,
          const Address & dst,
          unsigned proto,
          PINDEX length
        );

        /** Extract the Internet Protocol payload.
            @return the protocol identifier for the payload.
         */
        int GetIP(
          PBYTEArray & payload
        );
        int GetIP(
          PBYTEArray & payload,
          PIPSocket::Address & src,
          PIPSocket::Address & dst
        );
        BYTE * CreateIP(
          const PIPSocket::Address & src,
          const PIPSocket::Address & dst,
          unsigned proto,
          PINDEX length
        );

        /** Extract the UDP payload.
            @return the protocol identifier for the payload.
         */
        bool GetUDP(
          PBYTEArray & payload,
          WORD & srcPort,
          WORD & dstPort
        );
        bool GetUDP(
          PBYTEArray & payload,
          PIPSocketAddressAndPort & src,
          PIPSocketAddressAndPort & dst
        );
        BYTE * CreateUDP(
          const PIPSocketAddressAndPort & src,
          const PIPSocketAddressAndPort & dst,
          PINDEX length
        );

        /** Extract the TCP payload.
            @return the protocol identifier for the payload.
         */
        bool GetTCP(
          PBYTEArray & payload,
          WORD & srcPort,
          WORD & dstPort
        );
        bool GetTCP(
          PBYTEArray & payload,
          PIPSocketAddressAndPort & src,
          PIPSocketAddressAndPort & dst
        );
        BYTE * CreateTCP(
          const PIPSocketAddressAndPort & src,
          const PIPSocketAddressAndPort & dst,
          PINDEX length
        );

        const PTime & GetTimestamp() const { return m_timestamp; }
        bool IsFragmentated() const { return m_fragmentated; }

        PINDEX GetSize() const { return m_rawSize; }

      protected:
        PBYTEArray  m_rawData;
        PINDEX      m_rawSize;

        PBYTEArray  m_fragments;
        bool        m_fragmentated;
        unsigned    m_fragmentProto;
        bool        m_fragmentProcessed;

        PTime              m_timestamp;
        PIPSocket::Address m_fragmentSrcIP;
        PIPSocket::Address m_fragmentDstIP;
    };

    /**Read a frame from the interface.
       Note that for correct decoding of fragmented IP packets, you should
       make consecutive calls to this function with the same instance of
       Frame which maintains some context.

       @return
       true if the packet read, false on error.
      */
    bool ReadFrame(
      Frame & frame
    ) { return frame.Read(*this); }
  //@}

  protected:
    virtual PBoolean OpenSocket();
    virtual const char * GetProtocolName() const;

    PString  m_channelName;
    bool     m_promiscuous;
    unsigned m_snapLength;
    PString  m_filter;  // Filter expression

    struct InternalData;
    InternalData * m_internal;

    PTime m_lastPacketTime;
};


class PEthSocketThread : public PObject
{
    PCLASSINFO(PEthSocketThread, PObject);
  public:
    #define PDECLARE_EthFrameNotifier(cls, fn) PDECLARE_NOTIFIER2(PEthSocket, cls, fn, PEthSocket::Frame &)
    typedef PNotifierTemplate<PEthSocket::Frame &> FrameNotifier;

    PEthSocketThread(const FrameNotifier & notifier = NULL);
    ~PEthSocketThread() { Stop(); }

    virtual bool Start(
      const PString & device,
      const PString & filter = PString::Empty(),
      PThread::Priority priority = PThread::NormalPriority
    );

    virtual void Stop();

    bool IsRunning() const { return m_running; }

    virtual PEthSocket * CreateEthSocket() const;

    void SetNotifier(
      const FrameNotifier & notifier
    ) { m_notifier = notifier; }

    PEthSocket * GetSocket() const { return m_socket; }

  protected:
    virtual void MainLoop();

    FrameNotifier     m_notifier;
    PThread         * m_thread;
    PEthSocket      * m_socket;
    PEthSocket::Frame m_frame;
    bool              m_running;
};


#endif // PTLIB_ETHSOCKET_H


// End Of File ///////////////////////////////////////////////////////////////
