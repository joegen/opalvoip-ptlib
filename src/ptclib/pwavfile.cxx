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
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
  format = fmt;
}


PWAVFile::PWAVFile(OpenMode mode, int opts, unsigned fmt)
  : PFile(mode, opts)
{
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
  format = fmt;
}


PWAVFile::PWAVFile(const PFilePath & name, OpenMode mode, int opts, unsigned fmt)
{
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
  format = fmt;

  Open(name, mode, opts);
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
  // Some wav files have extra data after the sound samples in a LIST chunk.
  // e.g. WAV files made in GoldWave have a copyright and a URL in this chunk.
  // We do not want to return this data by mistake.

  PINDEX readlen = len;
  if ((off_t)(PWAVFile::GetPosition() + len) > lenData) {
    PTRACE(1, "WAV\tRead: Detected non audio data after the sound samples");
    readlen = lenData-PWAVFile::GetPosition(); 
  }

  if (!PFile::Read(buf, readlen))
    return FALSE;

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
  // Note: swab only works on even length buffers.
  if (bitsPerSample == 16) {
    SWAB(buf, buf, PFile::GetLastReadCount());
  }

  return TRUE;
}


// Performs necessary byte-order swapping on for big-endian platforms.
BOOL PWAVFile::Write(const void * buf, PINDEX len)
{
  // Needs to update header on close.
  header_needs_updating = TRUE;

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
  // Note: swab only works on even length buffers.
  if (bitsPerSample == 16) {
    SWAB(buf, (void *)buf, len);
  }

  if (format != fmt_VivoG7231 && format != fmt_MSG7231)
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
  if (isValidWAV) {
    pos += lenHeader;
  }

  return (PFile::SetPosition(pos, origin));
}


off_t PWAVFile::GetPosition() const
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
    return format;
  else
    return 0;
}


unsigned PWAVFile::GetChannels() const
{
  if (isValidWAV)
    return numChannels;
  else
    return 0;
}


unsigned PWAVFile::GetSampleRate() const
{
  if (isValidWAV)
    return sampleRate;
  else
    return 0;
}


unsigned PWAVFile::GetSampleSize() const
{
  if (isValidWAV)
    return bitsPerSample;
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

  header_needs_updating = FALSE;
  format = fmt;
  return TRUE;
}


#define WAV_LABEL_RIFF "RIFF"
#define WAV_LABEL_WAVE "WAVE"
#define WAV_LABEL_FMT_ "fmt "
#define WAV_LABEL_FACT "fact"
#define WAV_LABEL_DATA "data"

// Because it is possible to process a file that we just created, or a
// corrupted WAV file, we check each read.
#define RETURN_ON_READ_FAILURE(readStat, len) \
  if (readStat != TRUE || PFile::GetLastReadCount() != len) return (FALSE);

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
  char    hdr_label_riff[4]; // ascii RIFF
  PInt32l hdr_len_after;     // length of data to follow
  char    hdr_label_wave[4]; // ascii WAVE
  int     size_riff_chunk = 12;

  // Use PFile::Read as we do not want to use our PWAVFile::Read code
  // as that does some byte swapping
  RETURN_ON_READ_FAILURE(PFile::Read(hdr_label_riff,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_len_after,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(hdr_label_wave,4), 4);

  // check if labels are correct
  if (strncmp(hdr_label_riff, WAV_LABEL_RIFF, strlen(WAV_LABEL_RIFF))) {
    PTRACE(1,"WAV\tProcessHeader: Not RIFF");
    return (FALSE);
  }

  if (strncmp(hdr_label_wave, WAV_LABEL_WAVE, strlen(WAV_LABEL_WAVE))) {
    PTRACE(1,"WAV\tProcessHeader: Not WAVE");
    return (FALSE);
  }

  // Read the FORMAT chunk.
  char    hdr_label_fmt[4];     // fmt_ in ascii
  PInt32l hdr_len_format;       // length of format chunk
  PInt16l hdr_format;           // Format 0x01 = PCM, 0x42 = Microsoft G.723.1
                                //        0x111 = VivoActive G.723.1
  PInt16l hdr_num_channels;     // Channels 0x01 = mono, 0x02 = stereo
  PInt32l hdr_samples_per_sec;  // Sample Rate in Hz
  PInt32l hdr_bytes_per_sec;    // Average bytes Per Second
  PInt16l hdr_bytes_per_sample; // Bytes Per Sample, eg 2
  PInt16l hdr_bits_per_sample;  // Bits Per Sample, eg 16

  RETURN_ON_READ_FAILURE(PFile::Read(hdr_label_fmt,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_len_format,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_format,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_num_channels,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_samples_per_sec,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_bytes_per_sec,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_bytes_per_sample,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_bits_per_sample,2), 2);

  int size_format_chunk = (int)hdr_len_format + 8;

  // The spec allows for extra bytes at the end of the FORMAT chunk
  // Use the len_format field from FORMAT to determine where to move to
  // in the file to find the start of the next chunk.
  if (!PFile::SetPosition(size_riff_chunk+size_format_chunk)) {
    PTRACE(1,"WAV\tProcessHeader: Cannot reset position");
    return (FALSE);
  }

  // check if labels are correct
  if (strncmp(hdr_label_fmt, WAV_LABEL_FMT_, strlen(WAV_LABEL_FMT_)) ) {
    PTRACE(1,"WAV\tProcessHeader: Not FMT");
    return (FALSE);
  }


  // Peek at the title of the next chunk and then restore the file pointer.
  int position = PFile::GetPosition();
  char    chunk_title[4];
  RETURN_ON_READ_FAILURE(PFile::Read(chunk_title,4), 4);
  PFile::SetPosition(position);


  // Read the FACT chunk (optional)
  int size_fact_chunk = 0;

  if (strncmp(chunk_title, WAV_LABEL_FACT, 4)==0) {
    char    hdr_label_fact[4];  // ascii fact
    PInt32l hdr_len_fact;       // length of fact

    RETURN_ON_READ_FAILURE(PFile::Read(hdr_label_fact,4), 4);
    RETURN_ON_READ_FAILURE(PFile::Read(&hdr_len_fact,4), 4);
    size_fact_chunk = (int)hdr_len_fact + 8;

    // The spec allows for extra bytes in the FACT chunk which contain
    // information relating to the format of the audio. This is mainly
    // used when the WAV file contains compressed audio instead of PCM audio.
    // Use the len_fact field from FACT to determine where to move to
    // in the file to find the start of the next chunk.
    if (!PFile::SetPosition(size_riff_chunk+size_format_chunk+size_fact_chunk)) {
      PTRACE(1,"WAV\tProcessHeader: Cannot reset position");
      return (FALSE);
    }
  }


  // Read the DATA chunk.
  char    hdr_label_data[4];  // ascii data
  PInt32l hdr_data_len;       // length of data
  int     size_data_chunk = 8;

  RETURN_ON_READ_FAILURE(PFile::Read(hdr_label_data,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&hdr_data_len,4), 4);

  if (strncmp(hdr_label_data, WAV_LABEL_DATA, strlen(WAV_LABEL_DATA))) {
    PTRACE(1,"WAV\tProcessHeader: Not DATA");
    return (FALSE);
  }

  // set the class variables for the Get methods.
  format        = hdr_format;
  numChannels   = hdr_num_channels;
  sampleRate    = hdr_samples_per_sec;
  bitsPerSample = hdr_bits_per_sample;
  lenHeader     = size_riff_chunk + size_format_chunk + size_fact_chunk + size_data_chunk;
  lenData       = hdr_data_len;

  return (TRUE);

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

  // go to the beginning of the file
  if (!PFile::SetPosition(0)) {
    PTRACE(1,"WAV\tGenerateHeader: Cannot Set Pos");
    return (FALSE);
  }


  // local variables
  int len_format = 0;
  int averageBytesPerSec = 0;
  int bytesPerSample = 0;


  // Set the file parameters.
  switch (format) {
    case fmt_PCM :
      lenHeader   = 44;
      len_format  = 16;   // size of the FORMAT chunk;
      numChannels = 1;
      sampleRate  = 8000;
      bytesPerSample = 2;
      bitsPerSample  = 16;
      averageBytesPerSec = sampleRate * bytesPerSample;
      break;

    case fmt_ALaw :
    case fmt_uLaw :
      lenHeader   = 44;
      len_format  = 16;   // size of the FORMAT chunk;
      numChannels = 1;
      sampleRate  = 8000;
      bytesPerSample = 1;
      bitsPerSample  = 8;
      averageBytesPerSec = sampleRate * bytesPerSample;
      break;

    case fmt_GSM :
      lenHeader   = 48;
      len_format  = 16;   // size of the FORMAT chunk;
      numChannels = 1;
      sampleRate  = 8000;
      bytesPerSample = 33;
      bitsPerSample  = 0;
      averageBytesPerSec = 33*50;
      break;

    case fmt_VivoG7231 :
    case fmt_MSG7231:
      lenHeader   = 60;
      len_format  = 20;    // size of the FORMAT chunk;
      numChannels = 1;
      sampleRate  = 8000;
      bytesPerSample = 24; // There are 24 bytes in a G.723.1 frame
      bitsPerSample  = 0;
      averageBytesPerSec = 800; // Windows Sound Recorder requires this value
      break;

    default :
      return FALSE;
  }

  // length of audio data is set to a large value if lenData does not
  // contain a valid (non negative) number. We must then write out real values
  // when we close the wav file.
  int audioData;
  if (lenData < 0) {
    audioData = LONG_MAX - lenHeader;
    header_needs_updating = TRUE;
  } else {
    audioData = lenData;
  }
  
  // Write the RIFF chunk.
  PInt32l hdr_len_after = audioData + (lenHeader - 8);

  // Use PFile::Write as we do not want to use our PWAVFile::Write code
  // as that does some byte swapping
  if (!PFile::Write(WAV_LABEL_RIFF,4) ||
      !PFile::Write(&hdr_len_after,4) ||
      !PFile::Write(WAV_LABEL_WAVE,4))
    return FALSE;


  // Write the FORMAT chunk
  PInt32l hdr_len_format       = len_format; //excludes label_fmt and len_format
  PInt16l hdr_format           = (WORD)format;
  PInt16l hdr_num_channels     = (WORD)numChannels;
  PInt32l hdr_sample_per_sec   = sampleRate;
  PInt16l hdr_bits_per_sample  = (WORD)bitsPerSample;
  PInt16l hdr_bytes_per_sample = (WORD)bytesPerSample;
  PInt32l hdr_average_bytes_per_sec = averageBytesPerSec;
  
  if (!PFile::Write(WAV_LABEL_FMT_,4) ||
      !PFile::Write(&hdr_len_format,4) ||
      !PFile::Write(&hdr_format,2) ||
      !PFile::Write(&hdr_num_channels,2) ||
      !PFile::Write(&hdr_sample_per_sec,4) ||
      !PFile::Write(&hdr_average_bytes_per_sec,4) ||
      !PFile::Write(&hdr_bytes_per_sample,2) ||
      !PFile::Write(&hdr_bits_per_sample,2))
    return FALSE;

  // The format chunk then has extra data for G.723.1 files
  if ((format == fmt_VivoG7231) || (format == fmt_MSG7231)) {
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

  // Write the DATA chunk.
  PInt32l hdr_data_len = audioData;

  if (!PFile::Write(WAV_LABEL_DATA,4) ||
      !PFile::Write(&hdr_data_len,4))
    return FALSE;

  isValidWAV = TRUE;

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
