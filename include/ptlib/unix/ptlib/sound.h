/*
 * sound.h
 *
 * Sound class.
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
 * $Log: sound.h,v $
 * Revision 1.10  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.9  2001/02/07 03:33:43  craigs
 * Added functions to get sound channel parameters
 *
 * Revision 1.8  2000/07/02 14:15:55  craigs
 * Fixed minor formatting issues
 *
 * Revision 1.7  2000/05/02 08:28:34  craigs
 * Removed "memory leaks" caused by brain-dead GNU linker
 *
 * Revision 1.6  2000/04/19 00:13:52  robertj
 * BeOS port changes.
 *
 * Revision 1.5  1999/07/19 01:34:22  craigs
 * Rewite to compensate for linux OSS sensitivity to ioctl order.
 *
 * Revision 1.4  1999/06/30 13:50:21  craigs
 * Added code to allow full duplex audio
 *
 * Revision 1.3  1998/11/30 22:07:13  robertj
 * New directory structure.
 *
 * Revision 1.2  1998/09/24 04:11:57  robertj
 * Added open software license.
 *
 */

#ifndef _PSOUND

#pragma interface


///////////////////////////////////////////////////////////////////////////////
// declare type for sound handle dictionary

#ifdef __BEOS__
class PSoundInput;
class PSoundPlayer;
#endif


///////////////////////////////////////////////////////////////////////////////
// PSound

#define _PSOUND_PLATFORM_INCLUDE
#include "../../sound.h"

#endif
#ifdef _PSOUND_PLATFORM_INCLUDE
#undef _PSOUND_PLATFORM_INCLUDE

  public:
    BOOL Close();
    BOOL Write(const void * buf, PINDEX len);
    BOOL Read(void * buf, PINDEX len);
  
  protected:
    BOOL  Setup();

    static PMutex dictMutex;

    Directions direction;
    PString device;
    BOOL isInitialised;

#ifdef __BEOS__
    PSoundInput* mpInput;
    PSoundPlayer* mpOutput;
#endif

    unsigned mNumChannels;
    unsigned mSampleRate;
    unsigned mBitsPerSample;
    unsigned actualSampleRate;

#endif


// End Of File ////////////////////////////////////////////////////////////////
