/*
 * plugin.h
 *
 * Plugin interface class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2003 Post Increment
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
 * Contributor(s): Craig Southeren
 *                 Snark at GnomeMeeting
 *
 * $Log: plugin.h,v $
 * Revision 1.1.2.1  2003/10/07 01:33:19  csoutheren
 * Initial checkin of pwlib code to do plugins.
 * Modified from original code and concept provided by Snark of Gnomemeeting
 *
 */

#ifndef _PLUGIN_H
#define _PLUGIN_H

#define	PCREATE_PLUGIN_COMMON_API(classT, baseT) \
unsigned PWLibPlugin_GetAPIVersion()     { return 0; } \
char *   PWLibPlugin_GetClassName()      { return #classT; } \
char *   PWLibPlugin_GetBaseClassName()  { return #baseT; } \

/****************************************************

  support for C only plugins

 ****************************************************/

#ifndef __cplusplus

#define	PCREATE_PLUGIN(classT, baseT, typeT) \
PCREATE_PLUGIN_COMMON_API(classT, baseT, typeT) 

#endif // __cplusplus

/****************************************************

  support for C++ plugins

  With C++ plugins, we can use a static global to automatically
  register the plugin when the plugin code is statically linked

 ****************************************************/

#ifdef __cplusplus

#include <ptlib.h>

/***** dynamically loaded plugins */

#define	PCREATE_DYNAMIC_PLUGIN(classT, baseT) \
extern "C" { \
PCREATE_PLUGIN_COMMON_API(classT, baseT) \
}; 

/***** statically loaded plugins */

#include <ptlib/pluginmgr.h>
#define	PDECLARE_STATIC_PLUGIN_CLASS_START(classT, baseT) \
class classT##_Static : public PPlugin { \
  public: \
  unsigned GetAPIVersion() const     { return 0; } \
  PString  GetClassName() const      { return #classT; } \
  PString  GetBaseClassName() const  { return #baseT; } \

#define	PDECLARE_STATIC_PLUGIN_CLASS_END(classT, baseT) \
}; \
class classT##_Registration : public PPluginRegistration { \
  public: \
    classT##_Registration() \
      : PPluginRegistration(new classT##_Static) { } \
} classT##_Registration_Instance; \


#define	PCREATE_STATIC_PLUGIN(classT, baseT) \
PDECLARE_STATIC_PLUGIN_CLASS_START(classT, baseT) \
PDECLARE_STATIC_PLUGIN_CLASS_END(classT, baseT) 

/***** define macro the depends on confguration */

#ifdef P_HAS_PLUGINS
#  define	PCREATE_PLUGIN(classT, baseT)	PCREATE_DYNAMIC_PLUGIN(classT, baseT)
#else
#  define	PCREATE_PLUGIN(classT, baseT)	PCREATE_STATIC_PLUGIN(classT, baseT)
#endif // HAS_PLUGINS

#endif // __cplusplus

#endif // _PLUGIN_H

