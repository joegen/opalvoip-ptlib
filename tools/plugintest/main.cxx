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
  PError << "usage: plugintest dir\n";
}

void PluginTest::Main()
{
  PArgList & args = GetArguments();

  args.Parse("t:o:d:slx");

  PTrace::Initialise(args.GetOptionCount('t'),
                     args.HasOption('o') ? (const char *)args.GetOptionString('o') : NULL);

  if (args.HasOption('d')) {
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    pluginMgr.LoadPluginDirectory(args.GetOptionString('d'));
  }

  if (args.HasOption('s')) {
    cout << "Default device names = " << setfill(',') << PSoundChannel::GetDeviceNames(PSoundChannel::Player) << setfill(' ') << endl;
    cout << "Sound plugin names = " << setfill(',') << PSoundChannel::GetPluginNames() << setfill(' ') << endl;
    PSoundChannel * snd = new PSoundChannel();
    return;
  }

  if (args.HasOption('l')) {
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    PStringArray plugins = pluginMgr.GetPluginNames();
    cout << "Plugins loaded:" << setfill(',') << plugins << endl;
    return;
  }

  if (args.HasOption('x')) {
    PPluginManager & pluginMgr = PPluginManager::GetPluginManager();
    PPlugin * plugin = pluginMgr.GetPlugin("PSoundChannelOSS", "PSoundChannel");
    if (plugin == NULL) {
      cout << "No OSS plugin" << endl;
      return;
    }

    //cout << "Device names = " << PSoundChannelOSS_Static::GetDeviceNames(0) << endl;

    PStringArray (*dnFn)(int);
    if (!plugin->GetFunction("GetDeviceNames", (PDynaLink::Function &)dnFn)) {
      cout << "No GetDeviceNames function" << endl;
    }
    PStringArray names = (*dnFn)(0);
    cout << "Device names = " << names << endl;
  }

  if (args.GetCount() < 1) {
    Usage();
    return;
  }

}

// End of File ///////////////////////////////////////////////////////////////
