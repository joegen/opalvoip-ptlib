/*
 * videoio1394dc.h
 *
 * Copyright:
 * Copyright (c) 2002 Ryutaroh Matsumoto <ryutaroh@rmatsumoto.org>
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
 * Classes to support streaming video input from IEEE 1394 cameras.
 * Detailed explanation can be found at src/ptlib/unix/video4dc1394.cxx
 *
 */


#ifndef _PVIDEOIO1394DC

#define _PVIDEOIO1394DC

#ifdef __GNUC__
#pragma interface
#endif

#include <dc1394/dc1394.h>
#include <ptclib/delaychan.h>

/** This class defines a video input device that
    generates fictitous image data.
*/
class PVideoInputDevice_1394DC : public PVideoInputDevice
{
    PCLASSINFO(PVideoInputDevice_1394DC, PVideoInputDevice);
 public:
  /** Create a new video input device.
   */
    PVideoInputDevice_1394DC();

    /**Close the video input device on destruction.
      */
    ~PVideoInputDevice_1394DC();

    /**Open the device given the device name.
      */
    PBoolean Open(
      const PString & deviceName,   /// Device name to open
      PBoolean startImmediate = true    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    PBoolean IsOpen();

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
    static PStringList GetInputDeviceNames();

    PStringArray GetDeviceNames() const
    { return GetInputDeviceNames(); }

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxFrameBytes();

    /**Get the minimum & maximum size of a frame on the device.
    */
    PBoolean GetFrameSizeLimits(
      unsigned & minWidth,   /// Variable to receive minimum width
      unsigned & minHeight,  /// Variable to receive minimum height
      unsigned & maxWidth,   /// Variable to receive maximum width
      unsigned & maxHeight   /// Variable to receive maximum height
    ) ;

    void ClearMapping();

    int GetNumChannels();
    PBoolean SetChannel(
         int channelNumber  /// New channel number for device.
    );
    PBoolean SetFrameRate(
      unsigned rate  /// Frames  per second
    );
    PBoolean SetVideoFormat(
      VideoFormat videoFormat   /// New video format
    );
    PBoolean SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );
    PBoolean SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );


    /**Try all known video formats & see which ones are accepted by the video driver
     */
    PBoolean TestAllFormats();


 protected:
    virtual bool InternalGetFrameData(BYTE * buffer, PINDEX & bytesReturned, bool & keyFrame, bool wait);

    PINDEX frameBytes;
    dc1394_t* handle;
    PBoolean is_capturing;
    PBoolean UseDMA;
    dc1394camera_list_t* camera_list;
    int numCameras;
    dc1394camera_t* camera;
    int capturing_duration;
    int supportedFormat;
    PAdaptiveDelay m_pacing;
#define DC1394_FORMAT_160x120	1
#define DC1394_FORMAT_320x240	2
};

#endif


// End Of File ///////////////////////////////////////////////////////////////
