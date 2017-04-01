/*
 * main.cpp
 *
 * An PTLib/wxWidgets GUI application.
 *    
 * Copyright (c) 2011 Vox Lucida
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
 * The Original Code is wxSample.
 *
 * The Initial Developer of the Original Code is Robert Jongbloed
 *
 * Contributor(s): ______________________________________.
 *
 */

#include "main.h"

#include "version.h"

#include <wx/config.h>
#include <wx/valgen.h>

#undef LoadMenu // Bizarre but necessary before the xml code
#include <wx/xrc/xmlres.h>

#include <ptclib/pwavfile.h>
#include <ptlib/sound.h>

#include <algorithm>


#if defined(__WXGTK__)   || \
    defined(__WXMOTIF__) || \
    defined(__WXX11__)   || \
    defined(__WXMAC__)   || \
    defined(__WXMGL__)   || \
    defined(__WXCOCOA__)
  #include "app.xpm"

  #define VIDEO_WINDOW_DEVICE "SDL"
#else
  #define VIDEO_WINDOW_DEVICE "MSWIN STYLE=0x80C80000"  // WS_POPUP|WS_BORDER|WS_SYSMENU|WS_CAPTION
#endif

#if wxUSE_UNICODE
typedef wstringstream tstringstream;
#else
typedef  stringstream tstringstream;
#endif

extern void InitXmlResource(); // From resource.cpp whichis compiled resource.xrc


// Definitions of the configuration file section and key names

#define DEF_FIELD(name) static const wxChar name##Key[] = wxT(#name)

static const wxChar AppearanceGroup[] = wxT("/Appearance");
DEF_FIELD(MainFrameX);
DEF_FIELD(MainFrameY);
DEF_FIELD(MainFrameWidth);
DEF_FIELD(MainFrameHeight);

static const wxChar AudioGroup[] = wxT("/Audio");
DEF_FIELD(SoundPlayer);
DEF_FIELD(SoundRecorder);

static const wxChar VideoGroup[] = wxT("/Video");
DEF_FIELD(VideoGrabber);
DEF_FIELD(VideoGrabFormat);
DEF_FIELD(VideoGrabSource);
DEF_FIELD(VideoGrabFrameRate);
DEF_FIELD(VideoGrabFrameSize);
DEF_FIELD(VideoGrabFlipped);


enum {
  ID_VIDEO_ENDED,
};

DECLARE_EVENT_TYPE(wxMyEventMessage, -1)
DEFINE_EVENT_TYPE(wxMyEventMessage)


///////////////////////////////////////////////////////////////////////////////

IMPLEMENT_APP(MyApp)

MyApp::MyApp()
  : PProcess(MANUFACTURER_TEXT, PRODUCT_NAME_TEXT,
             MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
  wxConfig::Set(new wxConfig(PwxString(GetName()), PwxString(GetManufacturer())));
}


void MyApp::Main()
{
  // Dummy function
}

//////////////////////////////////

bool MyApp::OnInit()
{
  // make sure various URL types are registered to this application
  {
    PString urlTypes("sip\nh323\nsips\nh323s");
    PProcess::HostSystemURLHandlerInfo::RegisterTypes(urlTypes, true);
  }

  // Create the main frame window
  MyFrame * frame = new MyFrame();
  SetTopWindow(frame);
  wxBeginBusyCursor();
  bool ok = frame->Initialise();
  wxEndBusyCursor();
  return ok;
}


///////////////////////////////////////////////////////////////////////////////

BEGIN_EVENT_TABLE(MyFrame, wxFrame)
  EVT_CLOSE(MyFrame::OnClose)

  EVT_MENU_OPEN(MyFrame::OnAdjustMenus)

  EVT_MENU(wxID_EXIT,                    MyFrame::OnMenuQuit)
  EVT_MENU(wxID_ABOUT,                   MyFrame::OnMenuAbout)
  EVT_MENU(wxID_PREFERENCES,             MyFrame::OnMenuOptions)
  EVT_MENU(XRCID("MenuRecordWAV"),       MyFrame::OnMenuRecordWAV)
  EVT_MENU(XRCID("MenuPlayWAV"),         MyFrame::OnMenuPlayWAV)
  EVT_MENU(XRCID("MenuVideoTest"),       MyFrame::OnMenuVideoTest)
  
END_EVENT_TABLE()


MyFrame::MyFrame()
  : wxFrame(NULL, -1, wxT(PRODUCT_NAME_TEXT), wxDefaultPosition, wxSize(640, 480))
  , m_player(PSoundChannel::GetDefaultDevice(PSoundChannel::Player))
  , m_recorder(PSoundChannel::GetDefaultDevice(PSoundChannel::Recorder))
{
  m_grabber.deviceName = PVideoInputDevice::GetDriversDeviceNames("*")[0];

  // Give it an icon
  SetIcon(wxICON(AppIcon));
}


MyFrame::~MyFrame()
{
  wxMenuBar * menubar = GetMenuBar();
  SetMenuBar(NULL);
  delete menubar;

  delete wxXmlResource::Set(NULL);
}


bool MyFrame::Initialise()
{
  wxImage::AddHandler(new wxGIFHandler);
  wxXmlResource::Get()->InitAllHandlers();
  InitXmlResource();

  // Make a menubar
  wxMenuBar * menubar;
  {
    PMEMORY_IGNORE_ALLOCATIONS_FOR_SCOPE;
    if ((menubar = wxXmlResource::Get()->LoadMenuBar(wxT("MenuBar"))) == NULL)
      return false;
    SetMenuBar(menubar);
  }

  wxAcceleratorEntry accelEntries[] = {
      wxAcceleratorEntry(wxACCEL_CTRL,  'T',         XRCID("MenuVideoTest")),
      wxAcceleratorEntry(wxACCEL_CTRL,  'X',         wxID_CUT),
      wxAcceleratorEntry(wxACCEL_CTRL,  'C',         wxID_COPY),
      wxAcceleratorEntry(wxACCEL_CTRL,  'V',         wxID_PASTE),
      wxAcceleratorEntry(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE)
  };
  wxAcceleratorTable accel(PARRAYSIZE(accelEntries), accelEntries);
  SetAcceleratorTable(accel);

  wxConfigBase * config = wxConfig::Get();
  config->SetPath(PwxString(AppearanceGroup));

  wxPoint initalPosition = wxDefaultPosition;
  if (config->Read(MainFrameXKey, &initalPosition.x) && config->Read(MainFrameYKey, &initalPosition.y))
    Move(initalPosition);

  wxSize initialSize(512, 384);
  if (config->Read(MainFrameWidthKey, &initialSize.x) && config->Read(MainFrameHeightKey, &initialSize.y))
    SetSize(initialSize);

  wxXmlResource::Get()->LoadPanel(&m_panel, this, wxT("MainPanel"));

  // Show the frame window
  Show(true);

  return true;
}


void MyFrame::OnClose(wxCloseEvent& /*event*/)
{
  ::wxBeginBusyCursor();

  wxConfigBase * config = wxConfig::Get();
  config->SetPath(AppearanceGroup);

  int x, y;
  GetPosition(&x, &y);
  config->Write(MainFrameXKey, x);
  config->Write(MainFrameYKey, y);

  int w, h;
  GetSize(&w, &h);
  config->Write(MainFrameWidthKey, w);
  config->Write(MainFrameHeightKey, h);

  Destroy();
}


void MyFrame::PostEvent(unsigned id, const PString & str, const void * data)
{
  wxCommandEvent theEvent(wxMyEventMessage, id);
  theEvent.SetEventObject(this);
  theEvent.SetString(PwxString(str));
  theEvent.SetClientData((void *)data);
  GetEventHandler()->AddPendingEvent(theEvent);
}


void MyFrame::OnEvent(wxCommandEvent & theEvent)
{
  switch (theEvent.GetId()) {
    case ID_VIDEO_ENDED :
      break;
  }
}


void MyFrame::OnAdjustMenus(wxMenuEvent & WXUNUSED(theEvent))
{
  wxMenuBar * menubar = GetMenuBar();
  menubar->Enable(XRCID("MenuVideoTest"), true);
}


void MyFrame::OnMenuQuit(wxCommandEvent & WXUNUSED(theEvent))
{
  Close(true);
}


void MyFrame::OnMenuAbout(wxCommandEvent & WXUNUSED(theEvent))
{
  tstringstream text;
  text  << PRODUCT_NAME_TEXT " Version " << PProcess::Current().GetVersion() << "\n"
           "\n"
           "Copyright (c) " COPYRIGHT_YEAR " " COPYRIGHT_HOLDER ", All rights reserved.\n"
           "\n"
           "This application may be used for any purpose so long as it is not sold "
           "or distributed for profit on it's own, or it's ownership by " COPYRIGHT_HOLDER
           " disguised or hidden in any way.\n"
           "\n"
           "Part of the Portable Tools Library, http://www.opalvoip.org\n"
           "  PTLib Version: " << PProcess::GetLibVersion() << '\n';
  wxMessageDialog dialog(this, text.str().c_str(), wxT("About ..."), wxOK);
  dialog.ShowModal();
}


void MyFrame::OnMenuRecordWAV(wxCommandEvent & WXUNUSED(theEvent))
{
}


void MyFrame::OnMenuPlayWAV(wxCommandEvent & WXUNUSED(theEvent))
{
}


void MyFrame::OnMenuVideoTest(wxCommandEvent & WXUNUSED(theEvent))
{
  PVideoInputDevice * grabber = PVideoInputDevice::CreateOpenedDevice(m_grabber);
  if (grabber == NULL) {
    wxMessageBox(wxT("Could not open video capture."), wxT("Video Test"), wxCANCEL|wxICON_EXCLAMATION);
    return;
  }

  wxRect box(GetRect().GetBottomLeft(), wxSize(m_grabber.width, m_grabber.height));

  PVideoDevice::OpenArgs displayArgs;
  displayArgs.deviceName = psprintf(VIDEO_WINDOW_DEVICE" TITLE=\"Video Test\" X=%i Y=%i", box.GetLeft(), box.GetTop());
  displayArgs.width = m_grabber.width;
  displayArgs.height = m_grabber.height;

  PVideoOutputDevice * display = PVideoOutputDevice::CreateOpenedDevice(displayArgs, true);
  if (PAssertNULL(display) == NULL) {
    delete grabber;
    return;
  }

  if (!grabber->Start())
    wxMessageBox(wxT("Could not start video capture."), wxT("Video Test"), wxCANCEL|wxICON_EXCLAMATION);
  else {
    PBYTEArray frame;
    unsigned frameCount = 0;
    while (grabber->GetFrame(frame) &&
           display->SetFrameData(0, 0, grabber->GetFrameWidth(), grabber->GetFrameHeight(), frame))
      frameCount++;
  }

  delete display;
  delete grabber;
}


///////////////////////////////////////////////////////////////////////////////

#ifdef _MSC_VER
#pragma warning(disable:4100)
#endif

template <class cls> cls * FindWindowByNameAs(wxWindow * window, const wxChar * name)
{
  wxWindow * baseChild = window->FindWindow(name);
  if (PAssert(baseChild != NULL, "Windows control not found")) {
    cls * derivedChild = dynamic_cast<cls *>(baseChild);
    if (PAssert(derivedChild != NULL, "Cannot cast window object to selected class"))
      return derivedChild;
  }

  return NULL;
}

#ifdef _MSC_VER
#pragma warning(default:4100)
#endif


static PwxString AudioDeviceNameToScreen(const PwxString & name)
{
  PwxString str = name;
  str.Replace(wxT("\t"), wxT(": "));
  return str;
}

static PString AudioDeviceNameFromScreen(const wxString & name)
{
  PwxString str = name;
  str.Replace(wxT(": "), wxT("\t"));
  return str.p_str();
}

static void FillAudioDeviceComboBox(wxItemContainer * list, PSoundChannel::Directions dir)
{
  PStringArray devices = PSoundChannel::GetDeviceNames(dir);
  for (PINDEX i = 0; i < devices.GetSize(); i++)
    list->Append(AudioDeviceNameToScreen(devices[i]));
}


class wxFrameSizeValidator: public wxGenericValidator
{
public:
  wxFrameSizeValidator(wxString* val)
    : wxGenericValidator(val)
  {
  }

  virtual wxObject *Clone() const
  {
    return new wxFrameSizeValidator(*this);
  }

  virtual bool Validate(wxWindow *)
  {
    wxComboBox *control = (wxComboBox *) m_validatorWindow;
    PwxString size = control->GetValue();

    unsigned width, height;
    if (PVideoFrameInfo::ParseSize(size, width, height))
      return true;

    wxMessageBox(wxT("Illegal value \"") + size + wxT("\" for video size."),
                 wxT("wxSample Error"), wxCANCEL|wxICON_EXCLAMATION);
    return false;
  }
};


#define INIT_FIELD(name, value) \
  m_##name = value; \
  FindWindowByName(name##Key)->SetValidator(wxGenericValidator(&m_##name))


///////////////////////////////////////////////////////////////////////////////

void MyFrame::OnMenuOptions(wxCommandEvent & WXUNUSED(theEvent))
{
  OptionsDialog dlg(this);
  dlg.ShowModal();
}


BEGIN_EVENT_TABLE(OptionsDialog, wxDialog)
END_EVENT_TABLE()


OptionsDialog::OptionsDialog(MyFrame * frame)
  : m_frame(*frame)
{
  SetExtraStyle(GetExtraStyle() | wxWS_EX_VALIDATE_RECURSIVELY);
  wxXmlResource::Get()->LoadDialog(this, frame, wxT("OptionsDialog"));

  PTRACE(4, "wxSample\tLoaded options dialog");

  // Fill sound player combo box with available devices and set selection
  wxComboBox * combo = FindWindowByNameAs<wxComboBox>(this, SoundPlayerKey);
  combo->SetValidator(wxGenericValidator(&m_SoundPlayer));
  FillAudioDeviceComboBox(combo, PSoundChannel::Player);
  m_SoundPlayer = AudioDeviceNameToScreen(m_frame.m_player);

  // Fill sound recorder combo box with available devices and set selection
  combo = FindWindowByNameAs<wxComboBox>(this, SoundRecorderKey);
  combo->SetValidator(wxGenericValidator(&m_SoundRecorder));
  FillAudioDeviceComboBox(combo, PSoundChannel::Recorder);
  m_SoundRecorder = AudioDeviceNameToScreen(m_frame.m_recorder);

  // Fill sound recorder combo box with available devices and set selection
  INIT_FIELD(VideoGrabber, m_frame.m_grabber.deviceName);
  INIT_FIELD(VideoGrabFormat, m_frame.m_grabber.videoFormat);
  m_videoSourceChoice = FindWindowByNameAs<wxChoice>(this, wxT("VideoGrabSource"));
  m_VideoGrabSource = m_frame.m_grabber.channelNumber+1;
  m_videoSourceChoice->SetValidator(wxGenericValidator(&m_VideoGrabSource));
  INIT_FIELD(VideoGrabFrameRate, m_frame.m_grabber.rate);
  INIT_FIELD(VideoGrabFlipped, m_frame.m_grabber.flip);

  PStringArray knownSizes = PVideoFrameInfo::GetSizeNames();
  m_VideoGrabFrameSize = PVideoFrameInfo::AsString(m_frame.m_grabber.width, m_frame.m_grabber.height);
  combo = FindWindowByNameAs<wxComboBox>(this, VideoGrabFrameSizeKey);
  combo->SetValidator(wxFrameSizeValidator(&m_VideoGrabFrameSize));
  for (PINDEX i = 0; i < knownSizes.GetSize(); ++i)
    combo->Append(PwxString(knownSizes[i]));

  m_videoGrabDevice = FindWindowByNameAs<wxComboBox>(this, wxT("VideoGrabber"));
  PStringArray devices = PVideoInputDevice::GetDriversDeviceNames("*");
  for (PINDEX i = 0; i < devices.GetSize(); i++)
    m_videoGrabDevice->Append(PwxString(devices[i]));
}


OptionsDialog::~OptionsDialog()
{
}


bool OptionsDialog::TransferDataFromWindow()
{
  if (!wxDialog::TransferDataFromWindow())
    return false;

  ::wxBeginBusyCursor();

  wxConfigBase * config = wxConfig::Get();

  ////////////////////////////////////////
  // Sound fields
  config->SetPath(AudioGroup);
  m_frame.m_player = AudioDeviceNameFromScreen(m_SoundPlayer);
  config->Write(SoundPlayerKey, m_frame.m_player);
  m_frame.m_recorder = AudioDeviceNameFromScreen(m_SoundRecorder);
  config->Write(SoundRecorderKey, m_frame.m_recorder);

  ////////////////////////////////////////
  // Video fields
  config->SetPath(VideoGroup);

  m_frame.m_grabber.deviceName = m_VideoGrabber.p_str();
  config->Write(VideoGrabberKey, m_VideoGrabber);

  m_frame.m_grabber.videoFormat = (PVideoDevice::VideoFormat)m_VideoGrabFormat;
  config->Write(VideoGrabberKey, m_VideoGrabFormat);

  m_frame.m_grabber.channelNumber = --m_VideoGrabSource;
  config->Write(VideoGrabSourceKey, m_VideoGrabSource);

  m_frame.m_grabber.rate = m_VideoGrabFrameRate;
  config->Write(VideoGrabFrameRateKey, m_VideoGrabFrameRate);

  PVideoDevice::ParseSize(m_VideoGrabFrameSize,  m_frame.m_grabber.width,  m_frame.m_grabber.height);
  config->Write(VideoGrabFrameSizeKey, m_VideoGrabFrameSize);

  m_frame.m_grabber.flip = m_VideoGrabFlipped;
  config->Write(VideoGrabFrameSizeKey, m_VideoGrabFlipped);

  ::wxEndBusyCursor();

  return true;
}


// End of File ///////////////////////////////////////////////////////////////
