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

#include <ptbuildopts.h>

#if P_GSTREAMER

#include <list>


//////////////////////////////////////////////////////////////////////////////
// GStreamer classes

/** GLib object.
 */
class PGBaseObject : public PObject
{
    PCLASSINFO(PGBaseObject, PObject)
  protected:
    PGBaseObject() : m_object(NULL) { }

  private:
    PGBaseObject(const PGBaseObject &) { }
    void operator=(const PGBaseObject &) { }

  public:
    template<typename T> T * As() const { return reinterpret_cast<T *>(m_object); }
    void * Ptr() const { return m_object; }
    bool IsValid() const { return m_object != NULL; }
    operator bool() const { return m_object != NULL; }
    bool operator!() const { return m_object == NULL; }

    bool Attach(void * object);
    void * Detach();

  protected:
    virtual void Unreference() = 0;

  private:
    void * m_object;
};


class PGObject : public PGBaseObject
{
    PCLASSINFO(PGObject, PGBaseObject)
  protected:
    PGObject() { }
    PGObject(const PGObject & other);
    void operator=(const PGObject &);

  public:
    ~PGObject() { Unreference(); }

  protected:
    virtual void Unreference();

  public:
    bool Set(
      const char * attribute,
      const char * value
    );
    bool Get(
      const char * attribute,
      PString & value
    );
};


class PGstMiniObject : public PGBaseObject
{
    PCLASSINFO(PGstMiniObject, PGBaseObject)
  protected:
    PGstMiniObject() { }
    PGstMiniObject(const PGstMiniObject & other);
    void operator=(const PGstMiniObject &);

  public:
    ~PGstMiniObject() { Unreference(); }

  protected:
    virtual void Unreference();
};


class PGstObject : public PGObject
{
    PCLASSINFO(PGstObject, PGObject)
  public:

    PString GetName() const;
};


class PGstPluginFeature : public PGstObject
{
    PCLASSINFO(PGstPluginFeature, PGstObject)
  public:
    PString GetName() const;

    static PStringList Inspect(
      const char * klassRegex,
      bool detailed
    );
};


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


class PGstElement : public PGstObject
{
    PCLASSINFO(PGstElement, PGstObject)
  public:
    PGstElement() { }
    PGstElement(
      const char * factoryName,
      const char * name
    );

    bool Link(
      const PGstElement & dest
    );

    enum States {
      VoidPending,
      Null,
      Ready,
      Paused,
      Playing
    };

    enum StateResult {
      Failed,
      Success,
      Changing,
      NoPreRoll
    };
    StateResult SetState(
      States newState
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


class PGstBin : public PGstElement
{
    PCLASSINFO(PGstBin, PGstElement)
  public:
    bool AddElement(
      const PGstElement & element
    );
};


class PGstAppSrc : public PGstElement
{
    PCLASSINFO(PGstAppSrc, PGstElement)
  public:
    PGstAppSrc(
      const PGstBin & bin,
      const char * name
    );

    enum Types {
      Stream,
      Seekable,
      RandomAccess
    };
    void SetType(Types type);
    Types GetType() const;

    bool Push(
      const void * data,
      PINDEX size
    );

    bool EndStream();
};


class PGstAppSink : public PGstElement
{
    PCLASSINFO(PGstAppSink, PGstElement)
  public:
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


class PGstPipeline : public PGstBin
{
    PCLASSINFO(PGstPipeline, PGstBin)
  public:
    PGstPipeline(
      const char * name = NULL
    );

    bool Parse(
      const char * pipeline
    );
};


class PGstMessage : public PGstMiniObject
{
    PCLASSINFO(PGstMessage, PGstMiniObject)
  public:

    PString GetType() const;

    void PrintOn(ostream & strm) const;
};


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
    bool POp(PGstMessage & message, PTimeInterval & wait);

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
