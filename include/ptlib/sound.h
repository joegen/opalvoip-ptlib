/*
 * sound.h
 *
 * Sound interface class.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: sound.h,v $
 * Revision 1.13  1999/02/22 10:15:14  robertj
 * Sound driver interface implementation to Linux OSS specification.
 *
 * Revision 1.12  1999/02/16 06:02:27  robertj
 * Major implementation to Linux OSS model
 *
 * Revision 1.11  1998/09/23 06:21:27  robertj
 * Added open source copyright license.
 *
 * Revision 1.10  1995/06/17 11:13:26  robertj
 * Documentation update.
 *
 * Revision 1.9  1995/03/14 12:42:40  robertj
 * Updated documentation to use HTML codes.
 *
 * Revision 1.8  1995/01/16  09:42:05  robertj
 * Documentation.
 *
 * Revision 1.7  1994/08/23  11:32:52  robertj
 * Oops
 *
 * Revision 1.6  1994/08/22  00:46:48  robertj
 * Added pragma fro GNU C++ compiler.
 *
 * Revision 1.5  1994/06/25  11:55:15  robertj
 * Unix version synchronisation.
 *
 * Revision 1.4  1994/01/03  04:42:23  robertj
 * Mass changes to common container classes and interactors etc etc etc.
 *
 * Revision 1.3  1993/09/29  03:06:30  robertj
 * Added unix compatibility to Beep()
 *
 * Revision 1.2  1993/07/14  12:49:16  robertj
 * Fixed RCS keywords.
 *
 */


#define _PSOUND

#ifdef __GNUC__
#pragma interface
#endif


class PSound : public PBYTEArray
{
  PCLASSINFO(PSound, PBYTEArray)
/* A class representing a sound. A sound is a highly platform dependent
   entity that is abstracted for use here. Very little manipulation of the
   sounds are possible.

   The most common sound to use is the static function <A>Beep()</A> which
   emits the system standard "warning" or "attention" sound.
 */

  public:
    PSound(
      unsigned numChannels = 1,    // Number of channels eg mono/stereo
      unsigned sampleRate = 8000,  // Samples per second
      unsigned bitsPerSample = 16, // Number of bits per sample
      PINDEX   bufferSize = 0,     // Size of data
      const BYTE * data = NULL     // Pointer to initial data
    );
    PSound(
      const PFilePath & filename   // Sound file to load.
    );
    /* Create a new sound, using the parameters provided. A value of zero for
       the <CODE>format</CODE> argument indicates a "lowest common
       denominator" encoding, linear PCM.

       All other values for the encoding are platform dependent.
     */


    static void Beep(); // Play the "standard" warning beep for the platform.


    PSound & operator=(
      const PBYTEArray & data  // New data for sound
    );
    /* set new data bytes for the sound.
     */


    void SetFormat(
      unsigned numChannels,   // Number of channels eg mono/stereo
      unsigned sampleRate,    // Samples per second
      unsigned bitsPerSample  // Number of bits per sample
    );
    /* Set the internal sound forman to linear PCM at the specification in
       the parameters.
     */


    BOOL Load(
      const PFilePath & filename   // Sound file to load.
    );
    /* Load a platform dependent sound file (eg .WAV file for Win32) into the
       object. Note the whole file must able to be loaded into memory.

       Also note that not all possible files are playable by this library. No
       format conversions between file and driver are performed.

       <H2>Returns:</H2>
       TRUE if the sound is loaded successfully.
     */

    BOOL Save(
      const PFilePath & filename   // Sound file to load.
    );
    /* Save a platform dependent sound file (eg .WAV file for Win32) from the
       object.

       <H2>Returns:</H2>
       TRUE if the sound is saved successfully.
     */


    unsigned GetEncoding()   const { return encoding; }
    unsigned GetChannels()   const { return numChannels; }
    unsigned GetSampleRate() const { return sampleRate; }
    unsigned GetSampleSize() const { return sampleSize; }
    DWORD    GetErrorCode()  const { return dwLastError; }
    PINDEX   GetFormatInfoSize()  const { return formatInfo.GetSize(); }
    const void * GetFormatInfoData() const { return (const BYTE *)formatInfo; }


  protected:
    unsigned   encoding;      // Format code
    unsigned   numChannels;   // Number of channels eg mono/stereo
    unsigned   sampleRate;    // Samples per second
    unsigned   sampleSize;    // Number of bits per sample

    DWORD      dwLastError;   // Last error code for Load()/Save() functions

    PBYTEArray formatInfo;    // Full info on the format (platform dependent)
};


class PSoundChannel : public PChannel
{
  PCLASSINFO(PSoundChannel, PChannel)
/* A class representing a sound channel. This class is provided mainly for
   the playback or recording of sounds on the system.

   A sound driver is either playing or recording. If simultaneous playing and
   recording is desired, two instances of PSoundChannel must be created.

   The sound is buffered and the size and number of buffers should be set
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

   Note that this sound channel is implicitly a linear PCM channel. No data
   conversion is performed on data to/from the channel.
 */

  public:
    enum Directions {
      Recorder,
      Player
    };

    PSoundChannel();
    PSoundChannel(
      const PString & device,       // Name of sound driver/device
      Directions dir,               // Sound I/O direction
      unsigned numChannels = 1,     // Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   // Samples per second
      unsigned bitsPerSample = 16   // Number of bits per sample
    );
    // Create a reference to the sound drivers for the platform.

    ~PSoundChannel();
    // Destroy and close the sound driver


    static PStringArray GetDeviceNames(
      Directions dir    // Sound I/O direction
    );
    /* Get all of the names for sound devices/drivers that are available on
       this platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the <CODE>dir</CODE>
       parameter in each value is not necessarily the same.

       <H2>Returns:</H2>
       An array of platform dependent strings for each sound player/recorder.
     */


    BOOL Open(
      const PString & device,       // Name of sound driver/device
      Directions dir,               // Sound I/O direction
      unsigned numChannels = 1,     // Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   // Samples per second
      unsigned bitsPerSample = 16   // Number of bits per sample
    );
    /* Open the specified device for playing or recording. The device name is
       platform specific and is as returned in the GetDevices() function.

       <H2>Returns:</H2>
       TRUE if the sound device is valid for playing/recording.
     */


    BOOL SetFormat(
      unsigned numChannels = 1,     // Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   // Samples per second
      unsigned bitsPerSample = 16   // Number of bits per sample
    );
    /* Set the format for play/record. Note that linear PCM data is the only
       one supported at this time.

       Note that if the PlayFile() function is used, this may be overridden
       by information in the file being played.

       <H2>Returns:</H2>
       TRUE if the format is valid.
     */


    BOOL SetBuffers(
      PINDEX size,      // Size of each buffer
      PINDEX count = 2  // Number of buffers
    );
    /* Set the internal buffers for the sound channel I/O.

       Note that with Linux OSS, the size is always rounded up to the nearest
       power of two, so 20000 => 32768.

       <H2>Returns:</H2>
       TRUE if the sound device is valid for playing/recording.
     */

    BOOL GetBuffers(
      PINDEX & size,    // Size of each buffer
      PINDEX & count    // Number of buffers
    );
    /* Get the internal buffers for the sound channel I/O.

       <H2>Returns:</H2>
       TRUE if the buffer size were obtained.
     */


    BOOL PlaySound(
      const PSound & sound,   // Sound to play.
      BOOL wait = TRUE        // Flag to play sound synchronously.
    );
    /* Play a sound to the open device. If the <CODE>wait</CODE> parameter is
       TRUE then the function does not return until the file has been played.
       If FALSE then the sound play is begun asynchronously and the function
       returns immediately.

       Note if the driver is closed of the object destroyed then the sound
       play is aborted.

       Also note that not all possible sounds and sound files are playable by
       this library. No format conversions between sound object and driver are
       performed.

       <H2>Returns:</H2>
       TRUE if the sound is playing or has played.
     */

    BOOL RecordSound(
      PSound & sound // Sound recorded
    );
    /* Record into the sound object all of the buffer's of sound data. Use the
       SetBuffers() function to determine how long the recording will be made.

       For the Win32 platform, the most efficient way to record a PSound is to
       use the SetBuffers() function to set a single buffer of the desired
       size and then do the recording. For Linux OSS this can cause problems
       as the buffers are rounded up to a power of two, so to gain more
       accuracy you need a number of smaller buffers.

       Note that this function will block until all of the data is buffered.
       If you wish to do this asynchronously, use StartRecording() and
       AreAllrecordBuffersFull() to determine when you can call RecordSound()
       without blocking.

       <H2>Returns:</H2>
       TRUE if the sound has been recorded.
     */

    BOOL PlayFile(
      const PFilePath & file, // Sound file to play.
      BOOL wait = TRUE        // Flag to play sound synchronously.
    );
    /* Play a sound file to the open device. If the <CODE>wait</CODE>
       parameter is TRUE then the function does not return until the file has
       been played. If FALSE then the sound play is begun asynchronously and
       the function returns immediately.

       Note if the driver is closed of the object destroyed then the sound
       play is aborted.

       Also note that not all possible sounds and sound files are playable by
       this library. No format conversions between sound object and driver are
       performed.

       <H2>Returns:</H2>
       TRUE if the sound is playing or has played.
     */

    BOOL RecordFile(
      const PFilePath & file // Sound file recorded
    );
    /* Record into the platform dependent sound file all of the buffer's of
       sound data. Use the SetBuffers() function to determine how long the
       recording will be made.

       Note that this function will block until all of the data is buffered.
       If you wish to do this asynchronously, use StartRecording() and
       AreAllrecordBuffersFull() to determine when you can call RecordSound()
       without blocking.

       <H2>Returns:</H2>
       TRUE if the sound has been recorded.
     */


    BOOL HasPlayCompleted();
    /* Indicate if the sound play begun with PlayBuffer() or PlayFile() has
       completed.

       <H2>Returns:</H2>
       TRUE if the sound has completed playing.
     */

    BOOL WaitForPlayCompletion();
    /* Block the thread until the sound play begun with PlayBuffer() or
       PlayFile() has completed.

       <H2>Returns:</H2>
       TRUE if the sound has successfully completed playing.
     */


    BOOL StartRecording();
    /* Start filling record buffers. The first call to Read() will also
       initiate the recording.

       <H2>Returns:</H2>
       TRUE if the sound driver has successfully started recording.
     */

    BOOL IsRecordBufferFull();
    /* Determine if a record buffer has been filled, so that the next Read()
       call will not block. Provided that the amount of data read is less than
       the buffer size.

       <H2>Returns:</H2>
       TRUE if the sound driver has filled a buffer.
     */

    BOOL AreAllRecordBuffersFull();
    /* Determine if all of the record buffer allocated has been filled. There
       is an implicit Abort() of the recording if this occurs and recording is
       stopped. The channel may need to be closed and opened again to start
       a new recording.

       <H2>Returns:</H2>
       TRUE if the sound driver has filled a buffer.
     */

    BOOL WaitForRecordBufferFull();
    /* Block the thread until a record buffer has been filled, so that the
       next Read() call will not block. Provided that the amount of data read
       is less than the buffer size.

       <H2>Returns:</H2>
       TRUE if the sound driver has filled a buffer.
     */

    BOOL WaitForAllRecordBuffersFull();
    /* Block the thread until all of the record buffer allocated has been
       filled. There is an implicit Abort() of the recording if this occurs
       and recording is stopped. The channel may need to be closed and opened
       again to start a new recording.

       <H2>Returns:</H2>
       TRUE if the sound driver has filled a buffer.
     */


    BOOL Abort();
    /* Abort the background playing/recording of the sound channel.

       <H2>Returns:</H2>
       TRUE if the sound has successfully been aborted.
     */


  private:
    void Construct();


// Class declaration continued in platform specific header file ///////////////
