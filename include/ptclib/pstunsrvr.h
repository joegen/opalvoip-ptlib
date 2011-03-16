
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

  protected:
    PUDPSocket m_stunSocket;
};


#endif // PTLIB_PSTUNSRVR_H
