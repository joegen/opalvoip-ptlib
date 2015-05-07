/*
 * main.h
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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _wx_MAIN_H
#define _wx_MAIN_H

#include <ptlib.h>
#include <ptlib/pprocess.h>
#include <ptlib/videoio.h>

#include <wx/wx.h>
#include <wx/dataobj.h>

#include <ptlib/wxstring.h>


class MyApp : public wxApp, public PProcess
{
    PCLASSINFO(MyApp, PProcess);

  public:
    MyApp();

    void Main(); // Dummy function

      // FUnction from wxWindows
    virtual bool OnInit();
};


class MyFrame : public wxFrame
{
  public:
    MyFrame();
    ~MyFrame();

    bool Initialise();

    void PostEvent(
      unsigned id,
      const PString & str = PString::Empty(),
      const void * data = NULL
    );

  private:
    void OnClose(wxCloseEvent & theEvent);
    void OnAdjustMenus(wxMenuEvent & theEvent);

    void OnMenuQuit(wxCommandEvent & theEvent);
    void OnMenuAbout(wxCommandEvent & theEvent);
    void OnMenuOptions(wxCommandEvent & theEvent);

    void OnMenuRecordWAV(wxCommandEvent & theEvent);
    void OnMenuPlayWAV(wxCommandEvent & theEvent);
    void OnMenuVideoTest(wxCommandEvent & theEvent);

    void OnEvent(wxCommandEvent & theEvent);

    PwxString              m_player;
    PwxString              m_recorder;
    PVideoDevice::OpenArgs m_grabber;

    wxPanel m_panel;

    DECLARE_EVENT_TABLE()

  friend class OptionsDialog;
};


class OptionsDialog : public wxDialog
{
  public:
    OptionsDialog(MyFrame *parent);
    ~OptionsDialog();
    virtual bool TransferDataFromWindow();

  private:
    MyFrame & m_frame;

    PwxString m_SoundPlayer;
    PwxString m_SoundRecorder;
    PwxString m_VideoGrabber;
    int       m_VideoGrabFormat;
    int       m_VideoGrabSource;
    int       m_VideoGrabFrameRate;
    PwxString m_VideoGrabFrameSize;
    bool      m_VideoGrabFlipped;

    wxComboBox * m_videoGrabDevice;
    wxChoice   * m_videoSourceChoice;

    DECLARE_EVENT_TABLE()
};


#endif // _wx_MAIN_H


// End of File ///////////////////////////////////////////////////////////////
