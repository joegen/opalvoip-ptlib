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

#define _PWAVFILE

//#ifdef __GNUC__
//#pragma interface
//#endif


/**A class representing a a WAV audio file.
  */
class PWAVFile : public PFile
{
  PCLASSINFO(PWAVFile, PFile);

  public:
  /**@name Construction */
  //@{
    /**Create a WAV file object but do not open it. It does not
       initially have a valid file name. However, an attempt to open the file
       using the #PFile::Open()# function will generate a unique
       temporary file.
     */
    PWAVFile();

    /**Create a unique temporary file name, and open the file in the specified
       mode and using the specified options. Note that opening a new, unique,
       temporary file name in ReadOnly mode will always fail. This would only
       be usefull in a mode and options that will create the file.

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PWAVFile(
      OpenMode mode,          /// Mode in which to open the file.
      int opts = ModeDefault  /// #OpenOptions enum# for open operation.
    );

    /**Create a WAV file object with the specified name and open it in
       the specified mode and with the specified options.

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PWAVFile(
      const PFilePath & name,    /// Name of file to open.
      OpenMode mode = ReadWrite, /// Mode in which to open the file.
      int opts = ModeDefault     /// #OpenOptions enum# for open operation.
    );
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

    /** Close the file channel.
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
    BOOL SetPosition(
      off_t pos,                         /// New position to set.
      FilePositionOrigin origin = Start  /// Origin for position change.
    );

    /**Get the current active position in the file for the next read
       or write operation. The WAV header is excluded from calculation
       the position.

       @return
       current file position relative to the end of the WAV header.
     */
    off_t GetPosition() const;
  //@}

  /**@name Member variable access */
  //@{
    /**Get how many channels the WAV file has. Typically this is 1 for
       mono and 2 for stereo.
    */
    unsigned GetChannels() const;

    /**Find out the sample rate of the WAV file in Hz.
    */
    unsigned GetSampleRate() const;

    /**Find out how may bits there are per sample, eg 8 or 16.
    */
    unsigned GetSampleSize() const;

    /**Find out the size of WAV header presented in the file.
    */
    off_t GetHeaderLength() const;

    /**Find out how many bytes of audio data there are.
    */
    off_t GetDataLength() const;

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
    unsigned numChannels;
    unsigned sampleRate;
    unsigned bitsPerSample;
    off_t    lenHeader;
    off_t    lenData;
    BOOL     header_needs_updating;

};


// End Of File ///////////////////////////////////////////////////////////////
