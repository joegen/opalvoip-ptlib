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
* Updated by Robert Jongbloed (robertj@voxlucida.com.au)
*
* $Revision$
* $Author$
* $Date$
*/

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/sound.h>
#include <ptclib/cli.h>
#include <ptclib/qchannel.h>


#define PTraceModule() "AudioTest"


class AudioTest : public PProcess
{
    PCLASSINFO(AudioTest, PProcess);

  public:
    AudioTest();

    void Main();

  private:
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, RecordVolume);
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, PlayerVolume);
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, Statistics);
    PDECLARE_NOTIFIER(PCLI::Arguments, AudioTest, Quit);

    PQueueChannel m_queue;

    class AudioInfo : public PSoundChannel {
      PQueueChannel & m_queue;
      PTimeInterval   m_avgTime;
      unsigned        m_avgCount;
      PTimeInterval   m_lastTime;
      PTimeInterval   m_minTime;
      PTimeInterval   m_maxTime;
      PMutex          m_mutex;

    public:
      AudioInfo(PQueueChannel & queue)
        : m_queue(queue)
        , m_avgCount(0)
        , m_minTime(0,1)
      { }

      bool OpenSoundChannel(
        PSoundChannel::Directions dir,
        const PArgList & args,
        char driverArgLetter,
        char deviceArgLetter,
        char volumeArgLetter,
        const char * bufferSizeArgString,
        const char * bufferCountArgString,
        const char * bufferCountDefault
      );

      void ReadLoop();
      void WriteLoop();
      void AccumulateStatistics();
      void DisplayStatistics();
    } m_recorder, m_player;
};


///////////////////////////////////////////////////////////////////////////////

AudioTest::AudioTest()
  : PProcess("Roger Hardiman & Derek Smithies code factory", "audio")
  , m_queue(8000)
  , m_recorder(m_queue)
  , m_player(m_queue)
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

  if (!m_recorder.OpenSoundChannel(PSoundChannel::Recorder, args, 'R', 'r', 'V', "record-buffer-size", "record-buffer-count", "2"))
    return;

  if (!  m_player.OpenSoundChannel(PSoundChannel::Player,   args, 'P', 'p', 'v', "player-buffer-size", "player-buffer-count", "8"))
    return;

  cout << "Starting record/play back thread" << endl;
  PThreadObj<AudioInfo> recordThread(m_recorder, &AudioInfo::ReadLoop);
  PThread::Sleep(100);
  PThreadObj<AudioInfo> playThread(m_player, &AudioInfo::WriteLoop);

  PCLIStandard cli("Audio> ");
  cli.SetCommand("quit\nq", PCREATE_NOTIFIER(Quit), "Quit test");
  cli.SetCommand("stats\ns", PCREATE_NOTIFIER(Statistics), "Show statistics");
  cli.SetCommand("recvol", PCREATE_NOTIFIER(RecordVolume), "Set record volume", "<percent>");
  cli.SetCommand("playvol", PCREATE_NOTIFIER(PlayerVolume), "Set play back volume", "<percent>");
  cli.Start(false);

  cout << "Closing record channel, waiting for thread to terminate ..." << endl;
  m_recorder.Close();

  recordThread.WaitForTermination(5000);
  playThread.WaitForTermination(5000);
}


bool AudioTest::AudioInfo::OpenSoundChannel(PSoundChannel::Directions dir, 
                                            const PArgList & args,
                                            char driverArgLetter,
                                            char deviceArgLetter,
                                            char volumeArgLetter,
                                            const char * bufferSizeArgString,
                                            const char * bufferCountArgString,
                                            const char * bufferCountDefault)
{
  PString driver = args.GetOptionString(driverArgLetter);
  PString device = args.GetOptionString(deviceArgLetter);
  unsigned sampleRate = args.GetOptionString('s', "8000").AsUnsigned();

  std::cout << dir << " opening ..." << endl;
  if (!Open(driver + '\t' + device, dir, 1, sampleRate)) {
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

  PTRACE(3, dir << " selected \"" << GetName() << '"');
  std::cout << dir << ": \"" << GetName() << "\", sample rate: " << sampleRate << endl;

  if (args.HasOption(volumeArgLetter))
    SetVolume(args.GetOptionString(volumeArgLetter).AsUnsigned());

  std::cout << dir << " volume ";
  unsigned int vol;
  if (GetVolume(vol))
    std::cout << "is " << vol << '%';
  else
    std::cout << "cannot be obtained.";
  std::cout << endl;

  PINDEX size = args.GetOptionString(bufferSizeArgString, "320").AsUnsigned();
  PINDEX count = args.GetOptionString(bufferCountArgString, bufferCountDefault).AsUnsigned();
  if (!SetBuffers(size, count))
    std::cout << dir << " could not set " << count << " buffers of " << size << " bytes." << endl;

  std::cout << dir << ' ';
  if (GetBuffers(size, count))
    std::cout << "is using " << count << " buffers of " << size << " bytes.\n";
  else {
    std::cout << "cannot obtain buffer information.";
    return false;
  }
  std::cout << endl;

  return true;
}


void AudioTest::RecordVolume(PCLI::Arguments & args, P_INT_PTR)
{
  if (args.GetCount() < 1)
    args.Usage();
  else
    m_recorder.SetVolume(args[0].AsUnsigned());
}


void AudioTest::PlayerVolume(PCLI::Arguments & args, P_INT_PTR)
{
  if (args.GetCount() < 1)
    args.Usage();
  else
    m_player.SetVolume(args[0].AsUnsigned());
}


void AudioTest::Statistics(PCLI::Arguments &, P_INT_PTR)
{
  m_recorder.DisplayStatistics();
  m_player.DisplayStatistics();
}


void AudioTest::Quit(PCLI::Arguments & args, P_INT_PTR)
{
  args.GetContext().Stop();
}


void AudioTest::AudioInfo::ReadLoop()
{
  PTRACE(2, "Starting read loop");

  PINDEX size, count;
  GetBuffers(size, count);
  PBYTEArray buffer(size);

  m_avgTime = m_lastTime = PTimer::Tick();
  for (;;) {
    if (!Read(buffer.GetPointer(), buffer.GetSize())) {
      PTRACE_IF(1, IsOpen(), "Error reading from \"" << GetName() << "\" - " << GetErrorText(LastReadError));
      break;
    }

    AccumulateStatistics();

    PINDEX count = GetLastReadCount();
    PTRACE(5, "Read " << count << " bytes");
    PAssert(m_queue.Write(buffer.GetPointer(), count), PLogicError);
  }

  // Close queue to write thread exits
  m_queue.Close();

  PTRACE(2, "Finished read loop");
}


void AudioTest::AudioInfo::WriteLoop()
{
  PTRACE(2, "Starting write loop");

  PINDEX size, count;
  GetBuffers(size, count);
  PBYTEArray buffer(size);

  m_avgTime = m_lastTime = PTimer::Tick();
  for (;;) {
    if (!m_queue.Read(buffer.GetPointer(), buffer.GetSize())) {
      PTRACE_IF(1, m_queue.IsOpen(), "Error reading from queue");
      break;
    }

    PINDEX count = m_queue.GetLastReadCount();
    PTRACE(5, "Writing " << count << " bytes");

    if (!Write(buffer.GetPointer(), count)) {
      PTRACE(1, "Error writing to \"" << GetName() << "\" - " << GetErrorText(LastWriteError));
      break;
    }

    AccumulateStatistics();
  }

  PTRACE(2, "Awaiting last of audio to be played");
  WaitForPlayCompletion();

  PTRACE(2, "Finished write loop");
}


void AudioTest::AudioInfo::AccumulateStatistics()
{
  m_mutex.Wait();

  PTimeInterval tick = PTimer::Tick();
  PTimeInterval delta = tick - m_lastTime;
  m_lastTime = tick;

  if (m_minTime > delta)
    m_minTime = delta;
  if (m_maxTime < delta)
    m_maxTime = delta;

  ++m_avgCount;

  m_mutex.Signal();
}


void AudioTest::AudioInfo::DisplayStatistics()
{
  m_mutex.Wait();

  std::cout << activeDirection << " min=" << m_minTime << ", max=" << m_maxTime << ", avg=";
  if (m_avgCount == 0)
    std::cout << "n/a";
  else
    std::cout << (PTimer::Tick() - m_avgTime)/m_avgCount;
  std::cout << endl;

  m_avgTime = PTimer::Tick();
  m_avgCount = 0;
  m_minTime.SetInterval(0,1);
  m_maxTime = 0;

  m_mutex.Signal();
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
