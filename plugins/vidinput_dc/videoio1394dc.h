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
 * $Revision$
 * $Author$
 * $Date$
 */


#ifndef _PVIDEOIO1394DC

#define _PVIDEOIO1394DC

#ifdef __GNUC__
#pragma interface
#endif

#include <libraw1394/raw1394.h>
#include <libdc1394/dc1394_control.h>
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
    BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    BOOL IsOpen();

    /**Close the device.
      */
    BOOL Close();

    /**Start the video device I/O.
      */
    BOOL Start();

    /**Stop the video device I/O capture.
      */
    BOOL Stop();

    /**Determine if the video device I/O capture is in progress.
      */
    BOOL IsCapturing();

    /**Get a list of all of the drivers available.
      */
    static PStringList GetInputDeviceNames();

    PStringList GetDeviceNames() const
    { return GetInputDeviceNames(); }

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    PINDEX GetMaxFrameBytes();

    /**Grab a frame, after a delay as specified by the frame rate.
      */
    BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );

    /**Grab a frame. Do not delay according to the current frame rate parameter.
      */
    BOOL GetFrameDataNoDelay(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// OPtional bytes returned.
    );


    /**Get the brightness of the image. 0xffff-Very bright.
     */
    int GetBrightness();

    /**Set brightness of the image. 0xffff-Very bright.
     */
    BOOL SetBrightness(unsigned newBrightness);


    /**Get the whiteness of the image. 0xffff-Very white.
     */
    int GetWhiteness();

    /**Set whiteness of the image. 0xffff-Very white.
     */
    BOOL SetWhiteness(unsigned newWhiteness);


    /**Get the colour of the image. 0xffff-lots of colour.
     */
    int GetColour();

    /**Set colour of the image. 0xffff-lots of colour.
     */
    BOOL SetColour(unsigned newColour);


    /**Get the contrast of the image. 0xffff-High contrast.
     */
    int GetContrast();

    /**Set contrast of the image. 0xffff-High contrast.
     */
    BOOL SetContrast(unsigned newContrast);


    /**Get the hue of the image. 0xffff-High hue.
     */
    int GetHue();

    /**Set hue of the image. 0xffff-High hue.
     */
    BOOL SetHue(unsigned newHue);
    
    
    /**Return whiteness, brightness, colour, contrast and hue in one call.
     */
    BOOL GetParameters (int *whiteness, int *brightness, 
				int *colour, int *contrast, int *hue);

    /**Get the minimum & maximum size of a frame on the device.
    */
    BOOL GetFrameSizeLimits(
      unsigned & minWidth,   /// Variable to receive minimum width
      unsigned & minHeight,  /// Variable to receive minimum height
      unsigned & maxWidth,   /// Variable to receive maximum width
      unsigned & maxHeight   /// Variable to receive maximum height
    ) ;

    void ClearMapping();

    int GetNumChannels();
    BOOL SetChannel(
         int channelNumber  /// New channel number for device.
    );
    BOOL SetFrameRate(
      unsigned rate  /// Frames  per second
    );
    BOOL SetVideoFormat(
      VideoFormat videoFormat   /// New video format
    );
    BOOL SetFrameSize(
      unsigned width,   /// New width of frame
      unsigned height   /// New height of frame
    );
    BOOL SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );


    /**Try all known video formats & see which ones are accepted by the video driver
     */
    BOOL TestAllFormats();


 protected:
    PINDEX frameBytes;
    raw1394handle_t handle;
    BOOL is_capturing;
    BOOL UseDMA;
    nodeid_t * camera_nodes;
    int numCameras;
    dc1394_cameracapture camera;
    int capturing_duration;
    int supportedFormat;
    PAdaptiveDelay m_pacing;
#define DC1394_FORMAT_160x120	1
#define DC1394_FORMAT_320x240	2
};

#endif


// End Of File ///////////////////////////////////////////////////////////////
