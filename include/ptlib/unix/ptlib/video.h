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
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: video.h,v $
 * Revision 1.1  2000/11/09 00:28:34  dereks
 * Initial release. Required for implementation of PVideoChannel.
 *
 *
 */

#ifndef _PVIDEO

#pragma interface

///////////////////////////////////////////////////////////////////////////////
// declare type for sound handle dictionary

#ifdef __BEOS__
class PVideoInput;
class PVideoPlayer;
#endif

///////////////////////////////////////////////////////////////////////////////
// PVideo

#include "../../video.h"
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
    PVideoInput* mpInput;
    PVideoPlayer* mpOutput;
    unsigned mNumChannels;
    unsigned mSampleRate;
    unsigned mBitsPerSample;
#endif
};


#endif
