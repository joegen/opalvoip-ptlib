/*
 * ptts.cxx
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
 * $Log: ptts.cxx,v $
 * Revision 1.5  2003/04/17 01:21:33  craigs
 * Added import of pybuildopts to correctly detect if P_SAPI is set
 *
 * Revision 1.4  2003/04/16 08:00:19  robertj
 * Windoes psuedo autoconf support
 *
 * Revision 1.3  2002/11/06 22:47:25  robertj
 * Fixed header comment (copyright etc)
 *
 * Revision 1.2  2002/08/14 15:18:25  craigs
 * Fixed Festval implementation
 *
 * Revision 1.1  2002/08/06 04:45:58  craigs
 * Initial version
 *
 */

#ifdef __GNUC__
#pragma implementation "ptts.h"
#endif

#include "ptbuildopts.h"

////////////////////////////////////////////////////////////

// WIN32 COM stuff must be first in file to compile properly

#if P_SAPI

#ifndef _WIN32_DCOM
#define _WIN32_DCOM 1
#endif

#include <objbase.h>
#include <atlbase.h>
#include <objbase.h>
#include <windows.h>
#include <windowsx.h>
#include <sphelper.h>

#endif

////////////////////////////////////////////////////////////

#include <ptlib.h>
#include <ptlib/pipechan.h>
#include <ptclib/ptts.h>

PMutex PTextToSpeech::engineMutex;
PTextToSpeechEngineDict * PTextToSpeech::engineDict = NULL;

////////////////////////////////////////////////////////////
//
// Text to speech using Microsoft's Speech API (SAPI)
// Can be downloaded from http://www.microsoft.com/speech/download/sdk51
//

#if P_SAPI

#define MAX_FN_SIZE 1024

class PTextToSpeech_SAPI : public PTextToSpeechEngine
{
  PCLASSINFO(PTextToSpeech_SAPI, PTextToSpeechEngine);
  public:
    PTextToSpeech_SAPI();
    ~PTextToSpeech_SAPI();

    // overrides
    PStringArray GetVoiceList();
    BOOL SetVoice(const PString & voice);

    BOOL SetRate(unsigned rate);
    unsigned GetRate();

    BOOL SetVolume(unsigned volume);
    unsigned GetVolume();

    BOOL OpenFile   (const PFilePath & fn);
    BOOL OpenChannel(PChannel * channel);
    BOOL IsOpen()     { return opened; }

    BOOL Close      ();
    BOOL Speak      (const PString & str, TextType hint);

  protected:
    BOOL OpenVoice();

    static PMutex refMutex;
    static int * refCount;

    PMutex mutex;
    CComPtr<ISpVoice> m_cpVoice;
    CComPtr<ISpStream> cpWavStream;
    BOOL opened;
    BOOL usingFile;
    unsigned rate, volume;
    PString voice;
};

int * PTextToSpeech_SAPI::refCount;
PMutex PTextToSpeech_SAPI::refMutex;

static PTextToSpeechEngine * PTextToSpeech_SAPI_Creator()
{
  return new PTextToSpeech_SAPI;
}

PTextToSpeech_SAPI::PTextToSpeech_SAPI()
{
  PWaitAndSignal m(refMutex);

  if (refCount == NULL) {
    refCount = new int;
    *refCount = 1;
    ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
  } else {
    (*refCount)++;
  }

  usingFile = opened = FALSE;
}


PTextToSpeech_SAPI::~PTextToSpeech_SAPI()
{
  PWaitAndSignal m(refMutex);

  if ((--(*refCount)) == 0) {
    ::CoUninitialize();
    delete refCount;
    refCount = NULL;
  }
}

BOOL PTextToSpeech_SAPI::OpenVoice()
{
  PWaitAndSignal m(mutex);

  HRESULT hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
  return (opened = SUCCEEDED(hr));
}

BOOL PTextToSpeech_SAPI::OpenChannel(PChannel *)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = FALSE;
  return (opened = FALSE);
}


BOOL PTextToSpeech_SAPI::OpenFile(const PFilePath & fn)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = TRUE;

  if (!OpenVoice())
    return FALSE;

  CSpStreamFormat wavFormat;
  wavFormat.AssignFormat(SPSF_8kHz16BitMono);

  WCHAR szwWavFileName[MAX_FN_SIZE] = L"";;

  USES_CONVERSION;
  wcscpy(szwWavFileName, T2W((const char *)fn));
  HRESULT hr = SPBindToFile(szwWavFileName, SPFM_CREATE_ALWAYS, &cpWavStream, &wavFormat.FormatId(), wavFormat.WaveFormatExPtr()); 

  if (!SUCCEEDED(hr)) {
    cpWavStream.Release();
    return FALSE;
  }

	hr = m_cpVoice->SetOutput(cpWavStream, TRUE);

  return (opened = SUCCEEDED(hr));
}

BOOL PTextToSpeech_SAPI::Close()
{
  PWaitAndSignal m(mutex);

  if (!opened)
    return TRUE;

  if (usingFile) {
    if (opened)
      m_cpVoice->WaitUntilDone(INFINITE);
    cpWavStream.Release();
  }

  if (opened)
    m_cpVoice.Release();

  opened = FALSE;

  return TRUE;
}


BOOL PTextToSpeech_SAPI::Speak(const PString & otext, TextType hint)
{
  PWaitAndSignal m(mutex);

  if (!IsOpen())
    return FALSE;

  PString text = otext;

  // do various things to the string, depending upon the hint
  switch (hint) {
    case Digits:
      {
      }
      break;

    default:
    ;
  };

  // quick hack to calculate length of Unicode string
  unsigned short * uStr = new unsigned short[text.GetLength()+1];

  USES_CONVERSION;
  wcscpy(uStr, T2W((const char *)text));

  HRESULT hr = m_cpVoice->Speak(uStr, SPF_DEFAULT, NULL);

  delete[] uStr;

  return SUCCEEDED(hr);
}

PStringArray PTextToSpeech_SAPI::GetVoiceList()
{
  PWaitAndSignal m(mutex);

  PStringArray voiceList;

  CComPtr<ISpObjectToken> cpVoiceToken;
  CComPtr<IEnumSpObjectTokens> cpEnum;
  ULONG ulCount = 0;

  //Enumerate the available voices 
  HRESULT hr = SpEnumTokens(SPCAT_VOICES, NULL, NULL, &cpEnum);

  // Get the number of voices
  if (SUCCEEDED(hr))
    hr = cpEnum->GetCount(&ulCount);

  // Obtain a list of available voice tokens, set the voice to the token, and call Speak
  while (SUCCEEDED(hr) && ulCount--) {

    cpVoiceToken.Release();

    if (SUCCEEDED(hr))
      hr = cpEnum->Next(1, &cpVoiceToken, NULL );

    if (SUCCEEDED(hr)) {
      voiceList.AppendString("voice");
    }
  } 

  return voiceList;
}

BOOL PTextToSpeech_SAPI::SetVoice(const PString & v)
{
  PWaitAndSignal m(mutex);
  voice = v;
  return TRUE;
}

BOOL PTextToSpeech_SAPI::SetRate(unsigned v)
{
  rate = v;
  return TRUE;
}

unsigned PTextToSpeech_SAPI::GetRate()
{
  return rate;
}

BOOL PTextToSpeech_SAPI::SetVolume(unsigned v)
{
  volume = v;
  return TRUE;
}

unsigned PTextToSpeech_SAPI::GetVolume()
{
  return volume;
}

#endif
// P_SAPI

////////////////////////////////////////////////////////////
//
//  Generic text to speech using Festival
//

class PTextToSpeech_Festival : public PTextToSpeechEngine
{
  PCLASSINFO(PTextToSpeech_Festival, PTextToSpeechEngine);
  public:
    PTextToSpeech_Festival();
    ~PTextToSpeech_Festival();

    // overrides
    PStringArray GetVoiceList();
    BOOL SetVoice(const PString & voice);

    BOOL SetRate(unsigned rate);
    unsigned GetRate();

    BOOL SetVolume(unsigned volume);
    unsigned GetVolume();

    BOOL OpenFile   (const PFilePath & fn);
    BOOL OpenChannel(PChannel * channel);
    BOOL IsOpen()    { return opened; }

    BOOL Close      ();
    BOOL Speak      (const PString & str, TextType hint);

  protected:
    BOOL Invoke(const PString & str, const PFilePath & fn);

    PMutex mutex;
    BOOL opened;
    BOOL usingFile;
    PString text;
    PFilePath path;
    unsigned volume, rate;
    PString voice;
};

static PTextToSpeechEngine * PTextToSpeech_Festival_Creator()
{
  return new PTextToSpeech_Festival;
}

PTextToSpeech_Festival::PTextToSpeech_Festival()
{
  PWaitAndSignal m(mutex);
  usingFile = opened = FALSE;
}


PTextToSpeech_Festival::~PTextToSpeech_Festival()
{
  PWaitAndSignal m(mutex);
}

BOOL PTextToSpeech_Festival::OpenChannel(PChannel *)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = FALSE;
  opened = FALSE;

  return TRUE;
}


BOOL PTextToSpeech_Festival::OpenFile(const PFilePath & fn)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = TRUE;
  path = fn;
  opened = TRUE;

  PTRACE(3, "TTS\tWriting speech to " << fn);

  return TRUE;
}

BOOL PTextToSpeech_Festival::Close()
{
  PWaitAndSignal m(mutex);

  if (!opened)
    return TRUE;

  BOOL stat = FALSE;

  if (usingFile)
    stat = Invoke(text, path);

  text = PString();

  opened = FALSE;

  return stat;
}


BOOL PTextToSpeech_Festival::Speak(const PString & ostr, TextType hint)
{
  PWaitAndSignal m(mutex);

  if (!IsOpen()) {
    PTRACE(3, "TTS\tAttempt to speak whilst engine not open");
    return FALSE;
  }

  PString str = ostr;

  // do various things to the string, depending upon the hint
  switch (hint) {
    case Digits:
    default:
    ;
  };

  if (usingFile) {
    PTRACE(3, "TTS\tSpeaking " << ostr);
    text = text & str;
    return TRUE;
  }

  PTRACE(3, "TTS\tStream mode not supported for Festival");

  return FALSE;
}

PStringArray PTextToSpeech_Festival::GetVoiceList()
{
  PWaitAndSignal m(mutex);

  PStringArray voiceList;

  voiceList.AppendString("default");

  return voiceList;
}

BOOL PTextToSpeech_Festival::SetVoice(const PString & v)
{
  PWaitAndSignal m(mutex);
  voice = v;
  return TRUE;
}

BOOL PTextToSpeech_Festival::SetRate(unsigned v)
{
  rate = v;
  return TRUE;
}

unsigned PTextToSpeech_Festival::GetRate()
{
  return rate;
}

BOOL PTextToSpeech_Festival::SetVolume(unsigned v)
{
  volume = v;
  return TRUE;
}

unsigned PTextToSpeech_Festival::GetVolume()
{
  return volume;
}

BOOL PTextToSpeech_Festival::Invoke(const PString & otext, const PFilePath & fname)
{
  PString text = otext;
  text.Replace('\n', ' ', TRUE);
  text.Replace('\"', '\'', TRUE);
  text.Replace('\\', ' ', TRUE);
  text = "\"" + text + "\"";

  PString cmdLine = "echo " + text + " | ./text2wave -F " + PString(PString::Unsigned, rate) + " -otype riff > " + fname;

#if 1

  system(cmdLine);
  return TRUE;

#else

  PPipeChannel cmd;
  int code = -1;
  if (!cmd.Open(cmdLine, PPipeChannel::ReadWriteStd)) {
    PTRACE(2, "TTS\tCannot execute command " << cmd);
  } else {
    PTRACE(2, "TTS\tCreating " << fname << " using " << cmdLine);
    cmd.Execute();
    code = cmd.WaitForTermination();
    if (code >= 0) {
      PTRACE(2, "TTS\tdata generated");
    } else {
      PTRACE(2, "TTS\tgeneration failed");
    }
  }

  return code == 0;

#endif
}

////////////////////////////////////////////////////////////
//
//  Text to speech for underlying engines
//

PTextToSpeech::PTextToSpeech()
{
  PWaitAndSignal m(engineMutex);

  if (engineDict == NULL) {
    engineDict =  new PTextToSpeechEngineDict;

    // register known engines
#if P_SAPI
    RegisterEngine("Microsoft SAPI", new PTextToSpeechEngineDef(PTextToSpeech_SAPI_Creator));
#endif
    RegisterEngine("Festival",       new PTextToSpeechEngineDef(PTextToSpeech_Festival_Creator));
  }

  engine = NULL;

  SetVolume(100);
  SetRate(8000);
}

PTextToSpeech::~PTextToSpeech()
{
  Close();
  if (engine != NULL)
    delete engine;
}

void PTextToSpeech::RegisterEngine(const PString & engineName, PTextToSpeechEngineDef * engineDef)
{
  PWaitAndSignal m(engineMutex);
  engineDict->SetAt(engineName, engineDef);
}

void PTextToSpeech::UnregisterEngine(const PString & engineName)
{
  PWaitAndSignal m(engineMutex);
  engineDict->SetAt(engineName, NULL);
}

PStringArray PTextToSpeech::GetEngines()
{
  PWaitAndSignal m(engineMutex);
  PStringArray engines;
  PINDEX i;
  for (i = 0; i < engineDict->GetSize(); i++) {
    engines.AppendString(engineDict->GetKeyAt(i));
  }

  return engines;
}

BOOL PTextToSpeech::SetEngine(const PString & format)
{
  PWaitAndSignal m2(mutex);
  if (engine != NULL) {
    delete engine;
    engine = NULL;
  }

  PWaitAndSignal m(engineMutex);
  PStringArray engines;
  PINDEX i;
  for (i = 0; i < engineDict->GetSize(); i++) {
    PString key = engineDict->GetKeyAt(i);
    if (key *= format) {
      //int val = (*(*engineDict)[key].creator)();
      PTextToSpeechEngineCreator * creator = (*engineDict)[key].creator;
      if (creator != NULL) {
        engine = creator();
        if (engine != NULL) {
          engine->SetRate(rate);
          engine->SetVolume(volume);
          return TRUE;
        }
      }
    }
  }

  return FALSE;
}

#define TTS_FUNCTION(name) \
{ \
  PWaitAndSignal m(mutex); \
  if (engine == NULL) \
    return FALSE; \
  return engine->name(); \
} \

#define TTS_FUNCTION_PARM(name, parm) \
{ \
  PWaitAndSignal m(mutex); \
  if (engine == NULL) \
    return FALSE; \
  return engine->name(parm); \
} \

#define TTS_FUNCTION_PARM2(name, parm1, parm2) \
{ \
  PWaitAndSignal m(mutex); \
  if (engine == NULL) \
    return FALSE; \
  return engine->name(parm1, parm2); \
} \

#define TTS_FUNCTION_SET_MIRROR_PARM(name, parm) \
{ \
  PWaitAndSignal m(mutex); \
  parm = _##parm; \
  if (engine == NULL) \
    return TRUE; \
  return engine->name(_##parm); \
} \

#define TTS_FUNCTION_GET_MIRROR_PARM(name, parm) \
{ \
  PWaitAndSignal m(mutex); \
  if (engine == NULL) \
    return engine->name(); \
  return parm; \
} \

BOOL PTextToSpeech::IsOpen() 
TTS_FUNCTION(IsOpen)

BOOL PTextToSpeech::Close() 
TTS_FUNCTION(Close)

BOOL PTextToSpeech::OpenFile(const PFilePath & fn)
TTS_FUNCTION_PARM(OpenFile, fn)

BOOL PTextToSpeech::OpenChannel(PChannel * channel)
TTS_FUNCTION_PARM(OpenChannel, channel)

BOOL PTextToSpeech::Speak(const PString & text, TextType hint)
TTS_FUNCTION_PARM2(Speak, text, hint)

PStringArray PTextToSpeech::GetVoiceList()
TTS_FUNCTION(GetVoiceList)

BOOL PTextToSpeech::SetVoice(const PString & voice)
TTS_FUNCTION_PARM(SetVoice, voice)

BOOL PTextToSpeech::SetRate(unsigned _rate)
TTS_FUNCTION_SET_MIRROR_PARM(SetRate, rate)

unsigned PTextToSpeech::GetRate()
TTS_FUNCTION_GET_MIRROR_PARM(GetRate, rate)

BOOL PTextToSpeech::SetVolume(unsigned _volume)
TTS_FUNCTION_SET_MIRROR_PARM(SetVolume, volume)

unsigned PTextToSpeech::GetVolume()
TTS_FUNCTION_GET_MIRROR_PARM(GetVolume, volume)


// End Of File ///////////////////////////////////////////////////////////////
