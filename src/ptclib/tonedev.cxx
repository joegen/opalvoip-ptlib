/*
 * pwavfiledev.cxx
 *
 * Implementation of sound file device
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
 *
 * $Revision$
 * $Author$
 * $Date$
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


class PSoundChannel_Tones : public PSoundChannel
{
    PCLASSINFO(PSoundChannel_Tones, PSoundChannel);
  public:
    PSoundChannel_Tones();
    ~PSoundChannel_Tones();
    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);
    bool Open(const Params & params);
    virtual PString GetName() const;
    PBoolean Close();
    PBoolean IsOpen() const;
    PBoolean Write(const void * buf, PINDEX len);
    PBoolean Read(void * buf, PINDEX len);
    PBoolean SetFormat(unsigned numChannels,
                   unsigned sampleRate,
                   unsigned bitsPerSample);
    unsigned GetChannels() const;
    unsigned GetSampleRate() const;
    unsigned GetSampleSize() const;
    PBoolean SetBuffers(PINDEX size, PINDEX count);
    PBoolean GetBuffers(PINDEX & size, PINDEX & count);
    PBoolean HasPlayCompleted();
    PBoolean WaitForPlayCompletion();
    PBoolean StartRecording();
    PBoolean IsRecordBufferFull();
    PBoolean AreAllRecordBuffersFull();
    PBoolean WaitForRecordBufferFull();
    PBoolean WaitForAllRecordBuffersFull();

protected:
    PString        m_descriptor;
    PTones         m_tones;
    PAdaptiveDelay m_Pacing;
    bool           m_autoClose;
    PINDEX         m_bufferSize;
    PINDEX         m_bufferPosition;
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
  : m_Pacing(1000)
  , m_autoClose(false)
  , m_bufferSize(320)
  , m_bufferPosition(0)
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
  m_bufferPosition = 0;

  if (params.m_direction != Recorder || !SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample))
    return false;

  if (PCaselessString(params.m_device).NumCompare(TonePrefix) != EqualTo)
    m_descriptor = params.m_device;
  else
    m_descriptor = params.m_device.Mid(TonePrefix.GetLength());
  if (m_descriptor.IsEmpty())
    return false;

  m_autoClose = m_descriptor[m_descriptor.GetLength()-1] == '$';
  if (m_autoClose)
    m_descriptor.Delete(m_descriptor.GetLength()-1, 1);

  return m_tones.Generate(m_descriptor);
}


PBoolean PSoundChannel_Tones::IsOpen() const
{ 
  return !m_tones.IsEmpty();
}


PBoolean PSoundChannel_Tones::SetFormat(unsigned numChannels,
                                        unsigned sampleRate,
                                        unsigned bitsPerSample)
{
  if (numChannels != 1 || bitsPerSample != 16)
    return false;

  if (!IsOpen())
    return true;

  return m_tones.Generate(m_descriptor, sampleRate);
}


unsigned PSoundChannel_Tones::GetChannels() const
{
  return 1;
}


unsigned PSoundChannel_Tones::GetSampleRate() const
{
  return m_tones.GetSampleRate();
}


unsigned PSoundChannel_Tones::GetSampleSize() const
{
  return 16;
}


PBoolean PSoundChannel_Tones::Close()
{
  if (CheckNotOpen())
    return false;

  m_tones.SetSize(0);
  os_handle = -1;
  return true;
}


PBoolean PSoundChannel_Tones::SetBuffers(PINDEX size, PINDEX /*count*/)
{
  m_bufferSize = size;
  return true;
}


PBoolean PSoundChannel_Tones::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = m_bufferSize;
  count = 1;
  return true;
}


PBoolean PSoundChannel_Tones::Write(const void *, PINDEX)
{
  return false;
}


PBoolean PSoundChannel_Tones::HasPlayCompleted()
{
  return true;
}


PBoolean PSoundChannel_Tones::WaitForPlayCompletion()
{
  return true;
}


PBoolean PSoundChannel_Tones::StartRecording()
{
  return true;
}


PBoolean PSoundChannel_Tones::Read(void * data, PINDEX size)
{
  PINDEX samples = std::min(size/2, m_tones.GetSize() - m_bufferPosition);
  memcpy(data, &m_tones[m_bufferPosition], SetLastReadCount(samples*sizeof(short)));

  m_bufferPosition += samples;
  if (m_bufferPosition >= m_tones.GetSize()) {
    if (m_autoClose)
      Close();
    else
      m_bufferPosition = 0;
  }

  m_Pacing.Delay(1000*samples/GetSampleRate());
  return true;
}


PBoolean PSoundChannel_Tones::IsRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannel_Tones::AreAllRecordBuffersFull()
{
  return true;
}


PBoolean PSoundChannel_Tones::WaitForRecordBufferFull()
{
  return true;
}


PBoolean PSoundChannel_Tones::WaitForAllRecordBuffersFull()
{
  return true;
}


#endif // P_WAVFILE


// End of File ///////////////////////////////////////////////////////////////


