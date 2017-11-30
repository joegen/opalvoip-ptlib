/*
 * mediafile.cxx
 *
 * Media file declaration
 *
 * Portable Windows Library
 *
 * Copyright (C) 2017 Vox Lucida Pty. Ltd.
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
 * The Original Code is Portable Tools Library.
 *
 * All Rights Reserved.
 *
 * Contributor(s): ______________________________________.
 */

#ifndef PTLIB_PMEDIAFILE_H
#define PTLIB_PMEDIAFILE_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>
#include <ptlib/pfactory.h>
#include <ptlib/videoio.h>


/**Abstract class for a file containing a audio/visual media.
  */
class PMediaFile : public PObject
{
  PCLASSINFO(PMediaFile, PObject);
  protected:
    PMediaFile();

    bool m_reading;

    bool CheckOpenAndTrack(unsigned track) const;
    bool CheckMode(bool reading) const;
    bool CheckOpenTrackAndMode(unsigned track, bool reading) const;
    bool CheckModeAndTrackType(bool reading, const PString & expectedTrackType, const PString & actualTrackType) const;

  public:
    typedef PFactory<PMediaFile, PFilePathString> Factory;

    static PMediaFile * Create(const PFilePath & file) { return Factory::CreateInstance(file.GetType()); }

    static const PString & Audio();
    static const PString & Video();

    virtual bool IsSupported(const PString & type) const = 0;

    virtual bool OpenForReading(const PFilePath & filePath) = 0;
    virtual bool OpenForWriting(const PFilePath & filePath) = 0;

    virtual bool IsOpen() const = 0;
    virtual bool Close() = 0;

    bool IsReading() const { return m_reading; }

    struct TrackInfo
    {
      TrackInfo(const PString & type = PString::Empty(), const PString & format = PString::Empty())
        : m_type(type)
        , m_format(format)
        , m_size(0)
        , m_frames(-1)
        , m_rate(0)
        , m_channels(1)
        , m_width(0)
        , m_height(0)
      { }

      PCaselessString m_type;            ///< Type of media, e.g. "audio"/"video"
      PString         m_format;          ///< Format of media, e.g. "PCM-16"
      unsigned        m_size;            ///< Size in bytes of a "frame" for a native read/write
      int64_t         m_frames;          ///< Length of track in frames, -1 if not known
      double          m_rate;            ///< Sample rate for audio, frame rate for video
      unsigned        m_channels;        ///< Number of audio channels
      unsigned        m_width, m_height; ///< Video resolution
      PStringOptions  m_options;         ///< Format specific options
    };
    typedef std::vector<TrackInfo> TracksInfo;

    virtual unsigned GetTrackCount() const = 0;
    virtual bool GetTracks(TracksInfo & tracks) = 0;
    virtual bool SetTracks(const TracksInfo & tracks) = 0;

    virtual bool ReadNative(unsigned track, BYTE * data, PINDEX & size, unsigned & frames) = 0;
    virtual bool WriteNative(unsigned track, const BYTE * data, PINDEX & size, unsigned & frames) = 0;

    virtual bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length) = 0;
    virtual bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written) = 0;

#if OPAL_VIDEO
    virtual bool ConfigureVideo(unsigned track, const PVideoFrameInfo & frameInfo) = 0;
    virtual bool ReadVideo(unsigned track, BYTE * data) = 0;
    virtual bool WriteVideo(unsigned track, const BYTE * data) = 0;
#endif
};


PFACTORY_LOAD(PMediaFile_WAV);

#endif // PTLIB_PMEDIAFILE_H


// End Of File ///////////////////////////////////////////////////////////////
