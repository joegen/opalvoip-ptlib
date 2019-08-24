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
    ~PTextToSpeech_SAPI();

    // overrides
    PStringArray GetVoiceList();
    PBoolean SetVoice(const PString & voice);

    PBoolean SetSampleRate(unsigned rate);
    unsigned GetSampleRate();

    PBoolean SetChannels(unsigned channels);
    unsigned GetChannels();

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
    SPSTREAMFORMAT     m_spsfFormat;
    PString            m_CurrentVoice;
};

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_SAPI, "Microsoft SAPI", false);


#define new PNEW
#define PTraceModule() "SAPI-TTS"

PTextToSpeech_SAPI::PTextToSpeech_SAPI()
  : m_opened(false)
  , m_spsfFormat(SPSF_8kHz16BitMono)
{
  PThread::Current()->CoInitialise();
  PTRACE(4, "Constructed " << this);
}


PTextToSpeech_SAPI::~PTextToSpeech_SAPI()
{
  PTRACE(4, "Destroyed " << this);
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
    PTRACE(2, "Could not start SAPI: " << hr);
    return false;
  }

  CSpStreamFormat wavFormat;
  wavFormat.AssignFormat(m_spsfFormat);

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
    PTRACE(4, "Trying to set voice \"" << m_CurrentVoice << "\""
              " of voices: " << setfill(',') << GetVoiceList());

    //Enumerate voice tokens with attribute "Name=<specified voice>"
    CComPtr<IEnumSpObjectTokens> cpEnum;
    if (PCOM_SUCCEEDED(SpEnumTokens,(SPCAT_VOICES, m_CurrentVoice.AsUCS2(), NULL, &cpEnum))) {
      //Get the closest token
      CComPtr<ISpObjectToken> cpVoiceToken;
      if (PCOM_SUCCEEDED(cpEnum->Next,(1, &cpVoiceToken, NULL))) {
        //set the voice
        if (PCOM_SUCCEEDED(m_cpVoice->SetVoice,(cpVoiceToken))) {
          PTRACE(4, "SetVoice(" << m_CurrentVoice << ") OK!");
        }
      }
    } 
  }

  PTRACE(4, "Speaking...");
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
      PTRACE(4, "Found " << ulCount << " voices..");

      // Obtain a list of available voice tokens, set the voice to the token, and call Speak
      while (ulCount-- > 0) {
        CComPtr<ISpObjectToken> cpVoiceToken;
        if (hr.Succeeded(cpEnum->Next(1, &cpVoiceToken, NULL))) {
          PWSTR pDescription = NULL;
          SpGetDescription(cpVoiceToken, &pDescription);
          PWideString desc(pDescription);
          CoTaskMemFree(pDescription);
          voiceList.AppendString(desc);
          PTRACE(4, "Found voice: " << desc);
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


PBoolean PTextToSpeech_SAPI::SetSampleRate(unsigned rate)
{
  switch (rate) {
    case 8000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_8kHz16BitStereo : SPSF_8kHz16BitMono;
      return true;
    case 11000 :
    case 11025 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_11kHz16BitStereo : SPSF_11kHz16BitMono;
      return true;
    case 12000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_12kHz16BitStereo : SPSF_12kHz16BitMono;
      return true;
    case 16000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_16kHz16BitStereo : SPSF_16kHz16BitMono;
      return true;
    case 22000 :
    case 22050 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_22kHz16BitStereo : SPSF_22kHz16BitMono;
      return true;
    case 24000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_24kHz16BitStereo : SPSF_24kHz16BitMono;
      return true;
    case 32000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_32kHz16BitStereo : SPSF_32kHz16BitMono;
      return true;
    case 44000 :
    case 44100 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_44kHz16BitStereo : SPSF_44kHz16BitMono;
      return true;
    case 48000 :
      m_spsfFormat = GetChannels() > 1 ? SPSF_8kHz16BitStereo : SPSF_48kHz16BitMono;
      return true;
  }
  return false;
}

unsigned PTextToSpeech_SAPI::GetSampleRate()
{
  switch (m_spsfFormat) {
    case SPSF_8kHz16BitMono :
    case SPSF_8kHz16BitStereo :
      return 8000;
    case SPSF_11kHz16BitMono :
    case SPSF_11kHz16BitStereo :
      return 11025;
    case SPSF_12kHz16BitMono :
    case SPSF_12kHz16BitStereo :
      return 12000;
    case SPSF_16kHz16BitMono :
    case SPSF_16kHz16BitStereo :
      return 16000;
    case SPSF_22kHz16BitMono :
    case SPSF_22kHz16BitStereo :
      return 22050;
    case SPSF_24kHz16BitMono :
    case SPSF_24kHz16BitStereo :
      return 24000;
    case SPSF_32kHz16BitMono :
    case SPSF_32kHz16BitStereo :
      return 32000;
    case SPSF_44kHz16BitMono :
    case SPSF_44kHz16BitStereo :
      return 44100;
    case SPSF_48kHz16BitMono :
    case SPSF_48kHz16BitStereo :
      return 48000;
  }
  return 0;
}

PBoolean PTextToSpeech_SAPI::SetChannels(unsigned channels)
{
  switch (channels) {
    case 1:
      switch (m_spsfFormat) {
        case SPSF_8kHz16BitMono :
        case SPSF_11kHz16BitMono :
        case SPSF_12kHz16BitMono :
        case SPSF_16kHz16BitMono :
        case SPSF_22kHz16BitMono :
        case SPSF_24kHz16BitMono :
        case SPSF_32kHz16BitMono :
        case SPSF_44kHz16BitMono :
        case SPSF_48kHz16BitMono :
          return true;
        case SPSF_8kHz16BitStereo :
          m_spsfFormat = SPSF_8kHz16BitMono;
          return true;
        case SPSF_11kHz16BitStereo :
          m_spsfFormat = SPSF_11kHz16BitMono;
          return true;
        case SPSF_12kHz16BitStereo :
          m_spsfFormat = SPSF_12kHz16BitMono;
          return true;
        case SPSF_16kHz16BitStereo :
          m_spsfFormat = SPSF_16kHz16BitMono;
          return true;
        case SPSF_22kHz16BitStereo :
          m_spsfFormat = SPSF_22kHz16BitMono;
          return true;
        case SPSF_24kHz16BitStereo :
          m_spsfFormat = SPSF_24kHz16BitMono;
          return true;
        case SPSF_32kHz16BitStereo :
          m_spsfFormat = SPSF_16kHz16BitMono;
          return true;
        case SPSF_44kHz16BitStereo :
          m_spsfFormat = SPSF_44kHz16BitMono;
          return true;
        case SPSF_48kHz16BitStereo :
          m_spsfFormat = SPSF_48kHz16BitMono;
          return true;
      }
      break;

    case 2:
      switch (m_spsfFormat) {
        case SPSF_8kHz16BitMono :
          m_spsfFormat = SPSF_8kHz16BitStereo;
          return true;
        case SPSF_11kHz16BitMono :
          m_spsfFormat = SPSF_11kHz16BitStereo;
          return true;
        case SPSF_12kHz16BitMono :
          m_spsfFormat = SPSF_12kHz16BitStereo;
          return true;
        case SPSF_16kHz16BitMono :
          m_spsfFormat = SPSF_16kHz16BitStereo;
          return true;
        case SPSF_22kHz16BitMono :
          m_spsfFormat = SPSF_22kHz16BitStereo;
          return true;
        case SPSF_24kHz16BitMono :
          m_spsfFormat = SPSF_24kHz16BitStereo;
          return true;
        case SPSF_32kHz16BitMono :
          m_spsfFormat = SPSF_32kHz16BitStereo;
          return true;
        case SPSF_44kHz16BitMono :
          m_spsfFormat = SPSF_44kHz16BitStereo;
          return true;
        case SPSF_48kHz16BitMono :
          m_spsfFormat = SPSF_48kHz16BitStereo;
          return true;
        case SPSF_8kHz16BitStereo :
        case SPSF_11kHz16BitStereo :
        case SPSF_12kHz16BitStereo :
        case SPSF_16kHz16BitStereo :
        case SPSF_22kHz16BitStereo :
        case SPSF_24kHz16BitStereo :
        case SPSF_32kHz16BitStereo :
        case SPSF_44kHz16BitStereo :
        case SPSF_48kHz16BitStereo :
          return true;
      }
  }
  return false;
}

unsigned PTextToSpeech_SAPI::GetChannels()
{
  switch (m_spsfFormat) {
    case SPSF_8kHz16BitMono :
    case SPSF_11kHz16BitMono :
    case SPSF_12kHz16BitMono :
    case SPSF_16kHz16BitMono :
    case SPSF_22kHz16BitMono :
    case SPSF_24kHz16BitMono :
    case SPSF_32kHz16BitMono :
    case SPSF_44kHz16BitMono :
    case SPSF_48kHz16BitMono :
      return 1;
    case SPSF_8kHz16BitStereo :
    case SPSF_11kHz16BitStereo :
    case SPSF_12kHz16BitStereo :
    case SPSF_16kHz16BitStereo :
    case SPSF_22kHz16BitStereo :
    case SPSF_24kHz16BitStereo :
    case SPSF_32kHz16BitStereo :
    case SPSF_44kHz16BitStereo :
    case SPSF_48kHz16BitStereo :
      return 2;
  }
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
#undef PTraceModule

class PTextToSpeech_Festival : public PTextToSpeech
{
  PCLASSINFO(PTextToSpeech_Festival, PTextToSpeech);
  public:
    PTextToSpeech_Festival();
    ~PTextToSpeech_Festival();

    // overrides
    PStringArray GetVoiceList();
    PBoolean SetVoice(const PString & voice);

    PBoolean SetSampleRate(unsigned rate);
    unsigned GetSampleRate();

    PBoolean SetChannels(unsigned channels);
    unsigned GetChannels();

    PBoolean SetVolume(unsigned volume);
    unsigned GetVolume();

    PBoolean OpenFile(const PFilePath & fn);
    PBoolean OpenChannel(PChannel * channel);
    PBoolean IsOpen()    { return m_opened; }

    PBoolean Close();
    PBoolean Speak(const PString & str, TextType hint);

  protected:
    PDECLARE_MUTEX(m_mutex);
    bool      m_opened;
    PString   m_text;
    PFilePath m_filePath;
    unsigned  m_volume;
    unsigned  m_sampleRate;
};

#define new PNEW
#define PTraceModule() "Festival-TTS"

PFACTORY_CREATE(PFactory<PTextToSpeech>, PTextToSpeech_Festival, "Festival", false);


PTextToSpeech_Festival::PTextToSpeech_Festival()
  : m_opened(false)
  , m_volume(100)
  , m_sampleRate(8000)
{
  PTRACE(4, "Constructed " << this);
}


PTextToSpeech_Festival::~PTextToSpeech_Festival()
{
  PWaitAndSignal mutex(m_mutex);
  PTRACE(4, "Destroyed " << this);
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

  PTRACE(4, "Writing speech to \"" << fn << '"');

  return true;
}


PBoolean PTextToSpeech_Festival::Close()
{
  PWaitAndSignal mutex(m_mutex);

  if (!m_opened)
    return true;

  m_opened = false;

  if (m_filePath.IsEmpty()) {
    PTRACE(1, "Stream mode not supported (yet)");
    return false;
  }

  if (m_text.IsEmpty()) {
    PTRACE(1, "Nothing spoken");
    return false;
  }

  PFile wav;
  if (!wav.Open(m_filePath, PFile::WriteOnly)) {
    PTRACE(1, "Could not create WAV file: \"" << m_filePath << '"');
    return false;
  }

  PStringStream cmdLine;
  cmdLine << "text2wave -scale " << std::fixed << m_volume/100.0 << " -F " << m_sampleRate;

  PTRACE(4, "Creating \"" << m_filePath << "\" from \"" << m_text << "\" using \"" << cmdLine << '"');
  PPipeChannel cmd(cmdLine, PPipeChannel::ReadWrite, true, true);

  cmd << m_text << '\n';
  if (!cmd.Execute()) { // Flushes stream and sends EOF
    PTRACE(1, "Festival Generation failed: code=" << cmd.WaitForTermination());
    wav.Remove();
    return false;
  }

  char buf[1000];
  while (cmd.Read(buf, sizeof(buf))) {
    if (!wav.Write(buf, cmd.GetLastReadCount())) {
      PTRACE(1, "Could not write to WAV file: \"" << m_filePath << '"');
      wav.Remove();
      return false;
    }
  }

  int result = cmd.WaitForTermination();

#if PTRACING
  PString error;
  if (cmd.ReadStandardError(error, false)) {
    PTRACE(2, "Error: \"" << error.Trim() << '"');
    result = 1;
  }
  else if (result != 0) {
    PTRACE(2, "Error from sub-process: result=" << result);
  }
  else {
    PTRACE(5, "Generation complete: " << wav.GetLength() << " bytes");
  }
#endif

  return result == 0;
}


PBoolean PTextToSpeech_Festival::Speak(const PString & str, TextType hint)
{
  PWaitAndSignal mutex(m_mutex);

  if (!IsOpen()) {
    PTRACE(2, "Attempt to speak whilst engine not open");
    return false;
  }

  if (m_filePath.IsEmpty()) {
    PTRACE(1, "Stream mode not supported (yet)");
    return false;
  }

  PTRACE(4, "Speaking \"" << str << "\", hint=" << hint);

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


PBoolean PTextToSpeech_Festival::SetSampleRate(unsigned v)
{
  m_sampleRate = v;
  return true;
}


unsigned PTextToSpeech_Festival::GetSampleRate()
{
  return m_sampleRate;
}


PBoolean PTextToSpeech_Festival::SetChannels(unsigned v)
{
  return v == 1;
}


unsigned PTextToSpeech_Festival::GetChannels()
{
  return 1;
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
