/*
 * local.cxx
 *
 * copyright 2005 Derek J Smithies
 *
 *
 * Simple program to report the host name of this machine
 *                          the address of one network interface
 *
 * $Log: local.cxx,v $
 * Revision 1.1  2005/03/10 09:19:01  dereksmithies
 * Initial release of code to illustrate the reading of the machines external ip address and hostname
 *
 *
 *
 */
#include <ptlib.h>
#include <ptlib/sockets.h>

class LocalAddress : public PProcess
{
  PCLASSINFO(LocalAddress, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(LocalAddress)

void LocalAddress::Main()
{
    PUDPSocket localSocket;
    PIPSocket::Address addr;
    if(localSocket.GetNetworkInterface(addr)) {
        cout << "local address is " <<addr.AsString() << endl;
        if (addr == 0)
            cout << "sorry, that is a 0.0.0.0 address" << endl;
    } else
	cout << "Sorry, failed to get local address" << endl;
    
    cout << localSocket.GetHostName() << endl;
}

// End of local.cxx
