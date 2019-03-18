/*
 * mediafile.cxx
 *
 * Media file implementation
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

#ifdef __GNUC__
#pragma implementation "mediafile.h"
#endif

#include <ptlib.h>

#define P_FORCE_STATIC_PLUGIN 1
 
#include <ptclib/mediafile.h>

#if P_MEDIAFILE

#include <ptclib/pwavfile.h>
#include <ptlib/vconvert.h>
#include <ptlib/pprocess.h>
#include <ptclib/pvidfile.h>


#define PTraceModule() "MediaFile"


const PString & PMediaFile::Audio() { static PConstString s("audio"); return s; }
const PString & PMediaFile::Video() { static PConstString s("video"); return s; }

PMediaFile::PMediaFile()
  : m_reading(false)
{
  PTRACE(4, "Constructed " << this);
}


PMediaFile::~PMediaFile()
{
  PTRACE(4, "Deleted " << this);
}


bool PMediaFile::CheckOpenAndTrack(unsigned track) const
{
  if (!IsOpen()) {
    PTRACE(2, "Media file not open");
    return false;
  }

  if (track >= GetTrackCount()) {
    PTRACE(2, "Media track not available");
    return false;
  }

  return true;
}


bool PMediaFile::CheckMode(bool reading) const
{
  if (reading && !m_reading) {
    PTRACE(2, "Trying to read media track on write only file");
    return false;
  }

  if (!reading && m_reading) {
    PTRACE(2, "Trying to write media track on read only file");
    return false;
  }

  return true;
}


bool PMediaFile::CheckOpenTrackAndMode(unsigned track, bool reading) const
{
  return CheckOpenAndTrack(track) && CheckMode(reading);
}


bool PMediaFile::CheckModeAndTrackType(bool reading, const PString & expectedTrackType, const PString & actualTrackType) const
{
  if (!CheckMode(reading))
    return false;

  if (expectedTrackType != actualTrackType) {
    PTRACE(2, "Trying to " << (reading ? "read" : "write") << ' '
           << expectedTrackType << " media to a " << actualTrackType << " track.");
    return false;
  }

  return true;
}


bool PMediaFile::SetErrorText(const PString & error)
{
  m_lastErrorText = error;
  PTRACE(2, "Error: " << error);
  return false;
}


///////////////////////////////////////////////////////////////////////////////

PMediaFile::TrackInfo::TrackInfo(const PString & type, const PString & format)
  : m_type(type)
  , m_format(format)
  , m_size(0)
  , m_frames(-1)
  , m_rate(0)
  , m_channels(1)
  , m_width(0)
  , m_height(0)
{
}


PMediaFile::TrackInfo::TrackInfo(unsigned rate, unsigned channels)
  : m_type(Audio())
  , m_format(PSOUND_PCM16)
  , m_size(channels*2)
  , m_frames(-1)
  , m_rate(rate)
  , m_channels(channels)
  , m_width(0)
  , m_height(0)
{
}


#if P_VIDEO
PMediaFile::TrackInfo::TrackInfo(unsigned width, unsigned height, double rate)
  : m_type(Video())
  , m_format(PVideoFrameInfo::YUV420P())
  , m_size(PVideoFrameInfo::CalculateFrameBytes(width, height))
  , m_frames(-1)
  , m_rate(rate)
  , m_channels(1)
  , m_width(width)
  , m_height(height)
{
}
#endif // P_VIDEO


bool PMediaFile::TrackInfo::operator==(const TrackInfo & other) const
{
  return m_type     == other.m_type     &&
         m_format   == other.m_format   &&
         m_rate     == other.m_rate     &&
         m_channels == other.m_channels &&
         m_width    == other.m_width    &&
         m_height   == other .m_height;
}


///////////////////////////////////////////////////////////////////////////////

PMediaFile::SoundChannel::SoundChannel(const Ptr & mediaFile, unsigned track)
  : m_mediaFile(mediaFile)
  , m_track(track)
{
}


PMediaFile::SoundChannel::~SoundChannel()
{
  Close();
}


bool PMediaFile::SoundChannel::Open(const Params & params)
{
  m_activeDirection = params.m_direction;

  if (m_mediaFile.IsNULL()) {
    m_mediaFile = PMediaFile::Create(params.m_driver);
    if (m_mediaFile.IsNULL()) {
      PTRACE(2, "Could not create media file for " << params.m_driver);
      return false;
    }
  }

  if (params.m_direction == PSoundChannel::Player) {
    SetFormat(params.m_channels, params.m_sampleRate, params.m_bitsPerSample);

    if (!m_mediaFile->IsOpen() || m_mediaFile->IsReading()) {
      if (!m_mediaFile->OpenForWriting(params.m_driver)) {
        PTRACE(2, "Open for writing failed for " << m_mediaFile->GetFilePath());
        return false;
      }
    }

    PMediaFile::TracksInfo tracks;
    tracks.push_back(PMediaFile::TrackInfo(m_sampleRate, m_channels));
    if (m_mediaFile->SetTracks(tracks))
      return true;

    PTRACE(2, "Could not set audio track in " << m_mediaFile->GetFilePath());
  }
  else {
    if (!m_mediaFile->IsOpen() || !m_mediaFile->IsReading()) {
      if (!m_mediaFile->OpenForReading(params.m_driver)) {
        PTRACE(2, "Open for reading failed for " << m_mediaFile->GetFilePath());
        return false;
      }
    }

    PMediaFile::TracksInfo tracks;
    if (m_mediaFile->GetTracks(tracks)) {
      for (m_track = 0; m_track < tracks.size(); ++m_track) {
        const PMediaFile::TrackInfo & track = tracks[m_track];
        if (track.m_type == PMediaFile::Audio()) {
          unsigned sampleRate = params.m_sampleRate != 0 ? params.m_sampleRate : (unsigned)track.m_rate;
          unsigned channels = params.m_channels != 0 ? params.m_channels : track.m_channels;
          m_mediaFile->ConfigureAudio(m_track, channels, sampleRate);
          SetFormat(channels, sampleRate, 16);
          return true;
        }
      }
    }

    PTRACE(2, "Could not find audio track in " << m_mediaFile->GetFilePath());
  }

  return false;
}


PString PMediaFile::SoundChannel::GetName() const
{
  return m_mediaFile.IsNULL() ? PString::Empty() : static_cast<PString>(m_mediaFile->GetFilePath());
}


PBoolean PMediaFile::SoundChannel::Close()
{
  if (CheckNotOpen())
    return false;

  m_mediaFile = NULL;
  return true;
}


PBoolean PMediaFile::SoundChannel::IsOpen() const
{
  return !m_mediaFile.IsNULL() && m_mediaFile->IsOpen();
}


bool PMediaFile::SoundChannel::RawWrite(const void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  PINDEX actual;
  if (m_mediaFile->WriteAudio(m_track, reinterpret_cast<const BYTE *>(buf), len, actual)) {
    SetLastWriteCount(actual);
    return true;
  }

  SetErrorValues(Miscellaneous, 1000001, LastWriteError);
  return false;
}


bool PMediaFile::SoundChannel::RawRead(void * buf, PINDEX len)
{
  if (CheckNotOpen())
    return false;

  PINDEX actual;
  if (m_mediaFile->ReadAudio(m_track, reinterpret_cast<BYTE *>(buf), len, actual)) {
    SetLastReadCount(actual);
    return true;
  }

  SetErrorValues(Miscellaneous, 1000002, LastReadError);
  return false;
}


bool PMediaFile::SoundChannel::Rewind()
{
  if (CheckNotOpen())
    return false;

  return true;
}


///////////////////////////////////////////////////////////////////////////////

#if P_VIDEO

PMediaFile::VideoInputDevice::VideoInputDevice(const Ptr & mediaFile, unsigned track)
  : m_mediaFile(mediaFile)
  , m_track(track)
{
}


PMediaFile::VideoInputDevice::~VideoInputDevice()
{
  Close();
}


PStringArray PMediaFile::VideoInputDevice::GetDeviceNames() const
{
  PStringArray s;
  PMediaFile::Factory::KeyList_T keys = PMediaFile::Factory::GetKeyList();
  for (PMediaFile::Factory::KeyList_T::iterator it = keys.begin(); it != keys.end(); ++it)
    s.AppendString(*it);
  return s;
}


PBoolean PMediaFile::VideoInputDevice::Open(const PString & devName, PBoolean)
{
  if (devName.IsEmpty())
    return false;

  PFilePath filePath;

  PINDEX pos = devName.GetLength()-1;
  if (devName[pos] != '*')
    filePath = devName;
  else {
    filePath = devName.Left(pos);
    SetChannel(Channel_PlayAndRepeat);
  }

  if (m_mediaFile.IsNULL())
    m_mediaFile = PMediaFile::Factory::CreateInstance(filePath.GetType());
  if (m_mediaFile.IsNULL()) {
    PTRACE(1, "Cannot open file of type \"" << filePath.GetType() << "\" as video input device");
    return false;
  }

  if (!m_mediaFile->IsOpen() || !m_mediaFile->IsReading()) {
    if (!m_mediaFile->OpenForReading(filePath)) {
      PTRACE(1, "Cannot open file \"" << filePath << "\" as video input device: " << m_mediaFile->GetErrorText());
      return false;
    }
  }

  PMediaFile::TracksInfo tracks;
  if (!m_mediaFile->GetTracks(tracks)) {
    PTRACE(1, "Cannot get tracks in file \"" << filePath << "\" as video input device: " << m_mediaFile->GetErrorText());
    return false;
  }

  for (m_track = 0; m_track < tracks.size(); ++m_track) {
    if (tracks[m_track].m_type == PMediaFile::Video())
      break;
  }

  if (m_track >= tracks.size()) {
    PTRACE(1, "no video track in file \"" << filePath << "\" as video input device: " << m_mediaFile->GetErrorText());
    return false;
  }

  PTRACE(3, "Opening file " << filePath);

  m_frameWidth = tracks[m_track].m_width;
  m_frameHeight = tracks[m_track].m_height;
  m_frameRate = (unsigned)(tracks[m_track].m_rate+0.5);
  m_deviceName = m_mediaFile->GetFilePath();
  return true;
}


PBoolean PMediaFile::VideoInputDevice::IsOpen()
{
  return !m_mediaFile.IsNULL() && m_mediaFile->IsOpen();
}


PBoolean PMediaFile::VideoInputDevice::Close()
{
  m_mediaFile = NULL;
  return true;
}


bool PMediaFile::VideoInputDevice::InternalReadFrameData(BYTE * frame)
{
  return m_mediaFile->ReadVideo(m_track, frame);
}


#endif // P_VIDEO


///////////////////////////////////////////////////////////////////////////////

class PMediaFile_WAV : public PMediaFile
{
  PCLASSINFO(PMediaFile_WAV, PMediaFile);

protected:
  PWAVFile m_wavFile;

public:
  PMediaFile_WAV()
  {
  }


  ~PMediaFile_WAV()
  {
    Close();
  }


  bool IsSupported(const PString & type) const
  {
    return type == Audio();
  }


  bool OpenForReading(const PFilePath & filePath)
  {
    if (!m_wavFile.Open(filePath, PFile::ReadOnly))
      return SetErrorText(m_wavFile.GetErrorText());

    m_reading = true;
    m_filePath = filePath;
    return true;
  }


  bool OpenForWriting(const PFilePath & filePath)
  {
    if (!m_wavFile.Open(filePath, PFile::WriteOnly))
      return SetErrorText(m_wavFile.GetErrorText());

    m_reading = false;
    m_filePath = filePath;
    return true;
  }


  bool IsOpen() const
  {
    return m_wavFile.IsOpen();
  }


  bool Close()
  {
    return m_wavFile.Close();
  }


  bool GetDefaultTrackInfo(const PCaselessString & type, TrackInfo & info) const
  {
    if (type != Audio())
      return false;

    info = TrackInfo(16000, 1);
    return true;
  }


  unsigned GetTrackCount() const
  {
    return 1;
  }


  bool GetTracks(TracksInfo & tracks)
  {
    if (!m_wavFile.IsOpen())
      return SetErrorText("Cannot get tracks info when WAV file not open");

    TrackInfo info(Audio(), m_wavFile.GetFormatString());
    info.m_size = m_wavFile.GetChannels()*(m_wavFile.GetSampleSize()+7)/8;
    info.m_rate = m_wavFile.GetSampleRate();
    info.m_frames = m_wavFile.GetLength()/info.m_size;
    info.m_channels = m_wavFile.GetChannels();
    tracks.clear();
    tracks.push_back(info);
    return true;
  }


  bool SetTracks(const TracksInfo & tracks)
  {
    if (!m_wavFile.IsOpen())
      return SetErrorText("Cannot Set tracks info when WAV file not open");

    if (m_reading)
      return SetErrorText("Cannot set tracks info when reading WAV file");

    if (tracks.size() != 1)
      return SetErrorText("WAV file only supports one track");

    const TrackInfo & track = tracks.front();

    if (track.m_type != Audio())
      return SetErrorText("WAV file only supports audio");

    if (  !m_wavFile.SetFormat(track.m_format) ||
          !m_wavFile.SetSampleSize(track.m_size*8/track.m_channels) ||
          !m_wavFile.SetSampleRate((unsigned)track.m_rate) ||
          !m_wavFile.SetChannels(track.m_channels))
      return SetErrorText("Cannot set WAV file track after started writing");

    PTRACE(4, "Track set for WAV file");
    return true;
  }


  bool ReadNative(unsigned track, BYTE * data, PINDEX & size, unsigned & frames)
  {
    if (!CheckOpenTrackAndMode(track, true))
      return false;

    PINDEX frameBytes = (m_wavFile.GetRawSampleSize() + 7) / 8;
    if (!m_wavFile.RawRead(data, std::min(size, (PINDEX)(frames*frameBytes))))
      return false;

    size = m_wavFile.GetLastReadCount();
    frames = size / frameBytes;
    return true;
  }


  bool WriteNative(unsigned track, const BYTE * data, PINDEX & size, unsigned & frames)
  {
    if (!CheckOpenTrackAndMode(track, false))
      return false;

    PINDEX frameBytes = (m_wavFile.GetRawSampleSize() + 7) / 8;
    if (size < frameBytes)
      return false;

    if (!m_wavFile.RawWrite(data, frames*frameBytes))
      return false;

    size = m_wavFile.GetLastWriteCount();
    frames = m_wavFile.GetLastWriteCount() / frameBytes;
    return true;
  }


  bool ConfigureAudio(unsigned track, unsigned channels, unsigned sampleRate)
  {
    return CheckOpenAndTrack(track) && m_wavFile.SetSampleRate(sampleRate) && m_wavFile.SetChannels(channels);
  }


  bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length)
  {
    if (!CheckOpenTrackAndMode(track, true))
      return false;

    if (!m_wavFile.Read(data, size))
      return false;

    length = m_wavFile.GetLastReadCount();
    return true;
  }


  bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written)
  {
    if (!CheckOpenTrackAndMode(track, false))
      return false;

    if (!m_wavFile.Write(data, length))
      return false;

    written = m_wavFile.GetLastWriteCount();
    return true;
  }


#if P_VIDEO
  bool ConfigureVideo(unsigned, const PVideoFrameInfo &)
  {
    return false;
  }


  bool ReadVideo(unsigned, BYTE *)
  {
    return false;
  }


  bool WriteVideo(unsigned, const BYTE *)
  {
    return false;
  }
#endif // P_VIDEO
};

PFACTORY_CREATE(PMediaFile::Factory, PMediaFile_WAV, ".wav");


#if P_FFMPEG_FULL

extern "C" {
  P_PUSH_MSVC_WARNINGS(4244)
  #include <libavformat/avformat.h>
  #include <libavutil/imgutils.h>
  #include <libswresample/swresample.h>
  P_POP_MSVC_WARNINGS()
};

#ifdef P_AVFORMAT_LIB
  #pragma comment(lib, P_AVFORMAT_LIB)
#endif
#ifdef P_AVCODEC_LIB
  #pragma comment(lib, P_AVCODEC_LIB)
#endif
#ifdef P_AVUTIL_LIB
  #pragma comment(lib, P_AVUTIL_LIB)
#endif
#ifdef P_SWRESAMPLE_LIB
  #pragma comment(lib, P_SWRESAMPLE_LIB)
#endif


PStringSet PMediaFile::GetAllFileTypes()
{
  PStringSet fileTypes(PFilePathString(".wav"));
  const AVInputFormat *fmt;

#if LIBAVFORMAT_VERSION_MAJOR < 58
  // Can't rely on factory initialisation, as is used by other factories
  av_register_all();

  fmt = NULL;
  while ((fmt = av_iformat_next(fmt)) != NULL)
#else
  void * iter = NULL;
  while ((fmt = av_demuxer_iterate(&iter)) != NULL)
#endif
  {
    PStringArray formatsExtensions(PString(fmt->extensions).Tokenise(","));
    for (PINDEX i = 0; i < formatsExtensions.GetSize(); ++i)
      fileTypes += PFilePathString('.' + formatsExtensions[i]);
  }

  return fileTypes;
}


class PMediaFile_FFMPEG : public PMediaFile
{
    PCLASSINFO(PMediaFile_FFMPEG, PMediaFile);
  public:
    struct FactoryInitialiser : PMediaFile::Factory::WorkerBase
    {
      FactoryInitialiser()
      {
        PStringSet fileTypes = PMediaFile::GetAllFileTypes();

        PCaselessString firstExt;
        for (PStringSet::iterator it = fileTypes.begin(); it != fileTypes.end(); ++it) {
          if (!firstExt.IsEmpty())
            PMediaFile::Factory::RegisterAs(*it, firstExt);
          else {
            PMediaFile::Factory::Register(*it, this);
            firstExt = *it;
          }
        }
      }

      virtual PMediaFile * Create(PMediaFile::Factory::Param_T) const
      {
        return new PMediaFile_FFMPEG();
      }
    };

  protected:
    AVFormatContext * m_formatContext;
    PDECLARE_MUTEX(   m_mutex);
    bool              m_headerWritten;

#if PTRACING
    bool CheckError(int result, const char * fn)
    {
      if (result >= 0)
        return false;

      if (PTrace::CanTrace(2)) {
        ostream & trace = PTRACE_BEGIN(2);
        trace << "FFMPEG internal error " << result << " in " << fn;
        char strbuf[AV_ERROR_MAX_STRING_SIZE];
        if (av_strerror(result, strbuf, sizeof(strbuf)) == 0)
          trace << ": " << strbuf;
        trace << PTrace::End;
      }

      return true;
    }
    #define CHECK_ERROR_RESULT(...) CheckError(__VA_ARGS__)
    #define CHECK_ERROR_RESULT_TRK(...) m_owner->CheckError(__VA_ARGS__)
#else
    #define CHECK_ERROR_RESULT(result, ...) (result < 0)
    #define CHECK_ERROR_RESULT_TRK(result, ...) CHECK_ERROR_RESULT(result, __VA_ARGS__)
#endif
    #define CHECK_ERROR(fn, args) CHECK_ERROR_RESULT(fn args, #fn)
    #define CHECK_ERROR_TRK(fn, args) CHECK_ERROR_RESULT_TRK(fn args, #fn)


    struct TrackContext : TrackInfo
    {
      PMediaFile_FFMPEG  * m_owner;
      int                  m_index;
      AVStream           * m_stream;
      AVCodec      const * m_codecInfo;
      AVCodecContext     * m_codecContext;
      bool                 m_codecOpened;
      AVFrame            * m_frame;
      int64_t              m_frameTime;
      int64_t              m_currentTime;
      std::queue<AVPacket> m_interleaved;
      SwrContext         * m_swrContext;

      TrackContext()
        : m_owner(NULL)
        , m_index(-1)
        , m_stream(NULL)
        , m_codecInfo(NULL)
        , m_codecContext(NULL)
        , m_codecOpened(false)
        , m_frame(NULL)
        , m_frameTime(1)
        , m_currentTime(0)
        , m_swrContext(NULL)
      {
      }


      TrackContext & operator=(const TrackInfo & info)
      {
        Close();
        TrackInfo::operator=(info);
        return *this;
      }


      bool CreateCodecContext()
      {
        m_codecContext = avcodec_alloc_context3(m_codecInfo);
        if (m_codecContext == NULL) {
          PTRACE(2, m_owner, "Could not allocate codec context for " << m_owner->m_filePath);
          return false;
        }

        if (CHECK_ERROR_TRK(avcodec_parameters_to_context, (m_codecContext, m_stream->codecpar)))
          return false;

        m_frame = av_frame_alloc();
        if (m_frame == NULL) {
          PTRACE(2, m_owner, "Could not allocate frame for " << m_owner->m_filePath);
          return false;
        }

        PTRACE(4, m_owner, "Codec context created");
        return true;
      }


      bool OpenCodec()
      {
        AVDictionary * options = NULL;
        for (PStringOptions::iterator it = m_options.begin(); it != m_options.end(); ++it)
          av_dict_set(&options, it->first, it->second, 0);

        m_codecOpened = !CHECK_ERROR_TRK(avcodec_open2, (m_codecContext, m_codecInfo, &options));

        av_dict_free(&options);

        PTRACE_IF(4, m_codecOpened, m_owner, "Codec opened");
        return m_codecOpened;
      }


      bool Open(PMediaFile_FFMPEG * owner, unsigned track)
      {
        m_owner = owner;
        m_index = (int)track;
        m_stream = m_owner->m_formatContext->streams[track];

        AVCodecID codecID = m_stream->codecpar->codec_id;
        m_codecInfo = avcodec_find_decoder(codecID);
        if (m_codecInfo == NULL) {
          PTRACE(2, owner, "Unsupported video codec " << avcodec_get_name(codecID) << " in " << owner->m_filePath);
          return false;
        }

        if (!CreateCodecContext())
          return false;

        switch (m_codecContext->codec_type) {
        case AVMEDIA_TYPE_AUDIO:
          m_type = Audio();
          m_rate = m_codecContext->sample_rate;
          m_frames = m_stream->nb_frames;
          m_channels = m_codecContext->channels;
          if (m_codecInfo->id == AV_CODEC_ID_PCM_S16LE) {
            m_format = PSOUND_PCM16;
            m_size = 2 * m_channels;
          }
          else {
            m_format = m_codecInfo->name;
            m_size = m_codecContext->frame_size;
            if (m_size == 0)
              m_size = (unsigned)(m_codecContext->bit_rate / 8 / 50);
          }
          break;

        case AVMEDIA_TYPE_VIDEO:
          m_type = Video();
          m_format = m_codecInfo->name;
          m_codecContext->framerate = av_guess_frame_rate(owner->m_formatContext, m_stream, NULL);
          m_rate = av_q2d(m_codecContext->framerate);
          m_frames = m_stream->nb_frames;
          m_width = m_codecContext->width;
          m_height = m_codecContext->height;
          m_size = m_width*m_height*3/2;
          break;

        case AVMEDIA_TYPE_DATA:
          m_type = "Data";
          break;

        case AVMEDIA_TYPE_SUBTITLE:
          m_type = "Subtitle";
          break;

        default:
          PTRACE(2, owner, "Unknown media type in " << owner->m_filePath);
          return false;
        }

        return true;
      }


      bool Create(PMediaFile_FFMPEG * owner, unsigned track)
      {
        m_owner = owner;
        m_index = (int)track;

        if (m_format.FindSpan("0123456789") == P_MAX_INDEX)
          m_codecInfo = avcodec_find_encoder((AVCodecID)m_format.AsUnsigned());
        else {
          if (m_format == PSOUND_PCM16"PCM-16")
            m_codecInfo = avcodec_find_encoder(AV_CODEC_ID_PCM_S16LE);
          else
            m_codecInfo = avcodec_find_encoder_by_name(m_format);
        }
        if (m_codecInfo == NULL) {
          PTRACE(2, m_owner, "Cannot find output encoder \"" << m_format << '"');
          return false;
        }

        if ((m_stream = avformat_new_stream(m_owner->m_formatContext, m_codecInfo)) == NULL) {
          PTRACE(2, m_owner, "Cannot allocate FFMPEG stream!");
          return false;
        }

        m_stream->id = m_index;

        m_stream->codecpar->codec_type = m_codecInfo->type;
        m_stream->codecpar->codec_id = m_codecInfo->id;
        m_stream->codecpar->bit_rate = m_options.GetInteger("BitRate");
        switch (m_codecInfo->type) {
          case AVMEDIA_TYPE_AUDIO:
            if (m_codecInfo->supported_samplerates == NULL)
              m_stream->codecpar->sample_rate = (int)m_rate;
            else {
              m_stream->codecpar->sample_rate = m_codecInfo->supported_samplerates[0];
              for (int i = 0; m_codecInfo->supported_samplerates[i]; i++) {
                if (m_codecInfo->supported_samplerates[i] == m_rate) {
                  m_stream->codecpar->sample_rate = (int)m_rate;
                  break;
                }
              }
            }

            m_stream->codecpar->channels = m_channels;
            m_stream->codecpar->channel_layout = av_get_default_channel_layout(m_channels);
            break;

          case AVMEDIA_TYPE_VIDEO:
            m_stream->codecpar->width = m_width;
            m_stream->codecpar->height = m_height;
            m_stream->codecpar->profile = m_options.GetInteger("Profile");
            m_stream->codecpar->level = m_options.GetInteger("Level");
            break;

          default:
            break;
        }

        if (!CreateCodecContext())
          return false;

        // some formats want stream headers to be separate
        if (m_owner->m_formatContext->oformat->flags & AVFMT_GLOBALHEADER)
          m_codecContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

        switch (m_codecContext->codec_type) {
          case AVMEDIA_TYPE_AUDIO:
            m_codecContext->sample_fmt = m_codecInfo->sample_fmts ? m_codecInfo->sample_fmts[0] : AV_SAMPLE_FMT_FLTP;
            m_codecContext->time_base.num = 1;
            m_codecContext->time_base.den = m_codecContext->sample_rate;

            // AAC has been experimental for a decade ...
            if (m_codecContext->codec_id == AV_CODEC_ID_AAC)
              m_codecContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
            break;

          case AVMEDIA_TYPE_VIDEO:
            m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
            m_codecContext->bit_rate_tolerance = (int)(m_codecContext->bit_rate / 2);
            m_codecContext->rc_buffer_size = (int)(m_codecContext->bit_rate * 5);
            m_codecContext->gop_size = m_options.GetInteger("KeyFrameInterval");
            m_codecContext->time_base.num = 1;
            m_codecContext->time_base.den = 90000;

            if ((m_owner->m_formatContext->oformat->flags & AVFMT_VARIABLE_FPS) == 0) {
              AVRational frameRate = av_d2q(m_rate, 90000);
              if (m_codecInfo->supported_framerates == NULL)
                m_codecContext->framerate = frameRate;
              else {
                int idx = av_find_nearest_q_idx(m_codecContext->framerate, m_codecInfo->supported_framerates);
                m_codecContext->framerate = m_codecInfo->supported_framerates[idx];
              }
              m_frameTime = lrint(1/(av_q2d(m_codecContext->framerate) * av_q2d(m_codecContext->time_base)));
            }

            switch (m_codecContext->codec_id) {
              case AV_CODEC_ID_MPEG1VIDEO:
                /* Needed to avoid using macroblocks in which some coeffs overflow.
                    This does not happen with normal video, it just happens here as
                    the motion of the chroma plane does not match the luma plane. */
                m_codecContext->mb_decision = 2;
                break;

              case AV_CODEC_ID_MPEG2VIDEO:
                /* just for testing, we also add B frames */
                m_codecContext->max_b_frames = 2;
                break;

              default:
                break;
            }
            break;

          default:
            PTRACE(2, m_owner, "Cannot create track of that type!");
            return false;
        }

        return true;
      }


      void Close()
      {
        avcodec_free_context(&m_codecContext);
        av_frame_free(&m_frame);
        swr_free(&m_swrContext);
      }


      bool ReadNative(BYTE * data, PINDEX & size, unsigned & frames)
      {
        if (m_interleaved.empty()) {
          if (!m_owner->ReadInterleaved(m_index))
            return false;
          if (m_interleaved.empty()) // End of file
            return false;
        }

        AVPacket packet = m_interleaved.front();
        m_interleaved.pop();
        if (size < (PINDEX)packet.size)
          return false;

        size = packet.size;
        memcpy(data, packet.data, size);
        frames = 1;
        return true;
      }


      bool WriteNative(const BYTE * data, PINDEX & size, unsigned & frames)
      {
        AVPacket packet;
        av_init_packet(&packet);
        packet.stream_index = m_index;
        packet.data = const_cast<BYTE *>(data);
        packet.size = size;
        packet.pts = packet.dts = m_currentTime;

        if (CHECK_ERROR_TRK(av_write_frame, (m_owner->m_formatContext, &packet)))
          return false;

        m_currentTime += m_frameTime;
        frames = 1;
        return true;
      }


      bool ReadAndDecodeFrame()
      {
        int result;
        while ((result = avcodec_receive_frame(m_codecContext, m_frame)) < 0) {
          if (result != AVERROR(EAGAIN) && result == AVERROR_EOF)
            return false;

          if (m_interleaved.empty()) {
            if (!m_owner->ReadInterleaved(m_index))
              return false;
          }

          if (m_interleaved.empty()) {
            if (CHECK_ERROR_TRK(avcodec_send_packet, (m_codecContext, NULL)))
              return false;
          }
          else {
            AVPacket packet = m_interleaved.front();
            m_interleaved.pop();

            if (packet.pts == AV_NOPTS_VALUE)
              packet.pts = packet.dts = m_currentTime;

            if (CHECK_ERROR_TRK(avcodec_send_packet, (m_codecContext, &packet)))
              return false;
          }
        }

        return true;
      }


      bool CreateResampler(AVSampleFormat inFormat, int inRate, int inChannels,
                         AVSampleFormat outFormat, int outRate, int outChannels)
      {
          m_swrContext = swr_alloc_set_opts(NULL,
                                            av_get_default_channel_layout(outChannels),
                                            outFormat,
                                            outRate,
                                            av_get_default_channel_layout(inChannels),
                                            inFormat,
                                            inRate,
                                            0, NULL);
          if (m_swrContext == NULL) {
            PTRACE(2, m_owner, "Could not allocagte audio resampler for " << m_owner->m_filePath);
            return false;
          }

          if (CHECK_ERROR_TRK(swr_init, (m_swrContext)))
            return false;

          PTRACE(4, m_owner, "Created resampler");
          return true;
      }


      bool ConfigureAudio(unsigned channels, unsigned sampleRate)
      {
        m_channels = channels;
        m_rate = sampleRate;
        return true;
      }


      bool ReadAudio(BYTE * data, PINDEX size, PINDEX & length)
      {
        if (!m_owner->CheckModeAndTrackType(true, Audio(), m_type))
          return false;

        if (!m_codecOpened) {
          m_codecContext->request_sample_fmt = AV_SAMPLE_FMT_S16;
          if (!OpenCodec() || !CreateResampler(m_codecContext->sample_fmt, m_codecContext->sample_rate, m_codecContext->channels,
                                               AV_SAMPLE_FMT_S16, (int)m_rate, m_channels))
            return false;
        }

        int requestedSamples = size/m_channels/2;

        if (swr_get_out_samples(m_swrContext, 0) < requestedSamples) {
          if (!ReadAndDecodeFrame())
            return false;
        }

        int convertedSamples = swr_convert(m_swrContext,
                                            &data, requestedSamples,
                                            (const uint8_t **)m_frame->data, m_frame->nb_samples);
        if (CHECK_ERROR_RESULT_TRK(convertedSamples, "swr_convert"))
          return false;

        length = convertedSamples*m_channels*2;
        return true;
      }


      bool EncodeAndWriteFrame()
      {
        if (CHECK_ERROR_TRK(avcodec_send_frame, (m_codecContext, m_frame)))
          return false;

        AVPacket packet;
        av_init_packet(&packet);

        for (;;) {
          int result = avcodec_receive_packet(m_codecContext, &packet);
          if (result == AVERROR(EAGAIN) || result == AVERROR_EOF)
            return true;

          if (CHECK_ERROR_RESULT_TRK(result, "avcodec_receive_packet"))
            return false;

          if (CHECK_ERROR_TRK(av_write_frame, (m_owner->m_formatContext, &packet)))
            return false;
        }
      }


      bool WriteAudio(const BYTE * data, PINDEX length, PINDEX & written)
      {
        if (!m_owner->CheckModeAndTrackType(false, Audio(), m_type))
          return false;

        if (!m_codecOpened) {
          m_codecContext->request_sample_fmt = AV_SAMPLE_FMT_S16;
          if (!OpenCodec() || !CreateResampler(AV_SAMPLE_FMT_S16, (int)m_rate, m_channels,
                                               m_codecContext->sample_fmt, m_codecContext->sample_rate, m_codecContext->channels))
            return false;
        }

        m_frame->data[0] = const_cast<BYTE *>(data);
        m_frame->linesize[0] = length;
        m_frame->nb_samples = length/m_channels/2;
        if (!EncodeAndWriteFrame())
          return false;

        written = m_frame->nb_samples*m_channels*2;
        return true;
      }


#if P_VIDEO
      static AVPixelFormat GetPixelFormatFromColourFormat(const PCaselessString & colourFormat)
      {
        static struct {
          const char *  m_colourFormat;
          AVPixelFormat m_pixelFormat;
        } const table[] = {
          { PVideoFrameInfo::YUV420P(), AV_PIX_FMT_YUV420P },
          { "RGB24", AV_PIX_FMT_RGB24 },
          { "BGR24", AV_PIX_FMT_BGR24 },
          { "RGB32", AV_PIX_FMT_RGBA },
          { "BGR32", AV_PIX_FMT_BGRA }
        };

        for (PINDEX i = 0; i < PARRAYSIZE(table); ++i) {
          if (colourFormat == table[i].m_colourFormat)
            return table[i].m_pixelFormat;
        }
        return AV_PIX_FMT_NONE;
      }

      bool ConfigureReadVideo(const PVideoFrameInfo & frameInfo)
      {
        if (!m_owner->CheckModeAndTrackType(true, Video(), m_type))
          return false;

        m_codecContext->pix_fmt = GetPixelFormatFromColourFormat(frameInfo.GetColourFormat());
        if (m_codecContext->pix_fmt == AV_PIX_FMT_NONE) {
          PTRACE(2, m_owner, "Unsupported colour format " << frameInfo << " in " << m_owner->m_filePath);
          return false;
        }

        return OpenCodec();
      }


      bool ReadVideo(BYTE * data)
      {
        if (!m_owner->CheckModeAndTrackType(true, Video(), m_type))
          return false;

        if (!m_codecOpened) {
          if (!ConfigureReadVideo(PVideoFrameInfo(m_width, m_height)))
            return false;
        }

        if (!ReadAndDecodeFrame())
          return false;

        if (m_width != (unsigned)m_frame->width || m_height != (unsigned)m_frame->height) {
          PTRACE(2, m_owner, "Change of video size in mid playback not supported.");
          return false;
        }

        if (m_codecContext->pix_fmt == AV_PIX_FMT_YUV420P) {
          unsigned planeWidth = (m_width + 1)&~1;
          unsigned planeHeight = (m_height + 1)&~1;

          av_image_copy_plane(data, planeWidth, m_frame->data[0], m_frame->linesize[0], m_width, m_height);
          data += planeWidth*planeHeight;
          planeWidth /= 2;
          planeHeight /= 2;
          av_image_copy_plane(data, planeWidth, m_frame->data[1], m_frame->linesize[1], planeWidth, planeHeight);
          data += planeWidth*planeHeight;
          av_image_copy_plane(data, planeWidth, m_frame->data[2], m_frame->linesize[2], planeWidth, planeHeight);
        }

        m_currentTime += m_frameTime;
        return true;
      }


      bool ConfigureWriteVideo(const PVideoFrameInfo & frameInfo)
      {
        if (!m_owner->CheckModeAndTrackType(false, Video(), m_type))
          return false;

        if (!OpenCodec())
          return false;

        m_frame->format = GetPixelFormatFromColourFormat(frameInfo.GetColourFormat());
        if (m_frame->format == AV_PIX_FMT_NONE) {
          PTRACE(2, m_owner, "Unsupported colour format " << frameInfo << " in " << m_owner->m_filePath);
          return false;
        }

        m_frame->width  = m_width;
        m_frame->height = m_height;

        if (CHECK_ERROR_TRK(av_frame_get_buffer,(m_frame, 32)))
          return false;

        return true;
      }


      bool WriteVideo(const BYTE * data)
      {
        if (!m_owner->CheckModeAndTrackType(false, Video(), m_type))
          return false;

        if (!m_codecOpened) {
          if (!ConfigureWriteVideo(PVideoFrameInfo(m_width, m_height)))
            return false;
        }

        uint8_t * ptr = const_cast<BYTE *>(data);
        m_frame->data[0] = ptr;
        m_frame->data[1] = ptr += m_width*m_height;
        m_frame->data[1] = ptr += m_width*m_height/4;
        m_frame->linesize[0] = m_width;
        m_frame->linesize[1] = m_width/2;
        m_frame->linesize[2] = m_width/2;
        return EncodeAndWriteFrame();
      }
#endif // P_VIDEO
    };

    std::vector<TrackContext> m_tracks;


    bool ReadInterleaved(unsigned track)
    {
      for (;;) {
        AVPacket packet;
        av_init_packet(&packet);

        int result = av_read_frame(m_formatContext, &packet);
        if (result == AVERROR_EOF)
          return true;
        if (CHECK_ERROR_RESULT(result, "av_read_frame"))
          return false;

        m_tracks[packet.stream_index].m_interleaved.push(packet);

        if (packet.stream_index == (int)track)
          return true;
      }
    }


    bool WriteHeader()
    {
      if (m_headerWritten)
        return true;

      /* Write the stream header, if any. */
      AVDictionary * options = NULL;
      m_headerWritten = !CHECK_ERROR(avformat_write_header, (m_formatContext, &options));
      av_dict_free(&options);

      return m_headerWritten;
    }


  public:
    PMediaFile_FFMPEG()
      : m_formatContext(NULL)
      , m_headerWritten(false)
    {
    }


    ~PMediaFile_FFMPEG()
    {
    }


    bool IsSupported(const PString & type) const
    {
      return type == Audio() || type == Video();
    }


    bool OpenForReading(const PFilePath & filePath)
    {
      m_reading = true;
      m_filePath = filePath;

      if (CHECK_ERROR(avformat_open_input, (&m_formatContext, filePath, NULL, NULL)))
        return false;

      if (CHECK_ERROR(avformat_find_stream_info, (m_formatContext, NULL)))
        return false;

      m_tracks.resize(m_formatContext->nb_streams);
      for (size_t i = 0; i < m_tracks.size(); ++i) {
        if (!m_tracks[i].Open(this, i))
          return false;
      }

      PTRACE(3, "Opened " << filePath);
      return true;
    }


    bool OpenForWriting(const PFilePath & filePath)
    {
      m_reading = false;
      m_filePath = filePath;
      m_headerWritten = false;

      if (avformat_alloc_output_context2(&m_formatContext, NULL, NULL, filePath) < 0) {
        if (CHECK_ERROR(avformat_alloc_output_context2,(&m_formatContext, NULL, "mpeg", filePath)))
          return false;
        PTRACE(3, "Could not deduce FFMPEG format for \"" << filePath << "\", assuming MPEG");
      }

      if (CHECK_ERROR(avio_open,(&m_formatContext->pb, filePath, AVIO_FLAG_WRITE)))
        return false;

      return true;
    }


    bool IsOpen() const
    {
      return m_formatContext != NULL;
    }


    bool Close()
    {
      PWaitAndSignal mutex(m_mutex);

      if (m_formatContext == NULL)
        return false;

      for (size_t i = 0; i < m_tracks.size(); ++i)
        m_tracks[i].Close();
      m_tracks.clear();

      avformat_close_input(&m_formatContext);
      return true;
    }


    bool GetDefaultTrackInfo(const PCaselessString & type, TrackInfo & info) const
    {
      PWaitAndSignal mutex(m_mutex);

      if (m_formatContext == NULL)
        return false;

      if (type == Audio()) {
        info = TrackInfo(16000, 1);
        const AVCodec * codec = avcodec_find_encoder(m_formatContext->oformat->audio_codec);
        if (codec != NULL) {
          info.m_format = codec->name;
          info.m_channels = 2;
          info.m_rate = codec->supported_samplerates ? codec->supported_samplerates[0] : 16000;
        }
      }
      else if (type == Video()) {
        info = TrackInfo(PVideoFrameInfo::CIF4Width, PVideoFrameInfo::CIF4Height, 25);
        const AVCodec * codec = avcodec_find_encoder(m_formatContext->oformat->video_codec);
        if (codec != NULL) {
          info.m_format = codec->name;
          info.m_rate = codec->supported_framerates ? codec->supported_framerates[0].num : 25;
        }
      }
      return true;
    }


    unsigned GetTrackCount() const
    {
      return m_tracks.size();
    }


    bool GetTracks(TracksInfo & tracks)
    {
      if (m_formatContext == NULL) {
        PTRACE(2, "Cannot get track info as file not open");
        return false;
      }

      tracks.resize(m_tracks.size());
      for (size_t i = 0; i < m_tracks.size(); ++i)
        tracks[i] = m_tracks[i];

      return true;
    }


    bool SetTracks(const TracksInfo & tracks)
    {
      if (m_formatContext == NULL) {
        PTRACE(2, "Cannot set track info as file not open");
        return false;
      }

      if (m_headerWritten) {
        PTRACE(2, "Cannot set track info after file header written");
        return false;
      }

      m_tracks.resize(tracks.size());
      for (size_t i = 0; i < m_tracks.size(); ++i) {
        TrackContext & existingTrack = m_tracks[i];
        if (existingTrack != tracks[i]) {
          existingTrack.Close();
          existingTrack = tracks[i];
          if (!existingTrack.Create(this, i))
            return false;
        }
      }

      return true;
    }


    bool ReadNative(unsigned track, BYTE * data, PINDEX & size, unsigned & frames)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenTrackAndMode(track, true) && m_tracks[track].ReadNative(data, size, frames);
    }


    bool WriteNative(unsigned track, const BYTE * data, PINDEX & size, unsigned & frames)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenTrackAndMode(track, false) && WriteHeader() && m_tracks[track].WriteNative(data, size, frames);
    }


    bool ConfigureAudio(unsigned track, unsigned channels, unsigned sampleRate)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) && m_tracks[track].ConfigureAudio(channels, sampleRate);
    }


    bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) && m_tracks[track].ReadAudio(data, size, length);
    }


    bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) && WriteHeader() && m_tracks[track].WriteAudio(data, length, written);
    }


#if P_VIDEO
    bool ConfigureVideo(unsigned track, const PVideoFrameInfo & frameInfo)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) &&
            (m_reading ? m_tracks[track].ConfigureReadVideo(frameInfo) : m_tracks[track].ConfigureWriteVideo(frameInfo));
    }


    bool ReadVideo(unsigned track, BYTE * data)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) && m_tracks[track].ReadVideo(data);
    }


    bool WriteVideo(unsigned track, const BYTE * data)
    {
      PWaitAndSignal mutex(m_mutex);
      return CheckOpenAndTrack(track) && WriteHeader() && m_tracks[track].WriteVideo(data);
    }
#endif // P_VIDEO
};

static PMediaFile_FFMPEG::FactoryInitialiser PMediaFile_FFMPEG_FactoryInitialiser_instance;


#elif WIN32

#include <vfw.h>
#pragma comment(lib, "vfw32.lib")


static PString FromFOURCC(DWORD fourCC)
{
  char str[5];
  str[0] = (char)fourCC;
  str[1] = (char)(fourCC>>8);
  str[2] = (char)(fourCC>>16);
  str[3] = (char)(fourCC>>24);
  str[4] = '\0';
  return str;
}


class PMediaFile_AVI : public PMediaFile
{
  PCLASSINFO(PMediaFile_AVI, PMediaFile);
protected:
  PDECLARE_MUTEX(m_mutex);
  PAVIFILE   m_file;

#if PTRACING
  static bool IsResultError(PObject * obj, HRESULT result, const char * msg)
  {
    if (result == AVIERR_OK)
      return false;

    if (!PTrace::CanTrace(2))
      return true;

    ostream & strm = PTRACE_BEGIN(2, obj, PTraceModule());
    strm << "Error " << msg << ": ";
    switch (result) {
    case AVIERR_UNSUPPORTED:
      strm << "Unsupported compressor";
      break;

    case AVIERR_NOCOMPRESSOR:
      strm << "No compressor";
      break;

    case AVIERR_BADFORMAT:
      strm << "Bad format";
      break;

    default:
      strm << "Error=0x" << hex << result;
    }
    strm << PTrace::End;

    return true;
  }

#define IS_RESULT_ERROR(owner, result, msg) IsResultError(owner, result, msg)

#else

#define IS_RESULT_ERROR(result, msg) ((result) != AVIERR_OK)

#endif


  struct TrackContext : TrackInfo
  {
    PMediaFile_AVI   * m_owner;
    PAVISTREAM         m_stream;
    LONG               m_position;
    PAVISTREAM         m_compressor;
    PGETFRAME          m_decompressor;
#if P_VIDEO
    PBYTEArray         m_videoBuffer;
    PColourConverter * m_videoConverter;
#endif


    TrackContext()
      : m_owner(NULL)
      , m_stream(NULL)
      , m_position(0)
      , m_compressor(NULL)
      , m_decompressor(NULL)
#if P_VIDEO
      , m_videoConverter(NULL)
#endif
    { }


    TrackContext & operator=(const TrackInfo & info)
    {
      Close();
      TrackInfo::operator=(info);
      return *this;
    }


    bool Open(PMediaFile_AVI * owner, PAVIFILE file, DWORD index)
    {
      m_owner = owner;

      if (IS_RESULT_ERROR(m_owner, AVIFileGetStream(file, &m_stream, 0, index), "getting stream"))
        return false;

      AVISTREAMINFO sinfo;
      if (IS_RESULT_ERROR(m_owner, AVIStreamInfo(m_stream, &sinfo, sizeof(sinfo)), "getting stream info"))
        return false;

      PBYTEArray formatInfo;
      LONG fmtSize;
      if (AVIStreamReadFormat(m_stream, 0, NULL, &fmtSize) == 0) {
        if (IS_RESULT_ERROR(m_owner, AVIStreamReadFormat(m_stream, 0, formatInfo.GetPointer(fmtSize), &fmtSize), "getting stream format info"))
          return false;
      }

      m_format = FromFOURCC(sinfo.fccHandler);
      m_rate = (double)sinfo.dwRate / sinfo.dwScale;
      m_frames = sinfo.dwLength;

      if (sinfo.fccType == streamtypeAUDIO) {
        m_type = Audio();
        m_size = sinfo.dwSampleSize;

        const PCMWAVEFORMAT & wave = formatInfo.GetAs<PCMWAVEFORMAT>(0);
        m_channels = wave.wf.nChannels;

        switch (wave.wf.wFormatTag) {
        case WAVE_FORMAT_PCM:
          m_format = PSOUND_PCM16"PCM-16";
          break;
        case PWAVFile::fmt_GSM:
          m_format = "GSM-06.10";
          break;
        default:
          PTRACE(2, m_owner, "Unsupported audio format: " << wave.wf.wFormatTag);
          return false;
        }
      }
#if P_VIDEO
      else if (sinfo.fccType == streamtypeVIDEO) {
        m_type = Video();

        const BITMAPINFOHEADER & bmh = formatInfo.GetAs<BITMAPINFOHEADER>(0);
        m_width = bmh.biWidth;
        m_height = std::abs(bmh.biHeight);

        if (bmh.biCompression == BI_RGB) {
          m_format = "RGB24";
          m_size = m_width*m_height * 3;
        }
        else {
          m_format = FromFOURCC(bmh.biCompression);
          if (m_format.IsEmpty()) {
            PTRACE(2, m_owner, "Unsupported video compression: " << bmh.biCompression);
            return false;
          }
        }
      }
#endif
      else {
        m_type = FromFOURCC(sinfo.fccType);
      }

      return true;
    }


    bool CreateAudio(PAVIFILE file)
    {
      PTRACE(4, m_owner, "Creating AVI stream for audio format '" << m_format << '\'');

      WAVEFORMATEX fmt;
      fmt.wFormatTag = m_format == "GSM-06.10" ? PWAVFile::fmt_GSM : WAVE_FORMAT_PCM;
      fmt.wBitsPerSample = 16;
      fmt.nChannels = (WORD)m_channels;
      fmt.nSamplesPerSec = (DWORD)m_rate;
      fmt.nBlockAlign = (fmt.nChannels*fmt.wBitsPerSample + 7) / 8;
      fmt.nAvgBytesPerSec = fmt.nSamplesPerSec*fmt.nBlockAlign;
      fmt.cbSize = 0;

      m_size = fmt.nBlockAlign;

      AVISTREAMINFO info;
      memset(&info, 0, sizeof(info));
      info.fccType = streamtypeAUDIO;
      info.dwScale = fmt.nBlockAlign;
      info.dwRate = fmt.nAvgBytesPerSec;
      info.dwSampleSize = fmt.nBlockAlign;
      info.dwQuality = (DWORD)-1;
      strcpy(info.szName, fmt.nChannels == 2 ? "Stereo Audio" : "Mixed Audio");

      if (IS_RESULT_ERROR(m_owner, AVIFileCreateStream(file, &m_stream, &info), "creating AVI audio stream"))
        return false;

      if (IS_RESULT_ERROR(m_owner, AVIStreamSetFormat(m_stream, 0, &fmt, sizeof(fmt)), "setting format of AVI audio stream"))
        return false;

      return true;
    }


    static bool GetVideoCompressorInfo(const PString & format, DWORD & videoCompressorKeyFrameRate, DWORD & videoCompressorQuality)
    {
      if (format.GetLength() != 4)
        return false;

      HIC hic = ICOpen(mmioFOURCC('v', 'i', 'd', 'c'), mmioFOURCC(format[0], format[1], format[2], format[3]), ICMODE_COMPRESS);
      if (hic == NULL)
        return false;

#if PTRACING
      ICINFO info;
      ICGetInfo(hic, &info, sizeof(info));
      PTRACE(4, NULL, PTraceModule(), "Found " << format << ' ' << PString(info.szDescription));
#endif

      ICSendMessage(hic, ICM_GETDEFAULTKEYFRAMERATE, (DWORD_PTR)(LPVOID)&videoCompressorKeyFrameRate, sizeof(DWORD));
      ICSendMessage(hic, ICM_GETDEFAULTQUALITY, (DWORD_PTR)(LPVOID)&videoCompressorQuality, sizeof(DWORD));

      ICClose(hic);
      return true;
    }


    bool CreateVideo(PAVIFILE file)
    {
      DWORD videoCompressorKeyFrameRate, videoCompressorQuality;
      if (m_format.IsEmpty()) {
        static const char * const DefaultCompressor[] =
        { "H264", "XVID", "DIVX", "MPEG", "H263", "IV50", "CVID", "MSVC" }; // Final default to Microsoft Video 1, every system has that!
        for (PINDEX i = 0; i < PARRAYSIZE(DefaultCompressor); ++i) {
          if (GetVideoCompressorInfo(DefaultCompressor[i], videoCompressorKeyFrameRate, videoCompressorQuality)) {
            m_format = DefaultCompressor[i];
            break;
          }
        }
      }
      else {
        if (!GetVideoCompressorInfo(m_format, videoCompressorKeyFrameRate, videoCompressorQuality)) {
          PTRACE(2, m_owner, "AVI file recording does not (yet) support format " << m_format);
          return false;
        }
      }

      PTRACE(4, m_owner, "Creating AVI stream for video format " << m_format);

      AVISTREAMINFO info;
      memset(&info, 0, sizeof(info));
      info.fccType = streamtypeVIDEO;
      info.dwRate = 90000;
      info.dwScale = (DWORD)(info.dwRate / m_rate);
      info.rcFrame.right = m_width;
      info.rcFrame.bottom = m_height;
      info.dwSuggestedBufferSize = m_width*m_height;
      info.dwQuality = videoCompressorQuality;

      return IS_RESULT_ERROR(m_owner, AVIFileCreateStream(file, &m_stream, &info), "creating AVI video stream");
    }


    void Close()
    {
#if OPAL_VIDEO
      delete m_videoConverter;
#endif

      if (m_compressor != NULL)
        AVIStreamRelease(m_compressor);

      if (m_decompressor != NULL)
        AVIStreamGetFrameClose(m_decompressor);

      if (m_stream != NULL)
        AVIStreamRelease(m_stream);
    }


    bool ReadNative(BYTE * data, PINDEX & size, unsigned & samples)
    {
      if (m_stream == NULL)
        return false;

      LONG bytesRead = size;
      LONG samplesRead;
      if (IS_RESULT_ERROR(m_owner,
                          AVIStreamRead(m_stream,
                                        m_position, samples,
                                        data, size,
                                        &bytesRead, &samplesRead),
                          "reading AVI native stream"))
        return false;

      PTRACE(6, m_owner, "Read " << samplesRead << ' ' << m_type << " samples"
             " (" << bytesRead << " bytes) at position " << m_position);
      m_position += samplesRead;
      samples = samplesRead;
      size = bytesRead;
      return true;
    }


    bool WriteNative(const BYTE * data, PINDEX & size, unsigned & samples)
    {
      if (m_stream == NULL)
        return false;

      if (size < (PINDEX)(samples*m_size))
        return false;

      LONG samplesWritten;
      LONG bytesWritten;
      if (IS_RESULT_ERROR(m_owner,
                          AVIStreamWrite(m_stream,
                                         m_position, samples,
                                         const_cast<BYTE *>(data), samples*m_size,
                                         0, &samplesWritten, &bytesWritten),
                          "writing AVI native stream"))
        return false;

      PTRACE(6, m_owner, "Written " << samplesWritten << ' ' << m_type << " samples at " << m_position);
      m_position += samplesWritten;
      samples = samplesWritten;
      size = bytesWritten;
      return true;
    }


    bool ConfigureAudio(unsigned channels, unsigned sampleRate)
    {
      return m_channels == channels && m_rate == sampleRate;
    }


    bool ReadAudio(BYTE * data, PINDEX size, PINDEX & length)
    {
      if (!m_owner->CheckModeAndTrackType(true, Audio(), m_type))
        return false;

      if (m_stream == NULL)
        return false;

      LONG bytesRead, samplesRead;
      if (IS_RESULT_ERROR(m_owner,
                          AVIStreamRead(m_stream,
                                        m_position, size / m_size,
                                        data, size,
                                        &bytesRead, &samplesRead),
                          "reading AVI PCM stream"))
        return false;

      PTRACE(6, m_owner, "Read " << bytesRead << " bytes of PCM at position " << m_position);
      m_position += samplesRead;
      length = bytesRead;
      return true;
    }


    bool WriteAudio(const BYTE * data, PINDEX size, PINDEX & written)
    {
      if (!m_owner->CheckModeAndTrackType(false, Audio(), m_type))
        return false;

      if (m_stream == NULL)
        return false;

      LONG bytesWritten, samplesWritten;
      if (IS_RESULT_ERROR(m_owner,
                          AVIStreamWrite(m_stream,
                                         m_position, size / m_size,
                                         const_cast<BYTE *>(data), size,
                                         0, &samplesWritten, &bytesWritten),
                          "writing AVI PCM stream"))
        return false;

      PTRACE(6, m_owner, "Written " << bytesWritten << " bytes of PCM at " << m_position);
      m_position += samplesWritten;
      written = samplesWritten;
      return true;
    }


#if P_VIDEO
    bool ConfigureReadVideo(const PVideoFrameInfo & frameInfo)
    {
      if (!m_owner->CheckModeAndTrackType(true, Video(), m_type))
        return false;

      m_size = frameInfo.CalculateFrameBytes();
      PVideoFrameInfo rgb(m_width, m_height, "BGR24");

      BITMAPINFOHEADER fmt;
      memset(&fmt, 0, sizeof(fmt));
      fmt.biSize = sizeof(fmt);
      fmt.biCompression = BI_RGB;
      fmt.biWidth = m_width;
      fmt.biHeight = m_height;
      fmt.biBitCount = 24;
      fmt.biPlanes = 1;
      fmt.biSizeImage = rgb.CalculateFrameBytes();

      m_decompressor = AVIStreamGetFrameOpen(m_stream, &fmt);
      if (m_decompressor == NULL) {
        PTRACE(4, m_owner, "Decompressor cannot be found for " << m_format);
        return false;
      }

      if (frameInfo.GetFrameWidth() == m_width &&
          frameInfo.GetFrameHeight() == m_height &&
          frameInfo.GetColourFormat() == rgb.GetColourFormat()) {
        PTRACE(4, m_owner, "Decompressor to RGB created, frame size=" << fmt.biSizeImage);
        return true;
      }

      m_videoConverter = PColourConverter::Create(rgb, frameInfo);
      if (m_videoConverter == NULL) {
        PTRACE(2, m_owner, "Decompressor requires RGB, but cannot create converter to " << frameInfo);
        return false;
      }

      PTRACE(4, m_owner, "Decompressor requires RGB, created converter to " << frameInfo << ", buffer size=" << fmt.biSizeImage);
      return true;
    }


    bool ReadVideo(BYTE * data)
    {
      if (!m_owner->CheckModeAndTrackType(true, Video(), m_type))
        return false;

      if (m_stream == NULL)
        return false;

      if (m_decompressor == NULL && !ConfigureReadVideo(PVideoFrameInfo(m_width, m_height)))
        return false;

      const BYTE * image = (const BYTE *)AVIStreamGetFrame(m_decompressor, ++m_position);
      if (image == NULL) {
        return false;
      }

      if (m_videoConverter == NULL)
        memcpy(data, image, m_size);
      else {
        if (!m_videoConverter->Convert(image, data)) {
          PTRACE(2, m_owner, "Conversion of RGB24 to YUV420P failed!");
          return false;
        }
      }

      PTRACE(6, m_owner, "Read video frame " << m_position << ", size=" << m_size);
      return true;
    }


    bool ConfigureWriteVideo(const PVideoFrameInfo & frameInfo)
    {
      if (!m_owner->CheckModeAndTrackType(false, Video(), m_type))
        return false;

      if (m_compressor != NULL)
        return true;

      DWORD videoCompressorKeyFrameRate, videoCompressorQuality;
      GetVideoCompressorInfo(m_format, videoCompressorKeyFrameRate, videoCompressorQuality);

      AVICOMPRESSOPTIONS opts;
      memset(&opts, 0, sizeof(opts));
      opts.fccType = streamtypeVIDEO;
      opts.fccHandler = mmioFOURCC(m_format[0], m_format[1], m_format[2], m_format[3]);
      opts.dwQuality = videoCompressorQuality;
      opts.dwKeyFrameEvery = videoCompressorKeyFrameRate;
      opts.dwBytesPerSecond = 100000;
      opts.dwFlags = AVICOMPRESSF_KEYFRAMES | AVICOMPRESSF_DATARATE;
      if (IS_RESULT_ERROR(m_owner, AVIMakeCompressedStream(&m_compressor, m_stream, &opts, NULL), "creating AVI video compressor"))
        return false;

      BITMAPINFOHEADER fmt;
      memset(&fmt, 0, sizeof(fmt));
      fmt.biSize = sizeof(fmt);
      fmt.biWidth = m_width;
      fmt.biHeight = m_height;

      if (frameInfo.GetFrameWidth() == m_width &&
          frameInfo.GetFrameHeight() == m_height &&
          frameInfo.GetColourFormat() == PVideoFrameInfo::YUV420P()) {
        fmt.biCompression = mmioFOURCC('I', '4', '2', '0');
        fmt.biBitCount = 12;
        fmt.biPlanes = 3;
        fmt.biSizeImage = frameInfo.CalculateFrameBytes();

        if (AVIStreamSetFormat(m_compressor, 0, &fmt, sizeof(fmt)) == AVIERR_OK) {
          PTRACE(4, m_owner, "Compressor can use YUV420P directly, no converter needed, buffer size=" << fmt.biSizeImage);
          return true;
        }
      }

      PVideoFrameInfo rgb(m_width, m_height, "BGR24");

      fmt.biCompression = BI_RGB;
      fmt.biBitCount = 24;
      fmt.biPlanes = 1;
      fmt.biSizeImage = rgb.CalculateFrameBytes();

      if (IS_RESULT_ERROR(m_owner, AVIStreamSetFormat(m_compressor, 0, &fmt, sizeof(fmt)), "setting format of AVI video compressor"))
        return false;

      if (frameInfo.GetFrameWidth() == m_width &&
          frameInfo.GetFrameHeight() == m_height &&
          frameInfo.GetColourFormat() == PVideoFrameInfo::YUV420P()) {
        PTRACE(4, m_owner, "Compressor from RGB created, frame size=" << fmt.biSizeImage);
        return true;
      }

      m_videoConverter = PColourConverter::Create(frameInfo, rgb);
      if (m_videoConverter == NULL) {
        PTRACE(2, m_owner, "Compressor requires RGB, but cannot create converter from " << frameInfo);
        return false;
      }

      if (!PAssert(m_videoBuffer.SetSize(fmt.biSizeImage), POutOfMemory))
        return false;

      PTRACE(4, m_owner, "Compressor requires RGB, created converter from " << frameInfo << ", buffer size=" << fmt.biSizeImage);
      return true;
    }


    bool WriteVideo(const BYTE * data)
    {
      if (!m_owner->CheckModeAndTrackType(false, Video(), m_type))
        return false;

      if (m_stream == NULL)
        return false;

      if (m_compressor == NULL && !ConfigureWriteVideo(PVideoFrameInfo(m_width, m_height)))
        return false;

      BYTE * bufferPtr;
      PINDEX bufferSize;
      if (m_videoConverter == NULL) {
        bufferPtr = const_cast<BYTE *>(data);
        bufferSize = m_width*m_height * 3 / 2;
      }
      else {
        bufferPtr = m_videoBuffer.GetPointer(m_videoConverter->GetMaxDstFrameBytes());
        bufferSize = m_videoBuffer.GetSize();
        if (!m_videoConverter->Convert(data, bufferPtr, &bufferSize)) {
          PTRACE(2, m_owner, "Conversion of YUV420P to RGB24 failed!");
          return false;
        }
      }

      if (IS_RESULT_ERROR(m_owner,
                          AVIStreamWrite(m_compressor,
                                         ++m_position, 1,
                                         bufferPtr, bufferSize,
                                         0, NULL, NULL),
                          "writing AVI video stream"))
        return false;

      PTRACE(6, m_owner, "Written video frame " << m_position << ", size=" << bufferSize);
      return true;
    }
#endif
  };

  std::vector<TrackContext> m_tracks;


public:
  PMediaFile_AVI()
    : m_file(NULL)
  {
    AVIFileInit();
  }


  ~PMediaFile_AVI()
  {
    Close();
    AVIFileExit();
  }


  bool IsSupported(const PString & type) const
  {
    return type == Audio() || type == Video();
  }


  bool OpenForReading(const PFilePath & filePath)
  {
    m_reading = true;
    m_filePath = filePath;

    if (IS_RESULT_ERROR(this, AVIFileOpen(&m_file, filePath, m_reading ? OF_READ : (OF_WRITE | OF_CREATE), NULL), "opening AVI file"))
      return false;

    AVIFILEINFO finfo;
    if (IS_RESULT_ERROR(this, AVIFileInfo(m_file, &finfo, sizeof(finfo)), "getting file info"))
      return false;

    m_tracks.resize(finfo.dwStreams);
    for (DWORD i = 0; i < finfo.dwStreams; ++i) {
      if (!m_tracks[i].Open(this, m_file, i))
        return false;
    }

    return true;
  }


  bool OpenForWriting(const PFilePath & filePath)
  {
    m_reading = false;
    m_filePath = filePath;
    return !IS_RESULT_ERROR(this, AVIFileOpen(&m_file, filePath, OF_WRITE | OF_CREATE, NULL), "opening AVI file");
  }


  bool IsOpen() const
  {
    return m_file != NULL;
  }


  bool Close()
  {
    m_mutex.Wait();

    for (size_t i = 0; i < m_tracks.size(); ++i)
      m_tracks[i].Close();
    m_tracks.clear();

    if (m_file != NULL) {
      AVIFileRelease(m_file);
      m_file = NULL;
    }

    m_mutex.Signal();

    return true;
  }


  bool GetDefaultTrackInfo(const PCaselessString & type, TrackInfo & info) const
  {
    if (type == Audio()) {
      info = TrackInfo(16000, 1);
    }
    else if (type == Video()) {
      info = TrackInfo(PVideoFrameInfo::CIF4Width, PVideoFrameInfo::CIF4Height, 25);
    }
    return true;
  }


  unsigned GetTrackCount() const
  {
    return m_tracks.size();
  }


  bool GetTracks(TracksInfo & tracks)
  {
    if (m_file == NULL)
      return false;

    tracks.resize(m_tracks.size());
    for (size_t i = 0; i < m_tracks.size(); ++i)
      tracks[i] = m_tracks[i];

    return true;
  }


  bool SetTracks(const TracksInfo & tracks)
  {
    m_tracks.resize(tracks.size());

    for (size_t i = 0; i < tracks.size(); ++i) {
      TrackContext & existingTrack = m_tracks[i];
      if (existingTrack != tracks[i]) {
        existingTrack = tracks[i];
        existingTrack.m_owner = this;

        if (existingTrack.m_type == Audio()) {
          if (!existingTrack.CreateAudio(m_file))
            return false;
        }
        else if (existingTrack.m_type == Video()) {
          if (!existingTrack.CreateVideo(m_file))
            return false;
        }
      }
    }
    return false;
  }


  bool ReadNative(unsigned track, BYTE * data, PINDEX & size, unsigned & frames)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenTrackAndMode(track, true) && m_tracks[track].ReadNative(data, size, frames);
  }


  bool WriteNative(unsigned track, const BYTE * data, PINDEX & size, unsigned & frames)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenTrackAndMode(track, false) && m_tracks[track].WriteNative(data, size, frames);
  }


  bool ConfigureAudio(unsigned track, unsigned channels, unsigned sampleRate)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) && m_tracks[track].ConfigureAudio(channels, sampleRate);
  }


  bool ReadAudio(unsigned track, BYTE * data, PINDEX size, PINDEX & length)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) && m_tracks[track].ReadAudio(data, size, length);
  }


  bool WriteAudio(unsigned track, const BYTE * data, PINDEX length, PINDEX & written)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) && m_tracks[track].WriteAudio(data, length, written);
  }


#if P_VIDEO
  bool ConfigureVideo(unsigned track, const PVideoFrameInfo & frameInfo)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) &&
          (m_reading ? m_tracks[track].ConfigureReadVideo(frameInfo) : m_tracks[track].ConfigureWriteVideo(frameInfo));
  }


  bool ReadVideo(unsigned track, BYTE * data)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) && m_tracks[track].ReadVideo(data);
  }


  bool WriteVideo(unsigned track, const BYTE * data)
  {
    PWaitAndSignal mutex(m_mutex);
    return CheckOpenAndTrack(track) && m_tracks[track].WriteVideo(data);
  }
#endif // P_VIDEO
};

PFACTORY_CREATE(PMediaFile::Factory, PMediaFile_AVI, ".avi");


PStringSet PMediaFile::GetAllFileTypes()
{
  // Can't rely on factory initialisation, as is used by other factories
  static const char * const SuperBasic[] = { ".wav", ".avi" };
  return PStringSet(PARRAYSIZE(SuperBasic), SuperBasic, true);
}

#else // P_FFMPEG_FULL - WIN32

PStringSet PMediaFile::GetAllFileTypes()
{
  // Can't rely on factory initialisation, as is used by other factories
  return ".wav";
}

#endif // P_FFMPEG_FULL


#if P_VIDEO

///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_MediaFile

class PVideoInputDevice_MediaFile : public PMediaFile::VideoInputDevice
{
  PCLASSINFO(PVideoInputDevice_MediaFile, PMediaFile::VideoInputDevice);
public:
  static PStringArray GetInputDeviceNames()
  {
    PStringArray names;

    PVideoFileFactory::KeyList_T ignoreExt = PVideoFileFactory::GetKeyList();

    PMediaFile::Factory::KeyList_T keyList = PMediaFile::Factory::GetKeyList();
    for (PMediaFile::Factory::KeyList_T::iterator it = keyList.begin(); it != keyList.end(); ++it) {
      if (std::find(ignoreExt.begin(), ignoreExt.end(), *it) == ignoreExt.end())
        names.AppendString("*" + *it);
    }

    return names;
  }

  virtual PStringArray GetDeviceNames() const
  {
    return PVideoInputDevice_MediaFile::GetInputDeviceNames();
  }
};


PCREATE_VIDINPUT_PLUGIN_EX(MediaFile,

  virtual const char * GetFriendlyName() const
  {
    return "Media File Video Input";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    return PVideoInputDevice_MediaFile::GetInputDeviceNames().GetValuesIndex("*"+PFilePath(deviceName).GetType()) != P_MAX_INDEX;
  }
);


///////////////////////////////////////////////////////////////////////////////
// PVideoOutputDevice_MediaFile

PCREATE_VIDOUTPUT_PLUGIN_EX(MediaFile,

  virtual const char * GetFriendlyName() const
  {
    return "Media File Video Output";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    PMediaFile::Factory::KeyList_T keyList = PMediaFile::Factory::GetKeyList();
    return std::find(keyList.begin(), keyList.end(), PFilePath(deviceName).GetType()) != keyList.end() &&
           (!PFile::Exists(deviceName) || PFile::Access(deviceName, PFile::WriteOnly));
  }
);


PVideoOutputDevice_MediaFile::PVideoOutputDevice_MediaFile()
  : m_file(NULL)
{
}


PVideoOutputDevice_MediaFile::~PVideoOutputDevice_MediaFile()
{
  Close();
}

static const char DefaultYUVFileName[] = "*.yuv";

PBoolean PVideoOutputDevice_MediaFile::Open(const PString & devName, PBoolean /*startImmediate*/)
{
  PFilePath fileName;
  if (devName != DefaultYUVFileName)
    fileName = devName;
  else {
    unsigned unique = 0;
    do {
      fileName.Empty();
      fileName.sprintf("video%03u.yuv", ++unique);
    } while (PFile::Exists(fileName));
  }

  m_file = PMediaFile::Factory::CreateInstance(fileName.GetType());
  if (m_file == NULL) {
    PTRACE(1, "Cannot create file " << fileName << " as video output device");
    return false;
  }

  if (!m_file->OpenForWriting(fileName)) {
    PTRACE(1, "Error creating file " << fileName << " as video output device");
    delete m_file;
    m_file = NULL;
    return false;
  }

  m_deviceName = m_file->GetFilePath();
  return true;
}

PBoolean PVideoOutputDevice_MediaFile::Close()
{
  bool ok = m_file == NULL || m_file->Close();

  PThread::Sleep(10);

  delete m_file;
  m_file = NULL;

  return ok;
}


PBoolean PVideoOutputDevice_MediaFile::Start()
{
  if (m_file == NULL)
    return false;
 
  PMediaFile::TracksInfo tracks;
  tracks.push_back(PMediaFile::TrackInfo(PMediaFile::Video(), YUV420P()));
  tracks.back().m_width = m_frameWidth;
  tracks.back().m_height = m_frameHeight;
  tracks.back().m_rate = m_frameRate;
  return m_file->SetTracks(tracks);
}


PBoolean PVideoOutputDevice_MediaFile::Stop()
{
  return true;
}


PBoolean PVideoOutputDevice_MediaFile::IsOpen()
{
  return m_file != NULL;
}


PStringArray PVideoOutputDevice_MediaFile::GetOutputDeviceNames()
{
  PStringArray names;

  PMediaFile::Factory::KeyList_T keyList = PMediaFile::Factory::GetKeyList();
  for (PMediaFile::Factory::KeyList_T::iterator it = keyList.begin(); it != keyList.end(); ++it) {
#if P_IMAGEMAGICK
    extern const char * PImageMagickExtensions[];
    const char * const * ext;
    for (ext = PImageMagickExtensions; *ext != NULL; ++ext) {
      if (*it == *ext)
        break;
    }
    if (*ext != NULL)
      continue;
#endif
    names.AppendString("*" + *it);
  }

  return names;
}


PBoolean PVideoOutputDevice_MediaFile::SetColourFormat(const PString & newFormat)
{
  return (newFormat *= PVideoFrameInfo::YUV420P()) && PVideoDevice::SetColourFormat(newFormat);
}


PBoolean PVideoOutputDevice_MediaFile::SetFrameData(const FrameData & frameData)
{
  if (m_file == NULL) {
    PTRACE(5, "Abort SetFrameData, closed.");
    return false;
  }

  if (frameData.x != 0 || frameData.y != 0 || frameData.width != m_frameWidth || frameData.height != m_frameHeight) {
    PTRACE(1, "Output device only supports full frame writes");
    return false;
  }

  if (m_converter == NULL)
    return m_file->WriteVideo(0, frameData.pixels);

  m_converter->Convert(frameData.pixels, m_frameStore.GetPointer(GetMaxFrameBytes()));
  return m_file->WriteVideo(0, m_frameStore);
}


#endif // P_VIDEO

#endif // P_MEDIAFILE
