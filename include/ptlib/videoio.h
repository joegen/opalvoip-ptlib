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
 */


#ifndef PTLIB_PVIDEOIO_H
#define PTLIB_PVIDEOIO_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif
#include <ptlib.h>

#if P_VIDEO

#include <ptlib/plugin.h>
#include <ptlib/pluginmgr.h>
#include <list>

class PColourConverter;


#define PTLIB_VIDEO_YUV420P "YUV420P"


class PVideoFrameInfo : public PObject
{
    PCLASSINFO(PVideoFrameInfo, PObject);
  public:
    P_DECLARE_ENUM_EX(ResizeMode,eMaxResizeMode,
      eScale,0,
      eCropCentre,
      eCropTopLeft,
      eScaleKeepAspect
    );
    friend ostream & operator<<(ostream & strm, ResizeMode mode);

    enum StandardSizes {
      SQCIFWidth = 128,  SQCIFHeight = 96,
      QCIFWidth  = 176,  QCIFHeight  = 144,
      CIFWidth   = 352,  CIFHeight   = 288,
      CIF4Width  = 704,  CIF4Height  = 576,
      CIF16Width = 1408, CIF16Height = 1152,
      HD480Width = 704,  HD480Height = 480,
      i480Width  = 704,  i480Height  = 480,
      HD720Width = 1280, HD720Height = 720,
      p720Width  = 1280, p720Height  = 720,
      HD1080Width= 1920, HD1080Height= 1080,
      i1080Width = 1920, i1080Height = 1080,
      HDTVWidth  = 1920, HDTVHeight  = 1080,
      MaxWidth   = 1920, MaxHeight   = 1200
    };

    static const PString & YUV420P();

    /// Construct video frame information
    PVideoFrameInfo();
    PVideoFrameInfo(
      unsigned        frameWidth,
      unsigned        frameHeight,
      const PString & colourFormat = PVideoFrameInfo::YUV420P(),
      unsigned        frameRate = 15,
      ResizeMode      resizeMode = eScale
    );

    /** Compare the two PVideoFrameInfo and return their relative rank.
        This ranking is by the relative area of the frame resolution, and
        frame rate if resolution equal. The final check for equality is on
        the colourFormat. The SAR and resize mode take no part.

       @return
       <code>LessThan</code>, <code>EqualTo</code> or <code>GreaterThan</code>
       according to the relative rank of the objects.
     */
    virtual Comparison Compare(
      const PObject & obj   // Object to compare against.
    ) const;

    /** Output the contents of the object to the stream. The exact output is
       dependent on the exact semantics of the descendent class. This is
       primarily used by the standard <code>#operator<<</code> function.

       The default behaviour is to print the class name.
     */
    virtual void PrintOn(
      ostream & strm   // Stream to print the object into.
    ) const;

    /**Set the frame size to be used.

       Default behaviour sets the frameWidth and frameHeight variables and
       returns true.
    */
    virtual PBoolean SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Get the frame size being used.

       Default behaviour returns the value of the frameWidth and frameHeight
       variable and returns true.
    */
    virtual PBoolean GetFrameSize(
      unsigned & width, ///< Current width of frame
      unsigned & height ///< Current height of frame
    ) const;

    /** Get the width of the frame being used.

        Default behaviour returns the value of the frameWidth variable
    */
    virtual unsigned GetFrameWidth() const;

    /** Get the height of the frame being used.

        Default behaviour returns the value of the frameHeight variable
    */
    virtual unsigned GetFrameHeight() const;

    /**Set the Storage Aspect Ratio size to be used.

       Default behaviour sets the sarWidth and sarHeight variables and
       returns true.
    */
    virtual PBoolean SetFrameSar(
      unsigned width,   ///< New SAR width of frame
      unsigned height   ///< New SAR height of frame
    );

     /**Get the Storage Aspect Ratio size being used.

       Default behaviour returns the value of the sarWidth and sarHeight
       variable and returns true.
    */
    virtual PBoolean GetSarSize(
      unsigned & width, ///< Current SAR width of frame
      unsigned & height ///< Current SAR height of frame
    ) const;

    /** Get the width of the Storage Aspect Ratio being used.

        Default behaviour returns the value of the sarWidth variable
    */
    virtual unsigned GetSarWidth() const;

    /** Get the height of the Storage Aspect Ratio being used.

        Default behaviour returns the value of the sarHeight variable
    */
    virtual unsigned GetSarHeight() const;
    
    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       returns true.
    */
    virtual PBoolean SetFrameRate(
      unsigned rate  ///< Frames  per second
    );

    /**Get the video frame rate used on the device.

       Default behaviour returns the value of the frameRate variable.
    */
    virtual unsigned GetFrameRate() const;

    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns true if not an empty string.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat // New colour format for device.
    );

    /**Get the colour format to be used.

       Default behaviour returns the value of the colourFormat variable.
    */
    virtual const PString & GetColourFormat() const;

    /**Set the resize mode to be used.
    */
    void SetResizeMode(
      ResizeMode mode
    ) { if (m_resizeMode < eMaxResizeMode) m_resizeMode = mode; }

    /**Get the resize mode to be used.
    */
    ResizeMode GetResizeMode() const { return m_resizeMode; }

    /** Get the number of bytes of an image, given a particular width, height and colour format.
      */
    PINDEX CalculateFrameBytes() const { return CalculateFrameBytes(m_frameWidth, m_frameHeight, m_colourFormat); }
    static PINDEX CalculateFrameBytes(
      unsigned width,               ///< Width of frame
      unsigned height,              ///< Height of frame
      const PString & colourFormat = PVideoFrameInfo::YUV420P()  ///< Colour format of frame
    );

    /** Parse a descriptor string for the video format.
        This is of the form [fmt ':' ] size [ '@' rate][ '/' crop ]. The size component
        is as for the ParseSize() function.

        The fmt string is the colour format such as "RGB32", "YUV420P" etc.

        The rate field is a simple integer from 1 to 100.

        The crop field is one of "scale", "resize" (synonym for "scale"),
        "centre", "center", "topleft" or "crop" (synonym for "topleft").

        Note no spaces are allowed in the descriptor.
      */
    bool Parse(
      const PString & str   ///< String to parse
    );

    /** Parse the standard size string names.
        This will parse a size desciption using either standard names: "qcif",
        "cif", "vga", "hd1080" etc or WxY form e.g. "640x480".
      */
    static bool ParseSize(
      const PString & str,  ///< String to parse
      unsigned & width,     ///< Resultant width
      unsigned & height     ///< Resulatant height
    );

    /**Get a width/height as a standard size string name.
      */
    static PString AsString(
      unsigned width,     ///< Width to convert
      unsigned height     ///< Height to convert
    );

    /**Get all "known" image size names.
       Returns all standard names for sizes, e.g. "qcif", "cif", "vga",
       "hd1080" etc.
      */
    static PStringArray GetSizeNames();

  protected:
    unsigned   m_frameWidth;
    unsigned   m_frameHeight;
    unsigned   m_sarWidth;
    unsigned   m_sarHeight;
    unsigned   m_frameRate;
    PString    m_colourFormat;
    ResizeMode m_resizeMode;
};


class PVideoControlInfo : public PObject
{
  PCLASSINFO(PVideoControlInfo, PObject);

 public:
    P_DECLARE_STREAMABLE_ENUM(Types,
      Pan,
      Tilt,
      Zoom,
      Focus
    );

    PVideoControlInfo(
      Types type = EndTypes,
      int minimum = 0,
      int maximum = 100,
      int step = 1,
      int reset = 0
    ) : m_type(type)
      , m_minimum(minimum)
      , m_maximum(maximum)
      , m_step(step)
      , m_reset(reset)
      , m_current(reset)
    { }

    virtual void PrintOn(ostream & strm) const;

    bool IsValid() const   { return m_type != EndTypes; }

    Types GetType() const  { return m_type; }
    int GetStep()    const { return m_step; }
    int Reset()            { return m_current = m_reset; }
    int GetCurrent() const { return m_current; }
    int SetCurrent(int current);

  protected:
    Types m_type;
    int   m_minimum;
    int   m_maximum;
    int   m_step;
    int   m_reset;
    int   m_current;
};


/**This class defines a video Input device Interactions (Remote Inputs/Controls)
*/
class PVideoInteractionInfo : public PObject
{
    PCLASSINFO(PVideoInteractionInfo, PObject);
  public:
    P_DECLARE_ENUM(Type,
      InteractKey,       /// Register remote KeyPresses
      InteractMouse,     /// Register remote Mouse Movement Clicks
      InteractNavigate,  /// Register remote Navigation commands
      InteractRTSP,      /// Register remote RTSP (Real Time Streaming Protocol) Inputs
      InteractOther      /// Register remote application specific Inputs
    );

    PVideoInteractionInfo();
    virtual void PrintOn(ostream & strm) const;
    static PString AsString(const Type & type);

    Type m_type;
};


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
     "YUV420P"  YUV 4:2:0 planar, use PTLIB_VIDEO_YUV420P or PVideoFrameInfo::YUV420P()
     "YUV410"   YUV 4:1:0 packed
     "YUV410P"  YUV 4:1:0 planar
     "MJPEG"    Motion JPEG
     "UYVY422"  YUV 4:2:2 packed as U Y V Y U Y V Y ...
     "UYV444"   YUV 4:4:4 packed as U Y V   U Y V   ...
                They are used in IEEE 1394 digital cameras. The specification
                is found at
http://www.1394ta.org/Download/Technology/Specifications/2000/IIDC_Spec_v1_30.pdf

 */
class PVideoDevice : public PVideoFrameInfo
{
  PCLASSINFO(PVideoDevice, PVideoFrameInfo);

  protected:
    /** Create a new video device (input or output).
     */
    PVideoDevice();


  public:
    /** Delete structures created by PVideoDevice(); 
     */
    virtual ~PVideoDevice();

    P_DECLARE_STREAMABLE_ENUM(VideoFormat,
      PAL,
      NTSC,
      SECAM,
      Auto
    );

    /**Get the device name of the open device.
      */
    const PString & GetDeviceName() const
      { return m_deviceName; }

    /**Get a list of all of the drivers available.
      */
    virtual PStringArray GetDeviceNames() const = 0;

    struct Attributes {
      Attributes();

      int m_brightness;
      int m_contrast;
      int m_saturation;
      int m_hue;
      int m_gamma;
      int m_exposure;
    };

    struct OpenArgs {
      OpenArgs();

      PPluginManager * pluginMgr;
      PString     driverName;
      PString     deviceName;
      VideoFormat videoFormat;
      int         channelNumber;
      PString     colourFormat;
      bool        convertFormat;
      unsigned    rate;
      unsigned    width;
      unsigned    height;
      bool        convertSize;
      ResizeMode  resizeMode;
      bool        flip;
      Attributes  m_attributes;

      template<class PVideoXxxDevice>
      bool Validate(PVideoDevice::OpenArgs & validated) const
      {
        // Check that the input device is legal
        PVideoXxxDevice * pDevice = PVideoXxxDevice::CreateDeviceByName(deviceName, driverName, pluginMgr);
        if (pDevice == NULL)
          return false;

        delete pDevice;
        validated = *this;
        return true;
      }
    };

    /**Open the device given the device name.
      */
    virtual PBoolean OpenFull(
      const OpenArgs & args,      ///< Parameters to set on opened device
      PBoolean startImmediate = true  ///< Immediately start device
    );

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & deviceName,   ///< Device name to open
      PBoolean startImmediate = true    ///< Immediately start device
    ) = 0;

    /**Determine if the device is currently open.
      */
    virtual PBoolean IsOpen() = 0;

    /**Close the device.
      */
    virtual PBoolean Close() = 0;

    /**Start the video device I/O capture.
      */
    virtual PBoolean Start() = 0;

    /**Stop the video device I/O capture.
      */
    virtual PBoolean Stop() = 0;


    /**Set the video format to be used.

       Default behaviour sets the value of the videoFormat variable and then
       returns true.
    */
    virtual PBoolean SetVideoFormat(
      VideoFormat videoFormat   ///< New video format
    );

    /**Get the video format being used.

       Default behaviour returns the value of the videoFormat variable.
    */
    virtual VideoFormat GetVideoFormat() const;

    /**Get the number of video channels available on the device.

       Default behaviour returns 1.
    */
    virtual int GetNumChannels();

    /**Get the names of video channels available on the device.
    */
    virtual PStringArray GetChannelNames();

    /**Set the video channel to be used on the device.
       The channel number is an integer from 0 to GetNumChannels()-1. The
       special value of -1 will find the first working channel number.

       Default behaviour sets the value of the channelNumber variable and then
       returns true.
    */
    virtual PBoolean SetChannel(
      int channelNumber  ///< New channel number for device.
    );

    /**Get the video channel to be used on the device.

       Default behaviour returns the value of the channelNumber variable.
    */
    virtual int GetChannel() const;

    /**Set the frame info to be used, trying converters if available.

       This function will simply call SetColourFormatConverter(),
       SetFrameSizeConverter() and SetFrameRate().
    */
    virtual bool SetFrameInfoConverter(
      const PVideoFrameInfo & info // New frame infofor device.
    );

    /**Set the colour format to be used, trying converters if available.

       This function will set the colour format on the device to one that
       is compatible with a registered converter, and install that converter
       so that the correct format is used.
    */
    virtual PBoolean SetColourFormatConverter(
      const PString & colourFormat // New colour format for device.
    );

    /**Get the video conversion vertical flip state.
       Default action is to return false.
     */
    virtual PBoolean GetVFlipState();

    /**Set the video conversion vertical flip state.
       Default action is to return false.
     */
    virtual PBoolean SetVFlipState(
      PBoolean newVFlipState    ///< New vertical flip state
    );

    /**Get the minimum & maximum size of a frame on the device.

       Default behaviour returns the value 1 to UINT_MAX for both and returns
       false.
    */
    virtual PBoolean GetFrameSizeLimits(
      unsigned & minWidth,   ///< Variable to receive minimum width
      unsigned & minHeight,  ///< Variable to receive minimum height
      unsigned & maxWidth,   ///< Variable to receive maximum width
      unsigned & maxHeight   ///< Variable to receive maximum height
    ) ;


    /**Set the frame size to be used, trying converters if available.

       If the device does not support the size, a set of alternate resolutions
       are attempted.  A converter is setup if possible.
    */
    virtual PBoolean SetFrameSizeConverter(
      unsigned width,  ///< New width of frame
      unsigned height, ///< New height of frame
      ResizeMode resizeMode = eMaxResizeMode ///< Mode to use if resizing is required.
    );

    /**Set the frame size to be used, trying converters if available.
       Function used for Backward compatibility only.
       If the device does not support the size, a set of alternate resolutions
       are attempted.  A converter is setup if possible.
    */
    virtual PBoolean SetFrameSizeConverter(
      unsigned width,                   ///< New width of frame
      unsigned height,                  ///< New height of frame
    PBoolean  /*bScaleNotCrop*/           ///< Not used.
    )  { return SetFrameSizeConverter(width,height,eScale); }


    /**Set the nearest available frame size to be used.

       Note that devices may not be able to produce the requested size, so
       this function picks the nearest available size.

       Default behaviour simply calls SetFrameSize().
    */
    virtual PBoolean SetNearestFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       then sets the converter sizes.
    */
    virtual PBoolean SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Get the frame size being used.
       If a converter exists, the destination frame size of the converter
       is returned, not the underlying physical device. If you wish the
       physical device frame size use PVideoFrameInfo::GetFrameSize().
    */
    virtual PBoolean GetFrameSize(
      unsigned & width,
      unsigned & height
    ) const;

    /**Get the colour format being used.
       If a converter exists, the destination colour format of the converter
       is returned, not the underlying physical device. If you wish the
       physical device colout format use PVideoFrameInfo::GetColourFormat().
      */
    virtual const PString& GetColourFormat() const;

    /**Get the maximum frame size in bytes.
       This gets the maximum of the physical device or the converted frame
       size and colour format. It is guranteed to be enought memory space for
       any video grab or output.

       Note, a particular device may be able to provide variable length
       frames (eg motion JPEG) so this will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes() = 0;

    
    /**Get the last error code. This is a platform dependent number.
      */
    int GetLastError() const { return m_lastError; }


    /** Is the device a camera, and obtain video
     */
    virtual PBoolean CanCaptureVideo() const = 0;

    /**Get video attributes.
     */
    virtual bool GetAttributes(
      Attributes & attributes
    );

    /**Set video attributes.
     */
    virtual bool SetAttributes(
      const Attributes & attributes
    );


    /** Set VideoFormat and VideoChannel in one ioctl
     */
    virtual PBoolean SetVideoChannelFormat (
      int channelNumber, 
      VideoFormat videoFormat
    );


    /**Set preferred native colour format from video capture device.
       Note empty == no preference.
     */
    void SetPreferredColourFormat(const PString & colourFmt) { m_preferredColourFormat = colourFmt; }

    /**Get preferred native colour format from video capture device.
       Returns empty == no preference
     */
    const PString & GetPreferredColourFormat() { return m_preferredColourFormat; }
    
  protected:
    PINDEX GetMaxFrameBytesConverted(PINDEX rawFrameBytes) const;
    PString GetDeviceNameFromOpenArgs(const OpenArgs & args) const;

    PCaselessString m_deviceName;
    int             m_lastError;
    VideoFormat     m_videoFormat;
    int             m_channelNumber;
    PString         m_preferredColourFormat; // Preferred native colour format from video input device, empty == no preference
    bool            m_nativeVerticalFlip;

    PColourConverter * m_converter;
    PBYTEArray         m_frameStore;

  private:
    P_REMOVE_VIRTUAL(int, GetBrightness(), 0);
    P_REMOVE_VIRTUAL(PBoolean, SetBrightness(unsigned), false);
    P_REMOVE_VIRTUAL(int, GetWhiteness(), 0);
    P_REMOVE_VIRTUAL(PBoolean, SetWhiteness(unsigned), false);
    P_REMOVE_VIRTUAL(int, GetColour(), 0);
    P_REMOVE_VIRTUAL(PBoolean, SetColour(unsigned), false);
    P_REMOVE_VIRTUAL(int, GetContrast(), 0);
    P_REMOVE_VIRTUAL(PBoolean, SetContrast(unsigned), false);
    P_REMOVE_VIRTUAL(int, GetHue(), 0);
    P_REMOVE_VIRTUAL(PBoolean, SetHue(unsigned), false);
    P_REMOVE_VIRTUAL(PBoolean, GetParameters(int *, int *, int *, int *, int *), false);
};


/**This class defines a video output device.- typically, a window.
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

    /**Get the list of available video output drivers (plug-ins)
    */
    static PStringArray GetDriverNames(
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Get video output devices that correspond to the specified driver name.
       If driverName is an empty string or the value "*" then this will return
       a list of unique device names across all of the available drivers. If
       two drivers have identical names for devices, then the string returned
       will be of the form driver+'\\t'+device.
    */
    static PStringArray GetDriversDeviceNames(
      const PString & driverName,         ///< Name of driver
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Create the video output device that corresponds to the specified driver name.
    */
    static PVideoOutputDevice * CreateDevice(
      const PString & driverName,         ///< Name of driver
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /* Create the matching video output device that corresponds to the device name.

       This is typically used with the return values from GetDriversDeviceNames().
     */
    static PVideoOutputDevice *CreateDeviceByName(
      const PString & deviceName,         ///< Name of device
      const PString & driverName = PString::Empty(),  ///< Name of driver (if any)
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Create an opened video output device that corresponds to the specified names.
       If the driverName parameter is an empty string or "*" then CreateDeviceByName
       is used with the deviceName parameter which is assumed to be a value returned
       from GetDriversDeviceNames().
    */
    static PVideoOutputDevice *CreateOpenedDevice(
      const PString & driverName,         ///< Name of driver
      const PString & deviceName,         ///< Name of device
      bool startImmediate = true,         ///< Immediately start display
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );
    static PVideoOutputDevice *CreateOpenedDevice(
      const PString & deviceName,         ///< Name of device
      bool startImmediate = true,         ///< Immediately start display
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    ) { return CreateOpenedDevice(PString::Empty(), deviceName, startImmediate, pluginMgr); }

    /**Create an opened video output device that corresponds to the specified arguments.
    */
    static PVideoOutputDevice *CreateOpenedDevice(
      const OpenArgs & args,              ///< Parameters to set on opened device
      bool startImmediate = true          ///< Immediately start display
    );

    /**Close the device.
      */
    virtual PBoolean Close() { return true; }

    /**Start the video device I/O display.
      */
    virtual PBoolean Start() { return true; }

    /**Stop the video device I/O display.
      */
    virtual PBoolean Stop() { return true; }

    /** Is the device a camera, and obtain video
     */
    virtual PBoolean CanCaptureVideo() const;

    /**Set a section of the output frame buffer.
      */
    virtual PBoolean SetFrameData(
      unsigned x,               ///< Horizontal position in frame where data is put
      unsigned y,               ///< Vertical position in frame where data is put
      unsigned width,           ///< Width of area in frame where data is put
      unsigned height,          ///< Height of area in frame where data is put
      const BYTE * data,        ///< Data to put into the video frame store
      PBoolean endFrame = true  ///< Indicate no more data for this video frame
    ) = 0;
    virtual PBoolean SetFrameData(
      unsigned x,               ///< Horizontal position in frame where data is put
      unsigned y,               ///< Vertical position in frame where data is put
      unsigned width,           ///< Width of area in frame where data is put
      unsigned height,          ///< Height of area in frame where data is put
      const BYTE * data,        ///< Data to put into the video frame store
      PBoolean endFrame,        ///< Indicate no more data for this video frame
      bool & keyFrameNeeded     ///< Indicates bad video and a new key frame is required
    );
    virtual PBoolean SetFrameData(
      unsigned x,               ///< Horizontal position in frame where data is put
      unsigned y,               ///< Vertical position in frame where data is put
      unsigned width,           ///< Width of area in frame where data is put
      unsigned height,          ///< Height of area in frame where data is put
      unsigned sarWidth,        ///< Aspect ratio width of area in frame where data is put
      unsigned sarHeight,       ///< Aspect ratio height of area in frame where data is put
      const BYTE * data,        ///< Data to put into the video frame store
      PBoolean endFrame,        ///< Indicate no more data for this video frame
      bool & keyFrameNeeded,    ///< Indicates bad video and a new key frame is required
      const void * mark
    );

    /**Allow the outputdevice decide whether the 
        decoder should ignore decode hence not render
        any output. 

        Returns: false if to decode and render.
      */
    virtual PBoolean DisableDecode();

    /**Get the position of the output device, where relevant. For devices such as
       files, this always returns zeros. For devices such as Windows, this is the
       position of the window on the screen.
       
       Returns: TRUE if the position is available.
      */
    virtual PBoolean GetPosition(
      int & x,  // X position of device surface
      int & y   // Y position of device surface
    ) const;

    /**Set the position of the output device, where relevant. For devices such as
       files, this does nothing. For devices such as Windows, this sets the
       position of the window on the screen.
       
       Returns: TRUE if the position can be set.
      */
    virtual bool SetPosition(
      int x,  // X position of device surface
      int y   // Y position of device surface
    );
};


/**This class defines a video output device for RGB in a frame store.
 */
class PVideoOutputDeviceRGB : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDeviceRGB, PVideoOutputDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDeviceRGB();

    /**Set the colour format to be used.
       Note that this function does not do any conversion. If it returns true
       then the video device does the colour format in native mode.

       To utilise an internal converter use the SetColourFormatConverter()
       function.

       Default behaviour sets the value of the colourFormat variable and then
       returns true.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat // New colour format for device.
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       returns true.
    */
    virtual PBoolean SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();

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

    /**Indicate frame may be displayed.
      */
    virtual PBoolean FrameComplete() = 0;

  protected:
    PMutex     mutex;
    PINDEX     bytesPerPixel;
    PINDEX     scanLineWidth;
    bool       swappedRedAndBlue;
};


#ifdef SHOULD_BE_MOVED_TO_PLUGIN

/**This class defines a video output device which outputs to a series of PPM files.
 */
class PVideoOutputDevicePPM : public PVideoOutputDeviceRGB
{
  PCLASSINFO(PVideoOutputDevicePPM, PVideoOutputDeviceRGB);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevicePPM();

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & deviceName,   ///< Device name (filename base) to open
      PBoolean startImmediate = true    ///< Immediately start device
    );

    /**Determine if the device is currently open.
      */
    virtual PBoolean IsOpen();

    /**Close the device.
      */
    virtual PBoolean Close();

    /**Get a list of all of the drivers available.
      */
    virtual PStringArray GetDeviceNames() const;

    /**Indicate frame may be displayed.
      */
    virtual PBoolean EndFrame();

  protected:
    unsigned   frameNumber;
};

#endif // SHOULD_BE_MOVED_TO_PLUGIN


/**This class defines a video input device.
 */
class PVideoInputDevice : public PVideoDevice
{
  PCLASSINFO(PVideoInputDevice, PVideoDevice);

  public:
    /** Create a new video input device.
     */
    //PVideoInputDevice();

    /**Close the video input device on destruction.
      */
    ~PVideoInputDevice() { Close(); }

    /**Get the list of available video input drivers (plug-ins)
    */
    static PStringArray GetDriverNames(
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Get video input devices that correspond to the specified driver name.
       If driverName is an empty string or the value "*" then this will return
       a list of unique device names across all of the available drivers. If
       two drivers have identical names for devices, then the string returned
       will be of the form driver+'\\t'+device.
    */
    static PStringArray GetDriversDeviceNames(
      const PString & driverName,         ///< Name of driver
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Create the video input device that corresponds to the specified driver name.
    */
    static PVideoInputDevice *CreateDevice(
      const PString & driverName,         ///< Name of driver
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /* Create the matching video input device that corresponds to the device name.
       So, for "fake" return a device that will generate fake video.
       For "Phillips 680 webcam" (eg) will return appropriate grabber.
       Note that Phillips will return the appropriate grabber also.

       This is typically used with the return values from GetDriversDeviceNames().
     */
    static PVideoInputDevice *CreateDeviceByName(
      const PString & deviceName,         ///< Name of device
      const PString & driverName = PString::Empty(),  ///< Name of driver (if any)
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );

    /**Create an opened video input device that corresponds to the specified names.
       If the driverName parameter is an empty string or "*" then CreateDeviceByName
       is used with the deviceName parameter which is assumed to be a value returned
       from GetDriversDeviceNames().
    */
    static PVideoInputDevice *CreateOpenedDevice(
      const PString & driverName,         ///< Name of driver
      const PString & deviceName,         ///< Name of device
      bool startImmediate = true,         ///< Immediately start grabbing
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    );
    static PVideoInputDevice *CreateOpenedDevice(
      const PString & deviceName,         ///< Name of device
      bool startImmediate = true,         ///< Immediately start grabbing
      PPluginManager * pluginMgr = NULL   ///< Plug in manager, use default if NULL
    ) { return CreateOpenedDevice(PString::Empty(), deviceName, startImmediate, pluginMgr); }

    /**Create an opened video output device that corresponds to the specified arguments.
    */
    static PVideoInputDevice *CreateOpenedDevice(
      const OpenArgs & args,              ///< Parameters to set on opened device
      bool startImmediate = true          ///< Immediately start display
    );

    class Capabilities : public PObject {
      PCLASSINFO(Capabilities, PObject);
    public:
      Capabilities();
      virtual void PrintOn(ostream & strm) const;

      unsigned                         m_channels;
      bool                             m_brightness;
      bool                             m_contrast;
      bool                             m_saturation;
      bool                             m_hue;
      bool                             m_gamma;
      bool                             m_exposure;
      std::list<PVideoFrameInfo>       m_frameSizes;
      std::list<PVideoControlInfo>     m_controls;
      std::list<PVideoInteractionInfo> m_interactions;
    };

    /**Retrieve a list of Device Capabilities
      */
    virtual bool GetDeviceCapabilities(
      Capabilities * capabilities          ///< List of supported capabilities
      ) const;

    /**Retrieve a list of Device Capabilities for particular device
      */
    static bool GetDeviceCapabilities(
      const PString & deviceName,           ///< Name of device
      Capabilities * capabilities,          ///< List of supported capabilities
      PPluginManager * pluginMgr = NULL     ///< Plug in manager, use default if NULL
    );

    /**Retrieve a list of Device Capabilities for a particular driver
      */
    static bool GetDeviceCapabilities(
      const PString & deviceName,           ///< Name of device
      const PString & driverName,           ///< Device Driver
      Capabilities * caps,                  ///< List of supported capabilities
      PPluginManager * pluginMgr = NULL     ///< Plug in manager, use default if NULL
    );

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & deviceName,   ///< Device name to open
      PBoolean startImmediate = true    ///< Immediately start device
    ) = 0;

    virtual PBoolean Close(
    ) { return true; }

    /** Is the device a camera, and obtain video
     */
    virtual PBoolean CanCaptureVideo() const;
 
    /**Determine if the video device I/O capture is in progress.
      */
    virtual PBoolean IsCapturing() = 0;

    /**Set the nearest available frame size to be used.

       Note that devices may not be able to produce the requested size, so
       this function picks the nearest available size.

       Default behaviour simply calls SetFrameSize().
    */
    virtual PBoolean SetNearestFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    /**Grab a frame.
      */
    virtual PBoolean GetFrame(
      PBYTEArray & frame
    );

    /**Grab a frame, after a delay as specified by the frame rate.
      */
    virtual PBoolean GetFrameData(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned,        ///< Optional bytes returned.
      bool & keyFrame         /**< On input, forces generation of key frame,
                                   On return indicates key frame generated */
    );
    virtual PBoolean GetFrameData(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned = NULL  ///< Optional bytes returned.
    ) = 0;

    /**Grab a frame. Do not delay according to the current frame rate parameter.
      */
    virtual PBoolean GetFrameDataNoDelay(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned,       ///< Optional bytes returned.
      bool & keyFrame         /**< On input, forces generation of key frame,
                                   On return indicates key frame generated */
    );
    virtual PBoolean GetFrameDataNoDelay(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned = NULL  ///< Optional bytes returned.
    ) = 0;

    /**Pass data to the inputdevice for flowControl determination.
      */
    virtual bool FlowControl(const void * flowData);

    /**Set the capture modes for implementations that support them.
       For example with Video For Windows, this is used to select picture (0)
       or video (1) modes.

       In picture-mode the implementation requests a single frame from the
       connected camera device. The camera device then does nothing until the
       frame has been processed and the next is requested.

       In video-mode the camera continuously sends new frames.

       The default implementation does nothing but returns false.
      */
    virtual bool SetCaptureMode(unsigned mode);

    /**Returns the current capture mode. See SetCaptureMode() for more details.
       A return value of -1 indicates an error or the mode is not supported.
    */
    virtual int GetCaptureMode() const;

    enum ControlMode {
      AutomaticControl,
      AbsoluteControl,
      RelativeControl,
      ResetControl
    };

    virtual bool SetControl(PVideoControlInfo::Types type, int value, ControlMode mode);
    virtual const PVideoControlInfo & GetControlInfo(PVideoControlInfo::Types type) const { return m_controlInfo[type]; }

  protected:
    PVideoControlInfo m_controlInfo[PVideoControlInfo::NumTypes];

  private:
    P_REMOVE_VIRTUAL(PBoolean, GetFrameData(BYTE *, PINDEX *, unsigned &), false);
    P_REMOVE_VIRTUAL(PBoolean, GetFrameDataNoDelay(BYTE *, PINDEX *, unsigned &), false);
};


////////////////////////////////////////////////////////
//
// declare macros and structures needed for video input plugins
//

PCREATE_PLUGIN_DEVICE(PVideoInputDevice);

#define PCREATE_VIDINPUT_PLUGIN_EX(name, extra) \
    PCREATE_PLUGIN(name, PVideoInputDevice, PVideoInputDevice_##name, PPlugin_PVideoInputDevice, \
      virtual PStringArray GetDeviceNames(P_INT_PTR /*userData*/) const { return PVideoInputDevice_##name::GetInputDeviceNames(); } \
      virtual bool GetDeviceCapabilities(const PString & deviceName, void * caps) const { return PVideoInputDevice_##name::GetDeviceCapabilities(deviceName, (PVideoInputDevice::Capabilities *)caps); } \
      extra)

#define PCREATE_VIDINPUT_PLUGIN(name) PCREATE_VIDINPUT_PLUGIN_EX(name, )


#define P_FAKE_VIDEO_DRIVER         "FakeVideo"
#define P_FAKE_VIDEO_PREFIX         "Fake/"
#define P_FAKE_VIDEO_MOVING_BLOCKS  P_FAKE_VIDEO_PREFIX"MovingBlocks"
#define P_FAKE_VIDEO_MOVING_LINE    P_FAKE_VIDEO_PREFIX"MovingLine"
#define P_FAKE_VIDEO_BOUNCING_BOXES P_FAKE_VIDEO_PREFIX"BouncingBoxes"
#define P_FAKE_VIDEO_SOLID_COLOUR   P_FAKE_VIDEO_PREFIX"SolidColour"
#define P_FAKE_VIDEO_TEXT           P_FAKE_VIDEO_PREFIX"Text"
#define P_FAKE_VIDEO_NTSC           P_FAKE_VIDEO_PREFIX"NTSCTest"

PPLUGIN_STATIC_LOAD(FakeVideo, PVideoInputDevice);

#ifdef P_APPSHARE
  #define P_APPLICATION_VIDEO_DRIVER "Application"
  PPLUGIN_STATIC_LOAD(Application, PVideoInputDevice);
#endif

#if P_FFVDEV
  PPLUGIN_STATIC_LOAD(FFMPEG, PVideoInputDevice);
#endif

#if P_VIDFILE
  #define P_VIDEO_FILE_DRIVER "VideoFile"
  PPLUGIN_STATIC_LOAD(VideoFile, PVideoInputDevice);
#endif

#ifdef WIN32
  #define P_VIDEO_FOR_WINDOWS_DRIVER "VideoForWindows"
  PPLUGIN_STATIC_LOAD(VideoForWindows, PVideoInputDevice);
#endif

#ifdef P_DIRECTSHOW
  #define P_DIRECT_SHOW_DRIVER "DirectShow"
  PPLUGIN_STATIC_LOAD(DirectShow, PVideoInputDevice);
#endif

#if defined(P_MACOSX)
  PPLUGIN_STATIC_LOAD(Mac, PVideoInputDevice);
#endif


////////////////////////////////////////////////////////
//
// declare macros and structures needed for video output plugins
//

PCREATE_PLUGIN_DEVICE(PVideoOutputDevice);

#define PCREATE_VIDOUTPUT_PLUGIN_EX(name, extra) \
    PCREATE_PLUGIN(name, PVideoOutputDevice, PVideoOutputDevice_##name, PPlugin_PVideoOutputDevice, \
      virtual PStringArray GetDeviceNames(P_INT_PTR /*userData*/) const { return PVideoOutputDevice_##name::GetOutputDeviceNames(); } \
    extra)

#define PCREATE_VIDOUTPUT_PLUGIN(name) PCREATE_VIDOUTPUT_PLUGIN_EX(name,)

#define P_NULL_VIDEO_DRIVER "NULLOutput"
#define P_NULL_VIDEO_DEVICE "Null Video Out"
PPLUGIN_STATIC_LOAD(NULLOutput, PVideoOutputDevice);


#if P_VFW_CAPTURE
#if _WIN32
  #define P_MSWIN_VIDEO_DRIVER "Window"
  #define P_MSWIN_VIDEO_PREFIX "MSWIN"
  #define P_MSWIN_VIDEO_DEVICE(x,y,width,height) P_MSWIN_VIDEO_PREFIX " X=" x " Y=" y " WIDTH=" width " HEIGHT=" height
  PPLUGIN_STATIC_LOAD(Window, PVideoOutputDevice);
#endif
#endif

#if P_SDL
  #if defined(P_MACOSX) || defined(P_IOS)
    #include <SDL_main.h>
  #endif
  #define P_SDL_VIDEO_DRIVER "SDL"
  #define P_SDL_VIDEO_PREFIX "SDL"
  #define P_SDL_VIDEO_DEVICE(x,y,width,height) P_SDL_VIDEO_PREFIX " X=" x " Y=" y " WIDTH=" width " HEIGHT=" height
  PPLUGIN_STATIC_LOAD(SDL, PVideoOutputDevice);
#endif


////////////////////////////////////////////////////////
//
// declare classes needed for access to simple video font
//

class PVideoFont : public PObject
{
  PCLASSINFO(PVideoFont, PObject);
  public:
    enum {
      MAX_L_HEIGHT = 11
    };
    struct LetterData {
      char ascii;
      const char *line[MAX_L_HEIGHT];
    };

    static const LetterData * GetLetterData(char ascii);
};

#endif // P_VIDEO

#endif   // PTLIB_PVIDEOIO_H

// End Of File ///////////////////////////////////////////////////////////////

