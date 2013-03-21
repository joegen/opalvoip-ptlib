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
#include <ptclib/dtmf.h>

#if P_DTMF

#include <ptlib/sound.h>
#include <ptclib/delaychan.h>

static const PConstCaselessString TonePrefix("Tones:");


class PSoundChannel_Tones : public PSoundChannel
{
    PCLASSINFO(PSoundChannel_Tones, PSoundChannel);
  public:
    PSoundChannel_Tones();
    PSoundChannel_Tones(const PString &device,
                     PSoundChannel::Directions dir,
                     unsigned numChannels,
                     unsigned sampleRate,
                     unsigned bitsPerSample);
    ~PSoundChannel_Tones();
    static PStringArray GetDeviceNames(PSoundChannel::Directions = Player);
    PBoolean Open(
      const PString & device,
      Directions dir,
      unsigned numChannels,
      unsigned sampleRate,
      unsigned bitsPerSample
    );
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


class PSoundChannel_Tones_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject * CreateInstance(int /*userData*/) const
    {
        return new PSoundChannel_Tones;
    }
    virtual PStringArray GetDeviceNames(int userData) const
    {
        return PSoundChannel_Tones::GetDeviceNames((PSoundChannel::Directions)userData);
    }
    virtual bool ValidateDeviceName(const PString & deviceName, int userData) const
    {
      PSoundChannel_Tones test;
      return test.Open(deviceName, (PSoundChannel::Directions)userData, 1, 8000, 16);
    }
} PSoundChannel_Tones_descriptor;

PCREATE_PLUGIN(Tones, PSoundChannel, &PSoundChannel_Tones_descriptor);


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PSoundChannel_Tones::PSoundChannel_Tones()
  : m_Pacing(1000)
  , m_autoClose(false)
  , m_bufferSize(320)
  , m_bufferPosition(0)
{
}


PSoundChannel_Tones::PSoundChannel_Tones(const PString & device,
                                         Directions dir,
                                         unsigned numChannels,
                                         unsigned sampleRate,
                                         unsigned bitsPerSample)
  : m_autoClose(false)
  , m_bufferSize(320)
  , m_bufferPosition(0)
{
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
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
  if (dir == Player)
    devices.AppendString(TonePrefix + ":440/1.0");
  return devices;
}


PBoolean PSoundChannel_Tones::Open(const PString & deviceName,
                                   Directions dir,
                                   unsigned numChannels,
                                   unsigned sampleRate,
                                   unsigned bitsPerSample)
{
  Close();

  if (dir != Recorder || !SetFormat(numChannels, sampleRate, bitsPerSample))
    return false;

  if (PCaselessString(deviceName).NumCompare(TonePrefix) != EqualTo)
    return false;

  m_bufferPosition = 0;
  m_descriptor = deviceName.Mid(TonePrefix.GetLength());
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
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

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

  lastReadCount = samples*sizeof(short);
  memcpy(data, &m_tones[m_bufferPosition], lastReadCount);

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


