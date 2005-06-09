#include <ptlib.h>

#include <ptclib/pwavfile.h>
#include <ptclib/dtmf.h>

#define SAMPLES 64000  

class WAVFileTest : public PProcess
{
  public:
    WAVFileTest()
    : PProcess() { }
    void Main();
};

PCREATE_PROCESS(WAVFileTest)

void WAVFileTest::Main()
{
  PArgList & args = GetArguments();
  args.Parse("p:c:");

  if (args.HasOption('h')) {
    cout << "usage: wavfile [-p device][-c fmt] fn" << endl;
    return;
  }

  if (args.HasOption('c')) {
    PString format = args.GetOptionString('c');
    PWAVFile file(format, args[0], PFile::WriteOnly);
    if (!file.IsOpen()) {
      cout << "error: cannot create file " << args[0] << endl;
      return;
    }

    //BYTE buffer[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    //file.Write(buffer, sizeof(buffer));

    PDTMFEncoder toneData;
    toneData.GenerateDialTone();
    PINDEX len = toneData.GetSize();
    file.Write((const BYTE *)toneData, len);

    file.Close();
  }

  PWAVFile file(args[0], PFile::ReadOnly, PFile::MustExist, PWAVFile::fmt_NotKnown);
  if (!file.IsOpen()) {
    cout << "error: cannot open " << args[0] << endl;
    return;
  }

  PINDEX dataLen = file.GetDataLength();
  PINDEX hdrLen  = file.GetHeaderLength();
  PINDEX fileLen = file.GetLength();

  cout << "Format:       " << file.wavFmtChunk.format << " (" << file.GetFormatString() << ")" << "\n"
       << "Channels:     " << file.wavFmtChunk.numChannels << "\n"
       << "Sample rate:  " << file.wavFmtChunk.sampleRate << "\n"
       << "Bytes/sec:    " << file.wavFmtChunk.bytesPerSec << "\n"
       << "Bytes/sample: " << file.wavFmtChunk.bytesPerSample << "\n"
       << "Bits/sample:  " << file.wavFmtChunk.bitsPerSample << "\n"
       << "\n"
       << "Hdr length :  " << hdrLen << endl
       << "Data length:  " << dataLen << endl
       << "File length:  " << fileLen << " (" << hdrLen + dataLen << ")" << endl
       << endl;

  PBYTEArray data;
  if (!file.Read(data.GetPointer(dataLen), dataLen) || (file.GetLastReadCount() != dataLen)) {
    cout << "error: cannot read " << dataLen << " bytes of WAV data" << endl;
    return;
  }

  if (args.HasOption('p')) {

    PString service = args.GetOptionString('p');
    PString device;
    if (args.GetCount() > 0)
      device  = args[0];
    else if (service != "default") {
      PStringList deviceList = PSoundChannel::GetDeviceNames(service, PSoundChannel::Player);
      if (deviceList.GetSize() == 0) {
        cout << "error: No devices for sound service " << service << endl;
        return;
      }
      device = deviceList[0];
    }
    
    cout << "Using sound service " << service << " with device " << device << endl;

    PSoundChannel * snd;
    if (service == "default") {
      snd = new PSoundChannel();
      device = PSoundChannel::GetDefaultDevice(PSoundChannel::Player);
    }
    else {
      snd = PSoundChannel::CreateChannel(service);
      if (snd == NULL) {
        cout << "Failed to create sound service " << service << " with device " << device << endl;
        return;
      }
    }

    cout << "Opening sound service " << service << " with device " << device << endl;

    if (!snd->Open(device, PSoundChannel::Player)) {
      cout << "Failed to open sound service " << service << " with device " << device << endl;
      return;
    }

    if (!snd->IsOpen()) {
      cout << "Sound device " << device << " not open" << endl;
      return;
    }

    if (!snd->SetBuffers(SAMPLES, 2)) {
      cout << "Failed to set samples to " << SAMPLES << " and 2 buffers. End program now." << endl;
      return;
    }

    snd->SetVolume(50);

    if (!snd->Write((const BYTE *)data, data.GetSize())) {
      cout << "error: write to audio device failed" << endl;
      return;
    }

    snd->WaitForPlayCompletion();

  }
}
