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
    typedef PFactory<PMediaFile, PFilePathString> Factory;

    typedef PSmartPtr<PMediaFile> Ptr;

    static PMediaFile * Create(const PFilePath & file) { return Factory::CreateInstance(file.GetType()); }
    static PStringSet GetAllFileTypes();

    static const PString & Audio();
    static const PString & Video();

    virtual bool IsSupported(const PString & type) const = 0;

    virtual bool OpenForReading(const PFilePath & filePath) = 0;
    virtual bool OpenForWriting(const PFilePath & filePath) = 0;

    virtual bool IsOpen() const = 0;
    virtual bool Close() = 0;

    bool IsReading() const { return m_reading; }
    const PFilePath & GetFilePath() const { return m_filePath; }
    const PString & GetErrorText() const { return m_lastErrorText; }

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

    virtual unsigned GetTrackCount() const = 0;
    virtual bool GetTracks(TracksInfo & tracks) = 0;
    virtual bool SetTracks(const TracksInfo & tracks) = 0;

    virtual bool ReadNative(unsigned track, BYTE * data, PINDEX & size, unsigned & frames) = 0;
    virtual bool WriteNative(unsigned track, const BYTE * data, PINDEX & size, unsigned & frames) = 0;

    virtual bool ConfigureAudio(unsigned track, unsigned channels, unsigned sampleRate) = 0;
    virtual bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length) = 0;
    virtual bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written) = 0;

#if P_VIDEO
    virtual bool ConfigureVideo(unsigned track, const PVideoFrameInfo & frameInfo) = 0;
    virtual bool ReadVideo(unsigned track, BYTE * data) = 0;
    virtual bool WriteVideo(unsigned track, const BYTE * data) = 0;
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
    class VideoInputDevice : public PVideoInputDevice
    {
      PCLASSINFO(VideoInputDevice, PVideoInputDevice);
      public:
        explicit VideoInputDevice(const Ptr & mediaFile = Ptr(), unsigned track = 0);
        ~VideoInputDevice();

        enum {
          Channel_PlayAndClose     = 0,
          Channel_PlayAndRepeat    = 1,
          Channel_PlayAndKeepLast  = 2,
          Channel_PlayAndShowBlack = 3,
          ChannelCount             = 4
        };

        virtual PStringArray GetDeviceNames() const;
        virtual PBoolean Open(const PString & deviceName, PBoolean startImmediate = true);
        virtual PBoolean IsOpen();
        virtual PBoolean Close();
        virtual PBoolean Start();
        virtual PBoolean Stop();
        virtual PBoolean IsCapturing();
        virtual PBoolean GetFrameData(BYTE * buffer, PINDEX * bytesReturned = NULL);
        virtual PBoolean GetFrameDataNoDelay(BYTE * buffer, PINDEX * bytesReturned = NULL);
        virtual PINDEX GetMaxFrameBytes();
        virtual int GetNumChannels();
        virtual PStringArray GetChannelNames();
        virtual PBoolean SetColourFormat(const PString & colourFormat);
        virtual PBoolean SetFrameRate(unsigned rate);
        virtual PBoolean GetFrameSizeLimits(unsigned & minWidth, unsigned & minHeight, unsigned & maxWidth, unsigned & maxHeight);
        virtual PBoolean SetFrameSize(unsigned width, unsigned height);

      protected:
        Ptr            m_mediaFile;
        unsigned       m_track;
        PAdaptiveDelay m_pacing;
        unsigned       m_frameRateAdjust;
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
    virtual PBoolean SetFrameData(
      unsigned x,
      unsigned y,
      unsigned width,
      unsigned height,
      const BYTE * data,
      PBoolean endFrame = true
    );

  protected:  
   PMediaFile * m_file;
};

#endif // P_VIDEO

#endif // P_MEDIAFILE

#endif // PTLIB_PMEDIAFILE_H


// End Of File ///////////////////////////////////////////////////////////////
