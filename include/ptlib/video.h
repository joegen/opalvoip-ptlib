/*
 * video.h
 *
 * Video interface class.
 *
 * Portable Windows Library
 *
 * Copyright (c) 1993-1998 Equivalence Pty. Ltd.
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
 * Portions are Copyright (C) 1993 Free Software Foundation, Inc.
 * All Rights Reserved.
 *
 * Contributor(s): Derek Smithies (derek@indranet.co.nz)
 *
 * $Log: video.h,v $
 * Revision 1.5  2001/10/23 02:11:00  dereks
 * Extend video channel so it can display raw data, using attached video devices.
 *
 * Revision 1.4  2001/05/22 12:49:32  robertj
 * Did some seriously wierd rewrite of platform headers to eliminate the
 *   stupid GNU compiler warning about braces not matching.
 *
 * Revision 1.3  2001/03/07 01:42:59  dereks
 * miscellaneous video fixes. Works on linux now. Add debug statements
 * (at PTRACE level of 1)
 *
 * Revision 1.2  2000/12/19 22:20:26  dereks
 * Add video channel classes to connect to the PwLib PVideoInputDevice class.
 * Add PFakeVideoInput class to generate test images for video.
 *
 * Revision 1.1  2000/11/09 00:43:04  dereks
 * Initial release.
 *
 * 
 *
 */

#define _PVIDEO

#ifdef __GNUC__
#pragma interface
#endif

#include <ptlib/videoio.h>
#include <ptlib/vfakeio.h>

/**A class representing a video channel. This class is provided mainly for
   the playback or recording of video on the system.

   Note that this video channel is implicitly a series of frames in YUV411P format.
   No conversion is performed on data to/from the channel.
 */
class PVideoChannel : public PChannel
{
  PCLASSINFO(PVideoChannel, PChannel);

  public:
  /**@name Construction */
  //@{
    enum Directions {
      Recorder,
      Player
    };

    /// Create a video channel.
    PVideoChannel();

    /** Create a video channel.
        Create a reference to the video drivers for the platform.
      */
    PVideoChannel(
      const PString & device,       /// Name of video driver/device
      Directions dir               /// Video I/O direction
    );
    // 

    ~PVideoChannel();
    // Destroy and close the video driver
  //@}

  /**@name Open functions */
  //@{
    /**Open the specified device for playing or recording. The device name is
       platform specific and is as returned in the GetDevices() function.

       @return
       TRUE if the video device is valid for playing/recording.
     */
    BOOL Open(
      const PString & device,       /// Name of video driver/device
      Directions dir               /// Video I/O direction
    );

    /** return True if one (or both) of the video device class pointers
       is non NULL. If either pointer is non NULL, then a device is ready
       to be written to, which indicates this channel is open.
    */
     BOOL IsOpen() const;
    
    /**Get all of the names for video devices/drivers that are available on
       this platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the #dir#
       parameter in each value is not necessarily the same.

       @return
       An array of platform dependent strings for each video player/recorder.
     */
    static PStringList GetDeviceNames(
      Directions dir    // Video I/O direction
    )  ;

    /**Get the name for the default video devices/driver that is on this
       platform. Note that a named device may not necessarily do both
       playing and recording so the arrays returned with the #dir#
       parameter in each value is not necessarily the same.

       @return
       A platform dependent string for the video player/recorder.
     */
    static PString GetDefaultDevice(
      Directions dir    // Video I/O direction
    );
    //@}

    
    /**Return the width of the currently selected grabbing device.
     */
    virtual PINDEX  GetGrabWidth(); 

    /**Return the height of the currently selected grabbing device.
     */
    virtual PINDEX  GetGrabHeight();

    virtual BOOL Read(void * buf, PINDEX  len);
      // Low level read from the video channel. This function will block until the
      // requested number of characters were read.
  
  
    /**Low level write to the channel, which is data to be rendered to the 
       local video display device.
       */
    BOOL Write(const void * buf,  //Pointer to the image data to be rendered
               PINDEX      len);
	       
    /**Cause the referenced data to be drawn to the 
       previously defined media 
     */
    virtual BOOL Redraw(const void * frame) 
        { return mpOutput->Redraw (frame); };

    /**Set the current time.
     */
    virtual void SetRenderNow(int _now)  
         { mpOutput->SetNow(_now); }

    /**Return the previously specified width.
     */
    PINDEX  GetRenderWidth()
      { return mpOutput->GetFrameWidth(); }

    /**Return the previously specified height.
     */
    PINDEX  GetRenderHeight()
      { return mpOutput->GetFrameHeight(); }

    /**Specifiy the width and height of the video stream, which is to be
       rendered onto the previously specified device.
     */
    virtual void SetRenderFrameSize(int _width, int _height) 
      { mpOutput->SetFrameSize(_width, _height); }

    /**Specifiy the width and height of the video stream, which is to be
       extracted from the previously specified device.
     */
    virtual void SetGrabberFrameSize(int _width, int _height) 
      { if(mpInput != NULL)
           mpInput->SetFrameSize((unsigned)_width, (unsigned)_height); }

    /**Attach a user specific class for rendering video 
     */
    virtual void AttachVideoPlayer(PVideoOutputDevice * device);

    /**Attach a user specific class for acquiring video 
     */
    virtual void AttachVideoReader(PVideoInputDevice * device);

    /**See if the grabber is open 
     */
    virtual BOOL IsGrabberOpen();
    
    /**See if the rendering device is open
     */
    virtual BOOL IsRenderOpen();

    /**Get data from the attached inputDevice, and display on the
       attached ouptutDevice.
    */
    BOOL DisplayRawData(void *videoBuffer);

 protected:
    Directions       direction;

    PString          deviceName;     ///Specified video device name, eg /dev/video0.
    PVideoInputDevice  *mpInput;    /// For grabbing video from the camera.
    PVideoOutputDevice *mpOutput;   /// For displaying video on the screen.

  private:
    void Construct();


// Include platform dependent part of class
#include <ptlib/video.h>
};


// End Of File ///////////////////////////////////////////////////////////////
