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
 * $Log: pwavfile.cxx,v $
 * Revision 1.31.4.1  2004/07/07 07:07:42  csoutheren
 * Changed PWAVFile to use abstract factories (extensively)
 * Removed redundant blocking/unblocking when using G.723.1
 * More support for call transfer
 *
 * Revision 1.31  2003/07/29 11:27:16  csoutheren
 * Changed to use autoconf detected swab function
 *
 * Revision 1.30  2003/07/28 18:39:09  dsandras
 * Linux has a swab function. Patch from Alexander Larsson <alexl@redhat.com>.
 *
 * Revision 1.29  2003/02/20 23:32:00  robertj
 * More RTEMS support patches, thanks Sebastian Meyer.
 *
 * Revision 1.28  2002/12/20 08:43:42  robertj
 * Fixed incorrect header length for MS-GSM, thanks Martijn Roest & Kanchana
 *
 * Revision 1.27  2002/07/12 01:25:25  craigs
 * Repaired reintroduced problem with SID frames in WAV files
 *
 * Revision 1.26  2002/07/02 06:25:25  craigs
 * Added ability to create files in MS G.723.1 format
 *
 * Revision 1.25  2002/06/20 00:54:41  craigs
 * Added explicit class names to some functions to alloew overriding
 *
 * Revision 1.24  2002/06/12 07:28:16  craigs
 * Fixed problem with opening WAV files in read mode
 *
 * Revision 1.23  2002/05/23 05:04:11  robertj
 * Set error code if get invalid sized write for G.723.1 wav file.
 *
 * Revision 1.22  2002/05/23 03:59:55  robertj
 * Changed G.723.1 WAV file so every frame is 24 bytes long.
 *
 * Revision 1.21  2002/05/21 01:59:54  robertj
 * Removed the enum which made yet another set of magic numbers for audio
 *   formats, now uses the WAV file format numbers.
 * Fixed missing Open() function which does not have file name parameter.
 * Added ability to set the audio format after construction.
 * Added automatic expansion of G.723.1 SID frames into 24 zero bytes as
 *   those formats do not currently support 4 byte frames.
 * Fixed trace output to include "module" section.
 *
 * Revision 1.20  2002/02/06 00:52:23  robertj
 * Fixed GNU warning.
 *
 * Revision 1.19  2002/01/31 15:29:26  rogerh
 * Fix a problem with .wav files recorded in GoldWave.  The GoldWave copyright
 * string (embedded at the end of the wav file) was returned as audio data and
 * heared as noise. Javi <fjmchm@hotmail.com> reported the problem.
 *
 * Revision 1.18  2002/01/22 03:55:59  craigs
 * Added include of ptclib/pwavfile.cxx as this is now in PTCLib
 *
 * Revision 1.17  2002/01/13 21:01:55  rogerh
 * The class contructor is now used to specify the type of new WAV files
 * (eg PCM or G7231)
 *
 * Revision 1.16  2002/01/11 16:33:46  rogerh
 * Create a PWAVFile Open() function, which processes the WAV header
 *
 * Revision 1.15  2001/10/16 13:27:37  rogerh
 * Add support for writing G.723.1 WAV files.
 * MS Windows can play G.723.1 WAV Files in Media Player and Sound Recorder.
 * Sound Recorder can also convert them to normal PCM format WAV files.
 * Thanks go to M.Stoychev <M.Stoychev@cnsys.bg> for sample WAV files.
 *
 * Revision 1.14  2001/10/15 11:48:15  rogerh
 * Add GetFormat to return the format of a WAV file
 *
 * Revision 1.13  2001/10/15 07:27:38  rogerh
 * Add support for reading WAV fils containing G.723.1 audio data.
 *
 * Revision 1.12  2001/09/29 07:41:42  rogerh
 * Add fix from Patrick Koorevaar <pkoorevaar@hotmail.com>
 *
 * Revision 1.11  2001/08/15 12:52:20  rogerh
 * Fix typo
 *
 * Revision 1.10  2001/08/15 12:21:45  rogerh
 * Make Solaris use our swab() function instead of the C library version.
 * Submitted by Andre Schulze <as8@rncmm2.urz.tu-dresden.de>
 *
 * Revision 1.9  2001/07/23 02:57:42  robertj
 * Fixed swab definition for Linux alpha.
 *
 * Revision 1.8  2001/07/23 01:20:20  rogerh
 * Add updates from Shawn - ensure isvalidWAV is false for zero length files.
 * GetDataLength uses actual file size to support file updates as well as appends.
 * Add updates from Roger - Update Header() just writes to specific fields which
 * preserves any 'extra' data in an existing header between FORMAT and DATA chunks.
 *
 * Revision 1.7  2001/07/20 07:32:36  rogerh
 * Back out previous change. BSD systems already have swab in the C library.
 * Also use swab in Write()
 *
 * Revision 1.6  2001/07/20 07:09:12  rogerh
 * We need to byte swap on more then just Linux and BeOS.
 *
 * Revision 1.5  2001/07/20 04:14:47  robertj
 * Fixed swab implementation on Linux alpha
 *
 * Revision 1.4  2001/07/20 03:30:59  robertj
 * Minor cosmetic changes to new PWAVFile class.
 *
 * Revision 1.3  2001/07/19 09:57:24  rogerh
 * Use correct filename
 *
 * Revision 1.2  2001/07/19 09:53:29  rogerh
 * Add the PWAVFile class to read and write .wav files
 * The PWAVFile class was written by Roger Hardiman <roger@freebsd.org>
 * and Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 */

#ifdef __GNUC__
#pragma implementation "pwavfile.h"
#endif

#include <ptlib.h>
#include <ptclib/pwavfile.h>

const char WAVLabelRIFF[4] = { 'R', 'I', 'F', 'F' };
const char WAVLabelWAVE[4] = { 'W', 'A', 'V', 'E' };
const char WAVLabelFMT_[4] = { 'f', 'm', 't', ' ' };
const char WAVLabelFACT[4] = { 'f', 'a', 'c', 't' };
const char WAVLabelDATA[4] = { 'd', 'a', 't', 'a' };

PINSTANTIATE_FACTORY(PWAVFileFormat, unsigned)
PINSTANTIATE_FACTORY(PWAVFileConverter, unsigned)

inline BOOL ReadAndCheck(PFile & file, void * buf, PINDEX len)
{
  return file.PFile::Read(buf, len) && (file.PFile::GetLastReadCount() == len);
}

inline BOOL WriteAndCheck(PFile & file, void * buf, PINDEX len)
{
  return file.Write(buf, len) && (file.GetLastWriteCount() == len);
}

#if PBYTE_ORDER==PBIG_ENDIAN
#  if defined(USE_SYSTEM_SWAB)
#    define	SWAB(a,b,c)	::swab(a,b,c)
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
#  define	SWAB(a,b,c)	{}
#endif


///////////////////////////////////////////////////////////////////////////////
// PWAVFile

PWAVFile::PWAVFile(unsigned fmt)
  : PFile()
{
  SelectFormat(fmt);
}

PWAVFile * PWAVFile::PWAVFile_format(const PString & format)
{
  PWAVFile * file = new PWAVFile;
  file->SelectFormat(format);
  return file;
}

PWAVFile::PWAVFile(OpenMode mode, int opts, unsigned fmt)
  : PFile(mode, opts)
{
  SelectFormat(fmt);
}

PWAVFile * PWAVFile::PWAVFile_format(
  const PString & format,
  PFile::OpenMode mode,
  int opts
)
{
  PWAVFile * file = new PWAVFile(mode, opts);
  file->SelectFormat(format);
  return file;
}

PWAVFile::PWAVFile(const PFilePath & name, OpenMode mode, int opts, unsigned fmt)
{
  SelectFormat(fmt);
  Open(name, mode, opts);
}

PWAVFile * PWAVFile::PWAVFile_format(
      const PString & format,  
      const PFilePath & name,  
      OpenMode mode,
      int opts 
)
{
  PWAVFile * file = new PWAVFile(mode, opts);
  file->SelectFormat(format);
  file->Open(name);
  return file;
}

void PWAVFile::SelectFormat(unsigned fmt)
{
  isValidWAV            = FALSE;
  header_needs_updating = FALSE;

  autoConvert           = FALSE;
  autoConverter         = NULL;
  formatHandler         = PWAVFileFormatByIDFactory::CreateInstance(fmt);
}

void PWAVFile::SelectFormat(const PString & format)
{
  // see if the format is known
  isValidWAV            = FALSE;
  header_needs_updating = FALSE;
  autoConvert           = FALSE;
  autoConverter         = NULL;

  formatHandler = PWAVFileFormatByFormatFactory::CreateInstance(format);
}

BOOL PWAVFile::Open(OpenMode  mode, int opts)
{
  if (!(PFile::Open(mode, opts)))
    return FALSE;

  isValidWAV = FALSE;

  // Try and process the WAV file header information.
  // Either ProcessHeader() or GenerateHeader() must be called.

  if (PFile::GetLength() > 0) {

    // try and process the WAV file header information
    if (mode == ReadOnly || mode == ReadWrite) {
      isValidWAV = ProcessHeader();
    }
    if (mode == WriteOnly) {
      lenData = -1;
      GenerateHeader();
    }
  }
  else {

    // generate header
    if (mode == ReadWrite || mode == WriteOnly) {
      lenData = -1;
      GenerateHeader();
    }
    if (mode == ReadOnly) {
      isValidWAV = FALSE; // ReadOnly on a zero length file
    }
  }

  return TRUE;
}


BOOL PWAVFile::Open(const PFilePath & name, OpenMode  mode, int opts)
{
  Close();
  SetFilePath(name);
  return Open(mode, opts);
}


BOOL PWAVFile::Close()
{
  if (header_needs_updating)
    UpdateHeader();

  return (PFile::Close());
}


// Performs necessary byte-order swapping on for big-endian platforms.
BOOL PWAVFile::Read(void * buf, PINDEX len)
{
  if (autoConverter != NULL)
    return autoConverter->Read(*this, buf, len);

  return RawRead(buf, len);
}

BOOL PWAVFile::RawRead(void * buf, PINDEX len)
{
  // Some wav files have extra data after the sound samples in a LIST chunk.
  // e.g. WAV files made in GoldWave have a copyright and a URL in this chunk.
  // We do not want to return this data by mistake.
  PINDEX readlen = len;
  if ((off_t)(PFile::GetPosition() + len - lenHeader) > lenData) {
    PTRACE(1, "WAV\tRead: Detected non audio data after the sound samples");
    readlen = lenData-PWAVFile::GetPosition(); 
  }

  if (!PFile::Read(buf, readlen))
    return FALSE;

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
  // Note: swab only works on even length buffers.
  if (wavHeader.bitsPerSample == 16) {
    SWAB(buf, buf, PFile::GetLastReadCount());
  }

  return TRUE;
}


// Performs necessary byte-order swapping on for big-endian platforms.
BOOL PWAVFile::Write(const void * buf, PINDEX len)
{
  // Needs to update header on close.
  header_needs_updating = TRUE;

  if (autoConverter != NULL)
    return autoConverter->Write(*this, buf, len);

  return RawWrite(buf, len);
}

BOOL PWAVFile::RawWrite(const void * buf, PINDEX len)
{
  // Needs to update header on close.
  header_needs_updating = TRUE;

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
  // Note: swab only works on even length buffers.
  if (wavHeader.bitsPerSample == 16) {
    SWAB(buf, (void *)buf, len);
  }

  if (wavHeader.format != fmt_VivoG7231 && wavHeader.format != fmt_MSG7231)
    return PFile::Write(buf, len);

  // The Microsoft && VivoActive G.2723.1 codec cannot do SID frames
  const BYTE * frame = (const BYTE *)buf;
  while (len > 0) {
    static PINDEX FrameSize[4] = { 24, 20, 4, 1 };
    PINDEX frameSize = FrameSize[*frame&3];

    if (len < frameSize)
      return SetErrorValues(Miscellaneous, EINVAL, LastWriteError);

    BYTE writebuf[24];
    memset(writebuf, 0, sizeof(writebuf));
    if (frameSize >= 20)
      memcpy(writebuf, frame, frameSize);

    if (!PFile::Write(writebuf, sizeof(writebuf)))
      return FALSE;

    frame += frameSize;
    len -= frameSize;
  }

  return TRUE;
}


// Both SetPosition() and GetPosition() are offset by lenHeader.
BOOL PWAVFile::SetPosition(off_t pos, FilePositionOrigin origin)
{
  if (autoConverter != NULL)
    return autoConverter->SetPosition(*this, pos, origin);

  return RawSetPosition(pos, origin);
}

BOOL PWAVFile::RawSetPosition(off_t pos, FilePositionOrigin origin)
{
  if (isValidWAV) {
    pos += lenHeader;
  }

  return PFile::SetPosition(pos, origin);
}


off_t PWAVFile::GetPosition() const
{
  if (autoConverter != NULL)
    return autoConverter->GetPosition(*this);

  return RawGetPosition();
}

off_t PWAVFile::RawGetPosition() const
{
  off_t pos = PFile::GetPosition();

  if (isValidWAV) {
    if (pos >= lenHeader) {
      pos -= lenHeader;
    }
    else {
      pos = 0;
    }
  }

  return (pos);
}


unsigned PWAVFile::GetFormat() const
{
  if (isValidWAV)
    return wavHeader.format;
  else
    return 0;
}

PString PWAVFile::GetFormatAsString() const
{
  if (isValidWAV && formatHandler != NULL)
    return formatHandler->GetFormat();
  else
    return PString::Empty();
}

unsigned PWAVFile::GetChannels() const
{
  if (isValidWAV)
    return wavHeader.numChannels;
  else
    return 0;
}


unsigned PWAVFile::GetSampleRate() const
{
  if (isValidWAV)
    return wavHeader.sampleRate;
  else
    return 0;
}


unsigned PWAVFile::GetSampleSize() const
{
  if (isValidWAV)
    return wavHeader.bitsPerSample;
  else
    return 0;
}


off_t PWAVFile::GetHeaderLength() const
{
  if (isValidWAV)
    return lenHeader;
  else
    return 0;
}


off_t PWAVFile::GetDataLength()
{
  if (autoConverter != NULL)
    return autoConverter->GetDataLength(*this);

  return RawGetDataLength();
}

off_t PWAVFile::RawGetDataLength()
{
  if (isValidWAV) {
    // Updates data length before returns.
    lenData = PFile::GetLength() - lenHeader;
    return lenData;
  }
  else
    return 0;
}


BOOL PWAVFile::SetFormat(unsigned fmt)
{
  if (IsOpen() || isValidWAV)
    return FALSE;

  SelectFormat(fmt);

  return TRUE;
}

BOOL PWAVFile::SetFormat(const PString & format)
{
  if (IsOpen() || isValidWAV)
    return FALSE;

  SelectFormat(format);

  return TRUE;
}

BOOL PWAVFile::ProcessHeader() {

  // Process the header information
  // This comes in 3 or 4 chunks, either RIFF, FORMAT and DATA
  // or RIFF, FORMAT, FACT and DATA.

  if (!IsOpen()) {
    PTRACE(1,"WAV\tProcessHeader: Not Open");
    return (FALSE);
  }

  // go to the beginning of the file
  if (!PFile::SetPosition(0)) {
    PTRACE(1,"WAV\tProcessHeader: Cannot Set Pos");
    return (FALSE);
  }

  // Read the RIFF chunk.
  struct PWAV::WAVHeader riffChunk;
  if (!ReadAndCheck(*this, &riffChunk, sizeof(riffChunk)))
    return FALSE;

  // check if labels are correct
  if (strncmp(riffChunk.hdr.tag, WAVLabelRIFF, sizeof(WAVLabelRIFF))) {
    PTRACE(1,"WAV\tProcessHeader: Not RIFF");
    return (FALSE);
  }

  if (strncmp(riffChunk.tag, WAVLabelWAVE, sizeof(WAVLabelWAVE))) {
    PTRACE(1,"WAV\tProcessHeader: Not WAVE");
    return (FALSE);
  }

  // Read the FORMAT chunk.
  if (!ReadAndCheck(*this, &wavHeader, sizeof(wavHeader)))
    return FALSE;

  // check if labels are correct
  if (strncmp(wavHeader.hdr.tag, WAVLabelFMT_, sizeof(WAVLabelFMT_)) ) {
    PTRACE(1,"WAV\tProcessHeader: Not FMT");
    return (FALSE);
  }

  // get the format handler
  formatHandler = PWAVFileFormatByIDFactory::CreateInstance(wavHeader.format);
  if (formatHandler == NULL) {
    PTRACE(1, "PWAVFILE\tUnknown WAV format " << wavHeader.format);
    return FALSE;
  }

  unsigned formatChunkLen = sizeof(wavHeader.hdr) + wavHeader.hdr.len;

  // move to the end of the FORMAT chunk
  if (!PFile::SetPosition(sizeof(riffChunk) + formatChunkLen)) {
    PTRACE(1,"WAV\tProcessHeader: Cannot reset position");
    return (FALSE);
  }

  // Peek at the title of the next chunk and then restore the file pointer.
  int position = PFile::GetPosition();
  char chunkTitle[4];
  if (!ReadAndCheck(*this, chunkTitle, sizeof(chunkTitle)))
    return FALSE;
  PFile::SetPosition(position);

  // Read the FACT chunk (optional)
  int factChunkLen = 0;

  if (strncmp(chunkTitle, WAVLabelFACT, sizeof(WAVLabelFACT)) == 0) {

    PWAV::RIFFHeader factChunk;
    if (!ReadAndCheck(*this, &factChunk, sizeof(factChunk)))
      return FALSE;

    factChunkLen = sizeof(factChunk) + factChunk.len;

    // The spec allows for extra bytes in the FACT chunk which contain
    // information relating to the format of the audio. This is mainly
    // used when the WAV file contains compressed audio instead of PCM audio.
    // Use the len_fact field from FACT to determine where to move to
    // in the file to find the start of the next chunk.
    if (!PFile::SetPosition(sizeof(riffChunk) + formatChunkLen + factChunkLen)) {
      PTRACE(1,"WAV\tProcessHeader: Cannot reset position");
      return (FALSE);
    }
  }

  // get the size of the data chunk
  PWAV::RIFFHeader dataChunk;
  if (!ReadAndCheck(*this, &dataChunk, sizeof(dataChunk)))
    return FALSE;

  if (strncmp(dataChunk.tag, WAVLabelDATA, sizeof(WAVLabelDATA))) {
    PTRACE(1,"WAV\tProcessHeader: Not DATA");
    return (FALSE);
  }

  // calculate the size of header and data for accessing the WAV data.
  lenHeader  = sizeof(riffChunk) + formatChunkLen + factChunkLen + sizeof(dataChunk);
  lenData    = dataChunk.len;

  // get ptr to data handler if in autoconvert mode
  if (autoConvert) {
    autoConverter = PWAVFileConverterFactory::CreateInstance(wavHeader.format);
    if (autoConverter == NULL) {
      PTRACE(1, "PWAVFile\tNo format converter for type " << wavHeader.format);
      return FALSE;
    }
  }

  return TRUE;
}


// Generates the wave file header.
// Two types of header are supported.
// a) PCM data, set to 8000Hz, mono, 16-bit samples
// b) G.723.1 data
// When this function is called with lenData < 0, it will write the header
// as if the lenData is LONG_MAX minus header length.
// Note: If it returns FALSE, the file may be left in inconsistent state.

BOOL PWAVFile::GenerateHeader()
{
  if (!IsOpen()) {
    PTRACE(1, "WAV\tGenerateHeader: Not Open");
    return (FALSE);
  }

  // get the format handler
  formatHandler = PWAVFileFormatByIDFactory::CreateInstance(wavHeader.format);
  if (formatHandler == NULL) {
    PTRACE(1, "PWAVFILE\tUnknown WAV format " << wavHeader.format);
    return FALSE;
  }

  // go to the beginning of the file
  if (!PFile::SetPosition(0)) {
    PTRACE(1,"WAV\tGenerateHeader: Cannot Set Pos");
    return (FALSE);
  }

  // length of audio data is set to a large value if lenData does not
  // contain a valid (non negative) number. We must then write out real values
  // when we close the wav file.
  int audioDataLen;
  if (lenData < 0) {
    audioDataLen = LONG_MAX - wavHeader.hdr.len;
    header_needs_updating = TRUE;
  } else {
    audioDataLen = lenData;
  }

  // write the RIFF header
  PWAV::RIFFHeader riffHeader;
  memcpy(riffHeader.tag, WAVLabelRIFF, sizeof(WAVLabelRIFF));
  riffHeader.len = lenHeader + audioDataLen - 8;
  memcpy(riffHeader.tag, WAVLabelWAVE, sizeof(WAVLabelWAVE));
  if (!WriteAndCheck(*this, &riffHeader, sizeof(riffHeader)))
    return FALSE;

  // populate and write the WAV header with the default data
  formatHandler->CreateHeader(wavHeader, extendedHeader);

  // write the basic WAV header
  if (
      !WriteAndCheck(*this, &wavHeader, sizeof(wavHeader)) ||
      !WriteAndCheck(*this, extendedHeader.GetPointer(), extendedHeader.GetSize())
     )
    return FALSE;

  // write extended headers (if required)
  if (!formatHandler->WriteExtraHeaders(*this, wavHeader))
    return FALSE;

  // Write the DATA chunk.
  PWAV::RIFFHeader dataChunk;
  memcpy(dataChunk.tag, WAVLabelDATA, sizeof(WAVLabelDATA));
  dataChunk.len = audioDataLen;
  if (!WriteAndCheck(*this, &dataChunk, sizeof(dataChunk)))
    return FALSE;

  isValidWAV = TRUE;

  // get pointer to auto converter 
  autoConverter = PWAVFileConverterFactory::CreateInstance(wavHeader.format);
  if (autoConverter == NULL) {
    PTRACE(1, "PWAVFile\tNo format converter for type " << wavHeader.format);
    return FALSE;
  }

  return (TRUE);
}

// Update the WAV header according to the file length
BOOL PWAVFile::UpdateHeader()
{
  // Check file is still open
  if (!IsOpen()) {
    PTRACE(1,"WAV\tUpdateHeader: Not Open");
    return (FALSE);
  }

  // Check there is already a valid header
  if (!isValidWAV) {
    PTRACE(1,"WAV\tUpdateHeader: File not valid");
    return (FALSE);
  }

  // Find out the length of the audio data
  lenData = PFile::GetLength() - lenHeader;

  // Write out the 'len_after' field
  PFile::SetPosition(4);
  PInt32l hdr_len_after = (lenHeader - 8) + lenData; // size does not include
						     // first 8 bytes
  if (!PFile::Write(&hdr_len_after,4))
    return (FALSE);

  // Write out the 'data_len' field
  PFile::SetPosition(lenHeader - 4);
  PInt32l hdr_data_len = lenData;
  if (!PFile::Write(&hdr_data_len,4))
    return (FALSE);

  header_needs_updating = FALSE;

  return (TRUE);
}


//////////////////////////////////////////////////////////////////

class PWAVFileFormatPCM : public PWAVFileFormat
{
  public:
    void CreateHeader(PWAV::WAVFormatHeader & header, PBYTEArray & extendedHeader);
    PString GetDescription() const;
    PString GetFormat() const;
};

PWAVFileFormatByIDFactory::Worker<PWAVFileFormatPCM> pcmIDWAVFormat(PWAVFile::fmt_PCM);
PWAVFileFormatByFormatFactory::Worker<PWAVFileFormatPCM> pcmFormatWAVFormat("PCM-16");

PString PWAVFileFormatPCM::GetDescription() const
{
  return "PCM";
}

PString PWAVFileFormatPCM::GetFormat() const
{
  return "PCM-16";
}

void PWAVFileFormatPCM::CreateHeader(PWAV::WAVFormatHeader & header, 
                                     PBYTEArray & /*extendedHeader*/)
{
  header.hdr.len         = 44;
  header.format          = PWAVFile::fmt_PCM;
  header.numChannels     = 1;
  header.sampleRate      = 8000;
  header.bytesPerSample  = 2;
  header.bitsPerSample   = 16;
  header.bytesPerSec     = header.sampleRate * header.bytesPerSample;
}

//////////////////////////////////////////////////////////////////

class PWAVFileFormatG7231 : public PWAVFileFormat
{
  public:
    PWAVFileFormatG7231(unsigned short _g7231)
      : g7231(_g7231) { }

    void CreateHeader(PWAV::WAVFormatHeader & header, PBYTEArray & extendedHeader);
    BOOL WriteExtraHeaders(PFile & file, const PWAV::WAVFormatHeader & header);
    PString GetFormat() const
    { return "G.723.1"; }   // must match string in mediafmt.h

  protected:
    unsigned short g7231;
};

void PWAVFileFormatG7231::CreateHeader(PWAV::WAVFormatHeader & header, PBYTEArray & extendedHeader)
{
  header.hdr.len         = 60;
  header.format          = g7231;
  header.numChannels     = 1;
  header.sampleRate      = 8000;
  header.bytesPerSample  = 24;
  header.bitsPerSample   = 0;
  header.bytesPerSec     = 800;

  extendedHeader.SetSize(4);
}

BOOL PWAVFileFormatG7231::WriteExtraHeaders(PFile & /*file*/, const PWAV::WAVFormatHeader & /*header*/)
{
/*
  // The format chunk then has extra data for G.723.1 files
  if ((wavHeader.hdr.format == fmt_VivoG7231) || (format == fmt_MSG7231)) {
    PInt16l hdr_format_data1 = 1;
    PInt16l hdr_format_data2 = 480;
    if (!PFile::Write(&hdr_format_data1,2) ||
        !PFile::Write(&hdr_format_data2,2))
      return FALSE;

    // Write the FACT chunk. (only for G.723.1 files)
    PInt32l hdr_fact_len  = 4; // length is 4, exclude label_fact and fact_len
    PInt32l hdr_fact_data = 0; // fact chunk data. Should be number of samples.
    if (!PFile::Write(WAV_LABEL_FACT,4) ||
        !PFile::Write(&hdr_fact_len,4) ||
        !PFile::Write(&hdr_fact_data,4))
      return FALSE;
  }
*/
  return TRUE;
}


class PWAVFileFormatG7231_vivo : public PWAVFileFormatG7231
{
  public:
    PWAVFileFormatG7231_vivo()
      : PWAVFileFormatG7231(PWAVFile::fmt_VivoG7231) { }
    PString GetDescription() const
    { return GetFormat() & "Vivo"; }
};

PWAVFileFormatByIDFactory::Worker<PWAVFileFormatG7231_vivo> g7231VivoWAVFormat(PWAVFile::fmt_VivoG7231);
PWAVFileFormatByFormatFactory::Worker<PWAVFileFormatG7231_vivo> g7231FormatWAVFormat("G.723.1");

class PWAVFileFormatG7231_ms : public PWAVFileFormatG7231
{
  public:
    PWAVFileFormatG7231_ms()
      : PWAVFileFormatG7231(PWAVFile::fmt_MSG7231) { }
    PString GetDescription() const
    { return GetFormat() & "MS"; }
};

PWAVFileFormatByIDFactory::Worker<PWAVFileFormatG7231_ms> g7231MSWAVFormat(PWAVFile::fmt_MSG7231);

//////////////////////////////////////////////////////////////////

class PWAVFileConverterPCM : public PWAVFileConverter
{
  public:
    unsigned GetFormat    (const PWAVFile & file) const;
    off_t GetPosition     (const PWAVFile & file) const;
    BOOL SetPosition      (PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin);
    unsigned GetSampleSize(const PWAVFile & file) const;
    off_t GetDataLength   (PWAVFile & file);
    BOOL Read             (PWAVFile & file, void * buf, PINDEX len);
    BOOL Write            (PWAVFile & file, const void * buf, PINDEX len);
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

BOOL PWAVFileConverterPCM::SetPosition(PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin)
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

BOOL PWAVFileConverterPCM::Read(PWAVFile & file, void * buf, PINDEX len)
{
  if (file.wavHeader.bitsPerSample == 16)
    return file.RawRead(buf, len);

  if (file.wavHeader.bitsPerSample != 8) {
    PTRACE(1, "PWAVFile\tAttempt to read autoconvert PCM data with unsupported number of bits per sample " << file.wavHeader.bitsPerSample);
    return FALSE;
  }

  // read the PCM data with 8 bits per sample
  PINDEX samples = (len / 2);
  PBYTEArray pcm8;
  if (!file.PFile::Read(pcm8.GetPointer(samples), samples))
    return FALSE;

  // convert to PCM-16
  PINDEX i;
  short * pcmPtr = (short *)buf;
  for (i = 0; i < samples; i++)
    *pcmPtr++ = (unsigned short)((pcm8[i] << 8) - 0x8000);

  // fake the lastReadCount
  file.SetLastReadCount(len);

  return TRUE;
}


BOOL PWAVFileConverterPCM::Write(PWAVFile & file, const void * buf, PINDEX len)
{
  if (file.wavHeader.bitsPerSample == 16)
    return file.RawWrite(buf, len);

  PTRACE(1, "PWAVFile\tAttempt to write autoconvert PCM data with unsupported number of bits per sample " << file.wavHeader.bitsPerSample);
  return FALSE;
}

PWAVFileConverterFactory::Worker<PWAVFileConverterPCM> pcmConverter(PWAVFile::fmt_PCM, true);

//////////////////////////////////////////////////////////////////
