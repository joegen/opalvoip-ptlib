//////////////////////////////////////////////////

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/psockbun.h>

class SockBundleProcess : public PProcess
{
  PCLASSINFO(SockBundleProcess, PProcess)

  public:
    SockBundleProcess();
    void Main();
};

PCREATE_PROCESS(SockBundleProcess);

SockBundleProcess::SockBundleProcess()
{
}

void SockBundleProcess::Main()
{
  PSocketBundle bundle;

  if (!bundle.Open(1720)) {
    cout << "Cannot open socket bundle" << endl;
    return;
  }

  cout << "Initial interfaces:" << endl;
  PStringList interfaces = bundle.GetInterfaceList();
  PINDEX i;
  for (i = 0; i < interfaces.GetSize(); ++i) {
    PInterfaceBundle::InterfaceEntry entry;
    cout << "  #" << i+1 << " " << interfaces[i];
    if (!bundle.GetInterfaceInfo(interfaces[i], entry)) 
      cout << " - no longer active" << endl;
    else
      cout << " - " << entry << endl;
  }

  cout << endl << "Waiting for interface changes" << endl;

  Sleep(300000);
}
