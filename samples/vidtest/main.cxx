/*
 * main.cxx
 *
 * PWLib application source file for vidtest
 *
 * Main program entry point.
 *
 * Copyright (c) 2003 Equivalence Pty. Ltd.
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
 * $Log: main.cxx,v $
 * Revision 1.1  2003/04/28 08:18:42  craigs
 * Initial version
 *
 */

#include "precompile.h"
#include "main.h"
#include "version.h"


PCREATE_PROCESS(Vidtest);

#include  <ptlib/video.h>
#include  <ptclib/vsdl.h>

Vidtest::Vidtest()
  : PProcess("Equivalence", "vidtest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void Vidtest::Main()
{
  PArgList & args = GetArguments();

  args.Parse("-videodevice:");

  PString videoDevice = args.GetOptionString("videodevice");
  if (videoDevice.IsEmpty()) {
    cout << "Available video input devices:" << endl;
    PStringArray devs = PVideoInputDevice().GetDeviceNames();
    PINDEX i;
    for (i = 0; i < devs.GetSize(); i++)
      cout << "   " << devs[i] << endl;
    return;
  }

  PVideoInputDevice grabber;
  if (!grabber.Open(videoDevice, FALSE)) {
    PError << "Cannot open device " << videoDevice << endl;
    return;
  }

  if (!grabber.SetVideoFormat(PVideoDevice::NTSC)) {
    PTRACE(3, "Failed to set format");
    return;
  }

  if (!grabber.SetFrameRate(30)) {
    PTRACE(3, "Failed to set framerate");
    return;
  }

  PVideoChannel * channel = new PVideoChannel;
  grabber.Start();
  channel->AttachVideoReader(&grabber);

  BOOL isEncoding = FALSE;
  PSDLDisplayThread * sdlThread = new PSDLDisplayThread(FALSE);
  PSDLVideoDevice * display = new PSDLVideoDevice("VideoTest", isEncoding, sdlThread);
  display->SetFrameSize(channel->GetGrabWidth(), channel->GetGrabHeight());
  display->SetColourFormatConverter("YUV420P");

  channel->AttachVideoPlayer(display);

  cout << "Displaying video" << endl;

  char ch;
  cin >> ch;

  cout << "Closing down" << endl;
}


// End of File ///////////////////////////////////////////////////////////////
