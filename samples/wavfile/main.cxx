/*
 * main.cxx - do wave file things.
 *
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.0 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS"
 * basis, WITHOUT WARRANTY OF ANY KIND, either express or implied. See
 * the License for the specific language governing rights and limitations
 * under the License.
 *
 * The Original Code is Portable Windows Library.
 *
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptlib.h>
#include <ptclib/pwavfile.h>
#include <ptclib/dtmf.h>
#include <ptlib/sound.h>
#include <ptlib/pprocess.h>


class WAVFileTest : public PProcess
{
  public:
    WAVFileTest()
    : PProcess() { }
    void Main();
    void Create(PArgList & args);
    void Play(PArgList & args);
    void Record(PArgList & args);
};

PCREATE_PROCESS(WAVFileTest)


void WAVFileTest::Main()
{
  PArgList & args = GetArguments();
  if (!args.Parse("r: Record for N seconds\n"
                  "c: Create file from generated tones\n"
                  "F: WAV file format for record/create (default PCM-16)\n"
                  "R: Sample rate (default 8000)\n"
                  "C: Channels (1=mono, 2=stereo etc)\n"
                  "d: Device name for sound channel record/playback\n"
                  "D: Driver name for sound channel record/playback\n"
                  "v: Set sound device to vol (0..100)\n"
                  "B: Set sound device buffer size (10000)\n"
                  PTRACE_ARGLIST)) {
    args.Usage(cerr, "[ options ] filename");
    return;
  }

  PTRACE_INITIALISE(args);

  if (args.HasOption('c'))
    Create(args);
  else if (args.HasOption('r'))
    Record(args);
  else
    Play(args);
}


void WAVFileTest::Create(PArgList & args)
{
  PWAVFile file(args[0], PFile::WriteOnly);
  if (!file.IsOpen()) {
    cout << "Cannot create wav file " << args[0] << endl;
    return;
  }

  if (args.HasOption('F'))
    file.SetFormat(args.GetOptionString('F'));

  if (args.HasOption('C'))
    file.SetChannels(args.GetOptionString('C').AsUnsigned());

  if (args.HasOption('R'))
    file.SetSampleRate(args.GetOptionString('R').AsUnsigned());

  PTones toneData(args.GetOptionString('c'), PTones::MaxVolume, file.GetSampleRate());
  file.Write((const short *)toneData, toneData.GetSize()*sizeof(short));
}


void WAVFileTest::Play(PArgList & args)
{
  PWAVFile file(args[0], PFile::ReadOnly, PFile::MustExist);
  if (!file.IsOpen()) {
    cout << "Cannot open " << args[0] << endl;
    return;
  }

  PINDEX dataLen = file.GetLength();
  PINDEX hdrLen  = file.PFile::GetPosition();
  PINDEX fileLen = file.PFile::GetLength();

  cout << "Format:       " << file.GetFormat() << " (" << file.GetFormatString() << ")" << "\n"
          "Channels:     " << file.GetChannels() << "\n"
          "Sample rate:  " << file.GetSampleRate() << "Hz\n"
          "Bytes/sec:    " << file.GetBytesPerSecond() << "\n"
          "Bits/sample:  " << file.GetSampleSize() << "\n"
          "\n"
          "Hdr length :  " << hdrLen << "\n"
          "Data length:  " << dataLen << "\n"
       << "File length:  " << fileLen << " (" << hdrLen + dataLen << ")\n\n";

  if (args.HasOption('C')) {
    unsigned channels = args.GetOptionString('C').AsUnsigned();
    if (channels != file.GetChannels()) {
      file.SetChannels(channels);
      cout << "Converting to " << channels << " channels.\n";
    }
  }

  if (args.HasOption('R')) {
    unsigned rate = args.GetOptionString('R').AsUnsigned();
    if (rate != file.GetSampleRate()) {
      file.SetSampleRate(rate);
      cout << "Converting to " << rate << "Hz.\n";
    }
  }

  cout.flush();

  PSoundChannel * sound = PSoundChannel::CreateOpenedChannel(args.GetOptionString('D'),
                                                             args.GetOptionString('d'),
                                                             PSoundChannel::Player,
                                                             file.GetChannels(),
                                                             file.GetSampleRate(),
                                                             file.GetSampleSize());
  if (sound == NULL) {
    cout << "Failed to create sound channel." << endl;
    return;
  }

  sound->SetVolume(args.GetOptionString('v', "50").AsUnsigned());

  unsigned sampleSize = (file.GetChannels()*file.GetSampleSize() + 7) / 8;
  unsigned bufferSize = (args.GetOptionString('B', "10000").AsUnsigned()+sampleSize-1)/sampleSize*sampleSize;
  if (!sound->SetBuffers(bufferSize, 2)) {
    cout << "Failed to set samples to " << bufferSize << " and 2 buffers. End program now." << endl;
    return;
  }

  PBYTEArray data(bufferSize);
  while (file.Read(data.GetPointer(), bufferSize)) {
    if (!sound->Write(data.GetPointer(), file.GetLastReadCount())) {
      cout << "error: write to audio device failed" << endl;
      return;
    }
  }

  sound->WaitForPlayCompletion();
  delete sound;

  if (file.GetErrorCode() != PChannel::NoError)
    cout << "Error reading " << file.GetFilePath() << " - " << file.GetErrorText() << endl;
}


void WAVFileTest::Record(PArgList & args)
{
  PWAVFile file(args[0], PFile::WriteOnly);
  if (!file.IsOpen()) {
    cout << "Cannot open " << args[0] << endl;
    return;
  }

  if (args.HasOption('F'))
    file.SetFormat(args.GetOptionString('F'));

  if (args.HasOption('C'))
    file.SetChannels(args.GetOptionString('C').AsUnsigned());

  if (args.HasOption('R'))
    file.SetSampleRate(args.GetOptionString('R').AsUnsigned());

  PSoundChannel * sound = PSoundChannel::CreateOpenedChannel(args.GetOptionString('D'),
                                                             args.GetOptionString('d'),
                                                             PSoundChannel::Recorder,
                                                             file.GetChannels(),
                                                             file.GetSampleRate(),
                                                             file.GetSampleSize());
  if (sound == NULL) {
    cout << "Failed to create sound channel." << endl;
    return;
  }

  sound->SetVolume(args.GetOptionString('v', "50").AsUnsigned());

  PSimpleTimer timer(0, args.GetOptionString('r').AsUnsigned());
  cout << "Recording WAV file for " << timer << " seconds ..." << endl;
  while (timer.IsRunning()) {
    BYTE buffer[8192];
    if (!sound->Read(buffer, sizeof(buffer))) {
      cout << "Error reading sound channel: " << sound->GetErrorText() << endl;
      break;
    }
    if (!file.Write(buffer, sound->GetLastReadCount())) {
      cout << "Error writing WAV file: " << file.GetErrorText() << endl;
      break;
    }
  }

  delete sound;
}
