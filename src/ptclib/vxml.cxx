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
 */

#ifdef __GNUC__
#pragma implementation "vxml.h"
#endif

#include <ptlib.h>

#define P_DISABLE_FACTORY_INSTANCES

#if P_VXML

#include <ptclib/vxml.h>

#include <ptlib/pprocess.h>
#include <ptlib/vconvert.h>
#include <ptclib/memfile.h>
#include <ptclib/random.h>
#include <ptclib/http.h>
#include <ptclib/mediafile.h>
#include <ptclib/SignLanguageAnalyser.h>


#define PTraceModule() "VXML"


static PConstString const ApplicationScope("application");
static PConstString const DialogScope("dialog");
static PConstString const PropertyScope("property");

static PConstString const FormElement("form");
static PConstString const MenuElement("menu");
static PConstString const PromptElement("prompt");
static PConstString const FilledElement("filled");
static PConstString const NameAttribute("name");
static PConstString const IdAttribute("id");
static PConstString const ExprAttribute("expr");
static PConstString const SrcAttribute("src");
static PConstString const NextAttribute("next");
static PConstString const DtmfAttribute("dtmf");
static PConstString const DestAttribute("dest");
static PConstString const DestExprAttribute("destexpr");


class PVXMLChannelPCM : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelPCM, PVXMLChannel);

  public:
    PVXMLChannelPCM();

    // overrides from PVXMLChannel
    virtual PString GetAudioFormat() const;
    virtual unsigned GetSampleRate() const;
    virtual bool SetSampleRate(unsigned rate);
    virtual unsigned GetChannels() const;
    virtual bool SetChannels(unsigned channels);
    virtual PBoolean WriteFrame(const void * buf, PINDEX len);
    virtual PBoolean ReadFrame(void * buffer, PINDEX amount);
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount);
    virtual PBoolean IsSilenceFrame(const void * buf, PINDEX len) const;
    virtual void GetBeepData(PBYTEArray & data, unsigned ms);

  protected:
    unsigned m_sampleRate;
    unsigned m_channels;
};


class PVXMLChannelG7231 : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelG7231, PVXMLChannel);
  public:
    PVXMLChannelG7231();

    // overrides from PVXMLChannel
    virtual PString GetAudioFormat() const;
    virtual unsigned GetSampleRate() const;
    virtual bool SetSampleRate(unsigned rate);
    virtual unsigned GetChannels() const;
    virtual bool SetChannels(unsigned channels);
    virtual PBoolean WriteFrame(const void * buf, PINDEX len);
    virtual PBoolean ReadFrame(void * buffer, PINDEX amount);
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount);
    virtual PBoolean IsSilenceFrame(const void * buf, PINDEX len) const;
};


class PVXMLChannelG729 : public PVXMLChannel
{
  PCLASSINFO(PVXMLChannelG729, PVXMLChannel);
  public:
    PVXMLChannelG729();

    // overrides from PVXMLChannel
    virtual PString GetAudioFormat() const;
    virtual unsigned GetSampleRate() const;
    virtual bool SetSampleRate(unsigned rate);
    virtual unsigned GetChannels() const;
    virtual bool SetChannels(unsigned channels);
    virtual PBoolean WriteFrame(const void * buf, PINDEX len);
    virtual PBoolean ReadFrame(void * buffer, PINDEX amount);
    virtual PINDEX CreateSilenceFrame(void * buffer, PINDEX amount);
    virtual PBoolean IsSilenceFrame(const void * buf, PINDEX len) const;
};


#define TRAVERSE_NODE(name) \
  class PVXMLTraverse##name : public PVXMLNodeHandler { \
    virtual bool Start(PVXMLSession & session, PXMLElement & element) const \
    { return session.Traverse##name(element); } \
  }; \
  PFACTORY_CREATE(PVXMLNodeFactory, PVXMLTraverse##name, #name, true)

TRAVERSE_NODE(Block);
TRAVERSE_NODE(Audio);
TRAVERSE_NODE(Break);
TRAVERSE_NODE(Value);
TRAVERSE_NODE(SayAs);
TRAVERSE_NODE(Goto);
TRAVERSE_NODE(Grammar);
TRAVERSE_NODE(ElseIf);
TRAVERSE_NODE(Else);
TRAVERSE_NODE(Exit);
TRAVERSE_NODE(Var);
TRAVERSE_NODE(Submit);
TRAVERSE_NODE(Choice);
TRAVERSE_NODE(Property);
TRAVERSE_NODE(Disconnect);
TRAVERSE_NODE(Script);

#define TRAVERSE_NODE2(name) \
  class PVXMLTraverse##name : public PVXMLNodeHandler { \
    virtual bool Start(PVXMLSession & session, PXMLElement & element) const \
    { return session.Traverse##name(element); } \
    virtual bool Finish(PVXMLSession & session, PXMLElement & element) const \
    { return session.Traversed##name(element); } \
  }; \
  PFACTORY_CREATE(PVXMLNodeFactory, PVXMLTraverse##name, #name, true)

TRAVERSE_NODE2(Menu);
TRAVERSE_NODE2(Form);
TRAVERSE_NODE2(Field);
TRAVERSE_NODE2(Transfer);
TRAVERSE_NODE2(Record);
TRAVERSE_NODE2(Prompt);
TRAVERSE_NODE2(If);


static PConstString const InternalEventStateAttribute("PTLibInternalEventState");

class PVXMLTraverseEvent : public PVXMLNodeHandler
{
  virtual bool Start(PVXMLSession &, PXMLElement & element) const
  {
    return element.GetAttribute(InternalEventStateAttribute) == "true";
  }

  virtual bool Finish(PVXMLSession & session, PXMLElement & element) const
  {
    if (element.GetAttribute(InternalEventStateAttribute) != "true")
      return true;

    element.SetAttribute(InternalEventStateAttribute, "false");
    session.m_currentNode = element.GetParent();
    return false;
  }
};
PFACTORY_CREATE(PVXMLNodeFactory, PVXMLTraverseEvent, "Filled", true);
PFACTORY_SYNONYM(PVXMLNodeFactory, PVXMLTraverseEvent, NoInput, "NoInput");
PFACTORY_SYNONYM(PVXMLNodeFactory, PVXMLTraverseEvent, NoMatch, "NoMatch");
PFACTORY_SYNONYM(PVXMLNodeFactory, PVXMLTraverseEvent, Error, "Error");
PFACTORY_SYNONYM(PVXMLNodeFactory, PVXMLTraverseEvent, Catch, "Catch");

#if PTRACING
class PVXMLTraverseLog : public PVXMLNodeHandler {
  virtual bool Start(PVXMLSession & session, PXMLElement & node) const
  {
    unsigned level = node.GetAttribute("level").AsUnsigned();
    if (level == 0)
      level = 3;
    PTRACE(level, "VXML-Log\t" + session.EvaluateExpr(node.GetAttribute(ExprAttribute)));
    return true;
  }
};
PFACTORY_CREATE(PVXMLNodeFactory, PVXMLTraverseLog, "Log", true);
#endif


#define new PNEW


#define SMALL_BREAK_MSECS   1000
#define MEDIUM_BREAK_MSECS  2500
#define LARGE_BREAK_MSECS   5000


//////////////////////////////////////////////////////////

#if P_VXML_VIDEO

#define SIGN_LANGUAGE_PREVIEW_SCRIPT_FUNCTION "SignLanguageAnalyserPreview"
#define SIGN_LANGUAGE_CONTROL_SCRIPT_FUNCTION "SignLanguageAnalyserControl"

class PVXMLSignLanguageAnalyser : public PProcessStartup
{
  PDECLARE_READ_WRITE_MUTEX(m_mutex);
  PString      m_colourFormat;
  vector<bool> m_instancesInUse;


  struct Library : PDynaLink
  {
    EntryPoint<SLInitialiseFn> SLInitialise;
    EntryPoint<SLReleaseFn>    SLRelease;
    EntryPoint<SLAnalyseFn>    SLAnalyse;
    EntryPoint<SLPreviewFn>    SLPreview;
    EntryPoint<SLControlFn>    SLControl;
    EntryPoint<SLErrorTextFn>  SLErrorText;

    Library(const PFilePath & dllName)
      : PDynaLink(dllName)
      , P_DYNALINK_ENTRY_POINT(SLInitialise)
      , P_DYNALINK_OPTIONAL_ENTRY_POINT(SLRelease)
      , P_DYNALINK_ENTRY_POINT(SLAnalyse)
      , P_DYNALINK_OPTIONAL_ENTRY_POINT(SLPreview)
      , P_DYNALINK_OPTIONAL_ENTRY_POINT(SLControl)
      , P_DYNALINK_OPTIONAL_ENTRY_POINT(SLErrorText)
    {
    }

    ~Library()
    {
      if (SLRelease.IsPresent()) {
        PTRACE_PARAM(int result =) SLRelease();
        PTRACE(result < 0 ? 2 : 3, "Released Sign Language Analyser dynamic library"
                                  " \"" << GetName(true) << "\", result=" << result);
      }
    }
  } *m_library;

public:
  PVXMLSignLanguageAnalyser()
    : m_library(NULL)
  {
  }


  ~PVXMLSignLanguageAnalyser()
  {
    delete m_library; // Should always be NULL
  }


  PFACTORY_GET_SINGLETON(PProcessStartupFactory, PVXMLSignLanguageAnalyser);


  virtual void OnShutdown()
  {
    delete m_library;
    m_library = NULL;
  }


  bool CheckError(int code PTRACE_PARAM(, const char * msg))
  {
    if (code >= 0)
      return true;

#if PTRACING
    static unsigned const Level = 2;
    if (PTrace::CanTrace(2)) {
      ostream & trace = PTRACE_BEGIN(Level);
      trace << "Error " << code;
      if (m_library != NULL && m_library->SLErrorText.IsPresent()) {
        SLErrorData errorData;
        errorData.m_code = code;
        memset(errorData.m_text, 0, sizeof(errorData.m_text));
        m_library->SLErrorText(&errorData);
        if (errorData.m_text[0] != '\0')
          trace << " \"" << errorData.m_text << '"';
      }
      trace << ' ' << msg
            << " Sign Language Analyser dynamic library \"" << m_library->GetName(true) << '"'
            << PTrace::End;
    }
#endif
    return false;
  }


  bool SetAnalyser(const PFilePath & dllName)
  {
    PWriteWaitAndSignal lock(m_mutex);

    if (!dllName.IsEmpty()) {
      if (m_library != NULL && dllName == m_library->GetName(true))
        return true;

      delete m_library;
      m_library = new Library(dllName);

      if (!m_library->IsLoaded())
        PTRACE(2, NULL, PTraceModule(), "Could not open Sign Language Analyser dynamic library \"" << dllName << '"');
      else {
        SLAnalyserInit init;
        memset(&init, 0, sizeof(init));
        init.m_apiVersion = SL_API_VERSION;
        if (CheckError(m_library->SLInitialise(&init) PTRACE_PARAM(, "initialising"))) {
          switch (init.m_videoFormat) {
            case SL_GreyScale:
              m_colourFormat = "Grey";
              break;
            case SL_RGB24:
              m_colourFormat = "RGB24";
              break;
            case SL_BGR24:
              m_colourFormat = "BGR24";
              break;
            case SL_RGB32:
              m_colourFormat = "RGB32";
              break;
            case SL_BGR32:
              m_colourFormat = "BGR32";
              break;
            default:
              m_colourFormat = PVideoFrameInfo::YUV420P();
              break;
          }

          m_instancesInUse.resize(init.m_maxInstances);

          PTRACE(3, "Loaded Sign Language Analyser dynamic library"
                    " \"" << m_library->GetName(true) << "\","
                    " required format: " << m_colourFormat << " (" << init.m_videoFormat << ')');
          return true;
        }
      }
    }

    delete m_library;
    m_library = NULL;
    return false;
  }


  const PString & GetColourFormat() const
  {
    return m_colourFormat;
  }


  int AllocateInstance()
  {
    PWriteWaitAndSignal lock(m_mutex);
    if (m_library == NULL)
      return INT_MAX;

    for (size_t i = 0; i < m_instancesInUse.size(); ++i) {
      if (!m_instancesInUse[i]) {
        m_instancesInUse[i] = true;
        Control(i, "Instance Allocated");
        return i;
      }
    }

    return INT_MAX;
  }


  bool ReleaseInstance(int instance)
  {
    PWriteWaitAndSignal lock(m_mutex);
    if (instance >= (int)m_instancesInUse.size())
      return false;

    Control(instance, "Instance Released");
    m_instancesInUse[instance] = false;
    return true;
  }


  int Analyse(int instance, unsigned width, unsigned height, const PTimeInterval & timestamp, const void * pixels)
  {
    PReadWaitAndSignal lock(m_mutex);
    if (m_library == NULL || instance >= (int)m_instancesInUse.size())
      return 0;

    SLAnalyserData data;
    memset(&data, 0, sizeof(data));
    data.m_instance = instance;
    data.m_width = width;
    data.m_height = height;
    data.m_timestamp = static_cast<unsigned>(timestamp.GetMicroSeconds());
    data.m_pixels = pixels;

    PTRACE(5, "Sign Language Analyse of video frame:"
              " " << width << 'x' << height << ","
              " ts=" << timestamp << ","
              " instance=" << instance);
    int result = m_library->SLAnalyse(&data);
    CheckError(result PTRACE_PARAM(, "analysing"));
    return result;
  }


  class PreviewVideoDevice;

  bool CanPreview(int instance) const
  {
    PReadWaitAndSignal lock(m_mutex);
    return m_library != NULL &&
           m_library->SLPreview.IsPresent() &&
           instance >= 0 &&
           instance < (int)m_instancesInUse.size();
  }

  bool Preview(int instance, unsigned width, unsigned height, uint8_t * pixels)
  {
    PReadWaitAndSignal lock(m_mutex);
    if (!CanPreview(instance))
      return false;

    PTRACE(5, "Sign Language Preview video frame:"
              " " << width << 'x' << height << ","
              " instance=" << instance);
    SLPreviewData data;
    memset(&data, 0, sizeof(data));
    data.m_instance = instance;
    data.m_width = width;
    data.m_height = height;
    data.m_pixels = pixels;

    return CheckError(m_library->SLPreview(&data) PTRACE_PARAM(, "previewing"));
  }

  int Control(unsigned instance, const PString & ctrl)
  {
    PReadWaitAndSignal lock(m_mutex);
    if (m_library == NULL || !m_library->SLControl.IsPresent())
      return INT_MIN;

    SLControlData data;
    data.m_instance = instance;
    data.m_control = ctrl;
    int result = m_library->SLControl(&data);
    CheckError(result PTRACE_PARAM(, "controlling"));
    return result;
  }
};

PFACTORY_CREATE_SINGLETON(PProcessStartupFactory, PVXMLSignLanguageAnalyser);


class PVXMLSignLanguageAnalyser::PreviewVideoDevice : public PVideoInputEmulatedDevice
{
  const PVXMLSession::VideoReceiverDevice & m_receiver;

public:
  PreviewVideoDevice(const PVXMLSession::VideoReceiverDevice & receiver)
    : m_receiver(receiver)
  {
    m_colourFormat = GetInstance().GetColourFormat();
  }

  virtual PStringArray GetDeviceNames() const
  {
    return SIGN_LANGUAGE_PREVIEW_SCRIPT_FUNCTION;
  }

  virtual PBoolean Open(const PString &, PBoolean)
  {
    return IsOpen();
  }

  virtual PBoolean IsOpen()
  {
    return PVXMLSignLanguageAnalyser::GetInstance().CanPreview(m_receiver.GetAnalayserInstance());
  }

  virtual PBoolean Close()
  {
    return true;
  }

  virtual PBoolean SetFrameSize(unsigned width, unsigned height)
  {
    return width == m_frameWidth && height == m_frameHeight;
  }

  PBoolean SetColourFormat(const PString & colourFormat)
  {
    return colourFormat == m_colourFormat;
  }


protected:
  virtual bool InternalReadFrameData(BYTE * buffer)
  {
    if (!IsOpen())
      return false;

    // If remote resolution changes, we need to change the preview as well
    unsigned oldWidth, oldHeight;
    GetFrameSize(oldWidth, oldHeight);
    unsigned newWidth, newHeight;
    m_receiver.GetFrameSize(newWidth, newHeight);
    if (newWidth != oldWidth || newHeight != oldHeight) {
      m_frameWidth = newWidth;
      m_frameHeight = newHeight;
      SetFrameSizeConverter(oldWidth, oldHeight);
    }

    return PVXMLSignLanguageAnalyser::GetInstance().Preview(m_receiver.GetAnalayserInstance(), m_frameWidth, m_frameHeight, buffer);
  }
};
#endif // P_VXML_VIDEO


//////////////////////////////////////////////////////////

PVXMLPlayable::PVXMLPlayable()
  : m_vxmlChannel(NULL)
  , m_subChannel(NULL)
  , m_repeat(1)
  , m_delay(0)
  , m_autoDelete(false)
  , m_delayDone(false)
{
  PTRACE(4, "Constructed PVXMLPlayable " << this);
}


PVXMLPlayable::~PVXMLPlayable()
{
  OnStop();
  PTRACE(4, "Destroyed PVXMLPlayable " << this);
}


PBoolean PVXMLPlayable::Open(PVXMLChannel & channel, const PString &, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  m_vxmlChannel = &channel;
  m_delay = delay;
  m_repeat = repeat;
  m_autoDelete = autoDelete;
  return true;
}


bool PVXMLPlayable::OnRepeat()
{
  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  if (m_repeat <= 1)
    return false;

  --m_repeat;
  return true;
}


bool PVXMLPlayable::OnDelay()
{
  if (m_delayDone)
    return false;

  m_delayDone = true;
  if (m_delay == 0)
    return false;

  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  m_vxmlChannel->SetSilence(m_delay);
  return true;
}


void PVXMLPlayable::OnStop()
{
  if (m_vxmlChannel == NULL || m_subChannel == NULL)
    return;

  if (m_vxmlChannel->GetReadChannel() == m_subChannel)
    m_vxmlChannel->SetReadChannel(NULL, false, true);

  delete m_subChannel;
  m_subChannel = NULL;
}


///////////////////////////////////////////////////////////////

bool PVXMLPlayableStop::OnStart()
{
  if (m_vxmlChannel == NULL)
    return false;

  m_vxmlChannel->SetSilence(500);
  return false; // Return false so always stops
}


///////////////////////////////////////////////////////////////

PBoolean PVXMLPlayableFile::Open(PVXMLChannel & chan, const PString & fn, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  return chan.AdjustMediaFilename(fn, m_filePath) && PVXMLPlayable::Open(chan, fn, delay, repeat, autoDelete);
}


bool PVXMLPlayableFile::OnStart()
{
  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  m_subChannel = m_vxmlChannel->OpenMediaFile(m_filePath, false);
  if (m_subChannel == NULL)
    return false;

  PTRACE(3, "Playing file \"" << m_filePath << '"');
  return m_vxmlChannel->SetReadChannel(m_subChannel, false);
}


bool PVXMLPlayableFile::OnRepeat()
{
  if (!PVXMLPlayable::OnRepeat())
    return false;

  PFile * file = dynamic_cast<PFile *>(m_subChannel);
  return PAssert(file != NULL, PLogicError) && PAssertOS(file->SetPosition(0));
}


void PVXMLPlayableFile::OnStop()
{
  PVXMLPlayable::OnStop();

  if (m_autoDelete && !m_filePath.IsEmpty()) {
    PTRACE(3, "Deleting file \"" << m_filePath << "\"");
    PFile::Remove(m_filePath);
  }
}


PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableFile, "File");


///////////////////////////////////////////////////////////////

PVXMLPlayableFileList::PVXMLPlayableFileList()
  : m_currentIndex(0)
{
}


PBoolean PVXMLPlayableFileList::Open(PVXMLChannel & chan, const PString & list, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  return Open(chan, list.Lines(), delay, repeat, autoDelete);
}


PBoolean PVXMLPlayableFileList::Open(PVXMLChannel & chan, const PStringArray & list, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  for (PINDEX i = 0; i < list.GetSize(); ++i) {
    PFilePath fn;
    if (chan.AdjustMediaFilename(list[i], fn))
      m_fileNames.AppendString(fn);
  }

  if (m_fileNames.IsEmpty()) {
    PTRACE(2, "No files in list exist.");
    return false;
  }

  m_currentIndex = 0;

  return PVXMLPlayable::Open(chan, PString::Empty(), delay, ((repeat > 0) ? repeat : 1) * m_fileNames.GetSize(), autoDelete);
}


bool PVXMLPlayableFileList::OnStart()
{
  if (!PAssert(!m_fileNames.IsEmpty(), PLogicError))
    return false;

  m_filePath = m_fileNames[m_currentIndex++ % m_fileNames.GetSize()];
  return PVXMLPlayableFile::OnStart();
}


bool PVXMLPlayableFileList::OnRepeat()
{
  return PVXMLPlayable::OnRepeat() && OnStart();

}


void PVXMLPlayableFileList::OnStop()
{
  m_filePath.MakeEmpty();

  PVXMLPlayableFile::OnStop();

  if (m_autoDelete)  {
    for (PINDEX i = 0; i < m_fileNames.GetSize(); ++i) {
      PTRACE(3, "Deleting file \"" << m_fileNames[i] << "\"");
      PFile::Remove(m_fileNames[i]);
    }
  }
}

PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableFileList, "FileList");


///////////////////////////////////////////////////////////////

#if P_PIPECHAN

PBoolean PVXMLPlayableCommand::Open(PVXMLChannel & chan, const PString & cmd, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  if (cmd.IsEmpty()) {
    PTRACE(2, "Empty command line.");
    return false;
  }

  m_command = cmd;
  return PVXMLPlayable::Open(chan, cmd, delay, repeat, autoDelete);
}


bool PVXMLPlayableCommand::OnStart()
{
  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  PString cmd = m_command;
  cmd.Replace("%s", PString(PString::Unsigned, m_vxmlChannel->GetSampleRate()));
  cmd.Replace("%f", m_format);

  // execute a command and send the output through the stream
  PPipeChannel * pipe = new PPipeChannel;
  if (!pipe->Open(cmd, PPipeChannel::ReadOnly)) {
    PTRACE(2, "Cannot open command \"" << cmd << '"');
    delete pipe;
    return false;
  }

  if (!pipe->Execute()) {
    PTRACE(2, "Cannot start command \"" << cmd << '"');
    return false;
  }

  PTRACE(3, "Playing command \"" << cmd << '"');
  m_subChannel = pipe;
  return m_vxmlChannel->SetReadChannel(pipe, false);
}


void PVXMLPlayableCommand::OnStop()
{
  PPipeChannel * pipe = dynamic_cast<PPipeChannel *>(m_subChannel);
  if (PAssert(pipe != NULL, PLogicError))
    pipe->WaitForTermination();

  PVXMLPlayable::OnStop();
}

PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableCommand, "Command");

#endif


///////////////////////////////////////////////////////////////

PBoolean PVXMLPlayableData::Open(PVXMLChannel & chan, const PString & hex, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  return PVXMLPlayable::Open(chan, hex, delay, repeat, autoDelete);
}


void PVXMLPlayableData::SetData(const PBYTEArray & data)
{
  m_data = data;
}


bool PVXMLPlayableData::OnStart()
{
  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  m_subChannel = new PMemoryFile(m_data);
  PTRACE(3, "Playing " << m_data.GetSize() << " bytes of memory");
  return m_vxmlChannel->SetReadChannel(m_subChannel, false);
}


bool PVXMLPlayableData::OnRepeat()
{
  if (!PVXMLPlayable::OnRepeat())
    return false;

  PMemoryFile * memfile = dynamic_cast<PMemoryFile *>(m_subChannel);
  return PAssert(memfile != NULL, PLogicError) && PAssertOS(memfile->SetPosition(0));
}

PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableData, "PCM Data");


///////////////////////////////////////////////////////////////

#if P_DTMF

PBoolean PVXMLPlayableTone::Open(PVXMLChannel & chan, const PString & toneSpec, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  // populate the tone buffer
  PTones tones;

  if (!tones.Generate(toneSpec)) {
    PTRACE(2, "Could not generate tones with \"" << toneSpec << '"');
    return false;
  }

  PINDEX len = tones.GetSize() * sizeof(short);
  memcpy(m_data.GetPointer(len), tones.GetPointer(), len);

  return PVXMLPlayable::Open(chan, toneSpec, delay, repeat, autoDelete);
}

PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableTone, "Tone");

#endif // P_DTMF


///////////////////////////////////////////////////////////////

PBoolean PVXMLPlayableURL::Open(PVXMLChannel & chan, const PString & url, PINDEX delay, PINDEX repeat, PBoolean autoDelete)
{
  if (!m_url.Parse(url)) {
    PTRACE(2, "Invalid URL \"" << url << '"');
    return false;
  }

  return PVXMLPlayable::Open(chan, url, delay, repeat, autoDelete);
}


bool PVXMLPlayableURL::OnStart()
{
  if (PAssertNULL(m_vxmlChannel) == NULL)
    return false;

  // open the resource
  PHTTPClient * client = new PHTTPClient;
  client->SetPersistent(false);
  PMIMEInfo outMIME, replyMIME;
  int code = client->GetDocument(m_url, outMIME, replyMIME);
  if ((code != 200) || (replyMIME(PHTTP::TransferEncodingTag()) *= PHTTP::ChunkedTag())) {
    delete client;
    return false;
  }

  m_subChannel = client;
  return m_vxmlChannel->SetReadChannel(client, false);
}

PFACTORY_CREATE(PFactory<PVXMLPlayable>, PVXMLPlayableURL, "URL");


///////////////////////////////////////////////////////////////

PVXMLRecordable::PVXMLRecordable()
  : m_finalSilence(3000)
  , m_maxDuration(30000)
{
}


///////////////////////////////////////////////////////////////

PBoolean PVXMLRecordableFilename::Open(const PString & arg)
{
  m_fileName = arg;
  return true;
}


bool PVXMLRecordableFilename::OnStart(PVXMLChannel & outgoingChannel)
{
  PChannel * file = outgoingChannel.OpenMediaFile(m_fileName, true);
  if (file == NULL)
    return false;

  PTRACE(3, "Recording to file \"" << m_fileName << "\","
            " duration=" << m_maxDuration << ", silence=" << m_finalSilence);
  outgoingChannel.SetWriteChannel(file, true);

  m_silenceTimer = m_finalSilence;
  m_recordTimer = m_maxDuration;
  return true;
}


PBoolean PVXMLRecordableFilename::OnFrame(PBoolean isSilence)
{
  if (isSilence) {
    if (m_silenceTimer.HasExpired()) {
      PTRACE(4, "Recording silence detected.");
      return true;
    }
  }
  else
    m_silenceTimer = m_finalSilence;

  if (m_recordTimer.HasExpired()) {
    PTRACE(3, "Recording finished due to max time exceeded.");
    return true;
  }

  return false;
}


///////////////////////////////////////////////////////////////

static PVXMLCache DefaultCache;
static const PConstString KeyFileType(".key");

PVXMLCache::PVXMLCache()
  : m_directory("cache")
{
}


void PVXMLCache::SetDirectory(const PDirectory & directory)
{
  LockReadWrite();
  m_directory = directory;
  UnlockReadWrite();
}


PFilePath PVXMLCache::CreateFilename(const PString & prefix, const PString & key, const PString & suffix)
{
  if (!m_directory.Exists()) {
    if (!m_directory.Create()) {
      PTRACE(2, "Could not create cache directory \"" << m_directory << '"');
    }
  }

  PMessageDigest5::Result digest;
  PMessageDigest5::Encode(key, digest);

  return PSTRSTRM(m_directory << prefix << '_' << hex << digest << suffix);
}


bool PVXMLCache::Get(const PString & prefix,
                     const PString & key,
                     const PString & suffix,
                         PFilePath & filename)
{
  PAssert(!prefix.IsEmpty() && !key.IsEmpty(), PInvalidParameter);

  PSafeLockReadOnly mutex(*this);

  PTextFile keyFile(CreateFilename(prefix, key, KeyFileType), PFile::ReadOnly);
  PFile dataFile(CreateFilename(prefix, key, suffix), PFile::ReadOnly);

  if (dataFile.Open()) {
    if (keyFile.Open()) {
      if (keyFile.ReadString(P_MAX_INDEX) == key) {
        if (dataFile.GetLength() != 0) {
          PTRACE(5, "Cache data found for \"" << key << '"');
          filename = dataFile.GetFilePath();
          return true;
        }
        else {
          PTRACE(2, "Cached data empty for \"" << key << '"');
        }
      }
      else {
        PTRACE(2, "Cache coherence problem for \"" << key << '"');
      }
    }
    else {
      PTRACE(2, "Cannot open cache key file \"" << keyFile.GetFilePath() << "\""
                " for \"" << key << "\", error: " << keyFile.GetErrorText());
    }
  }
  else {
    PTRACE(2, "Cannot open cache data file \"" << dataFile.GetFilePath() << "\""
              " for \"" << key << "\", error: " << dataFile.GetErrorText());
  }

  keyFile.Remove(true);
  dataFile.Remove(true);
  return false;
}


bool PVXMLCache::PutWithLock(const PString & prefix,
                             const PString & key,
                             const PString & suffix,
                                     PFile & dataFile)
{
  PSafeLockReadWrite mutex(*this);

  // create the filename for the cache files
  if (!dataFile.Open(CreateFilename(prefix, key, suffix), PFile::WriteOnly, PFile::Create|PFile::Truncate)) {
    PTRACE(2, "Cannot create cache data file \"" << dataFile.GetFilePath() << "\""
              " for \"" << key << "\", error: " << dataFile.GetErrorText());
    return false;
  }

  // write the content type file
  PTextFile keyFile(CreateFilename(prefix, key, KeyFileType), PFile::WriteOnly, PFile::Create|PFile::Truncate);
  if (keyFile.IsOpen()) {
    if (keyFile.WriteString(key)) {
      LockReadWrite();
      PTRACE(5, "Cache data created for \"" << key << '"');
      return true;
    }
    else {
      PTRACE(2, "Cannot write cache key file \"" << keyFile.GetFilePath() << "\""
                " for \"" << key << "\", error: " << keyFile.GetErrorText());
    }
  }
  else {
    PTRACE(2, "Cannot create cache key file \"" << keyFile.GetFilePath() << "\""
              " for \"" << key << "\", error: " << keyFile.GetErrorText());
  }

  dataFile.Remove(true);
  return false;
}


//////////////////////////////////////////////////////////

PVXMLSession::PVXMLSession(PTextToSpeech * tts, PBoolean autoDelete)
  : m_textToSpeech(tts)
  , m_ttsCache(NULL)
  , m_autoDeleteTextToSpeech(autoDelete)
#if P_VXML_VIDEO
  , m_videoReceiver(*this)
#endif
  , m_vxmlThread(NULL)
  , m_abortVXML(false)
  , m_currentNode(NULL)
  , m_speakNodeData(true)
  , m_bargeIn(true)
  , m_bargingIn(false)
  , m_promptCount(0)
  , m_grammar(NULL)
  , m_defaultMenuDTMF('N') /// Disabled
  , m_recordingStatus(NotRecording)
  , m_recordStopOnDTMF(false)
  , m_recordingStartTime(0)
  , m_transferStatus(NotTransfering)
  , m_transferStartTime(0)
{
#if P_SCRIPTS
  m_scriptContext = PScriptLanguage::Create("JavaScript");
  if (m_scriptContext == NULL || !m_scriptContext->IsInitialised()) {
    delete m_scriptContext;
    m_scriptContext = PScriptLanguage::Create("Lua"); // Back up
  }

  if (m_scriptContext != NULL) {
    m_scriptContext->CreateComposite(ApplicationScope);
    m_scriptContext->CreateComposite(DialogScope);
    m_scriptContext->CreateComposite(PropertyScope);
    m_scriptContext->CreateComposite("session");
    m_scriptContext->CreateComposite("session.connection");
    m_scriptContext->CreateComposite("session.connection.local");
    m_scriptContext->CreateComposite("session.connection.remote");
#if P_VXML_VIDEO
    m_scriptContext->SetFunction(SIGN_LANGUAGE_PREVIEW_SCRIPT_FUNCTION, PCREATE_NOTIFIER(SignLanguagePreviewFunction));
    m_scriptContext->SetFunction(SIGN_LANGUAGE_CONTROL_SCRIPT_FUNCTION, PCREATE_NOTIFIER(SignLanguageControlFunction));
#endif
  }
#endif

  SetVar("property.timeout" , "10s");
  SetVar("property.bargein", "true");

#if P_VXML_VIDEO
  PVideoInputDevice::OpenArgs videoArgs;
  videoArgs.driverName = P_FAKE_VIDEO_DRIVER;
  videoArgs.deviceName = P_FAKE_VIDEO_BLACK;
  m_videoSender.SetActualDevice(PVideoInputDevice::CreateOpenedDevice(videoArgs));
#endif // P_VXML_VIDEO

  PTRACE(4, "Created session: " << this);
}


PVXMLSession::~PVXMLSession()
{
  PTRACE(4, "Destroying session: " << this);

  Close();

  if (m_autoDeleteTextToSpeech)
    delete m_textToSpeech;

#if P_SCRIPTS
  delete m_scriptContext;
#endif
}


PTextToSpeech * PVXMLSession::SetTextToSpeech(PTextToSpeech * tts, PBoolean autoDelete)
{
  PWaitAndSignal mutex(m_sessionMutex);

  if (m_autoDeleteTextToSpeech)
    delete m_textToSpeech;

  m_autoDeleteTextToSpeech = autoDelete;
  return m_textToSpeech = tts;
}


PTextToSpeech * PVXMLSession::SetTextToSpeech(const PString & ttsName)
{
  PFactory<PTextToSpeech>::Key_T name = (const char *)ttsName;
  if (ttsName.IsEmpty()) {
    PFactory<PTextToSpeech>::KeyList_T engines = PFactory<PTextToSpeech>::GetKeyList();
    if (engines.empty())
      return SetTextToSpeech(NULL, false);

#ifdef _WIN32
    name = "Microsoft SAPI";
    if (std::find(engines.begin(), engines.end(), name) == engines.end())
#endif
      name = engines[0];
  }

  return SetTextToSpeech(PFactory<PTextToSpeech>::CreateInstance(name), true);
}


void PVXMLSession::SetCache(PVXMLCache & cache)
{
  m_sessionMutex.Wait();
  m_ttsCache = &cache;
  m_sessionMutex.Signal();
}


PVXMLCache & PVXMLSession::GetCache()
{
  PWaitAndSignal mutex(m_sessionMutex);

  if (m_ttsCache == NULL)
    m_ttsCache = &DefaultCache;

  return *m_ttsCache;
}


PBoolean PVXMLSession::Load(const PString & source)
{
  // See if we have been given a file
  PFilePath file = source;
  if (PFile::Exists(file))
    return LoadFile(file);

  // see if a URL
  PURL url(source, NULL);
  if (!url.IsEmpty())
    return LoadURL(url);

  // Try and parse it as direct VXML
  return LoadVXML(source);
}


PBoolean PVXMLSession::LoadFile(const PFilePath & filename, const PString & firstForm)
{
  PTRACE(4, "Loading file: " << filename);

  PTextFile file(filename, PFile::ReadOnly);
  if (!file.IsOpen()) {
    PTRACE(1, "Cannot open " << filename);
    return false;
  }

  m_rootURL = PURL(filename);
  return InternalLoadVXML(file.ReadString(P_MAX_INDEX), firstForm);
}


PBoolean PVXMLSession::LoadURL(const PURL & url)
{
  PTRACE(4, "Loading URL " << url);

  // retreive the document (may be a HTTP get)

  PString xmlStr;
  if (url.LoadResource(xmlStr)) {
    m_rootURL = url;
    return InternalLoadVXML(xmlStr, url.GetFragment());
  }

  PTRACE(1, "Cannot load document " << url);
  return false;
}


PBoolean PVXMLSession::LoadVXML(const PString & xmlText, const PString & firstForm)
{
  m_rootURL = PString::Empty();
  return InternalLoadVXML(xmlText, firstForm);
}


bool PVXMLSession::InternalLoadVXML(const PString & xmlText, const PString & firstForm)
{
  PWaitAndSignal mutex(m_sessionMutex);

  m_speakNodeData = true;
  m_bargeIn = true;
  m_bargingIn = false;
  m_recordingStatus = NotRecording;
  m_transferStatus = NotTransfering;
  m_currentNode = NULL;

  FlushInput();

  // parse the XML
  auto_ptr<PXML> xml(new PXML);
  if (!xml->Load(xmlText)) {
    m_lastXMLError = PSTRSTRM(m_rootURL <<
                              '(' << xml->GetErrorLine() <<
                              ':' << xml->GetErrorColumn() << ")"
                              " " << xml->GetErrorString());
    PTRACE(1, "Cannot parse root document: " << m_lastXMLError);
    return false;
  }

  PXMLElement * root = xml->GetRootElement();
  if (root == NULL) {
    PTRACE(1, "No root element");
    return false;
  }

  if (root->GetName() != "vxml") {
    PTRACE(1, "Invalid root element: " << root->PrintTrace());
    return false;
  }

  if (firstForm.IsEmpty()) {
    if (root->GetElement(FormElement) == NULL && root->GetElement(MenuElement) == NULL) {
      PTRACE(1, "No form or menu element.");
      return false;
    }
  }
  else {
    if (root->GetElement(FormElement, IdAttribute, firstForm) == NULL) {
      PTRACE(1, "No form or menu element with id=\"" << firstForm << '"');
      return false;
    }
  }

  m_variableScope = m_variableScope.IsEmpty() ? ApplicationScope : "document";

  PURL pathURL = m_rootURL;
  pathURL.ChangePath(PString::Empty()); // Remove last element of root URL
  SetVar("path", pathURL);
  SetVar("uri", m_rootURL);

  if (m_currentXML.get() == NULL)
    m_currentXML = xml;
  else
    m_newXML = xml;
  m_newFormName = firstForm;
  InternalStartThread();
  return true;
}


PURL PVXMLSession::NormaliseResourceName(const PString & src)
{
  PURL url;
  if (url.Parse(src, NULL))
    return url;

  if (m_rootURL.IsEmpty()) {
    url.Parse(src, "file");
    return url;
  }

  // relative to scheme/path in root document
  url = m_rootURL;
  PStringArray path = url.GetPath();
  if (src[0] == '/' || path.IsEmpty()) {
    url.SetPathStr(src);
    return url;
  }

  PStringStream str;
  for (PINDEX i = 0; i < path.GetSize()-1; i++)
    str << path[i] << '/';
  str << src;
  url.SetPathStr(str);
  return url;
}


bool PVXMLSession::SetCurrentForm(const PString & searchId, bool fullURI)
{
  ClearBargeIn();
  m_eventCount.clear();
  m_promptCount = 0;

  PString id = searchId;

  if (fullURI) {
    if (searchId.IsEmpty()) {
      PTRACE(3, "Full URI required for this form/menu search");
      return false;
    }

    if (searchId[0] != '#') {
      PTRACE(4, "Searching form/menu \"" << searchId << '"');
      return LoadURL(NormaliseResourceName(searchId));
    }

    id = searchId.Mid(1);
  }

  // Only handle search of top level nodes for <form>/<menu> element
  // NOTE: should have some flag to know if it is loaded
  PXMLElement * root = m_currentXML->GetRootElement();
  if (root != NULL) {
    for (PINDEX i = 0; i < root->GetSize(); i++) {
      PXMLObject * xmlObject = root->GetElement(i);
      if (xmlObject->IsElement()) {
        PXMLElement * xmlElement = (PXMLElement*)xmlObject;
        if (
              (xmlElement->GetName() == FormElement || xmlElement->GetName() == MenuElement) &&
              (id.IsEmpty() || (xmlElement->GetAttribute(IdAttribute) *= id))
           ) {
          PTRACE(3, "Found <" << xmlElement->GetName() << " id=\"" << xmlElement->GetAttribute(IdAttribute) << "\">");

          if (m_currentNode != NULL) {
            PXMLElement * element = m_currentNode->GetParent();
            while (element != NULL) {
              PVXMLNodeHandler * handler = PVXMLNodeFactory::CreateInstance(element->GetName());
              if (handler != NULL) {
                handler->Finish(*this, *element);
                PTRACE(4, "Processed VoiceXML element: " << element->PrintTrace());
              }
              element = element->GetParent();
            }
          }

          m_currentNode = xmlObject;
          return true;
        }
      }
    }
  }

  PTRACE(3, "No form/menu with id \"" << searchId << '"');
  return false;
}


PVXMLChannel * PVXMLSession::GetAndLockVXMLChannel()
{
  m_sessionMutex.Wait();
  if (IsOpen())
    return GetVXMLChannel();

  m_sessionMutex.Signal();
  return NULL;
}


bool PVXMLSession::Open(const PString & mediaFormat, unsigned sampleRate, unsigned channels)
{
  PVXMLChannel * chan = PFactory<PVXMLChannel>::CreateInstance(mediaFormat);
  if (chan == NULL) {
    PTRACE(1, "Cannot create VXML channel with format " << mediaFormat);
    return false;
  }

  if (!chan->Open(this, sampleRate, channels)) {
    delete chan;
    return false;
  }

  // set the underlying channel
  if (!PIndirectChannel::Open(chan, chan))
    return false;

  InternalStartThread();
  return true;
}


void PVXMLSession::InternalStartThread()
{
  PWaitAndSignal mutex(m_sessionMutex);

  if (IsOpen() && IsLoaded()) {
    if (m_vxmlThread == NULL)
      m_vxmlThread = new PThreadObj<PVXMLSession>(*this, &PVXMLSession::InternalThreadMain, false, "VXML");
    else
      Trigger();
  }
}


PBoolean PVXMLSession::Close()
{
  m_sessionMutex.Wait();

  LoadGrammar(NULL);

  // Stop condition for thread
  m_abortVXML = true;
  Trigger();
  PThread::WaitAndDelete(m_vxmlThread, 10000, &m_sessionMutex, false);

#if P_VXML_VIDEO
  m_videoReceiver.Close();
  m_videoSender.Close();
#endif // P_VXML_VIDEO

  return PIndirectChannel::Close();
}


void PVXMLSession::InternalThreadMain()
{
  PTRACE(4, "Execution thread started.");

  m_sessionMutex.Wait();

  {
    PTime now;
    SetVar("session.time", now.AsString());
    SetVar("session.timeISO8601", now.AsString(PTime::ShortISO8601));
    SetVar("session.timeEpoch", now.GetTimeInSeconds());
  }

  InternalStartVXML();

  while (!m_abortVXML) {
    // process current node in the VXML script
    bool processChildren = ProcessNode();

    /* wait for something to happen, usually output of some audio. But under
        some circumstances we want to abort the script, but we  have to make
        sure the script has been run to the end so submit actions etc. can be
        performed. Record and audio and other user interaction commands can
        be skipped, so we don't wait for them */
    do {
      while (ProcessEvents())
        ;
    } while (NextNode(processChildren));

    if (m_newXML.get() != NULL) {
      /* Replace old XML with new, now it is safe to do so and no XML elements
         pointers are being referenced by the code any more */
      m_currentXML = m_newXML;
      InternalStartVXML();
    }

    // Determine if we should quit
    if (m_currentNode != NULL)
      continue;

    PTRACE(3, "End of VoiceXML elements.");

    m_sessionMutex.Signal();
    OnEndDialog();
    m_sessionMutex.Wait();

    // Wait for anything OnEndDialog plays to complete.
    while (ProcessEvents())
      ;

    if (m_newXML.get() != NULL) {
      // OnEndDialog() loaded some more XML to do.
      m_currentXML = m_newXML;
      InternalStartVXML();
    }

    if (m_currentNode == NULL)
      m_abortVXML = true;
  }

  m_sessionMutex.Signal();

  OnEndSession();

  PTRACE(4, "Execution thread ended");
}


void PVXMLSession::OnEndDialog()
{
}


void PVXMLSession::OnEndSession()
{
  Close();
}


bool PVXMLSession::ProcessEvents()
{
  // m_sessionMutex already locked

  if (m_abortVXML || !IsOpen())
    return false;

  PVXMLChannel * vxmlChannel = GetVXMLChannel();
  if (PAssertNULL(vxmlChannel) == NULL)
    return false;

  char ch;

  m_userInputMutex.Wait();
  if (m_userInputQueue.empty())
    ch = '\0';
  else {
    ch = m_userInputQueue.front();
    m_userInputQueue.pop();
  }
  m_userInputMutex.Signal();

  if (ch != '\0') {
    if (m_recordingStatus == RecordingInProgress) {
      PTRACE(3, "Recording ended via user input '" << ch << '\'');
      if (m_recordStopOnDTMF && vxmlChannel->EndRecording(false)) {
        if (!m_recordingName.IsEmpty())
          SetVar(m_recordingName + "$.termchar", ch);
      }
    }
    else if (m_bargeIn) {
      PTRACE(3, "Executing bargein with user input '" << ch << '\'');
      m_bargingIn = true;
      vxmlChannel->FlushQueue();
      if (m_grammar != NULL)
        m_grammar->OnUserInput(ch);
    }
    else if (vxmlChannel->IsPlaying()) {
      PTRACE(3, "No bargein, ignoring user input '" << ch << '\'');
    }
    else {
      PTRACE(3, "Handling user input '" << ch << '\'');
      if (m_grammar != NULL)
        m_grammar->OnUserInput(ch);
    }
  }

  if (vxmlChannel->IsPlaying()) {
    PTRACE(4, "Is playing, awaiting event");
  }
  else if (vxmlChannel->IsRecording()) {
    PTRACE(4, "Is recording, awaiting event");
  }
  else if (m_grammar != NULL && m_grammar->GetState() == PVXMLGrammar::Started) {
    PTRACE(4, "Awaiting input, awaiting event");
  }
  else if (m_transferStatus == TransferInProgress) {
    PTRACE(4, "Transfer in progress, awaiting event");
  }
  else {
    PTRACE(4, "Nothing happening, processing next node");
    return false;
  }

  m_sessionMutex.Signal();
  m_waitForEvent.Wait();
  m_sessionMutex.Wait();

  if (m_newXML.get() == NULL)
    return true;

  PTRACE(4, "XML changed, flushing queue");

  // Clear out any audio being output, so can start fresh on new VXML.
  if (IsOpen())
    GetVXMLChannel()->FlushQueue();

  return false;
}


void PVXMLSession::InternalStartVXML()
{
  LoadGrammar(NULL);

  PTRACE(4, "Processing global elements.");
  m_currentNode = m_currentXML->GetRootElement()->GetElement(0);
  do {
    PXMLElement * element;
    while ((element = dynamic_cast<PXMLElement *>(m_currentNode)) != NULL &&
           (element->GetName() == MenuElement || element->GetName() == FormElement))
      m_currentNode = m_currentNode->GetNextObject();

    bool processGlobalChildren = ProcessNode();
    while (NextNode(processGlobalChildren))
      ;
  } while (m_currentNode != NULL);

  PTRACE(4, "Setting initial form \"" << m_newFormName << '"');
  SetCurrentForm(m_newFormName, false);
}


bool PVXMLSession::NextNode(bool processChildren)
{
  // m_sessionMutex already locked

  if (m_abortVXML)
    return false;

  // No more nodes
  if (m_currentNode == NULL)
    return false;

  // Check for new XML to replace current
  if (m_newXML.get() != NULL)
    return false;

  if (m_grammar != NULL) {
    switch (m_grammar->GetState()) {
      case PVXMLGrammar::Idle:
      case PVXMLGrammar::Started:
        break;

      default:
        ClearBargeIn();

        PVXMLGrammar * grammar = m_grammar;
        m_grammar = NULL; // Moving to new node can set this, deleting it, so detach
        if (grammar->Process()) {
          // Note the PXML objects may have been replaced at this time,
          // pointers can't be used as we work back up the call stack.
          PTRACE(2, "Processed grammar: " << *grammar << ", moving to node " << PXMLObject::PrintTrace(m_currentNode));
          delete grammar;
          return false;
        }

        PTRACE(2, "Processed grammar: " << *grammar << ", restarting node " << PXMLObject::PrintTrace(m_currentNode));
        m_grammar = grammar;
        m_grammar->SetIdle();
    }
  }

  PXMLElement * element = dynamic_cast<PXMLElement *>(m_currentNode);
  if (element != NULL) {
    // if the current node has children, then process the first child
    if (processChildren && (m_currentNode = element->GetSubObject(0)) != NULL)
      return false;
  }
  else {
    // Data node
    PXMLObject * sibling = m_currentNode->GetNextObject();
    if (sibling != NULL) {
      m_currentNode = sibling;
      return false;
    }
    if ((element = m_currentNode->GetParent()) == NULL) {
      m_currentNode = NULL;
      return false;
    }
  }

  // No children, move to sibling
  do {
    PVXMLNodeHandler * handler = PVXMLNodeFactory::CreateInstance(element->GetName());
    if (handler != NULL) {
      PTRACE(4, "Finish processing VoiceXML element: " << element->PrintTrace());
      if (!handler->Finish(*this, *element)) {
        if (m_currentNode != NULL) {
          PTRACE(4, "Moved node after processing VoiceXML element: from=" << element->PrintTrace());
          return false;
        }

        PTRACE(4, "Continue processing VoiceXML element: " << element->PrintTrace());
        m_currentNode = element;
        return true;
      }
      PTRACE(4, "Processed VoiceXML element: " << element->PrintTrace());
    }

    if ((m_currentNode = element->GetNextObject()) != NULL)
      break;

  } while ((element = element->GetParent()) != NULL);

  return false;
}


void PVXMLSession::ClearBargeIn()
{
  PTRACE_IF(4, m_bargingIn, "Ending barge in");
  m_bargingIn = false;
}


void PVXMLSession::FlushInput()
{
  m_userInputMutex.Wait();
  while (!m_userInputQueue.empty())
    m_userInputQueue.pop();
  m_userInputMutex.Signal();
}


bool PVXMLSession::ProcessNode()
{
  // m_sessionMutex already locked

  if (m_abortVXML)
    return false;

  if (m_currentNode == NULL)
    return false;

  PXMLData * nodeData = dynamic_cast<PXMLData *>(m_currentNode);
  if (nodeData != NULL) {
    if (m_speakNodeData)
      PlayText(nodeData->GetString().Trim());
    return true;
  }

  m_speakNodeData = true;

  PXMLElement * element = dynamic_cast<PXMLElement *>(m_currentNode);
  PVXMLNodeHandler * handler = PVXMLNodeFactory::CreateInstance(element->GetName());
  if (handler == NULL) {
    PTRACE(2, "Unknown/unimplemented VoiceXML element: " << element->PrintTrace());
    return false;
  }

  PTRACE(3, "Processing VoiceXML element: " << element->PrintTrace());
  bool started = handler->Start(*this, *element);
  PTRACE_IF(4, !started, "Skipping VoiceXML element: " << element->PrintTrace());
  return started;
}


void PVXMLSession::OnUserInput(const PString & str)
{
  {
    PWaitAndSignal mutex(m_userInputMutex);
    for (PINDEX i = 0; i < str.GetLength(); i++)
      m_userInputQueue.push(str[i]);
  }
  Trigger();
}


PBoolean PVXMLSession::TraverseRecord(PXMLElement & element)
{
  if (!ExecuteCondition(element))
    return false;

  m_recordingStatus = NotRecording;
  return true;
}


PBoolean PVXMLSession::TraversedRecord(PXMLElement & element)
{
  if (m_abortVXML)
    return true; // True is not "good" but "done", that is move to next element in VXML.

  switch (m_recordingStatus) {
    case RecordingInProgress :
      return false;

    case RecordingComplete :
      return !GoToEventHandler(element, FilledElement);

    default :
      break;
  }

  static const PConstString supportedFileType(".wav");
  PCaselessString typeMIME = element.GetAttribute("type");
  if (typeMIME.IsEmpty())
    typeMIME = PMIMEInfo::GetContentType(supportedFileType);

  if (typeMIME != PMIMEInfo::GetContentType(supportedFileType)) {
    PTRACE(2, "Cannot save to file type \"" << typeMIME << '"');
    return true;
  }

  // see if we need a beep
  if (element.GetAttribute("beep").ToLower() *= "true") {
    PBYTEArray beepData;
    GetBeepData(beepData, 1000);
    if (beepData.GetSize() != 0)
      PlayData(beepData);
  }

  m_recordingName = element.GetAttribute(NameAttribute);

  PFilePath destination;

  // Get the destination filename (dest) which is a private extension, not standard VXML
  if (element.HasAttribute(DestAttribute)) {
    PURL uri;
    if (uri.Parse(element.GetAttribute(DestAttribute), "file"))
      destination = uri.AsFilePath();
  }
  else if (element.HasAttribute(DestExprAttribute)) {
    PURL uri;
    if (uri.Parse(EvaluateExpr(element.GetAttribute(DestExprAttribute)), "file"))
      destination = uri.AsFilePath();
  }

  if (destination.IsEmpty()) {
    if (!m_recordDirectory.Create()) {
      PTRACE(2, "Could not create recording directory \"" << m_recordDirectory << '"');
    }

    PStringStream fn;
    fn << m_recordDirectory;
    if (m_recordingName.IsEmpty())
      fn << "recording";
    else
      fn << m_recordingName;
    fn << '_' << PTime().AsString("yyyyMMdd_hhmmss") << supportedFileType;
    destination = fn;
  }

  if (!m_recordingName.IsEmpty()) {
    SetVar(m_recordingName + "$.type", typeMIME);
    SetVar(m_recordingName + "$.uri", PURL(destination));
    SetVar(m_recordingName + "$.maxtime", "false");
    SetVar(m_recordingName + "$.termchar", ' ');
    SetVar(m_recordingName + "$.duration" , '0');
    SetVar(m_recordingName + "$.size", '0');
  }

  // Disable stop on DTMF if attribute explicitly false, default is true
  m_recordStopOnDTMF = !(element.GetAttribute("dtmfterm") *= "false");

  PFile::Remove(destination);

  PVXMLRecordableFilename * recordable = new PVXMLRecordableFilename();
  if (!recordable->Open(destination)) {
    delete recordable;
    return true;
  }

  recordable->SetFinalSilence(StringToTime(element.GetAttribute("finalsilence"), 3000));
  recordable->SetMaxDuration(StringToTime(element.GetAttribute("maxtime"), INT_MAX));

  if (!GetVXMLChannel()->QueueRecordable(recordable))
    return true;

  m_recordingStatus = RecordingInProgress;
  m_recordingStartTime.SetCurrentTime();
  return false; // Are recording, stay in <record> element
}


PBoolean PVXMLSession::TraverseScript(PXMLElement & element)
{
#if P_SCRIPTS
  if (m_scriptContext != NULL) {
    PString src = element.GetAttribute(SrcAttribute);
    PString data = element.GetData();
    PString script = src.IsEmpty() ? data : src;

    PTRACE(4, "Traverse <script> " << script.ToLiteral());
  
    if (m_scriptContext->Run(script))
      PTRACE(4, m_scriptContext->GetLanguageName() << " executed properly!");
    else
      PTRACE(2, "Could not evaluate script \"" << script << "\" with script language " << m_scriptContext->GetLanguageName());
  }
#else
  PTRACE(2, "Unsupported <script> element");
#endif

  return false;
}


static bool IsFunction(const PString & expr, PINDEX offset, PINDEX & endname, PINDEX & startarg, PINDEX & endarg)
{
  if (!isalpha(expr[offset]) && expr[offset] != '_')
    return false;

  PINDEX pos = offset + 1;
  while (isalnum(expr[pos]) || expr[pos] == '_')
    ++pos;
  endname = pos - 1;

  while (iswspace(expr[pos]))
    ++pos;

  if (expr[pos] != '(')
    return false;

  startarg = pos + 1;

  endarg = expr.Find(')', startarg);
  if (endarg != P_MAX_INDEX)
    --endarg;
  else
    PTRACE(2, "Mismatched parenthesis");

  return true;
}


PString PVXMLSession::EvaluateExpr(const PString & expr)
{
  if (expr.IsEmpty())
    return PString::Empty();

#if P_SCRIPTS
  if (m_scriptContext != NULL) {
    static PConstString const EvalVarName("PTLibEvaluateExpressionResult");
    if (m_scriptContext->Run(PSTRSTRM(EvalVarName<<'='<<expr)))
      return m_scriptContext->GetString(EvalVarName);
    PTRACE(2, "Could not evaluate expression \"" << expr << "\" with script language " << m_scriptContext->GetLanguageName());
    return PString::Empty();
  }
#endif

  /* If we don't have full ECMAScript, or any other script engine for that matter ...
     We only support extremeley limited expressions, generally of the form
     'literal' or all numeric digits, with + signs allow concatenation of
     those elements. */

  PString result;

  PINDEX pos = 0;
  while (pos < expr.GetLength()) {
    PINDEX endname, startarg, endarg;
    if (IsFunction(expr, pos, endname, startarg, endarg)) {
      PString funcname = expr(pos, endname);
      PString expression = expr(startarg, endarg);
      PTRACE(4, "Found " << funcname << " function: expression=\"" << expression << "\"");
      if (funcname == "eval")
        result += EvaluateExpr(EvaluateExpr(expression));
#if P_VXML_VIDEO
      else if (funcname == SIGN_LANGUAGE_PREVIEW_SCRIPT_FUNCTION)
        SetRealVideoSender(new PVXMLSignLanguageAnalyser::PreviewVideoDevice(m_videoReceiver));
      else if (funcname == SIGN_LANGUAGE_CONTROL_SCRIPT_FUNCTION)
          PVXMLSignLanguageAnalyser::GetInstance().Control(m_videoReceiver.GetAnalayserInstance(), EvaluateExpr(expression));
#endif
      pos = endarg + 2;
    }
    else if (expr[pos] == '\'') {
      PINDEX quote = expr.Find('\'', ++pos);
      PTRACE_IF(2, quote == P_MAX_INDEX, "Mismatched quote, ignoring transfer");
      result += expr(pos, quote-1);
      pos = quote+1;
    }
    else if (isalpha(expr[pos])) {
      PINDEX span = expr.FindSpan("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789_.$", pos);
      result += GetVar(expr(pos, span-1));
      pos = span;
    }
    else if (isdigit(expr[pos])) {
      PINDEX span = expr.FindSpan("0123456789", pos);
      result += GetVar(expr(pos, span-1));
      pos = span;
    }
    else if (expr[pos] == '+' || isspace(expr[pos]))
      pos++;
    else {
      PTRACE(2, "Only '+' operator supported.");
      break;
    }
  }

  return result;
}


PStringToString PVXMLSession::GetVariables() const
{
#if P_SCRIPTS
  if (m_scriptContext != NULL) {
    PStringToString vars;
    for (PStringToString::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it)
      vars.SetAt(it->first, m_scriptContext->GetString(it->first));
    return vars;
  }
#endif

  return m_variables;
}


PCaselessString PVXMLSession::GetVar(const PString & varName) const
{
  // Check for literal
  if (varName[0] == '\'' || varName[0] == '"') {
    PINDEX endPos = varName.GetLength() - 1;
    if (varName[endPos] == '\'' || varName[endPos] == '"')
      --endPos;
    return varName(1, endPos);
  }

  PString fullVarName = varName;
  if (varName.Find('.') == P_MAX_INDEX)
    fullVarName = m_variableScope+'.'+varName;

#if P_SCRIPTS
  if (m_scriptContext != NULL)
    return m_scriptContext->GetString(fullVarName);
#endif

  PString value = m_variables(fullVarName);
  PTRACE(4, "GetVar[" << fullVarName << "]=" << value.ToLiteral());
  return value;
}


void PVXMLSession::SetVar(const PString & varName, const PString & value)
{
  PString fullVarName = varName;
  if (varName.Find('.') == P_MAX_INDEX)
    fullVarName = m_variableScope+'.'+varName;

#if P_SCRIPTS
  if (m_scriptContext != NULL) {
    m_scriptContext->SetString(fullVarName, value);
    m_variables.SetAt(fullVarName, PString::Empty()); // Just to remember what was set, value is always from script
    return;
  }
#endif

  m_variables.SetAt(fullVarName, value);
  PTRACE(4, "SetAt [" << fullVarName << "]=" << value.ToLiteral());
}


PBoolean PVXMLSession::PlayFile(const PString & fn, PINDEX repeat, PINDEX delay, PBoolean autoDelete)
{
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueueFile(fn, repeat, delay, autoDelete);
}


PBoolean PVXMLSession::PlayCommand(const PString & cmd, PINDEX repeat, PINDEX delay)
{
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueueCommand(cmd, repeat, delay);
}


PBoolean PVXMLSession::PlayData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueueData(data, repeat, delay);
}


PBoolean PVXMLSession::PlayTone(const PString & toneSpec, PINDEX repeat, PINDEX delay)
{
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueuePlayable("Tone", toneSpec, repeat, delay, true);
}


PBoolean PVXMLSession::PlayElement(PXMLElement & element)
{
  if (m_bargingIn)
    return false;

  PString str = element.GetAttribute(SrcAttribute).Trim();
  if (str.IsEmpty()) {
    str = EvaluateExpr(element.GetAttribute(ExprAttribute));
    if (str.IsEmpty()) {
      PTRACE(2, "No src attribute to play element.");
      return false;
    }
  }

  if (str[0] == '|')
    return PlayCommand(str.Mid(1));

  // files on the local file system get loaded locally
  PURL url(str);
  if (url.GetScheme() == "file" && url.GetHostName().IsEmpty())
    return PlayFile(url.AsFilePath());

  // get a normalised name for the resource
  bool safe = GetVar("caching") == "safe" || (element.GetAttribute("caching") *= "safe");

  PString fileType;
  {
    const PStringArray & path = url.GetPath();
    if (!path.IsEmpty())
      fileType = PFilePath(path[path.GetSize() - 1]).GetType();
    else
      fileType = ".dat";
  }

  if (!safe) {
    PFilePath filename;
    if (GetCache().Get("url", url.AsString(), fileType, filename))
      return PlayFile(filename);
  }

  PBYTEArray data;
  if (!url.LoadResource(data)) {
    PTRACE(2, "Cannot load resource " << url);
    return false;
  }

  PFile cacheFile;
  if (!GetCache().PutWithLock("url", url.AsString(), fileType, cacheFile))
    return false;

  // write the data in the file
  cacheFile.Write(data.GetPointer(), data.GetSize());

  GetCache().UnlockReadWrite();
  return PlayFile(cacheFile.GetFilePath(), 1, 0, safe);   // make sure we delete the file if not cacheing
}


void PVXMLSession::GetBeepData(PBYTEArray & data, unsigned ms)
{
  if (IsOpen())
    GetVXMLChannel()->GetBeepData(data, ms);
}


PBoolean PVXMLSession::PlaySilence(const PTimeInterval & timeout)
{
  return PlaySilence((PINDEX)timeout.GetMilliSeconds());
}


PBoolean PVXMLSession::PlaySilence(PINDEX msecs)
{
  PBYTEArray nothing;
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueueData(nothing, 1, msecs);
}


PBoolean PVXMLSession::PlayStop()
{
  return IsOpen() && GetVXMLChannel()->QueuePlayable(new PVXMLPlayableStop());
}


PBoolean PVXMLSession::PlayResource(const PURL & url, PINDEX repeat, PINDEX delay)
{
  return IsOpen() && !m_bargingIn && GetVXMLChannel()->QueueResource(url, repeat, delay);
}


PBoolean PVXMLSession::LoadGrammar(PVXMLGrammar * grammar)
{
  PTRACE_IF(2, m_grammar != NULL && grammar == NULL, "Grammar cleared from " << *m_grammar);

  delete m_grammar;
  m_grammar = grammar;
  ClearBargeIn();

  PTRACE_IF(2, grammar != NULL, "Grammar set to " << *grammar);
  return true;
}


PBoolean PVXMLSession::PlayText(const PString & textToPlay,
                    PTextToSpeech::TextType type,
                                     PINDEX repeat,
                                     PINDEX delay)
{
  if (!IsOpen() || textToPlay.IsEmpty() || m_bargingIn)
    return false;

  PTRACE(5, "Converting \"" << textToPlay << "\" to speech");

  PString prefix(PString::Printf, "tts%i", type);
  bool useCache = GetVar("caching") != "safe";

  PStringArray fileList;

  PString suffix = GetVXMLChannel()->GetMediaFileSuffix() + ".wav";

  // Convert each line into it's own cached WAV file.
  PStringArray lines = textToPlay.Lines();
  for (PINDEX i = 0; i < lines.GetSize(); i++) {
    PString line = lines[i].Trim();
    if (line.IsEmpty())
      continue;

    // see if we have converted this text before
    if (useCache) {
      PFilePath cachedFilename;
      if (GetCache().Get(prefix, line, suffix, cachedFilename)) {
        fileList.AppendString(cachedFilename);
        continue;
      }
    }

    PFile wavFile;
    if (!GetCache().PutWithLock(prefix, line, suffix, wavFile))
      continue;

    // Really want to use OpenChannel() but it isn't implemented yet.
    // So close file and just use filename.
    wavFile.Close();

    bool ok = m_textToSpeech->SetSampleRate(GetVXMLChannel()->GetSampleRate()) &&
              m_textToSpeech->SetChannels(GetVXMLChannel()->GetChannels()) &&
              m_textToSpeech->OpenFile(wavFile.GetFilePath()) &&
              m_textToSpeech->Speak(line, type) &&
              m_textToSpeech->Close();

    GetCache().UnlockReadWrite();

    if (ok)
      fileList.AppendString(wavFile.GetFilePath());
  }

  PVXMLPlayableFileList * playable = new PVXMLPlayableFileList;
  if (!playable->Open(*GetVXMLChannel(), fileList, delay, repeat, !useCache)) {
    delete playable;
    PTRACE(1, "Cannot create playable for filename list");
    return false;
  }

  if (!GetVXMLChannel()->QueuePlayable(playable))
    return false;

  PTRACE(2, "Queued filename list for playing");

  return true;
}


void PVXMLSession::SetPause(PBoolean pause)
{
  if (IsOpen())
    GetVXMLChannel()->SetPause(pause);
}


PBoolean PVXMLSession::TraverseBlock(PXMLElement & element)
{
  return ExecuteCondition(element);
}


PBoolean PVXMLSession::TraverseAudio(PXMLElement & element)
{
  if (PlayElement(element))
    return false; // Playing files, so don't process children

#if P_VXML_VIDEO
  SetRealVideoSender(NULL);
#endif // P_VXML_VIDEO
  return true;
}


PBoolean PVXMLSession::TraverseBreak(PXMLElement & element)
{
  // msecs is VXML 1.0
  if (element.HasAttribute("msecs"))
    return PlaySilence(element.GetAttribute("msecs").AsInteger());

  // time is VXML 2.0
  if (element.HasAttribute("time"))
    return PlaySilence(StringToTime(element.GetAttribute("time"), 1000));

  if (element.HasAttribute("size")) {
    PString size = element.GetAttribute("size");
    if (size *= "none")
      return true;
    if (size *= "small")
      return PlaySilence(SMALL_BREAK_MSECS);
    if (size *= "large")
      return PlaySilence(LARGE_BREAK_MSECS);
    return PlaySilence(MEDIUM_BREAK_MSECS);
  }

  // default to medium pause
  return PlaySilence(MEDIUM_BREAK_MSECS);
}


PBoolean PVXMLSession::TraverseValue(PXMLElement & element)
{
  PString className = element.GetAttribute("class");
  PString value = EvaluateExpr(element.GetAttribute(ExprAttribute));
  PString voice = element.GetAttribute("voice");
  if (voice.IsEmpty())
    voice = GetVar("voice");
  SayAs(className, value, voice);
  return true;
}


PBoolean PVXMLSession::TraverseSayAs(PXMLElement & element)
{
  SayAs(element.GetAttribute("class"), element.GetData());
  return true;
}


PBoolean PVXMLSession::TraverseGoto(PXMLElement & element)
{
  bool fullURI = false;
  PString target;

  if (element.HasAttribute("nextitem"))
    target = element.GetAttribute("nextitem");
  else if (element.HasAttribute("expritem"))
    target = EvaluateExpr(element.GetAttribute("expritem"));
  else if (element.HasAttribute(ExprAttribute)) {
    fullURI = true;
    target = EvaluateExpr(element.GetAttribute(ExprAttribute));
  }
  else if (element.HasAttribute(NextAttribute)) {
    fullURI = true;
    target = element.GetAttribute(NextAttribute);
  }

  if (SetCurrentForm(target, fullURI))
    return ProcessNode();

  // LATER: throw "error.semantic" or "error.badfetch" -- lookup which
  return false;
}


PBoolean PVXMLSession::TraverseGrammar(PXMLElement & element)
{
  // LATER: A bunch of work to do here!

  // For now we only support the builtin digits type and do not parse any grammars.

  // NOTE: For now we will process both <grammar> and <field> here.
  // NOTE: Later there needs to be a check for <grammar> which will pull
  //       out the text and process a grammar like '1 | 2'

  // Right now we only support one active grammar.
  if (m_grammar != NULL) {
    PTRACE(2, "Warning: can only process one grammar at a time, ignoring previous grammar");
    LoadGrammar(NULL);
  }

  m_speakNodeData = false;

  PCaselessString attrib = element.GetAttribute("mode");
  if (!attrib.IsEmpty() && attrib != DtmfAttribute) {
    PTRACE(2, "Only DTMF mode supported for grammar");
    return false;
  }

  attrib = element.GetAttribute("type");
  if (!attrib.IsEmpty() && attrib != "X-OPAL/digits") {
    PTRACE(2, "Only \"digits\" type supported for grammar");
    return false;
  }

  PTRACE(4, "Loading new grammar");
  PStringOptions tokens;
  PURL::SplitVars(element.GetData(), tokens, ';', '=');
  return LoadGrammar(new PVXMLDigitsGrammar(*this,
                                            *element.GetParent(),
                                            tokens.GetInteger("minDigits", 1),
                                            tokens.GetInteger("maxDigits", 10),
                                            tokens.GetString("terminators", "#")));
}


// Finds the proper event hander for 'noinput', 'filled', 'nomatch' and 'error'
// by searching the scope hiearchy from the current from
bool PVXMLSession::GoToEventHandler(PXMLElement & element, const PString & eventName)
{
  PXMLElement * level = &element;
  PXMLElement * handler = NULL;

  unsigned actualCount = ++m_eventCount[eventName];

  // Look in all the way up the tree for a handler either explicitly or in a catch
  while ((handler = FindElementWithCount(*level, eventName, actualCount)) == NULL) {
    level = level->GetParent();
    if (level == NULL) {
      PTRACE(4, "No event handler found for \"" << eventName << '"');
      return false;
    }
  }

  handler->SetAttribute(InternalEventStateAttribute, "true");
  m_currentNode = handler;
  PTRACE(4, "Setting event handler to node " << handler->PrintTrace() << " for \"" << eventName << '"');
  return true;
}


static unsigned GetCountAttribute(PXMLElement & element)
{
  PString str = element.GetAttribute("count");
  return str.IsEmpty() ? 1 : str.AsUnsigned();
}


PXMLElement * PVXMLSession::FindElementWithCount(PXMLElement & parent, const PString & name, unsigned count)
{
  std::map<unsigned, PXMLElement *> elements;
  PXMLElement * element;
  PINDEX idx = 0;
  while ((element = parent.GetElement(idx++)) != NULL) {
    if (element->GetName() == name)
      elements[GetCountAttribute(*element)] = element;
    else if (element->GetName() == "catch") {
      PStringArray events = element->GetAttribute("event").Tokenise(" ", false);
      for (size_t i = 0; i < events.size(); ++i) {
        if (events[i] *= name) {
          elements[GetCountAttribute(*element)] = element;
          break;
        }
      }
    }
  }

  if (elements.empty())
    return NULL;

  while (count > 0) {
    std::map<unsigned, PXMLElement *>::iterator it = elements.find(count);
    if (it != elements.end() && ExecuteCondition(*it->second))
      return it->second;
    --count;
  }

  return NULL;
}


void PVXMLSession::SayAs(const PString & className, const PString & text)
{
  SayAs(className, text, GetVar("voice"));
}


void PVXMLSession::SayAs(const PString & className, const PString & textToSay, const PString & voice)
{
  if (m_textToSpeech != NULL)
    m_textToSpeech->SetVoice(voice);

  PString text = textToSay.Trim();
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


PTimeInterval PVXMLSession::StringToTime(const PString & str, int dflt)
{
  if (str.IsEmpty())
    return dflt;

  PCaselessString units = str.Mid(str.FindSpan("0123456789")).Trim();
  if (units ==  "s")
    return PTimeInterval(0, str.AsInteger());
  else if (units ==  "m")
    return PTimeInterval(0, 0, str.AsInteger());
  else if (units ==  "h")
    return PTimeInterval(0, 0, 0, str.AsInteger());

  return str.AsInt64();
}


bool PVXMLSession::ExecuteCondition(PXMLElement & element)
{
  PString condition = element.GetAttribute("cond");
  if (condition.IsEmpty())
    return true;

  bool result;

#if P_SCRIPTS
  if (m_scriptContext != NULL)
    result = EvaluateExpr(condition) == "true";
  else
#endif
  {
    // Find comparison type
    PINDEX location = condition.Find("==");
    if (location == P_MAX_INDEX) {
      location = condition.Find("!=");
      if (location == P_MAX_INDEX) {
        PTRACE(1, "<if> element contains condition with operator other than == or !=, not implemented");
        return false;
      }
    }

    PString leftExpr = EvaluateExpr(condition.Left(location).Trim());
    PString rightExpr = EvaluateExpr(condition.Mid(location + 2).Trim());
    if (condition[location] == '=')
      result = leftExpr == rightExpr;
    else
      result = leftExpr != rightExpr;
  }

  PTRACE(4, "\tCondition \"" << condition << "\" is " << ::boolalpha << result);
  return result;
}


static PXMLObject * NextElseIfNode(PXMLElement & ifElement, PINDEX pos)
{
  PXMLObject * previousObject = ifElement.GetSubObject(pos);
  for (PINDEX i = pos; i < ifElement.GetSize(); ++i) {
    PXMLObject * thisObject = ifElement.GetSubObject(i);
    if (thisObject->IsElement()) {
      PXMLElement * thisElement = dynamic_cast<PXMLElement *>(thisObject);
      if (thisElement->GetName() == "else")
        return thisObject;
      if (thisElement->GetName() == "elseif") {
        if (previousObject->IsElement()) {
          // Need a non-element as previous, so NextNode() logic works
          previousObject = new PXMLData(PString::Empty());
          previousObject->SetParent(&ifElement);
          ifElement.GetSubObjects().InsertAt(i, previousObject);
        }
        return previousObject;
      }
    }
    previousObject = thisObject;
  }

  return NULL;
}


static PConstString const IfStateAttributeName("PTLibInternalIfState");

PBoolean PVXMLSession::TraverseIf(PXMLElement & element)
{
  // If 'cond' parameter evaluates to true, enter child entities, else
  // go to next element.

  if (ExecuteCondition(element)) {
    element.SetAttribute(IfStateAttributeName, "done");
    return true;
  }

  PXMLObject * nextNode = NextElseIfNode(element, 0);
  if (nextNode == NULL)
    return false; // Skip if subelements

  m_currentNode = nextNode;
  return true; // Do subelements
}


PBoolean PVXMLSession::TraversedIf(PXMLElement & element)
{
  element.SetAttribute(IfStateAttributeName, PString::Empty());
  return true;
}


PBoolean PVXMLSession::TraverseElseIf(PXMLElement & element)
{
  PXMLElement * ifElement = element.GetParent();
  PString state = ifElement->GetAttribute(IfStateAttributeName);
  if (state == "done") {
    m_currentNode = element.GetParent();
    return false;
  }

  if (ExecuteCondition(element)) {
    ifElement->SetAttribute(IfStateAttributeName, "done");
    return true;
  }

  PXMLObject * nextNode = NextElseIfNode(*ifElement, ifElement->FindObject(&element)+1);
  if (nextNode == NULL)
    return false; // Skip if subelements

  m_currentNode = nextNode;
  return true; // Do subelements
}


PBoolean PVXMLSession::TraverseElse(PXMLElement & element)
{
  m_currentNode = element.GetParent();
  return false;
}


PBoolean PVXMLSession::TraverseExit(PXMLElement &)
{
  PTRACE(2, "Exiting, fast forwarding through script");
  m_abortVXML = true;
  Trigger();
  return true;
}


PBoolean PVXMLSession::TraverseSubmit(PXMLElement & element)
{
  PURL url;

  if (element.HasAttribute(ExprAttribute))
    url.Parse(EvaluateExpr(element.GetAttribute(ExprAttribute)));
  else if (element.HasAttribute(NextAttribute))
    url.Parse(element.GetAttribute(NextAttribute));
  else {
    PTRACE(1, "<submit> does not contain \"next\" or \"expr\" attribute.");
    return false;
  }
  if (url.IsEmpty()) {
    PTRACE(1, "<submit> has an invalid URL.");
    return false;
  }

  bool urlencoded;
  PCaselessString str = element.GetAttribute("enctype");
  if (str.IsEmpty() || str == PHTTP::FormUrlEncoded())
    urlencoded = true;
  else if (str == "multipart/form-data")
    urlencoded = false;
  else {
    PTRACE(1, "<submit> has unknown \"enctype\" attribute of \"" << str << '"');
    return false;
  }

  bool get;
  str = element.GetAttribute("method");
  if (str.IsEmpty())
    get = urlencoded;
  else if (str == "GET")
    get = true;
  else if (str == "POST")
    get = false;
  else {
    PTRACE(1, "<submit> has unknown \"method\" attribute of \"" << str << '"');
    return false;
  }

  PHTTPClient client("PTLib VXML");
  client.SetReadTimeout(StringToTime(element.GetAttribute("fetchtimeout"), 10000));

  PStringArray namelist = element.GetAttribute("namelist").Tokenise(" \t", false);

  if (get) {
    if (namelist.IsEmpty())
      url.SetQueryVars(GetVariables());
    else {
      for (PINDEX i = 0; i < namelist.GetSize(); ++i)
        url.SetQueryVar(namelist[i], GetVar(namelist[i]));
    }

    PMIMEInfo replyMIME;
    PString body;
    if (client.GetTextDocument(url, body)) {
      PTRACE(4, "<submit> GET " << url << " succeeded and returned body:\n" << body);
      return InternalLoadVXML(body, PString::Empty());
    }

    PTRACE(1, "<submit> GET " << url << " failed with "
           << client.GetLastResponseCode() << ' ' << client.GetLastResponseInfo());
    return false;
  }

  if (urlencoded) {
    PStringToString vars;
    if (namelist.IsEmpty())
      vars = GetVariables();
    else {
      for (PINDEX i = 0; i < namelist.GetSize(); ++i)
        vars.SetAt(namelist[i], GetVar(namelist[i]));
    }

    PMIMEInfo replyMIME;
    PString replyBody;
    if (client.PostData(url, vars, replyMIME, replyBody)) {
      PTRACE(4, "<submit> POST " << url << " succeeded and returned body:\n" << replyBody);
      return InternalLoadVXML(replyBody, PString::Empty());
    }

    PTRACE(1, "<submit> POST " << url << " failed with "
           << client.GetLastResponseCode() << ' ' << client.GetLastResponseInfo());
    return false;
  }

  PMIMEInfo sendMIME;

  // Put in boundary
  PString boundary = "--------012345678901234567890123458VXML";
  sendMIME.SetAt( PHTTP::ContentTypeTag(), "multipart/form-data; boundary=" + boundary);

  // After this all boundaries have a "--" prepended
  boundary.Splice("--", 0, 0);

  PStringStream entityBody;

  for (PINDEX i = 0; i < namelist.GetSize(); ++i) {
    PString type = GetVar(namelist[i] + ".type");
    if (type.IsEmpty()) {
      PMIMEInfo part1;

      part1.Set(PMIMEInfo::ContentTypeTag, PMIMEInfo::TextPlain());
      part1.Set(PMIMEInfo::ContentDispositionTag, "form-data; name=\"" + namelist[i] + "\"; ");

      entityBody << "--" << boundary << "\r\n"
                 << part1 << GetVar(namelist[i]) << "\r\n";

      continue;
    }

    if (GetVar(namelist[i] + ".type") != "audio/wav" ) {
      PTRACE(1, "<submit> does not (yet) support submissions of types other than \"audio/wav\"");
      continue;
    }

    PFile file(GetVar(namelist[i] + ".filename"), PFile::ReadOnly);
    if (!file.IsOpen()) {
      PTRACE(1, "<submit> could not find file \"" << file.GetFilePath() << '"');
      continue;
    }

    PMIMEInfo part1, part2;
    part1.Set(PMIMEInfo::ContentTypeTag, "audio/wav");
    part1.Set(PMIMEInfo::ContentDispositionTag,
              "form-data; name=\"voicemail\"; filename=\"" + file.GetFilePath().GetFileName() + '"');
    // Make PHP happy?
    // Anyway, this shows how to add more variables, for when namelist containes more elements
    part2.Set(PMIMEInfo::ContentDispositionTag, "form-data; name=\"MAX_FILE_SIZE\"\r\n\r\n3000000");

    entityBody << "--" << boundary << "\r\n"
               << part1 << "\r\n"
               << file.ReadString(file.GetLength())
               << "--" << boundary << "\r\n"
               << part2
               << "\r\n";
  }

  if (entityBody.IsEmpty()) {
    PTRACE(1, "<submit> could not find anything to send using \"" << setfill(',') << namelist << '"');
    return false;
  }

  PMIMEInfo replyMIME;
  PString replyBody;
  if (client.PostData(url, sendMIME, entityBody, replyMIME, replyBody)) {
    PTRACE(1, "<submit> POST " << url << " succeeded and returned body:\n" << replyBody);
    return InternalLoadVXML(replyBody, PString::Empty());
  }

  PTRACE(1, "<submit> POST " << url << " failed with "
         << client.GetLastResponseCode() << ' ' << client.GetLastResponseInfo());
  return false;
}


PBoolean PVXMLSession::TraverseProperty(PXMLElement & element)
{
  if (element.HasAttribute(NameAttribute))
    SetVar("property." + element.GetAttribute(NameAttribute), element.GetAttribute("value"));

  return true;
}


PBoolean PVXMLSession::TraverseTransfer(PXMLElement & element)
{
  if (!ExecuteCondition(element))
    return false;

  m_transferStatus = NotTransfering;
  return true;
}


PBoolean PVXMLSession::TraversedTransfer(PXMLElement & element)
{
  const char * eventName = "error";

  switch (m_transferStatus) {
    case TransferCompleted :
      return true;

    case NotTransfering :
    {
      TransferType type = BridgedTransfer;
      if (element.GetAttribute("bridge") *= "false")
        type = BlindTransfer;
      else {
        PCaselessString typeStr = element.GetAttribute("type");
        if (typeStr == "blind")
          type = BlindTransfer;
        else if (typeStr == "consultation")
          type = ConsultationTransfer;
      }

      m_transferStartTime.SetCurrentTime();

      bool started = false;
      if (element.HasAttribute(DestAttribute))
        started = OnTransfer(element.GetAttribute(DestAttribute), type);
      else if (element.HasAttribute(DestExprAttribute))
        started = OnTransfer(EvaluateExpr(element.GetAttribute(DestExprAttribute)), type);

      if (started) {
        m_transferStatus = TransferInProgress;
        return false;
      }
      break;
    }

    case TransferSuccessful :
      eventName = FilledElement;
      // Do default case

    default :
      PString name = element.GetAttribute(NameAttribute);
      if (!name.IsEmpty())
        SetVar(name + "$.duration", PString(PString::Unsigned, (PTime() - m_transferStartTime).GetSeconds()));
  }

  m_transferStatus = TransferCompleted;

  return !GoToEventHandler(element, eventName);
}


void PVXMLSession::SetTransferComplete(bool state)
{
  PTRACE(3, "Transfer " << (state ? "completed" : "failed"));
  m_transferStatus = state ? TransferSuccessful : TransferFailed;
  Trigger();
}


PBoolean PVXMLSession::TraverseMenu(PXMLElement & element)
{
  LoadGrammar(new PVXMLMenuGrammar(*this, element));
  m_defaultMenuDTMF = (element.GetAttribute(DtmfAttribute) *= "true") ? '1' : 'N';
  ++m_promptCount;
  return true;
}


PBoolean PVXMLSession::TraversedMenu(PXMLElement &)
{
  if (m_grammar != NULL)
    m_grammar->Start();
  return false;
}


PBoolean PVXMLSession::TraverseChoice(PXMLElement & element)
{
  if (!element.HasAttribute(DtmfAttribute) && m_defaultMenuDTMF <= '9')
    element.SetAttribute(DtmfAttribute, PString(m_defaultMenuDTMF++));

  return true;
}


PBoolean PVXMLSession::TraverseVar(PXMLElement & element)
{
  PString name = element.GetAttribute(NameAttribute);
  PString expr = element.GetAttribute(ExprAttribute);

  if (name.IsEmpty() || expr.IsEmpty()) {
    PTRACE(1, "<var> must have both \"name=\" and \"expr=\" attributes." );
    return false;
  }

  SetVar(name, EvaluateExpr(expr));
  return true;
}


PBoolean PVXMLSession::TraverseDisconnect(PXMLElement &)
{
  m_currentNode = NULL;
  return true;
}


PBoolean PVXMLSession::TraverseForm(PXMLElement &)
{
  m_variableScope = DialogScope;
  ++m_promptCount;
  return true;
}


PBoolean PVXMLSession::TraversedForm(PXMLElement &)
{
  m_variableScope = ApplicationScope;
  return true;
}


PBoolean PVXMLSession::TraversePrompt(PXMLElement & element)
{
  PXMLElement * validPrompt = FindElementWithCount(*element.GetParent(), PromptElement, m_promptCount);
  if (validPrompt == NULL || GetCountAttribute(*validPrompt) != GetCountAttribute(element)) {
    PTRACE(3, "Prompt count/cond attribute preventing execution: " << element.PrintTrace());
    return false;
  }

  // Update timeout of current recognition (if 'timeout' attribute is set)
  if (m_grammar != NULL)
    m_grammar->SetTimeout(StringToTime(element.GetAttribute("timeout")));

  if ((element.GetAttribute("bargein") *= "false") || GetVar("property.bargein") == "false") {
    PTRACE(3, "Prompt bargein disabled.");
    m_bargeIn = false;
    ClearBargeIn();
    FlushInput();
  }
  return true;
}


PBoolean PVXMLSession::TraversedPrompt(PXMLElement &)
{
  m_bargeIn = GetVar("property.bargein") != "false";
  return true;
}


PBoolean PVXMLSession::TraverseField(PXMLElement & element)
{
  return ExecuteCondition(element);
}


PBoolean PVXMLSession::TraversedField(PXMLElement &)
{
  if (m_grammar != NULL)
    m_grammar->Start();
  return false;
}


void PVXMLSession::OnEndRecording(PINDEX bytesRecorded, bool timedOut)
{
  if (!m_recordingName.IsEmpty()) {
    SetVar(m_recordingName + "$.duration" , (PTime() - m_recordingStartTime).GetMilliSeconds());
    SetVar(m_recordingName + "$.size", bytesRecorded);
    SetVar(m_recordingName + "$.maxtime", timedOut ? "true" : "false");
  }

  m_recordingStatus = RecordingComplete;
  Trigger();
}


void PVXMLSession::Trigger()
{
  PTRACE(4, "Event triggered");
  m_waitForEvent.Signal();
}


#if P_VXML_VIDEO

bool PVXMLSession::SetSignLanguageAnalyser(const PString & dllName)
{
  return PVXMLSignLanguageAnalyser::GetInstance().SetAnalyser(dllName);
}


void PVXMLSession::SetRealVideoSender(PVideoInputDevice * device)
{
  if (device != NULL && !device->IsOpen()) {
    delete device;
    device = NULL;
  }

  if (device == NULL) {
    PVideoInputDevice::OpenArgs videoArgs;
    videoArgs.driverName = P_FAKE_VIDEO_DRIVER;
    videoArgs.deviceName = P_FAKE_VIDEO_TEXT "=" + PProcess::Current().GetName();
    device = PVideoInputDevice::CreateOpenedDevice(videoArgs);
  }

  m_videoSender.SetActualDevice(device);
}


void PVXMLSession::SignLanguagePreviewFunction(PScriptLanguage &, PScriptLanguage::Signature &)
{
  SetRealVideoSender(new PVXMLSignLanguageAnalyser::PreviewVideoDevice(m_videoReceiver));
}


void PVXMLSession::SignLanguageControlFunction(PScriptLanguage &, PScriptLanguage::Signature & sig)
{
  int instance = m_videoReceiver.GetAnalayserInstance();
  PString control;
  if (!sig.m_arguments.empty())
    control = sig.m_arguments.front().AsString();

  int result = PVXMLSignLanguageAnalyser::GetInstance().Control(instance, control);
  sig.m_results.push_back(PVarType(result));

  PTRACE(3, SIGN_LANGUAGE_CONTROL_SCRIPT_FUNCTION "(" << instance << ',' << control.ToLiteral() << ") = " << result);
}


PVXMLSession::VideoReceiverDevice::VideoReceiverDevice(PVXMLSession & vxmlSession)
  : m_vxmlSession(vxmlSession)
  , m_analayserInstance(PVXMLSignLanguageAnalyser::GetInstance().AllocateInstance())
{
  m_colourFormat = PVXMLSignLanguageAnalyser::GetInstance().GetColourFormat();
}


PStringArray PVXMLSession::VideoReceiverDevice::GetDeviceNames() const
{
  return "SignLanguageAnalyser";
}


PBoolean PVXMLSession::VideoReceiverDevice::Open(const PString &, PBoolean)
{
  return IsOpen();
}


PBoolean PVXMLSession::VideoReceiverDevice::IsOpen()
{
  return m_analayserInstance >= 0;
}


PBoolean PVXMLSession::VideoReceiverDevice::Close()
{
  if (!IsOpen())
    return false;

  if (PVXMLSignLanguageAnalyser::GetInstance().ReleaseInstance(m_analayserInstance)) {
    PTRACE(3, "Closing SignLanguageAnalyser instance " << m_analayserInstance);
  }

  m_analayserInstance = -1;
  return true;
}


PBoolean PVXMLSession::VideoReceiverDevice::SetColourFormat(const PString & colourFormat)
{
  return colourFormat == m_colourFormat;
}


PBoolean PVXMLSession::VideoReceiverDevice::SetFrameData(const FrameData & frameData)
{
  if (m_analayserInstance < 0 || frameData.partialFrame || frameData.x != 0 || frameData.y != 0)
    return false;

  const void * pixels;
  if (m_converter != NULL) {
    BYTE * storePtr = m_frameStore.GetPointer(m_converter->GetMaxDstFrameBytes());
    if (!m_converter->Convert(frameData.pixels, storePtr))
      return false;
    pixels = storePtr;
  }
  else
    pixels = frameData.pixels;

  int result = PVXMLSignLanguageAnalyser::GetInstance().Analyse(m_analayserInstance, frameData.width, frameData.height, frameData.sampleTime, pixels);
  if (result >= ' ') {
    PTRACE(4, "Sign language analyser detected " << result << " '" << (char)result << '\'');
    m_vxmlSession.OnUserInput((char)result);
  }

  return true;
}


PVXMLSession::VideoSenderDevice::VideoSenderDevice()
  : m_running(true)
{
}


void PVXMLSession::VideoSenderDevice::SetActualDevice(PVideoInputDevice * actualDevice, bool autoDelete)
{
  m_running = true;
  PVideoInputDeviceIndirect::SetActualDevice(actualDevice, autoDelete);
}


PBoolean PVXMLSession::VideoSenderDevice::Start()
{
  m_running = true;
  return PVideoInputDeviceIndirect::Start();
}


bool PVXMLSession::VideoSenderDevice::InternalGetFrameData(BYTE * buffer, PINDEX & bytesReturned, bool & keyFrame, bool wait)
{
  if (m_running && !PVideoInputDeviceIndirect::InternalGetFrameData(buffer, bytesReturned, keyFrame, wait))
    m_running = false;

  return true; // Always return true or upstream closes down straight away.
}
#endif // P_VXML_VIDEO


/////////////////////////////////////////////////////////////////////////////////////////

PVXMLGrammar::PVXMLGrammar(PVXMLSession & session, PXMLElement & field)
  : m_session(session)
  , m_field(field)
  , m_state(Idle)
{
  m_timer.SetNotifier(PCREATE_NOTIFIER(OnTimeout), "VXMLGrammar");
  SetSessionTimeout();
}


void PVXMLGrammar::SetSessionTimeout()
{
  SetTimeout(PVXMLSession::StringToTime(m_session.GetVar("property.timeout"), 10000));
}


void PVXMLGrammar::SetTimeout(const PTimeInterval & timeout)
{
  if (timeout > 0) {
    m_timeout = timeout;
    PTRACE(4, "Set timeout on " << *this << ", timeout=" << m_timeout << ", state=" << m_state);
    if (m_state != Idle)
      m_timer = timeout;
  }
}


void PVXMLGrammar::Start()
{
  SetSessionTimeout();
  m_state = Started;
  m_timer = m_timeout;
  PTRACE(3, "Started grammar " << *this << ", timeout=" << m_timeout);
}


void PVXMLGrammar::OnTimeout(PTimer &, P_INT_PTR)
{
  PTRACE(3, "Timeout for grammar " << *this );
  m_mutex.Wait();

  if (m_state == Started) {
    m_state = IsFilled() ? Filled : NoInput;
    m_session.Trigger();
  }

  m_mutex.Signal();
}


bool PVXMLGrammar::Process()
{
  // Figure out what happened
  switch (m_state) {
    case Filled:
      if (m_field.HasAttribute(NameAttribute))
        m_session.SetVar(m_field.GetAttribute(NameAttribute), m_value);
      return m_session.GoToEventHandler(m_field, FilledElement);

    case PVXMLGrammar::NoInput:
      return m_session.GoToEventHandler(m_field, "noinput");

    case PVXMLGrammar::NoMatch:
      return m_session.GoToEventHandler(m_field, "nomatch");

    default:
      break; //ERROR - unexpected grammar state
  }

  return true; // Next node
}


//////////////////////////////////////////////////////////////////

PVXMLMenuGrammar::PVXMLMenuGrammar(PVXMLSession & session, PXMLElement & field)
  : PVXMLGrammar(session, field)
{
}


void PVXMLMenuGrammar::OnUserInput(const char ch)
{
  m_mutex.Wait();

  m_value = ch;
  PTRACE(4, "Menu grammar filled with '" << ch << '\'');
  m_state = PVXMLGrammar::Filled;

  m_mutex.Signal();
}


bool PVXMLMenuGrammar::Process()
{
  if (m_state == Filled) {
    PXMLElement * choice;
    PINDEX index = 0;
    while ((choice = m_field.GetElement("choice", index++)) != NULL) {
      // Check if DTMF value for grammarResult matches the DTMF value for the choice
      if (choice->GetAttribute(DtmfAttribute) == m_value) {
        PTRACE(3, "Matched menu choice: " << m_value << " to " << choice->PrintTrace());

        PString next = choice->GetAttribute(NextAttribute);
        if (next.IsEmpty())
          next = m_session.EvaluateExpr(choice->GetAttribute(ExprAttribute));

        if (m_session.SetCurrentForm(next, true))
          return true;

        return m_session.GoToEventHandler(m_field, choice->GetAttribute("event"));
      }
    }

    PTRACE(3, "No match on menu choice: " << m_value.ToLiteral() << " on " << m_field.PrintTrace());
    m_state = NoMatch;
  }

  return PVXMLGrammar::Process();
}


//////////////////////////////////////////////////////////////////

PVXMLDigitsGrammar::PVXMLDigitsGrammar(PVXMLSession & session,
                                       PXMLElement & field,
                                       PINDEX minDigits,
                                       PINDEX maxDigits,
                                       PString terminators)
  : PVXMLGrammar(session, field)
  , m_minDigits(minDigits)
  , m_maxDigits(maxDigits)
  , m_terminators(terminators)
{
  PAssert(minDigits <= maxDigits, PInvalidParameter);
}


void PVXMLDigitsGrammar::OnUserInput(const char ch)
{
  PWaitAndSignal mutex(m_mutex);

  if (m_state == Idle)
    Start();

  // Ignore any keys unless we are running
  if (m_state != Started)
    return;

  // is this char the terminator?
  if (m_terminators.Find(ch) != P_MAX_INDEX) {
    m_state = IsFilled() ? Filled : NoMatch;
    return;
  }

  m_value += ch; // Add to collected digits string

  if (m_value.GetLength() >= m_maxDigits) {
    PTRACE(4, " Grammar " << *this << " has been FILLED, max=" << m_maxDigits);
    m_state = PVXMLGrammar::Filled;
  }
}


bool PVXMLDigitsGrammar::IsFilled()
{
  PINDEX len = m_value.GetLength();
  bool filled = len >= m_minDigits && len <= m_maxDigits;

  PTRACE(4, " Grammar " << *this << (filled ? " has been FILLED" : " has NOT yet been filled") << "."
            " Collected value=" << m_value << ", length: " << len << ", while min=" << m_minDigits << " max=" << m_maxDigits);

  return filled;
}

//////////////////////////////////////////////////////////////////

PVXMLChannel::PVXMLChannel(unsigned frameDelay, PINDEX frameSize)
  : PDelayChannel(DelayReadsAndWrites, frameDelay, frameSize)
  , m_vxmlSession(NULL)
  , m_closed(false)
  , m_paused(false)
  , m_totalData(0)
  , m_recordable(NULL)
  , m_currentPlayItem(NULL)
{
  PTRACE(4, "Constructed channel " << this);
}


PBoolean PVXMLChannel::Open(PVXMLSession * session, unsigned sampleRate, unsigned channels)
{
  PTRACE(4, "Opening channel:"
            " this=" << this << ","
            " session=" << session << ","
            " format=" << GetAudioFormat() << ","
            " rate=" << sampleRate << ","
            " channels=" << channels);

  m_currentPlayItem = NULL;
  m_vxmlSession = session;
  m_silenceTimer.SetInterval(500); // 1/2 a second delay before we start outputting stuff
  return SetSampleRate(sampleRate) && SetChannels(channels);
}


PVXMLChannel::~PVXMLChannel()
{
  Close();
  PTRACE(4, "Destroyed channel " << this);
}


PBoolean PVXMLChannel::IsOpen() const
{
  return !m_closed;
}


PBoolean PVXMLChannel::Close()
{
  if (!m_closed) {
    PTRACE(4, "Closing channel " << this);

    EndRecording(true);
    FlushQueue();

    m_closed = true;

    PDelayChannel::Close();
  }

  return true;
}


PString PVXMLChannel::GetMediaFileSuffix() const
{
  PStringStream suffix;

  if (!IsMediaPCM())
    suffix << '_' << GetAudioFormat().Replace(".", "", true).ToLower();
  else {
    switch (GetChannels()) {
      case 1:
        suffix << "_mono_";
        break;
      case 2:
        suffix << "_stereo_";
        break;
      default:
        suffix << '_' << GetChannels() << 'x';
    }
    suffix << PString(PString::ScaleSI, GetSampleRate(), 1) + "Hz";
  }

  return suffix;
}


bool PVXMLChannel::AdjustMediaFilename(const PFilePath & ifn, PFilePath & ofn)
{
  PFilePathString title = ifn.GetTitle();
  PFilePathString suffix = GetMediaFileSuffix();
  if (suffix != title.Right(suffix.GetLength())) {
    ofn = PSTRSTRM(ifn.GetDirectory() << title << suffix << ifn.GetType());
    if (PFile::Exists(ofn))
      return true;
  }

  if (PFile::Exists(ifn)) {
    ofn = ifn;
    return true;
  }

  PTRACE(2, "Playable file \"" << ifn << "\" not found.");
  return false;
}


PChannel * PVXMLChannel::OpenMediaFile(const PFilePath & fn, bool recording)
{
#if P_WAVFILE
  if (fn.GetType() == ".wav") {
    PWAVFile * wav = new PWAVFile;
    if (recording) {
      wav->SetChannels(GetChannels());
      wav->SetSampleSize(16);
      wav->SetSampleRate(GetSampleRate());
      if (!wav->SetFormat(GetAudioFormat()))
        PTRACE(2, "Unsupported codec " << GetAudioFormat());
      else if (!wav->Open(fn, PFile::WriteOnly))
        PTRACE(2, "Could not create WAV file \"" << wav->GetName() << "\" - " << wav->GetErrorText());
      else if (!wav->SetAutoconvert())
        PTRACE(2, "WAV file cannot convert to " << GetAudioFormat());
      else
        return wav;
    }
    else {
      if (!wav->Open(fn, PFile::ReadOnly))
        PTRACE(2, "Could not open WAV file \"" << wav->GetName() << "\" - " << wav->GetErrorText());
      else if (wav->GetFormatString() != GetAudioFormat() && !wav->SetAutoconvert())
        PTRACE(2, "WAV file cannot convert from " << wav->GetFormatString());
      else if (wav->GetChannels() != GetChannels())
        PTRACE(2, "WAV file has unsupported channel count " << wav->GetChannels());
      else if (wav->GetSampleSize() != 16)
        PTRACE(2, "WAV file has unsupported sample size " << wav->GetSampleSize());
      else if (wav->GetSampleRate() != GetSampleRate())
        PTRACE(2, "WAV file has unsupported sample rate " << wav->GetSampleRate());
      else
        return wav;
    }
    delete wav;
    return NULL;
  }
#endif // P_WAVFILE

#if P_MEDIAFILE
  PMediaFile::Ptr mediaFile = PMediaFile::Create(fn);
  if (!mediaFile.IsNULL()) {
    PSoundChannel * audio = new PMediaFile::SoundChannel(mediaFile);
    PSoundChannel::Params params;
    params.m_direction = recording ? PSoundChannel::Player : PSoundChannel::Recorder; // Counter intuitive
    params.m_driver = fn;
    params.m_sampleRate = GetSampleRate();
    params.m_channels = GetChannels();
    if (audio->Open(params)) {
#if P_VXML_VIDEO
      PVideoInputDevice * video = new PMediaFile::VideoInputDevice(mediaFile);
      video->SetChannel(PMediaFile::VideoInputDevice::Channel_PlayAndClose);
      if (video->Open(fn))
        m_vxmlSession->SetRealVideoSender(video);
      else
        delete video;
#endif //P_VXML_VIDEO
      return audio;
    }
    delete audio;
  }
  else
    PTRACE(2, "No media file handler for " << fn);
#endif // P_MEDIAFILE

  // Assume file just has bytes of correct media format
  PFile * file = new PFile(fn, recording ? PFile::WriteOnly : PFile::ReadOnly);
  if (file->Open(PFile::ReadOnly)) {
    return file;
  }

  PTRACE(2, "Could not open raw audio file \"" << fn << '"');
  delete file;
  return NULL;
}


PBoolean PVXMLChannel::Write(const void * buf, PINDEX len)
{
  if (m_closed)
    return false;

  m_recordingMutex.Wait();

  // let the recordable do silence detection
  if (m_recordable != NULL && m_recordable->OnFrame(IsSilenceFrame(buf, len)))
    EndRecording(true);

  m_recordingMutex.Signal();

  // write the data and do the correct delay
  if (WriteFrame(buf, len))
    m_totalData += GetLastWriteCount();
  else {
    EndRecording(true);
    SetLastWriteCount(len);
    Wait(len, nextWriteTick);
  }

  return true;
}


PBoolean PVXMLChannel::QueueRecordable(PVXMLRecordable * newItem)
{
  m_totalData = 0;

  // shutdown any existing recording
  EndRecording(true);

  // insert the new recordable
  PWaitAndSignal mutex(m_recordingMutex);
  m_recordable = newItem;
  m_totalData = 0;
  SetReadTimeout(frameDelay);
  return newItem->OnStart(*this);
}


PBoolean PVXMLChannel::EndRecording(bool timedOut)
{
  PWaitAndSignal mutex(m_recordingMutex);

  if (m_recordable == NULL)
    return false;

  PTRACE(3, "Finished recording " << m_totalData << " bytes");
  SetWriteChannel(NULL, false, true);
  m_recordable->OnStop();
  delete m_recordable;
  m_recordable = NULL;
  m_vxmlSession->OnEndRecording(m_totalData, timedOut);

  return true;
}


PBoolean PVXMLChannel::Read(void * buffer, PINDEX amount)
{
  for (;;) {
    if (m_closed)
      return false;

    if (m_paused || m_silenceTimer.IsRunning())
      break;

    // if the read succeeds, we are done
    if (ReadFrame(buffer, amount)) {
      m_totalData += GetLastReadCount();
#if P_VXML_VIDEO
      if (m_vxmlSession->m_videoSender.IsRunning())
        return true; // Already done real time delay
#endif
    }

    // if a timeout, send silence, try again in a bit
    if (GetErrorCode(LastReadError) == Timeout)
      break;

    // Other errors mean end of the playable
    PWaitAndSignal mutex(m_playQueueMutex);

    bool wasPlaying = false;

    // if current item still active, check for trailing actions
    if (m_currentPlayItem != NULL) {
      PTRACE(3, "Finished playing " << *m_currentPlayItem << ", " << m_totalData << " bytes");

      if (m_currentPlayItem->OnRepeat())
        continue;

      // see if end of queue delay specified
      if (m_currentPlayItem->OnDelay()) 
        break;

      // stop the current item
      m_currentPlayItem->OnStop();
      delete m_currentPlayItem;
      m_currentPlayItem = NULL;
      wasPlaying = true;
    }

    for (;;) {
      // check the queue for the next action, if none, send silence
      m_currentPlayItem = m_playQueue.Dequeue();
      if (m_currentPlayItem == NULL) {
        if (wasPlaying)
          m_vxmlSession->Trigger(); // notify about the end of queue
        goto double_break;
      }

      // start the new item
      if (m_currentPlayItem->OnStart())
        break;

      delete m_currentPlayItem;
    }

    PTRACE(4, "Started playing " << *m_currentPlayItem);
    SetReadTimeout(frameDelay);
    m_totalData = 0;
  }

double_break:
  SetLastReadCount(CreateSilenceFrame(buffer, amount));
  Wait(GetLastReadCount(), nextReadTick);
  return true;
}


void PVXMLChannel::SetSilence(unsigned msecs)
{
  PTRACE(3, "Playing silence for " << msecs << "ms");
  m_silenceTimer.SetInterval(msecs);
}


PBoolean PVXMLChannel::QueuePlayable(const PString & type,
                                 const PString & arg,
                                 PINDEX repeat,
                                 PINDEX delay,
                                 PBoolean autoDelete)
{
  if (repeat <= 0)
    repeat = 1;

  PVXMLPlayable * item = PFactory<PVXMLPlayable>::CreateInstance(type);
  if (item == NULL) {
    PTRACE(2, "Cannot find playable of type " << type);
    return false;
  }

  if (item->Open(*this, arg, delay, repeat, autoDelete)) {
    PTRACE(3, "Enqueueing playable " << type << " with arg \"" << arg
           << "\" for playing " << repeat << " times, followed by " << delay << "ms silence");
    return QueuePlayable(item);
  }

  delete item;
  return false;
}


PBoolean PVXMLChannel::QueuePlayable(PVXMLPlayable * newItem)
{
  if (!IsOpen()) {
    delete newItem;
    return false;
  }

  m_playQueueMutex.Wait();
  m_playQueue.Enqueue(newItem);
  m_playQueueMutex.Signal();
  return true;
}


PBoolean PVXMLChannel::QueueResource(const PURL & url, PINDEX repeat, PINDEX delay)
{
  if (url.GetScheme() *= "file")
    return QueuePlayable("File", url.AsFilePath(), repeat, delay, false);
  else
    return QueuePlayable("URL", url.AsString(), repeat, delay);
}


PBoolean PVXMLChannel::QueueData(const PBYTEArray & data, PINDEX repeat, PINDEX delay)
{
  PTRACE(3, "Enqueueing " << data.GetSize() << " bytes for playing, followed by " << delay << "ms silence");
  PVXMLPlayableData * item = PFactory<PVXMLPlayable>::CreateInstanceAs<PVXMLPlayableData>("PCM Data");
  if (item == NULL) {
    PTRACE(2, "Cannot find playable of type 'PCM Data'");
    delete item;
    return false;
  }

  if (!item->Open(*this, "", delay, repeat, true)) {
    PTRACE(2, "Cannot open playable of type 'PCM Data'");
    delete item;
    return false;
  }

  item->SetData(data);

  return QueuePlayable(item);
}


void PVXMLChannel::FlushQueue()
{
  PTRACE(4, "Flushing playable queue");

  PWaitAndSignal mutex(m_playQueueMutex);

  PVXMLPlayable * qItem;
  while ((qItem = m_playQueue.Dequeue()) != NULL) {
    qItem->OnStop();
    delete qItem;
  }

  if (m_currentPlayItem != NULL) {
    m_currentPlayItem->OnStop();
    delete m_currentPlayItem;
    m_currentPlayItem = NULL;
  }

  m_silenceTimer.Stop();

  PTRACE(4, "Flushed playable queue");
}


///////////////////////////////////////////////////////////////

PFACTORY_CREATE(PFactory<PVXMLChannel>, PVXMLChannelPCM, VXML_PCM16);

PVXMLChannelPCM::PVXMLChannelPCM()
  : PVXMLChannel(10, 160)
  , m_sampleRate(8000)
  , m_channels(1)
{
}


PString PVXMLChannelPCM::GetAudioFormat() const
{
  return VXML_PCM16;
}


unsigned PVXMLChannelPCM::GetSampleRate() const
{
  return m_sampleRate;
}


bool PVXMLChannelPCM::SetSampleRate(unsigned rate)
{
  if (!PAssert(rate > 0, PInvalidParameter))
    return false;

  m_sampleRate = rate;
  frameSize = rate*2*m_channels*frameDelay/1000;
  return true;
}


unsigned PVXMLChannelPCM::GetChannels() const
{
  return m_channels;
}


bool PVXMLChannelPCM::SetChannels(unsigned channels)
{
  if (!PAssert(channels > 0, PInvalidParameter))
    return false;

  m_channels = channels;
  return true;
}


PBoolean PVXMLChannelPCM::WriteFrame(const void * buf, PINDEX len)
{
  return PDelayChannel::Write(buf, len);
}


PBoolean PVXMLChannelPCM::ReadFrame(void * buffer, PINDEX amount)
{
  PINDEX len = 0;
  while (len < amount)  {
    if (!PDelayChannel::Read(len + (char *)buffer, amount-len))
      return false;
    len += GetLastReadCount();
  }

  SetLastReadCount(len);
  return true;
}


PINDEX PVXMLChannelPCM::CreateSilenceFrame(void * buffer, PINDEX amount)
{
  memset(buffer, 0, amount);
  return amount;
}


PBoolean PVXMLChannelPCM::IsSilenceFrame(const void * buf, PINDEX len) const
{
  // Calculate the average signal level of this frame
  int sum = 0;

  const short * pcmPtr = (const short *)buf;
  const short * endPtr = pcmPtr + len/2;
  while (pcmPtr != endPtr) {
    if (*pcmPtr < 0)
      sum -= *pcmPtr++;
    else
      sum += *pcmPtr++;
  }

  // calc average
  unsigned level = sum / (len / 2);

  return level < 500; // arbitrary level
}


static short beepData[] = { 0, 18784, 30432, 30400, 18784, 0, -18784, -30432, -30400, -18784 };


void PVXMLChannelPCM::GetBeepData(PBYTEArray & data, unsigned ms)
{
  data.SetSize(0);
  while (data.GetSize() < (PINDEX)(ms * 16)) {
    PINDEX len = data.GetSize();
    data.SetSize(len + sizeof(beepData));
    memcpy(len + data.GetPointer(), beepData, sizeof(beepData));
  }
}

///////////////////////////////////////////////////////////////

PFACTORY_CREATE(PFactory<PVXMLChannel>, PVXMLChannelG7231, VXML_G7231);

PVXMLChannelG7231::PVXMLChannelG7231()
  : PVXMLChannel(30, 0)
{
}


PString PVXMLChannelG7231::GetAudioFormat() const
{
  return VXML_G7231;
}


unsigned PVXMLChannelG7231::GetSampleRate() const
{
  return 8000;
}


bool PVXMLChannelG7231::SetSampleRate(unsigned rate)
{
  return rate == 8000;
}


unsigned PVXMLChannelG7231::GetChannels() const
{
  return 1;
}


bool PVXMLChannelG7231::SetChannels(unsigned channels)
{
  return channels == 1;
}


static const PINDEX g7231Lens[] = { 24, 20, 4, 1 };

PBoolean PVXMLChannelG7231::WriteFrame(const void * buffer, PINDEX actualLen)
{
  PINDEX len = g7231Lens[(*(BYTE *)buffer)&3];
  if (len > actualLen)
    return false;

  return PDelayChannel::Write(buffer, len);
}


PBoolean PVXMLChannelG7231::ReadFrame(void * buffer, PINDEX /*amount*/)
{
  if (!PDelayChannel::Read(buffer, 1))
    return false;

  PINDEX len = g7231Lens[(*(BYTE *)buffer)&3];
  if (len != 1) {
    if (!PIndirectChannel::Read(1+(BYTE *)buffer, len-1))
      return false;
    SetLastReadCount(GetLastReadCount()+1);
  }

  return true;
}


PINDEX PVXMLChannelG7231::CreateSilenceFrame(void * buffer, PINDEX /* len */)
{
  ((BYTE *)buffer)[0] = 2;
  memset(((BYTE *)buffer)+1, 0, 3);
  return 4;
}


PBoolean PVXMLChannelG7231::IsSilenceFrame(const void * buf, PINDEX len) const
{
  if (len == 4)
    return true;
  if (buf == NULL)
    return false;
  return ((*(const BYTE *)buf)&3) == 2;
}

///////////////////////////////////////////////////////////////

PFACTORY_CREATE(PFactory<PVXMLChannel>, PVXMLChannelG729, VXML_G729);

PVXMLChannelG729::PVXMLChannelG729()
  : PVXMLChannel(10, 0)
{
}


PString PVXMLChannelG729::GetAudioFormat() const
{
  return VXML_G729;
}


unsigned PVXMLChannelG729::GetSampleRate() const
{
  return 8000;
}


bool PVXMLChannelG729::SetSampleRate(unsigned rate)
{
  return rate == 8000;
}


unsigned PVXMLChannelG729::GetChannels() const
{
  return 1;
}


bool PVXMLChannelG729::SetChannels(unsigned channels)
{
  return channels == 1;
}


PBoolean PVXMLChannelG729::WriteFrame(const void * buf, PINDEX /*len*/)
{
  return PDelayChannel::Write(buf, 10);
}

PBoolean PVXMLChannelG729::ReadFrame(void * buffer, PINDEX /*amount*/)
{
  return PDelayChannel::Read(buffer, 10); // No silence frames so always 10 bytes
}


PINDEX PVXMLChannelG729::CreateSilenceFrame(void * buffer, PINDEX /* len */)
{
  memset(buffer, 0, 10);
  return 10;
}


PBoolean PVXMLChannelG729::IsSilenceFrame(const void * /*buf*/, PINDEX /*len*/) const
{
  return false;
}


//////////////////////////////////////////////////////////////////////////////////

#undef  PTraceModule
#define PTraceModule() "VXML-TTS"

class TextToSpeech_Sample : public PTextToSpeech
{
  public:
    TextToSpeech_Sample();
    PStringArray GetVoiceList();
    PBoolean SetVoice(const PString & voice);
    PBoolean SetSampleRate(unsigned rate);
    unsigned GetSampleRate();
    PBoolean SetChannels(unsigned channels);
    unsigned GetChannels();
    PBoolean SetVolume(unsigned volume);
    unsigned GetVolume();
    PBoolean OpenFile   (const PFilePath & fn);
    PBoolean OpenChannel(PChannel * chanel);
    PBoolean IsOpen()    { return m_opened; }
    PBoolean Close();
    PBoolean Speak(const PString & text, TextType hint = Default);
    PBoolean SpeakNumber(unsigned number);

    PBoolean SpeakFile(const PString & text);

  protected:
    //PTextToSpeech * defaultEngine;

    PDECLARE_MUTEX(mutex);
    bool      m_opened;
    bool      m_usingFile;
    PString   m_text;
    PFilePath m_path;
    unsigned  m_sampleRate;
    unsigned  m_channels;
    unsigned  m_volume;
    PString   m_voice;

    std::vector<PFilePath> m_filenames;
};


TextToSpeech_Sample::TextToSpeech_Sample()
  : m_opened(false)
  , m_usingFile(false)
  , m_sampleRate(8000)
  , m_channels(1)
  , m_volume(100)
{
}


PStringArray TextToSpeech_Sample::GetVoiceList()
{
  PStringArray r;
  return r;
}


PBoolean TextToSpeech_Sample::SetVoice(const PString & v)
{
  m_voice = v;
  return true;
}


PBoolean TextToSpeech_Sample::SetSampleRate(unsigned v)
{
  m_sampleRate = v;
  return true;
}


unsigned TextToSpeech_Sample::GetSampleRate()
{
  return m_sampleRate;
}


PBoolean TextToSpeech_Sample::SetChannels(unsigned v)
{
  m_channels = v;
  return true;
}


unsigned TextToSpeech_Sample::GetChannels()
{
  return m_channels;
}


PBoolean TextToSpeech_Sample::SetVolume(unsigned v)
{
  m_volume = v;
  return true;
}


unsigned TextToSpeech_Sample::GetVolume()
{
  return m_volume;
}


PBoolean TextToSpeech_Sample::OpenFile(const PFilePath & fn)
{
  PWaitAndSignal m(mutex);

  Close();
  m_usingFile = true;
  m_path = fn;
  m_opened = true;

  PTRACE(3, "Writing speech to " << fn);

  return true;
}


PBoolean TextToSpeech_Sample::OpenChannel(PChannel * /*chanel*/)
{
  PWaitAndSignal m(mutex);

  Close();
  m_usingFile = false;
  m_opened = false;

  return true;
}


PBoolean TextToSpeech_Sample::Close()
{
  PWaitAndSignal m(mutex);

  if (!m_opened)
    return true;

  PBoolean stat = true;

#if P_WAVFILE
  if (m_usingFile) {
    PWAVFile outputFile(PSOUND_PCM16, m_path, PFile::WriteOnly);
    if (!outputFile.IsOpen()) {
      PTRACE(1, "Cannot create output file " << m_path);
      stat = false;
    }
    else {
      std::vector<PFilePath>::const_iterator r;
      for (r = m_filenames.begin(); r != m_filenames.end(); ++r) {
        PFilePath f = *r;
        PWAVFile file;
        file.SetAutoconvert();
        if (!file.Open(f, PFile::ReadOnly)) {
          PTRACE(1, "Cannot open input file " << f);
          stat = false;
        } else {
          PTRACE(1, "Reading from " << f);
          BYTE buffer[1024];
          for (;;) {
            if (!file.Read(buffer, 1024))
              break;
            outputFile.Write(buffer, file.GetLastReadCount());
          }
        }
      }
    }
    m_filenames.erase(m_filenames.begin(), m_filenames.end());
  }
#endif // P_WAVFILE

  m_opened = false;
  return stat;
}


PBoolean TextToSpeech_Sample::SpeakNumber(unsigned number)
{
  return Speak(PString(PString::Signed, number), Number);
}


PBoolean TextToSpeech_Sample::Speak(const PString & text, TextType hint)
{
  // break into lines
  PStringArray lines = text.Lines();
  PINDEX i;
  for (i = 0; i < lines.GetSize(); ++i) {

    PString line = lines[i].Trim();
    if (line.IsEmpty())
      continue;

    PTRACE(3, "Asked to speak " << text << " with type " << hint);

    if (hint == DateAndTime) {
      PTRACE(3, "Speaking date and time");
      Speak(text, Date);
      Speak(text, Time);
      continue;
    }

    if (hint == Date) {
      PTime time(line);
      if (time.IsValid()) {
        PTRACE(4, "Speaking date " << time);
        SpeakFile(time.GetDayName(time.GetDayOfWeek(), PTime::FullName));
        SpeakNumber(time.GetDay());
        SpeakFile(time.GetMonthName(time.GetMonth(), PTime::FullName));
        SpeakNumber(time.GetYear());
      }
      continue;
    }

    if (hint == Time) {
      PTime time(line);
      if (time.IsValid()) {
        PTRACE(4, "Speaking time " << time);
        int hour = time.GetHour();
        if (hour < 13) {
          SpeakNumber(hour);
          SpeakNumber(time.GetMinute());
          SpeakFile(PTime::GetTimeAM());
        }
        else {
          SpeakNumber(hour-12);
          SpeakNumber(time.GetMinute());
          SpeakFile(PTime::GetTimePM());
        }
      }
      continue;
    }

    if (hint == Default) {
      PBoolean isTime = false;
      PBoolean isDate = false;

      for (i = 0; !isDate && i < 7; ++i) {
        isDate = isDate || (line.Find(PTime::GetDayName((PTime::Weekdays)i, PTime::FullName)) != P_MAX_INDEX);
        isDate = isDate || (line.Find(PTime::GetDayName((PTime::Weekdays)i, PTime::Abbreviated)) != P_MAX_INDEX);
        PTRACE(4, " " << isDate << " - " << PTime::GetDayName((PTime::Weekdays)i, PTime::FullName) << "," << PTime::GetDayName((PTime::Weekdays)i, PTime::Abbreviated));
      }
      for (i = 1; !isDate && i <= 12; ++i) {
        isDate = isDate || (line.Find(PTime::GetMonthName((PTime::Months)i, PTime::FullName)) != P_MAX_INDEX);
        isDate = isDate || (line.Find(PTime::GetMonthName((PTime::Months)i, PTime::Abbreviated)) != P_MAX_INDEX);
        PTRACE(4, " " << isDate << " - " << PTime::GetMonthName((PTime::Months)i, PTime::FullName) << "," << PTime::GetMonthName((PTime::Months)i, PTime::Abbreviated));
      }

      if (!isTime)
        isTime = line.Find(PTime::GetTimeSeparator()) != P_MAX_INDEX;
      if (!isDate)
        isDate = line.Find(PTime::GetDateSeparator()) != P_MAX_INDEX;

      if (isDate && isTime) {
        PTRACE(4, "Default changed to DateAndTime");
        Speak(line, DateAndTime);
        continue;
      }
      if (isDate) {
        PTRACE(4, "Default changed to Date");
        Speak(line, Date);
        continue;
      }
      else if (isTime) {
        PTRACE(4, "Default changed to Time");
        Speak(line, Time);
        continue;
      }
    }

    PStringArray tokens = line.Tokenise("\t ", false);
    for (PINDEX j = 0; j < tokens.GetSize(); ++j) {
      PString word = tokens[j].Trim();
      if (word.IsEmpty())
        continue;
      PTRACE(4, "Speaking word " << word << " as " << hint);
      switch (hint) {

        case Time:
        case Date:
        case DateAndTime:
          PAssertAlways("Logic error");
          break;

        default:
        case Default:
        case Literal:
          {
            PBoolean isDigits = true;
            PBoolean isIpAddress = true;

            PINDEX k;
            for (k = 0; k < word.GetLength(); ++k) {
              if (word[k] == '.')
                isDigits = false;
              else if (!isdigit(word[k]))
                isDigits = isIpAddress = false;
            }

            if (isIpAddress) {
              PTRACE(4, "Default changed to IPAddress");
              Speak(word, IPAddress);
            } else if (isDigits) {
              PTRACE(4, "Default changed to Number");
              Speak(word, Number);
            } else {
              PTRACE(4, "Default changed to Spell");
              Speak(word, Spell);
            }
          }
          break;

        case Spell:
          PTRACE(4, "Spelling " << text);
          for (PINDEX k = 0; k < text.GetLength(); ++k)
            SpeakFile(text[k]);
          break;

        case Phone:
        case Digits:
          PTRACE(4, "Speaking digits " << text);
          for (PINDEX k = 0; k < text.GetLength(); ++k) {
            if (isdigit(text[k]))
              SpeakFile(text[k]);
          }
          break;

        case Duration:
        case Currency:
        case Number:
          {
            int number = atoi(line);
            PTRACE(4, "Speaking number " << number);
            if (number < 0) {
              SpeakFile("negative");
              number = -number;
            }
            else if (number == 0) {
              SpeakFile("0");
            }
            else {
              if (number >= 1000000) {
                int millions = number / 1000000;
                number = number % 1000000;
                SpeakNumber(millions);
                SpeakFile("million");
              }
              if (number >= 1000) {
                int thousands = number / 1000;
                number = number % 1000;
                SpeakNumber(thousands);
                SpeakFile("thousand");
              }
              if (number >= 100) {
                int hundreds = number / 100;
                number = number % 100;
                SpeakNumber(hundreds);
                SpeakFile("hundred");
              }
              if (!SpeakFile(PString(PString::Signed, number))) {
                int tens = number / 10;
                number = number % 10;
                if (tens > 0)
                  SpeakFile(PString(PString::Signed, tens*10));
                if (number > 0)
                  SpeakFile(PString(PString::Signed, number));
              }
            }
          }
          break;

        case IPAddress:
          {
            PIPSocket::Address addr(line);
            PTRACE(4, "Speaking IP address " << addr);
            for (PINDEX k = 0; k < 4; ++k) {
              int octet = addr[k];
              if (octet < 100)
                SpeakNumber(octet);
              else
                Speak(octet, Digits);
              if (k != 3)
                SpeakFile("dot");
            }
          }
          break;
      }
    }
  }

  return true;
}


PBoolean TextToSpeech_Sample::SpeakFile(const PString & text)
{
  PFilePath f = PDirectory(m_voice) + (text.ToLower() + ".wav");
  if (!PFile::Exists(f)) {
    PTRACE(2, "Unable to find explicit file for " << text);
    return false;
  }
  m_filenames.push_back(f);
  return true;
}

PFACTORY_CREATE(PFactory<PTextToSpeech>, TextToSpeech_Sample, "sampler", false);


#endif   // P_VXML


///////////////////////////////////////////////////////////////
