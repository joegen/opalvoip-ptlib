////////////////////////////////////////////////////////
//
//  PSocket
//
////////////////////////////////////////////////////////

#define	_PSOCKET

#ifndef _PCHANNEL
#include <channel.h>
#endif

PDECLARE_CLASS(PSocket, PChannel)
  public:
    Socket();
      // create an unattached socket

    virtual BOOL Open (const PString & address, int portnum);
      // connect to another host 

    virtual BOOL Accept (const PString & address);
      // wait for another host to establish a connection 

// Platform specific declarations follow

