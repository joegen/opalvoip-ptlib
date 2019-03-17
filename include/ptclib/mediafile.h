/*
 * mediafile.h
 *
 * Media file declaration
 *
 * Portable Windows Library
 *
 * Copyright (C) 2017 Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tools Library.
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_PMEDIAFILE_H
#define PTLIB_PMEDIAFILE_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_MEDIAFILE

#include <ptlib/pfactory.h>
#include <ptlib/smartptr.h>
#include <ptlib/sound.h>
#include <ptlib/videoio.h>
#include <ptclib/delaychan.h>


/**Abstract class for a file containing a audio/visual media.
  */
class PMediaFile : public PSmartObject
{
    PCLASSINFO(PMediaFile, PSmartObject);
  protected:
    /** Create the media file abstraction.
      */
    PMediaFile();

    bool      m_reading;
    PFilePath m_filePath;
    PString   m_lastErrorText;

    bool CheckOpenAndTrack(unsigned track) const;
    bool CheckMode(bool reading) const;
    bool CheckOpenTrackAndMode(unsigned track, bool reading) const;
    bool CheckModeAndTrackType(bool reading, const PString & expectedTrackType, const PString & actualTrackType) const;

    bool SetErrorText(const PString & error);

  public:
    ~PMediaFile();

    typedef PFactory<PMediaFile, PFilePathString> Factory;

    typedef PSmartPtr<PMediaFile> Ptr;

    /** Create a concreate class for the media file, given it's file extension.
        Returns: NULL if the file type is not supported.
      */
    static PMediaFile * Create(
      const PFilePath & file      ///< File to create and instance from.
    ) { return Factory::CreateInstance(file.GetType()); }

    /** Get all of the supported file types across all concrete classes in factory.
      */
    static PStringSet GetAllFileTypes();

    /// Get string representing an audio track
    static const PString & Audio();

    /// Get a string representing a video track
    static const PString & Video();

    /**Indicate this media file supports the specified media format.
       For example, a WAV file would return false for Video().
      */
    virtual bool IsSupported(
      const PString & format    ///< Track media format name
    ) const = 0;

    /** Open the media file for reading.
      */
    virtual bool OpenForReading(
      const PFilePath & filePath    ///< File to open for reading
    ) = 0;

    /** Open the media file for writing.
        Note, this will always overwrite the existing file.
    */
    virtual bool OpenForWriting(
      const PFilePath & filePath    ///< FIle to open for writing
    ) = 0;

    /** Indicate the media file is open for reading/writing.
      */
    virtual bool IsOpen() const = 0;

    /** Close the media file.
      */
    virtual bool Close() = 0;

    /** Indicate the is a read only or write only file.
      */
    bool IsReading() const { return m_reading; }

    /** Get the name of the media file that is currently open for reading/writing.
      */
    const PFilePath & GetFilePath() const { return m_filePath; }

    /** Get the error message for the last failure.
      */
    const PString & GetErrorText() const { return m_lastErrorText; }

    /// Information about a media track
    struct TrackInfo
    {
      TrackInfo(const PString & type = PString::Empty(), const PString & format = PString::Empty());
      TrackInfo(unsigned rate, unsigned channels); // Audio track
#if P_VIDEO
      TrackInfo(unsigned width, unsigned height, double rate); // Video track
#endif

      bool operator==(const TrackInfo & other) const;
      bool operator!=(const TrackInfo & other) const { return !operator==(other); }

      PCaselessString m_type;            ///< Type of media, e.g. "audio"/"video"
      PString         m_format;          ///< Format of media, e.g. "PCM-16"
      unsigned        m_size;            ///< Size in bytes of a "frame" for a native read/write
      int64_t         m_frames;          ///< Length of track in frames, -1 if not known
      double          m_rate;            ///< Sample rate for audio, frame rate for video
      unsigned        m_channels;        ///< Number of audio channels
      unsigned        m_width, m_height; ///< Video resolution
      PStringOptions  m_options;         ///< Format specific options
    };
    typedef std::vector<TrackInfo> TracksInfo;

    /** Get the default track info for the media type.
        This returns a track info that is guaranteed to be able to be written to
        the specific media file container. e.g. for WAV files, "PCM-16", mono,
        16kHz is returned.
      */
    virtual bool GetDefaultTrackInfo(
      const PCaselessString & type,   ///< Media type
      TrackInfo & info                ///< Default track info
    ) const = 0;

    /** Get the current count of tracks in the media file.
        For reading, this is fixed. For writing this may be added to with SetTracks().
      */
    virtual unsigned GetTrackCount() const = 0;

    /** Get information on the current tracks in the file.
        For reading, this is fixed. For writing this may be added to with SetTracks().
    */
    virtual bool GetTracks(
      TracksInfo & tracks   ///< Vector to receive track info
    ) = 0;

    /** Set information on the current tracks in the file.
        This is not allowed for reading, and for writing, you can only add new tracks
        to the end of the list returned from GetTracks().
      */
    virtual bool SetTracks(
      const TracksInfo & tracks   ///< Vector of track info to create
    ) = 0;

    /** Read from the media file track in native format.
        This only applies to a file in reading mode.
      */
    virtual bool ReadNative(
      unsigned track,   ///< Track to read
      BYTE * data,      ///< Pointer to memory to receive the native data
      PINDEX & size,    ///< On input maxcimum size of data, on output the actual size of the data
      unsigned & frames ///< The number of frames, if relevant to the track format
    ) = 0;

    /** Write to the media file track in native format.
       This only applies to a file in write mode.
    */
    virtual bool WriteNative(
      unsigned track,     ///< Track to write
      const BYTE * data,  ///< Data to write to track
      PINDEX & size,      ///< Size of data to write, on output the amount actually written
      unsigned & frames ///< The number of frames, if relevant to the track format
    ) = 0;

    /** Configure the raw PCM-16 audio format to be used.
        This is the format that is used in ReadAudio()/WriteAudio(), the actual format in the
        file, which is determined via the TrackInfo fields, is decoded/encoded as required.
      */
    virtual bool ConfigureAudio(
      unsigned track,     ///< Track to configure
      unsigned channels,  ///< Number of channels of audio
      unsigned sampleRate ///< Sample rate for the audio
    ) = 0;

    /** Read the audio from the media file as PCM-16.
        The number of channels and sample rate are controlled by ConfigureAudio()
        This function will fail if the track is not an audio track.
      */
    virtual bool ReadAudio(
      unsigned track,   ///< Track to read audio
      BYTE * data,      ///< Buffer to receive PCM-16 data
      PINDEX size,      ///< Size of the buffer to receive data
      PINDEX & length   ///< Actual number of bytes written to buffer
    ) = 0;

    /** Write the audio to the media file as PCM-16.
        The number of channels and sample rate are controlled by ConfigureAudio()
        This function will fail if the track is not an audio track.
      */
    virtual bool WriteAudio(
      unsigned track,     ///< Track to write audio
      const BYTE * data,  ///< PCM-16 data to write.
      PINDEX length,      ///< Number of bytes of PCM-16 data to write
      PINDEX & written    ///< Number of bytes of PCM-16 data actually written
    ) = 0;

#if P_VIDEO
    /** Configure the raw video format to be used.
        This is the format that is used in ReadVideo()/WriteVideo(), the actual format in the
        file, which is determined via the TrackInfo fields, is decoded/encoded as required.
        The frameInfo would typically use "YUV420P" but "RGB24" or "RGB32" is also a common
        output. The width/height can be set and the media file resolution will be scaled
        accordingly.
    */
    virtual bool ConfigureVideo(
      unsigned track,                   ///< Track to configure for video
      const PVideoFrameInfo & frameInfo ///< Frame info (width/height/format) to use.
    ) = 0;

    /** Read one video frame from the media file.
        Note the size of the buffer pointed to by data is fixed by the PVideoFrameInfo
        in the ConfigureVideo() function.
      */
    virtual bool ReadVideo(
      unsigned track,   ///< Track to read video
      BYTE * data       ///< Buffer to receive the video frame
    ) = 0;

    /** Write one video frame to the media file.
        Note the size of the buffer pointed to by data is fixed by the PVideoFrameInfo
        in the ConfigureVideo() function.
    */
    virtual bool WriteVideo(
      unsigned track,     ///< Track to write video
      const BYTE * data   ///< Video frame to write to track
    ) = 0;
#endif


    class SoundChannel : public PSoundChannelEmulation
    {
      PCLASSINFO(SoundChannel, PSoundChannelEmulation);
      public:
        explicit SoundChannel(const Ptr & mediaFile = Ptr(), unsigned track = 0);
        ~SoundChannel();

        virtual bool Open(const Params & params);
        virtual PString GetName() const;
        virtual PBoolean Close();
        virtual PBoolean IsOpen() const;

      protected:
        virtual bool RawWrite(const void * buf, PINDEX len);
        virtual bool RawRead(void * buf, PINDEX len);
        virtual bool Rewind();

        Ptr      m_mediaFile;
        unsigned m_track;
    };

#if P_VIDEO
    class VideoInputDevice : public PVideoInputEmulatedDevice
    {
      PCLASSINFO(VideoInputDevice, PVideoInputEmulatedDevice);
      public:
        explicit VideoInputDevice(const Ptr & mediaFile = Ptr(), unsigned track = 0);
        ~VideoInputDevice();

        virtual PStringArray GetDeviceNames() const;
        virtual PBoolean Open(const PString & deviceName, PBoolean startImmediate = true);
        virtual PBoolean IsOpen();
        virtual PBoolean Close();

      protected:
        virtual bool InternalReadFrameData(BYTE * frame);

        Ptr            m_mediaFile;
        unsigned       m_track;
    };
#endif // P_VIDEO
};


PFACTORY_LOAD(PMediaFile_WAV);


#if P_VIDEO

///////////////////////////////////////////////////////////////////////////////////////////
//
// This class defines a video display (output) device that writes video to a raw YUV file
//

class PVideoOutputDevice_MediaFile : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDevice_MediaFile, PVideoOutputDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice_MediaFile();

    /** Destroy video output device.
     */
    virtual ~PVideoOutputDevice_MediaFile();

    /**Get a list of all of the drivers available.
      */
    static PStringArray GetOutputDeviceNames();

    virtual PStringArray GetDeviceNames() const
      { return GetOutputDeviceNames(); }

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & deviceName,   /// Device name to open
      PBoolean startImmediate = true    /// Immediately start device
    );

    /**Start the video device I/O.
      */
    PBoolean Start();

    /**Stop the video device I/O capture.
      */
    PBoolean Stop();

    /**Close the device.
      */
    virtual PBoolean Close();

    /**Determine if the device is currently open.
      */
    virtual PBoolean IsOpen();

    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns the IsOpen() status.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );
    
    /**Set a section of the output frame buffer.
      */
    virtual PBoolean SetFrameData(const FrameData & frameData);

  protected:  
   PMediaFile * m_file;
};

#endif // P_VIDEO

#endif // P_MEDIAFILE

#endif // PTLIB_PMEDIAFILE_H


// End Of File ///////////////////////////////////////////////////////////////
