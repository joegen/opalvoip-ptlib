/*
* audio.cxx
*
* PWLib application source file for audio testing.
*
* Main program entry point.
*
* Copyright 2005 Roger Hardiman
*
* Copied by Derek Smithies, 1)Add soundtest code from ohphone.
*                           2)Add headers.
*
* $Revision$
* $Author$
* $Date$
*/

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sound.h>
#include <ptclib/cli.h>


#define PTraceModule() "AudioTest"


class AudioTest : public PProcess
{
    PCLASSINFO(AudioTest, PProcess);

  public:
    AudioTest();

    void Main();
    void TestLoop();

  private:
    bool OpenSoundChannel(
      PSoundChannel::Directions dir,
      char driverArgLetter,
      char deviceArgLetter,
      char volumeArgLetter,
      const char * bufferSizeArgString,
      const char * bufferCountArgString,
      const char * bufferCountDefault
    );

    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, RecordVolume);
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, PlayerVolume);
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, Quit);

    std::auto_ptr<PSoundChannel> m_soundChannel[2];
};


///////////////////////////////////////////////////////////////////////////////

AudioTest::AudioTest()
  : PProcess("Roger Hardiman & Derek Smithies code factory", "audio")
{
}

PCREATE_PROCESS(AudioTest)


void AudioTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("l-list. list available sound devices\n"
             "r-record-device: recording device\n"
             "R-record-driver: recording driver\n"
             "p-player-device: play back device\n"
             "P-player-driver: play back driver\n"
             "V-record-volume: set record volume (percent)\n"
             "v-player-volume: set playback volume (percent)\n"
             "s-sample-rate: set sample rate (default 8000)\n"
             "-record-buffer-size: set record buffer size (default 320)\n"
             "-record-buffer-count: set record buffer size (default 2)\n"
             "-player-buffer-size: set play back buffer size (default 320)\n"
             "-player-buffer-count: set play back buffer size (default 8)\n"
             PTRACE_ARGLIST
             "h-help. help");

  if (!args.IsParsed() || args.HasOption('h')) {
    args.Usage(cerr, "[options]");
    return;
  }

  PTRACE_INITIALISE(args);

  cout << "Audio Test Program\n\n";

  if (args.HasOption('l')) {
    cout << "List of Record devices\n";
    PStringArray names = PSoundChannel::GetDeviceNames(PSoundChannel::Recorder);
    for (PINDEX i = 0; i < names.GetSize(); i++)
      cout << "  \"" << names[i] << "\"\n";      
    cout << "The default record device is \"" 
      << PSoundChannel::GetDefaultDevice(PSoundChannel::Recorder) << "\"\n\n";

    cout << "List of play devices\n";
    names = PSoundChannel::GetDeviceNames(PSoundChannel::Player);
    for (PINDEX i = 0; i < names.GetSize(); i++)
      cout << "  \"" << names[i] << "\"\n";      
    cout << "The default play device is \"" 
      << PSoundChannel::GetDefaultDevice(PSoundChannel::Player) << "\"\n";

    return;
  }

  if (!OpenSoundChannel(PSoundChannel::Recorder, 'R', 'r', 'V', "record-buffer-size", "record-buffer-count", "2"))
    return;

  if (!OpenSoundChannel(PSoundChannel::Player,   'P', 'p', 'v', "player-buffer-size", "player-buffer-count", "8"))
    return;

  cout << "Starting record/play back thread" << endl;
  PThreadObj<AudioTest> thread(*this, &AudioTest::TestLoop);

  PCLIStandard cli("Audio> ");
  cli.SetCommand("quit", PCREATE_NOTIFIER(Quit), "Quit test");
  cli.SetCommand("recvol", PCREATE_NOTIFIER(RecordVolume), "Set record volume", "<percent>");
  cli.SetCommand("playvol", PCREATE_NOTIFIER(PlayerVolume), "Set play back volume", "<percent>");
  cli.Start(false);

  cout << "Closing record/play back channels, waiting for thread to terminate ..." << endl;
  for (PSoundChannel::Directions dir = PSoundChannel::Recorder; dir <= PSoundChannel::Player; ++dir)
    m_soundChannel[dir]->Close();

  thread.WaitForTermination(5000);
}


bool AudioTest::OpenSoundChannel(PSoundChannel::Directions dir,
                                 char driverArgLetter,
                                 char deviceArgLetter,
                                 char volumeArgLetter,
                                 const char * bufferSizeArgString,
                                 const char * bufferCountArgString,
                                 const char * bufferCountDefault)
{
  PArgList & args = GetArguments();

  PString driver = args.GetOptionString(driverArgLetter);
  PString device = args.GetOptionString(deviceArgLetter);
  unsigned sampleRate = args.GetOptionString('s', "8000").AsUnsigned();

  cout << dir << " opening ..." << endl;
  PSoundChannel * channel = PSoundChannel::CreateOpenedChannel(driver, device, dir, 1, sampleRate);
  if (channel == NULL) {
    cerr << dir << " failed to open";
    if (driver.IsEmpty() && device.IsEmpty())
      cerr << " using default \"" << PSoundChannel::GetDefaultDevice(dir) << '"';
    else {
      if (!driver.IsEmpty()) {
        cerr << ", driver \"" << driver << '"';
        if (!device.IsEmpty())
          cerr << ',';
      }
      if (!device.IsEmpty())
        cerr << " device \"" << device << '"';
    }
    cerr << ", sample rate: " << sampleRate << endl;
    return false;
  }

  m_soundChannel[dir].reset(channel);

  PTRACE(3, channel, PTraceModule(), dir << " selected \"" << channel->GetName() << '"');
  cout << dir << ": \"" << channel->GetName() << "\", sample rate: " << sampleRate << endl;

  if (args.HasOption(volumeArgLetter))
    channel->SetVolume(args.GetOptionString(volumeArgLetter).AsUnsigned());

  cout << dir << " volume ";
  unsigned int vol;
  if (m_soundChannel[dir]->GetVolume(vol))
    cout << "is " << vol << '%';
  else
    cout << "cannot be obtained.";
  cout << endl;

  PINDEX size = args.GetOptionString(bufferSizeArgString, "320").AsUnsigned();
  PINDEX count = args.GetOptionString(bufferCountArgString, bufferCountDefault).AsUnsigned();
  if (!channel->SetBuffers(size, count))
    cout << dir << " could not set " << count << " buffers of " << size << " bytes." << endl;

  cout << dir << ' ';
  if (channel->GetBuffers(size, count))
    cout << "is using " << count << " buffers of " << size << " bytes.\n";
  else
    cout << "cannot obtain buffer information.";
  cout << endl;

  return true;
}


void AudioTest::RecordVolume(PCLI::Arguments & args, P_INT_PTR)
{
  if (args.GetCount() < 1)
    args.Usage();
  else
    m_soundChannel[PSoundChannel::Recorder]->SetVolume(args[0].AsUnsigned());
}


void AudioTest::PlayerVolume(PCLI::Arguments & args, P_INT_PTR)
{
  if (args.GetCount() < 1)
    args.Usage();
  else
    m_soundChannel[PSoundChannel::Player]->SetVolume(args[0].AsUnsigned());
}


void AudioTest::Quit(PCLI::Arguments & args, P_INT_PTR)
{
  args.GetContext().Stop();
}



void AudioTest::TestLoop()
{
  PTRACE(2, "Starting test loop");

  PBYTEArray buffer(1000);
  for (;;) {
    if (!m_soundChannel[PSoundChannel::Recorder]->Read(buffer.GetPointer(), buffer.GetSize())) {
      PTRACE(1, "Error reading from \"" << m_soundChannel[PSoundChannel::Recorder]->GetName() << '"');
      break;
    }

    PINDEX count = m_soundChannel[PSoundChannel::Recorder]->GetLastReadCount();
    PTRACE(5, "Read " << count << " bytes");

    if (!m_soundChannel[PSoundChannel::Player]->Write(buffer.GetPointer(), count)) {
      PTRACE(1, "Error writing to \"" << m_soundChannel[PSoundChannel::Player]->GetName() << '"');
      break;
    }
  }

  PTRACE(2, "Finished test loop");
}


////////////////////////////////////////////////////////////////////////////////

/* The comment below is magic for those who use emacs to edit this file. 
* With the comment below, the tab key does auto indent to 4 spaces.     
*
*
* Local Variables:
* mode:c
* c-basic-offset:2
* End:
*/

// End of file
