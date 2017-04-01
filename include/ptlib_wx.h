/*
 * ptlib_wx.h
 *
 * Header file for incuding both PTLib and wxWidgets
 *
 * Copyright (c) 2014 Vox Lucida
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
 * The Original Code is Portable Tools Library
 *
 * The Initial Developer of the Original Code is Robert Jongbloed
 *
 * Contributor(s): ______________________________________.
 *
 */

#ifndef _PTLIB_WX_H
#define _PTLIB_WX_H

/* This hack is so can compile debug version of OpenPhone/OPAL/PTLib with
   the release only version of wxWidgets found in most distributions. */
#if !defined(NDEBUG) && defined(OPAL_WX_DEBUG_HACK)
  #define NDEBUG
  #include <wx/defs.h>
  #undef NDEBUG
#endif


// Total insanity ....
#if _WIN32
  #include <ptlib.h>
  #include <wx/wx.h>
#else
  #include <wx/wx.h>
  #include <ptlib.h>
#endif


// The SDL main override inteferes with wxWidgets, disable it
#if P_SDL && defined(P_MACOSX)
  #include <ptlib/videoio.h>
  #undef main
#endif


// And this interferes with wxWidgets XML
#ifdef LoadMenu
  #undef LoadMenu
#endif


// Allow for older versions of wxWidgets
#ifndef wxIMPLEMENT_APP
  #define wxIMPLEMENT_APP(app) IMPLEMENT_APP(app)
#endif


#endif // _PTLIB_WX_H

