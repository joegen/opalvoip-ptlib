/*
 * vfakeio.h
 *
 * Classes to generate fictitous video output and input.
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
 * Contributor(s): Derek J Smithies (derek@indranet.co.nz)
 *
 * $Log: vfakeio.h,v $
 * Revision 1.2  2000/12/19 23:58:14  robertj
 * Fixed MSVC compatibility issues.
 *
 * Revision 1.1  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 *
 */

#ifndef _PFAKEIO

#define _PFAKEIO

#ifdef __GNUC__
#pragma interface
#endif



/** This class defines a video input device that
    generates fictitous image data.
*/
class PFakeVideoInputDevice: public PVideoInputDevice
{
  
  PCLASSINFO(PFakeVideoInputDevice,PVideoInputDevice);

 public:
  /** Create a new (fake) video input device.
   */
    PFakeVideoInputDevice(
      VideoFormat videoFormat   = PVideoDevice::PAL,
      int channelNumber         = 0,
      ColourFormat colourFormat = RGB24
      );


    /**Open the device given the device name.
      */
    BOOL Open(
      const PString & deviceName,   /// Device name to open
      BOOL startImmediate = TRUE    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    BOOL IsOpen() ;

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
    PStringList GetDeviceNames() const ;

    /**Get the maximum frame size in bytes.

       Note a particular device may be able to provide variable length
       frames (eg motion JPEG) so will be the maximum size of all frames.
      */
    virtual PINDEX GetMaxFrameBytes();

    /**Grab a frame.
      */
    BOOL GetFrameData(
      BYTE * buffer,                 /// Buffer to receive frame
      PINDEX * bytesReturned = NULL  /// Optional bytes returned.
    );

    /**Called when this class has to generate a fictitous image 
     */
    void FillFrameWithData(BYTE * buffer);

    /**A test image that contains area of low and high resolution.
       The picture changes every second*/
    void GrabMovingBlocksTestFrame(BYTE *resFrame);
    
    /**a test image consisting of a horizontal line moving down the image, 
       with a constantly varying background. */
    void GrabMovingLineTestFrame(BYTE *resFrame);

    /**Generate a constant image, which contains the colours for
       a NTSC test frame.*/
    void GrabNTSCTestFrame(BYTE *resFrame);
        
    /**Generate three bouncing boxes, which bounce from a different height
      */
    void GrabBouncingBoxes(BYTE *resFrame);
    
    /** Fills a region of the image with a constant colour.
     */
    void FillRect(BYTE * frame,  unsigned width, unsigned height,
		          int x,         int y,
                  int rectWidth, int rectHeight,
                  int r,         int g,          int b);

    /** Given a preset interval of n milliseconds, this function
        returns n msecs after the previous frame capture was initiated.
	    */
    virtual void WaitFinishPreviousFrame();

    BOOL SetVideoFormat(VideoFormat newFormat);

    int GetNumChannels();

    BOOL SetChannel(int newChannel);
    
    BOOL SetColourFormat(ColourFormat newFormat);
    
    BOOL SetFrameRate(unsigned rate);
         
    BOOL GetFrameSizeLimits(unsigned & minWidth,
                            unsigned & minHeight,
                            unsigned & maxWidth,
                            unsigned & maxHeight) ;


    BOOL SetFrameSize(unsigned width, unsigned height);
         
         
    void ClearMapping() { return ; }
    
 protected:
   PINDEX videoFrameSize;
   int    msBetweenFrames; 
   int    grabCount;       
   PTimeInterval lastTick; 
};

#endif

////////////////////////////////////////////////////////
//End of file.
