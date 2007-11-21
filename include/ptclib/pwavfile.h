/*
 * pwavfile.h
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
 * $Id$
 */

#ifndef _PWAVFILE
#define _PWAVFILE

//#ifdef P_USE_PRAGMA
//#pragma interface
//#endif

#ifndef _PTLIB_H
#include <ptlib.h>
#endif

#include <ptlib/pfactory.h>

class PWAVFile;

namespace PWAV {

#ifdef __GNUC__
#define P_PACKED    __attribute__ ((packed));
#else
#define P_PACKED
#pragma pack(1)
#endif

struct ChunkHeader
{
  char    tag[4];
  PInt32l len    P_PACKED;
};

struct RIFFChunkHeader 
{
  ChunkHeader hdr;
  char        tag[4];
};

struct FMTChunk
{
  ChunkHeader hdr;                    ///< chunk header (already packed)
  PUInt16l format          P_PACKED;  ///< Format 
  PUInt16l numChannels     P_PACKED;  ///< Channels 0x01 = mono, 0x02 = stereo
  PUInt32l sampleRate      P_PACKED;  ///< Sample Rate in Hz
  PUInt32l bytesPerSec     P_PACKED;  ///< Average bytes Per Second
  PUInt16l bytesPerSample  P_PACKED;  ///< Bytes Per Sample, eg 2
  PUInt16l bitsPerSample   P_PACKED;  ///< Bits Per Sample, eg 16
};

}; // namespace PWAV

#ifdef __GNUC__
#undef P_PACKED
#else
#pragma pack()
#endif

/**
  * abstract factory class for handling WAV files formats
  */
class PWAVFileFormat
{
  public:
    virtual ~PWAVFileFormat() { }

    /**
      * return a PWAVFile format code
      */
    virtual unsigned GetFormat() const = 0;

    /**
      * return a string that can be used as a media format
      */
    virtual PString GetFormatString() const = 0;

    /**
     *  return a string that can be used as a text description
     */
    virtual PString GetDescription() const = 0;

    /**
     *  populate the header with the correct values
     */
    virtual void CreateHeader(PWAV::FMTChunk & header, PBYTEArray & extendedHeader) = 0;

    /**
     *  populate the header with the correct values after initial parameters are set
     */
    virtual void UpdateHeader(PWAV::FMTChunk & /*header*/, PBYTEArray & /*extendedHeader*/)
    { }

    /**
      * write any extra headers after the FORMAT chunk
      */
    virtual BOOL WriteExtraChunks(PWAVFile & /*file*/)
    { return TRUE; }

    /**
      * read any extra headers after the FORMAT chunk
      */
    virtual BOOL ReadExtraChunks(PWAVFile & /*file*/)
    { return TRUE; }

     /**
      * called before the reading/writing starts
      */
    virtual void OnStart()
    { }

     /**
      * called after the reading/writing stops
      */
    virtual void OnStop()
    { }

    /**
      * write data to the file
      */
    virtual BOOL Read(PWAVFile & file, void * buf, PINDEX & len);

    /**
      * read data from the file
      */
    virtual BOOL Write(PWAVFile & file, const void * buf, PINDEX & len);
};

typedef PFactory<PWAVFileFormat, PCaselessString> PWAVFileFormatByFormatFactory;
typedef PFactory<PWAVFileFormat, unsigned> PWAVFileFormatByIDFactory;

/**
  * abstract factory class for autoconversion of WAV files to/from PCM-16
  */
class PWAVFileConverter 
{
  public:
    virtual ~PWAVFileConverter() { }
    virtual unsigned GetFormat    (const PWAVFile & file) const = 0;
    virtual off_t GetPosition     (const PWAVFile & file) const = 0;
    virtual BOOL SetPosition      (PWAVFile & file, off_t pos, PFile::FilePositionOrigin origin) = 0;
    virtual unsigned GetSampleSize(const PWAVFile & file) const = 0;
    virtual off_t GetDataLength   (PWAVFile & file) = 0;
    virtual BOOL Read             (PWAVFile & file, void * buf, PINDEX len)  = 0;
    virtual BOOL Write            (PWAVFile & file, const void * buf, PINDEX len) = 0;
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
    /**When a file is opened for writing, we can specify if this is a PCM
       wav file or a G.723.1 wav file.
     */
    enum {
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

      // allow opening files without knowing the format
      fmt_NotKnown    = 0x10000
    };

    /**Create a WAV file object but do not open it. It does not
       initially have a valid file name. However, an attempt to open the file
       using the #PFile::Open()# function will generate a unique
       temporary file.

       If a WAV file is being created, the type parameter can be used
       to create a PCM Wave file or a G.723.1 Wave file by using
       #WaveType enum#
     */
    PWAVFile(
      unsigned format = fmt_PCM ///< Type of WAV File to create
    );
    static PWAVFile * format(
      const PString & format    ///< Type of WAV File to create
    );

    /**Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       If a WAV file is being created, the type parameter can be used
       to create a PCM Wave file or a G.723.1 Wave file by using
       #WaveType enum#

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PWAVFile(
      OpenMode mode,          ///< Mode in which to open the file.
      int opts = ModeDefault, ///< #OpenOptions enum# for open operation.
      unsigned format = fmt_PCM ///< Type of WAV File to create
    );
    static PWAVFile * format(
      const PString & format,  ///< Type of WAV File to create
      PFile::OpenMode mode,          ///< Mode in which to open the file.
      int opts = PFile::ModeDefault ///< #OpenOptions enum# for open operation.
    );

    /**Create a WAV file object with the specified name and open it in
       the specified mode and with the specified options.
       If a WAV file is being created, the type parameter can be used
       to create a PCM Wave file or a G.723.1 Wave file by using
       #WaveType enum#

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PWAVFile(
      const PFilePath & name,     ///< Name of file to open.
      OpenMode mode = ReadWrite,  ///< Mode in which to open the file.
      int opts = ModeDefault,     ///< #OpenOptions enum# for open operation.
      unsigned format = fmt_PCM ///< Type of WAV File to create
    );
    PWAVFile(
      const PString & format,  ///< Type of WAV File to create
      const PFilePath & name,     ///< Name of file to open.
      OpenMode mode = PFile::ReadWrite,  ///< Mode in which to open the file.
      int opts = PFile::ModeDefault     ///< #OpenOptions enum# for open operation.
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
       TRUE indicates that at least one character was read from the channel.
       FALSE means no bytes were read due to timeout or some other I/O error.
    */
    virtual BOOL Read(
      void * buf,   ///< Pointer to a block of memory to receive the read bytes.
      PINDEX len    ///< Maximum number of bytes to read into the buffer.
    );

    /**Call PFile::Write() to write out audio data and perform necessary
       processing such as byte-order swaping.

       @return
       TRUE indicates that at least one character was written to the channel.
       FALSE means no bytes were written due to timeout or some other I/O error.
    */
    virtual BOOL Write(
      const void * buf,   ///< Pointer to a block of memory to receive the write bytes.
      PINDEX len    ///< Maximum number of bytes to write to the channel.
    );

    /**Open the current file in the specified mode and with
       the specified options. If the file object already has an open file then
       it is closed.
       
       If there has not been a filename attached to the file object (via
       #SetFilePath()#, the #name# parameter or a previous
       open) then a new unique temporary filename is generated.

       @return
       TRUE if the file was successfully opened.
     */
    virtual BOOL Open(
      OpenMode mode = ReadWrite,  ///< Mode in which to open the file.
      int opts = ModeDefault      ///< Options for open operation.
    );

    /**Open the specified WAV file name in the specified mode and with
       the specified options. If the file object already has an open file then
       it is closed.
       This reads (and validates) the header for existing files.
       For new files, it creates a new file (and header) using the type of
       WAV file specified in the class constructor.
       
       Note: if #mode# is StandardInput, StandardOutput or StandardError,   
       then the #name# parameter is ignored.

       @return
       TRUE if the file was successfully opened.
     */
    virtual BOOL Open(
      const PFilePath & name,    ///< Name of file to open.
      OpenMode mode = ReadWrite, ///< Mode in which to open the file.
      int opts = ModeDefault     ///< #OpenOptions enum# for open operation.
    );

    /** Close the file channel.
        If a WAV file has been written to, this will update the header
        to contain the correct size information.
        @return TRUE if close was OK.
      */
    virtual BOOL Close();

    /**Set the current active position in the file for the next read or write
       operation. The #pos# variable is a signed number which is
       added to the specified origin. For #origin == PFile::Start#
       only positive values for #pos# are meaningful. For
       #origin == PFile::End# only negative values for
       #pos# are meaningful.

       Note that for a WAV file, the origin of the file is right after
       the header. That is, the WAV header is not included when
       perform SetPosition().

       @return
       TRUE if the new file position was set.
     */
    virtual BOOL SetPosition(
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
    /**Find out the format of the WAV file. Eg 0x01 for PCM, 0x42 or 0x111 for G.723.1.
    */
    virtual BOOL SetFormat(unsigned fmt);
    virtual BOOL SetFormat(const PString & format);

    /**Find out the format of the WAV file. Eg 0x01 for PCM, 0x42 or 0x111 for G.723.1.
    */
    virtual unsigned GetFormat() const;
    virtual PString GetFormatAsString() const;

    /**Find out the number of channels the WAV file has. Typically this is 1 for
       mono and 2 for stereo.
    */
    virtual unsigned GetChannels() const;
    virtual void SetChannels(unsigned v);

    /**Find out the sample rate of the WAV file in Hz.
    */
    virtual unsigned GetSampleRate() const;
    virtual void SetSampleRate(unsigned v);

    /**Find out how may bits there are per sample, eg 8 or 16.
    */
    virtual unsigned GetSampleSize() const;
    virtual void SetSampleSize(unsigned v);

    /**Find out how may bytes there are per second
    */
    virtual unsigned GetBytesPerSecond() const;
    virtual void SetBytesPerSecond(unsigned v);

    /**Find out the size of WAV header presented in the file.
    */
    off_t GetHeaderLength() const;

    /**Find out how many bytes of audio data there are.
    */
    virtual off_t GetDataLength();

    /**Determine if the WAV file is a valid wave file.

      @return
      TRUE indicates that the WAV file is valid
      FALSE indicates that the WAV file is invalid
    */
    BOOL IsValid() const { return isValidWAV; }

    /**
      *Return a string that describes the WAV format
      */
    PString GetFormatString() const
    { if (formatHandler == NULL) return PString("N/A"); else return formatHandler->GetFormatString(); }

    /**
      * enable autoconversion between PCM-16 and the native format
      */
    void SetAutoconvert();

  //@}
 
    friend class PWAVFileConverter;

    BOOL RawRead(void * buf, PINDEX len);
    BOOL RawWrite(const void * buf, PINDEX len);

    BOOL FileRead(void * buf, PINDEX len);
    BOOL FileWrite(const void * buf, PINDEX len);

    off_t RawGetPosition() const;
    BOOL RawSetPosition(off_t pos, FilePositionOrigin origin);
    off_t RawGetDataLength();

    void SetLastReadCount(PINDEX v) { lastReadCount = v; } 

    PWAV::FMTChunk wavFmtChunk;
    PBYTEArray extendedHeader;

  protected:
    void Construct();
    void SelectFormat(unsigned fmt);
    void SelectFormat(const PString & format);

    PBYTEArray wavHeaderData;

    BOOL ProcessHeader();
    BOOL GenerateHeader();
    BOOL UpdateHeader();

    BOOL     isValidWAV;

    unsigned int origFmt;
    PWAVFileFormat * formatHandler;

    BOOL     autoConvert;
    PWAVFileConverter * autoConverter;

    off_t lenHeader;
    off_t lenData;

    BOOL     header_needs_updating;
};

#endif

// End Of File ///////////////////////////////////////////////////////////////
