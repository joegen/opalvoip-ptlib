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

#define G7231_FRAME_SIZE  30

#include <ptclib/vxml.h>
#include <ptclib/memfile.h>

PVXMLSession::PVXMLSession(PTextToSpeech * _tts, BOOL autoDelete)
{
  activeGrammar   = NULL;
  recording       = FALSE;
  vxmlThread      = NULL;
  incomingChannel = NULL;
  outgoingChannel = NULL;
  loaded          = FALSE;
  forceEnd        = FALSE;
  textToSpeech    = NULL;

  SetTextToSpeech(_tts, autoDelete);
}

PVXMLSession::~PVXMLSession()
{
  if ((textToSpeech != NULL) && autoDeleteTextToSpeech) {
    delete textToSpeech;
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

BOOL PVXMLSession::Load(const PFilePath & filename)
{
  PWaitAndSignal m(sessionMutex);

  loaded = TRUE;

  if (!xmlFile.LoadFile(filename)) {
    PString err = "Cannot open root document " + filename + " - " + GetXMLError();
    PTRACE(1, "PVXML\t" << err);
    return FALSE;
  }

  PXMLElement * root = xmlFile.GetRootElement();
  if (root == NULL)
    return FALSE;

  // find all dialogs in the document
  PINDEX i;
  for (i = 0; i < root->GetSize(); i++) {
    PXMLObject * xmlObject = root->GetElement(i);
    if (xmlObject->IsElement()) {
      PXMLElement * xmlElement = (PXMLElement *)xmlObject;
      PVXMLDialog * dialog = NULL;

      if (xmlElement->GetName() == "form") {
        dialog = new PVXMLFormDialog(*this, *xmlElement);
        dialog->Load();
      }

      if (dialog != NULL)
        dialogArray.SetAt(dialogArray.GetSize(), dialog);
    }
  }

  return TRUE;
}

BOOL PVXMLSession::Open(BOOL isPCM)
{
  PWaitAndSignal m(sessionMutex);

  PVXMLOutgoingChannel * out; 
  PVXMLIncomingChannel * in; 

  if (isPCM) {
    out = new PVXMLOutgoingChannelPCM(*this);
    in  = new PVXMLIncomingChannelPCM(*this);
  } else {
    out = new PVXMLOutgoingChannelG7231(*this);
    in  = new PVXMLIncomingChannelG7231(*this);
  }

  BOOL stat = PIndirectChannel::Open(out, in);

  if (stat) {
    outgoingChannel = out;
    incomingChannel = in;
  }

  return stat;
}

void PVXMLSession::EndSession()
{
  PWaitAndSignal m(sessionMutex);

  if (outgoingChannel == NULL)
    return;

  if (vxmlThread != NULL) {
    vxmlThread->WaitForTermination();
  }

  if (outgoingChannel != NULL)
    outgoingChannel->Close();

  if (outgoingChannel != NULL)
    incomingChannel->Close();
}


BOOL PVXMLSession::Execute()
{
  PWaitAndSignal m(sessionMutex);

  return ExecuteWithoutLock();
}


BOOL PVXMLSession::ExecuteWithoutLock()
{
  if (forceEnd) {
    OnEndSession();
    EndSession(); 
    return TRUE;
  }

  // check to see if a vxml thread has stopped since last we looked
  if ((vxmlThread != NULL) && (vxmlThread->IsTerminated())) {
    delete vxmlThread;
    vxmlThread = NULL;
    return vxmlStatus;
  }

  // if:
  //    no script has been loaded or
  //    there is already a thread running or
  //    a grammar defined or
  //    recording is in progress
  //    no outgoing channel
  // then just return silence
  //
  if (!loaded || (vxmlThread != NULL) || (activeGrammar != NULL) || recording || (outgoingChannel == NULL))
    return TRUE;

  // throw a thread to execute the VXML, because this can take some time
  vxmlThread = PThread::Create(PCREATE_NOTIFIER(DialogExecute), 0, PThread::NoAutoDeleteThread);
  return TRUE;
}

void PVXMLSession::DialogExecute(PThread &, INT)
{
  // find the first dialog that has an undefined form variable
  PINDEX i;
  for (i = 0; i < dialogArray.GetSize(); i++) {
    PVXMLDialog & dialog = dialogArray[i];

    // if this form is not yet defined, then enter it
    if (!dialog.GetGuardCondition()) {

      // execute the form, and clear call if error
      if (!dialog.Execute())
        break;
    }
  }

  // if all forms defined, nothing playing, and nothing recording, then end of call
  if ((activeGrammar == NULL) && !IsPlaying() && !IsRecording()) {
    if (OnEmptyAction())
      forceEnd = TRUE;
  }

  vxmlStatus = FALSE;
}


BOOL PVXMLSession::OnUserInput(char ch)
{
  PWaitAndSignal m(sessionMutex);

  if (activeGrammar != NULL) {

    // if the grammar has not completed, continue
    if (!activeGrammar->OnUserInput(ch))
      return TRUE;

    // if the grammar has completed, save the value and define the field
    activeGrammar->GetField().SetFormValue(activeGrammar->GetValue());

    // remove the grammar
    LoadGrammar(NULL);

    // execute whatever is going on
    ExecuteWithoutLock();
  }

  return TRUE;
}

void PVXMLSession::StartRecord(const PFilePath & _recordfn, BOOL dtmfTerm, int maxTime, int finalSilence)
{
  recording          = TRUE;
  recordFn           = _recordfn;
  recordDTMFTerm     = dtmfTerm;
  recordMaxTime      = maxTime;
  recordFinalSilence = finalSilence;
}


PString PVXMLSession::GetXMLError() const
{
  return psprintf("(%i:%i) ", xmlFile.GetErrorLine(), xmlFile.GetErrorColumn()) + xmlFile.GetErrorString();
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
  if (outgoingChannel != NULL)
    outgoingChannel->QueueFile(fn, repeat, delay, autoDelete);

  return TRUE;
}

BOOL PVXMLSession::PlayData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  if (outgoingChannel != NULL)
    outgoingChannel->QueueData(data, repeat, delay);

  return TRUE;
}

BOOL PVXMLSession::PlayText(const PString & text, PTextToSpeech::TextType type, PINDEX repeat, PINDEX delay)
{
  if (textToSpeech != NULL) {
    PFilePath fname("tts", NULL);
    fname = fname + ".wav";
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


BOOL PVXMLSession::IsPlaying() const
{
  return (outgoingChannel != NULL) && outgoingChannel->IsPlaying();
}

BOOL PVXMLSession::StartRecording(const PFilePath & fn)
{
  if (incomingChannel != NULL)
    return incomingChannel->StartRecording(fn);

  return FALSE;
}

BOOL PVXMLSession::EndRecording()
{
  if (incomingChannel != NULL)
    return incomingChannel->EndRecording();

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

///////////////////////////////////////////////////////////////

PVXMLElement::PVXMLElement(PVXMLSession & _vxml, PXMLElement & _xmlElement)
  : vxml(_vxml), xmlElement(_xmlElement)
{
  name = xmlElement.GetAttribute("name");
  if (name.IsEmpty())
    name = psprintf("item_%08x", (int)this);
}

PString PVXMLElement::GetVar(const PString & str) const
{
  return vars(str);
}

void PVXMLElement::SetVar(const PString & str, const PString & val)
{
  vars.SetAt(str, val); 
}

BOOL PVXMLElement::GetGuardCondition() const
{ 
  return !GetFormValue().IsEmpty();
}

PString PVXMLElement::GetFormValue() const
{ 
  return PVXMLElement::GetVar(name);
}

void PVXMLElement::SetFormValue(const PString & v)
{ 
  PVXMLElement::SetVar(name, v);
}

///////////////////////////////////////////////////////////////

PVXMLDialog::PVXMLDialog(PVXMLSession & _vxml, PXMLElement & _xmlForm)
  : PVXMLElement(_vxml, _xmlForm)
{
}

BOOL PVXMLDialog::Load()
{
  // find all items in form
  PINDEX i;
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsElement()) {
      PXMLElement * element = (PXMLElement *)object;
      PVXMLFormItem * formItem = NULL;

      if (element->GetName() == "block")
        formItem = new PVXMLBlockItem(vxml, *element, *this);

      else if (element->GetName() == "var") 
        formItem = new PVXMLVarItem(vxml, *element, *this);

      else if (element->GetName() == "field") 
        formItem = new PVXMLFieldItem(vxml, *element, *this);

      else if (element->GetName() == "record") 
        formItem = new PVXMLRecordItem(vxml, *element, *this);

      if (formItem != NULL) {
        formItem->Load();
        itemArray.SetAt(itemArray.GetSize(), formItem);
      }
    }
  }

  return TRUE;
}

BOOL PVXMLDialog::Execute()
{
  // return TRUE if we executed 
  PINDEX i;
  for (i = 0; i < itemArray.GetSize(); i++) {
    PVXMLFormItem & item = itemArray[i];
    if (!item.GetGuardCondition())
      return item.Execute();
  }

  return FALSE;
}

PString PVXMLDialog::GetVar(const PString & ostr) const
{
  PString str = ostr;

  // if the variable has scope, check to see if dialog otherwise move up the chain
  PINDEX pos = ostr.Find('.');
  if (pos != P_MAX_INDEX) {
    PString scope = str.Left(pos);
    if (!(scope *= "dialog"))
      return vxml.GetVar(ostr);

    str = str.Mid(pos+1);
  }

  // see if local
  if (vars.Contains(str))
    return PVXMLElement::GetVar(str);

  return vxml.GetVar(ostr);
}

void PVXMLDialog::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope if present
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str = str.Mid(pos+1);
  }

  // if scope is not dialog, for
  if (scope.IsEmpty() || (scope *= "dialog")) {
    PVXMLElement::SetVar(str, val);
    return;
  }

  PTRACE(3, "PVXML\tDialog(" << name << "): " << ostr << " = \"" << val << "\"");

  vxml.SetVar(ostr, val);
}

///////////////////////////////////////////////////////////////

PVXMLFormDialog::PVXMLFormDialog(PVXMLSession & vxml, PXMLElement & xmlItem)
  : PVXMLDialog(vxml, xmlItem)
{
}

///////////////////////////////////////////////////////////////

PVXMLFormItem::PVXMLFormItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLElement(_vxml, _xmlItem), parentDialog(_parentDialog)
{
}

PString PVXMLFormItem::GetFormValue() const
{
  return GetVar("dialog." + name);
}

void PVXMLFormItem::SetFormValue(const PString & v)
{
  SetVar("dialog." + name, v);
}

BOOL PVXMLFormItem::ProcessPrompt(PXMLElement & rootElement)
{
  PINDEX i;
  for (i = 0; i < rootElement.GetSize(); i++) {
    PXMLObject * object = rootElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());

    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() *= "value") {
        PString className = element->GetAttribute("class");
        PString value = EvaluateExpr(element->GetAttribute("expr"));
        SayAs(className, value);
      }

      else if (element->GetName() *= "sayas") {
        PString className = element->GetAttribute("class");
        PXMLObject * object = element->GetElement();
        if (object->IsData()) {
          PString text = ((PXMLData *)object)->GetString();
          SayAs(className, text);
        }
      }
    }
  }
  return TRUE;
}

void PVXMLFormItem::SayAs(const PString & className, const PString & text)
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

    vxml.PlayText(text, type);
  }
}

PString PVXMLFormItem::EvaluateExpr(const PString & oexpr)
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

PString PVXMLFormItem::GetVar(const PString & ostr) const
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  if (scope.IsEmpty()) {
    if (vars.Contains(str))
      return PVXMLElement::GetVar(str);
  }

  return parentDialog.GetVar(ostr);
}

void PVXMLFormItem::SetVar(const PString & ostr, const PString & val)
{
  PString str = ostr;
  PString scope;

  // get scope
  PINDEX pos = str.Find('.');
  if (pos != P_MAX_INDEX) {
    scope = str.Left(pos);
    str   = str.Mid(pos+1);
  }

  if (scope.IsEmpty())
    PVXMLElement::SetVar(str, val);

  parentDialog.SetVar(ostr, val);
}

///////////////////////////////////////////////////////////////

PVXMLBlockItem::PVXMLBlockItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLBlockItem::Execute()
{
  PINDEX i;
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());
    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() == "prompt")
        ProcessPrompt(*element);
    }
  }
  this->SetFormValue("1");

  return TRUE;
}

///////////////////////////////////////////////////////////////

PVXMLVarItem::PVXMLVarItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLVarItem::Execute()
{
  PString name = xmlElement.GetAttribute("name");
  PString expr = xmlElement.GetAttribute("expr");

  PTRACE(3, "PVXML\tAssigning expr \"" << expr << "\" to var \"" << name << "\" in scope of dialog \"" << parentDialog.GetName());

  parentDialog.SetVar(name, expr);

  return TRUE;
}

///////////////////////////////////////////////////////////////

PVXMLRecordItem::PVXMLRecordItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
  // get DTMF termination flag
  PString str = _xmlItem.GetAttribute("dtmfterm");
  if (str.IsEmpty())
    dtmfTerm = TRUE;
  else
    dtmfTerm = str *= "true";

  // get maximum record duration
  str = _xmlItem.GetAttribute("maxtime");
  if (str.IsEmpty())
    maxTime = -1;
  else
    maxTime = str.AsInteger();

  // get final silence duration
  str = _xmlItem.GetAttribute("finalsilence");
  if (str.IsEmpty())
    finalSilence = -1;
  else
    finalSilence = str.AsInteger();
}

BOOL PVXMLRecordItem::Execute()
{
  PFilePath recordFn("vxml"); 
  vxml.StartRecord(recordFn, dtmfTerm, maxTime, finalSilence);
  return TRUE;
}


///////////////////////////////////////////////////////////////

PVXMLFieldItem::PVXMLFieldItem(PVXMLSession & _vxml, PXMLElement & _xmlItem, PVXMLDialog & _parentDialog)
  : PVXMLFormItem(_vxml, _xmlItem, _parentDialog)
{
}

BOOL PVXMLFieldItem::Execute()
{
  PINDEX i;

  // queue up the prompts
  for (i = 0; i < xmlElement.GetSize(); i++) {
    PXMLObject * object = xmlElement.GetElement(i);
    if (object->IsData())
      vxml.PlayText(((PXMLData *)object)->GetString());
    else {
      PXMLElement * element = (PXMLElement *)object;
      if (element->GetName() == "prompt")
        ProcessPrompt(*element);
    }
  }

  // load the grammar for this field, if we can build it
  PVXMLGrammar * grammar = NULL;
  PString grammarType = xmlElement.GetAttribute("type");
  if (grammarType == "digits") {
    PString lengthStr = xmlElement.GetAttribute("length");
    if (!lengthStr.IsEmpty()) {
      grammar = new PVXMLDigitsGrammar(*this, lengthStr.AsInteger());
    }
  }

  if (grammar != NULL)
    return vxml.LoadGrammar(grammar);

  return FALSE;
}

//////////////////////////////////////////////////////////////////

PVXMLGrammar::PVXMLGrammar(PVXMLFieldItem & _field)
  : field(_field)
{
}

//////////////////////////////////////////////////////////////////

PVXMLMenuGrammar::PVXMLMenuGrammar(PVXMLFieldItem & _field)
  : PVXMLGrammar(_field)
{
}

//////////////////////////////////////////////////////////////////

PVXMLDigitsGrammar::PVXMLDigitsGrammar(PVXMLFieldItem & _field, PINDEX _digitCount)
  : PVXMLGrammar(_field), digitCount(_digitCount)
{
}

BOOL PVXMLDigitsGrammar::OnUserInput(char ch)
{
  value += ch;

  if (value.GetLength() < digitCount)
    return FALSE;

  cout << "grammar \"digits\" completed: value = " << value << endl;

  return TRUE;
}

//////////////////////////////////////////////////////////////////

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

//////////////////////////////////////////////////////////////////

PVXMLOutgoingChannel::PVXMLOutgoingChannel(PVXMLSession & _vxml)
  : PVXMLChannel(_vxml, FALSE)
{
  playing = FALSE;
  frameLen = frameOffs = 0;
  silentCount = 20;         // wait 20 frames before playing the OGM
}

BOOL PVXMLOutgoingChannel::AdjustFrame(void * buffer, PINDEX amount)
{
  if ((frameOffs + amount) > frameLen) {
    cerr << "Reading past end of frame:offs=" << frameOffs << ",amt=" << amount << ",len=" << frameLen << endl;
    return TRUE;
  }
  //PAssert((frameOffs + amount) <= frameLen, "Reading past end of frame");

  memcpy(buffer, frameBuffer.GetPointer()+frameOffs, amount);
  frameOffs += amount;

  lastReadCount = amount;

  return frameOffs == frameLen;
}

void PVXMLOutgoingChannel::QueueFile(const PString & fn, PINDEX repeat, PINDEX delay, BOOL autoDelete)
{
  PTRACE(3, "PVXML\tEnqueueing file " << fn << " for playing");
  QueueItem(new PVXMLQueueFilenameItem(fn, repeat, delay, autoDelete));
}

void PVXMLOutgoingChannel::QueueData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  PTRACE(3, "PVXML\tEnqueueing " << data.GetSize() << " bytes for playing");
  QueueItem(new PVXMLQueueDataItem(data, repeat, delay));
}

void PVXMLOutgoingChannel::QueueItem(PVXMLQueueItem * newItem)
{
  PWaitAndSignal mutex(queueMutex);
  playQueue.Enqueue(newItem);
}

void PVXMLOutgoingChannel::FlushQueue()
{
  PWaitAndSignal mutex(channelMutex);

  if (GetBaseReadChannel() != NULL)
    PIndirectChannel::Close();

  PWaitAndSignal m(queueMutex);
  PVXMLQueueItem * qItem;
  while ((qItem = playQueue.Dequeue()) != NULL)
    delete qItem;
}

BOOL PVXMLOutgoingChannel::Read(void * buffer, PINDEX amount)
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

    // if we are in a delay, then do nothing
    if (delayTimer.IsRunning())
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
      if (qSize == 0)
        vxml.Execute();

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
          PAssertNULL(qItem);

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
          if (qSize == 0)
            vxml.Execute();
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
    DelayFrame(amount);

  return TRUE;
}

PWAVFile * PVXMLOutgoingChannel::CreateWAVFile(const PFilePath & fn)
{ 
  PWAVFile * file = vxml.CreateWAVFile(fn, PFile::ReadOnly, PFile::ModeDefault, GetWavFileType()); 
  if (!IsWAVFileValid(*file)) {
    delete file;
    file = NULL;
  }

  return file;
}

///////////////////////////////////////////////////////////////

static BOOL CheckWAVFileValid(PWAVFile & chan, BOOL mustBePCM)
{
  // Check the wave file header
  if (!chan.IsValid()) {
    PTRACE(1, chan.GetName() << " wav file header invalid");
    return FALSE;
  }

  // Check the wave file format
  if (mustBePCM) {
    if (chan.GetFormat() != PWAVFile::fmt_PCM) {
      PTRACE(1, chan.GetName() << " is not a PCM format wav file");
      PTRACE(1, "It is format " << chan.GetFormat() );
      return FALSE;
    }

    if ((chan.GetSampleRate() != 8000) &&
        (chan.GetChannels() != 1) &&
        (chan.GetSampleSize() != 16)) {
      PTRACE(1, chan.GetName() << " is not a 16 Bit, Mono, 8000 Hz (8Khz) PCM wav file");
      PTRACE(1, "It is " << chan.GetSampleSize() << " bits, "
                         << (chan.GetChannels()==1 ? "mono " : "stereo ")
                         << chan.GetSampleRate() << " Hz");
      return FALSE;
    }
  }

  else if ((chan.GetFormat() != PWAVFile::fmt_MSG7231) && 
      (chan.GetFormat() != PWAVFile::fmt_VivoG7231)) {
    PTRACE(1, chan.GetName() << " is not a G.723.1 format wav file");
    PTRACE(1, "It is format " << chan.GetFormat() );
    return FALSE;
  }

  return TRUE;
}

static int GetG7231FrameLen(BYTE firstByte)
{
  static int g7231Lens[] = { 24, 20, 4, 1 };
  return g7231Lens[firstByte & 3];
}

///////////////////////////////////////////////////////////////

PVXMLOutgoingChannelPCM::PVXMLOutgoingChannelPCM(PVXMLSession & vxml)
  : PVXMLOutgoingChannel(vxml)
{
}

BOOL PVXMLOutgoingChannelPCM::IsWAVFileValid(PWAVFile & chan) 
{
  return CheckWAVFileValid(chan, TRUE);
}

void PVXMLOutgoingChannelPCM::DelayFrame(PINDEX amount)
{
  delay.Delay(amount / 16);
}

BOOL PVXMLOutgoingChannelPCM::ReadFrame(PINDEX amount)
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

void PVXMLOutgoingChannelPCM::CreateSilenceFrame(PINDEX amount)
{
  frameOffs = 0;
  frameLen  = amount;
  memset(frameBuffer.GetPointer(), 0, frameLen);
}

///////////////////////////////////////////////////////////////

PVXMLOutgoingChannelG7231::PVXMLOutgoingChannelG7231(PVXMLSession & vxml)
  : PVXMLOutgoingChannel(vxml)
{
}

void PVXMLOutgoingChannelG7231::QueueFile(const PString & ofn, PINDEX repeat, PINDEX delay)
{
  PString fn = ofn;;

  // add in _g7231 prefix, if not there already
  PINDEX pos = ofn.FindLast('.');
  if (pos == P_MAX_INDEX) {
    if (fn.Right(6) != "_g7231")
      fn += "_g7231";
  } else {
    PString basename = ofn.Left(pos);
    PString ext      = ofn.Mid(pos+1);
    if (basename.Right(6) != "_g7231")
      basename += "_g7231";
    fn = basename + "." + ext;
  }
  PVXMLOutgoingChannel::QueueFile(fn, repeat, delay);
}

BOOL PVXMLOutgoingChannelG7231::IsWAVFileValid(PWAVFile & chan) 
{
  return CheckWAVFileValid(chan, FALSE);
}

void PVXMLOutgoingChannelG7231::DelayFrame(PINDEX /*amount*/)
{
  delay.Delay(30);
}

BOOL PVXMLOutgoingChannelG7231::ReadFrame(PINDEX /*amount*/)
{
  if (!PIndirectChannel::Read(frameBuffer.GetPointer(), 1))
    return FALSE;

  frameOffs = 0;
  frameLen = GetG7231FrameLen(frameBuffer[0]);

  return PIndirectChannel::Read(frameBuffer.GetPointer()+1, frameLen-1);
}

void PVXMLOutgoingChannelG7231::CreateSilenceFrame(PINDEX /*amount*/)
{
  frameOffs = 0;
  frameLen  = 4;

  frameBuffer[0] = 2;
  memset(frameBuffer.GetPointer()+1, 0, 3);
}

///////////////////////////////////////////////////////////////

void PVXMLQueueFilenameItem::Play(PVXMLOutgoingChannel & outgoingChannel)
{
  // check the file extension and open a .wav or a raw (.sw or .g723) file
  if ((fn.Right(4)).ToLower() == ".wav") {
    PWAVFile * chan = outgoingChannel.CreateWAVFile(fn);
    if (chan == NULL) {
      PTRACE(3, "PVXML\tCannot open outgoing WAV file");
    }
    else if (!chan->IsOpen()) {
      PTRACE(3, "PVXML\tCannot open file \"" << chan->GetName() << '"');
      delete chan;
    } else {
      PTRACE(3, "PVXML\tPlaying file \"" << chan->GetName() << "\"");
      outgoingChannel.SetReadChannel(chan, TRUE);
    }
  } else { // raw file (eg .sw)
    PFile *chan;
    chan = new PFile(fn);
    if (!chan->Open(PFile::ReadOnly)) {
      PTRACE(3, "PVXML\tCannot open file \"" << chan->GetName() << "\"");
      delete chan;
    } else {
      PTRACE(3, "PVXML\tPlaying file \"" << chan->GetName() << "\"");
      outgoingChannel.SetReadChannel(chan, TRUE);
    }
  }
}

///////////////////////////////////////////////////////////////

void PVXMLQueueDataItem::Play(PVXMLOutgoingChannel & outgoingChannel)
{
  PMemoryFile * chan = new PMemoryFile(data);
  PTRACE(3, "PVXML\tPlaying " << data.GetSize() << " bytes");
  outgoingChannel.SetReadChannel(chan, TRUE);
}

///////////////////////////////////////////////////////////////

PVXMLIncomingChannel::PVXMLIncomingChannel(PVXMLSession & _vxml)
  : PVXMLChannel(_vxml, TRUE)
{
  wavFile = NULL;
}

PVXMLIncomingChannel::~PVXMLIncomingChannel()
{
  EndRecording();
}

BOOL PVXMLIncomingChannel::Write(const void * buf, PINDEX len)
{
  PWaitAndSignal mutex(channelMutex);

  if (closed)
    return FALSE;

  DelayFrame(len);

  if (wavFile == NULL || !wavFile->IsOpen())
    return TRUE;

  return WriteFrame(buf, len);
}

BOOL PVXMLIncomingChannel::StartRecording(const PFilePath & fn)
{
  // if there is already a file open, close it
  EndRecording();

  // open the output file
  PWaitAndSignal mutex(channelMutex);
  wavFile = CreateWAVFile(fn);
  PTRACE(3, "PVXML\tStarting recording to " << fn);
  if (!wavFile->IsOpen()) {
    PTRACE(2, "PVXML\tCannot create record file " << fn);
    return FALSE;
  }

  return TRUE;
}

BOOL PVXMLIncomingChannel::EndRecording()
{
  PWaitAndSignal mutex(channelMutex);

  if (wavFile == NULL)
    return TRUE;

  wavFile->Close();

  delete wavFile;
  wavFile = NULL;
  
  return TRUE;
}

PWAVFile * PVXMLIncomingChannel::CreateWAVFile(const PFilePath & fn)
{ 
  return vxml.CreateWAVFile(fn, PFile::WriteOnly, PFile::ModeDefault, GetWavFileType()); 
}

///////////////////////////////////////////////////////////////

PVXMLIncomingChannelPCM::PVXMLIncomingChannelPCM(PVXMLSession & vxml)
  : PVXMLIncomingChannel(vxml)
{
}

BOOL PVXMLIncomingChannelPCM::WriteFrame(const void * buf, PINDEX len)
{
  //cerr << "Writing PCM " << len << endl;
  return wavFile->Write(buf, len);
}

void PVXMLIncomingChannelPCM::DelayFrame(PINDEX len)
{
  delay.Delay(len/16);
}

///////////////////////////////////////////////////////////////
// Override some of the IncomingChannelPCM functions to write
// G723.1 data instead of PCM data.

PVXMLIncomingChannelG7231::PVXMLIncomingChannelG7231(PVXMLSession & vxml)
  : PVXMLIncomingChannel(vxml)
{
}

BOOL PVXMLIncomingChannelG7231::WriteFrame(const void * buf, PINDEX /*len*/)
{
  int frameLen = GetG7231FrameLen(*(BYTE *)buf);
  return wavFile->Write(buf, frameLen);
}

void PVXMLIncomingChannelG7231::DelayFrame(PINDEX /*len*/)
{
  // Ignore the len parameter as that is the compressed size.
  // We must delay by the actual sample time.
  delay.Delay(G7231_FRAME_SIZE);
}

/////////////////////////////////////////////////////////////////////////////////////////

#endif 
