/*
 * pglobalstatic.cxx
 *
 * Various global statics that need to be instantiated upon startup
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is Post Increment
 *
 * Contributor(s): ______________________________________.
 *
 * $Log: pglobalstatic.cxx,v $
 * Revision 1.3  2005/01/11 06:57:15  csoutheren
 * Fixed namespace collisions with plugin starup factories
 *
 * Revision 1.2  2005/01/04 08:09:42  csoutheren
 * Fixed Linux configure problems
 *
 * Revision 1.1  2005/01/04 07:44:03  csoutheren
 * More changes to implement the new configuration methodology, and also to
 * attack the global static problem
 *
 */

#ifndef _PGLOBALSTATIC_CXX
#define _PGLOBALSTATIC_CXX

#include <ptbuildopts.h>
#include <ptlib/plugin.h>


//
// Load static video modules as required 
//
#if defined(P_VIDEO) && ! defined(NO_VIDEO_CAPTURE)

  #include <ptlib/videoio.h>

  PWLIB_STATIC_LOAD_PLUGIN(PVideoInputDevice_FakeVideo);
  PWLIB_STATIC_LOAD_PLUGIN(PVideoOutputDevice_NULLOutput);
  PWLIB_STATIC_LOAD_PLUGIN(PSoundChannel_WindowsMultimedia);

  #if defined(_WIN32) 
    PWLIB_STATIC_LOAD_PLUGIN(PVideoInputDevice_VideoForWindows)
  #endif

#endif

//
// Load static audio modules as required for Windows
//
#if defined(__BEOS__) && defined(P_AUDIO)
  PWLIB_STATIC_LOAD_PLUGIN(PSoundChannelBeOS);
#endif

//
// instantiate text to speech factory
//
#if defined(P_TTS)
  PLOAD_FACTORY_DECLARE(PTextToSpeech, PString)
#endif

//
// instantiate WAV file factory
//
#if defined(P_WAVFILE)
  PLOAD_FACTORY_DECLARE(PWAVFileConverter, unsigned)
  PLOAD_FACTORY_DECLARE(PWAVFileFormat,    unsigned)
#endif

//
// instantiate URL factory
//
#if defined(P_HTTP)
  PLOAD_FACTORY_DECLARE(PURLScheme, PString)
#endif


//
//  instantiate startup factory
//
#if defined(P_HAS_PLUGINS)
  PLOAD_FACTORY_DECLARE(PluginLoaderStartup, PString)
#endif


namespace PWLibStupidWindowsHacks 
{

#ifdef P_AUDIO
extern int loadSoundStuff;
#endif

#ifdef P_VIDEO
extern int loadVideoStuff;
#endif

};

//
// declare a simple class to execute on startup
//
static class PInstantiateMe
{
  public:
    PInstantiateMe();
} initialiser;

PInstantiateMe::PInstantiateMe()
{

#ifdef P_AUDIO
  PWLibStupidWindowsHacks::loadSoundStuff = 1;
#endif

#ifdef P_VIDEO
  PWLibStupidWindowsHacks::loadVideoStuff = 1;
#endif

#if defined(P_TTS)
  PLOAD_FACTORY(PTextToSpeech, PString)
#endif

#if defined(P_WAVFILE)
  PLOAD_FACTORY(PWAVFileConverter, unsigned)
  PLOAD_FACTORY(PWAVFileFormat,    unsigned)
#endif

#if defined(P_HTTP)
  PLOAD_FACTORY(PURLScheme, PString)
#endif

#if defined(P_HAS_PLUGINS)
  PLOAD_FACTORY(PluginLoaderStartup, PString)
#endif

}

#endif // _PGLOBALSTATIC_CXX

