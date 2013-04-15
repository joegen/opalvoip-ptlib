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

#pragma message("GStreamer support enabled")

#ifdef _MSC_VER
  #pragma warning(disable:4127)
  #include <gst/gst.h>
  #pragma warning(default:4127)

  #pragma comment(lib, P_GSTREAMER_LIBRARY)
  #pragma comment(lib, P_GST_APP_LIBRARY)
  #pragma comment(lib, P_GOBJECT_LIBRARY)
  #pragma comment(lib, P_GLIB_LIBRARY)
#else
  #include <gst/gst.h>
#endif

#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>
#include <gst/app/gstappbuffer.h>


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
                        GstDebugLevel level,
                        const gchar *file,
                        const gchar *function,
                        gint line,
                        GObject * /*object*/,
                        GstDebugMessage *message,
                        gpointer /*data*/)
{
  PTrace::Begin(level, file+1, line, NULL)
        << "GStreamer\t"
        << function << ": "
        << gst_debug_message_get(message)
        << PTrace::End;
}
#endif


class PGstInitialiser : public PProcessStartup
{
    PCLASSINFO(PGstInitialiser, PProcessStartup)
  public:
    virtual void OnStartup()
    {
#if PTRACING
      gst_debug_add_log_function(LogFunction, NULL);
      gst_debug_set_default_threshold(GST_LEVEL_DEBUG);
      gst_debug_set_active(true);
#endif

      PGError error;
      if (gst_init_check(NULL, NULL, &error)) {
        PTRACE(3, "GStreamer\tUsing version " << gst_version_string());
      }
      else {
        PTRACE(1, "GStreamer\tCould not initialise, error: " << error);
      }
    }

    virtual void OnShutdown()
    {
      gst_deinit();

#if PTRACING
      gst_debug_set_active(false);
      gst_debug_add_log_function(NULL, NULL);
#endif
    }
};

static PFactory<PProcessStartup>::Worker<PGstInitialiser> PGstInitialiserInstance("GStreamer", true);


///////////////////////////////////////////////////////////////////////

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


///////////////////////////////////////////////////////////////////////

PGObject::PGObject(const PGObject & other)
{
  Attach(gst_object_ref(other.Ptr()));
}


void PGObject::operator=(const PGObject & other)
{
  Attach(gst_object_ref(other.Ptr()));
}


void PGObject::Unreference()
{
  if (IsValid())
    gst_object_unref(Ptr());
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


///////////////////////////////////////////////////////////////////////

PGstMiniObject::PGstMiniObject(const PGstMiniObject & other)
{
  Attach(gst_mini_object_ref(other.As<GstMiniObject>()));
}


void PGstMiniObject::operator=(const PGstMiniObject & other)
{
  Attach(gst_mini_object_ref(other.As<GstMiniObject>()));
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


PStringList PGstPluginFeature::Inspect(const char * klassRegex, bool detailed)
{
  PStringList elements;

  PRegularExpression regex(klassRegex == NULL || *klassRegex == '\0' ? "*" : klassRegex);

  GstRegistry * registry = gst_registry_get_default();

  GList * pluginList = gst_registry_get_plugin_list(registry);
  for (GList * pluginIter = pluginList; pluginIter != NULL; pluginIter = g_list_next(pluginIter)) {
    GstPlugin * plugin = GST_PLUGIN(pluginIter->data);

    GList * featureList = gst_registry_get_feature_list_by_plugin(registry, plugin->desc.name);
    for (GList * featureIter = featureList; featureIter != NULL; featureIter = g_list_next(featureIter)) {
      GstPluginFeature * feature = GST_PLUGIN_FEATURE(featureIter->data);

      if (GST_IS_ELEMENT_FACTORY(feature)) {
        GstElementFactory * factory = GST_ELEMENT_FACTORY(feature);
        PINDEX dummy;
        if (regex.Execute(factory->details.klass, dummy)) {
          PStringStream info;
          info << GST_PLUGIN_FEATURE_NAME(factory);
          if (detailed) {
            info << '\t' << factory->details.klass
                 << '\t' << factory->details.longname
                 << '\t' << factory->details.description;
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


bool PGstElement::Link(const PGstElement & dest)
{
  return IsValid() && dest.IsValid() && gst_element_link(As<GstElement>(), dest.As<GstElement>());
}


PGstElement::StateResult PGstElement::SetState(States newState)
{
  return IsValid() ? (StateResult)gst_element_set_state(As<GstElement>(), (GstState)newState) : Failed;
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

bool PGstBin::AddElement(const PGstElement & element)
{
  return IsValid() && element.IsValid() && gst_bin_add(As<GstBin>(), element.As<GstElement>());
}


///////////////////////////////////////////////////////////////////////

PGstAppSrc::PGstAppSrc(const PGstBin & bin, const char * name)
{
  if (bin.IsValid())
    Attach(gst_bin_get_by_name(bin.As<GstBin>(), name));
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
  GstBuffer * buffer = gst_app_buffer_new(bufPtr, size, (GstAppBufferFinalizeFunc)g_free, bufPtr);
  return gst_app_src_push_buffer(As<GstAppSrc>(), buffer) == GST_FLOW_OK;
}


bool PGstAppSrc::EndStream()
{
  return gst_app_src_end_of_stream(As<GstAppSrc>()) == GST_FLOW_OK;
}


///////////////////////////////////////////////////////////////////////

PGstAppSink::PGstAppSink(const PGstBin & bin, const char * name)
{
  if (bin.IsValid())
    Attach(gst_bin_get_by_name(bin.As<GstBin>(), name));
}


bool PGstAppSink::Pull(void * data, PINDEX & size)
{
  GstBuffer * buffer = gst_app_sink_pull_buffer(As<GstAppSink>());
  if (buffer == NULL)
    return false;

  if (size > (PINDEX)buffer->size)
    size = buffer->size;

  memcpy(data, buffer->data, size);
  gst_buffer_unref(buffer);
  return true;
}


bool PGstAppSink::IsEndStream()
{
  return gst_app_sink_is_eos(As<GstAppSink>()) == GST_FLOW_OK;
}


///////////////////////////////////////////////////////////////////////

PGstPipeline::PGstPipeline(const char * name)
{
  if (name != NULL && *name != '\0')
    Attach(gst_pipeline_new(name));
}


bool PGstPipeline::Parse(const char * pipeline)
{
  PGError error;
  if (!Attach(gst_parse_launch(pipeline, &error))) {
    PTRACE(1, "GStreamer\tCould not parse pipline \"" << pipeline << "\", error: " << error);
    return false;
  }

  if (!error) {
    PTRACE(4, "GStreamer\tParsed pipline \"" << pipeline << '"');
    return true;
  }

  PTRACE(2, "GStreamer\tError parsing pipline \"" << pipeline << "\": " << error);
  return false;
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
    case GST_MESSAGE_EOS :
      break;
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


bool PGstBus::POp(PGstMessage & message, PTimeInterval & wait)
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

  gst_bus_set_sync_handler(As<GstBus>(), MySyncHandler, &notifier);
  return true;
}


///////////////////////////////////////////////////////////////////////

#else
  #pragma message("GStreamer support DISABLED")
#endif // P_GSTREAMER


// End Of File ///////////////////////////////////////////////////////////////
