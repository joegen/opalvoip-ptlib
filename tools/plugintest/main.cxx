/*
 * main.cxx
 *
 * PWLib application source file for PluginTest
 *
 * Main program entry point.
 *
 * Copyright 2003 Equivalence
 *
 * $Log: main.cxx,v $
 * Revision 1.1.2.3  2003/10/12 21:22:12  dereksmithies
 * Add ability to play sample sound out PSoundChannel - illustrating operation of plugins.
 *
 * Revision 1.1.2.2  2003/10/08 03:55:54  dereksmithies
 * Add lots of debug statements, fix option parsing, improve Usage() function.
 *
 * Revision 1.1.2.1  2003/10/07 01:52:39  csoutheren
 * Test program for plugins
 *
 * Revision 1.3  2003/04/22 23:25:13  craigs
 * Changed help message for SRV records
 *
 * Revision 1.2  2003/04/15 08:15:16  craigs
 * Added single string form of GetSRVRecords
 *
 * Revision 1.1  2003/04/15 04:12:38  craigs
 * Initial version
 *
 */

#include <ptlib.h>
#include <ptlib/pluginmgr.h>
#include <ptlib/sound.h>
#include "main.h"

PCREATE_PROCESS(PluginTest);

PluginTest::PluginTest()
  : PProcess("Equivalence", "PluginTest", 1, 0, AlphaCode, 1)
{
}

void Usage()
{
  PError << "usage: plugintest dir\n \n"
	 << "-l List ALL plugins regardless of type\n"
	 << "-s Show the list of loaded sound plugin drivers\n"
	 << "-d dir Set the directory from which plugins are loaded\n"
	 << "-x Attempt to load the OSS sound plugin\n"
	 << "-t (more t's for more detail) logging on\n"
	 << "-o output file for logging \n"
	 << "-p play a beep beep beep sound, and testtest created PSoundChannel\n"
	 << "-h print this help\n";

}

void PluginTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse(
	     "t-trace."              "-no-trace."   
	     "o-output:"             "-no-output."
	     "l-list."               "-no-list."
	     "x-xamineOSS."          "-no-xamineOSS."
	     "s-soundPlugins."       "-no-soundPlugins."
	     "d-directory:"          "-no-directory."
	     "p-play."               "-no-play."
	     "h-help."               "-no-help."
	     );

  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL,
		     PTrace::Blocks | PTrace::Timestamp | PTrace::Thread | PTrace::FileAndLine);

  if (args.HasOption('d')) {
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    pluginMgr.LoadPluginDirectory(args.GetOptionString('d'));
  }

  if (args.HasOption('s')) {
    cout << "Examine PSoundChannel" <<endl;
    cout << "Default device names = " << setfill(',') << PSoundChannel::GetDeviceNames(PSoundChannel::Player) << setfill(' ') << endl;
    cout << "Sound plugin names = " << setfill(',') << PSoundChannel::GetPluginNames() << setfill(' ') << endl;
    PSoundChannel * snd = new PSoundChannel();
    cout << "PSoundChannel has a name of \"" << snd->GetClass() << "\"" << endl 
	 << endl;
  }

  if (args.HasOption('l')) {
    cout << "List all available plugins" << endl;
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    PStringArray plugins = pluginMgr.GetPluginNames();
    cout << "Plugins loaded = " << setfill(',') << plugins << endl
	 << endl;
  }

  if (args.HasOption('x')) {
    cout << "Examine PSoundChannelOSS" << endl;
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    PPlugin * plugin = pluginMgr.GetPlugin("PSoundChannelOSS", "PSoundChannel");
    if (plugin == NULL) {
      cout << "No OSS plugin to examine, so exit immediately" << endl;
      goto end_program;
    }

    //cout << "Device names = " << PSoundChannelOSS_Static::GetDeviceNames(0) << endl;

#define SAMPLES 64000  /*8 seconds of data */

#define GET_FUNCTION(x, y) \
   if (!plugin->GetFunction(#x, (PDynaLink::Function &)y)) {     \
      cout << " no " << #x << " function " << dlerror() << endl; \
      goto end_program;                                          \
   }
 
    PString      (*dnType)();
    PStringArray (*dnFn)(int);

    GET_FUNCTION(GetDeviceNames, dnFn);    /*GetDeviceNames is a static method in the sound_oss lib*/
    PStringArray names = (*dnFn)(0);
    cout << "Device names = " << names << endl
	 << endl;

    GET_FUNCTION(GetType, dnType);         /*GetType is a static method in the sound_oss lib. */
    cout << "Type is " << (*dnType)() << endl << endl;

    if (args.HasOption('p')) {
      cout << "Play a test beep beep beep sound out the PSoundChannel" << endl;
      PSoundChannel * snd = new PSoundChannel();
      PString deviceName = snd->GetDefaultDevice(PSoundChannel::Player);

      if (!snd->Open(deviceName, PSoundChannel::Player)) {
	cout << "Failed to open " << deviceName << " for sound. End program now." << endl;
	goto end_program;
      }

      cout << "sound channel has a handle of " << snd->GetHandle() << endl;
      if (!snd->IsOpen()) {
	cout << "Sound device is not open. Sorry. End program now." << endl;
	goto end_program;
      }
      
      if (!snd->SetBuffers(SAMPLES, 2)) {
	cout << "Failed to set samples to " << SAMPLES << " and 2 buffers. End program now." << endl;
	goto end_program;
      }

      snd->SetVolume(90);

      PWORDArray audio(SAMPLES);
      int i, pointsPerCycle = 80;
      int volume = 80;
      double angle;

      for (i = 0; i < SAMPLES; i++) {
	angle = M_PI * 2 * (double)(i % pointsPerCycle)/pointsPerCycle;
	if ((i % 4000) < 3000)
	  audio[i] = (unsigned short) ((16384 * cos(angle) * volume)/100);
	else
	  audio[i] = 0;
      }
	
      if (!snd->Write((unsigned char *)audio.GetPointer(), SAMPLES * 2)) {
	cout << "Failed to write  " << SAMPLES/8000  << " seconds of beep beep. End program now." << endl;
	goto end_program;
      }

      snd->WaitForPlayCompletion();
    } /* Play beep beep*/
  }

 end_program:
  PTrace::ClearOptions(0);
  PTrace::SetLevel(0);
}

// End of File ///////////////////////////////////////////////////////////////
