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
 * Revision 1.8  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.7  2000/11/09 00:20:38  robertj
 * Added qcif size constants
 *
 * Revision 1.6  2000/07/30 03:41:31  robertj
 * Added more colour formats to video device enum.
 *
 * Revision 1.5  2000/07/26 03:50:49  robertj
 * Added last error variable to video device.
 *
 * Revision 1.4  2000/07/26 02:13:46  robertj
 * Added some more "common" bounds checking to video device.
 *
 * Revision 1.3  2000/07/25 13:38:25  robertj
 * Added frame rate parameter to video frame grabber.
 *
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

class PVideoConvert; 


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

    /// Colour frame formats for video.
    enum ColourFormat {
      Grey,     /// Simple 8 bit linear grey scale
      Gray = Grey,
      RGB32,    /// 32 bit RGB
      RGB24,    /// 24 bit RGB
      RGB565,   /// 16 bit RGB (6 bit green)
      RGB555,   /// 15 bit RGB
      YUV422,   /// YUV 4:2:2 packed
      YUV422P,  /// YUV 4:2:2 planar
      YUV411,   /// YUV 4:1:1 packed
      YUV411P,  /// YUV 4:1:1 planar
      YUV420,   /// YUV 4:2:0 packed
      YUV420P,  /// YUV 4:2:0 planar
      YUV410,   /// YUV 4:1:0 packed
      YUV410P,  /// YUV 4:1:0 planar
      MJPEG,
      NumColourFormats
    };



    enum StandardSizes {
      CIFWidth = 352,
      CIFHeight = 288,
      QCIFWidth = 176,
      QCIFHeight = 144
    };

    /** Create a new video output device.
     */
    PVideoDevice(
      VideoFormat videoformat = Auto,
      int channelNumber = 0,
      ColourFormat colourFormat = RGB24
    );


    /**Open the device given the device name.
      */
    virtual BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    ) = 0;

    /**Determine if the device is currently open.
      */
    virtual BOOL IsOpen() = 0;

    /**Close the device.
      */
    virtual BOOL Close();

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
    virtual PStringList GetDeviceNames() const ;

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
    virtual int GetNumChannels() ;

    /**Set the video channel to be used on the device.

       Default behaviour sets the value of the channelNumber variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetChannel(
         int channelNumber  /// New channel number for device.
    );

    /**Get the video channel to be used on the device.

       Default behaviour returns the value of the channelNumber variable.
    */
    virtual int  GetChannel() const;

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

    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetFrameRate(
      unsigned rate  /// Frames per 100 seconds
    );

    /**Get the video frame rate used on the device.

       Default behaviour returns the value of the frameRate variable.
    */
    virtual unsigned GetFrameRate() const;

    /**Get the minimum & maximum size of a frame on the device.

       Default behaviour returns the value 1 to UINT_MAX for both and returns
       FALSE.
    */
    virtual BOOL GetFrameSizeLimits(
      unsigned & minWidth,   /// Variable to receive minimum width
      unsigned & minHeight,  /// Variable to receive minimum height
      unsigned & maxWidth,   /// Variable to receive maximum width
      unsigned & maxHeight   /// Variable to receive maximum height
    ) ;

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
    );

     /** Get the size of an image, given a particular width, height and colour format.
       */
    static unsigned CalcFrameSize( 
      unsigned width,
      unsigned height,
      int colourFormat
      );
   
    /** Get the width of the frame being used.

        Default  behaviour returns the value of the frameWidth variable
    */
    virtual unsigned GetFrameWidth() const
        { return frameWidth; }
    /** Get the height of the frame being used.

        Default  behaviour returns the value of the frameHeight variable
    */
    virtual unsigned GetFrameHeight() const
      { return frameHeight; }

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes() = 0;

    /**Get the last error code. This is a platform dependent number.
      */
    int GetLastError() const { return lastError; }


  protected:
    PString      deviceName;
    int          lastError;
    VideoFormat  videoFormat;
    int          channelNumber;
    ColourFormat colourFormat;
    unsigned     frameRate;
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
      VideoFormat  /*videoformat */   = PAL,
      int          /*channelNumber */ = 0 ,
      ColourFormat /*colourFormat */  = RGB24 
      ) {  };
    
    /**Close the video output device on destruction.
      */
    virtual ~PVideoOutputDevice() { Close(); };      

    /**Cause the referenced data to be drawn to the 
       previously defined media 
     */
    virtual BOOL Redraw(const void * /*frame*/ ) {	return FALSE; };

    /**Specifiy the width and height of the video stream.
     */
    virtual void SetFrameSize(int /*_width*/, int /*_height*/) 
	{   };

    /**Set the current time.
     */
    virtual void SetNow(int _now)  { now = _now; }

  protected:

    PINDEX  frameWidth;
    PINDEX  frameHeight;
    int now;
    BOOL suppress;

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
      VideoFormat videoFormat   = PAL,
      int channelNumber         = 0,
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
    virtual BOOL IsOpen() ;

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
    virtual PStringList GetDeviceNames() const ;

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

    
 protected:
    
    PVideoConvert * conversion; /// image grab format change. If NULL ptr, no change needed.
   
 public:

#ifdef DOC_PLUS_PLUS
};
#endif

// Class declaration continued in platform specific header file ///////////////
