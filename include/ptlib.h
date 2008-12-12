/*
 * ptlib.h
 *
 * Umbrella include for all non-GUI classes.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef PTLIB_PTLIB_H
#define PTLIB_PTLIB_H

#ifdef __GNUC__

#pragma interface

#if !defined(__USE_STD__) && __GNUC__ >= 3
#define __USE_STD__
#endif

#endif

#ifdef __NUCLEUS_PLUS__
#include "nucpp.h"
#endif

#ifdef __USE_STD__
//using namespace std;
#endif

#include "ptbuildopts.h"
#include <ptlib/contain.h>

///////////////////////////////////////////////////////////////////////////////
// PTime

#include <ptlib/ptime.h>


///////////////////////////////////////////////////////////////////////////////
// PTimeInterval

#include <ptlib/timeint.h>


///////////////////////////////////////////////////////////////////////////////
// PTimer

#include <ptlib/timer.h>


///////////////////////////////////////////////////////////////////////////////
// PDirectory

#include <ptlib/pdirect.h>


///////////////////////////////////////////////////////////////////////////////
// PFilePath

#include <ptlib/filepath.h>


///////////////////////////////////////////////////////////////////////////////
// PConfig

#include <ptlib/config.h>


///////////////////////////////////////////////////////////////////////////////
// PArgList

#include <ptlib/args.h>


///////////////////////////////////////////////////////////////////////////////
// PThread

#include <ptlib/thread.h>


///////////////////////////////////////////////////////////////////////////////
// PProcess

//#include <ptlib/pprocess.h>


///////////////////////////////////////////////////////////////////////////////
// PSemaphore

#include <ptlib/semaphor.h>


///////////////////////////////////////////////////////////////////////////////
// PMutex

#include <ptlib/mutex.h>


///////////////////////////////////////////////////////////////////////////////
// PSyncPoint

#include <ptlib/syncpoint.h>


///////////////////////////////////////////////////////////////////////////////
// PSyncPointAck, PCondMutex etc

#include <ptlib/syncthrd.h>


///////////////////////////////////////////////////////////////////////////////
// PFactory

//#include <ptlib/pfactory.h>


///////////////////////////////////////////////////////////////////////////////
// PSharedPtr

#include <ptlib/psharedptr.h>

///////////////////////////////////////////////////////////////////////////////
// PDynaLink

#include <ptlib/dynalink.h>


///////////////////////////////////////////////////////////////////////////////
// PChannel

//#include <ptlib/channel.h>


///////////////////////////////////////////////////////////////////////////////
// PIndirectChannel

#include <ptlib/indchan.h>


///////////////////////////////////////////////////////////////////////////////
// PFile

#include <ptlib/file.h>


///////////////////////////////////////////////////////////////////////////////
// PTextFile

#include <ptlib/textfile.h>


///////////////////////////////////////////////////////////////////////////////
// PStructuredFile

#include <ptlib/sfile.h>


///////////////////////////////////////////////////////////////////////////////
// PConsoleChannel

#include <ptlib/conchan.h>


///////////////////////////////////////////////////////////////////////////////
// PluginManager

//#include <ptlib/pluginmgr.h>

///////////////////////////////////////////////////////////////////////////////
// PSound

//#include <ptlib/sound.h>


///////////////////////////////////////////////////////////////////////////////
// PVideoChannel

//#include <ptlib/video.h>


///////////////////////////////////////////////////////////////////////////////


#if P_USE_INLINES

#ifdef _WIN32
#include <ptlib/msos/ptlib/ptlib.inl>
#else
#include <ptlib/unix/ptlib/ptlib.inl>
#endif
#include <ptlib/osutil.inl>

#endif

#endif // PTLIB_PTLIB_H


// End Of File ///////////////////////////////////////////////////////////////
