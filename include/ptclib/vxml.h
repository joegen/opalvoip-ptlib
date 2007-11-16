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
 * $Log: vxml.h,v $
 * Revision 1.46  2007/09/04 11:31:48  csoutheren
 * Add PlayTone
 * Add access to session variable table
 *
 * Revision 1.45  2007/07/09 00:10:44  csoutheren
 * Fix compilation without expat
 *
 * Revision 1.44  2005/12/15 21:14:33  dsandras
 * Fixed from Alexander Larsson <alexl _AT__ redhat.com> for gcc 4.1 compilation. Thanks!
 *
 * Revision 1.43  2005/11/30 12:47:37  csoutheren
 * Removed tabs, reformatted some code, and changed tags for Doxygen
 *
 * Revision 1.42  2005/10/30 23:25:51  csoutheren
 * Fixed formatting
 * Removed throw() declarations (PWLib does not do exceptions)
 * Removed duplicate destructor declarations and definitions
 *
 * Revision 1.41  2005/10/30 19:41:53  dominance
 * fixed most of the warnings occuring during compilation
 *
 * Revision 1.40  2005/05/12 13:40:45  csoutheren
 * Fixed locking problems with currentPLayItem optimisation
 *
 * Revision 1.39  2005/05/12 05:28:35  csoutheren
 * Optimised read loop and fixed problems with playing repeated continuous tones
 *
 * Revision 1.38  2005/04/21 05:27:58  csoutheren
 * Changed default value for OnFrame
 *
 * Revision 1.37  2005/03/19 02:52:53  csoutheren
 * Fix warnings from gcc 4.1-20050313 shapshot
 *
 * Revision 1.36  2004/12/08 00:52:06  csoutheren
 * Added GetSampleFrequency function
 *
 * Revision 1.35  2004/11/11 07:34:50  csoutheren
 * Added #include <ptlib.h>
 *
 * Revision 1.34  2004/08/09 11:10:34  csoutheren
 * Changed SetTextToSpeech to return ptr to new engine
 *
 * Revision 1.33  2004/07/26 00:40:41  csoutheren
 * Fixed thread starvation problem under Linux by splitting channelMutex
 * into seperate read and write mutexes
 *
 * Revision 1.32  2004/07/23 00:59:26  csoutheren
 * Check in latest changes
 *
 * Revision 1.31  2004/07/15 03:12:41  csoutheren
 * Migrated changes from crs_vxnml_devel branch into main trunk
 *
 * Revision 1.30.2.7  2004/07/13 08:13:05  csoutheren
 * Lots of implementation of factory-based PWAVFile
 *
 * Revision 1.30.2.6  2004/07/12 08:30:16  csoutheren
 * More fixes for abstract factory implementation of PWAVFile
 *
 * Revision 1.30.2.5  2004/07/08 04:58:11  csoutheren
 * Exposed VXML playable classes to allow descendants
 *
 * Revision 1.30.2.4  2004/07/07 07:07:41  csoutheren
 * Changed PWAVFile to use abstract factories (extensively)
 * Removed redundant blocking/unblocking when using G.723.1
 * More support for call transfer
 *
 * Revision 1.30.2.3  2004/07/06 01:38:57  csoutheren
 * Changed PVXMLChannel to use PDelayChannel
 * Fixed bug where played files were deleted after playing
 *
 * Revision 1.30.2.2  2004/07/02 07:22:37  csoutheren
 * Updated for latest factory changes
 *
 * Revision 1.30.2.1  2004/06/20 11:18:03  csoutheren
 * Rewrite of resource cacheing to cache text-to-speech output
 *
 * Revision 1.30  2004/06/19 07:21:08  csoutheren
 * Change TTS engine registration to use abstract factory code
 * Started disentanglement of PVXMLChannel from PVXMLSession
 * Fixed problem with VXML session closing if played data file is not exact frame size multiple
 * Allowed PVXMLSession to be used without a VXML script
 * Changed PVXMLChannel to handle "file:" URLs
 * Numerous other small improvements and optimisations
 *
 * Revision 1.29  2004/06/02 08:29:28  csoutheren
 * Added new code from Andreas Sikkema to implement various VXML features
 *
 * Revision 1.28  2004/06/02 06:17:21  csoutheren
 * Removed unnecessary buffer copying and removed potential busy loop
 *
 * Revision 1.27  2004/03/23 04:48:42  csoutheren
 * Improved ability to start VXML scripts as needed
 *
 * Revision 1.26  2003/04/23 11:55:13  craigs
 * Added ability to record audio
 *
 * Revision 1.25  2003/04/08 05:09:41  craigs
 * Added ability to use commands as an audio source
 *
 * Revision 1.24  2003/03/18 00:45:36  robertj
 * Fixed missing return in previous patch.
 *
 * Revision 1.23  2003/03/18 00:40:28  robertj
 * Added back the IsMediaPCM() function for backward compatibility.
 *
 * Revision 1.22  2003/03/17 08:02:54  robertj
 * Combined to the separate incoming and outgoing substream classes into
 *   a single class to make it easier to produce codec aware descendents.
 * Added G.729 substream class.
 *
 * Revision 1.21  2002/11/08 03:38:34  craigs
 * Fixed problem with G.723.1 files
 *
 * Revision 1.20  2002/09/18 06:37:13  robertj
 * Added functions to load vxml directly, via file or URL. Old function
 *   intelligently picks which one to use.
 *
 * Revision 1.19  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.18  2002/09/03 04:11:14  craigs
 * More VXML changes
 *
 * Revision 1.17  2002/08/30 05:06:13  craigs
 * Added changes for PVXMLGrammar
 *
 * Revision 1.16  2002/08/28 08:04:31  craigs
 * Reorganised VXMLSession class as per contributed code
 *
 * Revision 1.15  2002/08/28 05:10:27  craigs
 * Added ability to load resources via URI
 * Added cache
 *
 * Revision 1.14  2002/08/27 02:19:13  craigs
 * Added <break> command in prompt blocks
 * Fixed potential deadlock
 *
 * Revision 1.13  2002/08/15 04:11:16  robertj
 * Fixed shutdown problems with closing vxml session, leaks a thread.
 * Fixed potential problems with indirect channel Close() function.
 *
 * Revision 1.12  2002/08/08 01:03:19  craigs
 * Added function to re-enable automatic call clearing on script end
 *
 * Revision 1.11  2002/08/06 07:44:56  craigs
 * Added lots of stuff from OpalVXML
 *
 * Revision 1.10  2002/07/29 15:08:34  craigs
 * Added autodelete option to PlayFile
 *
 * Revision 1.9  2002/07/29 15:03:58  craigs
 * Added access to queue functions
 * Added autodelete option to AddFile
 *
 * Revision 1.8  2002/07/29 14:15:47  craigs
 * Added asynchronous VXML execution
 *
 * Revision 1.7  2002/07/17 08:34:12  craigs
 * Fixed deadlock problems
 *
 * Revision 1.6  2002/07/17 06:08:43  craigs
 * Added additional "sayas" classes
 *
 * Revision 1.5  2002/07/10 13:14:55  craigs
 * Moved some VXML classes from Opal back into PTCLib
 *
 * Revision 1.4  2002/07/05 06:27:26  craigs
 * Removed unused member variables
 * Added OnEmptyAction callback
 *
 * Revision 1.3  2002/07/02 06:23:51  craigs
 * Added recording functions
 *
 * Revision 1.2  2002/06/27 05:39:18  craigs
 * Fixed Linux warning
 *
 * Revision 1.1  2002/06/27 05:28:17  craigs
 * Initial version
 *
 *
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
    virtual PBoolean OnUserInput(const char /*ch*/) { return PTrue; }
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
    PBoolean OnUserInput(const char ch);
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

    PBoolean Get(const PString & prefix,
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
    PVXMLSession(PTextToSpeech * tts = NULL, PBoolean autoDelete = PFalse);
    virtual ~PVXMLSession();

    void SetFinishWhenEmpty(PBoolean v)
    { finishWhenEmpty = v; }

    // new functions
    PTextToSpeech * SetTextToSpeech(PTextToSpeech * _tts, PBoolean autoDelete = PFalse);
    PTextToSpeech * SetTextToSpeech(const PString & ttsName);
    PTextToSpeech * GetTextToSpeech() { return textToSpeech; }

    virtual PBoolean Load(const PString & source);
    virtual PBoolean LoadFile(const PFilePath & file);
    virtual PBoolean LoadURL(const PURL & url);
    virtual PBoolean LoadVXML(const PString & xml);
    virtual PBoolean IsLoaded() const { return loaded; }

    virtual PBoolean Open(const PString & mediaFormat);
    virtual PBoolean Close();

    PBoolean Execute();

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

    PBoolean LoadGrammar(PVXMLGrammar * grammar);

    virtual PBoolean PlayText(const PString & text, PTextToSpeech::TextType type = PTextToSpeech::Default, PINDEX repeat = 1, PINDEX delay = 0);
    PBoolean ConvertTextToFilenameList(const PString & _text, PTextToSpeech::TextType type, PStringArray & list, PBoolean useCacheing);

    virtual PBoolean PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = PFalse);
    virtual PBoolean PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayCommand(const PString & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayResource(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlayTone(const PString & toneSpec, PINDEX repeat = 1, PINDEX delay = 0);

    //virtual PBoolean PlayMedia(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual PBoolean PlaySilence(PINDEX msecs = 0);
    virtual PBoolean PlaySilence(const PTimeInterval & timeout);

    virtual void SetPause(PBoolean pause);
    virtual void GetBeepData(PBYTEArray & data, unsigned ms);

    virtual PBoolean StartRecording(const PFilePath & fn, PBoolean recordDTMFTerm, const PTimeInterval & recordMaxTime, const PTimeInterval & recordFinalSilence);
    virtual PBoolean EndRecording();
    virtual PBoolean IsPlaying() const;
    virtual PBoolean IsRecording() const;

    virtual PBoolean OnUserInput(const PString & str);

    PString GetXMLError() const;

    virtual void OnEndSession()         { }

    virtual PString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);
    virtual PString EvaluateExpr(const PString & oexpr);

    virtual PBoolean RetreiveResource(const PURL & url, PString & contentType, PFilePath & fn, PBoolean useCache = PTrue);

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

    PBoolean TraverseAudio();
    PBoolean TraverseGoto();
    PBoolean TraverseGrammar();
    PBoolean TraverseRecord();

    PBoolean TraverseIf();
    PBoolean TraverseExit();
    PBoolean TraverseVar();
    PBoolean TraverseSubmit();
    PBoolean TraverseMenu();
    PBoolean TraverseChoice(const PString & grammarResult);
    PBoolean TraverseProperty();

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
    PBoolean listening;                 // PTrue if waiting for recognition events
    int timeout;                    // timeout in msecs for the current recognition

    PStringToString sessionVars;
    PStringToString documentVars;

    PMutex userInputMutex;
    std::queue<char> userInputQueue;

    PBoolean recording;
    PFilePath recordFn;
    PBoolean recordDTMFTerm;
    PTimeInterval recordMaxTime;
    PTimeInterval recordFinalSilence;
    PSyncPoint    recordSync;

    PBoolean loaded;
    PBoolean finishWhenEmpty;
    PBoolean allowFinish;
    PURL rootURL;
    PBoolean emptyAction;

    PThread * vxmlThread;
    PBoolean threadRunning;
    PBoolean forceEnd;

    PString mediaFormat;
    PVXMLChannel * vxmlChannel;

    PTextToSpeech * textToSpeech;
    PBoolean autoDeleteTextToSpeech;

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

    virtual PBoolean Open(const PString & _arg) = 0;

    virtual void Record(PVXMLChannel & incomingChannel) = 0;

    virtual void OnStart() { }

    virtual PBoolean OnFrame(PBoolean /*isSilence*/) { return PFalse; }

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
    { repeat = 1; delay = 0; sampleFrequency = 8000; autoDelete = PFalse; delayDone = PFalse; }

    virtual PBoolean Open(PVXMLChannel & /*chan*/, PINDEX _delay, PINDEX _repeat, PBoolean _autoDelete)
    { delay = _delay; repeat = _repeat; autoDelete = _autoDelete; return PTrue; }

    virtual PBoolean Open(PVXMLChannel & chan, const PString & _arg, PINDEX _delay, PINDEX _repeat, PBoolean v)
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

    virtual PBoolean ReadFrame(PVXMLChannel & channel, void * buf, PINDEX len);

    virtual PBoolean Rewind(PChannel *) 
    { return PFalse; }

    friend class PVXMLChannel;

  protected:
    PString arg;
    PINDEX repeat;
    PINDEX delay;
    PString format;
    unsigned sampleFrequency;
    PBoolean autoDelete;
    PBoolean delayDone; // very tacky flag used to indicate when the post-play delay has been done
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableURL : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableURL, PVXMLPlayable);
  public:
    PBoolean Open(PVXMLChannel & chan, const PString & _url, PINDEX _delay, PINDEX _repeat, PBoolean v);
    void Play(PVXMLChannel & outgoingChannel);
  protected:
    PURL url;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableData : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableData, PVXMLPlayable);
  public:
    PBoolean Open(PVXMLChannel & chan, const PString & /*_fn*/, PINDEX _delay, PINDEX _repeat, PBoolean v);
    void SetData(const PBYTEArray & _data);
    void Play(PVXMLChannel & outgoingChannel);
    PBoolean Rewind(PChannel * chan);
  protected:
    PBYTEArray data;
};

//////////////////////////////////////////////////////////////////

#include <ptclib/dtmf.h>

class PVXMLPlayableTone : public PVXMLPlayableData
{
  PCLASSINFO(PVXMLPlayableTone, PVXMLPlayableData);
  public:
    PBoolean Open(PVXMLChannel & chan, const PString & toneSpec, PINDEX _delay, PINDEX _repeat, PBoolean v);
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
    PBoolean Open(PVXMLChannel & chan, const PString & _fn, PINDEX _delay, PINDEX _repeat, PBoolean _autoDelete);
    void Play(PVXMLChannel & outgoingChannel);
    void OnStop();
    virtual PBoolean Rewind(PChannel * chan);
  protected:
    PFilePath fn;
};

//////////////////////////////////////////////////////////////////

class PVXMLPlayableFilenameList : public PVXMLPlayable
{
  PCLASSINFO(PVXMLPlayableFilenameList, PVXMLPlayable);
  public:
    PBoolean Open(PVXMLChannel & chan, const PStringArray & _filenames, PINDEX _delay, PINDEX _repeat, PBoolean _autoDelete);
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
    PBoolean Open(const PString & _arg);
    void Record(PVXMLChannel & incomingChannel);
    PBoolean OnFrame(PBoolean isSilence);

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

    virtual PBoolean Open(PVXMLChannelInterface * _vxml);

    // overrides from PIndirectChannel
    virtual PBoolean IsOpen() const;
    virtual PBoolean Close();
    virtual PBoolean Read(void * buffer, PINDEX amount);
    virtual PBoolean Write(const void * buf, PINDEX len);

    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn, PBoolean recording = PFalse);

    const PString & GetMediaFormat() const { return mediaFormat; }
    PBoolean IsMediaPCM() const { return mediaFormat == "PCM-16"; }
    virtual PString AdjustWavFilename(const PString & fn);

    // Incoming channel functions
    virtual PBoolean WriteFrame(const void * buf, PINDEX len) = 0;
    virtual PBoolean IsSilenceFrame(const void * buf, PINDEX len) const = 0;

    virtual PBoolean QueueRecordable(PVXMLRecordable * newItem);

    PBoolean StartRecording(const PFilePath & fn, unsigned finalSilence = 3000, unsigned maxDuration = 30000);
    PBoolean EndRecording();
    PBoolean IsRecording() const { return recording; }

    // Outgoing channel functions
    virtual PBoolean ReadFrame(void * buffer, PINDEX amount) = 0;
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount) = 0;
    virtual void GetBeepData(PBYTEArray &, unsigned) { }

    virtual PBoolean QueueResource(const PURL & url, PINDEX repeat= 1, PINDEX delay = 0);

    virtual PBoolean QueuePlayable(const PString & type, const PString & str, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = PFalse);
    virtual PBoolean QueuePlayable(PVXMLPlayable * newItem);
    virtual PBoolean QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);

    virtual PBoolean QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, PBoolean autoDelete = PFalse)
    { return QueuePlayable("File", fn, repeat, delay, autoDelete); }

    virtual PBoolean QueueCommand(const PString & cmd, PINDEX repeat = 1, PINDEX delay = 0)
    { return QueuePlayable("Command", cmd, repeat, delay, PTrue); }

    virtual void FlushQueue();
    virtual PBoolean IsPlaying() const   { return (playQueue.GetSize() > 0) || playing ; }

    void SetPause(PBoolean _pause) { paused = _pause; }

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
    PBoolean closed;

    // Incoming audio variables
    PBoolean recording;
    PVXMLRecordable * recordable;
    unsigned finalSilence;
    unsigned silenceRun;

    // Outgoing audio variables
    PBoolean playing;
    PMutex queueMutex;
    PVXMLQueue playQueue;
    PVXMLPlayable * currentPlayItem;

    PBoolean paused;
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
