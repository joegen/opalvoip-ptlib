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
 */

#ifndef PTLIB_PVIDFILE_H
#define PTLIB_PVIDFILE_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_VIDFILE

#include <ptlib/videoio.h>


/**Abstract class for a file containing a sequence of video frames.
  */
class PVideoFile : public PFile
{
  PCLASSINFO(PVideoFile, PFile);
  protected:
    PVideoFile();

  public:
    virtual off_t GetLength() const;
    virtual PBoolean SetLength(
      off_t len   // New length of file in frames.
    );

    virtual off_t GetPosition() const;
    virtual PBoolean SetPosition(
      off_t pos,                                       ///< New position to set.
      PFile::FilePositionOrigin origin = PFile::Start  ///< Origin for position change.
    );

    virtual PBoolean WriteFrame(const void * frame);
    virtual PBoolean ReadFrame(void * frame);

    virtual PBoolean SetFrameSize(
      unsigned width,   ///< New width of frame
      unsigned height   ///< New height of frame
    );
    virtual PBoolean SetFrameRate(
      unsigned rate  ///< Frames  per second
    );

    PINDEX GetFrameBytes() const { return m_frameBytes; }

    bool SetFrameSizeFromFilename(const PString & fn);
    bool SetFPSFromFilename(const PString & fn);

    operator const PVideoFrameInfo &() const { return m_videoInfo; }
    bool IsFixedFrameSize() const { return m_fixedFrameSize; }
    bool GetFrameSize(unsigned & w, unsigned & h) const { return m_videoInfo.GetFrameSize(w, h); }
    unsigned GetFrameWidth() const { return m_videoInfo.GetFrameWidth(); }
    unsigned GetFrameHeight() const { return m_videoInfo.GetFrameHeight(); }
    bool IsFixedFrameRate() const { return m_fixedFrameRate; }
    unsigned GetFrameRate() const { return m_videoInfo.GetFrameRate(); }
    PString GetColourFormat() const { return m_videoInfo.GetColourFormat(); }

  protected:
    bool   m_fixedFrameSize;
    bool   m_fixedFrameRate;
    PINDEX m_frameBytes;
    off_t  m_headerOffset;
    off_t  m_frameHeaderLen;
    PVideoFrameInfo m_videoInfo;
};

typedef PFactory<PVideoFile, PFilePathString> PVideoFileFactory;


/**A file containing a sequence of raw YUV files in planar 4:2:0 format.
   Example files can be found at http://media.xiph.org/video/derf/
  */
class PYUVFile : public PVideoFile
{
    PCLASSINFO(PYUVFile, PVideoFile);
  public:
    PYUVFile();

    virtual PBoolean WriteFrame(const void * frame);
    virtual PBoolean ReadFrame(void * frame);

  protected:
    virtual bool InternalOpen(OpenMode mode, OpenOptions opts, PFileInfo::Permissions permissions);

    bool m_y4mMode;
};

PFACTORY_LOAD(PYUVFile);

#endif  // P_VIDFILE

#endif  // P_VIDEO

#endif // PTLIB_PVIDFILE_H


// End Of File ///////////////////////////////////////////////////////////////
