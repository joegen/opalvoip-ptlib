/*
 * pwavfile.cxx
 *
 * WAV file I/O channel class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 2001 Equivalence Pty. Ltd.
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
 * Roger Hardiman <roger@freebsd.org>
 * and Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
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
#pragma implementation "pwavfile.h"
#endif

#include <ptlib.h>
#include <ptclib/pwavfile.h>

#if P_WAVFILE

#include <ptlib/pfactory.h>
#include <ptlib/sound.h>


#define new PNEW
#define PTraceModule() "WAVFile"


const char WAVLabelRIFF[4] = { 'R', 'I', 'F', 'F' };
const char WAVLabelWAVE[4] = { 'W', 'A', 'V', 'E' };
const char WAVLabelFMT_[4] = { 'f', 'm', 't', ' ' };
const char WAVLabelDATA[4] = { 'd', 'a', 't', 'a' };


inline PBoolean ReadAndCheck(PWAVFile & file, void * buf, PINDEX len)
{
  return file.PFile::Read(buf, len) && (file.PFile::GetLastReadCount() == len);
}


#if PBYTE_ORDER==PBIG_ENDIAN
#  if defined(USE_SYSTEM_SWAB)
#    define SWAB(a,b,c) ::swab(a,b,c)
#  else
static void SWAB(const void * void_from, void * void_to, register size_t len)
{
  register const char * from = (const char *)void_from;
  register char * to = (char *)void_to;

  while (len > 1) {
    char b = *from++;
    *to++ = *from++;
    *to++ = b;
    len -= 2;
  }
}
#  endif
#else
#  define SWAB(a,b,c) {}
#endif


///////////////////////////////////////////////////////////////////////////////
// PWAVFile

PWAVFile::PWAVFile(unsigned fmt)
{
  Construct(WriteOnly);
  SetFormat(fmt);
}


PWAVFile::PWAVFile(OpenMode mode, OpenOptions opts, unsigned fmt)
  : PFile(mode, opts)
{
  Construct(mode);
  SetFormat(fmt);
}


PWAVFile::PWAVFile(const PFilePath & name, OpenMode mode, OpenOptions opts, unsigned fmt)
{
  Construct(mode);
  if (SetFormat(fmt))
    Open(name, mode, opts);
}


PWAVFile::PWAVFile(const PString & format, const PFilePath & name, OpenMode mode, OpenOptions opts)
{
  Construct(mode);
  if (SetFormat(format))
    Open(name, mode, opts);
}


PWAVFile::~PWAVFile()
{ 
  Close();
  delete m_autoConverter;
  delete m_formatHandler;
}


void PWAVFile::Construct(OpenMode mode)
{
  m_createFormat = fmt_PCM;
  m_headerLength = 0;
  m_dataLength = 0;
  m_formatHandler = NULL;
  m_autoConverter = NULL;
  m_status = mode == WriteOnly ? e_PreWrite : e_Reading;

  memset(&m_wavFmtChunk, 0, sizeof(m_wavFmtChunk));
  m_wavFmtChunk.hdr.len = sizeof(m_wavFmtChunk) - sizeof(m_wavFmtChunk.hdr);

  m_readSampleRate = m_readChannels = 0;  // Zero means automatically set in ProcessHeader
  m_readBufCount = m_readBufPos = 0;
}


PWAVFile * PWAVFile::format(const PString & format)
{
  PWAVFile * file = new PWAVFile;
  file->SetFormat(format);
  return file;
}


PWAVFile * PWAVFile::format(const PString & format, PFile::OpenMode mode, OpenOptions opts)
{
  PWAVFile * file = new PWAVFile(mode, opts);
  file->SetFormat(format);
  return file;
}


bool PWAVFile::InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions)
{
  if (!PFile::InternalOpen(mode, opts, permissions))
    return false;

  m_status = mode == WriteOnly ? e_PreWrite : e_Reading;
  if (!(m_status != e_Reading ? GenerateHeader() : ProcessHeader())) {
    Close();
    return false;
  }

  // if we did not know the format when we opened, then we had better know it now
  if (m_formatHandler == NULL) {
    Close();
    SetErrorValues(BadParameter, EINVAL);
    return false;
  }

  return true;
}


PBoolean PWAVFile::Close()
{
  if (m_status == e_Writing)
    UpdateHeader();

  return PFile::Close();
}


PString PWAVFile::GetFormatString() const
{
  if (m_formatHandler != NULL) 
    return m_formatHandler->GetFormatString();

  static const PConstString na("N/A");
  return na;
}


bool PWAVFile::SetAutoconvert(bool convert)
{
  if (m_status != e_Reading)
    return false;

  delete m_autoConverter;
  m_autoConverter = NULL;

  if (!convert)
    return true;

  if ((m_autoConverter = PWAVFileConverterFactory::CreateInstance(m_wavFmtChunk.format)) != NULL)
    return true;

  PTRACE(2, "No format converter for type " << (WORD)m_wavFmtChunk.format);
  return false;
}


// Performs necessary byte-order swapping on for big-endian platforms.
PBoolean PWAVFile::Read(void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  if (m_wavFmtChunk.sampleRate == m_readSampleRate && m_wavFmtChunk.numChannels == m_readChannels)
    return m_autoConverter != NULL ? m_autoConverter->Read(*this, buf, len) : RawRead(buf, len);

  if (GetSampleSize() != sizeof(short)* 8) {
    PTRACE(2, "Only 16 bit PCM supported in WAV file conversion.");
    return false;
  }

  if (m_readBufPos >= m_readBufCount) {
    if (!m_readBuffer.SetSize(10 * m_wavFmtChunk.sampleRate*m_wavFmtChunk.numChannels)) // 10 seconds worth
      return false;
    void * ptr = m_readBuffer.GetPointer();
    PINDEX sz = m_readBuffer.GetSize()*sizeof(short);
    if (!(m_autoConverter != NULL ? m_autoConverter->Read(*this, ptr, sz) : RawRead(ptr, sz)))
      return false;
    m_readBufCount = GetLastReadCount()/sizeof(short);
    m_readBufPos = 0;
  }

  PINDEX srcSize = (m_readBufCount - m_readBufPos)*sizeof(short);
  lastReadCount = len;
  PSound::ConvertPCM(&m_readBuffer[m_readBufPos], srcSize, m_wavFmtChunk.sampleRate, m_wavFmtChunk.numChannels,
                     (short *)buf, lastReadCount, m_readSampleRate, m_readChannels);
  m_readBufPos += srcSize / sizeof(short);
  return true;
}


bool PWAVFile::RawRead(void * buf, PINDEX len)
{
  // Some wav files have extra data after the sound samples in a LIST chunk.
  // e.g. WAV files made in GoldWave have a copyright and a URL in this chunk.
  // We do not want to return this data by mistake.
  PINDEX fileLength = m_headerLength + m_dataLength;

  off_t pos = PFile::GetPosition();
  if (pos >= fileLength) {
    // indicate eof (return false, but error=0, lastReadCount=0)
    lastReadCount = 0;
    ConvertOSError(0, LastReadError);
    return false;
  }

  if ((pos + len) > fileLength)
    len = fileLength - pos;

  if (m_formatHandler != NULL)
    return m_formatHandler->Read(*this, buf, len);

  return PFile::Read(buf, len);
}


// Performs necessary byte-order swapping on for big-endian platforms.
PBoolean PWAVFile::Write(const void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  // Needs to update header on close.
  m_status = e_Writing;

  if (m_autoConverter != NULL)
    return m_autoConverter->Write(*this, buf, len);

  return RawWrite(buf, len);
}


bool PWAVFile::RawWrite(const void * buf, PINDEX len)
{
  if (m_formatHandler != NULL)
    return m_formatHandler->Write(*this, buf, len);

  return PFile::Write(buf, len);
}


// Functions that are offset by m_headerLength.
off_t PWAVFile::GetLength() const
{
  switch (m_status) {
    case e_Reading:
      return m_dataLength;

    case e_Writing:
      return PFile::GetLength() - m_headerLength;

    default :
      return 0;
  }
}


PBoolean PWAVFile::SetLength(off_t)
{
  PAssertAlways("PWAVFile::SetLength() is not allowed");
  return false;
}


PBoolean PWAVFile::SetPosition(off_t pos, FilePositionOrigin origin)
{
  if (m_autoConverter != NULL)
    return m_autoConverter->SetPosition(*this, pos, origin);

  return RawSetPosition(pos, origin);
}


bool PWAVFile::RawSetPosition(off_t pos, FilePositionOrigin origin)
{
  return PFile::SetPosition(pos + m_headerLength, origin);
}


off_t PWAVFile::GetPosition() const
{
  if (m_autoConverter != NULL)
    return m_autoConverter->GetPosition(*this);

  return RawGetPosition();
}


off_t PWAVFile::RawGetPosition() const
{
  return PFile::GetPosition() - m_headerLength;
}


unsigned PWAVFile::GetFormat() const
{
  return m_wavFmtChunk.format;
}


unsigned PWAVFile::GetChannels() const
{
  if (m_status == e_Reading && m_readChannels > 0)
    return m_readChannels;
  return m_wavFmtChunk.numChannels;
}


void PWAVFile::SetChannels(unsigned channels) 
{
  switch (m_status) {
    case e_Reading :
      m_readChannels = channels;
      break;

    case e_PreWrite :
      if (m_formatHandler == NULL || m_formatHandler->CanSetChannels(channels)) {
        m_wavFmtChunk.numChannels = (WORD)channels;
        if (m_wavFmtChunk.format == fmt_PCM) {
          m_wavFmtChunk.bytesPerSample = (m_wavFmtChunk.bitsPerSample/8) * m_wavFmtChunk.numChannels;
          m_wavFmtChunk.bytesPerSec = m_wavFmtChunk.sampleRate * m_wavFmtChunk.bytesPerSample;
        }
        break;
      }
      // DO default case

    default :
      PTRACE(2, "SetChannels ignored after write started, or number of channels unsupported");
  }
}


unsigned PWAVFile::GetSampleRate() const
{
  if (m_status == e_Reading && m_readSampleRate > 0)
    return m_readSampleRate;
  return m_wavFmtChunk.sampleRate;
}


void PWAVFile::SetSampleRate(unsigned rate) 
{
  switch (m_status) {
    case e_Reading :
      m_readSampleRate = rate;
      break;

    case e_PreWrite :
      m_wavFmtChunk.sampleRate = (WORD)rate;
      if (m_wavFmtChunk.format == fmt_PCM)
        m_wavFmtChunk.bytesPerSec = m_wavFmtChunk.sampleRate * m_wavFmtChunk.bytesPerSample;
      break;

    default :
      PTRACE(2, "SetSampleRate ignored after write started");
  }
}


unsigned PWAVFile::GetSampleSize() const
{
  return m_wavFmtChunk.bitsPerSample;
}


void PWAVFile::SetSampleSize(unsigned v) 
{
  if (m_status == e_PreWrite) {
    m_wavFmtChunk.bitsPerSample = (WORD)v;
    if (m_wavFmtChunk.format == fmt_PCM) {
      m_wavFmtChunk.bytesPerSample = m_wavFmtChunk.bitsPerSample*8;
      m_wavFmtChunk.bytesPerSec = m_wavFmtChunk.sampleRate * m_wavFmtChunk.bytesPerSample;
    }
  }
}


unsigned PWAVFile::GetBytesPerSecond() const
{
  return m_wavFmtChunk.bytesPerSec;
}


void PWAVFile::SetBytesPerSecond(unsigned v)
{
  if (m_status == e_PreWrite)
    m_wavFmtChunk.bytesPerSec = (WORD)v;
}


PBoolean PWAVFile::SetFormat(unsigned fmt)
{
  return SelectFormat(PWAVFileFormatByIDFactory::CreateInstance(fmt));
}


PBoolean PWAVFile::SetFormat(const PString & format)
{
  PWAVFileFormat * handler = PWAVFileFormatByFormatFactory::CreateInstance(format);
  if (handler == NULL)
    handler = PWAVFileFormatByIDFactory::CreateInstance(format.AsUnsigned());
  return SelectFormat(handler);
}


bool PWAVFile::SelectFormat(PWAVFileFormat * handler)
{
  if (handler == NULL)
    return false;

  delete m_formatHandler;
  m_formatHandler = handler;

  if (m_status == e_PreWrite)
    m_wavFmtChunk.format = (WORD)m_formatHandler->GetFormat();

  return SetAutoconvert(m_autoConverter != NULL);
}


PBoolean PWAVFile::ProcessHeader() 
{
  // Process the header information
  // This comes in 3 or 4 chunks, either RIFF, FORMAT and DATA
  // or RIFF, FORMAT, FACT and DATA.

  // go to the beginning of the file
  if (!PFile::SetPosition(0)) {
    PTRACE(1,"ProcessHeader: Cannot Set Pos");
    return (false);
  }

  // Read the RIFF chunk.
  struct PWAV::RIFFChunkHeader riffChunk;
  if (!ReadAndCheck(*this, &riffChunk, sizeof(riffChunk)))
    return false;

  // check if tags are correct
  if (strncmp(riffChunk.hdr.tag, WAVLabelRIFF, sizeof(WAVLabelRIFF)) != 0) {
    PTRACE(1,"ProcessHeader: Not RIFF");
    return (false);
  }

  if (strncmp(riffChunk.tag, WAVLabelWAVE, sizeof(WAVLabelWAVE)) != 0) {
    PTRACE(1,"ProcessHeader: Not WAVE");
    return (false);
  }

  // Read the known part of the FORMAT chunk.
  if (!ReadAndCheck(*this, &m_wavFmtChunk, sizeof(m_wavFmtChunk)))
    return false;

  // check if labels are correct
  if (strncmp(m_wavFmtChunk.hdr.tag, WAVLabelFMT_, sizeof(WAVLabelFMT_)) != 0) {
    PTRACE(1,"ProcessHeader: Not FMT");
    return (false);
  }

  // if we opened the file without knowing the format, then try and set the format now
  if (!SetFormat(m_wavFmtChunk.format))
    return false;

  // read the extended format chunk (if any)
  m_extendedHeader.SetSize(0);
  if ((size_t)m_wavFmtChunk.hdr.len > (sizeof(m_wavFmtChunk) - sizeof(m_wavFmtChunk.hdr))) {
    m_extendedHeader.SetSize(m_wavFmtChunk.hdr.len - (sizeof(m_wavFmtChunk) - sizeof(m_wavFmtChunk.hdr)));
    if (!ReadAndCheck(*this, m_extendedHeader.GetPointer(), m_extendedHeader.GetSize()))
      return false;
  }

  // give format handler a chance to read extra chunks
  if (!m_formatHandler->ReadExtraChunks(*this))
    return false;

  // ignore chunks until we see a DATA chunk
  for (;;) {
    PWAV::ChunkHeader chunkHeader;
    if (!ReadAndCheck(*this, &chunkHeader, sizeof(chunkHeader)))
      return false;

    if (strncmp(chunkHeader.tag, WAVLabelDATA, sizeof(WAVLabelDATA)) == 0) {
      m_dataLength = chunkHeader.len;
      break;
    }

    if (!PFile::SetPosition(PFile::GetPosition() + + chunkHeader.len)) {
      PTRACE(1,"ProcessHeader: Cannot set new position");
      return false;
    }
  }

  // calculate the size of header and data for accessing the WAV data.
  m_headerLength = PFile::GetPosition(); 

  m_formatHandler->OnStart();

  if (m_readSampleRate == 0)
    m_readSampleRate = m_wavFmtChunk.sampleRate;
  if (m_readChannels == 0)
    m_readChannels = m_wavFmtChunk.numChannels;

#if PTRACING
  static unsigned const Level = 4;
  if (PTrace::CanTrace(Level)) {
    ostream & trace = PTRACE_BEGIN(Level);
    trace << "Opened \"" << GetFilePath() << "\" at " << m_readSampleRate << "Hz";
    if (m_readSampleRate != m_wavFmtChunk.sampleRate)
      trace << " (converted from " << m_wavFmtChunk.sampleRate << "Hz)";
    trace << " using " << m_readChannels;
    if (m_readChannels != m_wavFmtChunk.numChannels)
      trace << " (converted from " << m_wavFmtChunk.numChannels << ')';
    trace << " channel";
    if (m_readChannels > 1)
      trace << 's';
    trace << '.'
          << PTrace::End;
  }
#endif

  return true;
}


bool PWAVFile::GenerateHeader()
{
  if (m_formatHandler == NULL) {
    PTRACE(1,"GenerateHeader: format handler is null!");
    return false;
  }

  // go to the beginning of the file
  if (!PFile::SetPosition(0)) {
    PTRACE(1,"GenerateHeader: Cannot Set Pos");
    return (false);
  }

  // write the WAV file header
  PWAV::RIFFChunkHeader riffChunk;
  memcpy(riffChunk.hdr.tag, WAVLabelRIFF, sizeof(WAVLabelRIFF));
  memcpy(riffChunk.tag,     WAVLabelWAVE, sizeof(WAVLabelWAVE));
  riffChunk.hdr.len = 0; // Is filled in by UpdateHeader

  if (!PFile::Write(&riffChunk, sizeof(riffChunk)))
    return false;

  // populate and write the WAV header with the default data
  memcpy(m_wavFmtChunk.hdr.tag,  WAVLabelFMT_, sizeof(WAVLabelFMT_));
  m_wavFmtChunk.hdr.len = sizeof(m_wavFmtChunk) - sizeof(m_wavFmtChunk.hdr);  // set default length assuming no extra bytes

  // allow the format handler to modify the header and extra bytes
  m_formatHandler->CreateHeader(m_wavFmtChunk, m_extendedHeader);

  // write the basic WAV header
  if (!PFile::Write(&m_wavFmtChunk, sizeof(m_wavFmtChunk)))
    return false;

  if (m_extendedHeader.GetSize() > 0 && !PFile::Write(m_extendedHeader.GetPointer(), m_extendedHeader.GetSize()))
    return false;

  // allow the format handler to write additional chunks
  if (!m_formatHandler->WriteExtraChunks(*this))
    return false;

  // Write the DATA chunk.
  PWAV::ChunkHeader dataChunk;
  memcpy(dataChunk.tag, WAVLabelDATA, sizeof(WAVLabelDATA));
  dataChunk.len = 0; // Filled in by UpdateHeader
  if (!PFile::Write(&dataChunk, sizeof(dataChunk)))
    return false;

  // get the length of the header
  m_headerLength = PFile::GetPosition();

  return true;
}


// Update the WAV header according to the file length
PBoolean PWAVFile::UpdateHeader()
{
  if (m_formatHandler == NULL){
    PTRACE(1,"GenerateHeader: format handler is null!");
    return false;
  }

  // Check file is still open
  if (CheckNotOpen())
    return false;

  // Find out the length of the audio data
  m_dataLength = PFile::GetLength() - m_headerLength;

  // rewrite the length in the RIFF chunk
  PInt32l riffChunkLen = (m_headerLength - 8) + m_dataLength; // size does not include first 8 bytes
  PFile::SetPosition(4);
  if (!PFile::Write(&riffChunkLen, sizeof(riffChunkLen)))
    return false;

  // rewrite the data length field in the data chunk
  PInt32l dataChunkLen = m_dataLength;
  PFile::SetPosition(m_headerLength - 4);
  if (!PFile::Write(&dataChunkLen, sizeof(dataChunkLen)))
    return false;

  m_formatHandler->UpdateHeader(m_wavFmtChunk, m_extendedHeader);

  PFile::SetPosition(12);
  if (!PFile::Write(&m_wavFmtChunk, sizeof(m_wavFmtChunk)))
    return false;

  if (!PFile::Write(m_extendedHeader.GetPointer(), m_extendedHeader.GetSize()))
    return false;

  return true;
}


//////////////////////////////////////////////////////////////////

PBoolean PWAVFileFormat::Read(PWAVFile & file, void * buf, PINDEX & len)
{ 
  if (!file.PFile::Read(buf, len))
    return false;

  len = file.GetLastReadCount();
  return true;
}

PBoolean PWAVFileFormat::Write(PWAVFile & file, const void * buf, PINDEX & len)
{ 
  if (!file.PFile::Write(buf, len))
    return false;

  len = file.GetLastWriteCount();
  return true;
}


//////////////////////////////////////////////////////////////////

class PWAVFileFormatPCM : public PWAVFileFormat
{
  public:
    unsigned GetFormat() const
    {
      return PWAVFile::fmt_PCM;
    }

    bool CanSetChannels(unsigned channels) const
    {
      return channels > 0 && channels < 7;
    }

    PString GetDescription() const
    {
      return "PCM";
    }

    PString GetFormatString() const
    {
      return "PCM-16";
    }

    void CreateHeader(PWAV::FMTChunk & wavFmtChunk, PBYTEArray & /*extendedHeader*/)
    {
      wavFmtChunk.hdr.len         = sizeof(wavFmtChunk) - sizeof(wavFmtChunk.hdr);  // no extended information
      wavFmtChunk.format          = PWAVFile::fmt_PCM;
      wavFmtChunk.numChannels     = 1;
      wavFmtChunk.sampleRate      = 8000;
      wavFmtChunk.bytesPerSample  = 2;
      wavFmtChunk.bitsPerSample   = 16;
      wavFmtChunk.bytesPerSec     = wavFmtChunk.sampleRate * wavFmtChunk.bytesPerSample;
    }

    void UpdateHeader(PWAV::FMTChunk & wavFmtChunk, PBYTEArray & /*extendedHeader*/)
    {
      wavFmtChunk.bytesPerSample  = 2 * wavFmtChunk.numChannels;
      wavFmtChunk.bytesPerSec     = wavFmtChunk.sampleRate * 2 * wavFmtChunk.numChannels;
    }

    PBoolean Read(PWAVFile & file, void * buf, PINDEX & len)
    {
      if (!PWAVFileFormat::Read(file, buf, len))
        return false;

      // WAV files are little-endian. So swap the bytes if this is
      // a big endian machine and we have 16 bit samples
      // Note: swab only works on even length buffers.
      if (file.GetSampleSize() == 16) {
        SWAB(buf, buf, len);
      }

      return true;
    }

    PBoolean Write(PWAVFile & file, const void * buf, PINDEX & len)
    {
      // WAV files are little-endian. So swap the bytes if this is
      // a big endian machine and we have 16 bit samples
      // Note: swab only works on even length buffers.
      if (file.GetSampleSize() == 16) {
        SWAB(buf, (void *)buf, len);
      }

      return PWAVFileFormat::Write(file, buf, len);
    }
};

PCREATE_WAVFILE_FORMAT_FACTORY(PWAVFileFormatPCM, PWAVFile::fmt_PCM, "PCM-16");


//////////////////////////////////////////////////////////////////

class PWAVFileFormatG7231 : public PWAVFileFormat
{
  public:
    PWAVFileFormatG7231(unsigned short _g7231)
      : g7231(_g7231) { }

    void CreateHeader(PWAV::FMTChunk & wavFmtChunk, PBYTEArray & extendedHeader);
    PBoolean WriteExtraChunks(PWAVFile & file);

    bool CanSetChannels(unsigned channels) const
    { return channels == 1; }

    PString GetFormatString() const
    { return "G.723.1"; }   // must match string in mediafmt.h

    void OnStart();
    PBoolean Read(PWAVFile & file, void * buf, PINDEX & len);
    PBoolean Write(PWAVFile & file, const void * buf, PINDEX & len);

  protected:
    unsigned short g7231;
    BYTE cacheBuffer[24];
    PINDEX cacheLen;
    PINDEX cachePos;
};

void PWAVFileFormatG7231::CreateHeader(PWAV::FMTChunk & wavFmtChunk, PBYTEArray & extendedHeader)
{
  wavFmtChunk.hdr.len         = sizeof(wavFmtChunk) - sizeof(wavFmtChunk.hdr) + sizeof(sizeof(PWAV::G7231ExtendedInfo));
  wavFmtChunk.format          = g7231;
  wavFmtChunk.numChannels     = 1;
  wavFmtChunk.sampleRate      = 8000;
  wavFmtChunk.bytesPerSample  = 24;
  wavFmtChunk.bitsPerSample   = 0;
  wavFmtChunk.bytesPerSec     = 800;

  extendedHeader.SetSize(sizeof(PWAV::G7231ExtendedInfo));
  PWAV::G7231ExtendedInfo * g7231Info = (PWAV::G7231ExtendedInfo *)extendedHeader.GetPointer(sizeof(PWAV::G7231ExtendedInfo));

  g7231Info->data1 = 1;
  g7231Info->data2 = 480;
}

PBoolean PWAVFileFormatG7231::WriteExtraChunks(PWAVFile & file)
{
  // write the fact chunk
  PWAV::G7231FACTChunk factChunk;
  memcpy(factChunk.hdr.tag, "FACT", 4);
  factChunk.hdr.len = sizeof(factChunk) - sizeof(factChunk.hdr);
  factChunk.data1 = 0;
  return file.PFile::Write(&factChunk, sizeof(factChunk));
}

static PINDEX G7231FrameSizes[4] = { 24, 20, 4, 1 };

void PWAVFileFormatG7231::OnStart()
{
  cacheLen = cachePos = 0;
}

PBoolean PWAVFileFormatG7231::Read(PWAVFile & file, void * origData, PINDEX & origLen)
{
  // Note that Microsoft && VivoActive G.2723.1 codec cannot do SID frames, so
  // we must parse the data and remove SID frames
  // also note that frames are always written as 24 byte frames, so each frame must be unpadded

  PINDEX bytesRead = 0;
  while (bytesRead < origLen) {

    // keep reading until we find a 20 or 24 byte frame
    while (cachePos == cacheLen) {
      if (!file.PFile::Read(cacheBuffer, 24))
        return false;

      // calculate actual length of frame
      PINDEX frameLen = G7231FrameSizes[cacheBuffer[0] & 3];
      if (frameLen == 20 || frameLen == 24) {
        cacheLen = frameLen;
        cachePos = 0;
      }
    }

    // copy data to requested buffer
    PINDEX copyLen = PMIN(origLen-bytesRead, cacheLen-cachePos);
    memcpy(origData, cacheBuffer+cachePos, copyLen);
    origData = copyLen + (char *)origData;
    cachePos += copyLen;
    bytesRead += copyLen;
  }

  origLen = bytesRead;

  return true;
}

PBoolean PWAVFileFormatG7231::Write(PWAVFile & file, const void * origData, PINDEX & len)
{
  // Note that Microsoft && VivoActive G.2723.1 codec cannot do SID frames, so
  // we must parse the data and remove SID frames
  // also note that frames are always written as 24 byte frames, so each frame must be padded
  PINDEX written = 0;

  BYTE frameBuffer[24];
  while (len > 0) {

    // calculate actual length of frame
    PINDEX frameLen = G7231FrameSizes[(*(char *)origData) & 3];
    if (len < frameLen)
      return false;

    // we can write 24 byte frame straight out, 
    // 20 byte frames need to be reblocked
    // we ignore any other frames

    const void * buf = NULL;
    switch (frameLen) {
      case 24:
        buf = origData;
        break;
      case 20:
        memcpy(frameBuffer, origData, 20);
        buf = frameBuffer;
        break;
      default:
        break;
    }

    if (buf != NULL && !file.PFile::Write(buf, 24))
      return false;
    else
      written += 24;

    origData = (char *)origData + frameLen;
    len -= frameLen;
  }

  len = written;

  return true;
}

class PWAVFileFormatG7231_vivo : public PWAVFileFormatG7231
{
  public:
    PWAVFileFormatG7231_vivo()
      : PWAVFileFormatG7231(PWAVFile::fmt_VivoG7231) { }
    virtual ~PWAVFileFormatG7231_vivo() {}
    unsigned GetFormat() const
    { return PWAVFile::fmt_VivoG7231; }
    PString GetDescription() const
    { return GetFormatString() & "Vivo"; }
};

PCREATE_WAVFILE_FORMAT_FACTORY(PWAVFileFormatG7231_vivo, PWAVFile::fmt_VivoG7231, "G.723.1");

class PWAVFileFormatG7231_ms : public PWAVFileFormatG7231
{
  public:
    PWAVFileFormatG7231_ms()
      : PWAVFileFormatG7231(PWAVFile::fmt_MSG7231) { }
    virtual ~PWAVFileFormatG7231_ms() {}
    unsigned GetFormat() const
    { return PWAVFile::fmt_MSG7231; }
    PString GetDescription() const
    { return GetFormatString() & "MS"; }
};

PFACTORY_CREATE(PWAVFileFormatByIDFactory, PWAVFileFormatG7231_ms, PWAVFile::fmt_MSG7231);

//////////////////////////////////////////////////////////////////

class PWAVFileConverterPCM : public PWAVFileConverter
{
  public:
    virtual ~PWAVFileConverterPCM() {}
    unsigned GetFormat    (const PWAVFile & file) const;
    off_t GetPosition     (const PWAVFile & file) const;
    PBoolean SetPosition      (PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin);
    unsigned GetSampleSize(const PWAVFile & file) const;
    off_t GetDataLength   (PWAVFile & file);
    PBoolean Read             (PWAVFile & file, void * buf, PINDEX len);
    PBoolean Write            (PWAVFile & file, const void * buf, PINDEX len);
};

unsigned PWAVFileConverterPCM::GetFormat(const PWAVFile &) const
{
  return PWAVFile::fmt_PCM;
}

off_t PWAVFileConverterPCM::GetPosition(const PWAVFile & file) const
{
  off_t pos = file.RawGetPosition();
  return pos * 2;
}

PBoolean PWAVFileConverterPCM::SetPosition(PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin)
{
  pos /= 2;
  return file.SetPosition(pos, origin);
}

unsigned PWAVFileConverterPCM::GetSampleSize(const PWAVFile &) const
{
  return 16;
}

off_t PWAVFileConverterPCM::GetDataLength(PWAVFile & file)
{
  return file.RawGetDataLength() * 2;
}

PBoolean PWAVFileConverterPCM::Read(PWAVFile & file, void * buf, PINDEX len)
{
  if (file.GetSampleSize() == 16)
    return file.RawRead(buf, len);

  if (file.GetSampleSize() != 8) {
    PTRACE(1, "Attempt to read autoconvert PCM data with unsupported number of bits per sample " << file.GetSampleSize());
    return false;
  }

  // read the PCM data with 8 bits per sample
  PINDEX samples = (len / 2);
  PBYTEArray pcm8;
  if (!file.RawRead(pcm8.GetPointer(samples), samples))
    return false;

  // convert to PCM-16
  PINDEX i;
  short * pcmPtr = (short *)buf;
  for (i = 0; i < samples; i++)
    *pcmPtr++ = pcm8[i] == 0 ? 0 : (unsigned short)((pcm8[i] << 8) - 0x8000);

  // fake the lastReadCount
  file.SetLastReadCount(len);

  return true;
}


PBoolean PWAVFileConverterPCM::Write(PWAVFile & file, const void * buf, PINDEX len)
{
  if (file.GetSampleSize() == 16)
    return file.PWAVFile::RawWrite(buf, len);

  PTRACE(1, "Attempt to write autoconvert PCM data with unsupported number of bits per sample " << file.GetSampleSize());
  return false;
}

PFACTORY_CREATE(PWAVFileConverterFactory, PWAVFileConverterPCM, PWAVFile::fmt_PCM);


#endif // P_WAVFILE

//////////////////////////////////////////////////////////////////
