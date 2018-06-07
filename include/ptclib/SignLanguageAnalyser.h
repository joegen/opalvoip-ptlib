/*
*/

#ifndef SL_HEADER
#define SL_HEADER


#define SL_API_VERSION 1


#if defined(_WIN32)
  #define SL_STDCALL __stdcall
#else
  #define SL_STDCALL
#endif

#ifndef SL_EXPORT
  #define SL_EXPORT
#endif


#ifdef __cplusplus
extern "C" {
#endif

  typedef enum SLVideoFormat {
    SL_GreyScale, /* 8 bits per sample */
    SL_YUV420P,   /* YUV in 420 planar format: all the 8 bit Y samples,
                     followed by half resolution U samples, and then the
                     half resolution V samples. Total size is always
                     width*height*3/2 bytes. */
    SL_RGB24,     /* 24 bits per sample, 8 bits per component, each scan line is
                     rounded to 4 byte boundary. */
    SL_BGR24,     /* 24 bits per sample, 8 bits per component, each scan line is
                     rounded to 4 byte boundary. */
    SL_RGB32,     /* 32 bits per sample, 8 bits per component with the 4th byte
                     unused. */
    SL_BGR32,     /* 32 bits per sample, 8 bits per component with the 4th byte
                     unused. */
  } SLVideoFormat;


  /* Initialise the Sign Langauge analyser system.
     This is called once before any calls to SLAnalyse(). The caller will
     provide an API version number for backward and forward compatibility.
     For example, extra fields could be added to this structure and their
     presence indicated by a new API version number.

     The library should provide to the caller with the video format required
     and the maximum number of simultaneous anaysers that can be used.

     The function return value is zero for success, and a negative number for
     failure.
   */
  typedef struct SLAnalyserInit
  {
    unsigned      m_apiVersion;   // In  - API Version
    SLVideoFormat m_videoFormat;  // Out - Format for video
    unsigned      m_maxInstances; // Out - Maximum number of simultanoues instances
  } SLAnalyserInit;

  SL_EXPORT int SL_STDCALL SLInitialise(SLAnalyserInit * init);
  typedef int(*SLInitialiseFn)(SLAnalyserInit * init);

  /* Clean up the Sign Language analyser system.
     This is called whe the user no longer requires the DLL. It will allow the
     library to clean up any resources it uses.

     The function return value is zero for success, and a negative number for
     failure.
  */
  SL_EXPORT int SL_STDCALL SLRelease();
  typedef int (*SLReleaseFn)();


  /* Analyse video frame data.
      This will be called repeatedly as video frames become available. Note, that
      there is no guarantee that frames arrive at any particular rate, and the
      resolution can also change at any time. Fields are provided to indicate
      the current values for those attributes.

      The pointer m_videoData will point to block of memory that contains the
      video frame in the format indicated during initialisation. See the
      SLVideoFormat enum for more details on thee format. Note, that upon return
      from this function, the memory block may be reused, the library should not
      access this memory internally via any background threads.

      The return value is a code for the detected sign, zero for nothing detected,
      and a negative number for a fatal error. It is expected that, at least for
      basic signs, the standard ANSI codes are used, e.g. 49 for "1", 50 for "2"
      etc. Special codes for non-fatal errors such as "Room too dark", "Too far
      from camera", etc, may also be returned at values > 65536.
    */

  typedef struct SLAnalyserData
  {
    unsigned     m_instance;  // In - Index of the analyser instance, value from 0 .. m_maxInstances-1
    unsigned     m_width;     // In - Width of video frame
    unsigned     m_height;    // In - Height of video frame
    unsigned     m_timestamp; // In - Timestamp in microseconds from some arbitrary starting point
    const void * m_pixels;    // In - Video pixel data
  } SLAnalyserData;

  SL_EXPORT int SL_STDCALL SLAnalyse(const SLAnalyserData * data);
  typedef int (*SLAnalyseFn)(const SLAnalyserData * data);


  /** Get preview video frame data.
     This will be called repeatedly as video frames are required to be sent. A
     buffer large enought for the specified width, height and video format as
     defined by SLInitialise() is provided in m_videoData, It is to be filled
     in by the library with the annotated video during analysis.

     The function return value is 1 for video was returned, zero for no video
     available at this time, and a negative number for failure.
   */

  typedef struct SLPreviewData
  {
    unsigned m_instance;  // In - Index of the analyser instance, value from 0 .. m_maxInstances-1
    unsigned m_width;     // In - Width of video frame returned from SL
    unsigned m_height;    // In - Height of video frame returned from SL
    void *   m_pixels;    // In/Out - Video pixel data
  } SLPreviewData;

  SL_EXPORT int SL_STDCALL SLPreview(const SLPreviewData * data);
  typedef int (*SLPreviewFn)(const SLPreviewData * data);


#ifdef __cplusplus
};
#endif

#endif // SL_HEADAER

///////////////////////////////////////////////////////////////////////
