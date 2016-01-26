/*
 * pwavfile.h
 *
 * WAV file I/O channel class.
 *
 * Portable Tools Library
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

#ifndef PTLIB_PWAVFILE_H
#define PTLIB_PWAVFILE_H

//#ifdef P_USE_PRAGMA
//#pragma interface
//#endif

#include <ptlib.h>

#ifdef P_WAVFILE

#include <ptlib/pfactory.h>

class PWAVFile;

namespace PWAV {

#pragma pack(1)

  struct ChunkHeader
  {
    char    tag[4];
    PInt32l len;
  };

  struct RIFFChunkHeader
  {
    ChunkHeader hdr;
    char        tag[4];
  };

  struct FMTChunk
  {
    ChunkHeader hdr;          ///< chunk header (already packed)
    PUInt16l format;          ///< Format
    PUInt16l numChannels;     ///< Channels 0x01 = mono, 0x02 = stereo
    PUInt32l sampleRate;      ///< Sample Rate in Hz
    PUInt32l bytesPerSec;     ///< Average bytes Per Second
    PUInt16l bytesPerSample;  ///< Bytes Per Sample, eg 2
    PUInt16l bitsPerSample;   ///< Bits Per Sample, eg 16
  };

  struct G7231ExtendedInfo
  {
    PInt16l data1;      // 1
    PInt16l data2;      // 480
  };

  struct G7231FACTChunk
  {
    PWAV::ChunkHeader hdr;
    PInt32l data1;      // 0   Should be number of samples.
  };

#pragma pack()

}; // namespace PWAV


/**Abstract factory class for handling WAV files formats.
 */
class PWAVFileFormat
{
public:
  virtual ~PWAVFileFormat() { }

  /**Return a PWAVFile format code.
   */
  virtual unsigned GetFormat() const = 0;

  /**Return a string that can be used as a media format.
   */
  virtual PString GetFormatString() const = 0;

  /**Return a string that can be used as a text description.
   */
  virtual PString GetDescription() const = 0;

  /// Check that this format can be set to the number of channels
  virtual bool CanSetChannels(unsigned channels) const = 0;

  /**Populate the header with the correct values.
   */
  virtual void CreateHeader(PWAV::FMTChunk & header, PBYTEArray & extendedHeader) = 0;

  /**Populate the header with the correct values after initial parameters are set.
   */
  virtual void UpdateHeader(PWAV::FMTChunk & /*header*/, PBYTEArray & /*extendedHeader*/)
  { }

  /**Populate the header with the correct values after initial parameters are set.
   */
  virtual void ProcessHeader(const PWAV::FMTChunk & /*header*/, const PBYTEArray & /*extendedHeader*/)
  { }

  /**Write any extra headers after the FORMAT chunk.
   */
  virtual PBoolean WriteExtraChunks(PWAVFile & /*file*/)
  { return true; }

  /**Read any extra headers after the FORMAT chunk.
   */
  virtual PBoolean ReadExtraChunks(PWAVFile & /*file*/)
  { return true; }

  /**Called before the reading/writing starts.
   */
  virtual void OnStart()
  { }

  /**Called after the reading/writing stops.
   */
  virtual void OnStop()
  { }

  /**Write data to the file.
   */
  virtual PBoolean Read(PWAVFile & file, void * buf, PINDEX & len);

  /**Read data from the file.
   */
  virtual PBoolean Write(PWAVFile & file, const void * buf, PINDEX & len);
};

typedef PFactory<PWAVFileFormat, PCaselessString> PWAVFileFormatByFormatFactory;
typedef PFactory<PWAVFileFormat, unsigned> PWAVFileFormatByIDFactory;

#define PCREATE_WAVFILE_FORMAT_FACTORY(cls, id, name) \
  PFACTORY_CREATE(PWAVFileFormatByIDFactory, cls, id); \
  typedef cls cls##_ByFormat; \
  PFACTORY_CREATE(PWAVFileFormatByFormatFactory, cls##_ByFormat, name)

PFACTORY_LOAD(PWAVFileFormatPCM);


/**Abstract factory class for autoconversion of WAV files to/from PCM-16.
 */
class PWAVFileConverter
{
public:
  virtual ~PWAVFileConverter() { }
  virtual unsigned GetFormat    (const PWAVFile & file) const = 0;
  virtual off_t GetPosition     (const PWAVFile & file) const = 0;
  virtual PBoolean SetPosition  (PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin) = 0;
  virtual unsigned GetSampleSize(const PWAVFile & file) const = 0;
  virtual off_t GetDataLength   (PWAVFile & file) = 0;
  virtual PBoolean Read         (PWAVFile & file, void * buf, PINDEX len)  = 0;
  virtual PBoolean Write        (PWAVFile & file, const void * buf, PINDEX len) = 0;

protected:
};

typedef PFactory<PWAVFileConverter, unsigned> PWAVFileConverterFactory;

/**A class representing a WAV audio file.
 */
class PWAVFile : public PFile
{
  PCLASSINFO(PWAVFile, PFile);

public:
  /**@name Construction */
  //@{
  /**The type of codec being used to encode the audio in the WAV file.
  */
  enum WaveType {
    fmt_PCM         = 1,      ///< PCM, 8kHz, 16 bit, mono
    fmt_MSADPCM     = 2,      ///< MS-ADPCM, 8kHz, mono
    fmt_ALaw        = 6,      ///< A-Law 8kHz
    fmt_uLaw        = 7,      ///< u-Law 8kHz
    fmt_VOXADPCM    = 0x10,   ///< OKI ADPCM
    fmt_IMAADPCM    = 0x11,   ///< IMA-ADPCM, 8kHz mono
    fmt_GSM         = 0x31,   ///< GSM
    fmt_G728        = 0x41,   ///< RFC2361
    fmt_G723        = 0x42,   ///< RFC2361
    fmt_MSG7231     = 0x42,   ///< Microsoft G.723.1
    fmt_G726        = 0x64,   ///< RFC2361
    fmt_G722        = 0x65,   ///< RFC2361
    fmt_G729        = 0x83,   ///< RFC2361
    fmt_VivoG7231   = 0x111,  ///< VivoActive G.723.1

    // For backward compatibility
    PCM_WavFile     = fmt_PCM,
    G7231_WavFile   = fmt_VivoG7231,
  };

  /**Create a WAV file object but do not open it. It does not
     initially have a valid file name. However, an attempt to open the file
     using the <code>PFile::Open()</code> function will generate a unique
     temporary file.
  */
  PWAVFile(
    unsigned createFormat = fmt_PCM  ///< Type of WAV File to create
  );

  /**Create a unique temporary file name, and open the file in the specified
     mode and using the specified options. Note that opening a new, unique,
     temporary file name in ReadOnly mode will always fail. This would only
     be usefull in a mode and options that will create the file.

     The <code>PChannel::IsOpen()</code> function may be used after object
     construction to determine if the file was successfully opened.
  */
  PWAVFile(
    OpenMode mode,                  ///< Mode in which to open the file.
    OpenOptions opts = ModeDefault, ///< <code>OpenOptions</code> enum for open operation.
    unsigned createFormat = fmt_PCM ///< Type of WAV File to create, if mode is WriteOnly
  );

  /**Create a WAV file object with the specified name and open it in
     the specified mode and with the specified options.
     If a WAV file is being created, the type parameter can be used
     to create a PCM Wave file or a G.723.1 Wave file by using
     <code>WaveType</code> enum.

     The <code>PChannel::IsOpen()</code> function may be used after object
     construction to determine if the file was successfully opened.
  */
  PWAVFile(
    const PFilePath & name,         ///< Name of file to open.
    OpenMode mode = ReadWrite,      ///< Mode in which to open the file.
    OpenOptions opts = ModeDefault, ///< <code>OpenOptions</code> enum for open operation.
    unsigned createFormat = fmt_PCM ///< Type of WAV File to create, if mode is WriteOnly
  );

  PWAVFile(
    const PString & createFormat,   ///< Type of WAV File to create, if mode is WriteOnly
    const PFilePath & name,         ///< Name of file to open.
    OpenMode mode = ReadWrite,      ///< Mode in which to open the file.
    OpenOptions opts = ModeDefault  ///< <code>OpenOptions</code> enum for open operation.
  );

  /**Close the file before destruction.
   */
  ~PWAVFile();
  //@}

  /**@name Overrides from class PFile */
  //@{
  /**Call PFile::Read() to read in audio data and perform necessary
     processing such as byte-order swaping.

     @return
     true indicates that at least one character was read from the channel.
     false means no bytes were read due to timeout or some other I/O error.
  */
  virtual PBoolean Read(
    void * buf,   ///< Pointer to a block of memory to receive the read bytes.
    PINDEX len    ///< Maximum number of bytes to read into the buffer.
  );

  /**Call PFile::Write() to write out audio data and perform necessary
     processing such as byte-order swaping.

     @return
     true indicates that at least one character was written to the channel.
     false means no bytes were written due to timeout or some other I/O error.
  */
  virtual PBoolean Write(
    const void * buf,   ///< Pointer to a block of memory to receive the write bytes.
    PINDEX len    ///< Maximum number of bytes to write to the channel.
  );

  /** Close the file channel.
      If a WAV file has been written to, this will update the header
      to contain the correct size information.
      @return true if close was OK.
  */
  virtual PBoolean Close();

  /**Get the current size of the file.

      @return
      length of file in bytes.
    */
  virtual off_t GetLength() const;
      
  /**Set the size of the file, padding with 0 bytes if it would require
      expanding the file, or truncating it if being made shorter.

      @return
      true if the file size was changed to the length specified.
    */
  virtual PBoolean SetLength(
    off_t len   // New length of file.
  );

  /**Set the current active position in the file for the next read or write
     operation. The \p pos variable is a signed number which is
     added to the specified origin. For \p origin == PFile::Start
     only positive values for \p pos are meaningful. For
     \p origin == PFile::End only negative values for
     \p pos are meaningful.

     Note that for a WAV file, the origin of the file is right after
     the header. That is, the WAV header is not included when
     perform SetPosition().

     @return
     true if the new file position was set.
  */
  virtual PBoolean SetPosition(
    off_t pos,                         ///< New position to set.
    FilePositionOrigin origin = Start  ///< Origin for position change.
  );

  /**Get the current active position in the file for the next read
     or write operation. The WAV header is excluded from calculation
     the position.

     @return
     current file position relative to the end of the WAV header.
  */
  virtual off_t GetPosition() const;
  //@}

  /**@name Member variable access */
  //@{
  /**Set the codec type, format, of the WAV file.
     Note this can only be performed for WriteOnly files, and before the first
     call to Write() is executed.
   */
  virtual PBoolean SetFormat(unsigned fmt);
  virtual PBoolean SetFormat(const PString & format);

  /**Find out the format of the WAV file. Eg 0x01 for PCM, 0x42 or 0x111 for G.723.1.
   */
  virtual unsigned GetFormat() const;
  virtual PString GetFormatString() const;

  /**Get the number of channels the WAV file has.
     Typically this is 1 for mono and 2 for stereo.
    */
  virtual unsigned GetChannels() const;

  /**Set the number of channels the WAV file has.
     Typically this is 1 for mono and 2 for stereo.

     Note this can only be performed for WriteOnly files, and before the first
     call to Write() is executed.
    */
  virtual void SetChannels(unsigned v);

  /**Get the sample rate of the WAV file in Hz.
   */
  virtual unsigned GetSampleRate() const;

  /**Set the sample rate of the WAV file in Hz.
     Note this can only be performed for WriteOnly files, and before the first
     call to Write() is executed.
   */
  virtual void SetSampleRate(unsigned v);

  /**Get how many bits there are per sample, eg 8 or 16.
   */
  virtual unsigned GetSampleSize() const;

  /// Get original unconverted samepl size
  unsigned GetRawSampleSize() const;

  /**Set how many bits there are per sample, eg 8 or 16.
     Note this can only be performed for WriteOnly files, and before the first
     call to Write() is executed.
   */
  virtual void SetSampleSize(unsigned v);

  /**Get how may bytes there are per second
   */
  virtual unsigned GetBytesPerSecond() const;

  /**Set how may bytes there are per second
     Note this can only be performed for WriteOnly files, and before the first
     call to Write() is executed.
   */
  virtual void SetBytesPerSecond(unsigned v);

  /**Enable autoconversion between the native format and PCM-16.
     Note, this only applies to ReadOnly files.
   */
  bool SetAutoconvert(bool convert = true);
  //@}

  // Internal stuff
  bool RawRead(void * buf, PINDEX len);
  bool RawWrite(const void * buf, PINDEX len);

  off_t RawGetPosition() const;
  bool RawSetPosition(off_t pos, FilePositionOrigin origin);
  off_t RawGetDataLength() { return m_dataLength; }

  // Restored for backward compatibility reasons
  static PWAVFile * format(const PString & format);
  static PWAVFile * format(const PString & format, OpenMode mode, OpenOptions opts = ModeDefault);
  __inline bool IsValid() const { return IsOpen(); }


protected:
  virtual bool InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions);
  void Construct(OpenMode mode);
  bool SelectFormat(PWAVFileFormat * handler);

  bool ProcessHeader();
  bool GenerateHeader();
  bool UpdateHeader();

  unsigned            m_createFormat;
  PWAV::FMTChunk      m_wavFmtChunk;
  PBYTEArray          m_wavHeaderData;
  PBYTEArray          m_extendedHeader;
  off_t               m_headerLength;
  off_t               m_dataLength;

  PWAVFileFormat    * m_formatHandler;
  PWAVFileConverter * m_autoConverter;

  enum {
    e_Reading,
    e_PreWrite,
    e_Writing
  } m_status;

  // Rate/channel conversion of WAV file
  unsigned     m_readSampleRate;
  unsigned     m_readChannels;
  PShortArray  m_readBuffer;
  PINDEX       m_readBufCount;
  PINDEX       m_readBufPos;
};

#endif // P_WAVFILE

#endif // PTLIB_PWAVFILE_H

// End Of File ///////////////////////////////////////////////////////////////
