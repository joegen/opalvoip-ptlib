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
    cout << "PSoundChannel has a name of \"" << snd->GetName() << "\"" << endl 
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

    PStringArray (*dnFn)(int);
    if (!plugin->GetFunction("GetDeviceNames", (PDynaLink::Function &)dnFn)) {
      cout << "No GetDeviceNames function" << endl;
    }
    PStringArray names = (*dnFn)(0);
    cout << "Device names = " << names << endl
	 << endl;;
  }

 end_program:
  PTrace::ClearOptions(0);
  PTrace::SetLevel(0);
}

// End of File ///////////////////////////////////////////////////////////////
