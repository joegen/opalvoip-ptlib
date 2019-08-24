/*
 * pwavfiledev.cxx
 *
 * Sound file device declaration
 *
 * Portable Windows Library
 *
 * Copyright (C) 2007 Post Increment
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
 * The Initial Developer of the Original Code is
 * Robert Jongbloed <robertj@postincrement.com>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_PWAVFILEDEV_H
#define PTLIB_PWAVFILEDEV_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#include <ptlib/sound.h>
#include <ptclib/pwavfile.h>

#if defined(P_WAVFILE)


///////////////////////////////////////////////////////////////////////////////////////////
//
// This class defines a sound channel device that reads audio from a raw WAV file
//

class PSoundChannel_WAVFile : public PSoundChannelEmulation
{
  PCLASSINFO(PSoundChannel_WAVFile, PSoundChannelEmulation);
  public:
    PSoundChannel_WAVFile();
    ~PSoundChannel_WAVFile();

    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);

    virtual bool Open(const Params & params);
    virtual PString GetName() const;
    virtual PBoolean Close();
    virtual PBoolean IsOpen() const;

  protected:
    virtual bool RawWrite(const void * data, PINDEX size);
    virtual bool RawRead(void * data, PINDEX size);
    virtual bool Rewind();

    PWAVFile m_WAVFile;
};


#endif // defined(P_WAVFILE)

#endif // PTLIB_PWAVFILEDEV_H


// End Of File ///////////////////////////////////////////////////////////////
