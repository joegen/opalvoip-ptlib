////////////////////////////////////////////////////////
//
//  PTCPSocket
//
////////////////////////////////////////////////////////

#define	_PTCPSOCKET

PDECLARE_CLASS(PTCPSocket, PIPSocket)

  public:
    virtual BOOL Open (const PString address, int port);
      // Open a socket to a remote host on the specified socket


// Platform specific declarations follow

