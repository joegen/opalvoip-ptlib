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
 * $Log: pwavfile.h,v $
 * Revision 1.13  2003/03/07 06:12:05  robertj
 * Added more WAV file "magic numbers".
 *
 * Revision 1.12  2002/09/16 01:08:59  robertj
 * Added #define so can select if #pragma interface/implementation is used on
 *   platform basis (eg MacOS) rather than compiler, thanks Robert Monaghan.
 *
 * Revision 1.11  2002/06/20 00:51:38  craigs
 * Added virtuals to allow overriding
 *
 * Revision 1.10  2002/05/21 01:56:53  robertj
 * Removed the enum which made yet another set of magic numbers for audio
 *   formats, now uses the WAV file format numbers.
 * Fixed failure to write header when destroying object without and explicit
 *   call to Close().
 * Fixed missing Open() function which does not have file name parameter.
 * Added ability to set the audio format after construction.
 *
 * Revision 1.9  2002/01/22 03:55:07  craigs
 * Added #define guards when file moved to PTCLib
 *
 * Revision 1.8  2002/01/13 21:00:41  rogerh
 * The type of new .WAV files must now be specified in the class constructor.
 * Take out Open() function from the last commit and create a new Open()
 * function which replaces the one in the PFile base class.
 *
 * Revision 1.7  2002/01/11 16:33:46  rogerh
 * Create a PWAVFile Open() function, which processes the WAV header
 *
 * Revision 1.6  2001/10/16 13:27:37  rogerh
 * Add support for writing G.723.1 WAV files.
 * MS Windows can play G.723.1 WAV Files in Media Player and Sound Recorder.
 * Sound Recorder can also convert them to normal PCM format WAV files.
 * Thanks go to M.Stoychev <M.Stoychev@cnsys.bg> for sample WAV files.
 *
 * Revision 1.5  2001/10/15 11:48:15  rogerh
 * Add GetFormat to return the format of a WAV file
 *
 * Revision 1.4  2001/07/23 01:20:20  rogerh
 * Add updates from Shawn - ensure isvalidWAV is false for zero length files.
 * GetDataLength uses actual file size to support file updates as well as appends.
 * Add updates from Roger - Update Header() just writes to specific fields which
 * preserves any 'extra' data in an existing header between FORMAT and DATA chunks.
 *
 * Revision 1.3  2001/07/20 07:06:27  rogerh
 * Fix a typo
 *
 * Revision 1.2  2001/07/20 03:30:59  robertj
 * Minor cosmetic changes to new PWAVFile class.
 *
 * Revision 1.1  2001/07/19 09:55:48  rogerh
 * Add PWAVFile, a class to read and write .wav files, written by
 * Roger Hardiman and <roger@freebsd.org> and
 * Shawn Pai-Hsiang Hsiao <shawn@eecs.harvard.edu>
 *
 *
 */

#ifndef _PWAVFILE
#define _PWAVFILE

//#ifdef P_USE_PRAGMA
//#pragma interface
//#endif


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
      fmt_PCM         = 1,      /// PCM, 8kHz, 16 bit, mono
      fmt_ALaw        = 6,      /// A-Law 8kHz
      fmt_uLaw        = 7,      /// u-Law 8kHz
      fmt_GSM         = 0x31,   /// GSM
      fmt_G728        = 0x41,   /// RFC2361
      fmt_G723        = 0x42,   /// RFC2361
      fmt_MSG7231     = 0x42,   /// Microsoft G.723.1
      fmt_G726        = 0x64,   /// RFC2361
      fmt_G722        = 0x65,   /// RFC2361
      fmt_G729        = 0x84,   /// RFC2361
      fmt_VivoG7231   = 0x111,  /// VivoActive G.723.1

      // For backward compatibility
      PCM_WavFile = fmt_PCM,
      G7231_WavFile = fmt_VivoG7231
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
      unsigned format = fmt_PCM /// Type of WAV File to create
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
      OpenMode mode,          /// Mode in which to open the file.
      int opts = ModeDefault, /// #OpenOptions enum# for open operation.
      unsigned format = fmt_PCM /// Type of WAV File to create
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
      const PFilePath & name,     /// Name of file to open.
      OpenMode mode = ReadWrite,  /// Mode in which to open the file.
      int opts = ModeDefault,     /// #OpenOptions enum# for open operation.
      unsigned format = fmt_PCM /// Type of WAV File to create
    );

    /**Close the file before destruction.
      */
    ~PWAVFile() { Close(); }
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
      void * buf,   /// Pointer to a block of memory to receive the read bytes.
      PINDEX len    /// Maximum number of bytes to read into the buffer.
    );

    /**Call PFile::Write() to write out audio data and perform necessary
       processing such as byte-order swaping.

       @return
       TRUE indicates that at least one character was written to the channel.
       FALSE means no bytes were written due to timeout or some other I/O error.
    */
    virtual BOOL Write(
      const void * buf,   /// Pointer to a block of memory to receive the write bytes.
      PINDEX len    /// Maximum number of bytes to write to the channel.
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
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // Options for open operation.
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
      const PFilePath & name,    // Name of file to open.
      OpenMode mode = ReadWrite, // Mode in which to open the file.
      int opts = ModeDefault     // #OpenOptions enum# for open operation.
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
      off_t pos,                         /// New position to set.
      FilePositionOrigin origin = Start  /// Origin for position change.
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

    /**Find out the format of the WAV file. Eg 0x01 for PCM, 0x42 or 0x111 for G.723.1.
    */
    virtual unsigned GetFormat() const;

    /**Find out the number of channels the WAV file has. Typically this is 1 for
       mono and 2 for stereo.
    */
    virtual unsigned GetChannels() const;

    /**Find out the sample rate of the WAV file in Hz.
    */
    virtual unsigned GetSampleRate() const;

    /**Find out how may bits there are per sample, eg 8 or 16.
    */
    virtual unsigned GetSampleSize() const;

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
  //@}
 

  protected:
    BOOL ProcessHeader();
    BOOL GenerateHeader();
    BOOL UpdateHeader();

    BOOL     isValidWAV;

    unsigned format;
    unsigned numChannels;
    unsigned sampleRate;
    unsigned bitsPerSample;

    off_t    lenHeader;
    off_t    lenData;

    BOOL     header_needs_updating;

};

#endif

// End Of File ///////////////////////////////////////////////////////////////
