
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
    bool Open(WORD port1, WORD port2);
    bool Open(PUDPSocket * socket, bool autoDelete);

    bool IsOpen() const;

    bool Close();

    virtual bool Read(PSTUNMessage & message, PIPSocketAddressAndPort & receivedInterface);
    virtual bool Write(const PSTUNMessage & message, const PIPSocketAddressAndPort & destination, const PIPSocketAddressAndPort & iface);

    virtual bool Process(
      PSTUNMessage & response, 
      const PSTUNMessage & message,
      const PIPSocketAddressAndPort & receivedInterface,
      const PIPSocketAddressAndPort * alternateInterface
    );

    virtual bool OnBindingRequest(
      PSTUNMessage & response, 
      const PSTUNMessage & request,
      const PIPSocketAddressAndPort & receivedInterface,
      const PIPSocketAddressAndPort * alternateInterface
    );

    virtual bool OnUnknownRequest(
      PSTUNMessage & response, 
      const PSTUNMessage & request,
      const PIPSocketAddressAndPort & receivedInterface
    );

  protected:
    PUDPSocket * m_stunSocket1;
    PIPSocketAddressAndPort m_interfaceAddress1;

    PUDPSocket * m_stunSocket2;
    PIPSocketAddressAndPort m_interfaceAddress2;

    bool m_autoDelete;
};


#endif // PTLIB_PSTUNSRVR_H
