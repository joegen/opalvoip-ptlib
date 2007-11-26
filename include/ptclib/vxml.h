/*
 * vxml.h
 *
 * VXML engine for pwlib library
 *
 * Copyright (C) 2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _VXML_H
#define _VXML_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif


#include <ptclib/pxml.h>

#if P_EXPAT

#include <ptlib/pipechan.h>
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>
#include <ptclib/ptts.h>
#include <ptclib/url.h>

#include <queue>


class PVXMLSession;
class PVXMLDialog;
class PVXMLSession;

// these are the same strings as the Opal equivalents, but as this is PWLib, we can't use Opal contants
#define VXML_PCM16         "PCM-16"
#define VXML_G7231         "G.723.1"
#define VXML_G729          "G.729"

#define PVXML_HAS_FACTORY   1

class PVXMLGrammar : public PObject
{
  PCLASSINFO(PVXMLGrammar, PObject);
  public:
    PVXMLGrammar(PXMLElement * field);
    virtual BOOL OnUserInput(const char /*ch*/) { return TRUE; }
    virtual void Stop() { }

    PString GetValue() const { return value; }
    PXMLElement * GetField() { return field; }

    enum GrammarState { 
      FILLED,       ///< got something that matched the grammar
      NOINPUT,      ///< timeout or still waiting to match
      NOMATCH,      ///< recognized something but didn't match the grammar
      HELP };       ///< help keyword

    GrammarState GetState() const { return state; }

  protected:
    PXMLElement * field;
    PString value;
    GrammarState state;
};


//////////////////////////////////////////////////////////////////

class PVXMLMenuGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLMenuGrammar, PVXMLGrammar);
  public:
    PVXMLMenuGrammar(PXMLElement * field);
};


//////////////////////////////////////////////////////////////////

class PVXMLDigitsGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLDigitsGrammar, PVXMLGrammar);
  public:
    PVXMLDigitsGrammar(PXMLElement * field, PINDEX minDigits, PINDEX maxDigits, PString terminators);
    BOOL OnUserInput(const char ch);
    virtual void Stop();

  protected:
    PINDEX minDigits;
    PINDEX maxDigits;
    PString terminators;
};


//////////////////////////////////////////////////////////////////

class PVXMLCache : public PMutex
{
  public:
    PVXMLCache(const PDirectory & _directory);

    PFilePath CreateFilename(const PString & prefix, const PString & key, const PString & fileType);

    void Put(const PString & prefix,
             const PString & key, 
             const PString & fileType, 
             const PString & contentType,       
           const PFilePath & fn, 
                 PFilePath & dataFn);

    BOOL Get(const PString & prefix,
             const PString & key, 
             const PString & fileType, 
                   PString & contentType,       
                 PFilePath & fn);

    PFilePath GetCacheDir() const
    { return directory; }

    PFilePath GetRandomFilename(const PString & prefix, const PString & fileType);

    static PVXMLCache & GetResourceCache();

  protected:
    PDirectory directory;
};

//////////////////////////////////////////////////////////////////

class PVXMLChannel;

class PVXMLChannelInterface {
  public:
    virtual ~PVXMLChannelInterface() { }
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn, PFile::OpenMode mode, int opts, unsigned fmt) = 0;
    virtual void RecordEnd() = 0;
    virtual void OnEndRecording(const PString & channelName) = 0;
    virtual void Trigger() = 0;
};

//////////////////////////////////////////////////////////////////

class PVXMLSession : public PIndirectChannel, public PVXMLChannelInterface
{
  PCLASSINFO(PVXMLSession, PIndirectChannel);
  public:
    PVXMLSession(PTextToSpeech * tts = NULL, BOOL autoDelete = FALSE);
    virtual ~PVXMLSession();

    // new functions
    PTextToSpeech * SetTextToSpeech(PTextToSpeech * _tts, BOOL autoDelete = FALSE);
    PTextToSpeech * SetTextToSpeech(const PString & ttsName);
    PTextToSpeech * GetTextToSpeech() { return textToSpeech; }

    virtual BOOL Load(const PString & source);
    virtual BOOL LoadFile(const PFilePath & file);
    virtual BOOL LoadURL(const PURL & url);
    virtual BOOL LoadVXML(const PString & xml);
    virtual BOOL IsLoaded() const { return loaded; }

    virtual BOOL Open(BOOL isPCM); // For backward compatibility FALSE=G.723.1
    virtual BOOL Open(const PString & mediaFormat);
    virtual BOOL Close();

    BOOL Execute();

    PVXMLChannel * GetAndLockVXMLChannel() 
    { 
      sessionMutex.Wait(); 
      if (vxmlChannel != NULL) 
        return vxmlChannel; 
      sessionMutex.Signal();
      return NULL;
    }
    void UnLockVXMLChannel() { sessionMutex.Signal(); }
    PMutex & GetSessionMutex() { return sessionMutex; }

    BOOL LoadGrammar(PVXMLGrammar * grammar);

    virtual BOOL PlayText(const PString & text, PTextToSpeech::TextType type = PTextToSpeech::Default, PINDEX repeat = 1, PINDEX delay = 0);
    BOOL ConvertTextToFilenameList(const PString & _text, PTextToSpeech::TextType type, PStringArray & list, BOOL useCacheing);

    virtual BOOL PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE);
    virtual BOOL PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayCommand(const PString & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayResource(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayTone(const PString & toneSpec, PINDEX repeat = 1, PINDEX delay = 0);

    //virtual BOOL PlayMedia(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlaySilence(PINDEX msecs = 0);
    virtual BOOL PlaySilence(const PTimeInterval & timeout);

    virtual BOOL PlayStop();

    virtual void SetPause(BOOL pause);
    virtual void GetBeepData(PBYTEArray & data, unsigned ms);

    virtual BOOL StartRecording(const PFilePath & fn, BOOL recordDTMFTerm, const PTimeInterval & recordMaxTime, const PTimeInterval & recordFinalSilence);
    virtual BOOL EndRecording();
    virtual BOOL IsPlaying() const;
    virtual BOOL IsRecording() const;

    virtual BOOL OnUserInput(const PString & str);

    PString GetXMLError() const;

    virtual void OnEndSession()         { }

    virtual PString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);
    virtual PString EvaluateExpr(const PString & oexpr);

    virtual BOOL RetreiveResource(const PURL & url, PString & contentType, PFilePath & fn, BOOL useCache = TRUE);

    PDECLARE_NOTIFIER(PThread, PVXMLSession, VXMLExecute);

    void SetCallingToken( PString& token ) { callingCallToken = token; }

    PXMLElement * FindHandler(const PString & event);

    // overrides from VXMLChannelInterface
    PWAVFile * CreateWAVFile(const PFilePath & fn, PFile::OpenMode mode, int opts, unsigned fmt);
    void OnEndRecording(const PString & channelName);
    void RecordEnd();
    void Trigger();

    PStringToString & GetSessionVars() { return sessionVars; }

  protected:
    void Initialise();

    void AllowClearCall();
    void ProcessUserInput();
    void ProcessNode();
    void ProcessGrammar();

    BOOL TraverseAudio();
    BOOL TraverseGoto();
    BOOL TraverseGrammar();
    BOOL TraverseRecord();

    BOOL TraverseIf();
    BOOL TraverseExit();
    BOOL TraverseVar();
    BOOL TraverseSubmit();
    BOOL TraverseMenu();
    BOOL TraverseChoice(const PString & grammarResult);
    BOOL TraverseProperty();

    void SayAs(const PString & className, const PString & text);
    void SayAs(const PString & className, const PString & text, const PString & voice);

    static PTimeInterval StringToTime(const PString & str);

    PURL NormaliseResourceName(const PString & src);

    PXMLElement * FindForm(const PString & id);

    //friend class PVXMLChannel;

    PSyncPoint waitForEvent;

    PMutex sessionMutex;

    PXML xmlFile;

    PVXMLGrammar * activeGrammar;
    BOOL listening;                 // TRUE if waiting for recognition events
    int timeout;                    // timeout in msecs for the current recognition

    PStringToString sessionVars;
    PStringToString documentVars;

    PMutex userInputMutex;
    std::queue<char> userInputQueue;

    BOOL recording;
    PFilePath recordFn;
    BOOL recordDTMFTerm;
    PTimeInterval recordMaxTime;
    PTimeInterval recordFinalSilence;
    PSyncPoint    recordSync;

    BOOL loaded;
    PURL rootURL;
    BOOL emptyAction;

    PThread * vxmlThread;
    BOOL threadRunning;
    BOOL forceEnd;

    PString mediaFormat;
    PVXMLChannel * vxmlChannel;

    PTextToSpeech * textToSpeech;
    BOOL autoDeleteTextToSpeech;

    PXMLElement * currentForm;
    PXMLElement * currentField;
    PXMLObject  * currentNode;

  private:
    void      ExecuteDialog();

    PString       callingCallToken;
    PSyncPoint    answerSync;
    PString       grammarResult;
    PString       eventName;
    PINDEX        defaultDTMF;
};


//////////////////////////////////////////////////////////////////

class PVXMLRecordable : public PObject
{
  PCLASSINFO(PVXMLRecordable, PObject);
  public:
    PVXMLRecordable()
    { consecutiveSilence = 0; finalSilence = 3000; maxDuration = 30000; }

    virtual BOOL Open(const PString & _arg) = 0;

    virtual void Record(PVXMLChannel & incomingChannel) = 0;

    virtual void OnStart() { }

    virtual BOOL OnFrame(BOOL /*isSilence*/) { return FALSE; }

    virtual void OnStop() { }

    void SetFinalSilence(unsigned v)
    { finalSilence = v; }

    unsigned GetFinalSilence()
    { return finalSilence; }

    void SetMaxDuration(unsigned v)
    { maxDuration = v; }

    unsigned GetMaxDuration()
    { return maxDuration; }

  protected:
    PTime silenceStart;
    PTime recordStart;
    unsigned finalSilence;
    unsigned maxDuration;
    unsigned consecutiveSilence;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayable : public PObject
{
  PCLASSINFO(PVXMLPlayable, PObject);
  public:
    PVXMLPlayable()
    { repeat = 1; delay = 0; sampleFrequency = 8000; autoDelete = FALSE; delayDone = FALSE; }

    virtual BOOL Open(PVXMLChannel & /*chan*/, PINDEX _delay, PINDEX _repeat, BOOL _autoDelete)
    { delay = _delay; repeat = _repeat; autoDelete = _autoDelete; return TRUE; }

    virtual BOOL Open(PVXMLChannel & chan, const PString & _arg, PINDEX _delay, PINDEX _repeat, BOOL v)
    { arg = _arg; return Open(chan, _delay, _repeat, v); }

    virtual void Play(PVXMLChannel & outgoingChannel) = 0;

    virtual void OnRepeat(PVXMLChannel & /*outgoingChannel*/)
    { }

    virtual void OnStart() { }

    virtual void OnStop() { }

    virtual void SetRepeat(PINDEX v) 
    { repeat = v; }

    virtual PINDEX GetRepeat() const
    { return repeat; }

    virtual PINDEX GetDelay() const
    { return delay; }

    void SetFormat(const PString & _fmt)
    { format = _fmt; }

    void SetSampleFrequency(unsigned _rate)
    { sampleFrequency = _rate; }

    virtual BOOL ReadFrame(PVXMLChannel & channel, void * buf, PINDEX len);

    virtual BOOL Rewind(PChannel *) 
    { return FALSE; }

    friend class PVXMLChannel;

  protected:
    PString arg;
    PINDEX repeat;
    PINDEX delay;
    PString format;
    unsigned sampleFrequency;
    BOOL autoDelete;
    BOOL delayDone; // very tacky flag used to indicate when the post-play delay has been done
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableStop : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableStop, PVXMLPlayable);
  public:
    virtual void Play(PVXMLChannel & outgoingChannel);
    virtual BOOL ReadFrame(PVXMLChannel & channel, void *, PINDEX);
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableURL : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableURL, PVXMLPlayable);
  public:
    BOOL Open(PVXMLChannel & chan, const PString & _url, PINDEX _delay, PINDEX _repeat, BOOL v);
    void Play(PVXMLChannel & outgoingChannel);
  protected:
    PURL url;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableData : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableData, PVXMLPlayable);
  public:
    BOOL Open(PVXMLChannel & chan, const PString & /*_fn*/, PINDEX _delay, PINDEX _repeat, BOOL v);
    void SetData(const PBYTEArray & _data);
    void Play(PVXMLChannel & outgoingChannel);
    BOOL Rewind(PChannel * chan);
  protected:
    PBYTEArray data;
};

//////////////////////////////////////////////////////////////////

#include <ptclib/dtmf.h>

class PVXMLPlayableTone : public PVXMLPlayableData
{
  PCLASSINFO(PVXMLPlayableTone, PVXMLPlayableData);
  public:
    BOOL Open(PVXMLChannel & chan, const PString & toneSpec, PINDEX _delay, PINDEX _repeat, BOOL v);
  protected:
    PTones tones;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableCommand : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableCommand, PVXMLPlayable);
  public:
    PVXMLPlayableCommand();
    void Play(PVXMLChannel & outgoingChannel);
    void OnStop();

  protected:
    PPipeChannel * pipeCmd;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableFilename : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableFilename, PVXMLPlayable);
  public:
    BOOL Open(PVXMLChannel & chan, const PString & _fn, PINDEX _delay, PINDEX _repeat, BOOL _autoDelete);
    void Play(PVXMLChannel & outgoingChannel);
    void OnStop();
    virtual BOOL Rewind(PChannel * chan);
  protected:
    PFilePath fn;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableFilenameList : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableFilenameList, PVXMLPlayable);
  public:
    BOOL Open(PVXMLChannel & chan, const PStringArray & _filenames, PINDEX _delay, PINDEX _repeat, BOOL _autoDelete);
    void Play(PVXMLChannel & outgoingChannel)
    { OnRepeat(outgoingChannel); }
    void OnRepeat(PVXMLChannel & outgoingChannel);
    void OnStop();
  protected:
    PINDEX currentIndex;
    PStringArray filenames;
};

//////////////////////////////////////////////////////////////////

class PVXMLRecordableFilename : public PVXMLRecordable
{
  PCLASSINFO(PVXMLRecordableFilename, PVXMLRecordable);
  public:
    BOOL Open(const PString & _arg);
    void Record(PVXMLChannel & incomingChannel);
    BOOL OnFrame(BOOL isSilence);

  protected:
    PFilePath fn;
};

//////////////////////////////////////////////////////////////////

PQUEUE(PVXMLQueue, PVXMLPlayable);

//////////////////////////////////////////////////////////////////

class PVXMLChannel : public PDelayChannel
{
  PCLASSINFO(PVXMLChannel, PDelayChannel);
  public:
    PVXMLChannel(unsigned frameDelay, PINDEX frameSize);
    ~PVXMLChannel();

    virtual BOOL Open(PVXMLChannelInterface * _vxml);

    // overrides from PIndirectChannel
    virtual BOOL IsOpen() const;
    virtual BOOL Close();
    virtual BOOL Read(void * buffer, PINDEX amount);
    virtual BOOL Write(const void * buf, PINDEX len);

    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn, BOOL recording = FALSE);

    const PString & GetMediaFormat() const { return mediaFormat; }
    BOOL IsMediaPCM() const { return mediaFormat == "PCM-16"; }
    virtual PString AdjustWavFilename(const PString & fn);

    // Incoming channel functions
    virtual BOOL WriteFrame(const void * buf, PINDEX len) = 0;
    virtual BOOL IsSilenceFrame(const void * buf, PINDEX len) const = 0;

    virtual BOOL QueueRecordable(PVXMLRecordable * newItem);

    BOOL StartRecording(const PFilePath & fn, unsigned finalSilence = 3000, unsigned maxDuration = 30000);
    BOOL EndRecording();
    BOOL IsRecording() const { return recording; }

    // Outgoing channel functions
    virtual BOOL ReadFrame(void * buffer, PINDEX amount) = 0;
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount) = 0;
    virtual void GetBeepData(PBYTEArray &, unsigned) { }

    virtual BOOL QueueResource(const PURL & url, PINDEX repeat= 1, PINDEX delay = 0);

    virtual BOOL QueuePlayable(const PString & type, const PString & str, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE);
    virtual BOOL QueuePlayable(PVXMLPlayable * newItem);
    virtual BOOL QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);

    virtual BOOL QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE)
    { return QueuePlayable("File", fn, repeat, delay, autoDelete); }

    virtual BOOL QueueCommand(const PString & cmd, PINDEX repeat = 1, PINDEX delay = 0)
    { return QueuePlayable("Command", cmd, repeat, delay, TRUE); }

    virtual void FlushQueue();
    virtual BOOL IsPlaying() const   { return (playQueue.GetSize() > 0) || playing ; }

    void SetPause(BOOL _pause) { paused = _pause; }

    void SetName(const PString & name) { channelName = name; }

    unsigned GetSampleFrequency() const
    { return sampleFrequency; }

  protected:
    PVXMLChannelInterface * vxmlInterface;

    unsigned sampleFrequency;
    PString mediaFormat;
    PString wavFilePrefix;

    PMutex channelWriteMutex;
    PMutex channelReadMutex;
    BOOL closed;

    // Incoming audio variables
    BOOL recording;
    PVXMLRecordable * recordable;
    unsigned finalSilence;
    unsigned silenceRun;

    // Outgoing audio variables
    BOOL playing;
    PMutex queueMutex;
    PVXMLQueue playQueue;
    PVXMLPlayable * currentPlayItem;

    BOOL paused;
    int silentCount;
    int totalData;
    PTimer delayTimer;

    // "channelname" (which is the name of the <record> tag) so
    // results can be saved in vxml session variable
    PString channelName;
};


//////////////////////////////////////////////////////////////////

#endif // P_EXPAT
#endif


// End of file ////////////////////////////////////////////////////////////////
