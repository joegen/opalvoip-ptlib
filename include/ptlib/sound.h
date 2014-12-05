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
 * $Revision$
 * $Author$
 * $Date$
 */


#ifndef PTLIB_SOUND_H
#define PTLIB_SOUND_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib/plugin.h>
#include <ptlib/pluginmgr.h>

/** A class representing a sound. A sound is a highly platform dependent
   entity that is abstracted for use here. Very little manipulation of the
   sounds are possible.

   The most common sound to use is the static function <code>Beep()</code> which
   emits the system standard "warning" or "attention" sound.
 */
class PSound : public PBYTEArray
{
  PCLASSINFO(PSound, PBYTEArray);

  public:
  /**@name Construction */
  //@{
    /**Create a new sound, using the parameters provided.
       It is expected that the "lowest common denominator" encoding, linear PCM,
       is used.

       All other values for the encoding are platform dependent.
     */
    PSound(
      unsigned numChannels = 1,    ///< Number of channels eg mono/stereo
      unsigned sampleRate = 8000,  ///< Samples per second
      unsigned bitsPerSample = 16, ///< Number of bits per sample
      PINDEX   bufferSize = 0,     ///< Size of data
      const BYTE * data = NULL     ///< Pointer to initial data
    );

    /**Create a new sound, reading from a platform dependent file.
     */
    PSound(
      const PFilePath & filename   ///< Sound file to load.
    );

    /**Set new data bytes for the sound.
     */
    PSound & operator=(
      const PBYTEArray & data  ///< New data for sound
    );
  //@}

  /**@name File functions */
  //@{
    /**Load a platform dependent sound file (eg .WAV file for Win32) into the
       object. Note the whole file must able to be loaded into memory.

       Also note that not all possible files are playable by this library. No
       format conversions between file and driver are performed.

       @return
       true if the sound is loaded successfully.
     */
    PBoolean Load(
      const PFilePath & filename   ///< Sound file to load.
    );

    /**Save a platform dependent sound file (eg .WAV file for Win32) from the
       object.

       @return
       true if the sound is saved successfully.
     */
    PBoolean Save(
      const PFilePath & filename   ///< Sound file to load.
    );
  //@}

  /**@name Access functions */
  //@{
    /// Play the sound on the default sound device.
    PBoolean Play();

    /// Play the sound to the specified sound device.
    PBoolean Play(const PString & device);

    /**Set the internal sound format to linear PCM at the specification in
       the parameters.
     */
    void SetFormat(
      unsigned numChannels,   ///< Number of channels eg mono/stereo
      unsigned sampleRate,    ///< Samples per second
      unsigned bitsPerSample  ///< Number of bits per sample
    );

    /**Get the current encoding. A value of 0 indicates linear PCM, any other
       value is platform dependent.
     */
    unsigned GetEncoding()   const { return encoding; }

    /// Get  the number of channels (mono/stereo) in the sound.
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
    /**Play a sound file to the default device. If the <code>wait</code>
       parameter is true then the function does not return until the file has
       been played. If false then the sound play is begun asynchronously and
       the function returns immediately.

       @return
       true if the sound is playing or has played.
     */
    static PBoolean PlayFile(
      const PFilePath & file, ///< Sound file to play.
      PBoolean wait = true        ///< Flag to play sound synchronously.
    );

    /// Play the "standard" warning beep for the platform.
    static void Beep();

    /** Convert PCM data sample rates and channel depth.
        @ return true if all the input could be converted in the output buffer size.
      */
    static bool ConvertPCM(
      const short * srcPtr, ///< Source PCM data
      PINDEX & srcSize,     ///< In: number of bytes of source PCM, Out: bytes consumed
      unsigned srcRate,     ///< Sample rate for source PCM
      unsigned srcChannels, ///< Number of channels for source PCM
      short * dstPtr,       ///< Destination PCM data, may be same as srcPtr
      PINDEX & dstSize,     ///< In: size of destination buffer, Out: bytes written
      unsigned dstRate,     ///< Sample rate for destination PCM
      unsigned dstChannels  ///< Number of channels for destination PCM
    );
  //@}

  protected:
    /// Format code
    unsigned   encoding;      
    /// Number of channels eg mono/stereo
    unsigned   numChannels;   
    /// Samples per second
    unsigned   sampleRate;    
    /// Number of bits per sample
    unsigned   sampleSize;    
    /// Last error code for Load()/Save() functions
    DWORD      dwLastError;   
    /// Full info on the format (platform dependent)
    PBYTEArray formatInfo;    
};


/**
   Abstract class for a generalised sound channel, and an implementation of
   PSoundChannel for old code that is not plugin-aware.
   When instantiated, it selects the first plugin of the base class 
   "PSoundChannel"

   As an abstract class, this represents a sound channel. Drivers for real, 
   platform dependent sound hardware will be ancestors of this class and 
   can be found in the plugins section of PTLib.

   A sound channel is either playing or recording. If simultaneous
   playing and recording is desired, two instances of PSoundChannel
   must be created. It is an error for the same thread to attempt to
   both read and write audio data to once instance of a PSoundChannel
   class.

   PSoundChannel instances are designed to be reentrant. The actual
   usage model employed is left to the developer. One model could be
   where one thread is responsible for construction, setup, opening and 
   read/write operations. After creating and eventually opening the channel
   this thread is responsible for handling read/writes fast enough to avoid
   gaps in the generated audio stream.

   Remaining operations may beinvoked from other threads.
   This includes Close() and actually gathering the necessary data to
   be sent to the device.

   Besides the basic I/O task, the Read()/Write(() functions have well
   defined timing characteristics. When a PSoundChannel instance is
   used from Opal, the read/write operations are designed to also act
   as timers so as to nicely space the generated network packets of
   audio/ sound packets to the speaker.


   Read and Writes of audio data to a PSoundChannel are blocking. The
   length of time required to read/write a block of audio from/to a
   PSoundChannel instance is equal to the time required for that block
   of audio to record/play. So for a sound rate of 8khz, 240 samples,
   it is going to take 30ms to do a read/write.

   Since the Read()/Write(() functions have well defined
   timing characteristics; they are designed to also act as timers in a loop
   involving data transfers to/from the codecs.

   The sound is buffered and the size and number of buffers should be set
   before playing/recording. Each call to Write() will use one buffer, so care
   needs to be taken not to use a large number of small writes but tailor the
   buffers to the size of each write you make.

   Similarly for reading, an entire buffer must be read before any of it is
   available to a Read() call. Note that once a buffer is filled you can read
   it a byte at a time if desired, but as soon as all the data in the buffer
   is used, the next read will wait until the entire next buffer is
   read from the hardware. So again, tailor the number and size of buffers to
   the application. To avoid being blocked until the buffer fills, you can use
   the StartRecording() function to initiate the buffer filling, and the
   IsRecordingBufferFull() function to determine when the Read() function will
   no longer block.

   Note that this sound channel is implicitly a linear PCM channel. No data
   conversion is performed on data to/from the channel.

 */
class PSoundChannel : public PIndirectChannel
{
  PCLASSINFO(PSoundChannel, PIndirectChannel);

  public:
  /**@name Construction */
  //@{
    P_DECLARE_STREAMABLE_ENUM(Directions,
      Recorder,
      Player,
      Closed
    );

    /// Create a sound channel.
    PSoundChannel();

    enum {
      DefaultBufferSize = 320, // 20ms at 8kHz
      DefaultBufferCount = 2
    };

    /// Parameters for opening a sound channel
    struct Params {
      Params(
        Directions dir = Player,
        const PString & device = PString::Empty(),
        const PString & driver = PString::Empty(),
        unsigned channels = 1,
        unsigned sampleRate = 8000,
        unsigned bitsPerSample = 16,
        unsigned bufferSize =   DefaultBufferSize,
        unsigned bufferCount = DefaultBufferCount,
        PPluginManager * pluginMgr = NULL
      );

      Directions m_direction;     ///< Sound I/O direction
      PString    m_device;        ///< Name of sound device
      PString    m_driver;        ///< Name of sound driver
      unsigned   m_channels;      ///< Number of channels eg mono/stereo
      unsigned   m_sampleRate;    ///< Samples per second
      unsigned   m_bitsPerSample; ///< Number of bits per sample
      unsigned   m_bufferSize;    /**< Size of the internal buffer in bytes, typically
                                       the smallest amount of data channel can read or
                                       written in one go. */
      unsigned   m_bufferCount;   ///< Number of buffers to queue.
      PPluginManager * m_pluginMgr;
      
      void SetBufferCountFromMS(unsigned milliseconds);
      friend ostream & operator<<(ostream &, const Params & params);
    };

    /** Create a sound channel.
        Create a reference to the sound drivers for the platform.
      */
    PSoundChannel(
      const Params & params  ///< Parameters for opening channel
    );

    // Backward compatibility
    PSoundChannel(
      const PString & device,       ///< Name of sound driver/device
      Directions dir,               ///< Sound I/O direction
      unsigned numChannels = 1,     ///< Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   ///< Samples per second
      unsigned bitsPerSample = 16   ///< Number of bits per sample
    );
  //@}

  /**@name Open functions */
  //@{
    /**Get the list of available sound drivers (plug-ins)
     */
    static PStringArray GetDriverNames(
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Get sound devices that correspond to the specified driver name.
       If driverName is an empty string or the value "*" then GetAllDeviceNames()
       is used.
     */
    static PStringArray GetDriversDeviceNames(
      const PString & driverName,         ///< Name of driver
      Directions direction,               ///< Direction for device (record or play)
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    // For backward compatibility
    static inline PStringArray GetDeviceNames(
      const PString & driverName,
      Directions direction,
      PPluginManager * pluginMgr = NULL
    ) { return GetDriversDeviceNames(driverName, direction, pluginMgr); }

    /**Create the sound channel that corresponds to the specified driver name.
     */
    static PSoundChannel * CreateChannel (
      const PString & driverName,         ///< Name of driver
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /* Create the matching sound channel that corresponds to the device name.
       So, for "fake" return a device that will generate fake video.
       For "Phillips 680 webcam" (eg) will return appropriate grabber.
       Note that Phillips will return the appropriate grabber also.

       This is typically used with the return values from GetDeviceNames().
     */
    static PSoundChannel * CreateChannelByName(
      const PString & deviceName,         ///< Name of device
      Directions direction,               ///< Direction for device (record or play)
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Create an opened sound channel that corresponds to the specified names.
       If the driverName parameter is an empty string or "*" then CreateChannelByName
       is used with the deviceName parameter which is assumed to be a value returned
       from GetAllDeviceNames().
     */
    static PSoundChannel * CreateOpenedChannel(
      const Params & params   ///< Parameters for opening channel
    );

    // For backward compatibility
    static PSoundChannel * CreateOpenedChannel(
      const PString & driverName,         ///< Name of driver
      const PString & deviceName,         ///< Name of device
      Directions direction,               ///< Direction for device (record or play)
      unsigned numChannels = 1,           ///< Number of channels 1=mon, 2=stereo
      unsigned sampleRate = 8000,         ///< Sample rate
      unsigned bitsPerSample = 16,        ///< Bits per sample
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Get the name for the default sound devices/driver that is on this
       platform. Note that a named device may not necessarily do both
       playing and recording so the string returned with the <code>dir</code>
       parameter in each value is not necessarily the same.

       @return
       A platform dependent string for the sound player/recorder.
     */
    static PString GetDefaultDevice(
      Directions dir    // Sound I/O direction
    );

    /**Get the list of all devices name for the default sound devices/driver that is on this
       platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the <code>dir</code>
       parameter in each value is not necessarily the same.

       This will return a list of unique device names across all of the available
       drivers. If two drivers have identical names for devices, then the string
       returned will be of the form driver+'\\t'+device.

       @return
       Platform dependent strings for the sound player/recorder.
     */
    static PStringArray GetDeviceNames(
      Directions direction,               ///< Direction for device (record or play)
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Open the specified device for playing or recording. The device name is
       platform specific and is as returned in the GetDevices() function.

       @return
       true if the sound device is valid for playing/recording.
     */
    virtual bool Open(
      const Params & params   ///< Parameters for opening channel
    );

    // For backward compatibility
    bool Open(
      const PString & device,       ///< Name of sound driver/device
      Directions dir,               ///< Sound I/O direction
      unsigned numChannels = 1,     ///< Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   ///< Samples per second
      unsigned bitsPerSample = 16,  ///< Number of bits per sample
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /// Get the direction of the channel
    Directions GetDirection() const
    {
      return activeDirection;
    }

    /// Get text representing the direction of a channel
    static const char * GetDirectionText(Directions dir);

    /// Get text representing the direction of the channel
    virtual const char * GetDirectionText() const
    {
      return GetDirectionText(activeDirection);
    }

    /** Abort the background playing/recording of the sound channel.
        There will be a logic assertion if you attempt to Abort a
        sound channel operation, when the device is currently closed.

       @return
       true if the sound has successfully been aborted.
     */
    virtual PBoolean Abort();
  //@}

  /**@name Channel set up functions */
  //@{
    /**Set the format for play/record. Note that linear PCM data is the only
       one supported at this time.

       Note that if the PlayFile() function is used, this may be overridden
       by information in the file being played.

       @return
       true if the format is valid.
     */
    virtual PBoolean SetFormat(
      unsigned numChannels = 1,     ///< Number of channels eg mono/stereo
      unsigned sampleRate = 8000,   ///< Samples per second
      unsigned bitsPerSample = 16   ///< Number of bits per sample
    );

    /// Get  the number of channels (mono/stereo) in the sound.
    virtual unsigned GetChannels() const;

    /// Get the sample rate in samples per second.
    virtual unsigned GetSampleRate() const;

    /// Get the sample size in bits per sample.
    virtual unsigned GetSampleSize() const;

    /**Set the internal buffers for the sound channel I/O. 

       Note that with Linux OSS, the size is always rounded up to the nearest
       power of two, so 20000 => 32768. 

       @return
       true if the sound device is valid for playing/recording.
     */
    virtual PBoolean SetBuffers(
      PINDEX size,      ///< Size of each buffer in bytes
      PINDEX count = 2  ///< Number of buffers
    );

    /**Get the internal buffers for the sound channel I/O. 

       @return
       true if the buffer size were obtained.
     */
    virtual PBoolean GetBuffers(
      PINDEX & size,    // Size of each buffer in bytes
      PINDEX & count    // Number of buffers
    );

    enum {
      MaxVolume = 100
    };

    /**Set the volume of the play/read process.
       The volume range is 0 == muted, 100 == LOUDEST. The volume is a
       logarithmic scale mapped from the lowest gain possible on the device to
       the highest gain.
        
       @return
       true if there were no errors.
    */
    virtual PBoolean SetVolume(
      unsigned volume   ///< New volume level
    );

    /**Get the volume of the play/read process.
       The volume range is 0 == muted, 100 == LOUDEST. The volume is a
       logarithmic scale mapped from the lowest gain possible on the device to
       the highest gain.

       @return
       true if there were no errors.
    */
    virtual PBoolean GetVolume(
      unsigned & volume   ///< Variable to receive volume level.
    );

    /**Set the mute state of the play/read process.
        
       @return
       true if there were no errors.
    */
    virtual bool SetMute(
      bool mute   ///< New mute state
    );

    /**Get the mute state of the play/read process.

       @return
       true if there were no errors.
    */
    virtual bool GetMute(
      bool & mute   ///< Variable to receive mute state.
    );

  //@}

  /**@name Play functions */
  //@{
    /**Play a sound to the open device. If the <code>wait</code> parameter is
       true then the function does not return until the file has been played.
       If false then the sound play is begun asynchronously and the function
       returns immediately.

       Note:  if the driver is closed while playing the sound, the play 
       operation stops immediately.

       Also note that not all possible sounds and sound files are playable by
       this library. No format conversions between sound object and driver are
       performed.

       @return
       true if the sound is playing or has played.
     */

    virtual PBoolean PlaySound(
      const PSound & sound,   ///< Sound to play.
      PBoolean wait = true        ///< Flag to play sound synchronously.
    );

    /**Play a sound file to the open device. If the <code>wait</code>
       parameter is true then the function does not return until the file has
       been played. If false then the sound play is begun asynchronously and
       the function returns immediately.

       Note if the driver is closed of the object destroyed then the sound
       play is aborted.

       Also note that not all possible sounds and sound files are playable by
       this library. No format conversions between sound object and driver are
       performed.

       @return
       true if the sound is playing or has played.
     */
    virtual PBoolean PlayFile(
      const PFilePath & file, ///< Sound file to play.
      PBoolean wait = true        ///< Flag to play sound synchronously.
    );

    /**Indicate if the sound play begun with PlayBuffer() or PlayFile() has
       completed.

       @return
       true if the sound has completed playing.
     */
    virtual PBoolean HasPlayCompleted();

    /**Block calling thread until the sound play begun with PlaySound() or
       PlayFile() has completed. 

       @return
       true if the sound has successfully completed playing.
     */
    virtual PBoolean WaitForPlayCompletion();

    /**Test the specified device for playing.
       A series of tones are played to the channel and the result return as a
       string. The string will start with "Success" or "Error" with the former
       having some statistics on the performace of the palyer and the latter
       containing whatever error information is available.

       @return
       true if the sound device is valid for playing/recording.
     */
    static PString TestPlayer(
      const Params & params,        ///< Parameters for opening channel
      const PNotifier & progress = PNotifier(), ///< Call back for progress in playback
      const char * toneSpec = NULL  ///< Tones as used by PTones class.
    );
  //@}

  /**@name Record functions */
  //@{
    /**Record into the sound object all of the buffer's of sound data. Use the
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

       @return
       true if the sound has been recorded.
     */
    virtual PBoolean RecordSound(
      PSound & sound ///< Sound recorded
    );

    /**Record into the platform dependent sound file all of the buffer's of
       sound data. Use the SetBuffers() function to determine how long the
       recording will be made.

       Note that this function will block until all of the data is buffered.
       If you wish to do this asynchronously, use StartRecording() and
       AreAllrecordBuffersFull() to determine when you can call RecordSound()
       without blocking.

       @return
       true if the sound has been recorded.
     */
    virtual PBoolean RecordFile(
      const PFilePath & file ///< Sound file recorded
    );

    /**Start filling record buffers. The first call to Read() will also
       initiate the recording.

       @return
       true if the sound driver has successfully started recording.
     */
    virtual PBoolean StartRecording();

    /**Determine if a record buffer has been filled, so that the next Read()
       call will not block. Provided that the amount of data read is less than
       the buffer size.

       @return
       true if the sound driver has filled a buffer.
     */
    virtual PBoolean IsRecordBufferFull();

    /**Determine if all of the record buffer allocated has been filled. There
       is an implicit Abort() of the recording if this occurs and recording is
       stopped. The channel may need to be closed and opened again to start
       a new recording.

       @return
       true if the sound driver has filled a buffer.
     */
    virtual PBoolean AreAllRecordBuffersFull();

    /**Block the thread until a record buffer has been filled, so that the
       next Read() call will not block. Provided that the amount of data read
       is less than the buffer size.

       @return
       true if the sound driver has filled a buffer.
     */
    virtual PBoolean WaitForRecordBufferFull();

    /**Block the thread until all of the record buffer allocated has been
       filled. There is an implicit Abort() of the recording if this occurs
       and recording is stopped. The channel may need to be closed and opened
       again to start a new recording.

       @return
       true if the sound driver has filled a buffer.
     */
    virtual PBoolean WaitForAllRecordBuffersFull();

    /**Test the specified device for playing.
       A recording of a number of seconds of audio is made, and is played back
       using the second set of parameters.

       @return
       true if the sound device is valid for playing/recording.
     */
    static PString TestRecorder(
      const Params & recorderParams,  ///< Parameters for opening channel
      const Params & playerParams,    ///< Parameters for opening channel
      const PNotifier & progress = PNotifier(), ///< Call back for progress in playback
      unsigned seconds = 5            ///< Seconds to record
    );
  //@}

  protected:
    PSoundChannel * GetSoundChannel() const { return dynamic_cast<PSoundChannel *>(readChannel); }

    /**This is the direction that this sound channel is opened for use
       in.  Should the user attempt to used this opened class instance
       in a direction opposite to that specified in activeDirection,
       an assert happens. */
    Directions activeDirection;

    P_REMOVE_VIRTUAL(PBoolean, Open(const PString &,Directions,unsigned,unsigned,unsigned),false);
};


/////////////////////////////////////////////////////////////////////////

// define the sound plugin service descriptor

PCREATE_PLUGIN_DEVICE(PSoundChannel);

#define PCREATE_SOUND_PLUGIN_EX(name, InstanceClass, extra) \
   PCREATE_PLUGIN(name, PSoundChannel, InstanceClass, PPlugin_PSoundChannel, \
      virtual PStringArray GetDeviceNames(P_INT_PTR userData) const { return InstanceClass::GetDeviceNames((PSoundChannel::Directions)userData); } \
      extra)

#define PCREATE_SOUND_PLUGIN(name, InstanceClass) PCREATE_SOUND_PLUGIN_EX(name, InstanceClass, )


#define P_NULL_AUDIO_DEVICE "NullAudio"
PPLUGIN_STATIC_LOAD(NullAudio, PSoundChannel)

#ifdef _WIN32
  #define P_WINDOWS_MULTIMEDIA_DRIVER "WindowsMultimedia"
  PPLUGIN_STATIC_LOAD(WindowsMultimedia, PSoundChannel);
#elif defined(P_ANDROID)
  PPLUGIN_STATIC_LOAD(OpenSL_ES, PSoundChannel);
#elif defined(P_MACOSX) || defined(P_IOS)
  PPLUGIN_STATIC_LOAD(Apple, PSoundChannel);
#elif defined(__BEOS__)
  PPLUGIN_STATIC_LOAD(BeOS, PSoundChannel);
#endif

#if defined(P_DIRECTSOUND)
  #define P_DIRECT_SOUND_DRIVER "DirectSound"
  PPLUGIN_STATIC_LOAD(DirectSound, PSoundChannel);
#endif

#if defined(P_WAVFILE)
  #define P_WAV_FILE_DRIVER "WAVFile"
  PPLUGIN_STATIC_LOAD(WAVFile, PSoundChannel)
#endif

#if P_DTMF
  PPLUGIN_STATIC_LOAD(Tones, PSoundChannel)
#endif


#endif // PTLIB_SOUND_H


// End Of File ///////////////////////////////////////////////////////////////
