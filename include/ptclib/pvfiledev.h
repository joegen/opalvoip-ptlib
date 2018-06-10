/*
 * pvfiledev.cxx
 *
 * Video file declaration
 *
 * Portable Windows Library
 *
 * Copyright (C) 2004 Post Increment
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
 * The Initial Developer of the Original Code is
 * Craig Southeren <craigs@postincrement.com>
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_PVFILEDEV_H
#define PTLIB_PVFILEDEV_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#if P_VIDEO
#if P_VIDFILE

#include <ptlib/video.h>
#include <ptlib/vconvert.h>
#include <ptclib/pvidfile.h>


///////////////////////////////////////////////////////////////////////////////////////////
//
// This class defines a video capture (input) device that reads video from a raw YUV file
//

class PVideoInputDevice_VideoFile : public PVideoInputEmulatedDevice
{
  PCLASSINFO(PVideoInputDevice_VideoFile, PVideoInputEmulatedDevice);
  public:
    /** Create a new file based video input device.
    */
    PVideoInputDevice_VideoFile();

    /** Destroy video input device.
    */
    virtual ~PVideoInputDevice_VideoFile();


    /**Get a list of all of the drivers available.
      */
    static PStringArray GetInputDeviceNames();
    virtual PStringArray GetDeviceNames() const;


    /**Open the device given the device name.
      */
    PBoolean Open(
      const PString & deviceName,   /// Device name to open
      PBoolean startImmediate = true    /// Immediately start device
    );

    /**Determine of the device is currently open.
      */
    PBoolean IsOpen() ;

    /**Close the device.
      */
    PBoolean Close();

   
protected:
  virtual bool InternalReadFrameData(BYTE * frame);

   PVideoFile    * m_file;
   PDECLARE_READ_WRITE_MUTEX(m_mutex);
};


///////////////////////////////////////////////////////////////////////////////////////////
//
// This class defines a video display (output) device that writes video to a raw YUV file
//

class PVideoOutputDevice_VideoFile : public PVideoOutputDevice
{
  PCLASSINFO(PVideoOutputDevice_VideoFile, PVideoOutputDevice);

  public:
    /** Create a new video output device.
     */
    PVideoOutputDevice_VideoFile();

    /** Destroy video output device.
     */
    virtual ~PVideoOutputDevice_VideoFile();

    /**Get a list of all of the drivers available.
      */
    static PStringArray GetOutputDeviceNames();

    virtual PStringArray GetDeviceNames() const
      { return GetOutputDeviceNames(); }

    /**Open the device given the device name.
      */
    virtual PBoolean Open(
      const PString & deviceName,   /// Device name to open
      PBoolean startImmediate = true    /// Immediately start device
    );

    /**Start the video device I/O.
      */
    PBoolean Start();

    /**Stop the video device I/O capture.
      */
    PBoolean Stop();

    /**Close the device.
      */
    virtual PBoolean Close();

    /**Determine if the device is currently open.
      */
    virtual PBoolean IsOpen();

    /**Set the colour format to be used.

       Default behaviour sets the value of the colourFormat variable and then
       returns the IsOpen() status.
    */
    virtual PBoolean SetColourFormat(
      const PString & colourFormat   // New colour format for device.
    );
    
    /**Set a section of the output frame buffer.
      */
    virtual PBoolean SetFrameData(const FrameData & frameData);

  protected:  
   PVideoFile * m_file;
   bool         m_opened;
};


#endif // P_VIDFILE
#endif

#endif // PTLIB_PVFILEDEV_H


// End Of File ///////////////////////////////////////////////////////////////
