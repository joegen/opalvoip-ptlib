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

#ifdef __GNUC__
#pragma interface
#endif

#include <ptclib/pxml.h>
#include <ptclib/delaychan.h>
#include <ptclib/pwavfile.h>

class PVXMLSession;


class PVXMLDialog;
class PVXMLSession;

class PVXMLElement : public PObject
{
  PCLASSINFO(PVXMLElement, PObject);
  public:
    PVXMLElement(PVXMLSession & vxml, PXMLElement & xmlElement);

    virtual BOOL Load()     { return TRUE; }
    virtual BOOL Execute()  { return TRUE; }

    virtual PString GetVar(const PString & str) const;
    virtual void    SetVar(const PString & str, const PString & val);

    virtual BOOL GetGuardCondition() const;

    virtual PString GetFormValue() const;
    virtual void SetFormValue(const PString & v);

    PString GetName() const { return name; }

  protected:
    PVXMLSession & vxml;
    PXMLElement  & xmlElement;
    PStringToString vars;
    PString name;
};

//////////////////////////////////////////////////////////////////

class PVXMLFormItem : public PVXMLElement 
{
  PCLASSINFO(PVXMLFormItem, PObject);
  public:
    PVXMLFormItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL ProcessPrompt(PXMLElement & element);
    void SayAs(const PString & className, const PString & text);

    PVXMLDialog & GetParentDialog() { return parentDialog; }

    PString EvaluateExpr(const PString & oexpr);

    PString GetFormValue() const;
    void SetFormValue(const PString & v);

    PString GetVar(const PString & str) const;
    void    SetVar(const PString & str, const PString & val);

  protected:
    PVXMLDialog & parentDialog;
    PStringToString formVars;
};

PARRAY(PVXMLFormItemArray, PVXMLFormItem);

//////////////////////////////////////////////////////////////////

class PVXMLBlockItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLBlockItem, PVXMLFormItem);
  public:
    PVXMLBlockItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLFieldItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLFieldItem, PVXMLFormItem);
  public:
    PVXMLFieldItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLVarItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLVarItem, PVXMLFormItem);
  public:
    PVXMLVarItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();
};

//////////////////////////////////////////////////////////////////

class PVXMLRecordItem : public PVXMLFormItem
{
  PCLASSINFO(PVXMLRecordItem, PVXMLFormItem);
  public:
    PVXMLRecordItem(PVXMLSession & vxml, PXMLElement & xmlItem, PVXMLDialog & parentDialog);
    BOOL Execute();

  protected:
    BOOL dtmfTerm;
    int maxTime;
    int finalSilence;
};

//////////////////////////////////////////////////////////////////

class PVXMLDialog : public PVXMLElement
{
  PCLASSINFO(PVXMLDialog, PObject);
  public:
    PVXMLDialog(PVXMLSession & vxml, PXMLElement & xmlItem);
    BOOL Load();
    BOOL Execute();

    PString GetVar(const PString & str) const;
    void SetVar(const PString & ostr, const PString & val);

  protected:
    PVXMLFormItemArray itemArray;
};

PARRAY(PVXMLDialogArray, PVXMLDialog);

//////////////////////////////////////////////////////////////////

class PVXMLFormDialog : public PVXMLDialog
{
  PCLASSINFO(PVXMLFormDialog, PVXMLDialog);
  public:
    PVXMLFormDialog(PVXMLSession & vxml, PXMLElement & xmlItem);
};

//////////////////////////////////////////////////////////////////

class PVXMLGrammar : public PObject
{
  PCLASSINFO(PVXMLGrammar, PObject);
  public:
    PVXMLGrammar(PVXMLFieldItem & field);
    virtual BOOL OnUserInput(char /*ch*/) { return TRUE; }

    PString GetValue() const { return value; }
    PVXMLFieldItem & GetField() { return field; }

  protected:
    PVXMLFieldItem & field;
    PString value;
};

//////////////////////////////////////////////////////////////////

class PVXMLMenuGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLMenuGrammar, PVXMLGrammar);
  public:
    PVXMLMenuGrammar(PVXMLFieldItem & field);
};

//////////////////////////////////////////////////////////////////

class PVXMLDigitsGrammar : public PVXMLGrammar
{
  PCLASSINFO(PVXMLDigitsGrammar, PVXMLGrammar);
  public:
    PVXMLDigitsGrammar(PVXMLFieldItem & field, PINDEX digitCount);
    BOOL OnUserInput(char ch);

  protected:
    PINDEX digitCount;
    PString digits;
};

//////////////////////////////////////////////////////////////////

class PVXMLSession : public PObject 
{
  PCLASSINFO(PVXMLSession, PObject);
  public:
    enum TextType {
      Default,
      Literal,
      Digits,
      Number,
      Currency,
      Time,
      Date,
      Phone,
      IPAddress,
      Duration
    };

    PVXMLSession();

    virtual BOOL Load(const PFilePath & xmlSource);

    BOOL Execute();

    BOOL LoadGrammar(PVXMLGrammar * grammar);

    virtual BOOL PlayText(const PString & text, TextType type = Default, PINDEX repeat = 1, PINDEX delay = 0) = 0;
    virtual BOOL PlayFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0) = 0;
    virtual BOOL PlayData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0) = 0;
    virtual BOOL IsPlaying() const = 0;
    virtual BOOL IsRecording() const = 0;

    virtual void StartRecord(const PFilePath & recordfn, BOOL dtmfTerm, int maxTime, int finalSilence);

    virtual BOOL OnUserInput(char ch);

    PString GetXMLError() const;

    virtual BOOL OnEmptyAction()  { return TRUE; }

    virtual void ClearCall() { };

    virtual PString GetVar(const PString & str) const;
    virtual void SetVar(const PString & ostr, const PString & val);

  protected:
    BOOL ExecuteWithoutLock();

    PMutex sessionMutex;

    PXML xmlFile;
    PVXMLDialogArray dialogArray;

    PVXMLGrammar * activeGrammar;

    PStringToString sessionVars;
    PStringToString documentVars;

    BOOL recording;
    PFilePath recordFn;
    BOOL recordDTMFTerm;
    int recordMaxTime;
    int recordFinalSilence;
};

//////////////////////////////////////////////////////////////////

class PVXMLOutgoingChannel;

class PVXMLQueueItem : public PObject
{
  PCLASSINFO(PVXMLQueueItem, PObject);
  public:
    PVXMLQueueItem(PINDEX _repeat = 1, PINDEX _delay = 0)
      : repeat(_repeat), delay(_delay)
      { }

    virtual void Play(PVXMLOutgoingChannel & outgoingChannel) = 0;

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

    void Play(PVXMLOutgoingChannel & outgoingChannel);

  protected:
    PBYTEArray data;
};

class PVXMLQueueFilenameItem : public PVXMLQueueItem
{
  PCLASSINFO(PVXMLQueueFilenameItem, PObject);
  public:
    PVXMLQueueFilenameItem(const PFilePath & _fn, PINDEX repeat = 1, PINDEX delay = 0)
      : PVXMLQueueItem(repeat, delay), fn(_fn)
    { }

    void Play(PVXMLOutgoingChannel & outgoingChannel);

  protected:
    PFilePath fn;
};

PQUEUE(PVXMLQueue, PVXMLQueueItem);

//////////////////////////////////////////////////////////////////

class PVXMLChannel : public PIndirectChannel
{
  PCLASSINFO(PVXMLChannel, PIndirectChannel);
  public:
    PVXMLChannel(PVXMLSession & _vxml)
      : vxml(_vxml) { }

    virtual void DelayFrame(PINDEX msecs) = 0;

  protected:
    PVXMLSession & vxml;
    PMutex channelMutex;
    PAdaptiveDelay delay;
};

//////////////////////////////////////////////////////////////////

class PVXMLOutgoingChannel : public PVXMLChannel
{
  PCLASSINFO(PVXMLOutgoingChannel, PVXMLChannel);

  public:
    PVXMLOutgoingChannel(PVXMLSession & vxml);

    // overrides from PIndirectChannel
    virtual BOOL Read(void * buffer, PINDEX amount);
    BOOL Close();

    // new functions
    virtual PWAVFile * CreateWAVFile(const PFilePath & fn)
      { return new PWAVFile(fn, PFile::ReadOnly); }

    virtual BOOL AdjustFrame(void * buffer, PINDEX amount);
    virtual void QueueFile(const PString & fn, PINDEX repeat = 1, PINDEX delay = 0);
    virtual void QueueData(const PBYTEArray & data, PINDEX repeat = 1, PINDEX delay = 0);
    virtual void PlayFile(PFile * chan);
    virtual void FlushQueue();
    virtual BOOL IsPlaying() const   { return (playQueue.GetSize() > 0) || playing ; }

  protected:
    // new functions
    virtual BOOL ReadFrame(PINDEX amount)= 0;
    virtual void CreateSilenceFrame(PINDEX amount) = 0;

    PVXMLQueue playQueue;
    BOOL playing;

    PBYTEArray frameBuffer;
    PINDEX frameLen, frameOffs;

    int silentCount;
    int totalData;
    PTimer delayTimer;
};

//////////////////////////////////////////////////////////////////

class PVXMLIncomingChannel : public PVXMLChannel
{
  PCLASSINFO(PVXMLIncomingChannel, PVXMLChannel)

  public:
    PVXMLIncomingChannel(PVXMLSession & vxml);
    ~PVXMLIncomingChannel();

    // overrides from PIndirectChannel
    BOOL Write(const void * buf, PINDEX len);

    // new functions
    BOOL StartRecording(const PFilePath & fn);
    BOOL EndRecording();
    BOOL IsRecording() const { return wavFile != NULL; }

    virtual PWAVFile * CreateWAVFile(const PFilePath & fn)
      { return new PWAVFile(fn, PFile::WriteOnly, PFile::ModeDefault, PWAVFile::fmt_uLaw); }

  protected:
    // new functions
    virtual BOOL WriteFrame(const void * buf, PINDEX len) = 0;
    PWAVFile * wavFile;
};

#endif

