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

#include <ptlib.h>
#include <ptclib/pwavfiledev.h>


class PSoundChannel_WAVFile_PluginServiceDescriptor : public PDevicePluginServiceDescriptor
{
  public:
    virtual PObject * CreateInstance(int /*userData*/) const
    {
        return new PSoundChannel_WAVFile;
    }
    virtual PStringArray GetDeviceNames(int userData) const
    {
        return PSoundChannel_WAVFile::GetDeviceNames((PSoundChannel::Directions)userData);
    }
    virtual bool ValidateDeviceName(const PString & deviceName, int userData) const
    {
      PCaselessString adjustedDevice = deviceName;
      PINDEX length = adjustedDevice.GetLength();
      if (userData == PSoundChannel::Recorder && length > 5 && adjustedDevice.NumCompare(".wav*", 5, length-5) == PObject::EqualTo)
        adjustedDevice.Delete(length-1, 1);
      else if (length < 5 || adjustedDevice.NumCompare(".wav", 4, length-4) != PObject::EqualTo)
        return false;

      return PFile::Access(adjustedDevice, userData == PSoundChannel::Recorder ? PFile::ReadOnly : PFile::WriteOnly);
    }
} PSoundChannel_WAVFile_descriptor;

PCREATE_PLUGIN(WAVFile, PSoundChannel, &PSoundChannel_WAVFile_descriptor);
PINSTANTIATE_FACTORY(PSoundChannel, WAVFile)


#define new PNEW


///////////////////////////////////////////////////////////////////////////////

PSoundChannel_WAVFile::PSoundChannel_WAVFile()
  : m_autoRepeat(false)
{
}


PSoundChannel_WAVFile::PSoundChannel_WAVFile(const PString & device,
                                             Directions dir,
                                             unsigned numChannels,
                                             unsigned sampleRate,
                                             unsigned bitsPerSample)
  : m_autoRepeat(false)
{
  Open(device, dir, numChannels, sampleRate, bitsPerSample);
}


PSoundChannel_WAVFile::~PSoundChannel_WAVFile()
{
  Close();
}


PString PSoundChannel_WAVFile::GetName() const
{
  return m_WAVFile.GetFilePath();
}


PStringArray PSoundChannel_WAVFile::GetDeviceNames(Directions)
{
  PStringArray devices;
  devices.AppendString("*.wav");
  return devices;
}


PBoolean PSoundChannel_WAVFile::Open(const PString & device,
                                 Directions dir,
                                 unsigned numChannels,
                                 unsigned sampleRate,
                                 unsigned bitsPerSample)
{
  Close();
  if (dir == PSoundChannel::Player) {
    SetFormat(numChannels, sampleRate, bitsPerSample);
    return m_WAVFile.Open(device, PFile::WriteOnly);
  }

  PString adjustedDevice = device;
  PINDEX lastCharPos = adjustedDevice.GetLength()-1;
  if (adjustedDevice[lastCharPos] == '*') {
    adjustedDevice.Delete(lastCharPos, 1);
    m_autoRepeat = true;
  }

  if (!m_WAVFile.Open(adjustedDevice, PFile::ReadOnly))
    return PFalse;

  if (m_WAVFile.GetChannels() == numChannels &&
      m_WAVFile.GetSampleRate() == sampleRate &&
      m_WAVFile.GetSampleSize() == bitsPerSample)
    return PTrue;

  Close();
  return PFalse;
}


PBoolean PSoundChannel_WAVFile::IsOpen() const
{ 
  return m_WAVFile.IsOpen();
}

PBoolean PSoundChannel_WAVFile::SetFormat(unsigned numChannels,
                                      unsigned sampleRate,
                                      unsigned bitsPerSample)
{
  m_WAVFile.SetChannels(numChannels);
  m_WAVFile.SetSampleRate(sampleRate);
  m_WAVFile.SetSampleSize(bitsPerSample);

  return PTrue;
}


unsigned PSoundChannel_WAVFile::GetChannels() const
{
  return m_WAVFile.GetChannels();
}


unsigned PSoundChannel_WAVFile::GetSampleRate() const
{
  return m_WAVFile.GetSampleRate();
}


unsigned PSoundChannel_WAVFile::GetSampleSize() const
{
  return m_WAVFile.GetSampleSize();
}


PBoolean PSoundChannel_WAVFile::Close()
{
  if (!IsOpen())
    return SetErrorValues(NotOpen, EBADF);

  m_WAVFile.Close();
  os_handle = -1;
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::SetBuffers(PINDEX, PINDEX)
{
  return PFalse;
}


PBoolean PSoundChannel_WAVFile::GetBuffers(PINDEX & size, PINDEX & count)
{
  size = count = 0;
  return PFalse;
}


PBoolean PSoundChannel_WAVFile::Write(const void * data, PINDEX size)
{
  PBoolean ok = m_WAVFile.Write(data, size);
  lastWriteCount = m_WAVFile.GetLastWriteCount();
  m_Pacing.Delay(lastWriteCount*8/m_WAVFile.GetSampleSize()*1000/m_WAVFile.GetSampleRate());
  return ok;
}


PBoolean PSoundChannel_WAVFile::HasPlayCompleted()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::WaitForPlayCompletion()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::StartRecording()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::Read(void * data, PINDEX size)
{
  PBoolean ok = m_WAVFile.Read(data, size);
  if (!ok && m_autoRepeat) {
    m_WAVFile.SetPosition(0);
    ok = m_WAVFile.Read(data, size);
  }
  lastReadCount = m_WAVFile.GetLastReadCount();
  m_Pacing.Delay(lastReadCount*8/m_WAVFile.GetSampleSize()*1000/m_WAVFile.GetSampleRate());
  return ok;
}


PBoolean PSoundChannel_WAVFile::IsRecordBufferFull()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::AreAllRecordBuffersFull()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::WaitForRecordBufferFull()
{
  return PTrue;
}


PBoolean PSoundChannel_WAVFile::WaitForAllRecordBuffersFull()
{
  return PTrue;
}


// End of File ///////////////////////////////////////////////////////////////

