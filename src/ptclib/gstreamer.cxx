/*
 * gstreamer.cxx
 *
 * Interface classes for Gstreamer.
 *
 * Portable Tools Library
 *
 * Copyright (c) 2011 Vox Lucida Pty. Ltd.
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
 * The Initial Developer of the Original Code is Vox Lucida Pty. Ltd.
 *
 * Contributor(s): ______________________________________.
 *
 * $Revision$
 * $Author$
 * $Date$
 */

#ifdef __GNUC__
#pragma implementation "gstreamer.h"
#endif

#include <ptlib.h>

#include <ptclib/gstreamer.h>

#include <ptlib/pprocess.h>


#if P_GSTREAMER

#ifdef _MSC_VER
  #pragma warning(disable:4127)
  #include <gst/gst.h>
  #pragma warning(default:4127)

  #pragma comment(lib, P_GSTREAMER_LIBRARY)
  #pragma comment(lib, P_GST_APP_LIBRARY)
  #pragma comment(lib, P_GOBJECT_LIBRARY)
  #pragma comment(lib, P_GLIB_LIBRARY)
  #if GST_CHECK_VERSION(1,0,0)
    #pragma comment(lib, P_GIO_LIBRARY)
  #endif
#else
  #include <gst/gst.h>
#endif

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#if !GST_CHECK_VERSION(1,0,0)
#include <gst/app/gstappbuffer.h>
#else
/* GST_PLUGIN_FEATURE_NAME() was removed, use GST_OBJECT_NAME() instead. */
#define GST_PLUGIN_FEATURE_NAME(f) GST_OBJECT_NAME(f)

#include <gio/gio.h>
#endif

class PGError
{
  GError * m_error;
public:
  PGError()
    : m_error(NULL)
  {
  }

  ~PGError()
  {
    if (m_error != NULL)
      g_error_free(m_error);
  }

  GError ** operator&() { return &m_error; }
  operator bool() const  { return m_error != NULL; }
  bool operator!() const { return m_error == NULL; }

  friend ostream & operator<<(ostream & strm, const PGError & error)
  {
    if (error.m_error != NULL)
      strm << error.m_error->message;
    return strm;
  }
};



#if PTRACING
static void LogFunction(GstDebugCategory * /*category*/,
                        GstDebugLevel gstLevel,
                        const gchar *file,
                        const gchar *function,
                        gint line,
                        GObject * /*object*/,
                        GstDebugMessage *message,
                        gpointer /*data*/) G_GNUC_NO_INSTRUMENT;
  static void LogFunction(GstDebugCategory * /*category*/,
                        GstDebugLevel gstLevel,
                        const gchar *file,
                        const gchar *function,
                        gint line,
                        GObject * /*object*/,
                        GstDebugMessage *message,
                        gpointer /*data*/)
{
  // GStreamers DEBUG level is a little too verbose for our level 5
  unsigned ptLevel = gstLevel >= GST_LEVEL_DEBUG ? 6 : gstLevel;
  if (PTrace::CanTrace(ptLevel))
    PTrace::Begin(ptLevel, file+1, line, NULL)
          << "GStreamer\t"
          << function << ": "
          << gst_debug_message_get(message)
          << PTrace::End;
}
#endif


static atomic<bool> g_initialised;

static void DeinitialiseGstreamer()
{
#if PTRACING
  gst_debug_set_active(false);
  gst_debug_set_default_threshold(GST_LEVEL_NONE);
  gst_debug_remove_log_function(LogFunction);
#endif

  gst_deinit();
}


static void InitialiseGstreamer()
{
  if (g_initialised.exchange(true))
    return;

  if (gst_is_initialized())
    return;

  PGError error;
  if (gst_init_check(NULL, NULL, &error)) {
    PTRACE(3, "GStreamer\tUsing version " << gst_version_string());

#if PTRACING
#ifdef _MSC_VER
#pragma warning(disable:4127)
#endif
    gst_debug_remove_log_function(gst_debug_log_default);
#if !GST_CHECK_VERSION(1,0,0)
    gst_debug_add_log_function(LogFunction, NULL);
#else
    gst_debug_add_log_function(LogFunction, NULL, NULL);
#endif
#ifdef _MSC_VER
#pragma warning(default:4127)
#endif

    gst_debug_set_default_threshold(GST_LEVEL_DEBUG);
    gst_debug_set_active(true);
#endif

    atexit(DeinitialiseGstreamer);
  }
  else {
    PTRACE(1, "GStreamer\tCould not initialise, error: " << error);
  }
}


///////////////////////////////////////////////////////////////////////

PGBaseObject::PGBaseObject()
  : m_object(NULL)
{
  InitialiseGstreamer();
}


PGBaseObject::~PGBaseObject()
{
  PAssert(m_object == NULL, "GStreamer object should be unreferenced in derived class.");
}


bool PGBaseObject::IsValid() const
{
  return m_object != NULL;
}


bool PGBaseObject::Attach(void * object)
{
  Unreference();
  m_object = object;
  return IsValid();
}


void * PGBaseObject::Detach()
{
  void * object = m_object;
  m_object = NULL;
  return object;
}


void PGBaseObject::SetNULL()
{
  Unreference();
  m_object = NULL;
}


///////////////////////////////////////////////////////////////////////

PGObject::PGObject(const PGObject & other)
{
  Attach(gst_object_ref(other.Ptr()));
}


void PGObject::operator=(const PGObject & other)
{
  Attach(gst_object_ref(other.Ptr()));
}


bool PGObject::IsValid() const
{
  return PGBaseObject::IsValid() && G_IS_OBJECT(Ptr());
}


void PGObject::Unreference()
{
  if (IsValid())
    gst_object_unref(Ptr());
}


bool PGObject::Set(const char * attribute, bool value)
{
  if (!IsValid())
    return false;

  g_object_set(Ptr(), attribute, value, NULL);
  return true;
}


bool PGObject::Get(const char * attribute, bool & value)
{
  if (!IsValid())
    return false;

  g_object_get(Ptr(), attribute, &value, NULL);
  return true;
}


bool PGObject::Set(const char * attribute, int value)
{
  if (!IsValid())
    return false;

  g_object_set(Ptr(), attribute, value, NULL);
  return true;
}


bool PGObject::Get(const char * attribute, int & value)
{
  if (!IsValid())
    return false;

  g_object_get(Ptr(), attribute, &value, NULL);
  return true;
}


bool PGObject::Set(const char * attribute, double value)
{
  if (!IsValid())
    return false;

  g_object_set(Ptr(), attribute, value, NULL);
  return true;
}


bool PGObject::Get(const char * attribute, double & value)
{
  if (!IsValid())
    return false;

  g_object_get(Ptr(), attribute, &value, NULL);
  return true;
}


bool PGObject::Set(const char * attribute, const char * value)
{
  if (!IsValid())
    return false;

  g_object_set(Ptr(), attribute, value, NULL);
  return true;
}


bool PGObject::Get(const char * attribute, PString & value)
{
  if (!IsValid())
    return false;

  gchar * ptr;
  g_object_get(Ptr(), attribute, &ptr, NULL);
  value = ptr;
  g_free(ptr);
  return true;
}


///////////////////////////////////////////////////////////////////////

PString PGstObject::GetName() const
{
  gchar * ptr = gst_object_get_name(As<GstObject>());
  PString str(ptr);
  g_free(ptr);
  return str;
}

bool PGstObject::SetSockFd(int fd)
{
  if (!IsValid())
    return false;

#if !GST_CHECK_VERSION(1,0,0)
  g_object_set(Ptr(), "sockfd", fd, "closefd", FALSE, NULL);
#else
  GSocket * socket = g_socket_new_from_fd (fd, NULL);
  if (!socket)
    return false;
  g_object_set(Ptr(), "socket", socket, "close-socket", FALSE, NULL);
  g_object_unref(socket);
#endif

  return true;
}


///////////////////////////////////////////////////////////////////////

PGstMiniObject::PGstMiniObject(const PGstMiniObject & other)
{
  Attach(gst_mini_object_ref(other.As<GstMiniObject>()));
}


void PGstMiniObject::operator=(const PGstMiniObject & other)
{
  Attach(gst_mini_object_ref(other.As<GstMiniObject>()));
}


bool PGstMiniObject::IsValid() const
{
#if !GST_CHECK_VERSION(1,0,0)
  return PGBaseObject::IsValid() && GST_IS_MINI_OBJECT(Ptr());
#else
  return PGBaseObject::IsValid() && GST_IS_MINI_OBJECT_TYPE(Ptr(), GST_MINI_OBJECT_TYPE(Ptr()));
#endif
}


void PGstMiniObject::Unreference()
{
  if (IsValid())
    gst_mini_object_unref(As<GstMiniObject>());
}


///////////////////////////////////////////////////////////////////////

PString PGstPluginFeature::GetName() const
{
  if (IsValid())
    return gst_plugin_feature_get_name(As<GstPluginFeature>());
  else
    return PString::Empty();
}


PStringList PGstPluginFeature::Inspect(const char * theRegex, InspectSearchField searchField, bool detailed)
{
  PStringList elements;

  InitialiseGstreamer();

  PRegularExpression regex(theRegex == NULL || *theRegex == '\0' ? ".*" : theRegex);

#if !GST_CHECK_VERSION(1,0,0)
  GstRegistry * registry = gst_registry_get_default();
#else
  GstRegistry * registry = gst_registry_get();
#endif

  GList * pluginList = gst_registry_get_plugin_list(registry);
  for (GList * pluginIter = pluginList; pluginIter != NULL; pluginIter = g_list_next(pluginIter)) {
    GstPlugin * plugin = GST_PLUGIN(pluginIter->data);

    GList * featureList = gst_registry_get_feature_list_by_plugin(registry, gst_plugin_get_name(plugin));
    for (GList * featureIter = featureList; featureIter != NULL; featureIter = g_list_next(featureIter)) {
      GstPluginFeature * feature = GST_PLUGIN_FEATURE(featureIter->data);

      if (GST_IS_ELEMENT_FACTORY(feature)) {
        GstElementFactory * factory = GST_ELEMENT_FACTORY(feature);
        PINDEX found = P_MAX_INDEX;
        switch (searchField) {
          case ByKlass :
            regex.Execute(gst_element_factory_get_klass(factory), found);
            break;
          case ByName :
            regex.Execute(GST_PLUGIN_FEATURE_NAME(factory), found);
            break;
          case ByLongName :
            regex.Execute(gst_element_factory_get_longname(factory), found);
            break;
          case ByDescription :
            regex.Execute(gst_element_factory_get_description(factory), found);
            break;
        }
        if (found != P_MAX_INDEX) {
          PStringStream info;
          info << GST_PLUGIN_FEATURE_NAME(factory);
          if (detailed) {
            info << '\t' << gst_element_factory_get_klass(factory)
                 << '\t' << gst_element_factory_get_longname(factory)
                 << '\t' << gst_element_factory_get_description(factory);
          }
          elements.AppendString(info);
        }
      }
    }

    gst_plugin_feature_list_free(featureList);
  }

  gst_plugin_list_free(pluginList);

  return elements;
}


///////////////////////////////////////////////////////////////////////

PGstElementFactory::PGstElementFactory(const char * factoryName)
{
  if (IsValid())
    Attach(gst_element_factory_find(factoryName));
}


PString PGstElementFactory::GetKlass() const
{
  if (IsValid())
    return gst_element_factory_get_klass(As<GstElementFactory>());
  else
    return PString::Empty();
}


PString PGstElementFactory::GetDescription() const
{
  if (IsValid())
    return gst_element_factory_get_description(As<GstElementFactory>());
  else
    return PString::Empty();
}


///////////////////////////////////////////////////////////////////////

PGstElement::PGstElement(const char * factoryName, const char * name)
{
  Attach(gst_element_factory_make(factoryName, name));
}


PString PGstElement::GetName() const
{
  return IsValid() ? PString(gst_element_get_name(As<GstElement>())) : PString::Empty();
}


bool PGstElement::SetName(const PString & name)
{
  return gst_element_set_name(As<GstElement>(), name);
}


bool PGstElement::Link(const PGstElement & dest)
{
  return IsValid() && dest.IsValid() && gst_element_link(As<GstElement>(), dest.As<GstElement>());
}


PGstElement::StateResult PGstElement::SetState(States newState, const PTimeInterval & timeout)
{
  if (!IsValid())
    return Failed;

  StateResult result = (StateResult)gst_element_set_state(As<GstElement>(), (GstState)newState);

  if (timeout > 0) {
    PSimpleTimer timer = timeout;
    while (result == PGstElement::Changing && timer.IsRunning()) {
      States pendingState, currentState;
      if (GetPendingState(pendingState) == Failed || GetState(currentState) == Failed)
        return Failed;
      if (currentState == newState)
        return Success;

      if (pendingState != newState) {
        PTRACE(2, "State change inconsistency: intended=" << newState << ", actual=" << pendingState);
        return Changing;
      }

      PTRACE(4, "Awaiting state change from " << currentState << " to " << pendingState << ", timeout=" << timeout);
      result = WaitStateChange(timeout);
    }
  }

  return result;
}


PGstElement::StateResult PGstElement::GetState(States & state)
{
  return IsValid() ? (StateResult)gst_element_get_state(As<GstElement>(), (GstState *)&state, NULL, 0) : Failed;
}


PGstElement::StateResult PGstElement::GetPendingState(States & state)
{
  return IsValid() ? (StateResult)gst_element_get_state(As<GstElement>(), NULL, (GstState *)&state, 0) : Failed;
}


PGstElement::StateResult PGstElement::WaitStateChange()
{
  return IsValid() ? (StateResult)gst_element_get_state(As<GstElement>(), NULL, NULL, GST_CLOCK_TIME_NONE) : Failed;
}


PGstElement::StateResult PGstElement::WaitStateChange(const PTimeInterval & timeout)
{
  return IsValid() ? (StateResult)gst_element_get_state(As<GstElement>(), NULL, NULL,
                                                        timeout.GetMilliSeconds()*1000000) : Failed;
}


///////////////////////////////////////////////////////////////////////

PGstBaseIterator::PGstBaseIterator(PGBaseObject & valueRef)
  : m_valueRef(valueRef)
  , m_lastResult(Done)
{
}


bool PGstBaseIterator::Attach(void * object)
{
  if (!PGBaseObject::Attach(object))
    return false;

  InternalNext();
  return true;
}


void PGstBaseIterator::Unreference()
{
  if (IsValid())
    gst_iterator_free(As<GstIterator>());
}


void PGstBaseIterator::Resync()
{
  if (IsValid())
    gst_iterator_resync(As<GstIterator>());
}


PGstBaseIterator::Result PGstBaseIterator::InternalNext()
{
  gpointer valuePtr;

#if !GST_CHECK_VERSION(1,0,0)
  m_lastResult = (Result)gst_iterator_next(As<GstIterator>(), &valuePtr);
#else
  GValue value = G_VALUE_INIT;
  m_lastResult = (Result)gst_iterator_next(As<GstIterator>(), &value);
  valuePtr = g_value_get_object(&value);
  g_value_unset(&value);
#endif

  if (m_lastResult == Success)
    m_valueRef.Attach(valuePtr);
  return m_lastResult;
}


///////////////////////////////////////////////////////////////////////

bool PGstBin::AddElement(const PGstElement & element)
{
  return IsValid() && element.IsValid() && gst_bin_add(As<GstBin>(), element.As<GstElement>());
}


bool PGstBin::GetByName(const char * name, PGstElement & element) const
{
  return IsValid() && element.Attach(gst_bin_get_by_name(As<GstBin>(), name));
}


bool PGstBin::GetElements(PGstIterator<PGstElement> & iterator, bool recursive) const
{
  return IsValid() && iterator.Attach(recursive ? gst_bin_iterate_recurse(As<GstBin>())
                                                : gst_bin_iterate_elements(As<GstBin>()));
}


///////////////////////////////////////////////////////////////////////

PGstAppSrc::PGstAppSrc(const PGstBin & bin, const char * name)
{
  bin.GetByName(name, *this);
}


void PGstAppSrc::SetType(Types type)
{
  gst_app_src_set_stream_type(As<GstAppSrc>(), (GstAppStreamType)type);
}


PGstAppSrc::Types PGstAppSrc::GetType() const
{
  return (Types)gst_app_src_get_stream_type(As<GstAppSrc>());
}


bool PGstAppSrc::Push(const void * data, PINDEX size)
{
  gchar * bufPtr = (gchar*)g_malloc0 (size);
  memcpy(bufPtr, data, size);
#if !GST_CHECK_VERSION(1,0,0)
  GstBuffer * buffer = gst_app_buffer_new(bufPtr, size, (GstAppBufferFinalizeFunc)g_free, bufPtr);
#else
  GstBuffer * buffer = gst_buffer_new_wrapped(bufPtr, size);
#endif
  return gst_app_src_push_buffer(As<GstAppSrc>(), buffer) == GST_FLOW_OK;
}


bool PGstAppSrc::EndStream()
{
  return gst_app_src_end_of_stream(As<GstAppSrc>()) == GST_FLOW_OK;
}


///////////////////////////////////////////////////////////////////////

PGstAppSink::PGstAppSink(const PGstBin & bin, const char * name)
{
  bin.GetByName(name, *this);
}


bool PGstAppSink::Pull(void * data, PINDEX & size)
{
  if (!IsValid() || IsEndStream())
    return false;

#if !GST_CHECK_VERSION(1,0,0)
  GstBuffer * buffer = gst_app_sink_pull_buffer(As<GstAppSink>());
  if (buffer == NULL)
    return false;

  size = std::min(size, (PINDEX)GST_BUFFER_SIZE(buffer));
  memcpy(data, GST_BUFFER_DATA(buffer), size);
  gst_buffer_unref(buffer);
#else
  GstSample * sample = gst_app_sink_pull_sample(As<GstAppSink>());
  if (sample == NULL)
    return false;
  GstBuffer * buffer = gst_sample_get_buffer(sample);
  if (buffer == NULL) {
    gst_sample_unref(sample);
    return false;
  }
  gst_buffer_extract(buffer, 0, data, size);
  gst_sample_unref(sample);
#endif

  return true;
}


bool PGstAppSink::IsEndStream()
{
  return gst_app_sink_is_eos(As<GstAppSink>());
}


///////////////////////////////////////////////////////////////////////

PGstPipeline::PGstPipeline(const char * name)
{
  if (name != NULL && *name != '\0')
    Attach(gst_pipeline_new(name));
}


bool PGstPipeline::Parse(const char * description)
{
  if (description == NULL || *description == '\0')
    return false;

  PGError error;
  if (!Attach(gst_parse_launch(description, &error)) || error) {
    PTRACE(1, "GStreamer\tCould not parse pipeline \"" << description << "\", error: " << error);
    return false;
  }

  PTRACE(4, "GStreamer\tParsed pipeline \"" << description << '"');
  return true;
}


///////////////////////////////////////////////////////////////////////

PString PGstMessage::GetType() const
{
  if (!IsValid())
    return PString::Empty();

  return GST_MESSAGE_TYPE_NAME(As<GstMessage>());
}


void PGstMessage::PrintOn(ostream & strm) const
{
  if (!IsValid()) {
    strm << "(null)";
    return;
  }

  GstMessage * msg = As<GstMessage>();

  strm << GST_MESSAGE_TYPE_NAME(msg) << ": ";

  if (GST_MESSAGE_SRC(msg) != NULL) {
    gchar * srcName = gst_object_get_name(GST_MESSAGE_SRC(msg));
    strm << " src=" << srcName << ", ";
    g_free(srcName);
  }

  PGError error;
  gchar * debug = NULL;

  switch (GST_MESSAGE_TYPE(msg)) {
    case GST_MESSAGE_ERROR :
      gst_message_parse_error(msg, &error, &debug);
      break;
    case GST_MESSAGE_WARNING :
      gst_message_parse_warning(msg, &error, &debug);
      break;
    case GST_MESSAGE_INFO :
      gst_message_parse_info(msg, &error, &debug);
      break;
    case GST_MESSAGE_STATE_CHANGED :
      {
        GstState oldstate, newstate, pending;
        gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);
        strm <<       "old=" << gst_element_state_get_name(oldstate)
             <<     ", new=" << gst_element_state_get_name(newstate)
             << ", pending=" << gst_element_state_get_name(pending);
      }
      break;
    case GST_MESSAGE_EOS :
    default :
      break;
  }

  strm << error;
  if (debug != NULL) {
    strm << ", debug=" << debug;
    g_free(debug);
  }
}


///////////////////////////////////////////////////////////////////////

PGstBus::PGstBus(const PGstPipeline & pipeline)
{
  if (pipeline.IsValid())
    Attach(gst_pipeline_get_bus(pipeline.As<GstPipeline>()));
}


bool PGstBus::HavePending()
{
  return IsValid() && gst_bus_have_pending(As<GstBus>());
}


bool PGstBus::Peek(PGstMessage & message)
{
  return IsValid() && message.Attach(gst_bus_peek(As<GstBus>()));
}


bool PGstBus::Pop(PGstMessage & message)
{
  return IsValid() && message.Attach(gst_bus_pop(As<GstBus>()));
}


bool PGstBus::Pop(PGstMessage & message, PTimeInterval & wait)
{
  return IsValid() && message.Attach(gst_bus_timed_pop(As<GstBus>(), wait.GetMilliSeconds()*1000000));
}


static GstBusSyncReply MySyncHandler(GstBus * bus, GstMessage * message, gpointer data)
{
  PGstBus pbus;
  pbus.Attach(bus);

  PGstMessage pmessage;
  pmessage.Attach(message);

  PGstBus::Notifier & notifier = *reinterpret_cast<PGstBus::Notifier *>(data);

  notifier(pbus, pmessage);
  return GST_BUS_PASS;
}


bool PGstBus::SetNotifier(Notifier & notifier)
{
  if (!IsValid() || notifier.IsNULL())
    return false;

#if !GST_CHECK_VERSION(1,0,0)
  gst_bus_set_sync_handler(As<GstBus>(), MySyncHandler, &notifier);
#else
  gst_bus_set_sync_handler(As<GstBus>(), MySyncHandler, &notifier, NULL);
#endif

  return true;
}
#endif // P_GSTREAMER
