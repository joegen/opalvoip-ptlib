/*
 * gstreamer.h
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

#ifndef PTLIB_GSTREMER_H
#define PTLIB_GSTREMER_H

#ifdef P_USE_PRAGMA
#pragma interface
#endif

#include <ptlib.h>

#if P_GSTREAMER

#include <list>


//////////////////////////////////////////////////////////////////////////////
// GStreamer classes


/// GLib object base wrapper class.
class PGBaseObject : public PObject
{
    PCLASSINFO(PGBaseObject, PObject)
  protected:
    PGBaseObject();

  private:
    PGBaseObject(const PGBaseObject &) { }
    void operator=(const PGBaseObject &) { }

  public:
    ~PGBaseObject();

    template<typename T> T * As() const { return reinterpret_cast<T *>(m_object); }
    void * Ptr() const { return m_object; }
    operator bool() const { return m_object != NULL; }
    bool operator!() const { return m_object == NULL; }

    virtual bool IsValid() const;
    virtual bool Attach(void * object);
    void * Detach();
    void SetNULL();

  protected:
    virtual void Unreference() = 0;

  private:
    void * m_object;
};


/**GLib object wrapper class.
   https://developer.gnome.org/gobject/unstable/gobject-The-Base-Object-Type.html
  */
class PGObject : public PGBaseObject
{
    PCLASSINFO(PGObject, PGBaseObject)
  protected:
    PGObject() { }
    PGObject(const PGObject & other);
    void operator=(const PGObject &);

  public:
    ~PGObject() { SetNULL(); }

  protected:
    virtual void Unreference();

  public:
    virtual bool IsValid() const;

    bool Set(
      const char * attribute,
      bool value
    );
    bool Get(
      const char * attribute,
      bool & value
    );

    bool Set(
      const char * attribute,
      int value
    );
    bool Get(
      const char * attribute,
      int & value
    );

    bool Set(
      const char * attribute,
      double value
    );
    bool Get(
      const char * attribute,
      double & value
    );

    bool Set(
      const char * attribute,
      const char * value
    );
    bool Get(
      const char * attribute,
      PString & value
    );
};


/**GLib mini-object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstMiniObject.html
  */
class PGstMiniObject : public PGBaseObject
{
    PCLASSINFO(PGstMiniObject, PGBaseObject)
  protected:
    PGstMiniObject() { }
    PGstMiniObject(const PGstMiniObject & other);
    void operator=(const PGstMiniObject &);

  public:
    ~PGstMiniObject() { SetNULL(); }

    virtual bool IsValid() const;

  protected:
    virtual void Unreference();
};


/**GStreamer object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstObject.html
  */
class PGstObject : public PGObject
{
    PCLASSINFO(PGstObject, PGObject)
  public:

    PString GetName() const;

    bool SetSockFd(int fd);
};


/**GStreamer plug in feature object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstPluginFeature.html
  */
class PGstPluginFeature : public PGstObject
{
    PCLASSINFO(PGstPluginFeature, PGstObject)
  public:
    PString GetName() const;

    enum InspectSearchField {
      ByKlass,
      ByName,
      ByLongName,
      ByDescription
    };
    static PStringList Inspect(
      const char * regex,
      InspectSearchField searchField,
      bool detailed = true
    );
    static PStringList Inspect(
      const char * regex,
      bool detailed = true
    ) { return Inspect(regex, ByKlass, detailed); }
};


/**GStreamer element factory object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElementFactory.html
  */
class PGstElementFactory : public PGstPluginFeature
{
    PCLASSINFO(PGstElementFactory, PGstPluginFeature)
  public:
    PGstElementFactory(
      const char * factoryName = NULL
    );

    PString GetKlass() const;
    PString GetDescription() const;
};


/**GStreamer element object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstElement.html
  */
class PGstElement : public PGstObject
{
    PCLASSINFO(PGstElement, PGstObject)
  public:
    PGstElement() { }
    PGstElement(
      const char * factoryName,
      const char * name
    );

    PString GetName() const;
    bool SetName(const PString & name);

    bool Link(
      const PGstElement & dest
    );

    P_DECLARE_TRACED_ENUM(States,
      VoidPending,
      Null,
      Ready,
      Paused,
      Playing
    );

    P_DECLARE_TRACED_ENUM(StateResult,
      Failed,
      Success,
      Changing,
      NoPreRoll
    );
    StateResult SetState(
      States newState,
      const PTimeInterval & timeout = 0
    );
    StateResult GetState(
      States & state
    );
    StateResult GetPendingState(
      States & state
    );
    StateResult WaitStateChange();
    StateResult WaitStateChange(
      const PTimeInterval & timeout
    );
};


/**GStreamer iterator object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstIterator.html
  */
class PGstBaseIterator : public PGBaseObject
{
    PCLASSINFO(PGstBaseIterator, PGBaseObject)
  public:
    ~PGstBaseIterator() { SetNULL(); }

    virtual bool Attach(void * object);

    P_DECLARE_TRACED_ENUM(Result,
      Done,
      Success,
      NeedResync,
      Error
    );
    Result operator++() { return InternalNext(); }

    Result GetLastResult() const { return m_lastResult; }

    void Resync();

  protected:
    PGstBaseIterator(PGBaseObject & valueRef);

    virtual void Unreference();
    Result InternalNext();

    PGBaseObject & m_valueRef;
    Result         m_lastResult;
};


template <class T>
class PGstIterator : public PGstBaseIterator
{
    PCLASSINFO(PGstIterator, PGstBaseIterator)
  public:
    PGstIterator() : PGstBaseIterator(m_value) { }
    const T * operator->() const { return &m_value; }
    const T & operator*() const  { return  m_value; }
  protected:
    T m_value;
};


/**GStreamer bin object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBin.html
  */
class PGstBin : public PGstElement
{
    PCLASSINFO(PGstBin, PGstElement)
  public:
    bool AddElement(
      const PGstElement & element
    );

    bool GetByName(
      const char * name,
      PGstElement & element
    ) const;

    bool GetElements(
      PGstIterator<PGstElement> & iterator,
      bool recursive = false
    ) const;
};


/**GStreamer appsrc object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsrc.html
  */
class PGstAppSrc : public PGstElement
{
    PCLASSINFO(PGstAppSrc, PGstElement)
  public:
    PGstAppSrc() { }
    PGstAppSrc(
      const PGstBin & bin,
      const char * name
    );

    P_DECLARE_TRACED_ENUM(Types,
      Stream,
      Seekable,
      RandomAccess
    );
    void SetType(Types type);
    Types GetType() const;

    bool Push(
      const void * data,
      PINDEX size
    );

    bool EndStream();
};


/**GStreamer appsink object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gst-plugins-base-libs/html/gst-plugins-base-libs-appsink.html
  */
class PGstAppSink : public PGstElement
{
    PCLASSINFO(PGstAppSink, PGstElement)
  public:
    PGstAppSink() { }
    PGstAppSink(
      const PGstBin & bin,
      const char * name
    );

    bool Pull(
      void * data,
      PINDEX & size
    );

    bool IsEndStream();
};


/**GStreamer pipeline object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstPipeline.html
  */
class PGstPipeline : public PGstBin
{
    PCLASSINFO(PGstPipeline, PGstBin)
  public:
    PGstPipeline(
      const char * name = NULL
    );

    bool Parse(
      const char * description
    );
};


/**GStreamer message object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/gstreamer-GstMessage.html
  */
class PGstMessage : public PGstMiniObject
{
    PCLASSINFO(PGstMessage, PGstMiniObject)
  public:

    PString GetType() const;

    void PrintOn(ostream & strm) const;
};


/**GStreamer bus object wrapper class.
   http://gstreamer.freedesktop.org/data/doc/gstreamer/head/gstreamer/html/GstBus.html
  */
class PGstBus : public PGstObject
{
    PCLASSINFO(PGstBus, PGstObject)
  public:
    PGstBus() { }
    PGstBus(
      const PGstPipeline & pipeline
    );

    bool HavePending();
    bool Peek(PGstMessage & message);
    bool Pop(PGstMessage & message);
    bool Pop(PGstMessage & message, PTimeInterval & wait);

    typedef PNotifierTemplate<PGstMessage> Notifier;
    #define PDECLARE_GstBusNotifier(cls, fn) PDECLARE_NOTIFIER2(PGstBus, cls, fn, PGstMessage)
    #define PDECLARE_ASYNC_GstBusNotifier(cls, fn) PDECLARE_ASYNC_NOTIFIER2(PGstBus, cls, fn, PGstMessage)
    #define PCREATE_GstBusNotifier(fn) PCREATE_NOTIFIER2(fn, PGstMessage)

    bool SetNotifier(
      Notifier & notifier
    );
};


#endif // P_GSTREAMER

#endif // PTLIB_GSTREMER_H


// End Of File ///////////////////////////////////////////////////////////////
