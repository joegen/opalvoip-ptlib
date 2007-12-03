/* vidinput_app.h
 *
 *
 * Application Input Implementation for the PTLib Library.
 *
 * Copyright (c) 2007 ISVO (Asia) Pte Ltd. All Rights Reserved.
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
 *
 *
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#include <ptbuildopts.h>

#ifdef P_APPSHARE

#define P_FORCE_STATIC_PLUGIN

#include <ptlib/plugin.h>
#include <ptclib/delaychan.h>

//////////////////////////////////////////////////////////////////////
// Video Input device

/**This class defines a video input device for Application sharing on Windows.
 */

class PVideoInputDevice_Application : public PVideoInputDevice
{
  PCLASSINFO(PVideoInputDevice_Application, PVideoInputDevice);

  public:
    /** Create a new video input device.
     */
    PVideoInputDevice_Application();

    /**Close the video input device on destruction.
      */
    ~PVideoInputDevice_Application() { Close(); }

    /** Is the device a camera, and obtain video
     */
    static PStringList GetInputDeviceNames();

    virtual PStringList GetDeviceNames() const
      { return GetInputDeviceNames(); }

    /**Retrieve a list of Device Capabilities
    */
    static PBoolean GetDeviceCapabilities(
      const PString & deviceName, ///< Name of device
      Capabilities * caps         ///< List of supported capabilities
    );

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & DeviceName,   ///< Device name to open
      PBoolean startImmediate = TRUE    ///< Immediately start device
    );

    /**Determine if the device is currently open.
      */
    virtual PBoolean IsOpen();

    /**Close the device.
      */
    virtual PBoolean Close();

    /**Start the video device I/O.
      */
    virtual PBoolean Start();

    /**Stop the video device I/O capture.
      */
    virtual PBoolean Stop();

    /**Determine if the video device I/O capture is in progress.
      */
    virtual PBoolean IsCapturing();

    /**Set the colour format to be used.
       Note that this function does not do any conversion. If it returns TRUE
       then the video device does the colour format in native mode.

       To utilise an internal converter use the SetColourFormatConverter()
       function.

       Default behaviour sets the value of the colourFormat variable and then
       returns TRUE.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat ///< New colour format for device.
    );

    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       returns TRUE.
    */
    virtual PBoolean SetFrameRate(
      unsigned rate  ///< Frames  per second
    );

    /**Set the frame size to be used.

       Note that devices may not be able to produce the requested size, and
       this function will fail.  See SetFrameSizeConverter().

       Default behaviour sets the frameWidth and frameHeight variables and
       returns TRUE.
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

    /**Grab a frame, after a delay as specified by the frame rate.
      */
    virtual PBoolean GetFrameData(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned         ///< OPtional bytes returned.
    );

    /**Grab a frame. Do not delay according to the current frame rate parameter.
      */
    virtual PBoolean GetFrameDataNoDelay(
      BYTE * buffer,                 ///< Buffer to receive frame
      PINDEX * bytesReturned         ///< OPtional bytes returned.
    );

    /**Try all known video formats & see which ones are accepted by the video driver
     */
    virtual PBoolean TestAllFormats();

    /**Set the video channel (not used)
      */
    virtual PBoolean SetChannel(int newChannel);

    /**Set the Handle of the window you wish to capture
      */
    void AttachCaptureWindow(
      HWND _hwnd,            ///< Handle of the window to capture
      bool _client = TRUE    ///< Only capture Client area and not caption
    );

  protected:
      HWND m_hWnd;               ///< Handle of Window to Capture
      bool m_client;             ///< Capture the client area only

      PMutex lastFrameMutex;     ///< Frame Grab Mutex
      PAdaptiveDelay grabDelay;  ///< Frame Grab delay
};


PWLIB_STATIC_LOAD_PLUGIN(Application, PVideoInputDevice);


#endif // P_APPSHARE
