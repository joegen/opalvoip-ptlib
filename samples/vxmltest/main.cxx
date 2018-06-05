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

#include "main.h"

PCREATE_PROCESS(VxmlTest);

#define PTraceModule() "VXMLTest"


VxmlTest::VxmlTest()
  : PProcess("Equivalence", "vxmltest", 1, 0, AlphaCode, 1)
  , m_player(NULL)
  , m_grabber(NULL)
  , m_viewer(NULL)
  , m_vxml(NULL)
{
}

#if P_VXML

void VxmlTest::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("-sound-driver: Output to sound driver\n"
                  "S-sound-device: Output to sound device\n"
                  "T-tts: Text to speech method\n"
#if P_VXML_VIDEO
                  "V-video. Enabled video support\n"
                  "L-sign-language: Set sign language analyser library (implicit -V)\n"
                  "-input-driver: Video input driver\n"
                  "I-input-device: Video input device\n"
                  "C-input-channel: Video input channel\n"
                  "-output-driver: Video output driver\n"
                  "O-output-device: Video output device\n"
                  "-output-channel: Video output channel\n"
#endif
                  PTRACE_ARGLIST)) {
    args.Usage(cerr, "[ options ] <vxml-file>");
    return;
  }

  PTRACE_INITIALISE(args);

  PSoundChannel::Params audioParams;
  audioParams.m_direction = PSoundChannel::Player;
  audioParams.m_driver = args.GetOptionString("sound-driver");
  audioParams.m_device = args.GetOptionString("sound-device");
  m_player = PSoundChannel::CreateOpenedChannel(audioParams);
  if (m_player == NULL) {
    cerr << "error: cannot open sound device \"" << audioParams.m_device << "\"" << endl;
    return;
  }
  cout << "Using audio device \"" << m_player->GetName() << "\"" << endl;


#if P_VXML_VIDEO
  if (args.HasOption("video") || args.HasOption("sign-language")) {
    PVideoOutputDevice::OpenArgs videoArgs;
    videoArgs.driverName = args.GetOptionString("input-driver");
    videoArgs.deviceName = args.GetOptionString("input-device");
    videoArgs.channelNumber = args.GetOptionString("input-channel", "-1").AsInteger();
    m_grabber = PVideoInputDevice::CreateOpenedDevice(videoArgs);
    if (m_grabber == NULL) {
      cerr << "error: cannot open video device \"" << videoArgs.deviceName << "\"" << endl;
      return;
    }
    cout << "Using input video device \"" << m_grabber->GetDeviceName() << "\"" << endl;

    videoArgs.driverName = args.GetOptionString("output-driver");
    videoArgs.deviceName = args.GetOptionString("output-device");
    videoArgs.channelNumber = args.GetOptionString("output-channel", "-1").AsInteger();
    m_viewer = PVideoOutputDevice::CreateOpenedDevice(videoArgs);
    if (m_player == NULL) {
      cerr << "error: cannot open video device \"" << videoArgs.deviceName << "\"" << endl;
      return;
    }
    cout << "Using output video device \"" << m_viewer->GetDeviceName() << "\"" << endl;
  }
#endif


  PTextToSpeech * tts = PFactory<PTextToSpeech>::CreateInstance(args.GetOptionString("tts", "Microsoft SAPI"));
  if (tts == NULL) {
    PFactory<PTextToSpeech>::KeyList_T engines = PFactory<PTextToSpeech>::GetKeyList();
    if (!engines.empty()) {
      tts = PFactory<PTextToSpeech>::CreateInstance(engines[0]);
      if (tts == NULL) {
        cerr << "error: cannot select default text to speech engine, use one of:\n";
        for (PFactory<PTextToSpeech>::KeyList_T::iterator it = engines.begin(); it != engines.end(); ++it)
          cerr << "  " << *it << '\n';
        return;
      }
    }
  }
  cout << "Using text to speech \"" << tts->GetVoiceList() << "\"" << endl;

  if (args.HasOption("sign-language")) {
    if (!PVXMLSession::SetSignLanguageAnalyser(args.GetOptionString("sign-language"))) {
      cerr << "error: cannot load sign language analyser" << endl;
      return;
    }
    cout << "Using sign language analyser." << endl;
  }

  m_vxml = new PVXMLSession(tts);
  if (!m_vxml->Load(args[0])) {
    cerr << "error: cannot loading VXML document \"" << args[0] << "\" - " << m_vxml->GetXMLError() << endl;
    return;
  }

  if (!m_vxml->Open(VXML_PCM16)) {
    cerr << "error: cannot open VXML device in PCM mode" << endl;
    return;
  }

  cout << "Starting media, type numeric values to emulate DTMF input." << endl;
  PConsoleChannel console(PConsoleChannel::StandardInput);
  console.SetLineBuffered(false);
  PThread * inputThread = new PThreadObj1Arg<VxmlTest, PConsoleChannel&>(*this, console, &VxmlTest::HandleInput, false, "HandleInput");

#if P_VXML_VIDEO
  PThread * videoSenderThread = new PThreadObj<VxmlTest>(*this, &VxmlTest::CopyVideoSender, false, "CopyVideoSender");
  PThread * videoReceiverThread = new PThreadObj<VxmlTest>(*this, &VxmlTest::CopyVideoReceiver, false, "CopyVideoReceiver");
#endif

  PThread * audioThread = new PThreadObj<VxmlTest>(*this, &VxmlTest::CopyAudio, false, "CopyAudio");
  PThread::WaitAndDelete(audioThread, PMaxTimeInterval);

#if P_VXML_VIDEO
  PThread::WaitAndDelete(videoSenderThread, PMaxTimeInterval);
  PThread::WaitAndDelete(videoReceiverThread, PMaxTimeInterval);
  delete m_viewer;
  delete m_grabber;
#endif
  delete m_player;

  cout << "Completed VXML text" << endl;
  console.Close();
  PThread::WaitAndDelete(inputThread);

  delete m_vxml;
}


void VxmlTest::HandleInput(PConsoleChannel & console)
{
  int inp;
  while ((inp = console.ReadChar()) >= 0) {
    if (inp >= ' ' && inp < '~') {
      m_vxml->OnUserInput((char)inp);
      cout << (char)inp << endl;
    }
  }
}


void VxmlTest::CopyAudio()
{
  PBYTEArray audioPCM(1024);

  for (;;) {
    if (!m_vxml->Read(audioPCM.GetPointer(), audioPCM.GetSize())) {
      if (m_vxml->GetErrorCode(PChannel::LastReadError) != PChannel::NotOpen) {
        PTRACE(2, "Read error " << m_vxml->GetErrorText());
      }
      break;
    }

    if (!m_player->Write(audioPCM, m_vxml->GetLastReadCount())) {
      PTRACE(2, "Write error " << m_player->GetErrorText());
      break;
    }
  }
}


#if P_VXML_VIDEO
void VxmlTest::CopyVideoSender()
{
  PBYTEArray frame;
  PVideoOutputDevice::FrameData frameData;
  PVideoInputDevice & sender = m_vxml->GetVideoSender();
  sender.SetColourFormatConverter(PVideoFrameInfo::YUV420P());
  m_viewer->SetColourFormatConverter(PVideoFrameInfo::YUV420P());

  while (m_viewer != NULL) {
    if (!sender.GetFrame(frame, frameData.width, frameData.height)) {
      PTRACE(2, "Grab video failed");
      break;
    }

    m_viewer->SetFrameSize(frameData.width, frameData.height);
    frameData.pixels = frame;

    if (!m_viewer->SetFrameData(frameData)) {
      PTRACE(2, "Output video failed");
      break;
    }
  }
}

void VxmlTest::CopyVideoReceiver()
{
  PBYTEArray frame;
  PVideoOutputDevice::FrameData frameData;
  PVideoOutputDevice & receiver = m_vxml->GetVideoReceiver();

  receiver.SetColourFormatConverter(m_grabber->GetColourFormat());

  while (m_grabber != NULL) {
    if (!m_grabber->GetFrame(frame, frameData.width, frameData.height)) {
      PTRACE(2, "Grab video failed");
      break;
    }

    receiver.SetFrameSize(frameData.width, frameData.height);
    frameData.pixels = frame;

    if (!receiver.SetFrameData(frameData)) {
      PTRACE(2, "Output video failed");
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
