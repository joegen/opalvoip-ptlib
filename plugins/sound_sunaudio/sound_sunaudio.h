#ifndef SUNAUDIO_H 
#define SUNAUDIO_H

#include <ptlib.h>
#include <ptlib/sound.h>
#include <ptlib/socket.h>
#include <ptlib/plugin.h>

#include <sys/audio.h>

class PSoundChannelSunAudio: public PSoundChannel
{
 public:
    PSoundChannelSunAudio();
    ~PSoundChannelSunAudio();
    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);
    static PString GetDefaultDevice(PSoundChannel::Directions);
    bool Open(const Params & params);
    PBoolean Setup();
    PBoolean Close();
    PBoolean IsOpen() const;
    PBoolean Write(const void * buf, PINDEX len);
    PBoolean Read(void * buf, PINDEX len);
    PBoolean SetFormat(unsigned numChannels,
                   unsigned sampleRate,
                   unsigned bitsPerSample);
    unsigned GetChannels() const;
    unsigned GetSampleRate() const;
    unsigned GetSampleSize() const;
    PBoolean SetBuffers(PINDEX size, PINDEX count);
    PBoolean GetBuffers(PINDEX & size, PINDEX & count);
    PBoolean PlaySound(const PSound & sound, PBoolean wait);
    PBoolean PlayFile(const PFilePath & filename, PBoolean wait);
    PBoolean HasPlayCompleted();
    PBoolean WaitForPlayCompletion();
    PBoolean RecordSound(PSound & sound);
    PBoolean RecordFile(const PFilePath & filename);
    PBoolean StartRecording();
    PBoolean IsRecordBufferFull();
    PBoolean AreAllRecordBuffersFull();
    PBoolean WaitForRecordBufferFull();
    PBoolean WaitForAllRecordBuffersFull();
    PBoolean Abort();
    PBoolean SetVolume(unsigned newVal);
    PBoolean GetVolume(unsigned &devVol);

  protected:
    unsigned mNumChannels;
    unsigned mSampleRate;
    unsigned mBitsPerSample;
    unsigned actualSampleRate;
    PString device;
    PBoolean isInitialised;
    unsigned resampleRate;

    /* save the default settings for resetting */
    /* play */
    unsigned mDefaultPlayNumChannels;
    unsigned mDefaultPlaySampleRate;
    unsigned mDefaultPlayBitsPerSample;

    /* record */
    unsigned mDefaultRecordNumChannels;
    unsigned mDefaultRecordSampleRate;
    unsigned mDefaultRecordBitsPerSample;
    unsigned mDefaultRecordEncoding;
    unsigned mDefaultRecordPort;
};

#endif
