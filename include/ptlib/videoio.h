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
 * Contributor(s): Mark Cooke (mpc@star.sr.bham.ac.uk)
 *
 * $Log: videoio.h,v $
 * Revision 1.22  2002/01/14 02:59:54  robertj
 * Added preferred colour format selection, thanks Walter Whitlock
 *
 * Revision 1.21  2002/01/04 04:11:45  dereks
 * Add video flip code from Walter Whitlock, which flips code at the grabber.
 *
 * Revision 1.20  2001/11/28 00:07:32  dereks
 * Locking added to PVideoChannel, allowing reader/writer to be changed mid call
 * Enabled adjustment of the video frame rate
 * New fictitous image, a blank grey area
 *
 * Revision 1.19  2001/08/06 06:12:45  rogerh
 * Fix comments
 *
 * Revision 1.18  2001/08/03 04:21:51  dereks
 * Add colour/size conversion for YUV422->YUV411P
 * Add Get/Set Brightness,Contrast,Hue,Colour for PVideoDevice,  and
 * Linux PVideoInputDevice.
 * Add lots of PTRACE statement for debugging colour conversion.
 * Add support for Sony Vaio laptop under linux. Requires 2.4.7 kernel.
 *
 * Revision 1.17  2001/05/22 23:38:45  robertj
 * Fixed bug in PVideoOutputDevice, removed redundent SetFrameSize.
 *
 * Revision 1.16  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.15  2001/03/20 02:21:57  robertj
 * More enhancements from Mark Cooke
 *
 * Revision 1.14  2001/03/08 23:04:19  robertj
 * Fixed up some documentation.
 *
 * Revision 1.13  2001/03/08 08:31:34  robertj
 * Numerous enhancements to the video grabbing code including resizing
 *   infrastructure to converters. Thanks a LOT, Mark Cooke.
 *
 * Revision 1.12  2001/03/07 01:42:59  dereks
 * miscellaneous video fixes. Works on linux now. Add debug statements
 * (at PTRACE level of 1)
 *
 * Revision 1.11  2001/03/06 23:34:20  robertj
 * Added static function to get input device names.
 * Moved some inline virtuals to non-inline.
 *
 * Revision 1.10  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
 * Revision 1.9  2001/02/28 01:47:14  robertj
 * Removed function from ancestor and is not very useful, thanks Thorsten Westheider.
 *
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


class PColourConverter;


/**This class defines a video device.
   This class is used to abstract the few parameters that are common to both\
   input and output devices.

   Example colour formats are:

     "Grey"     Simple 8 bit linear grey scale
     "Gray"     Synonym for Grey
     "RGB32"    32 bit RGB
     "RGB24"    24 bit RGB
     "RGB565"   16 bit RGB (6 bit green)
     "RGB555"   15 bit RGB
     "YUV422"   YUV 4:2:2 packed
     "YUV422P"  YUV 4:2:2 planar
     "YUV411"   YUV 4:1:1 packed
     "YUV411P"  YUV 4:1:1 planar
     "YUV420"   YUV 4:2:0 packed
     "YUV420P"  YUV 4:2:0 planar
     "YUV410"   YUV 4:1:0 packed
     "YUV410P"  YUV 4:1:0 planar
     "MJPEG"    Motion JPEG
 */


class PVideoDevice : public PObject
{
  PCLASSINFO(PVideoDevice, PObject);

  protected:
    /** Create a new video device (input or output).
     */
    PVideoDevice();

    /** Delete structures created by PVideoDevice(); 
     */
    ~PVideoDevice();


  public:
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


    /**Get the device name of the open device.
      */
    const PString & GetDeviceName() const
      { return deviceName; }

    /**Get a list of all of the drivers available.
      */
    virtual PStringList GetDeviceNames() const = 0;


    enum VideoFormat {
      PAL,
      NTSC,
      SECAM,
      Auto,
      NumVideoFormats
    };

    /**Set the video format to be used.

       Default behaviour sets the value of the videoFormat variable and then
       returns TRUE.
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
    virtual int GetNumChannels();

    /**Set the video channel to be used on the device.
       The channel number is an integer from 0 to GetNumChannels()-1. The
       special value of -1 will find the first working channel number.

       Default behaviour sets the value of the channelNumber variable and then
       returns TRUE.
    */
    virtual BOOL SetChannel(
         int channelNumber  /// New channel number for device.
    );

    /**Get the video channel to be used on the device.

       Default behaviour returns the value of the channelNumber variable.
    */
    virtual int GetChannel() const;

    /**Set the colour format to be used, trying converters if available.

       This function will set the colour format on the device to one that
       is compatible with a registered converter, and install that converter
       so that the correct format is used.
    */
    virtual BOOL SetColourFormatConverter(
      const PString & colourFormat // New colour format for device.
    );

    /**Set the colour format to be used.
       Note that this function does not do any conversion. If it returns TRUE
       then the video device does the colour format in native mode.

       To utilise an internal converter use the SetColourFormatConverter()
       function.

       Default behaviour sets the value of the colourFormat variable and then
       returns TRUE.
    */
    virtual BOOL SetColourFormat(
      const PString & colourFormat // New colour format for device.
    );

    /**Get the colour format to be used.

       Default behaviour returns the value of the colourFormat variable.
    */
    const PString & GetColourFormat() const;

    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       returns TRUE.
    */
    virtual BOOL SetFrameRate(
      unsigned rate  /// Frames  per second
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


    enum StandardSizes {
      CIFWidth = 352,
      CIFHeight = 288,
      QCIFWidth = 176,
      QCIFHeight = 144
    };

    /**Set the frame size to be used, trying converters if available.

       If the device does not support the size, a set of alternate resolutions
       are attempted.  A converter is setup if possible.
    */
    virtual BOOL SetFrameSizeConverter(
      unsigned width,        /// New width of frame
      unsigned height,       /// New height of frame
      BOOL     bScaleNotCrop /// Scale or crop/pad preference
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       returns TRUE.
    */
    virtual BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );

    /**Get the frame size being used.

       Default behaviour returns the value of the frameWidth and frameHeight
       variable and returns TRUE.
    */
    virtual BOOL GetFrameSize(
      unsigned & width,
      unsigned & height
    );

    /** Get the width of the frame being used.

        Default  behaviour returns the value of the frameWidth variable
    */
    virtual unsigned GetFrameWidth() const;

    /** Get the height of the frame being used.

        Default  behaviour returns the value of the frameHeight variable
    */
    virtual unsigned GetFrameHeight() const;

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes() = 0;

    /** Get the number of bytes of an image, given a particular width, height and colour format.
      */
    static unsigned CalculateFrameBytes( 
      unsigned width,
      unsigned height,
      const PString & colourFormat
    );

    
    /**Get the last error code. This is a platform dependent number.
      */
    int GetLastError() const { return lastError; }


    /**Get the brightness of the image. 0xffff-Very bright.
     */
    virtual int GetBrightness() { return frameBrightness; }

    /**Set brightness of the image. 0xffff-Very bright.
     */
    virtual BOOL SetBrightness(unsigned newBrightness) 
      { frameBrightness=newBrightness; return TRUE;}


    /**Get the whiteness of the image. 0xffff-Very white.
     */
    virtual int GetWhiteness() { return frameWhiteness; }

    /**Set whiteness of the image. 0xffff-Very white.
     */
    virtual BOOL SetWhiteness(unsigned newWhiteness) 
      { frameWhiteness=newWhiteness; return TRUE;}


    /**Get the colour of the image. 0xffff-lots of colour.
     */
    virtual int GetColour() { return frameColour; }

    /**Set colour of the image. 0xffff-lots of colour.
     */
    virtual BOOL SetColour(unsigned newColour) 
      { frameColour=newColour; return TRUE; }


    /**Get the contrast of the image. 0xffff-High contrast.
     */
    virtual int GetContrast() { return frameContrast; }

    /**Set contrast of the image. 0xffff-High contrast.
     */
    virtual BOOL SetContrast(unsigned newContrast) 
      { frameContrast=newContrast; return TRUE; }


    /**Get the hue of the image. 0xffff-High hue.
     */
    virtual int GetHue() { return frameHue; }

    /**Set hue of the image. 0xffff-High hue.
     */
    virtual BOOL SetHue(unsigned newHue) 
      { frameHue=newHue; return TRUE; }
    
    
    /** Is the device a camera, and obtain video
     */
    virtual BOOL CanCaptureVideo(void)
      { return deviceCanCaptureVideo; }
 

    /**Return whiteness, brightness, colour, contrast and hue in one call.
     */
    virtual BOOL GetParameters (int *whiteness, int *brightness, 
				int *colour, int *contrast, int *hue);

    /**Set preferred native colour format from video capture device.
       Note empty == no preference.
     */
    void SetPreferredColourFormat(const PString & colourFmt) { preferredColourFormat = colourFmt; }

    /**Get preferred native colour format from video capture device.
       Returns empty == no preference
     */
    const PString & GetPreferredColourFormat() { return preferredColourFormat; }


  protected:
    /**Set variable which states this device can capture
       video. Default value for this variable is False.
    */
    virtual void SetCanCaptureVideo(BOOL newState)
      { deviceCanCaptureVideo = newState; }

    PString      deviceName;
    int          lastError;
    VideoFormat  videoFormat;
    int          channelNumber;
    PString      colourFormat;
    // Preferred native colour format from video input device, empty == no preference
    PString      preferredColourFormat;
    unsigned     frameRate;
    unsigned     frameWidth;
    unsigned     frameHeight;

    PColourConverter * converter;
 
    unsigned     frameBrightness;
    unsigned     frameWhiteness;
    unsigned     frameContrast;
    unsigned     frameColour;
    unsigned     frameHue;

    BOOL         deviceCanCaptureVideo; ///device can grab video from a port. (camera)
    
    
    PTime        previousFrameTime; // Time of the last frame.
    int          msBetweenFrames;// msBetween subsequent frames. 
    int          frameTimeError; // determines  when this frame should happen.
};


/**This class defines a video output device.
 */
class PVideoOutputDevice : public PVideoDevice
{
  PCLASSINFO(PVideoOutputDevice, PVideoDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice();
    
    /**Close the video output device on destruction.
      */
    virtual ~PVideoOutputDevice() { Close(); };      

    /**Cause the referenced data to be drawn to the 
       previously defined media 
     */
    virtual BOOL Redraw(
      const void * /*frame*/
    );

    /**Set the current time.
     */
    virtual void SetNow(
      int _now
    );


  protected:
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
    PVideoInputDevice();

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
    virtual BOOL IsOpen();

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

    /**Get a list of all of the drivers available.
      */
    static PStringList GetInputDeviceNames();

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

    /**Grab a frame, after a delay as specified by the frame rate.
      */
    virtual BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );

    /**Grab a frame. Do not delay according to the current frame rate parameter.
      */
    virtual BOOL GetFrameDataNoDelay(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );


    /**Get the video conversion vertical flip state
     */
    virtual BOOL GetVFlipState();

    /**Set the video conversion vertical flip state
     */
    virtual BOOL SetVFlipState(BOOL newVFlipState);

    /**Toggle the video conversion vertical flip state
     */
    virtual BOOL ToggleVFlipState();
        
    /**Try all known video formats & see which ones are accepted by the video driver
     */
    virtual BOOL TestAllFormats();


 protected:
    /**Check the hardware can do the asked for size.

       Note that not all cameras can provide all frame sizes.
     */
    virtual BOOL VerifyHardwareFrameSize(unsigned width, unsigned height);


// Include platform dependent part of class
#include <ptlib/videoio.h>
};


// End Of File ///////////////////////////////////////////////////////////////
