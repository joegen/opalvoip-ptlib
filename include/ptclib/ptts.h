/*
 * ptts.h
 *
 * Text To Speech classes
 *
 * Portable Windows Library
 *
 * Copyright (c) 2002 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _PTEXTTOSPEECH
#define _PTEXTTOSPEECH

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#ifndef _PTLIB_H
#include <ptlib.h>
#endif

#include <ptclib/ptts.h>

class PTextToSpeech : public PObject
{
  PCLASSINFO(PTextToSpeech, PObject);
  public:
    enum TextType {
      Default,
      Literal,
      Digits,
      Number,
      Currency,
      Time,
      Date,
      DateAndTime,
      Phone,
      IPAddress,
      Duration,
      Spell
    };

    virtual PStringArray GetVoiceList() = 0;
    virtual BOOL SetVoice(const PString & voice) = 0;

    virtual BOOL SetRate(unsigned rate) = 0;
    virtual unsigned GetRate() = 0;

    virtual BOOL SetVolume(unsigned volume) = 0;
    virtual unsigned GetVolume() = 0;

    virtual BOOL OpenFile   (const PFilePath & fn) = 0;
    virtual BOOL OpenChannel(PChannel * chanel) = 0;
    virtual BOOL IsOpen() = 0;

    virtual BOOL Close      () = 0;
    virtual BOOL Speak      (const PString & text, TextType hint = Default) = 0;
};

#endif
