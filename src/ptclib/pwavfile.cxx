/*
 * wavfile.cxx
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
 * Revision 1.2  2001/07/19 09:53:29  rogerh
 * Add the PWAVFile class to read and write .wav files
 *
 * Revision 1.1  2001/07/19 09:36:22  rogerh
 * The PWAVFile class can read and write .wav audio files. Used in OpenAM.
 * The PWAVFile class was written by Roger Hardiman <roger@freebsd.org>
 * and Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 */

#ifdef __GNUC__
#pragma implementation "wavfile.h"
#endif

#include <ptlib.h>


///////////////////////////////////////////////////////////////////////////////
// PWAVFile

PWAVFile::PWAVFile()
{
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
}


PWAVFile::PWAVFile(OpenMode mode, int opts)
  : PFile(mode, opts)
{
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
}


PWAVFile::PWAVFile(const PFilePath & name, OpenMode mode, int opts)
  : PFile(name, mode, opts)
{
  isValidWAV = FALSE;
  header_needs_updating = FALSE;
  if (!IsOpen()) return; 

  // try and process the WAV file header information
  if (PFile::GetLength() > 0) {
    if (mode == ReadOnly || mode == ReadWrite) {
      isValidWAV = ProcessHeader();
    }
    if (mode == WriteOnly) {
      lenData = -1;
      GenerateHeader();
    }
  }
  else {
    if (mode == ReadWrite || mode == WriteOnly) {
      lenData = -1;
      GenerateHeader();
    }
  }

}

// Performs necessary byte-order swapping on for big-endian platforms.
BOOL PWAVFile::Read(void * buf, PINDEX len)
{
  BOOL rval = PFile::Read(buf, len);

  if (rval == FALSE) {
    return (rval);
  }

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
#if PBYTE_ORDER==PBIG_ENDIAN
  if (bitsPerSample == 16) {
    PINDEX i;
    unsigned char tmp, *mybuf;
    PINDEX num = PFile::GetLastReadCount();
    mybuf = (unsigned char *)buf;
    for (i = 0; (i+1) < num; i+=2) {
      tmp = mybuf[i];
      mybuf[i] = mybuf[i+1];
      mybuf[i+1] = tmp;
    }
  }
#endif

  return (rval);
}

// Performs necessary byte-order swapping on for big-endian platforms.
BOOL PWAVFile::Write(const void * buf, PINDEX len)
{
  BOOL rval;

  // WAV files are little-endian. So swap the bytes if this is
  // a big endian machine and we have 16 bit samples
#if PBYTE_ORDER==PBIG_ENDIAN
  if (bitsPerSample == 16) {
    PINDEX i;
    unsigned char tmp, *mybuf;
    mybuf = (unsigned char *)buf;
    for (i = 0; (i+1) < len; i+=2) {
      tmp = mybuf[i];
      mybuf[i] = mybuf[i+1];
      mybuf[i+1] = tmp;
    }
  }
  // Note: The trailing byte of an odd-length buffer won't get swap correctly. 
#endif

  rval = PFile::Write(buf, len);
  // update lenData is the write is succeed
  if (rval == TRUE) {
    if (lenData < 0) lenData = 0;
    lenData += PFile::GetLastWriteCount();
    header_needs_updating = TRUE;
  }

  return (rval);
}

BOOL PWAVFile::Close()
{
  if (header_needs_updating)
    UpdateHeader();
  return (PFile::Close());
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


#define WAV_LABEL_RIFF "RIFF"
#define WAV_LABEL_WAVE "WAVE"
#define WAV_LABEL_FMT_ "fmt "
#define WAV_LABEL_DATA "data"

// Because it is possible to process a file that we just created, or a
// corrupted WAV file, we check each read.
#define RETURN_ON_READ_FAILURE(readStat, len) \
  if (readStat != TRUE || GetLastReadCount() != len) return (FALSE);

BOOL PWAVFile::ProcessHeader() {

  // Process the header information
  // This comes in 3 chunks, RIFF, FORMAT and DATA.

  if (IsOpen() == FALSE) {
    PTRACE(1,"Not Open");
    return (FALSE);
  }

  // go to the beginning of the file
  if (PFile::SetPosition(0) == FALSE) {
    PTRACE(1,"Cannot Set Pos");
    return (FALSE);
  }

  // Read the RIFF chunk.
  char    label_riff[4]; // ascii RIFF
  PInt32l len_after;     // length of data to follow
  char    label_wave[4]; // ascii WAVE
  int     size_riff_chunk = 12;

  // Use PFile::Read as we do not want to use our PWAVFile::Read code
  // as that does some byte swapping
  RETURN_ON_READ_FAILURE(PFile::Read(label_riff,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&len_after,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(label_wave,4), 4);

  // check if labels are correct
  if (strncmp(label_riff, WAV_LABEL_RIFF, strlen(WAV_LABEL_RIFF))) {
    PTRACE(1,"Not RIFF");
    return (FALSE);
  }

  if (strncmp(label_wave, WAV_LABEL_WAVE, strlen(WAV_LABEL_WAVE))) {
    PTRACE(1,"Not WAVE");
    return (FALSE);
  }

  // Read the FORMAT chunk.
  char    label_fmt[4];     // fmt_ in ascii
  PInt32l len_format;       // length of format chunk
  PInt16l format;           // Format 0x01 = PCM
  PInt16l num_channels;     // Channels 0x01 = mono, 0x02 = stereo
  PInt32l samples_per_sec;  // Sample Rate in Hz
  PInt32l bytes_per_sec;    // Bytes Per Second
  PInt16l bytes_per_sample; // Bytes Per Sample, 1,2 or 4
  PInt16l bits_per_sample;  // Bits Per Sample, 1,2 or 4

  RETURN_ON_READ_FAILURE(PFile::Read(label_fmt,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&len_format,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&format,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&num_channels,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&samples_per_sec,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&bytes_per_sec,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&bytes_per_sample,2), 2);
  RETURN_ON_READ_FAILURE(PFile::Read(&bits_per_sample,2), 2);

  int size_format_chunk = len_format + 8;

  // The spec allows for extra bytes between the FORMAT and DATA chunks.
  // Use the len_format field from FORMAT to determine where to move to
  // in the file to find the DATA chunk.
  if (PFile::SetPosition(size_riff_chunk+size_format_chunk) == FALSE) {
    PTRACE(1,"Cannot reset position");
    return (FALSE);
  }

  // check if labels are correct
  if (strncmp(label_fmt, WAV_LABEL_FMT_, strlen(WAV_LABEL_FMT_)) ) {
    PTRACE(1,"Not FMT");
    return (FALSE);
  }

  // Read the DATA chunk.
  char    label_data[4];  // ascii data
  PInt32l data_len;       // length of data
  int     size_data_chunk = 8;

  RETURN_ON_READ_FAILURE(PFile::Read(label_data,4), 4);
  RETURN_ON_READ_FAILURE(PFile::Read(&data_len,4), 4);

  if (strncmp(label_data, WAV_LABEL_DATA, strlen(WAV_LABEL_DATA))) {
    PTRACE(1,"Not DATA");
    return (FALSE);
  }

  // set the class variables
  numChannels   = num_channels;
  sampleRate    = samples_per_sec;
  bitsPerSample = bits_per_sample;
  lenHeader     = size_riff_chunk+size_format_chunk+size_data_chunk;
  lenData       = data_len;

  return (TRUE);

}

#define RETURN_ON_WRITE_FAILURE(writeStat, len) \
  if (writeStat != TRUE || GetLastWriteCount() != len) return (FALSE);

// Generates a 8000Hz, mono, 16-bit sample WAV header.  When it is
// called with lenData < 0, it will write the header as if the lenData
// are LONG_MAX minus header length.
// Note: If it returns FALSE, the file may be left in inconsistent
// state.
BOOL PWAVFile::GenerateHeader()
{
  if (IsOpen() == FALSE) {
    PTRACE(1, "Not Open");
    return (FALSE);
  }

  // go to the beginning of the file
  if (PFile::SetPosition(0) == FALSE) {
    PTRACE(1,"Cannot Set Pos");
    return (FALSE);
  }

  // Make sure this is a sane operation.
  if (isValidWAV == TRUE) {
    PTRACE(1, "Overwrites existing WAV header and data (with length " << lenData << ")");
  }

  // only deal with the following format
  numChannels = 1;
  sampleRate = 8000;
  bitsPerSample = 16;

  // Write the RIFF chunk.
  // length of data to follow; use LONG_MAX for now
  PInt32l len_after = (lenData >= 0) ? lenData + 44 - 8 : LONG_MAX;

  // Use PFile::Write as we do not want to use our PWAVFile::Write code
  // as that does some byte swapping
  RETURN_ON_WRITE_FAILURE(PFile::Write(WAV_LABEL_RIFF,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&len_after,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(WAV_LABEL_WAVE,4), 4);

  // Write the FORMAT chunk
  PInt32l len_format = 16;    // length is 16, exclude label_fmt and len_format
  PInt16l format = 0x01;      // Format 0x01 = PCM
  PInt16l num_channels = numChannels;
  PInt32l sample_per_sec = sampleRate;
  PInt16l bits_per_sample = bitsPerSample;
  // These are calculated from the above configuration.
  PInt16l bytes_per_sample = bitsPerSample >> 3;
  PInt32l bytes_per_sec = sample_per_sec * bytes_per_sample;
  
  RETURN_ON_WRITE_FAILURE(PFile::Write(WAV_LABEL_FMT_,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&len_format,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&format,2), 2);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&num_channels,2), 2);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&sample_per_sec,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&bytes_per_sec,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&bytes_per_sample,2), 2);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&bits_per_sample,2), 2);

  // Write the DATA chunk.
  // max data length is LONG_MAX - header length (44)
  PInt32l data_len = (lenData >= 0) ? lenData : LONG_MAX - 44;


  RETURN_ON_WRITE_FAILURE(PFile::Write(WAV_LABEL_DATA,4), 4);
  RETURN_ON_WRITE_FAILURE(PFile::Write(&data_len,4), 4);

  lenHeader = 44;

  isValidWAV = TRUE;
  return (TRUE);
}

// Update the WAV header according to the amount of data that were
// written.  Note we just blindly rewrite the entire header, so if the
// original file has a different FORMAT chunk length, we may corrupt
// the file.
BOOL PWAVFile::UpdateHeader()
{
  GenerateHeader();
  return (TRUE);
}
