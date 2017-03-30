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
 */

#ifdef __GNUC__
#pragma implementation "ptts.h"
#endif

#include <ptlib.h>

#if P_TTS

#include <ptclib/ptts.h>

#include <ptlib/pipechan.h>

#if P_SAPI

// Following defined by both stdint.h and intsafe.h
#undef INT8_MIN
#undef INT16_MIN
#undef INT32_MIN
#undef INT64_MIN
#undef INT8_MAX
#undef UINT8_MAX
#undef INT16_MAX
#undef UINT16_MAX
#undef INT32_MAX
#undef UINT32_MAX
#undef INT64_MAX
#undef UINT64_MAX

#include <intsafe.h>

#include <ptlib/msos/ptlib/pt_atl.h>

////////////////////////////////////////////////////////////
//
// Text to speech using Microsoft's Speech API (SAPI)

#ifdef _MSC_VER
  #pragma comment(lib, "sapi.lib")
#endif

#include <sphelper.h>


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
    PString            m_CurrentVoice;
};

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_SAPI, "Microsoft SAPI", false);


#define new PNEW


PTextToSpeech_SAPI::PTextToSpeech_SAPI()
  : m_opened(false)
{
  PThread::Current()->CoInitialise();
  PTRACE(5, "SAPI-TTS", "Constructed");
}


PBoolean PTextToSpeech_SAPI::OpenChannel(PChannel *)
{
  Close();
  return false;
}


PBoolean PTextToSpeech_SAPI::OpenFile(const PFilePath & fn)
{
  Close();

  PComResult hr = m_cpVoice.CoCreateInstance(CLSID_SpVoice);
  if (hr.Failed()) {
    PTRACE(2, "SAPI-TTS", "Could not start SAPI: " << hr);
    return false;
  }

  CSpStreamFormat wavFormat;
  wavFormat.AssignFormat(SPSF_8kHz16BitMono);

  PWideString wfn = fn;
  hr = SPBindToFile(wfn, SPFM_CREATE_ALWAYS, &m_cpWavStream, &wavFormat.FormatId(), wavFormat.WaveFormatExPtr()); 
  if (hr.Failed()) {
    m_cpWavStream.Release();
    return false;
  }

  hr = m_cpVoice->SetOutput(m_cpWavStream, true);
  m_opened = hr.Succeeded();
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

  if (m_CurrentVoice != NULL && !m_CurrentVoice.IsEmpty()) {
    PTRACE(4, "SAPI-TTS", "Trying to set voice \"" << m_CurrentVoice << "\""
              " of voices: " << setfill(',') << GetVoiceList());

    //Enumerate voice tokens with attribute "Name=<specified voice>"
    CComPtr<IEnumSpObjectTokens> cpEnum;
    if (PCOM_SUCCEEDED(SpEnumTokens,(SPCAT_VOICES, m_CurrentVoice.AsUCS2(), NULL, &cpEnum))) {
      //Get the closest token
      CComPtr<ISpObjectToken> cpVoiceToken;
      if (PCOM_SUCCEEDED(cpEnum->Next,(1, &cpVoiceToken, NULL))) {
        //set the voice
        if (PCOM_SUCCEEDED(m_cpVoice->SetVoice,(cpVoiceToken))) {
          PTRACE(4, "SAPI-TTS", "SetVoice(" << m_CurrentVoice << ") OK!");
        }
      }
    } 
  }

  PTRACE(4, "SAPI-TTS", "Speaking...");
  return PCOM_SUCCEEDED(m_cpVoice->Speak,(wtext, SPF_DEFAULT, NULL));
}


PStringArray PTextToSpeech_SAPI::GetVoiceList()
{
  //Enumerate the available voices 
  PStringArray voiceList;

  CComPtr<IEnumSpObjectTokens> cpEnum;
  ULONG ulCount = 0;
  PComResult hr;

  // Get the number of voices
  if (PCOM_SUCCEEDED_EX(hr,SpEnumTokens,(SPCAT_VOICES, NULL, NULL, &cpEnum))) {
    if (PCOM_SUCCEEDED_EX(hr,cpEnum->GetCount,(&ulCount))) {
      PTRACE(4, "SAPI-TTS", "Found " << ulCount << " voices..");

      // Obtain a list of available voice tokens, set the voice to the token, and call Speak
      while (ulCount-- > 0) {
        CComPtr<ISpObjectToken> cpVoiceToken;
        if (hr.Succeeded(cpEnum->Next(1, &cpVoiceToken, NULL))) {
          voiceList.AppendString("voice");
          PTRACE(4, "SAPI-TTS", "Found voice:" << cpVoiceToken);
        }
      } 
    }
  }

  return voiceList;
}


PBoolean PTextToSpeech_SAPI::SetVoice(const PString & voice)
{
  m_CurrentVoice = voice;
  return true;
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


#if P_PIPECHAN

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
    PBoolean IsOpen()    { return m_opened; }

    PBoolean Close();
    PBoolean Speak(const PString & str, TextType hint);

  protected:
    PMutex    m_mutex;
    bool      m_opened;
    PString   m_text;
    PFilePath m_filePath;
    unsigned  m_volume;
    unsigned  m_sampleRate;
};

#define new PNEW

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_Festival, "Festival", false);


PTextToSpeech_Festival::PTextToSpeech_Festival()
  : m_opened(false)
  , m_volume(100)
  , m_sampleRate(8000)
{
  PTRACE(5, "Festival-TTS", "Constructed");
}


PTextToSpeech_Festival::~PTextToSpeech_Festival()
{
  PWaitAndSignal mutex(m_mutex);
  PTRACE(5, "Festival-TTS", "Destroyed");
}


PBoolean PTextToSpeech_Festival::OpenChannel(PChannel *)
{
  PWaitAndSignal mutex(m_mutex);

  Close();

  m_text.MakeEmpty();
  m_filePath.MakeEmpty();

  return false;
}


PBoolean PTextToSpeech_Festival::OpenFile(const PFilePath & fn)
{
  PWaitAndSignal mutex(m_mutex);

  Close();

  m_text.MakeEmpty();
  m_filePath = fn;
  m_opened = true;

  PTRACE(4, "Festival-TTS", "Writing speech to \"" << fn << '"');

  return true;
}


PBoolean PTextToSpeech_Festival::Close()
{
  PWaitAndSignal mutex(m_mutex);

  if (!m_opened)
    return true;

  m_opened = false;

  if (m_filePath.IsEmpty()) {
    PTRACE(1, "Festival-TTS", "Stream mode not supported (yet)");
    return false;
  }

  if (m_text.IsEmpty()) {
    PTRACE(1, "Festival-TTS", "Nothing spoken");
    return false;
  }

  PFile wav;
  if (!wav.Open(m_filePath, PFile::WriteOnly)) {
    PTRACE(1, "Festival-TTS", "Could not create WAV file: \"" << m_filePath << '"');
    return false;
  }

  PStringStream cmdLine;
  cmdLine << "text2wave -scale " << std::fixed << m_volume/100.0 << " -F " << m_sampleRate;

  PTRACE(4, "Festival-TTS", "Creating \"" << m_filePath << "\" from \"" << m_text << "\" using \"" << cmdLine << '"');
  PPipeChannel cmd(cmdLine, PPipeChannel::ReadWrite, true, true);

  cmd << m_text << '\n';
  if (!cmd.Execute()) { // Flushes stream and sends EOF
    PTRACE(1, "Festival-TTS", "Festival Generation failed: code=" << cmd.WaitForTermination());
    wav.Remove();
    return false;
  }

  char buf[1000];
  while (cmd.Read(buf, sizeof(buf))) {
    if (!wav.Write(buf, cmd.GetLastReadCount())) {
      PTRACE(1, "Festival-TTS", "Could not write to WAV file: \"" << m_filePath << '"');
      wav.Remove();
      return false;
    }
  }

  int result = cmd.WaitForTermination();

#if PTRACING
  PString error;
  if (cmd.ReadStandardError(error, false)) {
    PTRACE(2, "Festival-TTS", "Error: \"" << error.Trim() << '"');
    result = 1;
  }
  else if (result != 0) {
    PTRACE(2, "Festival-TTS", "Error from sub-process: result=" << result);
  }
  else {
    PTRACE(5, "Festival-TTS", "Generation complete: " << wav.GetLength() << " bytes");
  }
#endif

  return result == 0;
}


PBoolean PTextToSpeech_Festival::Speak(const PString & str, TextType hint)
{
  PWaitAndSignal mutex(m_mutex);

  if (!IsOpen()) {
    PTRACE(2, "Festival-TTS", "Attempt to speak whilst engine not open");
    return false;
  }

  if (m_filePath.IsEmpty()) {
    PTRACE(1, "Festival-TTS", "Stream mode not supported (yet)");
    return false;
  }

  PTRACE(4, "Festival-TTS", "Speaking \"" << str << "\", hint=" << hint);

  // do various things to the string, depending upon the hint
  switch (hint) {
    case Digits:
    default:
      m_text = m_text & str;
  }

  return true;
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
  m_sampleRate = v;
  return true;
}


unsigned PTextToSpeech_Festival::GetRate()
{
  return m_sampleRate;
}


PBoolean PTextToSpeech_Festival::SetVolume(unsigned v)
{
  m_volume = v;
  return true;
}


unsigned PTextToSpeech_Festival::GetVolume()
{
  return m_volume;
}


#endif // P_PIPECHAN

#endif // P_TTS
