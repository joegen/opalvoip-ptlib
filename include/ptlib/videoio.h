/*
 * videoio.h
 *
 * Classes to support streaming video input (grabbing) and output.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-2000 Equivalence Pty. Ltd.
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
 * Contributor(s): ______________________________________.
 *
 * $Log: videoio.h,v $
 * Revision 1.2  2000/07/25 13:14:05  robertj
 * Got the video capture stuff going!
 *
 * Revision 1.1  2000/07/15 09:47:34  robertj
 * Added video I/O device classes.
 *
 */


#define _PVIDEOIO

#ifdef __GNUC__
#pragma interface
#endif



/**This class defines a video device.
   This class is used to abstract the few parameters that are common to both\
   input and output devices.
 */
class PVideoDevice : public PObject
{
  PCLASSINFO(PVideoDevice, PObject);

  public:
    enum VideoFormat {
      PAL,
      NTSC,
      SECAM,
      Auto,
      NumVideoFormats
    };

    enum ColourFormat {
      Grey,
      Gray = Grey,
      RGB24,
      RGB32,
      YUV422,
      RGB565,
      MJPEG,
      NumColourFormats
    };

    enum StandardSizes {
      CIF_WIDTH = 352,
      CIF_HEIGHT = 288
    };

    /** Create a new video output device.
     */
    PVideoDevice(
      VideoFormat videoformat = PAL,
      unsigned channelNumber = 0,
      ColourFormat colourFormat = RGB24
    );


    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    ) = 0;

    /**Determine of the device is currently open.
      */
    virtual BOOL IsOpen() const = 0;

    /**Close the device.
      */
    virtual BOOL Close() = 0;

    /**Start the video device I/O capture.
      */
    virtual BOOL Start() = 0;

    /**Stop the video device I/O capture.
      */
    virtual BOOL Stop() = 0;

    /**Determine if the video device I/O capture is in progress.
      */
    virtual BOOL IsCapturing() = 0;


    /**Get the device name of the open device.
      */
    const PString & GetDeviceName() const
      { return deviceName; }

    /**Get a list of all of the drivers available.
      */
    virtual PStringList GetDeviceNames() const = 0;

    /**Set the video format to be used.

       Default behaviour sets the value of the videoFormat variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetVideoFormat(
      VideoFormat videoFormat   /// New video format
    );

    /**Get the video format being used.

       Default behaviour returns the value of the videoFormat variable.
    */
    virtual VideoFormat GetVideoFormat() const;

    /**Get the number of video channels available on the device.

       Default behaviour returns 1.
    */
    virtual unsigned GetNumChannels() const;

    /**Set the video channel to be used on the device.

       Default behaviour sets the value of the channelNumber variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetChannel(
      unsigned channelNumber  /// New channel number for device.
    );

    /**Get the video channel to be used on the device.

       Default behaviour returns the value of the channelNumber variable.
    */
    virtual unsigned GetChannel() const;

    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetColourFormat(
      ColourFormat colourFormat   // New colour format for device.
    );

    /**Get the colour format to be used.

       Default behaviour returns the value of the colourFormat variable.
    */
    ColourFormat GetColourFormat() const;

    /**Get the maximum size of a frame on the device.

       Default behaviour returns the value UINT_MAX for both and returns
       FALSE.
    */
    virtual BOOL GetMaxFrameSize(
      unsigned & width,   /// Variable to receive width
      unsigned & height   /// Variable to receive height
    ) const;

    /**Get the minimum size of a frame on the device.

       Default behaviour returns the value 1 for both and returns FALSE.
    */
    virtual BOOL GetMinFrameSize(
      unsigned & width,   /// Variable to receive width
      unsigned & height   /// Variable to receive height
    ) const;

    /**Set the frame size to be used.

       Default behaviour sets the frameWidth and frameHeight variables and
       returns the IsOpen() status.
    */
    virtual BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Get the frame size being used.

       Default behaviour returns the value of the frameWidth and frameHeight
       variable and returns the IsOpen() status.
    */
    virtual BOOL GetFrameSize(
      unsigned & width,
      unsigned & height
    ) const;

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes() = 0;


  protected:
    PString      deviceName;
    VideoFormat  videoFormat;
    unsigned     channelNumber;
    ColourFormat colourFormat;
    unsigned     frameWidth;
    unsigned     frameHeight;
};


/**This class defines a video output device.
 */
class PVideoOutputDevice : public PVideoDevice
{
  PCLASSINFO(PVideoOutputDevice, PVideoDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice(
      VideoFormat videoformat = PAL,
      int channelNumber = 0,
      ColourFormat colourFormat = RGB24
    );
};


/**This class defines a video input device.
 */
class PVideoInputDevice : public PVideoDevice
{
  PCLASSINFO(PVideoInputDevice, PVideoDevice);

  public:
    /** Create a new video input device.
     */
    PVideoInputDevice(
      VideoFormat videoformat = PAL,
      unsigned channelNumber = 0,
      ColourFormat colourFormat = RGB24
    );

    /**Close the video input device on destruction.
      */
    ~PVideoInputDevice() { Close(); }


    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    virtual BOOL IsOpen() const;

    /**Close the device.
      */
    virtual BOOL Close();

    /**Start the video device I/O.
      */
    virtual BOOL Start();

    /**Stop the video device I/O capture.
      */
    virtual BOOL Stop();

    /**Determine if the video device I/O capture is in progress.
      */
    virtual BOOL IsCapturing();

    /**Get a list of all of the drivers available.
      */
    virtual PStringList GetDeviceNames() const;

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();


    /**Grab a frame.
      */
    virtual BOOL GetFrame(
      PBYTEArray & frame
    );

    /**Grab a frame.
      */
    virtual BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
