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
 * Revision 1.5  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.4  2001/03/12 03:54:11  dereks
 * Make setting frame rate consistent with that for real video device.
 *
 * Revision 1.3  2001/03/03 05:06:31  robertj
 * Major upgrade of video conversion and grabbing classes.
 *
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
class PFakeVideoInputDevice : public PVideoInputDevice
{
    PCLASSINFO(PFakeVideoInputDevice,PVideoInputDevice);
 public:
  /** Create a new (fake) video input device.
   */
    PFakeVideoInputDevice();


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

    /**Set the video format to be used.

       Default behaviour sets the value of the videoFormat variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetVideoFormat(
      VideoFormat videoFormat   /// New video format
    );

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
    
    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns the IsOpen() status.
    */
    virtual BOOL SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );
    
    /**Set the video frame rate to be used on the device.

       Default behaviour sets the value of the frameRate variable and then
       return the IsOpen() status.
    */
    virtual BOOL SetFrameRate(
      unsigned rate  /// Frames per second
    );
         
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
         
    void ClearMapping() { return ; }
    
 protected:
   PINDEX videoFrameSize;
   int    msBetweenFrames; 
   int    grabCount;       
   PTimeInterval lastTick; 
};

#endif


// End Of File ///////////////////////////////////////////////////////////////
