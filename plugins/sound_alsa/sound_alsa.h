#include <ptlib.h>
#include <ptlib/sound.h>

#define ALSA_PCM_NEW_HW_PARAMS_API 1
#include <alsa/asoundlib.h>


class PSoundChannelALSA : public PSoundChannel {
 public:
  PSoundChannelALSA();
  ~PSoundChannelALSA();
  static PStringArray GetDeviceNames(PSoundChannel::Directions);
  static PString GetDefaultDevice(PSoundChannel::Directions);
  bool Open(const Params & params);
  PString GetName() const { return device; }
  PBoolean Close();
  PBoolean Write(const void * buf, PINDEX len);
  PBoolean Read(void * buf, PINDEX len);
  PBoolean SetFormat(unsigned numChannels, unsigned sampleRate, unsigned bitsPerSample);
  unsigned GetChannels() const;
  unsigned GetSampleRate() const;
  unsigned GetSampleSize() const;
  PBoolean SetBuffers(PINDEX size, PINDEX count);
  PBoolean GetBuffers(PINDEX & size, PINDEX & count);
  PBoolean HasPlayCompleted();
  PBoolean WaitForPlayCompletion();
  PBoolean StartRecording();
  PBoolean IsRecordBufferFull();
  PBoolean AreAllRecordBuffersFull();
  PBoolean WaitForRecordBufferFull();
  PBoolean WaitForAllRecordBuffersFull();
  PBoolean Abort();
  PBoolean SetVolume(unsigned);
  PBoolean GetVolume(unsigned &);
  PBoolean IsOpen() const;

 private:
  static void UpdateDictionary(PSoundChannel::Directions);
  bool SetHardwareParams();
  PBoolean Volume(PBoolean, unsigned, unsigned &);

  PString device;
  unsigned mNumChannels;
  unsigned mSampleRate;
  unsigned mBitsPerSample;
  PBoolean isInitialised;

  snd_pcm_t *pcm_handle; /* Handle, different from the PChannel handle */
  int card_nr;

  PMutex device_mutex;

  PINDEX m_bufferSize;
  PINDEX m_bufferCount;

  /** Number of bytes in a ALSA frame. a frame may only be 4ms long*/
  int frameBytes; 
};
