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
 * Revision 1.6  2003/12/14 10:01:25  rjongbloed
 * Resolved issue with name space conflict os static and virtual forms of GetDeviceNames() function.
 *
 * Revision 1.5  2003/12/08 01:28:52  dereksmithies
 * Compiles now with new video plugins.
 *
 * Revision 1.4  2003/11/04 03:21:26  dereksmithies
 * Fix compile on windows OS.
 *
 * Revision 1.3  2003/04/29 00:57:21  dereks
 * Add user interface, option setting for format/input/fake. Works on Linux.
 *
 * Revision 1.2  2003/04/28 14:30:21  craigs
 * Started rearranging code
 *
 * Revision 1.1  2003/04/28 08:18:42  craigs
 * Initial version
 *
 */


#include "precompile.h"
#include "main.h"
#include "version.h"


PCREATE_PROCESS(VidTest);

#include  <ptlib/video.h>
#include  <ptclib/vsdl.h>

VidTest::VidTest()
  : PProcess("Equivalence", "vidtest", MAJOR_VERSION, MINOR_VERSION, BUILD_TYPE, BUILD_NUMBER)
{
}


void VidTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse("-videodevice:"
             "-videoformat:"         "-no-videoformat."
             "-videoinput:"          "-no-videoinput."
#if PTRACING
             "o-output:"             "-no-output."
             "t-trace."              "-no-trace."
#endif
	     );

#if PTRACING
  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
         PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);
#endif

  PVideoInputDevice * grabber;

  cout << "Available video input devices:" << endl
       << "  video type              device name" << endl;
  PStringList drivers = PVideoInputDevice::GetDriverNames();
  for (int i = 0; i < drivers.GetSize(); i++) {
    PStringList devices = PVideoInputDevice::GetDriversDeviceNames(drivers[i]);
    for (int j = 0; j < devices.GetSize(); j++) {
      cout << "   " << drivers[i] << "                    " << devices[j] << endl;
    }
  }
  PString videoDevice = args.GetOptionString("videodevice");
  if (videoDevice.IsEmpty()) {
    cout << " No video device specified" << endl;
    return;
  }

  grabber = PVideoInputDevice::CreateDevice(videoDevice);
  if (grabber == NULL) {
    PError << "Cannot create video input device " << videoDevice << endl;
    return;
  }
    
  if (!grabber->Open(videoDevice, FALSE)) {
    PError << "Cannot open device " << videoDevice << endl;
    return;
  }

  BOOL videoIsPal = TRUE;
  if (args.HasOption("videoformat"))
    videoIsPal = args.GetOptionString("videoformat") *= "pal";

  if (!grabber->SetVideoFormat(videoIsPal ? PVideoDevice::PAL : PVideoDevice::NTSC)) {
    PError << "Failed to set format" << endl;
    return;
  }

  PINDEX videoInput = 0;
  if (args.HasOption("videoinput")) 
    videoInput = args.GetOptionString("videoinput").AsInteger();
  
  if (!grabber->SetChannel(videoInput)) {
    PError << "Failed to set channel to " << videoInput << endl;
    return;
  }

  if (!grabber->SetColourFormatConverter("YUV420P") ) {
    PError << "Failed to set format to yuv420p" << endl;
    return;
    }

  PINDEX newFrameRate = videoIsPal ? 25 : 30;
  if (!grabber->SetFrameRate(newFrameRate)) {
    PError <<  "Failed to set framerate" << newFrameRate << endl;
    return;
  }

  if  (!grabber->SetFrameSizeConverter(352, 288, FALSE)) {
    PError <<  "Failed to set framesize to " << 352  << "x" << 288 << endl;
    return;
  }

  channel = new PVideoChannel;
  channel->AttachVideoReader(grabber);

  grabber->Start();

  PThread * userInterfaceThread = new UserInterfaceThread(*this);
  
  BOOL isEncoding = TRUE;
  PSDLDisplayThread * sdlThread = new PSDLDisplayThread(FALSE);
  PSDLVideoDevice * display = new PSDLVideoDevice("VideoTest", isEncoding, sdlThread);

  PINDEX width = channel->GetGrabWidth();
  PINDEX height = channel->GetGrabHeight();
  PINDEX bytesInFrame = (width * height * 3) >> 1;

  PBYTEArray dataBuffer(bytesInFrame);

  display->SetFrameSize(width, height);
  display->SetColourFormatConverter("YUV420P");

  channel->AttachVideoPlayer(display);


  for( ;; ) {
    channel->Read(dataBuffer.GetPointer(), bytesInFrame);
    channel->Write((const void *)dataBuffer, 0);
     if (!exitFlag.WillBlock())
      break;
 }

  userInterfaceThread->Terminate();
  userInterfaceThread->WaitForTermination();
  delete userInterfaceThread;

  delete channel;

  sdlThread->Terminate();
  sdlThread->WaitForTermination();
  delete sdlThread;


  cout << "Closing down" << endl;
}

void VidTest::HandleUserInterface()
{
  PConsoleChannel console(PConsoleChannel::StandardInput);

  PTRACE(2, "VidTest\tTESTING interface thread started.");

  PStringStream help;
  help << "Select:\n"
          "  J   : Flip video input top to bottom\n"
          "  Q   : Exit program\n"
          "  X   : Exit program\n";

  for (;;) {

    // display the prompt
    cout << "(testing) Command ? " << flush;

    // terminate the menu loop if console finished
    char ch = (char)console.peek();
    if (console.eof()) {
      cout << "\nConsole gone - menu disabled" << endl;
      return;
    }

    console >> ch;
    switch (tolower(ch)) {
    case 'j' :
      if (!channel->ToggleVFlipInput())
	cout << "\nCould not toggle Vflip state of video input device" << endl;
      break;

    case 'x' :
    case 'q' :
      cout << "Exiting." << endl;
      exitFlag.Signal();
      console.ignore(INT_MAX, '\n');
      return;
      break;
    case '?' :
    default:
      cout << help << endl;
      break;

    } // end switch
  } // end for
}



// End of File ///////////////////////////////////////////////////////////////
