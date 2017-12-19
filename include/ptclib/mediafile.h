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
#include <ptlib/videoio.h>
#include <ptclib/delaychan.h>


/**Abstract class for a file containing a audio/visual media.
  */
class PMediaFile : public PObject
{
  PCLASSINFO(PMediaFile, PObject);
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

    virtual bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length) = 0;
    virtual bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written) = 0;

#if P_VIDEO
    virtual bool ConfigureVideo(unsigned track, const PVideoFrameInfo & frameInfo) = 0;
    virtual bool ReadVideo(unsigned track, BYTE * data) = 0;
    virtual bool WriteVideo(unsigned track, const BYTE * data) = 0;
#endif
};


PFACTORY_LOAD(PMediaFile_WAV);


#if P_VIDEO

///////////////////////////////////////////////////////////////////////////////////////////
//
// This class defines a video capture (input) device that reads video from a raw YUV file
//

class PVideoInputDevice_MediaFile : public PVideoInputDevice
{
  PCLASSINFO(PVideoInputDevice_MediaFile, PVideoInputDevice);
  public:
    enum {
      Channel_PlayAndClose     = 0,
      Channel_PlayAndRepeat    = 1,
      Channel_PlayAndKeepLast  = 2,
      Channel_PlayAndShowBlack = 3,
      ChannelCount             = 4
    };

    /** Create a new file based video input device.
    */
    PVideoInputDevice_MediaFile();

    /** Destroy video input device.
    */
    virtual ~PVideoInputDevice_MediaFile();


    /**Open the device given the device name.
      */
    PBoolean Open(
      const PString & deviceName,   /// Device name to open
      PBoolean startImmediate = true    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    PBoolean IsOpen() ;

    /**Close the device.
      */
    PBoolean Close();

    /**Start the video device I/O.
      */
    PBoolean Start();

    /**Stop the video device I/O capture.
      */
    PBoolean Stop();

    /**Determine if the video device I/O capture is in progress.
      */
    PBoolean IsCapturing();

    /**Get a list of all of the drivers available.
      */
    static PStringArray GetInputDeviceNames();

    virtual PStringArray GetDeviceNames() const
      { return GetInputDeviceNames(); }

    /**Retrieve a list of Device Capabilities
      */
    static bool GetDeviceCapabilities(
      const PString & /*deviceName*/, ///< Name of device
      Capabilities * /*caps*/         ///< List of supported capabilities
    ) { return false; }

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();

    /**Grab a frame. 

       There will be a delay in returning, as specified by frame rate.
      */
    virtual PBoolean GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// Optional bytes returned.
    );

    /**Grab a frame.

       Do not delay according to the current frame rate.
      */
    virtual PBoolean GetFrameDataNoDelay(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );


    /**Set the video format to be used.

       Default behaviour sets the value of the videoFormat variable and then
       returns the IsOpen() status.
    */
    virtual PBoolean SetVideoFormat(
      VideoFormat videoFormat   /// New video format
    );

    /**Get the number of video channels available on the device.
        0 (default) = play file and close device
        1           = play file and repeat
        2           = play file and replay last frame
        3           = play file and display black frame

       Default behaviour returns 4.
    */
    virtual int GetNumChannels();

    /**Get the names of video channels available on the device.
    */
    virtual PStringArray GetChannelNames();

    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns the IsOpen() status.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );
    
    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       return the IsOpen() status.
    */
    virtual PBoolean SetFrameRate(
      unsigned rate  /// Frames per second
    );
         
    /**Get the minimum & maximum size of a frame on the device.

       Default behaviour returns the value 1 to UINT_MAX for both and returns
       false.
    */
    virtual PBoolean GetFrameSizeLimits(
      unsigned & minWidth,   /// Variable to receive minimum width
      unsigned & minHeight,  /// Variable to receive minimum height
      unsigned & maxWidth,   /// Variable to receive maximum width
      unsigned & maxHeight   /// Variable to receive maximum height
    ) ;

    /**Set the frame size to be used.

       Default behaviour sets the frameWidth and frameHeight variables and
       returns the IsOpen() status.
    */
    virtual PBoolean SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

   
 protected:
   PMediaFile   * m_file;
   PAdaptiveDelay m_pacing;
   unsigned       m_frameRateAdjust;
   unsigned       m_track;
};


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
