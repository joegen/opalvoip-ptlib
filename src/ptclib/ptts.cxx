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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "ptts.h"
#endif

#include <ptlib.h>

#include "ptbuildopts.h"

#include <ptclib/ptts.h>

#include <ptlib/pipechan.h>
#include <ptclib/ptts.h>


#if P_SAPI

////////////////////////////////////////////////////////////
//
// Text to speech using Microsoft's Speech API (SAPI)
// Can be downloaded from http://www.microsoft.com/speech/download/sdk51
// It is also present in Windows Software Development Kit 6.0 and later.
//

#if defined(P_SAPI_LIBRARY)
  #pragma comment(lib, P_SAPI_LIBRARY)
#endif

#ifndef _WIN32_DCOM
  #define _WIN32_DCOM 1
#endif

#ifdef P_ATL

  #define _INTSAFE_H_INCLUDED_

#else

  // We are using express edition of MSVC which does not come with ATL support
  // So hand implement just enough for the SAPI code to work.
  #define __ATLBASE_H__

  #include <objbase.h>

  typedef WCHAR OLECHAR;
  typedef OLECHAR *LPOLESTR;
  typedef const OLECHAR *LPCOLESTR;
  typedef struct IUnknown IUnknown;
  typedef IUnknown *LPUNKNOWN;

  template <class T> class CComPtr
  {
    public:
      CComPtr(T * pp = NULL) : p(pp) { }
      ~CComPtr() { Release(); }

      T *  operator-> () const { return  PAssertNULL(p); }
           operator T*() const { return  PAssertNULL(p); }
      T &  operator*  () const { return *PAssertNULL(p); }
      T ** operator&  ()       { return &p; }
      bool operator!() const   { return (p == NULL); }
      bool operator<(__in_opt T* pT) const  { return p <  pT; }
      bool operator==(__in_opt T* pT) const { return p == pT; }
      bool operator!=(__in_opt T* pT) const { return p != pT; }

      void Attach(T * p2)
      {
        if (p)
	  p->Release();
        p = p2;
      }

      T * Detach()
      {
        T * pt = p;
        p = NULL;
        return pt;
      }

      void Release()
      {
        T * pt = p;
        if (pt != NULL) {
	  p = NULL;
	  pt->Release();
        }
      }

      __checkReturn HRESULT CoCreateInstance(__in REFCLSID rclsid, __in_opt LPUNKNOWN pUnkOuter = NULL, __in DWORD dwClsContext = CLSCTX_ALL)
      {
        return ::CoCreateInstance(rclsid, pUnkOuter, dwClsContext, __uuidof(T), (void**)&p);
      }

    private:
      T * p;
  };

#endif // P_ATL

#include <sphelper.h>


#define MAX_FN_SIZE 1024

class PTextToSpeech_SAPI : public PTextToSpeech
{
    PCLASSINFO(PTextToSpeech_SAPI, PTextToSpeech);
  public:
    PTextToSpeech_SAPI();

    // overrides
    PStringArray GetVoiceList();
    PBoolean SetVoice(const PString & voice);

    PBoolean SetRate(unsigned rate);
    unsigned GetRate();

    PBoolean SetVolume(unsigned volume);
    unsigned GetVolume();

    PBoolean OpenFile(const PFilePath & fn);
    PBoolean OpenChannel(PChannel * channel);
    PBoolean IsOpen()     { return m_opened; }

    PBoolean Close();
    PBoolean Speak(const PString & str, TextType hint);

  protected:
    CComPtr<ISpVoice>  m_cpVoice;
    CComPtr<ISpStream> m_cpWavStream;
    bool               m_opened;
};

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_SAPI, "Microsoft SAPI", false);


#define new PNEW


PTextToSpeech_SAPI::PTextToSpeech_SAPI()
  : m_opened(false)
{
  ::CoInitializeEx(NULL, COINIT_MULTITHREADED);
}


PBoolean PTextToSpeech_SAPI::OpenChannel(PChannel *)
{
  Close();
  return false;
}


PBoolean PTextToSpeech_SAPI::OpenFile(const PFilePath & fn)
{
  Close();

  HRESULT hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
  if (FAILED(hr))
    return false;

  CSpStreamFormat wavFormat;
  wavFormat.AssignFormat(SPSF_8kHz16BitMono);

  PWideString wfn = fn;
  hr = SPBindToFile(wfn, SPFM_CREATE_ALWAYS, &m_cpWavStream, &wavFormat.FormatId(), wavFormat.WaveFormatExPtr()); 
  if (FAILED(hr)) {
    m_cpWavStream.Release();
    return false;
  }

  hr = m_cpVoice->SetOutput(m_cpWavStream, true);
  m_opened = SUCCEEDED(hr);
  return m_opened;
}


PBoolean PTextToSpeech_SAPI::Close()
{
  if (!m_opened)
    return false;

  m_cpVoice->WaitUntilDone(INFINITE);
  m_cpWavStream.Release();
  m_cpVoice.Release();

  m_opened = false;
  return true;
}


PBoolean PTextToSpeech_SAPI::Speak(const PString & text, TextType hint)
{
  if (!IsOpen())
    return false;

  PWideString wtext = text;

  // do various things to the string, depending upon the hint
  switch (hint) {
    case Digits:
      break;
    default:
      break;
  };

  HRESULT hr = m_cpVoice->Speak(wtext, SPF_DEFAULT, NULL);
  if (SUCCEEDED(hr))
    return true;

  PTRACE(4, "SAPI\tError speaking text: " << hr);
  return false;
}


PStringArray PTextToSpeech_SAPI::GetVoiceList()
{
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

    if (SUCCEEDED(hr))
      voiceList.AppendString("voice");
  } 

  return voiceList;
}

PBoolean PTextToSpeech_SAPI::SetVoice(const PString &)
{
  return false;
}

PBoolean PTextToSpeech_SAPI::SetRate(unsigned)
{
  return false;
}

unsigned PTextToSpeech_SAPI::GetRate()
{
  return 0;
}

PBoolean PTextToSpeech_SAPI::SetVolume(unsigned)
{
  return false;
}

unsigned PTextToSpeech_SAPI::GetVolume()
{
  return 0;
}

#endif // P_SAPI


#ifndef _WIN32_WCE

////////////////////////////////////////////////////////////
//
//  Generic text to speech using Festival
//

#undef new

class PTextToSpeech_Festival : public PTextToSpeech
{
  PCLASSINFO(PTextToSpeech_Festival, PTextToSpeech);
  public:
    PTextToSpeech_Festival();
    ~PTextToSpeech_Festival();

    // overrides
    PStringArray GetVoiceList();
    PBoolean SetVoice(const PString & voice);

    PBoolean SetRate(unsigned rate);
    unsigned GetRate();

    PBoolean SetVolume(unsigned volume);
    unsigned GetVolume();

    PBoolean OpenFile(const PFilePath & fn);
    PBoolean OpenChannel(PChannel * channel);
    PBoolean IsOpen()    { return opened; }

    PBoolean Close();
    PBoolean Speak(const PString & str, TextType hint);

  protected:
    PBoolean Invoke(const PString & str, const PFilePath & fn);

    PMutex mutex;
    PBoolean opened;
    PBoolean usingFile;
    PString text;
    PFilePath path;
    unsigned volume, rate;
};

#define new PNEW

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_Festival, "Festival", false);

PTextToSpeech_Festival::PTextToSpeech_Festival()
{
  PWaitAndSignal m(mutex);
  usingFile = opened = false;
  rate = 8000;
  volume = 100;
}


PTextToSpeech_Festival::~PTextToSpeech_Festival()
{
  PWaitAndSignal m(mutex);
}

PBoolean PTextToSpeech_Festival::OpenChannel(PChannel *)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = false;
  opened = false;

  return true;
}


PBoolean PTextToSpeech_Festival::OpenFile(const PFilePath & fn)
{
  PWaitAndSignal m(mutex);

  Close();
  usingFile = true;
  path = fn;
  opened = true;

  PTRACE(3, "TTS\tWriting speech to " << fn);

  return true;
}

PBoolean PTextToSpeech_Festival::Close()
{
  PWaitAndSignal m(mutex);

  if (!opened)
    return true;

  PBoolean stat = false;

  if (usingFile)
    stat = Invoke(text, path);

  text = PString();

  opened = false;

  return stat;
}


PBoolean PTextToSpeech_Festival::Speak(const PString & ostr, TextType hint)
{
  PWaitAndSignal m(mutex);

  if (!IsOpen()) {
    PTRACE(2, "TTS\tAttempt to speak whilst engine not open");
    return false;
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
    return true;
  }

  PTRACE(1, "TTS\tStream mode not supported for Festival");

  return false;
}

PStringArray PTextToSpeech_Festival::GetVoiceList()
{
  PStringArray voiceList;
  voiceList.AppendString("default");
  return voiceList;
}

PBoolean PTextToSpeech_Festival::SetVoice(const PString & v)
{
  return v == "default";
}

PBoolean PTextToSpeech_Festival::SetRate(unsigned v)
{
  rate = v;
  return true;
}

unsigned PTextToSpeech_Festival::GetRate()
{
  return rate;
}

PBoolean PTextToSpeech_Festival::SetVolume(unsigned v)
{
  volume = v;
  return true;
}

unsigned PTextToSpeech_Festival::GetVolume()
{
  return volume;
}

PBoolean PTextToSpeech_Festival::Invoke(const PString & otext, const PFilePath & fname)
{
  PString text = otext;
  text.Replace('\n', ' ', true);
  text.Replace('\"', '\'', true);
  text.Replace('\\', ' ', true);
  text = "\"" + text + "\"";

  PString cmdLine = "echo " + text + " | ./text2wave -F " + PString(PString::Unsigned, rate) + " -otype riff > " + fname;

#if 1

  return system(cmdLine) != -1;

#else

  PPipeChannel cmd;
  int code = -1;
  if (!cmd.Open(cmdLine, PPipeChannel::ReadWriteStd)) {
    PTRACE(1, "TTS\tCannot execute command " << cmd);
  } else {
    PTRACE(3, "TTS\tCreating " << fname << " using " << cmdLine);
    cmd.Execute();
    code = cmd.WaitForTermination();
    if (code >= 0) {
      PTRACE(4, "TTS\tdata generated");
    } else {
      PTRACE(1, "TTS\tgeneration failed");
    }
  }

  return code == 0;

#endif
}


#endif


// End Of File ///////////////////////////////////////////////////////////////
