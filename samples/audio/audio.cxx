//
// audio.cxx
//
// Roger Hardiman
//

#include <ptlib.h>

class Audio : public PProcess
{
  PCLASSINFO(Audio, PProcess)
  public:
    void Main();
};

PCREATE_PROCESS(Audio)

void Audio::Main()
{
  cout << "Audio Test Program\n";

  PSoundChannel::Directions dir;
  PStringArray names;

  cout << "\n";
  cout << "List of Play devices\n";

  dir = PSoundChannel::Player;
  names = PSoundChannel::GetDeviceNames(dir);
  for (PINDEX i = 0; i < names.GetSize(); i++)
    cout << "  \"" << names[i] << "\"\n";

  cout << "The default play device is \"" << PSoundChannel::GetDefaultDevice(dir) << "\"\n";


  cout << "\n";
  cout << "List of Record devices\n";

  dir = PSoundChannel::Recorder;
  names = PSoundChannel::GetDeviceNames(dir);
  for (PINDEX i = 0; i < names.GetSize(); i++)
    cout << "  \"" << names[i] << "\"\n";

  cout << "The default record device is \"" << PSoundChannel::GetDefaultDevice(dir) << "\"\n";

  cout << "\n";

}

// End of hello.cxx
