
#include <ptlib.h>
#include <ptlib/sound.h>
#include <ptlib/socket.h>

//#if !P_USE_INLINES
//#include <ptlib/contain.inl>
//#endif

#ifdef P_SOLARIS
#include <sys/soundcard.h>
#endif

#ifdef P_GNU_HURD
#include <sys/soundcard.h>
#endif

#ifdef P_LINUX
#include <sys/soundcard.h>
#endif

#ifdef P_FREEBSD
#if P_FREEBSD >= 500000
#include <sys/soundcard.h>
#else
#include <machine/soundcard.h>
#endif
#endif

#if defined(P_OPENBSD) || defined(P_NETBSD)
#include <soundcard.h>
#endif

class SoundHandleEntry : public PObject {

  PCLASSINFO(SoundHandleEntry, PObject)

  public:
    SoundHandleEntry();

    int handle;
    int direction;

    unsigned numChannels;
    unsigned sampleRate;
    unsigned bitsPerSample;
    unsigned fragmentValue;
    PBoolean isInitialised;
    unsigned resampleRate;
};

class PSoundChannelOSS: public PSoundChannel
{
 public:
    PSoundChannelOSS();
    ~PSoundChannelOSS();
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
    PBoolean HasPlayCompleted();
    PBoolean WaitForPlayCompletion();
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
};
