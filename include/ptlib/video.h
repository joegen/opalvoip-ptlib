/*
 * video.h
 *
 * Video interface class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * The Initial Developer of the Original Code is Equivalence Pty. Ltd.
 *
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: video.h,v $
 * Revision 1.1  2000/11/09 00:43:04  dereks
 * Initial release.
 *
 * 
 *
 */


#define _PVIDEO

#ifdef __GNUC__
#pragma interface
#endif


/** A class representing a video. A video is a highly platform dependent
   entity that is abstracted for use here. Very little manipulation of the
   videos are possible.

 */
class PVideo : public PBYTEArray
{
  PCLASSINFO(PVideo, PBYTEArray);

  public:
  /**@name Construction */
  //@{
    /**Create a new video, using the parameters provided.
       It is expected that the "lowest common denominator" encoding, linear PCM,
       is used.

       All other values for the encoding are platform dependent.
     */
    PVideo(
      unsigned numChannels = 1,    /// Number of channels eg mono/stereo
      unsigned sampleRate = 8000,  /// Samples per second
      unsigned bitsPerSample = 16, /// Number of bits per sample
      PINDEX   bufferSize = 0,     /// Size of data
      const BYTE * data = NULL     /// Pointer to initial data
    );

    /**Create a new video, reading from a platform dependent file.
     */
    PVideo(
      const PFilePath & filename   /// Video file to load.
    );

    /**Set new data bytes for the video.
     */
    PVideo & operator=(
      const PBYTEArray & data  // New data for video
    );
  //@}

  /**@name File functions */
  //@{
    /**Load a platform dependent video file 

       Also note that not all possible files are playable by this library. No
       format conversions between file and driver are performed.

       @return
       TRUE if the video is loaded successfully.
     */
    BOOL Load(
      const PFilePath & filename   /// Video file to load.
    );

    /**Save a platform dependent video file (eg .WAV file for Win32) from the
       object.

       @return
       TRUE if the video is saved successfully.
     */
    BOOL Save(
      const PFilePath & filename   // Video file to load.
    );
  //@}

  /**@name Access functions */
  //@{
    /// Play the video on the default video device.
    BOOL Play();

    /**Set the internal video format to be:
     */
    void SetFormat(
      unsigned numChannels,   // Number of channels eg mono/stereo
      unsigned sampleRate,    /// Samples per second
      unsigned bitsPerSample  /// Number of bits per sample
    );

    /**Get the current encoding. 
     */
    unsigned GetEncoding()   const { return encoding; }

    /// Get  the number of channels (mono/stereo) in the video.
    unsigned GetChannels()   const { return numChannels; }

    /// Get the sample rate in samples per second.
    unsigned GetSampleRate() const { return sampleRate; }

    /// Get the sample size in bits per sample.
    unsigned GetSampleSize() const { return sampleSize; }

    /// Get the platform dependent error code from the last file load.
    DWORD    GetErrorCode()  const { return dwLastError; }

    /// Get the size of the platform dependent format info.
    PINDEX   GetFormatInfoSize()  const { return formatInfo.GetSize(); }

    /// Get pointer to the platform dependent format info.
    const void * GetFormatInfoData() const { return (const BYTE *)formatInfo; }
  //@}

  /**@name Miscellaneous functions */
  //@{
    /**Play a video file to the default device. If the #wait#
       parameter is TRUE then the function does not return until the file has
       been played. If FALSE then the video play is begun asynchronously and
       the function returns immediately.

       @return
       TRUE if the video is playing or has played.
     */
    static BOOL PlayFile(
      const PFilePath & file, /// Video file to play.
      BOOL wait = TRUE        /// Flag to play video synchronously.
    );

    /// Play the "standard" warning beep for the platform.
    static void Beep();
  //@}

  protected:
    /// Format code
    unsigned   encoding;      
    /// Samples per second
    unsigned   sampleRate;    
    /// Number of bits per sample
    unsigned   sampleSize;    
    /// Last error code for Load()/Save() functions
    DWORD      dwLastError;   
    /// Full info on the format (platform dependent)
    PBYTEArray formatInfo;    
};


/**A class representing a video channel. This class is provided mainly for
   the playback or recording of videos on the system.

   A video driver is either playing or recording. If simultaneous playing and
   recording is desired, two instances of PVideoChannel must be created.

   The video is buffered and the size and number of buffers should be set
   before playing/recording. Each call to Write() will use one buffer, so care
   needs to be taken not to use a large number of small writes but tailor the
   buffers to the size of each write you make.

   Similarly for reading, an entire buffer must be read before any of it is
   available to a Read() call. Note that once a buffer is filled you can read
   it a byte at a time if desired, but as soon as all the data in the buffer
   is used returned, the next read will wait until the entire next buffer is
   read from the hardware. So again, tailor the number and size of buffers to
   the application. To avoid being blocked until the buffer fills, you can use
   the StartRecording() function to initiate the buffer filling, and the
   IsRecordingBufferFull() function to determine when the Read() function will
   no longer block.

   Note that this video channel is implicitly a series of frames in YUV411 format.
   No conversion is performed on data to/from the channel.
 */
class PVideoChannel : public PChannel
{
  PCLASSINFO(PVideoChannel, PChannel);

  public:
  /**@name Construction */
  //@{
    enum Directions {
      Recorder,
      Player
    };

    /// Create a video channel.
    PVideoChannel();

    /** Create a video channel.
        Create a reference to the video drivers for the platform.
      */
    PVideoChannel(
      const PString & device,       /// Name of video driver/device
      Directions dir,               /// Video I/O direction
      unsigned numChannels = 1,     /// Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   /// Samples per second
      unsigned bitsPerSample = 16   /// Number of bits per sample
    );
    // 

    ~PVideoChannel();
    // Destroy and close the video driver
  //@}

  /**@name Open functions */
  //@{
    /**Get all of the names for video devices/drivers that are available on
       this platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the #dir#
       parameter in each value is not necessarily the same.

       @return
       An array of platform dependent strings for each video player/recorder.
     */
    static PStringArray GetDeviceNames(
      Directions dir    // Video I/O direction
    );

    /**Get the name for the default video devices/driver that is on this
       platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the #dir#
       parameter in each value is not necessarily the same.

       @return
       A platform dependent string for the video player/recorder.
     */
    static PString GetDefaultDevice(
      Directions dir    // Video I/O direction
    );


    /**Open the specified device for playing or recording. The device name is
       platform specific and is as returned in the GetDevices() function.

       @return
       TRUE if the video device is valid for playing/recording.
     */
    BOOL Open(
      const PString & device,       /// Name of video driver/device
      Directions dir,               /// Video I/O direction
      unsigned numChannels = 1,     /// Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   /// Samples per second
      unsigned bitsPerSample = 16   /// Number of bits per sample
    );

    /**Abort the background playing/recording of the video channel.

       @return
       TRUE if the video has successfully been aborted.
     */
    BOOL Abort();
  //@}

  /**@name Channel set up functions */
  //@{
    /**Set the format for play/record. Note that linear PCM data is the only
       one supported at this time.

       Note that if the PlayFile() function is used, this may be overridden
       by information in the file being played.

       @return
       TRUE if the format is valid.
     */
    BOOL SetFormat(
      unsigned numChannels = 1,     /// Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   /// Samples per second
      unsigned bitsPerSample = 16   /// Number of bits per sample
    );

  /**@name Play functions */
  //@{
    /**Play a video to the open device. If the #wait# parameter is
       TRUE then the function does not return until the file has been played.
       If FALSE then the video play is begun asynchronously and the function
       returns immediately.

       Note if the driver is closed of the object destroyed then the video
       play is aborted.

       Also note that not all possible videos and video files are playable by
       this library. No format conversions between video object and driver are
       performed.

       @return
       TRUE if the video is playing or has played.
     */

    BOOL PlayVideo(
      const PVideo & video,   /// Video to play.
      BOOL wait = TRUE        /// Flag to play video synchronously.
    );
    /**Play a video file to the open device. If the #wait#
       parameter is TRUE then the function does not return until the file has
       been played. If FALSE then the video play is begun asynchronously and
       the function returns immediately.

       Note if the driver is closed of the object destroyed then the video
       play is aborted.

       Also note that not all possible videos and video files are playable by
       this library. No format conversions between video object and driver are
       performed.

       @return
       TRUE if the video is playing or has played.
     */
    BOOL PlayFile(
      const PFilePath & file, /// Video file to play.
      BOOL wait = TRUE        /// Flag to play video synchronously.
    );

    /**Indicate if the video play begun with PlayBuffer() or PlayFile() has
       completed.

       @return
       TRUE if the video has completed playing.
     */
    BOOL HasPlayCompleted();

    /**Block the thread until the video play begun with PlayBuffer() or
       PlayFile() has completed.

       @return
       TRUE if the video has successfully completed playing.
     */
    BOOL WaitForPlayCompletion();

  //@}

  /**@name Record functions */
  //@{
    /**Record into the video object all of the buffer's of video data. Use the
       SetBuffers() function to determine how long the recording will be made.

       For the Win32 platform, the most efficient way to record a PVideo is to
       use the SetBuffers() function to set a single buffer of the desired
       size and then do the recording. For Linux OSS this can cause problems
       as the buffers are rounded up to a power of two, so to gain more
       accuracy you need a number of smaller buffers.

       Note that this function will block until all of the data is buffered.
       If you wish to do this asynchronously, use StartRecording() and
       AreAllrecordBuffersFull() to determine when you can call RecordVideo()
       without blocking.

       @return
       TRUE if the video has been recorded.
     */
    BOOL RecordVideo(
      PVideo & video /// Video recorded
    );

    /**Record into the platform dependent video file all of the buffer's of
       video data. Use the SetBuffers() function to determine how long the
       recording will be made.

       Note that this function will block until all of the data is buffered.
       If you wish to do this asynchronously, use StartRecording() and
       AreAllrecordBuffersFull() to determine when you can call RecordVideo()
       without blocking.

       @return
       TRUE if the video has been recorded.
     */
    BOOL RecordFile(
      const PFilePath & file /// Video file recorded
    );

    /**Start filling record buffers. The first call to Read() will also
       initiate the recording.

       @return
       TRUE if the video driver has successfully started recording.
     */
    BOOL StartRecording();

    /**Determine if a record buffer has been filled, so that the next Read()
       call will not block. Provided that the amount of data read is less than
       the buffer size.

       @return
       TRUE if the video driver has filled a buffer.
     */
    BOOL IsRecordBufferFull();

    /**Determine if all of the record buffer allocated has been filled. There
       is an implicit Abort() of the recording if this occurs and recording is
       stopped. The channel may need to be closed and opened again to start
       a new recording.

       @return
       TRUE if the video driver has filled a buffer.
     */
    BOOL AreAllRecordBuffersFull();

    /**Block the thread until a record buffer has been filled, so that the
       next Read() call will not block. Provided that the amount of data read
       is less than the buffer size.

       @return
       TRUE if the video driver has filled a buffer.
     */
    BOOL WaitForRecordBufferFull();

    /**Block the thread until all of the record buffer allocated has been
       filled. There is an implicit Abort() of the recording if this occurs
       and recording is stopped. The channel may need to be closed and opened
       again to start a new recording.

       @return
       TRUE if the video driver has filled a buffer.
     */
    BOOL WaitForAllRecordBuffersFull();
  //@}


  private:
    void Construct();

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
