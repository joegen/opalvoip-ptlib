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


  // Display the mixer settings for the default devices
  PSoundChannel sound;
  dir = PSoundChannel::Player;
  sound.Open(PSoundChannel::GetDefaultDevice(dir),dir);

  unsigned int vol;
  if (sound.GetVolume(vol))
    cout << "Play volume is " << vol << endl;
  else
    cout << "Play volume cannot be obtained" << endl;

  sound.Close();

  dir = PSoundChannel::Recorder;
  sound.Open(PSoundChannel::GetDefaultDevice(dir),dir);
  
  if (sound.GetVolume(vol))
    cout << "Record volume is " << vol << endl;
  else
    cout << "Record volume cannot be obtained" << endl;

  sound.Close();
}

// End of hello.cxx
