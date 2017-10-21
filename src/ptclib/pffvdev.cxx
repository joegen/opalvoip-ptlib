/*
 * pffvdev.cxx
 *
 * Video device for ffmpeg
 *
 * Portable Windows Library
 *
 * Copyright (C) 2008 Post Increment
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

#ifdef __GNUC__
#pragma implementation "pffvdev.h"
#endif

#include <ptlib.h>

#if P_VIDEO
#if P_FFVDEV

#define P_FORCE_STATIC_PLUGIN 1

#include <ptlib/vconvert.h>
#include <ptclib/pffvdev.h>
#include <ptclib/pvfiledev.h>
#include <ptlib/pfactory.h>
#include <ptlib/pluginmgr.h>
#include <ptlib/videoio.h>

#define new PNEW
#define PTraceModule() "FFMPEGVideo"


///////////////////////////////////////////////////////////////////////////////

PCREATE_VIDINPUT_PLUGIN_EX(FFMPEG,
  PStringSet m_extensions;

  PPlugin_PVideoInputDevice_FFMPEG();

  virtual const char * GetFriendlyName() const
  {
    return "File Video source using FFMPEG";
  }

  virtual bool ValidateDeviceName(const PString & deviceName, P_INT_PTR /*userData*/) const
  {
    return m_extensions.Contains("*" + PFilePath(deviceName).GetType());
  }
);


#if P_FFMPEG_FULL
  extern "C" {
    P_PUSH_MSVC_WARNINGS(4244)
    #include <libavformat/avformat.h>
    #include <libavutil/imgutils.h>
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

  PPlugin_PVideoInputDevice_FFMPEG::PPlugin_PVideoInputDevice_FFMPEG()
  {
    av_register_all();

#if P_VIDFILE
    PStringSet extensionsAlreadyAvailable = PVideoInputDevice_VideoFile::GetInputDeviceNames();
#endif

    AVInputFormat *fmt = NULL;
    while ((fmt = av_iformat_next(fmt)) != NULL) {
      PStringArray extensions(PString(fmt->extensions).Tokenise(","));
      for (PINDEX i = 0; i < extensions.GetSize(); ++i) {
        PString ext = "*." + extensions[i];
#if P_VIDFILE
        if (extensionsAlreadyAvailable.Contains(ext))
          continue;
#endif
        m_extensions += ext;
      }
    }
  }


  struct PVideoInputDevice_FFMPEG::Implementation : PVideoFrameInfo
  {
    AVFormatContext * m_formatContext;
    int               m_streamIndex;
    AVCodecContext  * m_codecContext;
    AVFrame         * m_frame;
    unsigned          m_frameCount;

    Implementation()
      : m_formatContext(NULL)
      , m_streamIndex(-1)
      , m_codecContext(NULL)
      , m_frame(NULL)
      , m_frameCount(0)
    {
    }

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
    #define CHECK_ERROR(fn,args) CheckError(fn args, #fn)
#else
    #define CHECK_ERROR(fn,args) fn args
#endif

    bool Open(const PString & filename)
    {
      if (CHECK_ERROR(avformat_open_input, (&m_formatContext, filename, nullptr, nullptr)))
        return false;

      if (CHECK_ERROR(avformat_find_stream_info, (m_formatContext, nullptr)))
        return false;

      m_streamIndex = av_find_best_stream(m_formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
      if (m_streamIndex < 0) {
        PTRACE(2, "No video stream found in " << filename);
        return false;
      }

      AVStream * stream = m_formatContext->streams[m_streamIndex];
      AVCodecParameters * codecParams = stream->codecpar;
      AVCodec * codec = avcodec_find_decoder(codecParams->codec_id);
      if (codec == NULL) {
        PTRACE(2, "Unsupported video codec " << avcodec_get_name(codecParams->codec_id) << " in " << filename);
        return false;
      }

      m_codecContext = avcodec_alloc_context3(codec);
      if (m_codecContext == NULL) {
        PTRACE(2, "Could not allocate codec context for " << filename);
        return false;
      }

      if (CHECK_ERROR(avcodec_parameters_to_context, (m_codecContext, codecParams)))
        return false;

      m_codecContext->pix_fmt = AV_PIX_FMT_YUV420P;
      m_codecContext->framerate = av_guess_frame_rate(m_formatContext, stream, NULL);

      AVDictionary * options = nullptr;
      av_dict_set(&options, "threads", "1", 0);
      //av_dict_set(&options, "refcounted_frames", "1", 0);
      bool bad = CHECK_ERROR(avcodec_open2, (m_codecContext, codec, &options));
      av_dict_free(&options);
      if (bad) {
        PTRACE(2, "Failed to open video codec " << avcodec_get_name(codecParams->codec_id) << " in " << filename);
        return false;
      }

      m_frame = av_frame_alloc();
      if (m_frame == NULL) {
        PTRACE(2, "Could not allocate frame for " << filename);
        return false;
      }

      m_frameWidth = m_codecContext->width;
      m_frameHeight = m_codecContext->height;
      m_frameRate = (unsigned)av_q2d(m_codecContext->framerate);
      m_frameCount = 0;

      PTRACE(3, "Opened " << filename);
      return true;
    }

    void Close()
    {
      avcodec_free_context(&m_codecContext);
      avformat_close_input(&m_formatContext);
      av_frame_free(&m_frame);
    }

    bool IsOpen()
    {
      return m_frame != nullptr;
    }

    bool ReadFrame(BYTE * readBuffer)
    {
      if (!IsOpen())
        return false;

      AVPacket packet;
      av_init_packet(&packet);

      int result;
      while ((result = avcodec_receive_frame(m_codecContext, m_frame)) < 0) {
        if (result != AVERROR(EAGAIN) && result == AVERROR_EOF)
          return false;

        do {
          if (CHECK_ERROR(av_read_frame, (m_formatContext, &packet)))
            return false;
        } while (packet.stream_index != m_streamIndex);

        if (packet.pts == AV_NOPTS_VALUE)
          packet.pts = packet.dts = m_frameCount;

        if (CHECK_ERROR(avcodec_send_packet, (m_codecContext, &packet)))
          return false;
      }

      if (m_frameWidth != (unsigned)m_frame->width || m_frameHeight != (unsigned)m_frame->height) {
        PTRACE(2, "Change of video size in mid playback not supported.");
        return false;
      }

      unsigned planeWidth = (m_frameWidth+1)&~1;
      unsigned planeHeight = (m_frameHeight+1)&~1;

      av_image_copy_plane(readBuffer, planeWidth, m_frame->data[0], m_frame->linesize[0], m_frameWidth, m_frameHeight);
      readBuffer += planeWidth*planeHeight;
      planeWidth /= 2;
      planeHeight /= 2;
      av_image_copy_plane(readBuffer, planeWidth, m_frame->data[1], m_frame->linesize[1], planeWidth, planeHeight);
      readBuffer += planeWidth*planeHeight;
      av_image_copy_plane(readBuffer, planeWidth, m_frame->data[2], m_frame->linesize[2], planeWidth, planeHeight);

      ++m_frameCount;
      return true;
    }
  };

#else // P_FFMPEG_FULL

  #if _WIN32
    static const char * ffmpegExe = "ffmpeg.exe";
  #else
    static const char * ffmpegExe = "ffmpeg";
  #endif

  static const char * const ffmpegExtensions[] = {
    #ifndef _WIN32
      "*.avi",
    #endif
      "*.mp4", "*.mpg", "*.wmv", "*.mov"
  };

  PPlugin_PVideoInputDevice_FFMPEG::PPlugin_PVideoInputDevice_FFMPEG()
    : m_extensions(PARRAYSIZE(ffmpegExtensions), ffmpegExtensions)
  {
  }


  struct PVideoInputDevice_FFMPEG::Implementation : PVideoFrameInfo
  {
    PPipeChannel m_command;

    bool Open(const PString & filename)
    {
      m_frameWidth = m_frameHeight = 0;
      m_frameRate = 25;

      PString cmd = PString(ffmpegExe) & "-i" & filename & "-f rawvideo -";

      // file information comes in on stderr
      if (!m_command.Open(cmd, PPipeChannel::ReadOnly, true, true)) {
        PTRACE(1, "Cannot open command " << cmd);
        return false;
      }

      //if (!m_command.Execute()) {
      //  PTRACE(1, "VidFFMPEG\tCannot execute command " << cmd);
      //  return false;
      //}

      // parse out file size information
      {
        int state = 0;
        PString text;
        PString line;
        PINDEX offs = 0, len = 0;
        while (m_command.IsOpen() && state != -1) {
          if (offs == len) {
            if (!m_command.ReadStandardError(text, true)) {
              PTRACE(1, "Failure while reading file information for " << cmd);
              return false;
            }
            offs = 0;
            len = text.GetLength();
          }
          else {
            char ch = text[offs++];
            if (ch == '\n') {
              line = line.Trim();
              // Stream #0.0: Video: mpeg4, yuv420p, 640x352 [PAR 1:1 DAR 20:11], 25.00 tb(r)
              if (line.Left(8) *= "Stream #") {
                PStringArray tokens = line.Tokenise(' ', false);
                if (tokens.GetSize() >= 6 && (tokens[2] *= "Video:")) {
                  PString size = tokens[5];
                  PINDEX x = size.Find('x');
                  if (x != P_MAX_INDEX) {
                    m_frameWidth = size.Left(x).AsUnsigned();
                    m_frameHeight = size.Mid(x+1).AsUnsigned();
                    PTRACE(3, "Video size parsed as " << m_frameWidth << "x" << m_frameHeight);
                    state = -1;
                  }
                  if (tokens.GetSize() >= 11) {
                    m_frameRate = tokens[10].AsUnsigned();
                    PTRACE(3, "Video frame rate parsed as " << m_frameRate);
                  }
                }
              }
              line.MakeEmpty();
            }
            else if (ch != '\n')
              line += ch;
          }
        }
      }

      // file is now open
      return true;
    }

    void Close()
    {
      m_command.Close();
    }

    bool IsOpen()
    {
      return m_command.IsOpen();
    }

    bool ReadFrame(BYTE * readBuffer)
    {
      // make sure that stderr is emptied, as too much unread data 
      // will cause ffmpeg to silently stop 
      {
        PString text;
        m_command.ReadStandardError(text, false);
        PTRACE(5, text);
      }

      PINDEX frameBytes = CalculateFrameBytes();
      PINDEX len = 0;
      while (len < frameBytes) {
        if (!m_command.Read(readBuffer+len, frameBytes-len)) {
          m_command.Close();
          return false;
        }
        len += m_command.GetLastReadCount();
      }

      return true;
    }
  };

#endif // P_FFMPEG_FULL



///////////////////////////////////////////////////////////////////////////////
// PVideoInputDevice_FFMPEG

PVideoInputDevice_FFMPEG::PVideoInputDevice_FFMPEG()
  : m_implementation(new Implementation)
{
}


PVideoInputDevice_FFMPEG::~PVideoInputDevice_FFMPEG()
{
  Close();
  delete m_implementation;
}


PBoolean PVideoInputDevice_FFMPEG::Open(const PString & deviceName, PBoolean /*startImmediate*/)
{
  Close();

  if (!PFile::Access(deviceName, PFile::ReadOnly)) {
    PTRACE(2, deviceName << " does not exist, or cannot be read due to permissions.");
    return false;
  }

  PWaitAndSignal lock(m_mutex);

  if (!m_implementation->Open(deviceName))
    return false;

  *static_cast<PVideoFrameInfo *>(this) = *static_cast<PVideoFrameInfo *>(m_implementation);
  m_deviceName = deviceName;
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::IsOpen() 
{
  return m_implementation->IsOpen();
}


PBoolean PVideoInputDevice_FFMPEG::Close()
{
  PWaitAndSignal lock(m_mutex);

  m_implementation->Close();
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::Start()
{
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::Stop()
{
  return true;
}


PBoolean PVideoInputDevice_FFMPEG::IsCapturing()
{
  return IsOpen();
}


PStringArray PVideoInputDevice_FFMPEG::GetInputDeviceNames()
{
  const PPlugin_PVideoInputDevice_FFMPEG * plugin = dynamic_cast<const PPlugin_PVideoInputDevice_FFMPEG *>(
                            PPluginManager::GetPluginManager().GetServiceDescriptor(PPlugin_PVideoInputDevice_FFMPEG::ServiceName(),
                                                                                    PPlugin_PVideoInputDevice_FFMPEG::ServiceType()));
  PStringArray names(PAssertNULL(plugin)->m_extensions.size());
  PINDEX count = 0;
  for (PStringSet::const_iterator ext = plugin->m_extensions.begin(); ext != plugin->m_extensions.end(); ++ext)
    names[count++] = *ext;
  return names;
}


PBoolean PVideoInputDevice_FFMPEG::SetColourFormat(const PString & newFormat)
{
  return newFormat *= PVideoFrameInfo::YUV420P();
}


PBoolean PVideoInputDevice_FFMPEG::GetFrameSizeLimits(unsigned & minWidth,
                                           unsigned & minHeight,
                                           unsigned & maxWidth,
                                           unsigned & maxHeight) 
{
  // can't set unless the file is open
  if (!IsOpen())
    return false;

  m_implementation->GetFrameSize(maxWidth, maxHeight);
  minWidth  = maxWidth;
  minHeight = maxHeight;
  return true;
}

PBoolean PVideoInputDevice_FFMPEG::SetFrameSize(unsigned width, unsigned height)
{
  return m_implementation->IsOpen() && width == m_frameWidth && height == m_frameHeight;
}


PINDEX PVideoInputDevice_FFMPEG::GetMaxFrameBytes()
{
  return GetMaxFrameBytesConverted(m_implementation->CalculateFrameBytes());
}


PBoolean PVideoInputDevice_FFMPEG::GetFrameData(BYTE * buffer, PINDEX * bytesReturned)
{    
  m_pacing.Delay(1000/m_frameRate);    
  return GetFrameDataNoDelay(buffer, bytesReturned);
}

 
PBoolean PVideoInputDevice_FFMPEG::GetFrameDataNoDelay(BYTE *destFrame, PINDEX * bytesReturned)
{
  PWaitAndSignal lock(m_mutex);

  if (!IsOpen())
    return false;

  if (m_converter != NULL) {
    if (!m_implementation->ReadFrame(m_frameStore.GetPointer(m_implementation->CalculateFrameBytes())))
      return false;

    return m_converter->Convert(m_frameStore, destFrame, bytesReturned);
  }

  if (!m_implementation->ReadFrame(destFrame))
    return false;

  if (bytesReturned != NULL)
    *bytesReturned = m_implementation->CalculateFrameBytes();

  return true;
}

#endif // #if P_FFVDEV
#endif // #if P_VIDEO
