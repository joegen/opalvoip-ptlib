/*
 * ptts.h
 *
 * Text To Speech classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * $Log: ptts.h,v $
 * Revision 1.3  2002/11/06 22:47:24  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.2  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.1  2002/08/06 04:45:38  craigs
 * Initial version
 *
 */

#ifndef _PTEXTTOSPEECH
#define _PTEXTTOSPEECH

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptclib/ptts.h>

class PTextToSpeech_Base : public PObject
{
  PCLASSINFO(PTextToSpeech_Base, PObject);
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

    virtual PStringArray GetVoiceList() = 0;
    virtual BOOL SetVoice(const PString & voice) = 0;

    virtual BOOL SetRate(unsigned rate) = 0;
    virtual unsigned GetRate() = 0;

    virtual BOOL SetVolume(unsigned volume) = 0;
    virtual unsigned GetVolume() = 0;

    virtual BOOL OpenFile   (const PFilePath & fn) = 0;
    virtual BOOL OpenChannel(PChannel * chanel) = 0;
    virtual BOOL IsOpen() = 0;

    virtual BOOL Close      () = 0;
    virtual BOOL Speak      (const PString & text, TextType hint = Default) = 0;
};

class PTextToSpeechEngine : public PTextToSpeech_Base
{
  PCLASSINFO(PTextToSpeechEngine, PTextToSpeech_Base);
  public:
  protected:
};

typedef PTextToSpeechEngine * PTextToSpeechEngineCreator();

class PTextToSpeechEngineDef : public PObject
{
  PCLASSINFO(PTextToSpeechEngineDef, PObject);
  public:
    PTextToSpeechEngineDef(PTextToSpeechEngineCreator * _creator)
      : creator(_creator) { }

    PTextToSpeechEngineCreator * creator;
};

PDICTIONARY(PTextToSpeechEngineDict, PString, PTextToSpeechEngineDef);

class PTextToSpeech : public PTextToSpeech_Base
{
  PCLASSINFO(PTextToSpeech, PTextToSpeech_Base);
  public:
    PTextToSpeech();
    ~PTextToSpeech();

    // overrides
    PStringArray GetVoiceList();
    BOOL SetVoice(const PString & voice);

    BOOL SetRate(unsigned rate);
    unsigned GetRate();

    BOOL SetVolume(unsigned volume);
    unsigned GetVolume();

    BOOL OpenFile   (const PFilePath & fn);
    BOOL OpenChannel(PChannel * chanel);
    BOOL IsOpen();

    BOOL Close      ();
    BOOL Speak      (const PString & text, TextType hint = Default);

    // new functions
    void RegisterEngine(const PString & engineName, PTextToSpeechEngineDef * engineDef);
    void UnregisterEngine(const PString & engineName);

    static PStringArray GetEngines();
    BOOL SetEngine(const PString & format);

  protected:
    static PMutex engineMutex;
    static PTextToSpeechEngineDict * engineDict;

    PMutex mutex;
    PTextToSpeechEngine * engine;

    unsigned volume;
    unsigned rate;
};

#endif
