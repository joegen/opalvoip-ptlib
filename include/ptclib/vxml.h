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
 * More changes from Alexander Kovatch
 *
 * Revision 1.17  2002/08/30 05:06:13  craigs
 * Added changes for PVXMLGrammar from Alexander Kovatch
 *
 * Revision 1.16  2002/08/28 08:04:31  craigs
 * Reorganised VXMLSession class as per code from Alexander Kovatch
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
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>
#include <ptclib/ptts.h>
#include <ptclib/url.h>


class PVXMLSession;
class PVXMLDialog;
class PVXMLSession;


class PVXMLGrammar : public PObject
{
  PCLASSINFO(PVXMLGrammar, PObject);
  public:
    PVXMLGrammar(PXMLElement * field);
    virtual BOOL OnUserInput(const PString & /*str*/) { return TRUE; }
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
    BOOL OnUserInput(const PString & str);
    virtual void Stop();

  protected:
    PINDEX minDigits;
    PINDEX maxDigits;
    PString terminators;
};


//////////////////////////////////////////////////////////////////

class PVXMLCacheItem : public PURL
{
  PCLASSINFO(PVXMLCacheItem, PURL);
  public:
    PVXMLCacheItem(const PURL & url)
      : PURL(url)
    { }

    PFilePath fn;
    PString contentType;
    PTime loadTime;
    BOOL ok;
};


PLIST(PVXMLCache, PVXMLCacheItem);


//////////////////////////////////////////////////////////////////

class PVXMLChannel;

class PVXMLSession : public PIndirectChannel
{
  PCLASSINFO(PVXMLSession, PIndirectChannel);
  public:
    PVXMLSession(PTextToSpeech * tts = NULL, BOOL autoDelete = FALSE);
    ~PVXMLSession();

    // new functions
    void SetTextToSpeech(PTextToSpeech * _tts, BOOL autoDelete = FALSE);

    virtual BOOL Load(const PString & source);
    virtual BOOL LoadFile(const PFilePath & file);
    virtual BOOL LoadURL(const PURL & url);
    virtual BOOL LoadVXML(const PString & xml);
    virtual BOOL IsLoaded() const { return loaded; }

    virtual BOOL Open(BOOL isPCM); // For backward compatibility FALSE=G.723.1
    virtual BOOL Open(
      PVXMLChannel * in,
      PVXMLChannel * out
    );
    virtual BOOL Close();

    PVXMLChannel * GetIncomingChannel() const { return incomingChannel; }
    PVXMLChannel * GetOutgoingChannel() const { return outgoingChannel; }

    BOOL Execute();

    BOOL LoadGrammar(PVXMLGrammar * grammar);

    virtual BOOL PlayText(const PString & text, PTextToSpeech::TextType type = PTextToSpeech::Default, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE);
    virtual BOOL PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlayResource(const PURL & url, PINDEX repeat = 1, PINDEX delay = 0);
    virtual BOOL PlaySilence(PINDEX msecs = 0);

    virtual BOOL StartRecording(const PFilePath & fn);
    virtual BOOL EndRecording();
    virtual BOOL IsPlaying() const;
    virtual BOOL IsRecording() const;

    virtual PWAVFile * CreateWAVFile(const PFilePath & fn, PFile::OpenMode mode, int opts, unsigned fmt);

    virtual void StartRecord(const PFilePath & recordfn, BOOL dtmfTerm, int maxTime, int finalSilence);

    virtual BOOL OnUserInput(const PString & str);

    PString GetXMLError() const;

    virtual BOOL OnEmptyAction()  { return TRUE; }
    virtual void OnEndSession()   { }

    virtual PString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);
    virtual PString PVXMLSession::EvaluateExpr(const PString & oexpr);

    virtual BOOL RetrieveResource(const PURL & url, PBYTEArray & text, PString & contentType);
    virtual BOOL RetrieveResource(const PURL & url, PBYTEArray & text, PString & contentType, PFilePath & fn);

    PDECLARE_NOTIFIER(PThread, PVXMLSession, DialogExecute);

    void AllowClearCall();
    PURL NormaliseResourceName(const PString & src);

    PXMLElement * FindForm(const PString & id);

    BOOL TraverseAudio();
    BOOL TraverseGoto();
    BOOL TraverseGrammar();

    PXMLElement * FindHandler(const PString & event);

    void SayAs(const PString & className, const PString & text);

  protected:
    BOOL ExecuteWithoutLock();

    PMutex sessionMutex;

    PXML xmlFile;

    PVXMLGrammar * activeGrammar;
    BOOL listening;                 // TRUE if waiting for recognition events
    int timeout;                    // timeout in msecs for the current recognition

    PStringToString sessionVars;
    PStringToString documentVars;

    BOOL recording;
    PFilePath recordFn;
    BOOL recordDTMFTerm;
    int recordMaxTime;
    int recordFinalSilence;
    BOOL loaded;
    PURL rootURL;

    PThread * vxmlThread;
    BOOL forceEnd;

    PVXMLChannel * incomingChannel;
    PVXMLChannel * outgoingChannel;

    PTextToSpeech * textToSpeech;
    BOOL autoDeleteTextToSpeech;

    PXMLElement * currentForm;
    PXMLElement * currentField;
    PXMLObject  * currentNode;

    static PMutex cacheMutex;
    static PDirectory cacheDir;
    static PVXMLCache * resourceCache;
    static PINDEX cacheCount;
};


//////////////////////////////////////////////////////////////////

class PVXMLQueueItem : public PObject
{
  PCLASSINFO(PVXMLQueueItem, PObject);
  public:
    PVXMLQueueItem(PINDEX _repeat = 1, PINDEX _delay = 0)
      : repeat(_repeat), delay(_delay)
      { }

    virtual void Play(PVXMLChannel & outgoingChannel) = 0;

    virtual void OnStart() { }
    virtual void OnStop() { }

    PINDEX repeat;
    PINDEX delay;
};


//////////////////////////////////////////////////////////////////

class PVXMLQueueDataItem : public PVXMLQueueItem
{
  PCLASSINFO(PVXMLQueueDataItem, PVXMLQueueItem);
  public:
    PVXMLQueueDataItem(const PBYTEArray & _data, PINDEX repeat = 1, PINDEX delay = 0)
      : PVXMLQueueItem(repeat, delay), data(_data)
    { }

    void Play(PVXMLChannel & outgoingChannel);

  protected:
    PBYTEArray data;
};


class PVXMLQueueFilenameItem : public PVXMLQueueItem
{
  PCLASSINFO(PVXMLQueueFilenameItem, PObject);
  public:
    PVXMLQueueFilenameItem(const PFilePath & _fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL _autoDelete = FALSE)
      : PVXMLQueueItem(repeat, delay), fn(_fn), autoDelete(_autoDelete)
    { }

    void Play(PVXMLChannel & outgoingChannel);

    void OnStop() 
    { if (autoDelete) PFile::Remove(fn); }

  protected:
    PFilePath fn;
    BOOL autoDelete;
};


class PVXMLQueueURLItem : public PVXMLQueueItem
{
  PCLASSINFO(PVXMLQueueURLItem, PObject);
  public:
    PVXMLQueueURLItem(const PURL & _url, PINDEX repeat = 1, PINDEX delay = 0)
      : PVXMLQueueItem(repeat, delay), url(_url)
    { }

    void Play(PVXMLChannel & outgoingChannel);

  protected:
    PURL url;
};


PQUEUE(PVXMLQueue, PVXMLQueueItem);


//////////////////////////////////////////////////////////////////

class PVXMLChannel : public PIndirectChannel
{
  PCLASSINFO(PVXMLChannel, PIndirectChannel);
  public:
    PVXMLChannel(
      PVXMLSession & _vxml,
      BOOL incoming,
      const PString & fmtName,
      PINDEX frameBytes,
      unsigned frameTime,
      unsigned wavFileType,
      const PString & wavFilePrefix
    );
    ~PVXMLChannel();

    // overrides from PIndirectChannel
    virtual BOOL IsOpen() const;
    virtual BOOL Close();
    virtual BOOL Read(void * buffer, PINDEX amount);
    virtual BOOL Write(const void * buf, PINDEX len);

    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn);

    const PString & GetFormatName() const { return formatName; }
    BOOL IsMediaPCM() const { return formatName == "PCM-16"; }
    unsigned GetWavFileType() const { return wavFileType; }
    virtual PString AdjustWavFilename(const PString & fn);

    // Incoming channel functions
    virtual BOOL WriteFrame(const void * buf, PINDEX len) = 0;

    BOOL StartRecording(const PFilePath & fn);
    BOOL EndRecording();
    BOOL IsRecording() const { return wavFile != NULL; }

    // Outgoing channel functions
    virtual BOOL ReadFrame(PINDEX amount) = 0;
    virtual void CreateSilenceFrame(PINDEX amount) = 0;

    virtual BOOL AdjustFrame(void * buffer, PINDEX amount);
    virtual void QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0, BOOL autoDelete = FALSE);
    virtual void QueueResource(const PURL & url, PINDEX repeat= 1, PINDEX delay = 0);
    virtual void QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);

    virtual void QueueItem(PVXMLQueueItem * newItem);
    virtual void FlushQueue();
    virtual BOOL IsPlaying() const   { return (playQueue.GetSize() > 0) || playing ; }

  protected:
    PVXMLSession & vxml;
    BOOL isIncoming;
    PString formatName;
    PINDEX frameBytes;
    unsigned frameTime;
    unsigned wavFileType;
    PString wavFilePrefix;

    PMutex channelMutex;
    PAdaptiveDelay delay;
    BOOL closed;

    // Incoming call variables
    PWAVFile * wavFile;

    // Outgoing call variables
    PMutex queueMutex;
    PVXMLQueue playQueue;
    BOOL playing;

    PBYTEArray frameBuffer;
    PINDEX frameLen, frameOffs;

    int silentCount;
    int totalData;
    PTimer delayTimer;
};


//////////////////////////////////////////////////////////////////

class PVXMLChannelPCM : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelPCM, PVXMLChannel);

  public:
    PVXMLChannelPCM(PVXMLSession & vxml, BOOL incoming);

  protected:
    // overrides from PVXMLChannel
    virtual BOOL WriteFrame(const void * buf, PINDEX len);
    virtual BOOL ReadFrame(PINDEX amount);
    virtual void CreateSilenceFrame(PINDEX amount);
};


class PVXMLChannelG7231 : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelG7231, PVXMLChannel);
  public:
    PVXMLChannelG7231(PVXMLSession & vxml, BOOL incoming);

    // overrides from PVXMLChannel
    virtual BOOL WriteFrame(const void * buf, PINDEX len);
    virtual BOOL ReadFrame(PINDEX amount);
    virtual void CreateSilenceFrame(PINDEX amount);
};


class PVXMLChannelG729 : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelG729, PVXMLChannel);
  public:
    PVXMLChannelG729(PVXMLSession & vxml, BOOL incoming);

    // overrides from PVXMLChannel
    virtual BOOL WriteFrame(const void * buf, PINDEX len);
    virtual BOOL ReadFrame(PINDEX amount);
    virtual void CreateSilenceFrame(PINDEX amount);
};


#endif


// End of file ////////////////////////////////////////////////////////////////
