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

#include <queue>

#include <ptlib/pipechan.h>

#include <ptclib/pxml.h>
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>
#include <ptclib/ptts.h>
#include <ptclib/url.h>

class PVXMLSession;
class PVXMLDialog;
class PVXMLSession;

class PVXMLTransferOptions;
class PVXMLTransferResult;

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
      FILLED,       // got something that matched the grammar
      NOINPUT,      // timeout or still waiting to match
      NOMATCH,      // recognized something but didn't match the grammar
      HELP };       // help keyword

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
    ~PVXMLSession();

    void SetFinishWhenEmpty(BOOL v)
    { finishWhenEmpty = v; }

    // new functions
    void SetTextToSpeech(PTextToSpeech * _tts, BOOL autoDelete = FALSE);
    void SetTextToSpeech(const PString & ttsName);

    virtual BOOL Load(const PString & source);
    virtual BOOL LoadFile(const PFilePath & file);
    virtual BOOL LoadURL(const PURL & url);
    virtual BOOL LoadVXML(const PString & xml);
    virtual BOOL IsLoaded() const { return loaded; }

    virtual BOOL Open(BOOL isPCM); // For backward compatibility FALSE=G.723.1
    virtual BOOL Open(const PString & mediaFormat);
    virtual BOOL Open(
      PVXMLChannel * in,
      PVXMLChannel * out
    );
    virtual BOOL Close();

    BOOL Execute();

    PVXMLChannel * GetIncomingChannel() const { return incomingChannel; }
    PVXMLChannel * GetOutgoingChannel() const { return outgoingChannel; }

    BOOL LoadGrammar(PVXMLGrammar * grammar);

    virtual BOOL PlayText(const PString & text, PTextToSpeech::TextType type = PTextToSpeech::Default, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE);
    virtual BOOL PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayCommand(const PString & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayResource(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);

    //virtual BOOL PlayMedia(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlaySilence(PINDEX msecs = 0);
    virtual BOOL PlaySilence(const PTimeInterval & timeout);

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
    virtual PString PVXMLSession::EvaluateExpr(const PString & oexpr);

    virtual BOOL RetreiveResource(const PURL & url, PString & contentType, PFilePath & fn);

    PDECLARE_NOTIFIER(PThread, PVXMLSession, VXMLExecute);

    virtual BOOL DoTransfer(const PVXMLTransferOptions &) { return TRUE; }
    virtual void OnTransfer(const PVXMLTransferResult &);

    void SetCallingToken( PString& token ) { callingCallToken = token; }

    PXMLElement * FindHandler(const PString & event);

    // overrides from VXMLChannelInterface
    PWAVFile * CreateWAVFile(const PFilePath & fn, PFile::OpenMode mode, int opts, unsigned fmt);
    void OnEndRecording(const PString & channelName);
    void RecordEnd();
    void Trigger();

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

    void SayAs(const PString & className, const PString & text);
    static PTimeInterval StringToTime(const PString & str);

    PURL NormaliseResourceName(const PString & src);

    PXMLElement * FindForm(const PString & id);

    virtual BOOL TraverseTransfer();

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
    BOOL finishWhenEmpty;
    BOOL allowFinish;
    PURL rootURL;
    BOOL emptyAction;

    PThread * vxmlThread;
    BOOL threadRunning;
    BOOL forceEnd;

    PVXMLChannel * incomingChannel;
    PVXMLChannel * outgoingChannel;

    PTextToSpeech * textToSpeech;
    BOOL autoDeleteTextToSpeech;

    PXMLElement * currentForm;
    PXMLElement * currentField;
    PXMLObject  * currentNode;

  private:
    void      ExecuteDialog();

    PString       callingCallToken;
    PSyncPoint    transferSync;
    PSyncPoint    answerSync;
    PString       grammarResult;
    PString       eventName;
    PINDEX        defaultDTMF;
};


//////////////////////////////////////////////////////////////////

class PVXMLPlayable : public PObject
{
  PCLASSINFO(PVXMLPlayable, PObject);
  public:
    PVXMLPlayable()
    { repeat = 1; delay = 0; sampleFrequency = 8000; autoDelete = FALSE; }

    virtual BOOL Open(PINDEX _delay, PINDEX _repeat, BOOL _autoDelete)
    { delay = _delay; repeat = _repeat; autoDelete = _autoDelete; return TRUE; }

    virtual BOOL Open(const PString & _arg, PINDEX _delay, PINDEX _repeat, BOOL v)
    { arg = _arg; return Open(_delay, _repeat, v); }

    virtual void Play(PVXMLChannel & outgoingChannel) = 0;

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

  protected:
    PString arg;
    PINDEX repeat;
    PINDEX delay;
    PString format;
    unsigned sampleFrequency;
    BOOL autoDelete;
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

    virtual BOOL Open(PVXMLChannelInterface * _vxml, BOOL incoming);

    // overrides from PIndirectChannel
    virtual BOOL IsOpen() const;
    virtual BOOL Close();
    virtual BOOL Read(void * buffer, PINDEX amount);
    virtual BOOL Write(const void * buf, PINDEX len);

    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn);
    PWAVFile * GetWAVFile() const
    { return wavFile; }

    const PString & GetFormatName() const { return formatName; }
    BOOL IsMediaPCM() const { return formatName == "PCM-16"; }
    unsigned GetWavFileType() const { return wavFileType; }
    virtual PString AdjustWavFilename(const PString & fn);

    // Incoming channel functions
    virtual BOOL WriteFrame(const void * buf, PINDEX len) = 0;
    virtual BOOL IsSilenceFrame(const void * buf, PINDEX len) const = 0;

    BOOL StartRecording(const PFilePath & fn, unsigned finalSilence = 2000);
    BOOL EndRecording();
    BOOL IsRecording() const { return recording; }

    // Outgoing channel functions
    virtual BOOL ReadFrame(void * buffer, PINDEX amount) = 0;
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount) = 0;
    virtual void GetBeepData(PBYTEArray &, unsigned) { }

    virtual void QueueResource(const PURL & url, PINDEX repeat= 1, PINDEX delay = 0);

    virtual void QueuePlayable(const PString & type, const PString & str, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = TRUE);
    virtual void QueuePlayable(PVXMLPlayable * newItem);
    virtual void QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);

    virtual void QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE)
    { return QueuePlayable("File", fn, repeat, delay, autoDelete); }

    virtual void QueueCommand(const PString & cmd, PINDEX repeat = 1, PINDEX delay = 0)
    { return QueuePlayable("Command", cmd, repeat, delay, TRUE); }

    virtual void FlushQueue();
    virtual BOOL IsPlaying() const   { return (playQueue.GetSize() > 0) || playing ; }

    void SetPause(BOOL _pause) { paused = _pause; }

    void SetName(const PString & name) { channelName = name; }

  protected:
    PVXMLChannelInterface * vxmlInterface;
    BOOL isIncoming;

    unsigned sampleFrequency;
    PString formatName;
    unsigned wavFileType;
    PString wavFilePrefix;

    PMutex channelMutex;
    BOOL closed;

    // Incoming audio variables
    BOOL recording;
    PWAVFile * wavFile;
    unsigned finalSilence;
    unsigned silenceRun;

    // Outgoing audio variables
    BOOL playing;
    PMutex queueMutex;
    PVXMLQueue playQueue;

    BOOL paused;
    int silentCount;
    int totalData;
    PTimer delayTimer;

    // "channelname" (which is the name of the <record> tag) so
    // results can be saved in vxml session variable
    PString channelName;
};


//////////////////////////////////////////////////////////////////

class PVXMLTransferOptions : public PObject
{
  PCLASSINFO(PVXMLTransferOptions, PObject);
  public:
    PVXMLTransferOptions() { }

    void SetCallingToken(const PString & calling) { callingToken = calling; }
    PString GetCallingToken() const               { return callingToken; }
    
    void SetCalledToken(const PString & called)   { calledToken = called; }
    PString GetCalledToken( ) const               { return calledToken; }

    void SetSourceDNR(const PString & src)        { source = src; }
    PString GetSourceDNR() const                  { return source; }

    void SetDestinationDNR(const PString & dest ) { destination = dest; }
    PString GetDestinationDNR() const             { return destination; }

    void SetTimeout(unsigned int time)            { timeout = time; }
    unsigned int GetTimeout() const               { return timeout; }

    void SetBridge(BOOL brdg)                     { bridge = brdg; }
    BOOL GetBridge() const                        { return bridge; }

  private:
    PString callingToken;
    PString calledToken;
    PString destination;
    PString source;
    unsigned int timeout;
    BOOL bridge;
};

class PVXMLTransferResult : public PString
{
  PCLASSINFO(PVXMLTransferResult, PString);
  public:
    PVXMLTransferResult()
    { }

    PVXMLTransferResult(char * cstr) 
      : PString( cstr ) 
    { }

    PVXMLTransferResult(const PString & str )
      : PString(str)
    {}

    void SetName(const PString & n) 
    { name = n; }

    PString GetName() const         
    { return name; } 

  private:
    PString name;
};

#endif


// End of file ////////////////////////////////////////////////////////////////
