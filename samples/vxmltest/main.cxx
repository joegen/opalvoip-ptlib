/*
 * main.cxx
 *
 * PWLib application source file for vxmltest
 *
 * Main program entry point.
 *
 * Copyright 2002 Equivalence
 *
 */

#include <ptlib.h>
#include <ptlib/sound.h>
#include <ptclib/vxml.h>
#include <ptclib/cli.h>

#include "main.h"

PCREATE_PROCESS(VxmlTest);

#define PTraceModule() "VXMLTest"


#ifdef _WIN32
  #define PREVIEW_WINDOW_DEVICE "MSWIN"
#else
  #define PREVIEW_WINDOW_DEVICE "SDL"
#endif


VxmlTest::VxmlTest()
  : PProcess("Equivalence", "vxmltest", 1, 0, AlphaCode, 1)
{
}

#if P_VXML

void VxmlTest::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("-sound-driver: Output to sound driver\n"
                  "S-sound-device: Output to sound device\n"
                  "T-tts: Text to speech method\n"
                  "c-cache: Text to speech cache directory\n"
#if P_VXML_VIDEO
                  "V-video. Enabled video support\n"
                  "L-sign-language: Set sign language analyser library (implicit -V)\n"
                  "-input-driver: Video input driver\n"
                  "I-input-device: Video input device\n"
                  "C-input-channel: Video input channel\n"
                  "P-preview. Show preview window for video input\n"
                  "-output-driver: Video output driver\n"
                  "O-output-device: Video output device\n"
                  "-output-channel: Video output channel\n"
#endif
                  PTRACE_ARGLIST)) {
    args.Usage(cerr, "[ options ] <vxml-file> [ [ \"--\" | options ] <vxml-file> ... ]");
    return;
  }

  PTRACE_INITIALISE(args);

  if (args.HasOption("sign-language")) {
    if (!PVXMLSession::SetSignLanguageAnalyser(args.GetOptionString("sign-language"))) {
      cerr << "error: cannot load sign language analyser" << endl;
      return;
    }
    cout << "Using sign language analyser." << endl;
  }

  do {
    m_tests.push_back(TestInstance());
    if (!m_tests.back().Initialise(m_tests.size(), args))
      m_tests.pop_back();
  } while (args.Parse("-sound-driver:S-sound-device:T-tts:"
#if P_VXML_VIDEO
                      "V-video."
                      "-input-driver:I-input-device:C-input-channel:"
                      "-output-driver:O-output-device:-output-channel:"
#endif
  ));

  PCLIStandard cli("VXML-Test> ");
  cli.SetCommand("input", PCREATE_NOTIFIER(SimulateInput), "Simulate input for VXML instance (1..n)", "<digit> [ <n> ]");
  cli.Start(false);
  m_tests.clear();
}


void VxmlTest::SimulateInput(PCLI::Arguments & args, P_INT_PTR)
{
  unsigned num;
  if (args.GetCount() < 1) {
    args.WriteUsage();
    return;
  }

  if (args.GetCount() < 2)
    num = 1;
  else if ((num = args[0].AsUnsigned()) == 0) {
    args.WriteError("Invalid instance number");
    return;
  }

  if (num > m_tests.size())
    args.WriteError("No such instance");
  else
    m_tests[num - 1].SendInput(args[0]);
}



TestInstance::TestInstance()
  : m_instance(0)
  , m_player(NULL)
  , m_grabber(NULL)
  , m_preview(NULL)
  , m_viewer(NULL)
  , m_vxml(NULL)
  , m_audioThread(NULL)
  , m_videoSenderThread(NULL)
  , m_videoReceiverThread(NULL)
{
}


TestInstance::~TestInstance()
{
  if (m_player != NULL) {
    m_player->Close();
    PThread::WaitAndDelete(m_audioThread, PMaxTimeInterval);
    delete m_player;
  }

#if P_VXML_VIDEO
  if (m_viewer != NULL) {
    m_viewer->Close();
    PThread::WaitAndDelete(m_videoSenderThread, PMaxTimeInterval);
    delete m_viewer;
  }

  if (m_grabber != NULL) {
    m_grabber->Close();
    PThread::WaitAndDelete(m_videoReceiverThread, PMaxTimeInterval);
    delete m_grabber;
  }
#endif // P_VXML_VIDEO

  delete m_vxml;
}

bool TestInstance::Initialise(unsigned instance, const PArgList & args)
{
  m_instance = instance;

  PSoundChannel::Params audioParams;
  audioParams.m_direction = PSoundChannel::Player;
  audioParams.m_driver = args.GetOptionString("sound-driver");
  audioParams.m_device = args.GetOptionString("sound-device");
  m_player = PSoundChannel::CreateOpenedChannel(audioParams);
  if (m_player == NULL) {
    cerr << "Instance " << m_instance << " error: cannot open sound device \"" << audioParams.m_device << "\"" << endl;
    return false;
  }
  cout << "Instance " << m_instance << " using audio device \"" << m_player->GetName() << "\"" << endl;


#if P_VXML_VIDEO
  if (args.HasOption("video") || args.HasOption("sign-language")) {
    PVideoOutputDevice::OpenArgs videoArgs;
    videoArgs.driverName = args.GetOptionString("input-driver");
    videoArgs.deviceName = args.GetOptionString("input-device");
    videoArgs.channelNumber = args.GetOptionString("input-channel", "-1").AsInteger();
    m_grabber = PVideoInputDevice::CreateOpenedDevice(videoArgs);
    if (m_grabber == NULL) {
      cerr << "Instance " << m_instance << " error: cannot open video device \"" << videoArgs.deviceName << "\"" << endl;
      return false;
    }
    cout << "Instance " << m_instance << " using input video device \"" << m_grabber->GetDeviceName() << "\"" << endl;

    if (args.HasOption("preview"))
      m_preview = PVideoOutputDevice::CreateOpenedDevice(PREVIEW_WINDOW_DEVICE "TITLE=Preview");

    videoArgs.driverName = args.GetOptionString("output-driver");
    videoArgs.deviceName = args.GetOptionString("output-device");
    videoArgs.channelNumber = args.GetOptionString("output-channel", "-1").AsInteger();
    m_viewer = PVideoOutputDevice::CreateOpenedDevice(videoArgs);
    if (m_player == NULL) {
      cerr << "Instance " << m_instance << " error: cannot open video device \"" << videoArgs.deviceName << "\"" << endl;
      return false;
    }
    cout << "Instance " << m_instance << " using output video device \"" << m_viewer->GetDeviceName() << "\"" << endl;
  }
#endif


  PTextToSpeech * tts = PFactory<PTextToSpeech>::CreateInstance(args.GetOptionString("tts", "Microsoft SAPI"));
  if (tts == NULL) {
    PFactory<PTextToSpeech>::KeyList_T engines = PFactory<PTextToSpeech>::GetKeyList();
    if (!engines.empty()) {
      tts = PFactory<PTextToSpeech>::CreateInstance(engines[0]);
      if (tts == NULL) {
        cerr << "Instance " << m_instance << " error: cannot select default text to speech engine, use one of:\n";
        for (PFactory<PTextToSpeech>::KeyList_T::iterator it = engines.begin(); it != engines.end(); ++it)
          cerr << "  " << *it << '\n';
        return false;
      }
    }
  }
  cout << "Instance " << m_instance << " using text to speech \"" << tts->GetVoiceList() << "\"" << endl;

  m_vxml = new PVXMLSession(tts, true);
  if (!m_vxml->Load(args[0])) {
    cerr << "Instance " << m_instance << " error: cannot loading VXML document \"" << args[0] << "\" - " << m_vxml->GetXMLError() << endl;
    return false;
  }

  if (args.HasOption('C'))
    m_vxml->GetCache().SetDirectory(args.GetOptionString('C'));

  if (!m_vxml->Open(VXML_PCM16)) {
    cerr << "Instance " << m_instance << " error: cannot open VXML device in PCM mode" << endl;
    return false;
  }

#if P_VXML_VIDEO
  m_videoSenderThread = new PThreadObj<TestInstance>(*this, &TestInstance::CopyVideoSender, false, "CopyVideoSender");
  m_videoReceiverThread = new PThreadObj<TestInstance>(*this, &TestInstance::CopyVideoReceiver, false, "CopyVideoReceiver");
#endif

  m_audioThread = new PThreadObj<TestInstance>(*this, &TestInstance::CopyAudio, false, "CopyAudio");

  cout << "Started test instance " << m_instance << " on " << args[0].ToLiteral() << endl;
  return true;
}


void TestInstance::SendInput(const PString & digits)
{
  if (m_vxml != NULL)
    m_vxml->OnUserInput(digits);
}


void TestInstance::CopyAudio()
{
  PBYTEArray audioPCM(1024);

  for (;;) {
    if (!m_vxml->Read(audioPCM.GetPointer(), audioPCM.GetSize())) {
      if (m_vxml->GetErrorCode(PChannel::LastReadError) != PChannel::NotOpen) {
        PTRACE(2, "Instance " << m_instance << " audio read error " << m_vxml->GetErrorText());
      }
      break;
    }

    if (!m_player->Write(audioPCM, m_vxml->GetLastReadCount())) {
      PTRACE_IF(2, m_player->IsOpen(), "Instance " << m_instance << " audio write error " << m_player->GetErrorText());
      break;
    }
  }
}


#if P_VXML_VIDEO
void TestInstance::CopyVideoSender()
{
  PBYTEArray frame;
  PVideoOutputDevice::FrameData frameData;
  PVideoInputDevice & sender = m_vxml->GetVideoSender();
  sender.SetColourFormatConverter(PVideoFrameInfo::YUV420P());
  m_viewer->SetColourFormatConverter(PVideoFrameInfo::YUV420P());

  while (m_viewer != NULL) {
    if (!sender.GetFrame(frame, frameData.width, frameData.height)) {
      PTRACE(2, "Instance " << m_instance << " vxml video preview failed");
      break;
    }

    if (frame.IsEmpty())
      continue;

    m_viewer->SetFrameSize(frameData.width, frameData.height);
    frameData.pixels = frame;

    if (!m_viewer->SetFrameData(frameData)) {
      PTRACE_IF(2, m_viewer->IsOpen(), "Instance " << m_instance << " output video failed");
      break;
    }
  }
}

void TestInstance::CopyVideoReceiver()
{
  PBYTEArray frame;
  PVideoOutputDevice::FrameData frameData;
  PVideoOutputDevice & receiver = m_vxml->GetVideoReceiver();

  receiver.SetColourFormatConverter(m_grabber->GetColourFormat());
  if (m_preview)
    m_preview->SetColourFormatConverter(m_grabber->GetColourFormat());

  while (m_grabber != NULL) {
    if (!m_grabber->GetFrame(frame, frameData.width, frameData.height)) {
      PTRACE(2, "Instance " << m_instance << " grab video failed");
      break;
    }

    if (frame.IsEmpty())
      continue;

    receiver.SetFrameSize(frameData.width, frameData.height);
    frameData.pixels = frame;
    frameData.timestamp = PTime().GetTimestamp();

    if (m_preview)
      m_preview->SetFrameData(frameData);

    if (!receiver.SetFrameData(frameData)) {
      PTRACE(2, "Instance " << m_instance << " vxml video feed failed");
      break;
    }
  }
}
#endif

#else
#pragma message("Cannot compile test application without XML support!")

void VxmlTest::Main()
{
}
#endif


// End of File ///////////////////////////////////////////////////////////////
