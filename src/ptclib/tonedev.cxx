/*
 * tonedev.cxx
 *
 * Implementation of generated tones sound device
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

#ifdef __GNUC__
#pragma implementation "pwavfiledev.h"
#endif

#include <ptlib.h>

#if P_DTMF

#define P_FORCE_STATIC_PLUGIN 1

#include <ptclib/dtmf.h>

#include <ptlib/sound.h>
#include <ptclib/delaychan.h>

static const PConstCaselessString TonePrefix("Tones:");


class PSoundChannel_Tones : public PSoundChannelEmulation
{
    PCLASSINFO(PSoundChannel_Tones, PSoundChannelEmulation);
  public:
    PSoundChannel_Tones();
    ~PSoundChannel_Tones();

    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);

    bool Open(const Params & params);
    virtual PString GetName() const;
    PBoolean Close();
    PBoolean IsOpen() const;

protected:
    virtual bool RawWrite(const void * data, PINDEX size);
    virtual bool RawRead(void * data, PINDEX size);
    virtual bool Rewind();

    PString m_descriptor;
    PTones  m_tones;
};


PCREATE_SOUND_PLUGIN_EX(Tones, PSoundChannel_Tones,

  virtual const char * GetFriendlyName() const
  {
    return "Tone Generator Sound Input Channel";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR userData) const
  {
    if (TonePrefix != deviceName.Left(TonePrefix.GetLength()))
      return false;

    PSoundChannel_Tones test;
    return test.Open(PSoundChannel::Params((PSoundChannel::Directions)userData, deviceName));
  }
);


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PSoundChannel_Tones::PSoundChannel_Tones()
{
}


PSoundChannel_Tones::~PSoundChannel_Tones()
{
  Close();
}


PString PSoundChannel_Tones::GetName() const
{
  return TonePrefix + m_descriptor;
}


PStringArray PSoundChannel_Tones::GetDeviceNames(Directions dir)
{
  PStringArray devices;
  if (dir == Recorder)
    devices.AppendString(TonePrefix + "440:0.5/880:1.0");
  return devices;
}


bool PSoundChannel_Tones::Open(const Params & params)
{
  Close();

  if (params.m_direction != Recorder || !SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample))
    return false;

  if (PCaselessString(params.m_device).NumCompare(TonePrefix) != EqualTo)
    m_descriptor = params.m_device;
  else
    m_descriptor = params.m_device.Mid(TonePrefix.GetLength());
  if (m_descriptor.IsEmpty())
    return false;

  m_autoRepeat = m_descriptor[m_descriptor.GetLength()-1] != '$';
  if (!m_autoRepeat)
    m_descriptor.Delete(m_descriptor.GetLength()-1, 1);

  return m_tones.Generate(m_descriptor, m_sampleRate);
}


PBoolean PSoundChannel_Tones::IsOpen() const
{ 
  return !m_tones.IsEmpty();
}


PBoolean PSoundChannel_Tones::Close()
{
  if (CheckNotOpen())
    return false;

  m_tones.SetSize(0);
  os_handle = -1;
  return true;
}


bool PSoundChannel_Tones::RawWrite(const void *, PINDEX)
{
  return false;
}


bool PSoundChannel_Tones::RawRead(void * data, PINDEX size)
{
  PINDEX samples = std::min(size/2, m_tones.GetSize() - m_bufferPos);
  memcpy(data, &m_tones[m_bufferPos], SetLastReadCount(samples*sizeof(short)));

  m_bufferPos += samples;
  return m_bufferPos < m_tones.GetSize();
}


bool PSoundChannel_Tones::Rewind()
{
  m_bufferPos = 0;
  return true;
}


#endif // P_DTMF


// End of File ///////////////////////////////////////////////////////////////


