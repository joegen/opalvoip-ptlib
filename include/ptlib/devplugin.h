/*
 * devplugin.h
 *
 * Header file for "device" plugins
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
 * $Log: devplugin.h,v $
 * Revision 1.1.2.3  2003/10/28 02:45:21  dereksmithies
 * Fix warning about space between slash and newline.
 *
 * Revision 1.1.2.2  2003/10/20 03:25:12  dereksmithies
 * Fix GetFunction method so it is guaranteed to generate good results.
 *
 * Revision 1.1.2.1  2003/10/07 01:33:19  csoutheren
 * Initial checkin of pwlib code to do plugins.
 * Modified from original code and concept provided by Snark of Gnomemeeting
 *
 */

#ifndef _DEVPLUGIN_H
#define _DEVPLUGIN_H

#include <ptlib/plugin.h>
#include <ptlib/dynalink.h>

#define	PCREATE_DEVICE_PLUGIN_COMMON_API(classT, baseT, typeT) \
baseT   * Create() { return new classT(); } \
PStringArray GetDeviceNames(int dir) { return classT::GetDeviceNames((baseT::Directions)dir); } \
PString GetType() { return typeT; } \


/***** for dynamically loaded plugins */

#define	PCREATE_DYNAMIC_DEVICE_PLUGIN(classT, baseT, typeT) \
extern "C" { \
PCREATE_PLUGIN_COMMON_API(classT, baseT) \
PCREATE_DEVICE_PLUGIN_COMMON_API(classT, baseT, typeT) \
}; 

/***** for statically loaded plugins */

#define	PCREATE_STATIC_DEVICE_PLUGIN(classT, baseT, typeT) \
PDECLARE_STATIC_PLUGIN_CLASS_START(classT, baseT) \
  public: \
    static baseT * Create() \
      { return new classT(); } \
    static PStringArray GetDeviceNames(int dir) \
      { return classT##::GetDeviceNames((baseT##::Directions)dir); } \
    static PString GetType() \
      { return typeT; } \
    BOOL GetFunction(const PString & name, PDynaLink::Function & func) \
    { \
      void * p = NULL; \
      if (name *= "Create")         { p = (void *)&Create;          } else\
      if (name *= "GetType")        { p = (void *)&GetType;         } else\
      if (name *= "GetDeviceNames") { p = (void *)&GetDeviceNames;  } else\
      return FALSE; \
      func = (PDynaLink::Function &)p; \
      return TRUE; \
    } \
PDECLARE_STATIC_PLUGIN_CLASS_END(classT, baseT) 

/***** define macro the depends on confguration */

#ifdef P_HAS_PLUGINS
#  define	PCREATE_DEVICE_PLUGIN(classT, baseT, typeT)	PCREATE_DYNAMIC_DEVICE_PLUGIN(classT, baseT, typeT)
#else
#  define	PCREATE_DEVICE_PLUGIN(classT, baseT, typeT)	PCREATE_STATIC_DEVICE_PLUGIN(classT, baseT, typeT)
#endif // HAS_PLUGINS

#endif // _PLUGIN_H

