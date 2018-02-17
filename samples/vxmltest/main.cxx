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

VxmlTest::VxmlTest()
  : PProcess("Equivalence", "vxmltest", 1, 0, AlphaCode, 1)
  , m_player(NULL)
  , m_viewer(NULL)
  , m_vxml(NULL)
{
}

#if P_VXML

void VxmlTest::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("S-sound-driver: Output to sound driver\n"
                  "s-sound-device: Output to sound device\n"
                  "T-tts: Text to speech method\n"
#if P_VIDEO
                  "V-video-driver: Output to video driver\n"
                  "v-video-device: Output to video device\n"
#endif
                  PTRACE_ARGLIST)) {
    args.Usage(cerr, "[ options ] <vxml-file>");
    return;
  }

  PTRACE_INITIALISE(args);

  PSoundChannel::Params audioParams;
  audioParams.m_direction = PSoundChannel::Player;
  audioParams.m_driver = args.GetOptionString('S');
  audioParams.m_device = args.GetOptionString('s');
  m_player = PSoundChannel::CreateOpenedChannel(audioParams);
  if (m_player == NULL) {
    PError << "error: cannot open sound device \"" << audioParams.m_device << "\"" << endl;
    return;
  }
  cout << "Using audio device \"" << m_player->GetName() << "\"" << endl;


#if P_VIDEO
  PVideoOutputDevice::OpenArgs videoArgs;
  videoArgs.driverName = args.GetOptionString('V');
  videoArgs.deviceName = args.GetOptionString('v');
  m_viewer = PVideoOutputDevice::CreateOpenedDevice(videoArgs);
  if (m_player == NULL) {
    PError << "error: cannot open video device \"" << videoArgs.deviceName << "\"" << endl;
    return;
  }
  cout << "Using video device \"" << m_viewer->GetDeviceName() << "\"" << endl;
#endif


  PTextToSpeech * tts = PFactory<PTextToSpeech>::CreateInstance(args.GetOptionString('T', "Microsoft SAPI"));
  if (tts == NULL) {
    PFactory<PTextToSpeech>::KeyList_T engines = PFactory<PTextToSpeech>::GetKeyList();
    if (!engines.empty()) {
      tts = PFactory<PTextToSpeech>::CreateInstance(engines[0]);
      if (tts == NULL) {
        PError << "error: cannot select default text to speech engine" << endl;
        return;
      }
    }
  }
  cout << "Using text to speech \"" << tts->GetVoiceList() << "\"" << endl;


  m_vxml = new PVXMLSession(tts);
  if (!m_vxml->Load(args[0])) {
    PError << "error: cannot loading VXML document \"" << args[0] << "\" - " << m_vxml->GetXMLError() << endl;
    return;
  }

  if (!m_vxml->Open(VXML_PCM16)) {
    PError << "error: cannot open VXML device in PCM mode" << endl;
    return;
  }

  cout << "Starting media" << endl;
  PConsoleChannel console(PConsoleChannel::StandardInput);
  console.SetLineBuffered(false);
  PThread * inputThread = new PThreadObj1Arg<VxmlTest, PConsoleChannel&>(*this, console, &VxmlTest::HandleInput, false, "HandleInput");

#if P_VIDEO
  PThread * videoThread = new PThreadObj<VxmlTest>(*this, &VxmlTest::CopyVideo, false, "CopyVideo");
#endif

  PThread * audioThread = new PThreadObj<VxmlTest>(*this, &VxmlTest::CopyAudio, false, "CopyAudio");
  audioThread->WaitForTermination();

  delete audioThread;
#if P_VIDEO
  delete videoThread;
  delete m_viewer;
#endif
  delete m_player;

  cout << "Completed VXML text" << endl;
  console.Close();
  inputThread->WaitForTermination();

  delete inputThread;
  delete m_vxml;
}


void VxmlTest::HandleInput(PConsoleChannel & console)
{
  int inp;
  while ((inp = console.ReadChar()) >= 0)
    m_vxml->OnUserInput((char)inp);
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


#if P_VIDEO
void VxmlTest::CopyVideo()
{
  PBYTEArray frame;

  for (;;) {
    unsigned width, height;
    if (!m_vxml->GetVideoSender().GetFrame(frame, width, height)) {
      PTRACE(2, "Grab video failed");
      break;
    }

    m_viewer->SetFrameSize(width, height);
    if (!m_viewer->SetFrameData(0, 0, width, height, frame)) {
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
