/*
 * vxml.cxx
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
 * $Log: vxml.cxx,v $
 * Revision 1.39  2004/05/02 05:14:43  rjongbloed
 * Fixed possible deadlock in shutdown of VXML channel/session.
 *
 * Revision 1.38  2004/04/24 06:27:56  rjongbloed
 * Fixed GCC 3.4.0 warnings about PAssertNULL and improved recoverability on
 *   NULL pointer usage in various bits of code.
 *
 * Revision 1.37  2004/03/23 04:48:42  csoutheren
 * Improved ability to start VXML scripts as needed
 *
 * Revision 1.36  2003/11/12 20:38:16  csoutheren
 * Fixed problem with incorrect sense of ContentLength header detection thanks to Andreas Sikkema
 *
 * Revision 1.35  2003/05/14 01:12:53  rjongbloed
 * Fixed test for SID frames in record silence detection on G.723.1A
 *
 * Revision 1.34  2003/04/23 11:54:53  craigs
 * Added ability to record audio
 *
 * Revision 1.33  2003/04/10 04:19:43  robertj
 * Fixed incorrect timing on G.723.1 (framed codec)
 * Fixed not using correct codec file suffix for non PCM/G.723.1 codecs.
 *
 * Revision 1.32  2003/04/08 05:09:14  craigs
 * Added ability to use commands as an audio source
 *
 * Revision 1.31  2003/03/17 08:03:07  robertj
 * Combined to the separate incoming and outgoing substream classes into
 *   a single class to make it easier to produce codec aware descendents.
 * Added G.729 substream class.
 *
 * Revision 1.30  2002/12/03 22:39:14  robertj
 * Removed get document that just returns a content length as the chunked
 *   transfer encoding makes this very dangerous.
 *
 * Revision 1.29  2002/11/19 10:36:30  robertj
 * Added functions to set anf get "file:" URL. as PFilePath and do the right
 *   things with platform dependent directory components.
 *
 * Revision 1.28  2002/11/08 03:39:27  craigs
 * Fixed problem with G.723.1 files
 *
 * Revision 1.27  2002/09/24 13:47:41  robertj
 * Added support for more vxml commands, thanks Alexander Kovatch
 *
 * Revision 1.26  2002/09/18 06:37:40  robertj
 * Added functions to load vxml directly, via file or URL. Old function
 *   intelligently picks which one to use.
 *
 * Revision 1.25  2002/09/03 04:38:14  craigs
 * Added VXML 2.0 time attribute to <break>
 *
 * Revision 1.24  2002/09/03 04:11:37  craigs
 * More changes from Alexander Kovatch
 *
 * Revision 1.23  2002/08/30 07:33:16  craigs
 * Added extra initialisations
 *
 * Revision 1.22  2002/08/30 05:05:54  craigs
 * Added changes for PVXMLGrammar from Alexander Kovatch
 *
 * Revision 1.21  2002/08/29 00:16:12  craigs
 * Fixed typo, thanks to Peter Robinson
 *
 * Revision 1.20  2002/08/28 08:05:16  craigs
 * Reorganised VXMLSession class as per code from Alexander Kovatch
 *
 * Revision 1.19  2002/08/28 05:10:57  craigs
 * Added ability to load resources via URI
 * Added cache
 *
 * Revision 1.18  2002/08/27 02:46:56  craigs
 * Removed need for application to call AllowClearCall
 *
 * Revision 1.17  2002/08/27 02:20:09  craigs
 * Added <break> command in prompt blocks
 * Fixed potential deadlock
 * Added <prompt> command in top level fields, thanks to Alexander Kovatch
 *
 * Revision 1.16  2002/08/15 04:11:16  robertj
 * Fixed shutdown problems with closing vxml session, leaks a thread.
 * Fixed potential problems with indirect channel Close() function.
 *
 * Revision 1.15  2002/08/15 02:13:10  craigs
 * Fixed problem with handle leak (maybe) and change tts files back to autodelete
 *
 * Revision 1.14  2002/08/14 15:18:07  craigs
 * Improved random filename generation
 *
 * Revision 1.13  2002/08/08 01:03:06  craigs
 * Added function to re-enable automatic call clearing on script end
 *
 * Revision 1.12  2002/08/07 13:38:14  craigs
 * Fixed bug in calculating lengths of G.723.1 packets
 *
 * Revision 1.11  2002/08/06 07:45:28  craigs
 * Added lots of stuff from OpalVXML
 *
 * Revision 1.10  2002/07/29 15:08:50  craigs
 * Added autodelete option to PlayFile
 *
 * Revision 1.9  2002/07/29 15:03:36  craigs
 * Added access to queue functions
 * Added autodelete option to AddFile
 *
 * Revision 1.8  2002/07/29 14:16:05  craigs
 * Added asynchronous VXML execution
 *
 * Revision 1.7  2002/07/17 08:34:25  craigs
 * Fixed deadlock problems
 *
 * Revision 1.6  2002/07/17 06:08:23  craigs
 * Added additional "sayas" classes
 *
 * Revision 1.5  2002/07/10 13:15:20  craigs
 * Moved some VXML classes from Opal back into PTCLib
 * Fixed various race conditions
 *
 * Revision 1.4  2002/07/05 06:28:07  craigs
 * Added OnEmptyAction callback
 *
 * Revision 1.3  2002/07/02 06:24:53  craigs
 * Added recording functions
 *
 * Revision 1.2  2002/06/28 01:30:29  robertj
 * Fixed ability to compile if do not have expat library.
 *
 * Revision 1.1  2002/06/27 05:27:49  craigs
 * Initial version
 *
 *
 */

#ifdef __GNUC__
#pragma implementation "vxml.h"
#endif

#include <ptlib.h>

#if P_EXPAT

#include <ptclib/vxml.h>
#include <ptclib/memfile.h>
#include <ptclib/random.h>
#include <ptclib/http.h>


#define SMALL_BREAK_MSECS   1000
#define MEDIUM_BREAK_MSECS  2500
#define LARGE_BREAK_MSECS   5000

// LATER: Lookup what this value should be
#define DEFAULT_TIMEOUT     10000

#define CACHE_BUFFER_SIZE   1024


PMutex     PVXMLSession::cacheMutex;
PDirectory PVXMLSession::cacheDir;
PVXMLCache * PVXMLSession::resourceCache = NULL;
PINDEX       PVXMLSession::cacheCount = 0;


//////////////////////////////////////////////////////////

PVXMLSession::PVXMLSession(PTextToSpeech * _tts, BOOL autoDelete)
{
  //activeGrammar   = NULL;
  recording       = FALSE;
  vxmlThread      = NULL;
  incomingChannel = NULL;
  outgoingChannel = NULL;
  loaded          = FALSE;
  forceEnd        = FALSE;
  textToSpeech    = NULL;
  listening       = FALSE;
  activeGrammar   = NULL;
  currentNode     = NULL;
  emptyAction     = TRUE;

  SetTextToSpeech(_tts, autoDelete);

  PWaitAndSignal m(cacheMutex);
  cacheCount++;
  if (resourceCache == NULL) {
    resourceCache = new PVXMLCache;
    cacheDir = PDirectory() + "cache";

    // load the cache information, if already present
    PFilePath cacheInfo = cacheDir + "cache.txt";
    if (PFile::Exists(cacheInfo)) {
      PTextFile cacheFile;
      if (cacheFile.Open(cacheInfo, PFile::ReadOnly)) {
        PString line;
        while (cacheFile.ReadLine(line)) {
          PStringArray info = line.Tokenise("|", TRUE);
          if (info.GetSize() > 3) {
            PVXMLCacheItem * item = new PVXMLCacheItem(info[0]);
            item->fn          = cacheDir + info[1];
            item->contentType = info[2];
            item->loadTime    = PTime();
            item->ok          = info[3] *= "y";
            resourceCache->Append(item);
          }
        }
      }
    }
  }
}

PVXMLSession::~PVXMLSession()
{
  Close();

  if ((textToSpeech != NULL) && autoDeleteTextToSpeech) {
    delete textToSpeech;
  }

  PWaitAndSignal m(cacheMutex);
  cacheCount--;

  // write out the cache information
  if (cacheCount == 0) {
    PFilePath cacheInfo = cacheDir + "cache.txt";
    PTextFile cacheFile;
    if (cacheFile.Open(cacheInfo, PFile::WriteOnly)) {
      PINDEX i;
      for (i = 0; i < resourceCache->GetSize(); i++) {
        PVXMLCacheItem & item = (*resourceCache)[i];
        cacheFile << item.AsString() << "|" 
                  << item.fn.GetFileName() << "|"
                  << item.contentType << "|"
                  << (item.ok ? "Y" : "N") 
                  << endl;
      }
    }
    delete resourceCache;
    resourceCache = NULL;
  }
}

void PVXMLSession::SetTextToSpeech(PTextToSpeech * _tts, BOOL autoDelete)
{
  PWaitAndSignal m(sessionMutex);

  if (autoDeleteTextToSpeech && (textToSpeech != NULL))
    delete textToSpeech;

  autoDeleteTextToSpeech = autoDelete;
  textToSpeech = _tts;
}

BOOL PVXMLSession::Load(const PString & source)
{
  // Lets try and guess what was passed, if file exists then is file
  PFilePath file = source;
  if (PFile::Exists(file))
    return LoadFile(file);

  // see if looks like URL
  PINDEX pos = source.Find(':');
  if (pos != P_MAX_INDEX) {
    PString scheme = source.Left(pos);
    if ((scheme *= "http") || (scheme *= "https") || (scheme *= "file"))
      return LoadURL(source);
  }

  // See if is actual VXML
  if (PCaselessString(source).Find("<vxml") != P_MAX_INDEX)
    return LoadVXML(source);

  return FALSE;
}


BOOL PVXMLSession::LoadFile(const PFilePath & filename)
{
  // create a file URL from the filename
  return LoadURL(filename);
}


BOOL PVXMLSession::LoadURL(const PURL & url)
{
  // retreive the document (may be a HTTP get)
  PBYTEArray data;
  PString contentType;
  if (!RetrieveResource(url, data, contentType)) {
    PTRACE(1, "PVXML\tCannot load document " << url);
    return FALSE;
  }

  if (!LoadVXML(PString((const char *)(const BYTE *)data, data.GetSize()))) {
    PTRACE(1, "PVXML\tCannot load VXML in " << url);
    return FALSE;
  }

  rootURL = url;
  return TRUE;
}

BOOL PVXMLSession::LoadVXML(const PString & xmlText)
{
  PWaitAndSignal m(sessionMutex);

  loaded = FALSE;
  rootURL = PString::Empty();

  // parse the XML
  xmlFile.RemoveAll();
  if (!xmlFile.Load(xmlText)) {
    PTRACE(1, "PVXML\tCannot parse root document: " << GetXMLError());
    return FALSE;
  }  

  PXMLElement * root = xmlFile.GetRootElement();
  if (root == NULL)
    return FALSE;

  // find the first form
  if ((currentForm = FindForm("")) == NULL)
    return FALSE;

  // start processing with this <form> element
  currentNode = currentForm;

  loaded = TRUE;
  return TRUE;
}

PURL PVXMLSession::NormaliseResourceName(const PString & src)
{
  // if resource name has a scheme, then use as is
  PINDEX pos = src.Find(':');
  if ((pos != P_MAX_INDEX) && (pos < 5))
    return src;

  if (rootURL.IsEmpty())
    return "file:" + src;

  // else use scheme and path from root document
  PURL url = rootURL;
  PStringArray path = url.GetPath();
  PString pathStr;
  if (path.GetSize() > 0) {
    pathStr += path[0];
    PINDEX i;
    for (i = 1; i < path.GetSize()-1; i++)
      pathStr += "/" + path[i];
    pathStr += "/" + src;
    url.SetPathStr(pathStr);
  }

  return url;
}


BOOL PVXMLSession::RetrieveResource(const PURL & url, PBYTEArray & text, PString & contentType, PFilePath & fn)
{
  BOOL loadFile = FALSE;
  text.SetSize(0);

  // do a HTTP get when appropriate
  if ((url.GetScheme() *= "http") || (url.GetScheme() *= "https")) {

    PWaitAndSignal m(cacheMutex);

    // see if the URL is in the cache
    PINDEX index = resourceCache->GetValuesIndex(url);
    if (index != P_MAX_INDEX) {

      // if too long since last examined, then expire the cache
      PTimeInterval interval = PTime() - (*resourceCache)[index].loadTime;
      if (interval.GetMilliSeconds() > 1000*60)
        resourceCache->RemoveAt(index);
      else {
        PVXMLCacheItem & item = (*resourceCache)[index];

        // if the cache indicates the resource was invalid
        if (!item.ok)
          return FALSE;

        // check the content type, maybe
        if (!contentType.IsEmpty() && (contentType != item.contentType))
          return FALSE;

        // set the file load information
        fn          = item.fn;
        contentType = item.contentType;
        loadFile    = TRUE;
      }
    } 

    // resource was not in the cache, so add it
    if (!loadFile) {
      PHTTPClient client;
      PINDEX contentLength;
      PMIMEInfo outMIME, replyMIME;

      // create a cache item indicating a failed load
      PVXMLCacheItem * item = new PVXMLCacheItem(url);
      item->ok = FALSE;
      resourceCache->Append(item);

      // get the resource header information
      if (!client.GetDocument(url, outMIME, replyMIME)) {
        PTRACE(2, "PVXML\tCannot load resource " << url);
        return FALSE;
      }

      // get the length of the data
      if (replyMIME.Contains(PHTTPClient::ContentLengthTag))
        contentLength = (PINDEX)replyMIME[PHTTPClient::ContentLengthTag].AsUnsigned();
      else
        contentLength = P_MAX_INDEX;

      // create the cache directory, if not already in existence
      if (!cacheDir.Exists())
        cacheDir.Create();

      // create a filename for the cache item
      PRandom r;
      for (;;) {
        fn = cacheDir + psprintf("url_%i.wav", r.Generate() % 1000000);
        if (!PFile::Exists(fn))
          break;
      }

      // open the cache file
      PFile cacheFile;
      if (!cacheFile.Open(fn, PFile::WriteOnly)) {
        PTRACE(2, "PVXML\tCannot create temporary cache file " << fn);
        return FALSE;
      }

      // download the resource into the cache file
      PINDEX offs = 0;
      for (;;) {
        PINDEX len;
        if (contentLength == P_MAX_INDEX)
          len = CACHE_BUFFER_SIZE;
        else if (offs == contentLength)
          break;
        else
          len = PMIN(contentLength = offs, CACHE_BUFFER_SIZE);

        if (!client.Read(offs + text.GetPointer(offs + len), len))
          break;

        len = client.GetLastReadCount();
        if (!cacheFile.Write(offs + (const BYTE *)text, len))
          break;

        offs += len;
      }

      // set the cache information
      item->ok          = TRUE;
      item->fn          = fn;
      item->loadTime    = PTime();
      item->contentType = replyMIME(PHTTPClient::ContentTypeTag);

      // check the content type, maybe
      if (!contentType.IsEmpty() && (contentType != item->contentType))
        return FALSE;

      // data is loaded
      return TRUE;
    }
  }

  // files on the local file system get loaded locally
  else if (url.GetScheme() *= "file") {

    fn = url.AsFilePath();
    loadFile = TRUE;
  }

  // unknown schemes give an error
  else 
    return FALSE;

  return loadFile;
}

BOOL PVXMLSession::RetrieveResource(const PURL & url, PBYTEArray & text, PString & contentType)
{
  PFilePath fn;

  // get name of file
  if (!RetrieveResource(url, text, contentType, fn))
    return FALSE;

  // if data was already loaded, do nothing
  if (text.GetSize() != 0)
    return TRUE;


  // load the data
  PFile file;
  if (!file.Open(fn, PFile::ReadOnly)) 
    return FALSE;

  // read the data
  off_t len = file.GetLength();
  if (!file.Read(text.GetPointer(len), len))
    return FALSE;

  // set content type from extension, if required
  if (!contentType.IsEmpty()) {
    if (fn.GetType() *= ".vxml")
      contentType = "text/vxml";
    else if (fn.GetType() *= ".wav")
      contentType = "audio/x-wav";
  }

  return TRUE;
}

PXMLElement * PVXMLSession::FindForm(const PString & id)
{
  // NOTE: should have some flag to know if it is loaded
  PXMLElement * root = xmlFile.GetRootElement();
  if (root == NULL)
    return NULL;
  
  // Only handle search of top level nodes for <form> element
  PINDEX i;
  for (i = 0; i < root->GetSize(); i++) {
    PXMLObject * xmlObject = root->GetElement(i); 
    if (xmlObject->IsElement()) {
      PXMLElement * xmlElement = (PXMLElement*)xmlObject;
      if (
          (xmlElement->GetName() *= "form") && 
          (id.IsEmpty() || (xmlElement->GetAttribute("id") *= id))
         )
        return xmlElement;
    }
  }
  return NULL;
}


BOOL PVXMLSession::Open(BOOL isPCM)
{
  if (isPCM)
    return Open(new PVXMLChannelPCM(*this, TRUE), new PVXMLChannelPCM(*this, FALSE));
  else
    return Open(new PVXMLChannelG7231(*this, TRUE), new PVXMLChannelG7231(*this, FALSE));
}


BOOL PVXMLSession::Open(PVXMLChannel * in, PVXMLChannel * out)
{
  if (!PIndirectChannel::Open(out, in))
    return FALSE;

  PWaitAndSignal m(sessionMutex);
  outgoingChannel = out;
  incomingChannel = in;
  return TRUE;
}


BOOL PVXMLSession::Close()
{
  sessionMutex.Wait();

  if (vxmlThread != NULL) {
    vxmlThread->WaitForTermination();
    delete vxmlThread;
    vxmlThread = NULL;
  }

  outgoingChannel = NULL;
  incomingChannel = NULL;

  sessionMutex.Signal();

  return PIndirectChannel::Close();
}


BOOL PVXMLSession::Execute()
{
  PWaitAndSignal m(sessionMutex);

  return ExecuteWithoutLock();
}

BOOL PVXMLSession::ExecuteWithoutLock()
{
  // check to see if a vxml thread has stopped since last we looked
  if ((vxmlThread != NULL) && (vxmlThread->IsTerminated())) {
    vxmlThread->WaitForTermination();
    delete vxmlThread;
    vxmlThread = NULL;
  }

  // check to see if we are ending a call
  if (forceEnd) {
    OnEndSession();
    return FALSE;
  }

  // if:
  //    no script has been loaded or
  //    there is already a thread running or
  //    a grammar defined or
  //    recording is in progress
  //    no outgoing channel
  // then just return silence
  //
  if (!loaded || (vxmlThread != NULL) || recording || (outgoingChannel == NULL))
    return TRUE;

  // throw a thread to execute the VXML, because this can take some time
  vxmlThread = PThread::Create(PCREATE_NOTIFIER(DialogExecute), 0, PThread::NoAutoDeleteThread);
  return TRUE;
}

void PVXMLSession::DialogExecute(PThread &, INT)
{
  // --- Process any recognition ---

  // If we have an active grammar then check to see if there has been some recognition
  if (activeGrammar) {
    BOOL processGrammar(FALSE);

    // Stop if we've matched a grammar or have a failed recognition
    if (activeGrammar->GetState() == PVXMLGrammar::FILLED || activeGrammar->GetState() == PVXMLGrammar::NOMATCH)
      processGrammar = TRUE;
    // Stop the grammar if we've timed out
    else if (listening && !IsPlaying())   {
      activeGrammar->Stop();
      processGrammar = TRUE;
    }

    // Let the loop run again if we're still waiting to time out and haven't resolved the grammar one way or the other
    if (!processGrammar && listening)
      return;

    if (processGrammar) {
      PVXMLGrammar::GrammarState state = activeGrammar->GetState();
      PString value = activeGrammar->GetValue();
      LoadGrammar(NULL);
      listening = FALSE;

      // Stop any playback
      if (outgoingChannel != NULL) {
        outgoingChannel->FlushQueue();
      }

      // Figure out what happened
      PString eventname;
      switch (state)
      {
        case PVXMLGrammar::FILLED:
          eventname = "filled";
          break;
        case PVXMLGrammar::NOINPUT:
          eventname = "noinput";
          break;
        case PVXMLGrammar::NOMATCH:
          eventname = "nomatch";
          break;
        default:
          ; //ERROR - unexpected grammar state
      }

      // Find the handler and move there
      PXMLElement * handler = FindHandler(eventname);
      if (handler != NULL)
        currentNode = handler;
      // otherwise move on to the next node (already pointed there)
    }
  } // (end) there is an active grammar


  // --- Process the current element ---

  // Process the current node
  if (currentNode != NULL) {

    if (!currentNode->IsElement())
      TraverseAudio();
    else {
      PXMLElement * element = (PXMLElement*)currentNode;
      PCaselessString nodeType = element->GetName();
      PTRACE(3, "PVXML\t**** Processing VoiceXML element: <" << nodeType << "> ***");

      if (nodeType *= "audio") {
        TraverseAudio();
      }

      else if (nodeType *= "block") {
        // check 'cond' attribute to see if this element's children are to be skipped
        // go on and process the children
      }

      else if (nodeType *= "break") {
        TraverseAudio();
      }

      else if (nodeType *= "disconnect") {
        currentNode = NULL;
      }

      else if (nodeType *= "field") {
        currentField = (PXMLElement*)currentNode;
        timeout = DEFAULT_TIMEOUT;
        TraverseGrammar();  // this will set activeGrammar
      }

      else if (nodeType *= "form") {
        // this is now the current element - go on
        currentForm = element;
        currentField = NULL;  // no active field in a new form
      }

      else if (nodeType *= "goto") {
        TraverseGoto();
      }

      else if (nodeType *= "grammar") {
        TraverseGrammar();  // this will set activeGrammar
      }

      else if (nodeType *= "record") {
		    TraverseRecord();		
      }

      else if (nodeType *= "prompt") {
        // LATER:
        // check 'cond' attribute to see if the children of this node should be processed
        // check 'count' attribute to see if this node should be processed
        // flush all prompts if 'bargein' attribute is set to false

        // Update timeout of current recognition (if 'timeout' attribute is set)
        if (element->HasAttribute("timeout")) {
          PTimeInterval timeout = StringToTime(element->GetAttribute("timeout"));
        }

        // go on to process the children
      }

      else if (nodeType *= "say-as") {
      }

      else if (nodeType *= "value") {
        TraverseAudio();
      }

      else if (nodeType *= "var") {
      }
    } // (end) else the current node is a PXMLElement
  } // (end) there is a current node


  // --- Move along the XML tree ---

  // Wait for the buffer to complete before quiting
  if (currentNode == NULL) {
    if (IsPlaying())
      return;
  }

  // if the current node has children, then process the first child
  else if (currentNode->IsElement() && (((PXMLElement *)currentNode)->GetElement(0) != NULL))
    currentNode = ((PXMLElement *)currentNode)->GetElement(0);

  // else process the next sibling
  else {
    // Keep moving up the parents until we find a next sibling
    while ((currentNode != NULL) && currentNode->GetNextObject() == NULL) {
      currentNode = currentNode->GetParent();
       // if we are on the backwards traversal through a <field> then wait
      // for a grammar recognition and throw events if necessary
      if (currentNode != NULL && (currentNode->IsElement() == TRUE) && (((PXMLElement*)currentNode)->GetName() *= "field")) {
        listening = TRUE;
        PlaySilence(timeout);
      }
    }

    if (currentNode != NULL)
      currentNode = currentNode->GetNextObject();
  }

  // Determine if we should quit
  if ((currentNode == NULL) && (activeGrammar == NULL) && !IsPlaying() && !IsRecording()) {
    if (OnEmptyAction())
      forceEnd = TRUE;
  }
}

BOOL PVXMLSession::OnUserInput(const PString & str)
{
  PWaitAndSignal m(sessionMutex);

  // record
  if (recording) {
    if (recordDTMFTerm)
      RecordEnd();
    return TRUE;
  } 

  // playback
    else if (activeGrammar != NULL && activeGrammar->OnUserInput(str)) {
    ExecuteWithoutLock();
    return TRUE;
  }

  return FALSE;
}

BOOL PVXMLSession::TraverseRecord()
{
  if (currentNode->IsElement()) {
    
    PString strName;
    PXMLElement * element = (PXMLElement *)currentNode;
    
    // Get the name (name)
    if (element->HasAttribute("name"))
      strName = element->GetAttribute("name");
    else if (element->HasAttribute("id"))
      strName = element->GetAttribute("id");
    
    // Get the destination filename (dest)
    PString strDest("c:\\temp.wav");
    if (element->HasAttribute("dest")) 
      strDest = element->GetAttribute("dest");
    
    // see if we need a beep
    if (element->GetAttribute("beep").ToLower() *= "true") {
      PBYTEArray beepData;
      GetBeepData(beepData, 1000);
      if (beepData.GetSize() != 0)
        PlayData(beepData);
    }
    
    // For some reason, if the file is there the create 
    // seems to fail. 
    PFile::Remove(strDest);
    PFilePath file(strDest);
    
    // Get max record time (maxtime)
    PTimeInterval maxTime = PMaxTimeInterval;
    if (element->HasAttribute("maxtime")) 
      maxTime = StringToTime(element->GetAttribute("maxtime"));
    
    // Get terminating silence duration (finalsilence)
    PTimeInterval termTime(3000);
    if (element->HasAttribute("finalsilence")) 
      termTime = StringToTime(element->GetAttribute("finalsilence"));
    
    // Get dtmf term (dtmfterm)
    BOOL dtmfTerm = TRUE;
    if (element->HasAttribute("dtmfterm"))
      dtmfTerm = !(element->GetAttribute("dtmfterm").ToLower() *= "false");
    
    // create a semaphore, and then wait for the recording to terminate
    StartRecording(file, dtmfTerm, maxTime, termTime);
    recordSync.Wait(maxTime);
    
    // when this returns, we are done
    EndRecording();
  }
  
  return TRUE;
}

PString PVXMLSession::GetXMLError() const
{
  return psprintf("(%i:%i) ", xmlFile.GetErrorLine(), xmlFile.GetErrorColumn()) + xmlFile.GetErrorString();
}

PString PVXMLSession::EvaluateExpr(const PString & oexpr)
{
  PString expr = oexpr.Trim();

  // see if all digits
  PINDEX i;
  BOOL allDigits = TRUE;
  for (i = 0; i < expr.GetLength(); i++) {
    allDigits = allDigits && isdigit(expr[i]);
  }

  if (allDigits)
    return expr;

  return GetVar(expr);
}

PString PVXMLSession::GetVar(const PString & ostr) const
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  // process session scope
  if (scope.IsEmpty() || (scope *= "session")) {
    if (sessionVars.Contains(str))
      return sessionVars(str);
  }

  // assume any other scope is actually document or application
  return documentVars(str);
}

void PVXMLSession::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  // do session scope
  if (scope.IsEmpty() || (scope *= "session")) {
    sessionVars.SetAt(str, val);
    return;
  }

  PTRACE(3, "PVXML\tDocument: " << str << " = \"" << val << "\"");

  // assume any other scope is actually document or application
  documentVars.SetAt(str, val);
}

BOOL PVXMLSession::PlayFile(const PString & fn, PINDEX repeat, PINDEX delay, BOOL autoDelete)
{
  if (outgoingChannel != NULL) {
    outgoingChannel->QueueFile(fn, repeat, delay, autoDelete);
    AllowClearCall();
  }

  return TRUE;
}

BOOL PVXMLSession::PlayCommand(const PString & cmd, PINDEX repeat, PINDEX delay)
{
  if (outgoingChannel != NULL) {
    outgoingChannel->QueueCommand(cmd, repeat, delay);
    AllowClearCall();
  }

  return TRUE;
}

BOOL PVXMLSession::PlayData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  if (outgoingChannel != NULL) {
    outgoingChannel->QueueData(data, repeat, delay);
    AllowClearCall();
  }

  return TRUE;
}


void PVXMLSession::GetBeepData(PBYTEArray & data, unsigned ms)
{
  if (outgoingChannel != NULL)
    outgoingChannel->GetBeepData(data, ms);
}


BOOL PVXMLSession::PlaySilence(const PTimeInterval & timeout)
{
  return PlaySilence((PINDEX)timeout.GetMilliSeconds());
}

BOOL PVXMLSession::PlaySilence(PINDEX msecs)
{
  if (outgoingChannel != NULL) {
    PBYTEArray nothing;
    outgoingChannel->QueueData(nothing, 1, msecs);
    AllowClearCall();
  }

  return TRUE;
}

BOOL PVXMLSession::PlayResource(const PURL & url, PINDEX repeat, PINDEX delay)
{
  if (outgoingChannel != NULL) {
    outgoingChannel->QueueResource(url, repeat, delay);
    AllowClearCall();
  }

  return TRUE;
}

BOOL PVXMLSession::LoadGrammar(PVXMLGrammar * grammar)
{
  if (activeGrammar != NULL) {
    delete activeGrammar;
    activeGrammar = FALSE;
  }

  activeGrammar = grammar;

  return TRUE;
}

BOOL PVXMLSession::PlayText(const PString & text, PTextToSpeech::TextType type, PINDEX repeat, PINDEX delay)
{
  if (textToSpeech != NULL) {
    PFilePath tmpfname("tts", NULL);
    PRandom r;
    PFilePath fname(tmpfname.GetDirectory() + (psprintf("tts_%i.wav", r.Generate() % 1000000)));
    if (!textToSpeech->OpenFile(fname)) {
      PTRACE(2, "PVXML\tcannot open file " << fname);
    } else {
      BOOL spoken = textToSpeech->Speak(text, type);
      if (!textToSpeech->Close()) {
        PTRACE(2, "PVXML\tcannot close TTS engine");
      }
      if (!spoken) {
        PTRACE(2, "PVXML\tcannot speak text using TTS engine");
      } else if (!PlayFile(fname, repeat, delay, TRUE)) {
        PTRACE(2, "PVXML\tCannot play " << fname);
      } else {
        PTRACE(2, "PVXML\tText queued");
      }
    }
  }

  return TRUE;
}

void PVXMLSession::SetPause(BOOL _pause)
{
  incomingChannel->SetPause(_pause);
  outgoingChannel->SetPause(_pause);
}



BOOL PVXMLSession::IsPlaying() const
{
  return (outgoingChannel != NULL) && outgoingChannel->IsPlaying();
}

BOOL PVXMLSession::StartRecording(const PFilePath & _recordFn, 
                                               BOOL _recordDTMFTerm, 
                              const PTimeInterval & _recordMaxTime, 
                              const PTimeInterval & _recordFinalSilence)
{
  recording          = TRUE;
  recordFn           = _recordFn;
  recordDTMFTerm     = _recordDTMFTerm;
  recordMaxTime      = _recordMaxTime;
  recordFinalSilence = _recordFinalSilence;

  if (incomingChannel != NULL)
    return incomingChannel->StartRecording(recordFn, (unsigned )recordFinalSilence.GetMilliSeconds());

  return FALSE;
}

void PVXMLSession::RecordEnd()
{
  if (recording)
    recordSync.Signal();
}

BOOL PVXMLSession::EndRecording()
{
  if (recording) {
	  recording = FALSE;
    if (incomingChannel != NULL)
      return incomingChannel->EndRecording();
  }

  return FALSE;
}


BOOL PVXMLSession::IsRecording() const
{
  return (incomingChannel != NULL) && incomingChannel->IsRecording();
}

PWAVFile * PVXMLSession::CreateWAVFile(const PFilePath & fn, PFile::OpenMode mode, int opts, unsigned fmt)
{ 
  return new PWAVFile(fn, mode, opts, fmt); 
}

void PVXMLSession::AllowClearCall()
{
  loaded = TRUE;
}

BOOL PVXMLSession::TraverseAudio()
{
  if (!currentNode->IsElement()) {
    PlayText(((PXMLData *)currentNode)->GetString());
  }

  else {
    PXMLElement * element = (PXMLElement *)currentNode;

    if (element->GetName() *= "value") {
      PString className = element->GetAttribute("class");
      PString value = EvaluateExpr(element->GetAttribute("expr"));
      SayAs(className, value);
    }

    else if (element->GetName() *= "sayas") {
      PString className = element->GetAttribute("class");
      PXMLObject * object = element->GetElement();
      if (!object->IsElement()) {
        PString text = ((PXMLData *)object)->GetString();
        SayAs(className, text);
      }
    }

    else if (element->GetName() *= "break") {

      // msecs is VXML 1.0
      if (element->HasAttribute("msecs"))
        PlaySilence(element->GetAttribute("msecs").AsInteger());

      // time is VXML 2.0
      else if (element->HasAttribute("time")) {
        PTimeInterval time = StringToTime(element->GetAttribute("time"));
        PlaySilence(time);
      }
      
      else if (element->HasAttribute("size")) {
        PString size = element->GetAttribute("size");
        if (size *= "none")
          ;
        else if (size *= "small")
          PlaySilence(SMALL_BREAK_MSECS);
        else if (size *= "large")
          PlaySilence(LARGE_BREAK_MSECS);
        else 
          PlaySilence(MEDIUM_BREAK_MSECS);
      } 
      
      // default to medium pause
      else {
        PlaySilence(MEDIUM_BREAK_MSECS);
      }
    }

    else if (element->GetName() *= "audio") {
      BOOL loaded = FALSE;

      if (element->HasAttribute("src")) {

        PString str = element->GetAttribute("src").Trim();
        if (!str.IsEmpty() && (str[0] == '|')) {
          loaded = TRUE;
          PlayCommand(str.Mid(1));
        } 
        
        else {
          PFilePath fn; 
          BOOL haveFn = FALSE;

          // get a normalised name for the resource
          PURL url = NormaliseResourceName(str);

          if ((url.GetScheme() *= "http") || (url.GetScheme() *= "https")) {

            PString contentType;
            PBYTEArray data;
            if (RetrieveResource(url, data, contentType, fn))
              haveFn = TRUE;
          }

          // attempt to load the resource if a file
          else if (url.GetScheme() *= "file") {
            fn = url.AsFilePath();
            haveFn = TRUE;
          }

          // check the file type
          if (haveFn) {
            PWAVFile * wavFile = outgoingChannel->CreateWAVFile(fn);
            if (wavFile == NULL)
              PTRACE(3, "PVXML\tCannot create audio file " + fn);
            else if (!wavFile->IsOpen())
              delete wavFile;
            else {
              loaded = TRUE;
              PlayFile(fn);
            }
          }
        }

        if (loaded) {
          // skip to the next node
          if (element->HasSubObjects())
					  currentNode = element->GetElement(element->GetSize() - 1);
        }
      }
    }

    else 
      PTRACE(3, "PVXML\tUnknown audio tag " << element->GetName() << " encountered");
  }

  return TRUE;
}


BOOL PVXMLSession::TraverseGoto()		// <goto>
{
  PAssert(currentNode != NULL, "ProcessGotoElement(): Expected valid node");
  if (currentNode == NULL)
    return FALSE;
  
  // LATER: handle expr, expritem, fetchaudio, fetchhint, fetchtimeout, maxage, maxstale
  
  PAssert(currentNode->IsElement(), "ProcessGotoElement(): Expected element");
  
  // nextitem
  PString nextitem = ((PXMLElement*)currentNode)->GetAttribute("nextitem");
  if (!nextitem.IsEmpty()) {
    // LATER: Take out the optional #
    currentForm = FindForm(nextitem);
    currentNode = currentForm;
    if (currentForm == NULL) {
      // LATER: throw "error.semantic" or "error.badfetch" -- lookup which
      return FALSE;
    }
    return TRUE;
  }
  
  // next
  PString next = ((PXMLElement*)currentNode)->GetAttribute("next");
  // LATER: fixup filename to prepend path
  if (!next.IsEmpty())
  {
    PURL url = NormaliseResourceName(next);
    if (!LoadURL(url)) {
      // LATER: throw 'error.badfetch'
      return FALSE;
    }
    
    // LATER: handle '#' for picking the correct form (inside of the document)
    if (currentForm == NULL) {
      // LATER: throw 'error.badfetch'
      return FALSE;
    }
    return TRUE;
  }
  
  return FALSE;
}

BOOL PVXMLSession::TraverseGrammar()		// <grammar>
{
  // LATER: A bunch of work to do here!

  // For now we only support the builtin digits type and do not parse any grammars.

  // NOTE: For now we will process both <grammar> and <field> here.
  // NOTE: Later there needs to be a check for <grammar> which will pull
  //       out the text and process a grammar like '1 | 2'

  // Right now we only support one active grammar.
  if (activeGrammar != NULL) {
    PTRACE(2, "PVXML\tWarning: can only process one grammar at a time, ignoring previous grammar");
    delete activeGrammar;
    activeGrammar = NULL;
  }

  PVXMLGrammar * newGrammar = NULL;

  // Is this a built-in type?
  PString type = ((PXMLElement*)currentNode)->GetAttribute("type");
  if (!type.IsEmpty()) {
    PStringArray tokens = type.Tokenise("?;", TRUE);
    PString builtintype;
    if (tokens.GetSize() > 0)
      builtintype = tokens[0];

    if (builtintype *= "digits") {
      PINDEX minDigits(1);
      PINDEX maxDigits(100);

      // look at each parameter
      for (PINDEX i(1); i < tokens.GetSize(); i++) {
        PStringArray params = tokens[i].Tokenise("=", TRUE);
        if (params.GetSize() == 2) {
          if (params[0] *= "minlength") {
            minDigits = params[1].AsInteger();
          }
          else if (params[0] *= "maxlength") {
            maxDigits = params[1].AsInteger();
          }
          else if (params[0] *= "length") {
            minDigits = maxDigits = params[1].AsInteger();
          }
        }
        else {
          // Invalid parameter skipped
          // LATER: throw 'error.semantic'
        }
      }
      newGrammar = new PVXMLDigitsGrammar((PXMLElement*)currentNode, minDigits, maxDigits, "");
    }
    else {
      // LATER: throw 'error.unsupported'
      return FALSE;
    }
  }

  if (newGrammar != NULL)
    return LoadGrammar(newGrammar);

  return TRUE;
}

// Finds the proper event hander for 'noinput', 'filled', 'nomatch' and 'error'
// by searching the scope hiearchy from the current from
PXMLElement * PVXMLSession::FindHandler(const PString & event)
{
  PAssert(currentNode->IsElement(), "Expected 'PXMLElement' in PVXMLSession::FindHandler");
  PXMLElement * tmp = (PXMLElement *)currentNode;
  PXMLElement * handler = NULL;

  // Look in all the way up the tree for a handler either explicitly or in a catch
  while (tmp != NULL) {
    // Check for an explicit hander - i.e. <error>, <filled>, <noinput>, <nomatch>, <help>
    if ((handler = tmp->GetElement(event)) != NULL)
      return handler;

    // Check for a <catch>
    if ((handler = tmp->GetElement("catch")) != NULL) {
      PString strCond = handler->GetAttribute("cond");
      if (strCond.Find(event))
        return handler;
    }

    tmp = tmp->GetParent();
  }

  return NULL;
}

void PVXMLSession::SayAs(const PString & className, const PString & text)
{
  if (!text.IsEmpty()) {
    PTextToSpeech::TextType type = PTextToSpeech::Literal;

    if (className *= "digits")
      type = PTextToSpeech::Digits;

    else if (className *= "literal")
      type = PTextToSpeech::Literal;

    else if (className *= "number")
      type = PTextToSpeech::Number;

    else if (className *= "currency")
      type = PTextToSpeech::Currency;

    else if (className *= "time")
      type = PTextToSpeech::Time;

    else if (className *= "date")
      type = PTextToSpeech::Date;

    else if (className *= "phone")
      type = PTextToSpeech::Phone;

    else if (className *= "ipaddress")
      type = PTextToSpeech::IPAddress;

    else if (className *= "duration")
      type = PTextToSpeech::Duration;

    PlayText(text, type);
  }
}

PTimeInterval PVXMLSession::StringToTime(const PString & str)
{
  PTimeInterval timeout;

  long msecs = str.AsInteger();
  if (str.Find("ms") != P_MAX_INDEX)
    ;
  else if (str.Find("s") != P_MAX_INDEX)
    msecs = msecs * 1000;

  return PTimeInterval(msecs);
}



///////////////////////////////////////////////////////////////

PVXMLChannel::PVXMLChannel(PVXMLSession & _vxml,
                           BOOL incoming,
                           const PString & fmtName,
                           PINDEX frBytes,
                           unsigned frTime,
                           unsigned wavType,
                           const PString & prefix)
  : vxml(_vxml),
    isIncoming(incoming),
    frameBytes(frBytes),
    frameTime(frTime),
    wavFileType(wavType),
    wavFilePrefix(prefix)
{
  PINDEX pos = fmtName.Find('/');
  if (pos == P_MAX_INDEX) {
    formatName = fmtName;
    sampleFrequency = 8000;
  } else {
    formatName = fmtName.Left(pos);
    sampleFrequency = fmtName.Mid(pos+1).AsUnsigned();
  }
  closed      = FALSE;
  wavFile     = NULL;
  playing     = FALSE;
  recording   = FALSE;
  frameLen    = frameOffs = 0;
  silentCount = 20;         // wait 20 frames before playing the OGM
  paused      = FALSE;
}


PVXMLChannel::~PVXMLChannel()
{
  EndRecording();
}

BOOL PVXMLChannel::IsOpen() const
{
  return !closed;
}

BOOL PVXMLChannel::Close()
{ 
  PWaitAndSignal m(channelMutex);

  PIndirectChannel::Close(); 
  closed = TRUE; 
  return TRUE; 
}

PString PVXMLChannel::AdjustWavFilename(const PString & ofn)
{
  if (wavFilePrefix.IsEmpty())
    return ofn;

  PString fn = ofn;

  // add in _g7231 prefix, if not there already
  PINDEX pos = ofn.FindLast('.');
  if (pos == P_MAX_INDEX) {
    if (fn.Right(wavFilePrefix.GetLength()) != wavFilePrefix)
      fn += wavFilePrefix;
  }
  else {
    PString basename = ofn.Left(pos);
    PString ext      = ofn.Mid(pos+1);
    if (basename.Right(wavFilePrefix.GetLength()) != wavFilePrefix)
      basename += wavFilePrefix;
    fn = basename + "." + ext;
  }
  return fn;
}

PWAVFile * PVXMLChannel::CreateWAVFile(const PFilePath & fn)
{ 
  PWAVFile * wav = vxml.CreateWAVFile(AdjustWavFilename(fn),
                                       isIncoming ? PFile::WriteOnly : PFile::ReadOnly,
                                       PFile::ModeDefault,
                                       wavFileType);

  if (!wav->IsOpen())
    PTRACE(1, "VXML\tCould not open WAV file " << wav->GetName());
  else {
    // Check the wave file header
    if (!isIncoming && !wav->IsValid())
      PTRACE(1, "VXML\tWAV file header invalid for " << wav->GetName());
    else {
      if (wav->GetFormat() != wavFileType)
        PTRACE(1, "VXML\tIncorrect codec (is " << wav->GetFormat()
               << " should be " << GetWavFileType()
               << ") in WAV file " << wav->GetName());
      else {
        if (wav->GetFormat() == PWAVFile::fmt_PCM &&
            (wav->GetSampleRate() != 8000 ||
             wav->GetChannels() != 1 ||
             wav->GetSampleSize() != 16))
          PTRACE(1, "VXML\tIncorrect format for PCM WAV file" << wav->GetName() << "\n"
                    "Is " <<  wav->GetSampleSize() << " bits, "
                          << (wav->GetChannels()==1 ? "mono " : "stereo ")
                          <<  wav->GetSampleRate() << " Hz"
                    " and should be a 16 Bit, Mono, 8000 Hz (8Khz)");
        else {
          PTRACE(4, "VXML\tOpened WAV file " << wav->GetName());
          return wav;
        }
      }
    }
  }

  delete wav;
  return NULL;
}


BOOL PVXMLChannel::Write(const void * buf, PINDEX len)
{
  PWaitAndSignal mutex(channelMutex);

  if (closed)
    return FALSE;

  unsigned msecs = (len+frameBytes-1)/frameBytes * frameTime;
  delay.Delay(msecs);

  lastWriteCount = len;

  if (wavFile == NULL || !wavFile->IsOpen())
    return TRUE;

  // only look for silence if nothing is playing
  if (recording && !playing) {
    if (!IsSilenceFrame(buf, len))
      silenceRun = 0;
    else {
      silenceRun += msecs;
      if (silenceRun > finalSilence) {
        PTRACE(3, "PVXML\tTriggering end of record due to silence timeout");
        vxml.RecordEnd();
      }
    }
  }

  return WriteFrame(buf, len);
}

BOOL PVXMLChannel::StartRecording(const PFilePath & fn, unsigned _finalSilence)
{
  // if there is already a file open, close it
  EndRecording();

  // open the output file
  PWaitAndSignal mutex(channelMutex);
  wavFile = CreateWAVFile(fn);
  if (wavFile == NULL || !wavFile->IsOpen()) {
    PTRACE(2, "PVXML\tCannot create record file " << fn);
    delete wavFile;
    return FALSE;
  }

  PTRACE(3, "PVXML\tStarting recording to " << fn);

  // save the number of seconds of silence that will terminate recording
  finalSilence = _finalSilence;
  silenceRun = 0;
  recording = TRUE;

  return TRUE;
}

BOOL PVXMLChannel::EndRecording()
{
  PWaitAndSignal mutex(channelMutex);

  recording = FALSE;

  PTRACE(3, "PVXML\tRecording finished");

  if (wavFile == NULL)
    return TRUE;

  wavFile->Close();

  delete wavFile;
  wavFile = NULL;
  
  return TRUE;
}


BOOL PVXMLChannel::Read(void * buffer, PINDEX amount)
{
  PWaitAndSignal m(channelMutex);

  if (closed)
    return FALSE;

  // Create the frame buffer using the amount of bytes the codec wants to
  // read. Different codecs use different read sizes.
  frameBuffer.SetMinSize(1024); //amount);

  // assume we are returning silence
  BOOL doSilence = TRUE;
  BOOL frameBoundary = FALSE;

  // if still outputting a frame from last time, then keep doing it
  if (frameOffs < frameLen) {

    frameBoundary = AdjustFrame(buffer, amount);
    doSilence = FALSE;

  } else {

    // if we are paused or in a delay, then do nothing
    if (paused || delayTimer.IsRunning())
      ;

    // if we are returning silence frames, then decrement the frame count
    else if (silentCount > 0) 
      silentCount--;

    // if a channel is already open, don't do silence
    else if (GetBaseReadChannel() != NULL)
      doSilence = FALSE;

    // check play queue
    else {
      PINDEX qSize;
      {
        PWaitAndSignal m(queueMutex);
        qSize = playQueue.GetSize();
      }

      // if nothing in queue, then re-execute VXML
      if (qSize == 0) {
        if (!vxml.Execute())
          return FALSE;
      }

      // otherwise queue the next data item
      else {
        {
          PWaitAndSignal m(queueMutex);
          PVXMLQueueItem * qItem = (PVXMLQueueItem *)playQueue.GetAt(0);
          qItem->OnStart();
          qItem->Play(*this);
        }
        doSilence = FALSE;
        totalData = 0;
        playing = TRUE;
      }
    }

    // if not doing silence, try and read more data
    if (!doSilence) {
  
      if (ReadFrame(amount)) {
        frameBoundary = AdjustFrame(buffer, amount);
        totalData += amount;
  
      } else {

        playing = FALSE;
        doSilence = TRUE;

        PTRACE(3, "PVXML\tFinished playing " << totalData << " bytes");
        PIndirectChannel::Close();

        // get the item that was just playing
        PINDEX delay;
        {
          PWaitAndSignal m(queueMutex);
          PVXMLQueueItem * qItem = (PVXMLQueueItem *)playQueue.GetAt(0);
          if (PAssertNULL(qItem) == NULL)
            return FALSE;

          // get the delay time BEFORE deleting the info
          delay = qItem->delay;

          // if the repeat count is zero, then dequeue entry 
          if (--qItem->repeat == 0) {
            qItem->OnStop();
            delete playQueue.Dequeue();
          }
        }

        // if delay required, then setup the delay
        if (delay != 0) {
          PTRACE(3, "PVXML\tDelaying for " << delay);
          delayTimer = delay;
        }

        // if no delay, re-evaluate VXML script
        else {
          PINDEX qSize;
          {
            PWaitAndSignal m(queueMutex);
            qSize = playQueue.GetSize();
          }
          if (qSize == 0) {
            if (!vxml.Execute())
              return FALSE;
          }
        }
      }
    }
  }
  
  // start silence frame if required
  if (doSilence) {
    CreateSilenceFrame(amount);
    frameBoundary = AdjustFrame(buffer, amount);
  }

  // delay to synchronise to frame boundary
  if (frameBoundary)
    delay.Delay((amount+frameBytes-1)/frameBytes * frameTime);

  return TRUE;
}


BOOL PVXMLChannel::AdjustFrame(void * buffer, PINDEX amount)
{
  if ((frameOffs + amount) > frameLen) {
    PTRACE(5, "Reading past end of frame:offs=" << frameOffs << ",amt=" << amount << ",len=" << frameLen);
    amount = frameLen - frameOffs;
  }

  memcpy(buffer, frameBuffer.GetPointer()+frameOffs, amount);
  frameOffs += amount;

  lastReadCount = amount;

  return frameOffs == frameLen;
}

void PVXMLChannel::QueueResource(const PURL & url, PINDEX repeat, PINDEX delay)
{
  PTRACE(3, "PVXML\tEnqueueing resource " << url << " for playing");
  QueueItem(new PVXMLQueueURLItem(url, repeat, delay));
}

void PVXMLChannel::QueueFile(const PString & fn, PINDEX repeat, PINDEX delay, BOOL autoDelete)
{
  PTRACE(3, "PVXML\tEnqueueing file " << fn << " for playing");
  QueueItem(new PVXMLQueueFilenameItem(fn, repeat, delay, autoDelete));
}

void PVXMLChannel::QueueCommand(const PString & cmd, PINDEX repeat, PINDEX delay)
{
  PTRACE(3, "PVXML\tEnqueueing command " << cmd << " for playing");
  QueueItem(new PVXMLQueueCommandItem(cmd, formatName, sampleFrequency, repeat, delay));
}

void PVXMLChannel::QueueData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  PTRACE(3, "PVXML\tEnqueueing " << data.GetSize() << " bytes for playing");
  QueueItem(new PVXMLQueueDataItem(data, repeat, delay));
}

void PVXMLChannel::QueueItem(PVXMLQueueItem * newItem)
{
  PWaitAndSignal mutex(queueMutex);
  playQueue.Enqueue(newItem);
}

void PVXMLChannel::FlushQueue()
{
  PWaitAndSignal mutex(channelMutex);

  if (GetBaseReadChannel() != NULL)
    PIndirectChannel::Close();

  PWaitAndSignal m(queueMutex);
  PVXMLQueueItem * qItem;
  while ((qItem = playQueue.Dequeue()) != NULL)
    delete qItem;
}

///////////////////////////////////////////////////////////////

PVXMLChannelPCM::PVXMLChannelPCM(PVXMLSession & vxml, BOOL incoming)
  : PVXMLChannel(vxml, incoming, "PCM-16", 16, 1, PWAVFile::PCM_WavFile, PString::Empty())
{
}

BOOL PVXMLChannelPCM::WriteFrame(const void * buf, PINDEX len)
{
  return wavFile->Write(buf, len);
}

BOOL PVXMLChannelPCM::ReadFrame(PINDEX amount)
{
  frameOffs = 0;
  frameLen  = amount;

  BOOL result = PIndirectChannel::Read(frameBuffer.GetPointer(), frameLen);

  // if we did not read a full frame of audio, fill the end of the
  // frame with zeros.
  PINDEX count = GetLastReadCount();
  if (count < frameLen)
    memset(frameBuffer.GetPointer()+count, 0, frameLen-count);

  return result;
}

void PVXMLChannelPCM::CreateSilenceFrame(PINDEX amount)
{
  frameOffs = 0;
  frameLen  = amount;
  memset(frameBuffer.GetPointer(), 0, frameLen);
}

BOOL PVXMLChannelPCM::IsSilenceFrame(const void * buf, PINDEX len) const
{
  // Calculate the average signal level of this frame
  int sum = 0;

  const short * pcm = (const short *)buf;
  const short * end = pcm + len/2;
  while (pcm != end) {
    if (*pcm < 0)
      sum -= *pcm++;
    else
      sum += *pcm++;
  }

  // calc average
  unsigned level = sum / (len / 2);

  return level < 500; // arbitrary level
}

static short beepData[] = { 0, 18784, 30432, 30400, 18784, 0, -18784, -30432, -30400, -18784 };


void PVXMLChannelPCM::GetBeepData(PBYTEArray & data, unsigned ms)
{
  data.SetSize(0);
  while (data.GetSize() < (PINDEX)((ms * 8) / 2)) {
    PINDEX len = data.GetSize();
    data.SetSize(len + sizeof(beepData));
    memcpy(len + data.GetPointer(), beepData, sizeof(beepData));
  }
}

///////////////////////////////////////////////////////////////

PVXMLChannelG7231::PVXMLChannelG7231(PVXMLSession & vxml, BOOL incoming)
  : PVXMLChannel(vxml, incoming, "G.723.1", 24, 30, PWAVFile::fmt_VivoG7231, "_g7231")
{
}

BOOL PVXMLChannelG7231::WriteFrame(const void * buf, PINDEX /*len*/)
{
  return wavFile->Write(buf, 24);
}

BOOL PVXMLChannelG7231::ReadFrame(PINDEX /*amount*/)
{
  if (!PIndirectChannel::Read(frameBuffer.GetPointer(), 1))
    return FALSE;

  frameOffs = 0;
  static const PINDEX g7231Lens[] = { 24, 20, 4, 1 };
  frameLen = g7231Lens[frameBuffer[0]&3];

  return PIndirectChannel::Read(frameBuffer.GetPointer()+1, frameLen-1);
}

void PVXMLChannelG7231::CreateSilenceFrame(PINDEX /*amount*/)
{
  frameOffs = 0;
  frameLen  = 4;

  frameBuffer[0] = 2;
  memset(frameBuffer.GetPointer()+1, 0, 3);
}

BOOL PVXMLChannelG7231::IsSilenceFrame(const void * buf, PINDEX len) const
{
  if (len == 4)
    return TRUE;
  if (buf == NULL)
    return FALSE;
  return ((*(const BYTE *)buf)&3) == 2;
}

///////////////////////////////////////////////////////////////

PVXMLChannelG729::PVXMLChannelG729(PVXMLSession & vxml, BOOL incoming)
  : PVXMLChannel(vxml, incoming, "G.729", 10, 10, PWAVFile::fmt_G729, "_g729")
{
}

BOOL PVXMLChannelG729::WriteFrame(const void * buf, PINDEX /*len*/)
{
  return wavFile->Write(buf, 10);
}

BOOL PVXMLChannelG729::ReadFrame(PINDEX /*amount*/)
{
  return PIndirectChannel::Read(frameBuffer.GetPointer(), 10);
}

void PVXMLChannelG729::CreateSilenceFrame(PINDEX /*amount*/)
{
  frameOffs = 0;
  frameLen  = 10;

  memset(frameBuffer.GetPointer(), 0, 10);
}

BOOL PVXMLChannelG729::IsSilenceFrame(const void * /*buf*/, PINDEX /*len*/) const
{
  return FALSE;
}

///////////////////////////////////////////////////////////////

void PVXMLQueueFilenameItem::Play(PVXMLChannel & outgoingChannel)
{
  PChannel * chan = NULL;

  // check the file extension and open a .wav or a raw (.sw or .g723) file
  if ((fn.Right(4)).ToLower() == ".wav")
    chan = outgoingChannel.CreateWAVFile(fn);
  else {
    PFile * fileChan = new PFile(fn);
    if (fileChan->Open(PFile::ReadOnly))
      chan = fileChan;
    else
      delete fileChan;
  }

  if (chan == NULL)
    PTRACE(3, "PVXML\tCannot open file \"" << fn << "\"");
  else {
    PTRACE(3, "PVXML\tPlaying file \"" << fn << "\"");
    outgoingChannel.SetReadChannel(chan, TRUE);
  }
}

void PVXMLQueueFilenameItem::OnStop() 
{
  if (autoDelete) 
    PFile::Remove(fn); 
}

///////////////////////////////////////////////////////////////

void PVXMLQueueCommandItem::Play(PVXMLChannel & outgoingChannel)
{
  cmd.Replace("%s", PString(PString::Unsigned, sampleFrequency));
  cmd.Replace("%f", format);

  // execute a command and send the output through the stream
  pipeCmd = new PPipeChannel;
  if (!pipeCmd->Open(cmd, PPipeChannel::ReadOnly)) {
    PTRACE(3, "PVXML\tCannot open command " << cmd);
    delete pipeCmd;
    return;
  }

  if (pipeCmd == NULL)
    PTRACE(3, "PVXML\tCannot open command \"" << cmd << "\"");
  else {
    pipeCmd->Execute();
    PTRACE(3, "PVXML\tPlaying command \"" << cmd << "\"");
    outgoingChannel.SetReadChannel(pipeCmd, TRUE);
  }
}

void PVXMLQueueCommandItem::OnStop() 
{
  if (pipeCmd != NULL) {
    pipeCmd->WaitForTermination();
    delete pipeCmd;
  }
}

///////////////////////////////////////////////////////////////

void PVXMLQueueURLItem::Play(PVXMLChannel & outgoingChannel)
{
  // open the resource
  PHTTPClient * client = new PHTTPClient;
  PMIMEInfo outMIME, replyMIME;
  int code = client->GetDocument(url, outMIME, replyMIME, FALSE);
  if ((code != 200) || (replyMIME(PHTTP::TransferEncodingTag) *= PHTTP::ChunkedTag))
    delete client;
  else {
    outgoingChannel.SetReadChannel(client, TRUE);
  }
}

///////////////////////////////////////////////////////////////

void PVXMLQueueDataItem::Play(PVXMLChannel & outgoingChannel)
{
  PMemoryFile * chan = new PMemoryFile(data);
  PTRACE(3, "PVXML\tPlaying " << data.GetSize() << " bytes");
  outgoingChannel.SetReadChannel(chan, TRUE);
}

/////////////////////////////////////////////////////////////////////////////////////////

PVXMLGrammar::PVXMLGrammar(PXMLElement * _field)
  : field(_field), state(PVXMLGrammar::NOINPUT)
{
}

//////////////////////////////////////////////////////////////////

PVXMLMenuGrammar::PVXMLMenuGrammar(PXMLElement * _field)
  : PVXMLGrammar(_field)
{
}

//////////////////////////////////////////////////////////////////

PVXMLDigitsGrammar::PVXMLDigitsGrammar(PXMLElement * _field, PINDEX _minDigits, PINDEX _maxDigits, PString _terminators)
  : PVXMLGrammar(_field),
  minDigits(_minDigits),
  maxDigits(_maxDigits),
  terminators(_terminators)
{
  PAssert(_minDigits <= _maxDigits, "Error - invalid grammar parameter");
}

BOOL PVXMLDigitsGrammar::OnUserInput(const PString & str)
{
  // Ignore any other keys if we've already filled the grammar
  if (state == PVXMLGrammar::FILLED || state == PVXMLGrammar::NOMATCH)
    return TRUE;

  // Does this string contain a terminator?
  PINDEX idx = str.FindOneOf(terminators);
  if (idx != P_MAX_INDEX) {
    // ??? should we return the string with the terminator
    value += str.Left(idx);
    state = (value.GetLength() >= minDigits && value.GetLength() <= maxDigits) ? 
      PVXMLGrammar::FILLED : 
      PVXMLGrammar::NOMATCH;
    return TRUE;
  }

  // Otherwise add to the grammar and check to see if we're done
  value += str;
  if (value.GetLength() == maxDigits) {
    state = PVXMLGrammar::FILLED;   // the grammar is filled!
    return TRUE;
  }

  return FALSE;
}


void PVXMLDigitsGrammar::Stop()
{
  // Stopping recognition here may change the state if something was
  // recognized but it didn't fill the number of digits requested
  if (!value.IsEmpty())
    state = PVXMLGrammar::NOMATCH;
  // otherwise the state will stay as NOINPUT
}

//////////////////////////////////////////////////////////////////

#endif 
