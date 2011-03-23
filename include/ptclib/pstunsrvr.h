
#ifndef PTLIB_PSTUNSRVR_H
#define PTLIB_PSTUNSRVR_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptclib/pnat.h>
#include <ptlib/sockets.h>
#include <ptclib/pstun.h>


class PSTUNServer : public PObject, public PSTUN
{
  PCLASSINFO(PSTUNServer, PObject)
  public:
    PSTUNServer();
    
    bool Open(WORD port = DefaultPort);

    bool IsOpen() const;

    bool Close();

    struct SocketInfo {
      SocketInfo();

      PUDPSocket * m_socket;
      PIPSocketAddressAndPort m_socketAddress;
      PIPSocketAddressAndPort m_alternateAddressAndPort;

      PUDPSocket * m_alternatePortSocket;
      PUDPSocket * m_alternateAddressSocket;
      PUDPSocket * m_alternateAddressAndPortSocket;
    };

    virtual bool Read(
      PSTUNMessage & message, 
      SocketInfo & socketInfo
    );

    virtual bool Process(
      const PSTUNMessage & message,
      SocketInfo & socketInfo
    );

    virtual bool OnBindingRequest(
      const PSTUNMessage & request,
      SocketInfo & socketInfo
    );

    virtual bool OnUnknownRequest(
      const PSTUNMessage & request,
      SocketInfo & socketInfo
    );

  protected:
    void PopulateInfo(PSTUNServer::SocketInfo & info, const PIPSocket::Address & alternateAddress, WORD alternatePort, 
             PUDPSocket * alternatePortSocket, PUDPSocket * alternateAddressSocket, PUDPSocket * alternateAddressAndPortSocket);

    SocketInfo * CreateAndAddSocket(const PIPSocket::Address & addess, WORD port);
    bool WriteTo(const PSTUNMessage & message, PUDPSocket & socket, const PIPSocketAddressAndPort & dest);

    PSocket::SelectList m_sockets;
    PSocket::SelectList m_selectList;
    typedef std::map<PUDPSocket *, SocketInfo> SocketToSocketInfoMap;
    SocketToSocketInfoMap m_socketToSocketInfoMap;

    bool m_autoDelete;
};


#endif // PTLIB_PSTUNSRVR_H
