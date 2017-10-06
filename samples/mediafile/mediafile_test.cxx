/*
 * mediafile_test.cxx
 *
 * Test program for Media File abstraction.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2017 Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tools Library.
 *
 * The Initial Developer of the Original Code is Vox Lucida Pty. Ltd.
 *
 */

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptclib/mediafile.h>


class Test : public PProcess
{
  PCLASSINFO(Test, PProcess)
  public:
    void Main();
    void DoRead(const PFilePath & filename, bool native);
    void DoWrite(const PFilePath & filename, const PStringArray & trackInfo);
};


PCREATE_PROCESS(Test)

void Test::Main()
{
  cout << "Media File Test Utility" << endl;

  PArgList & args = GetArguments();
  if (!args.Parse("w-write: Write track\n"
                  "n-native. Use native format\n"
                  PTRACE_ARGLIST)) {
    args.Usage(cerr, "[ args ] <media-file> ...");
    return;
  }

  PTRACE_INITIALISE(args);

  if (args.HasOption('w')) {
    for (PINDEX i = 0; i < args.GetCount(); ++i)
      DoWrite(args[i], args.GetOptionString('w').Lines());
  }
  else {
    for (PINDEX i = 0; i < args.GetCount(); ++i)
      DoRead(args[i], args.HasOption('n'));
  }
}


void Test::DoRead(const PFilePath & filename, bool native)
{
  PMediaFile * file = PMediaFile::Create(filename);
  if (file == NULL) {
    cerr << "Could not create Media File for " << filename << endl;
    return;
  }

  if (!file->OpenForReading(filename)) {
    cerr << "Could not open " << filename << endl;
    return;
  }

  PMediaFile::TracksInfo tracks;
  if (!file->GetTracks(tracks)) {
    cerr << "Could not get tracks for " << filename << endl;
    return;
  }

  std::set<unsigned> activeTracks;
  for (size_t i = 0; i < tracks.size(); ++i) {
    const PMediaFile::TrackInfo & track = tracks[i];
    cout << "Track " << i << ' ' << track.m_type << ' ' << track.m_format << "\n"
            "  size=" << track.m_size << " bytes\n"
            "  rate=" << track.m_rate << "\n"
            "  frames=" << track.m_frames << " frames\n"
            "  duration=" << PTimeInterval::Seconds(track.m_frames/track.m_rate) << "\n"
            "  channels=" << track.m_channels << "\n"
            "  resolution=" << track.m_width << 'x' << track.m_height << "\n"
         << track.m_options
         << endl;
    activeTracks.insert(i);
  }

  if (native) {
    for (size_t i = 0; i < tracks.size(); ++i) {
      if (activeTracks.find(i) == activeTracks.end())
        continue;

      const PMediaFile::TrackInfo & track = tracks[i];
      PBYTEArray buffer(std::max(100000U, track.m_size));
      PINDEX size = buffer.GetSize();
      unsigned frames = buffer.GetSize() / track.m_size;
      if (file->ReadNative(i, buffer.GetPointer(), size, frames))
        cout << "Read " << frames << " native frames, " << size << " bytes, from track " << i << endl;
      else
        activeTracks.erase(i);
    }
    return;
  }

  PTimeInterval audioOutputTime, videoOutputTime;

  while (!activeTracks.empty()) {
    for (size_t i = 0; i < tracks.size(); ++i) {
      if (activeTracks.find(i) == activeTracks.end())
        continue;

      const PMediaFile::TrackInfo & track = tracks[i];

      if (track.m_type == PMediaFile::Audio()) {
        PBYTEArray buffer((PINDEX)(track.m_rate*track.m_channels*2)); // One second
        while (audioOutputTime <= videoOutputTime) {
          PINDEX length;
          if (file->ReadAudio(i, buffer.GetPointer(), buffer.GetSize(), length)) {
            cout << "Read " << length << " bytes of PCM" << endl;
            audioOutputTime += PTimeInterval::Seconds(length/track.m_rate/2);
          }
          else {
            activeTracks.erase(i);
            audioOutputTime = PMaxTimeInterval;
            break;
          }
        }
      }
      else if (track.m_type == PMediaFile::Video()) {
        PBYTEArray buffer(track.m_width*track.m_height * 3 / 2);
        while (videoOutputTime <= audioOutputTime) {
          if (file->ReadVideo(i, buffer.GetPointer())) {
            cout << "Read frame of YUV" << endl;
            videoOutputTime += PTimeInterval::Frequency(track.m_rate);
          }
          else {
            activeTracks.erase(i);
            videoOutputTime = PMaxTimeInterval;
            break;
          }
        }
      }
      else {
        cout << "Track " << i << " (" << track.m_type << ") be read in native mode" << endl;
        activeTracks.erase(i);
      }
    }
  }
}


void Test::DoWrite(const PFilePath & filename, const PStringArray & trackInfo)
{
  PMediaFile * file = PMediaFile::Create(filename);
  if (file == NULL) {
    cerr << "Could not create Media File for " << filename << endl;
    return;
  }

  if (!file->OpenForWriting(filename)) {
    cerr << "Could not open " << filename << endl;
    return;
  }

  PMediaFile::TracksInfo tracks(trackInfo.GetSize());
  for (PINDEX i = 0; i < trackInfo.GetSize(); ++i) {
    PStringArray params = trackInfo[i].Tokenise(',');
    tracks[i].m_type = params[0];
    if (tracks[i].m_type == PMediaFile::Audio()) {
      tracks[i].m_format = params[1];
      tracks[i].m_rate = params[2].AsReal();
      tracks[i].m_channels = params[3].AsUnsigned();
    }
    else if (tracks[i].m_type == PMediaFile::Video()) {
      tracks[i].m_format = params[1];
      tracks[i].m_width = params[2].AsUnsigned();
      tracks[i].m_height = params[3].AsUnsigned();
      tracks[i].m_rate = params[4].AsReal();
    }
  }

  if (!file->SetTracks(tracks)) {
    cerr << "Could not set tracks for " << filename << endl;
    return;
  }

  PBYTEArray buffer(16000); // One second
  PINDEX written;
  file->WriteAudio(0, buffer, buffer.GetSize(), written);

  buffer.SetSize(640 * 480 * 3 / 2);
  file->WriteVideo(1, buffer);
}


// End of file
