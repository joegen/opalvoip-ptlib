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
 * $Revision$
 * $Author$
 * $Date$
 */

#ifndef _PVIDFILE
#define _PVIDFILE

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#include <ptlib/videoio.h>


/**
 * Abstract class for a file containing a sequence of video frames
 */
class PVideoFile : public PVideoFrameInfo
{
  PCLASSINFO(PVideoFile, PVideoFrameInfo);
  protected:
    PVideoFile();

  public:
    virtual BOOL SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );

    virtual BOOL Open(
      const PFilePath & name,    // Name of file to open.
      PFile::OpenMode mode = PFile::ReadWrite, // Mode in which to open the file.
      int opts = PFile::ModeDefault     // #OpenOptions enum# for open operation.
    );

    virtual BOOL IsOpen() const { return file.IsOpen(); }
    virtual BOOL Close() { return file.Close(); }

    virtual BOOL WriteFrame(const void * frame);
    virtual BOOL ReadFrame(void * frame);

    virtual off_t GetLength() const;
    virtual BOOL SetLength(
      off_t len   // New length of file in frames.
    );

    virtual off_t GetPosition() const;
    virtual BOOL SetPosition(
      off_t pos,                                       ///< New position to set.
      PFile::FilePositionOrigin origin = PFile::Start  ///< Origin for position change.
    );

    const PFilePath & GetFilePath() const { return file.GetFilePath(); }
    bool IsUnknownFrameSize() const { return unknownFrameSize; }
    PINDEX GetFrameBytes() const { return frameBytes; }

    static BOOL ExtractHints(const PFilePath & fn, PVideoFrameInfo & info);

  protected:
    bool   unknownFrameSize;
    PINDEX frameBytes;
    off_t  headerOffset;
    PFile  file;
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

    virtual BOOL Open(
      const PFilePath & name,    // Name of file to open.
      PFile::OpenMode mode = PFile::ReadWrite, // Mode in which to open the file.
      int opts = PFile::ModeDefault     // #OpenOptions enum# for open operation.
    );

    virtual BOOL WriteFrame(const void * frame);
    virtual BOOL ReadFrame(void * frame);

  protected:
    BOOL y4mMode;
};

#endif
#endif

#endif // P_VIDFILE
