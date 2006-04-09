/*
 * main.h
 *
 * PWLib application header file for sound test.
 *
 *
 * $Log: audio.h,v $
 * Revision 1.3  2006/04/09 05:13:06  dereksmithies
 * add a means to write the collected audio to disk (as a wav file),
 *    or to the trace log (as text data)
 *
 * Revision 1.2  2005/11/30 12:47:39  csoutheren
 * Removed tabs, reformatted some code, and changed tags for Doxygen
 *
 * Revision 1.1  2005/08/18 22:29:15  dereksmithies
 * Add a full duplex sound card test (which was excised from ohphone).
 * Add copyright header and cvs log statements.
 * Fix startup and closedown segfaults.
 * Add safety mechanism so it can never fill up all computer memory.
 *
 *
 *
 *
 *
 *
 *
 *
 */
 
#ifndef _AUDIO_MAIN_H
#define _AUDIO_MAIN_H



class Audio : public PProcess
{
  PCLASSINFO(Audio, PProcess);

public:
  Audio();

  void Main();

  PString GetTestDeviceName() { return devName; }

    static Audio & Current()
        { return (Audio &)PProcess::Current(); }

 protected:
  PString devName;
};


/////////////////////////////////////////////////////////////////////////////
PDECLARE_LIST(TestAudioDevice, PBYTEArray *)
#if 0                                //This makes emacs bracket matching code happy.
{
#endif
 public:
  virtual ~TestAudioDevice();
  
  void Test(const PString & captureFileName);
  BOOL DoEndNow();
  
  void WriteAudioFrame(PBYTEArray *data);
  PBYTEArray *GetNextAudioFrame();
  
 protected:
  PMutex access;
  BOOL endNow;

};



class TestAudio : public PThread  
{
  PCLASSINFO(TestAudio, PThread)
  public:
    TestAudio(TestAudioDevice &master);
    virtual ~TestAudio();

    virtual void Terminate() { keepGoing = FALSE; }
    void LowerVolume();
    void RaiseVolume();

  protected:
    PString name;
    BOOL OpenAudio(enum PSoundChannel::Directions dir);

    PINDEX             currentVolume;
    TestAudioDevice    &controller;
    PSoundChannel      sound;
    BOOL               keepGoing;
};

class TestAudioRead : public TestAudio
{
    PCLASSINFO(TestAudioRead, TestAudio);
  public:
    TestAudioRead(TestAudioDevice &master, const PString & _captureFileName);
    
    void Main();
 protected:
    PString captureFileName;
};


class TestAudioWrite : public TestAudio
{
    PCLASSINFO(TestAudioWrite, TestAudio);
  public:
    TestAudioWrite(TestAudioDevice &master);
    
    void Main();
};




#endif  // _AUDIO_MAIN_H
