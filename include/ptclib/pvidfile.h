/*
 * pvidfile.cxx
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
 *
 * $Log: pvidfile.h,v $
 * Revision 1.3  2006/02/24 04:51:26  csoutheren
 * Fixed problem with using CIF from video files
 * Added support for video files in y4m format
 *
 * Revision 1.2  2006/02/20 06:49:45  csoutheren
 * Added video file and video file input device code
 *
 * Revision 1.1  2006/02/20 06:17:28  csoutheren
 * Added ability to read video from a file
 *
 */

#ifndef _PVIDFILE
#define _PVIDFILE

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_VIDFILE

/**
 * Abstract class for a file containing a sequence of video frames
 */
class PVideoFile : public PFile
{
  PCLASSINFO(PVideoFile, PFile);
  public:
    PVideoFile();

    PVideoFile(
      PINDEX width,
      PINDEX height,
      OpenMode mode,          ///< Mode in which to open the file.
      int opts = ModeDefault  ///< #OpenOptions enum# for open operation.
    );

    /**Create a file object with the specified name and open it in the
       specified mode and with the specified options.

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PVideoFile(
      PINDEX width,
      PINDEX height,
      const PFilePath & name,    ///< Name of file to open.
      OpenMode mode = ReadWrite, ///< Mode in which to open the file.
      int opts = ModeDefault     ///< #OpenOptions enum# for open operation.
    );

    virtual PINDEX GetWidth() const    { return yuvWidth; }
    virtual PINDEX GetHeight() const   { return yuvHeight; }

    virtual void SetWidth(PINDEX v);
    virtual void SetHeight(PINDEX v);

    virtual BOOL WriteFrame(const void * frame) = 0;
    virtual BOOL ReadFrame(void * frame) = 0;

  protected:
    PINDEX yuvWidth, yuvHeight, yuvSize;
};

/**
 * A file containing a sequence of raw YUV files in planar 4:2:0 format
 * Example files can be found at http://media.xiph.org/video/derf/
 */

class PYUVFile : public PVideoFile
{
  PCLASSINFO(PYUVFile, PVideoFile);
  public:
    PYUVFile();

    PYUVFile(
      PINDEX width,
      PINDEX height,
      OpenMode mode,          ///< Mode in which to open the file.
      int opts = ModeDefault  ///< #OpenOptions enum# for open operation.
    );

    /**Create a file object with the specified name and open it in the
       specified mode and with the specified options.

       The #PChannel::IsOpen()# function may be used after object
       construction to determine if the file was successfully opened.
     */
    PYUVFile(
      PINDEX width,
      PINDEX height,
      const PFilePath & name,    ///< Name of file to open.
      OpenMode mode = ReadWrite, ///< Mode in which to open the file.
      int opts = ModeDefault     ///< #OpenOptions enum# for open operation.
    );

    virtual BOOL Open(
      OpenMode mode = ReadWrite,  // Mode in which to open the file.
      int opts = ModeDefault      // Options for open operation.
    );
    virtual BOOL Open(
      const PFilePath & name,    // Name of file to open.
      OpenMode mode = ReadWrite, // Mode in which to open the file.
      int opts = ModeDefault     // #OpenOptions enum# for open operation.
    );

    virtual off_t GetLength() const;
      
    virtual BOOL SetLength(
      off_t len   // New length of file.
    );

    virtual BOOL SetPosition(
      off_t pos,                         ///< New position to set.
      FilePositionOrigin origin = Start  ///< Origin for position change.
    );

    virtual off_t GetPosition() const;

    virtual BOOL WriteFrame(const void * frame);
    virtual BOOL ReadFrame(void * frame);

  protected:
    void Construct();
    PINDEX offset;
    BOOL y4mMode;
};

#endif

#endif // P_VIDFILE
